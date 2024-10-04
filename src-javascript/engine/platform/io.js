"use strict";

/**
 * @typedef {object} IndexListingEntry
 * @property {string} name
 * @property {'file'|'directory'|'unknown'} type
 * @property {number} fileLength
 * @property {number} directoryIndex
**/
/**
 * @typedef {object} IndexListing
 * @property {IndexListingEntry[][]} directories
 * @property {number} rootIndex
**/

const IO_REQUEST_TEXT = 0;
const IO_REQUEST_BLOB = 1;
const IO_REQUEST_BITMAP = 2;
const IO_REQUEST_JSON = 3;
const IO_REQUEST_ARRAYBUFFER = 4;
const IO_REQUEST_HEAD = 5;

const IO_WEBKIT_DETECTED =
    (
        navigator.userAgent.includes("Chrome/") ||
        navigator.userAgent.includes("AppleWebKit/") ||
        navigator.userAgent.includes("Safari/") ||
        navigator.userAgent.includes("OPR/")
    ) && location.protocol == "file:";

var __fetch = null;

/**@type {IndexListing}*/
var io_indexlisting = null;
var io_indexlisting_loaded = false;


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

if (IO_WEBKIT_DETECTED) {
    //
    // In Chromium-based web browsers, fetch can not handle the "file://" URI
    // scheme, use XMLHttpRequest instead. Also "NavigatorUAData" is only available
    // in those web browsers
    //

    if (!location.href.endsWith(".js")) console.info("IO: using chromium workaround");

    __fetch = fetch;
    // @ts-ignore
    this.fetch = io_chromium_workaround;
}

const IO_BASE_URL = location.href.substring(0, location.href.lastIndexOf('/') + 1);


