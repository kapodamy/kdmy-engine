"use strict";

const FS_CHAR_SEPARATOR = '/';
const FS_CHAR_SEPARATOR_REJECT = '\\';
const FS_FILENAME_INVALID_CHARS = `<>:"/\\|?*`;
const FS_TEMP_MAX_LENGTH = 192;
const FS_ASSETS_FOLDER = "/assets";
const FS_EXPANSIONS_FOLDER = "/expansions";
const FS_ASSETS_COMMON_FOLDER = "/assets/common/";
const FS_NO_OVERRIDE_COMMON = "/~assets/common/";

var fs_tls_init = 1;
var fs_tls_key = {};
var fs_cod = null;

function fs_init() {
    if (fs_tls_init) {
        // first run, initialize the thread local storage key
        kthread_key_create(fs_tls_key, fs_destroy);
        fs_tls_init = 0;

    }

    if (kthread_getspecific(fs_tls_key)) throw new Error("Duplicate call to fs_init()");

    let fs_tls = {
        fs_cwd: strdup(FS_ASSETS_FOLDER),
        fs_stk: linkedlist_init()
    };

    kthread_setspecific(fs_tls_key, fs_tls);
}

function fs_destroy(fs_tls) {
    fs_tls.fs_cwd = undefined;
    linkedlist_destroy2(fs_tls.fs_stk, free);
    fs_tls = undefined;
}


async function fs_readtext(src) {
    src = await fs_get_full_path_and_override(src);
    return await io_foreground_request(src, IO_REQUEST_STRING);
}

async function fs_readblob(src) {
    src = await fs_get_full_path_and_override(src);
    return await io_foreground_request(src, IO_REQUEST_BLOB);
}

async function fs_readarraybuffer(src) {
    src = await fs_get_full_path_and_override(src);
    return await io_foreground_request(src, IO_REQUEST_ARRAYBUFFER);
}

async function fs_readimagebitmap(src) {
    src = await fs_get_full_path_and_override(src);
    if (DDS.IsDDS(src)) {
        let arraybuffer = await io_foreground_request(src, IO_REQUEST_ARRAYBUFFER);
        if (!arraybuffer) return null;

        let dds = DDS.Parse(arraybuffer);
        if (!dds) return null;

        return { data: dds, size: dds.size };
    }
    return await io_foreground_request(src, IO_REQUEST_BITMAP);
}

async function fs_readjson__(src) {
    try {
        src = await fs_get_full_path_and_override(src);
        return await io_foreground_request(src, IO_REQUEST_JSON);
    } catch (e) {
        console.error("fs: ", e);
        return null;
    }
}

async function fs_readxml(src) {
    src = await fs_get_full_path_and_override(src);
    let result = await io_foreground_request(src, IO_REQUEST_STRING);
    let xml = new DOMParser().parseFromString(result, "text/xml");

    if (xml.querySelector("parsererror")) {
        let message = "";
        for (let child of xml.querySelector("parsererror").children) {
            let msg = child.textContent.trim();
            if (msg.length > 0) message += child.textContent + "\n";
        }

        throw new SyntaxError("File: " + src + "\n" + message.trim());
    }

    return xml;
}



async function fs_file_exists(src) {
    src = await fs_get_full_path_and_override(src);
    return await io_resource_exists(src, 1, 0);
}

async function fs_folder_exists(src) {
    src = await fs_get_full_path_and_override(src);
    return await io_resource_exists(src, 0, 1);
}

async function fs_file_length(src) {
    src = await fs_get_full_path_and_override(src);
    return await io_file_size(src, null);
}

async function fs_folder_enumerate(src, folder_enumerator) {
    // needs C implemetation
    let text = await fs_readtext(src);
    if (!text) return 0;

    folder_enumerator.___index = 0;
    folder_enumerator.___entries = new Array();
    folder_enumerator.name = null;
    folder_enumerator.is_file = 0;
    folder_enumerator.is_folder = 0;

    if (navigator.userAgent.includes("Gecko/")) {
        let entries = text.split("\n");

        for (let entry of entries) {
            if (!entry.startsWith("201:")) continue;

            let entry_descriptor = entry.trim().split(' ');
            let type = entry_descriptor[entry_descriptor.length - 1];
            let name = decodeURIComponent(entry_descriptor[1]);
            let is_file = type == "FILE";
            let is_folder = type == "DIRECTORY";

            folder_enumerator.___entries.push({ name, is_file, is_folder });
        }

        return 1;
    } else if (navigator.userAgent.includes("Code/") && navigator.userAgent.includes("Electron/")) {
        let html = new DOMParser().parseFromString(text, "text/html");

        if (html.activeElement.tagName == "parsererror")
            throw new SyntaxError(html.activeElement.textContent);

        let entries = html.querySelectorAll("table>tbody>tr");

        for (let entry of entries) {
            if (!entry.querySelector("td")) continue;
            let is_file = entry.querySelector("td:nth-child(2)").textContent.length > 0;
            let is_folder = !is_file;
            let name = entry.querySelector("td:nth-child(1)").textContent;

            if (name == "../") continue;
            name = name.substring(0, name.length - 1);

            folder_enumerator.___entries.push({ name, is_file, is_folder });
        }
        return 1;
    } else if (IO_CHROMIUM_DETECTED) {
        const PREFIX = "addRow(";
        const SUFFIX = ");";

        let doc = (new DOMParser()).parseFromString(text, "text/html");

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
            folder_enumerator.___entries.push({
                name: fake_parsed_json[0],
                is_folder: fake_parsed_json[2],
                is_file: !fake_parsed_json[2]
            });
        }

        return 1;
    }

    throw new KDMYEngineIOError("Unknown web browser, fs_read_folder() can not continue");
}

