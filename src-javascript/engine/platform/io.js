"use strict";

const IO_REQUEST_STRING = 0;
const IO_REQUEST_BLOB = 1;
const IO_REQUEST_BITMAP = 2;
const IO_REQUEST_JSON = 3;
const IO_REQUEST_ARRAYBUFFER = 4;
const IO_REQUEST_HEAD = 5;

const IO_CHROMIUM_DETECTED = navigator.userAgent.includes("Chrome/") && location.protocol == "file:";

/** @type {Worker} **/
var IO_WORKER;
var IO_IDS = 0;
var IO_QUEUE = new Map();
var IO_BASE_URL = null;
var __fetch = null;


class KDMYEngineIOError extends Error {
    constructor(serialized_error_or_message, url) {
        if (typeof (serialized_error_or_message) == 'string') {
            super(serialized_error_or_message);
        } else {
            let msg = serialized_error_or_message.message;
            if (url) msg += `\n${url}`;

            // @ts-ignore
            super(msg, serialized_error_or_message.fileName, serialized_error_or_message.lineNumber);

            /*for (let prop in serialized_error) {
                if (prop != "message")
                    this[prop] = serialized_error[prop];
            }*/
        }

        this.name = "KDMYEngineIOError";
        this.url = url;
    }
}

if (IO_CHROMIUM_DETECTED) {
    //
    // In Chromium-based web browsers, fetch can not handle the "file://" URI
    // scheme, use XMLHttpRequest instead. Also "NavigatorUAData" is only available
    // in those web browsers
    //

    if (!location.href.endsWith(".js")) console.info("IO: using chromium workaround");

    __fetch = fetch;
    // @ts-ignore
    self.fetch = io_chromium_workaround;
}

if (location.href.endsWith(".js")) {
    addEventListener("message", function (evt) {
        if (!IO_BASE_URL) {
            IO_BASE_URL = evt.data;
            return;
        }
        io_background_load(evt.data.src, evt.data.type, evt.data.operation_id);
    });
} else {
    IO_WORKER = new Worker("engine/platform/io.js", { name: "io" });
    IO_WORKER.addEventListener("message", function (evt) {
        io_foreground_fulfill(evt.data.error, evt.data.data, evt.data.operation_id);
    });
    IO_BASE_URL = location.href.substring(0, location.href.lastIndexOf('/') + 1);
    IO_WORKER.postMessage(IO_BASE_URL);
    window.addEventListener("beforeunload", function () {
        IO_WORKER.terminate();
    });
}

async function io_background_load(src, type, operation_id) {
    /** @type {DedicatedWorkerGlobalScope} */
    /* @ts-ignore */
    const ctx = self;

    let url = src;
    let res, data;
    try {
        url = io_get_absolute_path(src);

        if (type == IO_REQUEST_HEAD) {
            res = await fetch(url, { method: 'HEAD' });
            data = {
                ok: res.ok,
                size: res.headers.get("Content-Length"),
                mime: res.headers.get("content-type"),
                url: res.url
            };
            ctx.postMessage({ operation_id, data });
            return;
        }

        res = await fetch(url);
        let transferable = null;

        switch (type) {
            case IO_REQUEST_STRING:
                data = await res.text();
                break;
            case IO_REQUEST_BLOB:
                data = await res.blob();
                break;
            case IO_REQUEST_BITMAP:
                let blob = await res.blob();
                transferable = await createImageBitmap(blob);
                data = {
                    data: transferable,
                    size: blob.size,
                    original_width: transferable.width,
                    original_height: transferable.height,
                    width: transferable.width,
                    height: transferable.height
                };
                break;
            case IO_REQUEST_JSON:
                data = await res.json();
                break;
            case IO_REQUEST_ARRAYBUFFER:
                data = await res.arrayBuffer();
                transferable = data;
                break;
            default:
                ctx.postMessage({ operation_id, error: "unknown io request type:" + type });
                return;
        }

        if (transferable) {
            ctx.postMessage({ operation_id, data }, [transferable]);
        } else
            ctx.postMessage({ operation_id, data });
    } catch (e) {
        if (e instanceof ProgressEvent) {
            e = { message: e.target["statusText"] };
            if (!e.message) e.message = "XMLHttpRequest failed (probably the file was not found)";
        }
        ctx.postMessage({ operation_id, error: io_background_serialize_error(e), data: url });
    }
}