async function io_background_load_resource(url, type) {
    /**@type {Response} */
    let res = null;

    try {
        if (type == IO_REQUEST_HEAD) {
            res = await fetch(url, { method: 'HEAD' });
            let data = {
                is_head_request: true,
                ok: res.ok,
                size: res.headers.get("Content-Length"),
                mime: res.headers.get("content-type"),
                url: res.url
            };
            return data;
        }

        res = await fetch(url);

        switch (type) {
            case IO_REQUEST_TEXT:
                return await res.text();
            case IO_REQUEST_BLOB:
                return res.blob();
            case IO_REQUEST_BITMAP:
                let blob = await res.blob();
                let imagebitmap = await createImageBitmap(blob);
                return {
                    data: imagebitmap,
                    size: blob.size,
                    original_width: imagebitmap.width,
                    original_height: imagebitmap.height,
                    width: imagebitmap.width,
                    height: imagebitmap.height
                };
            case IO_REQUEST_JSON:
                return await res.json();
            case IO_REQUEST_ARRAYBUFFER:
                return await res.arrayBuffer();
            default:
                throw new Error("unknown io request type:");
        }
    } catch (e) {
        if (e instanceof ProgressEvent) {
            e = { message: e.target["statusText"] };
            if (!e.message) e.message = "XMLHttpRequest failed (probably the file was not found)";
        }
        throw new KDMYEngineIOError(io_background_serialize_error(e), url);
    }
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


async function io_request_file(absolute_url, request_type) {
    let native_url = await io_native_get_path(absolute_url, true, false, true);
    try {
        return await io_background_load_resource(native_url, request_type);
    } catch (e) {
        console.error(e);
        return null;
    }
}

async function io_file_size(absolute_url) {
    let native_url = await io_native_get_path(absolute_url, true, false, true);
    return await io_native_file_size(native_url);
}

async function io_resource_exists(absolute_url, expect_file, expect_folder) {
    let native_url = await io_native_get_path(
        absolute_url, expect_file, expect_folder, expect_file || expect_folder
    );
    return await io_native_resource_exists(native_url, expect_file, expect_folder);
}

async function io_enumerate_folder(absolute_url) {
    let native_url = await io_native_get_path(absolute_url, false, true, true);
    return await io_native_enumerate_folder(native_url);
}

async function io_native_file_size(absolute_file_url) {
    try {
        if (io_indexlisting || await io_native_indexListing_file_size()) {
            return io_native_indexListing_file_size(absolute_file_url);
        }

        let res = await io_background_load_resource(absolute_file_url, IO_REQUEST_HEAD);
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

async function io_native_resource_exists(absolute_url, expect_file, expect_folder) {
    try {
        if (io_indexlisting || await io_native_indexListing_load()) {
            return io_native_indexListing_exists(absolute_url, expect_file, expect_folder);
        }

        let res = await io_background_load_resource(absolute_url, IO_REQUEST_HEAD);
        if (!res.ok) return false;

        if (expect_file && expect_folder) return true;

        if (IO_WEBKIT_DETECTED) {
            let is_folder = res.url.endsWith("/");
            if (expect_folder && is_folder) return true;
            if (expect_file && !is_folder) return true;
            return 0;
        } else if (res.mime == "application/http-index-format") {
            return expect_folder;
        } else if (res.is_head_request) {
            // should be a file
            return expect_file;
        } else {
            let unimpl = new Error("No IO implementation for: " + navigator.userAgent + ". When checking " + absolute_url);
            console.error(unimpl);
            throw unimpl;
        }
    } catch (e) {
        //console.error(e);
        return false;
    }
}

async function io_native_enumerate_folder(absolute_url) {
    if (io_indexlisting || await io_native_indexListing_load()) {
        return io_native_indexListing_enumerate_folder(absolute_url);
    } else if (navigator.userAgent.includes("Gecko/")) {
        let text = await io_background_load_resource(absolute_url, IO_REQUEST_TEXT);
        if (!text) return null;
        return io_native_parse_httpIndexFormat(text);
    } else if (IO_WEBKIT_DETECTED) {
        let text = await io_background_load_resource(absolute_url, IO_REQUEST_TEXT);
        if (!text) return null;
        return io_native_parse_webkit_indexDirectory(text);
    }

    throw new KDMYEngineIOError("Unknown web browser, newio_enumerate_folder() can not continue");
}

async function io_native_get_path(absolute_url, is_file, is_folder, resolve_expansion) {
    let base_path = IO_BASE_URL;
    let path = absolute_url;
    let index = 0;

    if (resolve_expansion && absolute_url.startsWith("/assets", 0)) {
        path = await expansions_resolve_path(path, is_file, is_folder);
    }

    if (path.startsWith("/~assets") || path.startsWith("/~expansions")) {
        index = 2;
    } else {
        switch (path[0]) {
            case '/':
            case '\\':
                index = 1;
                break;
        }

        if (!path.startsWith("assets", index) && !path.startsWith("expansions", index)) {
            console.warn(`io_get_absolute_path() path outside of 'assets' or 'expansions' folder: ${path}`);
            path = "assets";
        }
    }

    let new_path = new URL(path.substring(index), base_path);

    return new_path.href;
}


function io_native_parse_httpIndexFormat(/**@type {string}*/content) {
    /**@type {Array<{ name: string, length: number, is_file: boolean, is_folder: boolean }>} */
    let parsed_entries = new Array();

    let lines = content.split("\n");
    let search_field_names = true;
    let index_name = -1, index_length = -1, index_type = -1;

    function _replace_split(/**@type {string}*/line) {
        let ret = line.replace(/[\s\t]+/g, " ").trim().split(" ");
        ret.shift();
        return ret;
    }

    for (let line of lines) {
        if (search_field_names) {
            if (!line.startsWith("200:")) continue;

            let field_names = _replace_split(line);
            for (let i = 0; i < field_names.length; i++) {
                switch (field_names[i].toLowerCase()) {
                    case "filename":
                        index_name = i;
                        break;
                    case "content-length":
                        index_length = i;
                        break;
                    case "file-type":
                        index_type = i;
                        break;
                }
            }

            search_field_names = false;
            continue;
        }

        if (!line.startsWith("201:")) continue;

        let unparsed_entry = _replace_split(line);

        let parsed_entry = { name: null, length: -1, is_file: false, is_folder: false };


        if (index_name >= 0) {
            parsed_entry.name = decodeURIComponent(unparsed_entry[index_name]);
        }
        if (index_length >= 0) {
            parsed_entry.length = parseInt(decodeURIComponent(unparsed_entry[index_length]));
        }

        if (index_type >= 0) {
            switch (unparsed_entry[index_type].toUpperCase()) {
                case "FILE":
                case "SYM-FILE":
                    parsed_entry.is_file = true;
                    break;
                case "DIRECTORY":
                case "SYM-DIRECTORY":
                    parsed_entry.is_folder = true;
                    break;
            }
        }

        parsed_entries.push(parsed_entry);
    }

    return parsed_entries;
}

function io_native_parse_webkit_indexDirectory(/**@type {string}*/html) {
    const PREFIX = "addRow(";
    const SUFFIX = ");";
    const INDEX_NAME = 0;
    //const INDEX_URL = 1;
    const INDEX_ISDIR = 2;
    const INDEX_SIZE = 3;
    //const INDEX_SIZE_STRING = 4;
    //const INDEX_DATE_MODIFIED = 5;
    //const INDEX_DATE_MODIFIED_STRING = 6;

    /**@type {Array<{ name: string, length: number, is_file: boolean, is_folder: boolean }>} */
    let parsed_entries = new Array();
    let doc = (new DOMParser()).parseFromString(html, "text/html");

    // extract the entries
    for (let script of doc.querySelectorAll("script")) {
        // step 1: find all "<script>addArrow(...);</script>"
        let content = script.textContent.trim();
        if (!content.startsWith(PREFIX)) continue;
        if (!content.endsWith(SUFFIX)) break;// unexpected

        // step 2: extract the entry
        let entry = content.substring(PREFIX.length, content.length - SUFFIX.length);

        // step 3: parse the entry
        let fake_json = `[${entry}]`;
        let fake_parsed_json = JSON.parse(fake_json);

        // step 4: read the entry
        parsed_entries.push({
            name: fake_parsed_json[INDEX_NAME],
            is_folder: fake_parsed_json[INDEX_ISDIR],
            is_file: !fake_parsed_json[INDEX_ISDIR],
            length: fake_parsed_json[INDEX_SIZE]
        });
    }

    return parsed_entries;
}


async function io_native_indexListing_load() {
    if (io_indexlisting_loaded) return;

    let indexlisting_src = new URL("indexlisting.json", IO_BASE_URL);
    console.log(`io_native_enumerate_folder() attemping to read ${indexlisting_src}`);
    console.info("io_native_enumerate_folder() hint: to update/create indexlisting.json file " +
        'run "kdy_e.exe -indexlisting" or "funkin.exe -indexlisting"'
    );

    io_indexlisting_loaded = true;
    io_indexlisting = await io_background_load_resource(indexlisting_src, IO_REQUEST_JSON);

    if (io_indexlisting && io_indexlisting.directories?.length > 0) {
        console.log(`io_native_enumerate_folder() loaded with ${io_indexlisting.directories.length} entries`);
        return true;
    }

    console.log(`io_native_enumerate_folder() invalid indexlisting.json file`);
    return false;
}

function io_native_indexListing_enumerate_folder(/**@type {string}*/absolute_url) {
    absolute_url = absolute_url.substring(IO_BASE_URL.length);

    let subs = absolute_url.split(FS_CHAR_SEPARATOR).filter(sub => sub.length > 0);
    let current = io_indexlisting.directories[io_indexlisting.rootIndex];

    for (let sub of subs) {
        L_search: {
            for (let cur of current) {
                if (cur.type != "directory") continue;
                if (cur.name.localeCompare(sub, undefined, { sensitivity: "accent" }) != 0) continue;

                current = io_indexlisting.directories[cur.directoryIndex];
                break L_search;
            }

            // subdirectory not found
            return null;
        }
    }

    /**@type {Array<{ name: string, length: number, is_file: boolean, is_folder: boolean }>} */
    let entries = new Array(current.length);

    // process entries
    for (let i = 0; i < entries.length; i++) {
        let entry = current[i];

        entries[i] = {
            name: entry.name,
            length: entry.type == "file" ? entry.fileLength : -1,
            is_file: entry.type == "file",
            is_folder: entry.type == "directory"
        };
    }

    return entries;
}

function io_native_indexListing_exists(/**@type {string}*/absolute_url, is_file, is_folder) {
    absolute_url = absolute_url.substring(IO_BASE_URL.length);

    let subs = absolute_url.split(FS_CHAR_SEPARATOR).filter(sub => sub.length > 0);
    let current = io_indexlisting.directories[io_indexlisting.rootIndex];
    let last = subs.length - 1;

    for (let i = 0; i < subs.length; i++) {
        L_search: {
            for (let cur of current) {
                if (cur.name.localeCompare(subs[i], undefined, { sensitivity: "accent" }) != 0) {
                    continue;
                } else if (i == last) {
                    return (cur.type == "file" && is_file) || (cur.type == "directory" && is_folder);
                } else if (cur.type == "directory") {
                    current = io_indexlisting.directories[cur.directoryIndex];
                    break L_search;
                }
            }

            // subdirectory not found
            break;
        }
    }

    return false;
}

function io_native_indexListing_file_size(/**@type {string}*/absolute_url) {
    absolute_url = absolute_url.substring(IO_BASE_URL.length);

    let subs = absolute_url.split(FS_CHAR_SEPARATOR).filter(sub => sub.length > 0);
    let current = io_indexlisting.directories[io_indexlisting.rootIndex];
    let last = subs.length - 1;

    for (let i = 0; i < subs.length; i++) {
        L_search: {
            for (let cur of current) {
                if (cur.name.localeCompare(subs[i], undefined, { sensitivity: "accent" }) != 0) {
                    continue;
                } else if (i == last) {
                    return cur.type == "file" ? cur.fileLength : -2;
                } else if (cur.type == "directory") {
                    current = io_indexlisting.directories[cur.directoryIndex];
                    break L_search;
                }
            }

            // subdirectory not found
            break;
        }
    }

    return -1;
}