function fs_folder_enumerate_next(folder_enumerator) {
    if (!folder_enumerator.___entries) return 0;
    if (folder_enumerator.___index >= folder_enumerator.___entries.length) return 0;

    let entry = folder_enumerator.___entries[folder_enumerator.___index++];
    folder_enumerator.name = entry.name;
    folder_enumerator.is_file = entry.is_file;
    folder_enumerator.is_folder = entry.is_folder;
    return 1;
}

function fs_folder_enumerate_close(folder_enumerator) {
    // needs C implemetation
    folder_enumerator.___entries = undefined;
    folder_enumerator.___index = Infinity;
    folder_enumerator.name = null;
    folder_enumerator.is_file = 0;
    folder_enumerator.is_folder = 0;
}

function fs_is_invalid_filename(filename) {
    for (let i = 0; i < FS_FILENAME_INVALID_CHARS.length; i++) {
        for (let j = 0; j < filename.length; j++) {
            if (filename[j].indexOf(FS_FILENAME_INVALID_CHARS[i]) < 0) continue;
            return 1;
        }
    }
    return 0;
}

function fs_get_parent_folder(filename) {
    if (filename == null) throw new Error("filename is null");

    let index = filename.lastIndexOf(FS_CHAR_SEPARATOR);
    if (index < 1) return "";
    return filename.substring(0, index);
}

/** @deprecated */
function fs_combine_path_old(base_path, filename) {
    if (filename == null) throw new Error("filename is null");

    if (!base_path || filename.startsWith(FS_ASSETS_FOLDER)) return strdup(filename);

    let base_index = base_path.length;
    if (base_path.charAt(base_index - 1) == FS_CHAR_SEPARATOR) base_index--;

    let filename_index = 0;
    if (filename.charAt(0) == FS_CHAR_SEPARATOR) filename_index++;

    return base_path.substring(0, base_index) + FS_CHAR_SEPARATOR + filename.substring(filename_index);
}

function fs_build_path(base_path, filename) {
    if (filename == null) throw new Error("filename is null");

    if (!base_path || filename.startsWith(FS_CHAR_SEPARATOR)) return strdup(filename);

    let base_index = base_path.length;
    if (base_path.charAt(base_index - 1) == FS_CHAR_SEPARATOR) base_index--;

    return base_path.substring(0, base_index) + FS_CHAR_SEPARATOR + filename;
}

function fs_build_path2(reference_filename, filename) {
    if (filename == null) throw new Error("filename is null");

    const fs_tls = kthread_getspecific(fs_tls_key);

    if (!reference_filename || filename.startsWith(FS_CHAR_SEPARATOR, 0)) return strdup(filename);

    let reference_parent_index = reference_filename.lastIndexOf(FS_CHAR_SEPARATOR);
    if (reference_parent_index < 0) {
        // the "reference_filename" does not have any parent folder, use the working directory
        return fs_build_path(fs_tls.fs_cwd, filename);
    }

    // build the path using the reference parent folder
    // example: reference=/foobar/atlas.xml filename=texture.png result=/foobar/texture.png
    return reference_filename.substring(0, reference_parent_index + 1) + filename;
}

function fs_get_filename_without_extension(filename) {
    if (filename == null) return null;
    if (filename.length < 1) return "";

    let reference_parent_index = filename.lastIndexOf(FS_CHAR_SEPARATOR);
    let extension_index = filename.lastIndexOf('.');

    if (reference_parent_index < 0) reference_parent_index = 0;
    else reference_parent_index++;

    if (extension_index < reference_parent_index) extension_index = filename.length;

    return filename.substring(reference_parent_index, extension_index);
}

