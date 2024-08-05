"use strict";

const FS_CHAR_SEPARATOR = '/';
const FS_CHAR_SEPARATOR_REJECT = '\\';
const FS_FILENAME_INVALID_CHARS = `<>:"/\\|?*`;
const FS_TEMP_MAX_LENGTH = 192;
const FS_ASSETS_FOLDER = "/assets";
const FS_ASSETS_FOLDER_NO_OVERRIDE = "/~assets/";
const FS_EXPANSIONS_FOLDER = "/expansions";
const FS_ASSETS_COMMON_FOLDER = "/assets/common/";
const FS_NO_OVERRIDE_COMMON = "/~assets/common/";

var fs_tls_key = null;
var fs_cod = null;

function fs_init() {
    if (fs_tls_key == null) {
        // first run, initialize the thread local storage key
        kthread_key_create(fs_tls_key = {}, fs_destroy);
    }

    if (kthread_getspecific(fs_tls_key)) {
        throw new Error("Duplicate call to fs_init()");
    }

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
    return await io_request_file(src, IO_REQUEST_TEXT);
}

async function fs_readblob(src) {
    src = await fs_get_full_path_and_override(src);
    return await io_request_file(src, IO_REQUEST_BLOB);
}

async function fs_readarraybuffer(src) {
    src = await fs_get_full_path_and_override(src);
    return await io_request_file(src, IO_REQUEST_ARRAYBUFFER);
}

async function fs_readimagebitmap(src) {
    src = await fs_get_full_path_and_override(src);
    if (DDS.IsDDS(src)) {
        let arraybuffer = await io_request_file(src, IO_REQUEST_ARRAYBUFFER);
        if (!arraybuffer) return null;

        let dds = DDS.Parse(arraybuffer);
        if (!dds) return null;

        return { data: dds, size: dds.size };
    }
    return await io_request_file(src, IO_REQUEST_BITMAP);
}

async function fs_readxml(src) {
    src = await fs_get_full_path_and_override(src);
    let result = await io_request_file(src, IO_REQUEST_TEXT);
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
    return await io_resource_exists(src, true, false);
}

async function fs_folder_exists(src) {
    src = await fs_get_full_path_and_override(src);
    return await io_resource_exists(src, false, true);
}

async function fs_file_length(src) {
    src = await fs_get_full_path_and_override(src);
    return await io_file_size(src);
}

async function fs_folder_enumerate(src, folder_enumerator) {
    // needs C implemetation

    const collator_natural = new Intl.Collator(undefined, { numeric: true, sensitivity: 'base' });
    const collator_insensitive = new Intl.Collator(undefined, { sensitivity: 'accent' });

    async function __enumerate_to(entries, target_folder, resolve_expansion) {
        let path = await io_native_get_path(target_folder, false, true, resolve_expansion);
        if (!await io_native_resource_exists(path, false, true)) return;

        let info = await io_native_enumerate_folder(path);
        if (!info) return;

        for (let i = 0; i < info.length; i++) {
            __add_unique(entries, info[i]);
        }
    }

    function __add_unique(entries, new_entry) {
        for (let entry of entries) {
            if (entry.name === new_entry.name) return;
            if (collator_insensitive.compare(entry.name, new_entry.name) === 0) return;
        }
        entries.push(new_entry);
    }

    try {
        let entries = new Array();

        // this is a disaster, enumerate src under "/expansions" too
        if (!src.startsWith(FS_ASSETS_FOLDER_NO_OVERRIDE)) {
            for (let i = expansions_chain_array_size - 1; i >= 0; i--) {
                let path = await expansions_get_path_from_expansion(src, i);
                if (path == null) continue;

                await __enumerate_to(entries, path, true);
            }
        }

        // now enumerate the resqueted folder
        await __enumerate_to(entries, src, false);

        if (entries.length < 1) {
            return false;
        }

        folder_enumerator.___index = 0;
        folder_enumerator.___entries = entries;
        folder_enumerator.name = null;
        folder_enumerator.is_file = 0;
        folder_enumerator.is_folder = 0;

        // sort filenames if one or more expansions was enumerated
        entries.sort((a, b) => collator_natural.compare(a.name, b.name));

        return true;
    } catch (e) {
        console.warn(`fs_folder_enumerate() failed on ${src}`, e);

        folder_enumerator.___entries = null;
        folder_enumerator.name = null;
        return false;
    }
}

function fs_folder_enumerate_next(folder_enumerator) {
    if (!folder_enumerator.___entries) return false;
    if (folder_enumerator.___index >= folder_enumerator.___entries.length) return false;

    let entry = folder_enumerator.___entries[folder_enumerator.___index++];
    folder_enumerator.name = entry.name;
    folder_enumerator.is_file = entry.is_file;
    folder_enumerator.is_folder = entry.is_folder;
    return true;
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
            if (filename[j] == FS_FILENAME_INVALID_CHARS[i]) return true;
        }
    }
    return false;
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

    if (!base_path || filename[0] == FS_CHAR_SEPARATOR) return strdup(filename);

    let base_index = base_path.length;
    if (base_path.charAt(base_index - 1) == FS_CHAR_SEPARATOR) base_index--;

    return base_path.substring(0, base_index) + FS_CHAR_SEPARATOR + filename;
}

function fs_build_path2(reference_filename, filename) {
    if (filename == null) throw new Error("filename is null");

    const fs_tls = kthread_getspecific(fs_tls_key);

    if (!reference_filename || filename[0] == FS_CHAR_SEPARATOR) return strdup(filename);

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
    if (src_length < 1) return "";

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

    let path = stringbuilder_finalize(builder);

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
        let is_file = true;
        let is_folder = true;
        if (await io_resource_exists(resolved_path, true, false)) is_folder = false;
        else if (await io_resource_exists(resolved_path, false, true)) is_file = false;

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
        console.warn("fs_folder_stack_pop() failed, folder stack was empty");
        return;
    }

    fs_tls.fs_cwd = undefined;
    fs_tls.fs_cwd = linkedlist_remove_item_at(fs_tls.fs_stk, count - 1);
}