function io_foreground_fulfill(error, data, operation_id) {
    for (const [op_id, callbacks] of IO_QUEUE) {
        if (op_id != operation_id) continue;

        IO_QUEUE.delete(operation_id);

        if (error)
            callbacks.reject(new KDMYEngineIOError(error, data));
        else
            callbacks.resolve(data);
        return;
    }

    if (data instanceof ImageBitmap) data.close();
    throw new KDMYEngineIOError("Unknown operation id:" + operation_id);
}

function io_foreground_request(src, type) {
    // in C dispose the "src"
    return new Promise(function (resolve, reject) {
        let operation_id = IO_IDS++;
        IO_QUEUE.set(operation_id, { resolve, reject });
        IO_WORKER.postMessage({ src, type, operation_id });
    });
}

function io_background_serialize_error(e) {
    let props = Object.getOwnPropertyNames(e);
    let obj = {};
    for (let prop of props) {
        let descriptor = Object.getOwnPropertyDescriptor(e, prop);
        if (descriptor.value && !(descriptor.value instanceof Function)) {
            if (prop == "name") prop = "sourceName";
            obj[prop] = descriptor.value;
        }
    }
    return obj;
}

async function io_resource_exists(src, expect_file, expect_folder) {
    // in C dispose the "src"
    try {
        let res = await io_foreground_request(src, IO_REQUEST_HEAD);
        if (!res.ok) return 0;

        if (expect_file && expect_folder) return 1;

        if (IO_CHROMIUM_DETECTED) {
            let is_folder = res.url.endsWith("/");
            if (expect_folder && is_folder) return 1;
            if (expect_file && !is_folder) return 1;
            return 0;
        } else if (res.mime == "application/http-index-format") {
            return expect_folder;
        } else {
            return expect_file;
        }

        throw new KDMYEngineIOError("No IO implementation for: " + navigator.userAgent);
    } catch (e) {
        return 0;
    }
}

async function io_file_size(src, file_handle) {
    // needs C version
    if (file_handle) throw new NotImplementedError("file_handle was specified in JS version");
    try {
        let res = await io_foreground_request(src, IO_REQUEST_HEAD);
        if (!res.ok) return -1;

        let length = parseInt(res.headers.get("Content-Length"));
        if (Number.isFinite(length) && length >= 0) return length;

        // second attempt
        let blob = await res.blob();
        return blob.size;
    } catch (e) {
        return -1;
    }
}

function io_get_absolute_path(src) {
    let relative = "\\/".includes(src.charAt(0)) ? src.substring(1) : src;
    let url = new URL(relative, IO_BASE_URL);
    return url.toString();
}

function io_chromium_workaround(url, options) {
    /*if (!url.startsWith("file:")) {
        return __fetch.call(null, arguments);
    }*/

    let method = options?.method;
    if (!method) method = "GET";

    function read_arrayBuffer(arraybuffer) {
        return arraybuffer;
    }

    async function read_blob(arraybuffer) {
        return new Blob([arraybuffer]);
    }

    async function read_text(arraybuffer) {
        let decoder = new TextDecoder();
        return decoder.decode(arraybuffer);
    }

    async function read_json(arraybuffer) {
        let decoder = new TextDecoder();
        let text = decoder.decode(arraybuffer);
        return JSON.parse(text);
    }

    function main(resolve, reject) {
        let request = new XMLHttpRequest();
        request.addEventListener('error', reject);
        request.addEventListener('abort', reject);
        request.addEventListener('load', function () {
            let headers = new Headers();
            for (let line of request.getAllResponseHeaders().trim().split(/[\r\n]+/)) {
                if (!line) continue;
                var parts = line.split(': ');
                var header = parts.shift();
                var value = parts.join(': ');
                headers.append(header, value);
            }
            resolve({
                ok: request.status == 200 || request.status == 0,
                headers: headers,
                url: request.responseURL,
                arrayBuffer: function () { return read_arrayBuffer(request.response) },
                blob: function () { return read_blob(request.response) },
                text: function () { return read_text(request.response) },
                json: function () { return read_json(request.response) }
            });
        });
        request.responseType = "arraybuffer";
        request.open('GET', url, true);
        request.send();
    }

    return new Promise(main);
}