function fs_resolve_path(src) {
    if (src == null) throw new KDMYEngineIOError("fs_resolve_path() failed, the src was null");

    let src_length = src.length;
    if (src.length < 1) return "";

    if (src.indexOf(FS_CHAR_SEPARATOR_REJECT, 0) >= 0) {
        throw new KDMYEngineIOError(`fs_resolve_path() path has invalid separator char: ${src}`);
    }

    if (src_length == 1) {
        if (src.charAt(0) == '.') {
            // path is "." (current directory) or root directory "/"
            return "";
        } else {
            // match "/" and single-character files and/or folders 
            return strdup(src);
        }
    }

    let stack_length = string_occurrences_of_string(src, FS_CHAR_SEPARATOR) + 2;
    let stack = new Array(stack_length);
    let stack_size = 0;
    let index = 0;
    let last_index = 0;
    let size;

    L_parse:
    while (index >= 0 && index < src_length) {
        index = src.indexOf(FS_CHAR_SEPARATOR, last_index);

        if (index < 0) {
            size = src_length - last_index;
        } else {
            size = index - last_index;
            index++;
        }

        switch (size) {
            case 0:
                last_index = index;
                continue;
            case 1:
                if (src.charAt(last_index) == '.') {
                    // current directory token
                    last_index = index;
                    continue;
                }
                break;
            case 2:
                if (src.charAt(last_index) == '.' && src.charAt(last_index + 1) == '.') {
                    // goto parent directory
                    stack_size--;
                    if (stack_size < 0) break L_parse;
                    last_index = index;
                    continue;
                }
                break;
        }

        // store current part
        stack[stack_size] = last_index;
        if (stack_size < stack_length) stack_size++;

        last_index = index;
    }


    if (stack_size < 0) {
        // the path probably points ouside of assets folders
        stack = undefined;
        return strdup(FS_CHAR_SEPARATOR);
    }

    // compute the final path
    let builder = stringbuilder_init(src_length + 2);

    if (src.charAt(0) == FS_CHAR_SEPARATOR) stringbuilder_add(builder, FS_CHAR_SEPARATOR);

    for (let i = 0; i < stack_size; i++) {
        let index = src.indexOf(FS_CHAR_SEPARATOR, stack[i]);
        if (index < 0) index = src_length;

        if (i > 0) stringbuilder_add(builder, FS_CHAR_SEPARATOR);
        stringbuilder_add_substring(builder, src, stack[i], index);
    }

    let path = stringbuilder_get_copy(builder);
    stringbuilder_destroy(builder);

    stack = undefined;
    return path;
}



function fs_set_working_folder(base_path, get_parent_of_base_path) {
    const fs_tls = kthread_getspecific(fs_tls_key);

    if (!base_path) {
        fs_tls.fs_cwd = undefined;
        fs_tls.fs_cwd = strdup(FS_ASSETS_COMMON_FOLDER);
        return;
    }

    let temp_path;
    if (get_parent_of_base_path)
        temp_path = fs_get_parent_folder(base_path);
    else
        temp_path = strdup(base_path);

    // check if the parent directory is the current working directory
    if (temp_path.length < 1) {
        temp_path = undefined;
        return;
    }

    let resolved_path;
    if (temp_path.charAt(0) != FS_CHAR_SEPARATOR)
        resolved_path = fs_build_path(fs_tls.fs_cwd, temp_path);
    else
        resolved_path = fs_build_path(FS_ASSETS_FOLDER, temp_path);

    temp_path = undefined;
    temp_path = fs_resolve_path(resolved_path);
    resolved_path = undefined;

    if (!temp_path.startsWith(FS_ASSETS_FOLDER) && !temp_path.startsWith(FS_EXPANSIONS_FOLDER)) {
        throw new KDMYEngineIOError(`fs_set_working_folder() failed for: ${base_path}`);
    }

    fs_tls.fs_cwd = undefined;
    fs_tls.fs_cwd = temp_path;
}

function fs_set_working_subfolder(sub_path) {
    const fs_tls = kthread_getspecific(fs_tls_key);

    let temp_path = fs_build_path(fs_tls.fs_cwd, sub_path);
    let resolved_path = fs_resolve_path(temp_path);
    temp_path = undefined;

    if (!resolved_path.startsWith(FS_ASSETS_FOLDER) && !resolved_path.startsWith(FS_EXPANSIONS_FOLDER)) {
        throw new KDMYEngineIOError(
            `fs_set_working_subfolder() failed, cwd=${fs_tls.fs_cwd} sub_path=${sub_path}`
        );
    }

    fs_tls.fs_cwd = undefined;
    fs_tls.fs_cwd = resolved_path;
}

function fs_get_full_path(path) {
    if (!path || path.length < 1) return strdup(FS_ASSETS_FOLDER);

    const fs_tls = kthread_getspecific(fs_tls_key);

    let temp_path = fs_build_path(fs_tls.fs_cwd, path);
    let resolved_path = fs_resolve_path(temp_path);
    temp_path = undefined;

    if (resolved_path.length < 1) {
        resolved_path = undefined;
        return strdup(fs_tls.fs_cwd);
    }

    if (resolved_path.charAt(0) != FS_CHAR_SEPARATOR) {
        // expected "/***"
        throw new KDMYEngineIOError(`fs_get_full_path() failed cwd=${fs_tls.fs_cwd} path="${path}`);
    }

    return resolved_path;
}

async function fs_get_full_path_and_override(path) {
    if (!path || path.length < 1) return strdup(FS_ASSETS_FOLDER);

    const fs_tls = kthread_getspecific(fs_tls_key);

    let temp_path = fs_build_path(fs_tls.fs_cwd, path);
    let resolved_path = fs_resolve_path(temp_path);
    temp_path = undefined;

    if (resolved_path.length < 1) {
        resolved_path = undefined;
        return strdup(fs_tls.fs_cwd);
    }

    if (resolved_path.charAt(0) != FS_CHAR_SEPARATOR) {
        // expected "/***"
        throw new KDMYEngineIOError(
            `fs_get_full_path_and_override() failed cwd=${fs_tls.fs_cwd} path="${path}`
        );
    }

    let no_override = resolved_path.startsWith(FS_NO_OVERRIDE_COMMON);

    if (fs_cod && !no_override && resolved_path.startsWith(FS_ASSETS_COMMON_FOLDER, 0)) {
        // override "/assets/common/***" --> "/assets/weeks/abc123/custom_common/***"
        let custom_path = string_concat(
            3,
            fs_cod,
            FS_CHAR_SEPARATOR,
            resolved_path.substring(FS_ASSETS_COMMON_FOLDER.length, resolved_path.length)
        );

        //
        // Note: the resource in origin must match the type in custom
        //       if the file does not exists in origin the behavior is undefined
        //
        let is_file = 1;
        let is_folder = 1;
        if (await io_resource_exists(resolved_path, 1, 0)) is_folder = 0;
        else if (await io_resource_exists(resolved_path, 0, 1)) is_file = 0;

        if (await io_resource_exists(custom_path, is_file, is_folder)) {
            if (DEBUG && is_file && is_folder) {
                console.warn(
                    "fs_get_full_path_and_override() '" +
                    resolved_path +
                    "' does not exist and will be overrided by '" +
                    custom_path +
                    "'"
                );
            }
            resolved_path = undefined;
            return custom_path;
        }

        // the file to override does not exist in the custom common folder
        custom_path = undefined;
    } else if (no_override) {
        // replace "/~assets/common/file.txt" --> "/assets/common/file.txt"
        temp_path = FS_CHAR_SEPARATOR + resolved_path.substring(2, resolved_path.length);
        resolved_path = undefined;
        return temp_path;
    }

    return resolved_path;
}

function fs_override_common_folder(base_path) {
    fs_cod = undefined;

    const fs_tls = kthread_getspecific(fs_tls_key);

    if (!base_path) {
        fs_cod = null;
        return;
    } else if (base_path.startsWith(FS_ASSETS_FOLDER)) {
        let last_index = base_path.length;
        if (base_path.charAt(last_index - 1) == FS_CHAR_SEPARATOR) last_index = last_index - 1;
        fs_cod = base_path.substring(0, last_index);
    } else {
        let temp_path = fs_build_path(fs_tls.fs_cwd, base_path);
        fs_cod = fs_resolve_path(temp_path);
        temp_path = undefined;

        if (fs_cod.startsWith(FS_ASSETS_FOLDER, 0)) return;

        throw new KDMYEngineIOError(
            `fs_override_common_folder() failed base_path=${base_path} cwd=${fs_tls.fs_cwd}`
        );
    }
}

function fs_folder_stack_push() {
    const fs_tls = kthread_getspecific(fs_tls_key);
    linkedlist_add_item(fs_tls.fs_stk, strdup(fs_tls.fs_cwd));
}

function fs_folder_stack_pop() {
    const fs_tls = kthread_getspecific(fs_tls_key);

    let count = linkedlist_count(fs_tls.fs_stk);
    if (count < 1) {
        console.warn("fs: fs_folder_stack_pop() failed, folder stack was empty");
        return;
    }

    fs_tls.fs_cwd = undefined;
    fs_tls.fs_cwd = linkedlist_remove_item_at(fs_tls.fs_stk, count - 1);
}

