"use strict";

const EXPANSIONS_PATH = "/expansions";
const EXPANSIONS_SYMBOLIC_PATH = "/~expansions/";
const EXPANSIONS_FUNKIN_EXPANSION_NAME = "funkin";
const EXPANSIONS_FUNKIN_EXPANSION_PATH = EXPANSIONS_SYMBOLIC_PATH + EXPANSIONS_FUNKIN_EXPANSION_NAME;
const EXPANSIONS_CHAIN_FILENAME = "chain.ini";
const EXPANSIONS_ABOUT_FILENAME = "expansion.json";
const EXPANSIONS_COMMENT_CHAR = ';';
const EXPANSIONS_SELF_NAME = "/self/";

var expansions_chain_array = null;
var expansions_chain_array_size = 0;
var expansions_overrided_weeks_folder = null;


async function expansions_load(expansion_name) {
    if (expansion_name && fs_is_invalid_filename(expansion_name)) {
        console.error(`expansions_load() invalid expansion name, ilegal filename: ${expansion_name}`);
        return;
    }

    //for (let i = 0 ; i < expansions_chain_array_size ; i++) {
    //    if (expansions_chain_array[i] != Symbol(EXPANSIONS_FUNKIN_EXPANSION_PATH)) {
    //        expansions_chain_array[i] = undefined;
    //    }
    //}
    expansions_chain_array = undefined;
    expansions_chain_array_size = 0;

    // PreloadCache is not supported
    // force clear preload cache
    //PreloadCache.ClearCache();

    let chain = arraylist_init();

    if (await io_resource_exists(EXPANSIONS_FUNKIN_EXPANSION_PATH, false, true)) {
        await expansions_internal_load_dependency(chain, EXPANSIONS_FUNKIN_EXPANSION_NAME);
    }

    await expansions_internal_load_dependency(chain, expansion_name);

    if(expansions_overrided_weeks_folder) expansions_overrided_weeks_folder = undefined;
    expansions_overrided_weeks_folder = null;

    let expansion_base_path = string_concat(3, EXPANSIONS_SYMBOLIC_PATH, expansion_name, FS_CHAR_SEPARATOR);
    let about_path = string_concat(2, expansion_base_path, EXPANSIONS_ABOUT_FILENAME);
    let has_about = await io_resource_exists(about_path, true, false);

    if (has_about) {
        let about_json_path = string_concat(2, expansion_base_path, EXPANSIONS_ABOUT_FILENAME);

        about_path = undefined;
        about_path = await io_native_get_path(about_json_path, true, false, false);

        let json = await json_load_direct(about_path);
        let override_weeks_folder = json_read_boolean(json, "overrideWeeksFolder", false);

        // (JS & C#) only
        let window_icon = json_read_string(json, "windowIcon", null);
        let window_title = json_read_string(json, "windowTitle", null);
        await expansions_internal_update_window(expansion_base_path, window_title, window_icon);

        json_destroy(json);
        about_json_path = undefined;

        if (override_weeks_folder) {
            expansions_overrided_weeks_folder = string_concat(2, expansion_base_path, "weeks");
        }
    }

    expansion_base_path = undefined;
    about_path = undefined;

    expansions_chain_array_size = arraylist_trim(chain);
    expansions_chain_array = arraylist_peek_array(chain);
    arraylist_destroy(chain, true);
}

async function expansions_resolve_path(path, is_file, is_folder) {
    if (path == null) return null;
    if (!path.startsWith(FS_ASSETS_FOLDER, 0)) return path;

    let path_length = path.length;
    let index = FS_ASSETS_FOLDER.length;
    if (index < path_length && path[index] == FS_CHAR_SEPARATOR) index++;

    let relative_path = path.substring(index, path_length);
    let last_overrided_path = strdup(path);

    for (let i = 0; i < expansions_chain_array_size; i++) {
        let overrided_path = string_concat(
            3, expansions_chain_array[i], FS_CHAR_SEPARATOR, relative_path
        );

        if (await io_resource_exists(overrided_path, is_file, is_folder)) {
            if (last_overrided_path) last_overrided_path = undefined;
            last_overrided_path = overrided_path;
            continue;
        }

        overrided_path = undefined;
    }

    relative_path = undefined;

    return last_overrided_path;
}

async function expansions_get_path_from_expansion(path, expansion_index) {
    if (!path) return null;
    if (!path.startsWith(FS_ASSETS_FOLDER, 0)) return null;
    if (expansion_index < 0 || expansion_index >= expansions_chain_array_size) return null;

    let path_length = path.length;
    let index = FS_ASSETS_FOLDER.length;
    if (index < path_length && path[index] == FS_CHAR_SEPARATOR) index++;

    let relative_path = path.substring(index, path_length);

    let new_path = string_concat(
        3, expansions_chain_array[expansion_index], FS_CHAR_SEPARATOR, relative_path
    );

    relative_path = undefined;

    return new_path;

    /*if (await io_resource_exists(new_path, true, true)) {
        return new_path;
    }

    new_path = undefined;
    return null;*/
}


async function expansions_internal_load_dependency(chain, expansion_name) {
    if (!expansion_name) return;

    let expansion_path = string_concat(2, EXPANSIONS_SYMBOLIC_PATH, expansion_name);
    if (!await io_resource_exists(expansion_path, false, true)) {
        console.error(`expansions_load() '${expansion_name}' not found in ${expansion_path}`);
        expansion_path = undefined;
        return;
    }

    // PreloadCache is not supported
    //let expansion_preload_ini_path = string_concat(3, expansion_path, FS_CHAR_SEPARATOR, PRELOADCACHE_PRELOAD_FILENAME);
    //if (await io_resource_exists(expansion_preload_ini_path, true, false)) {
    //    PreloadCache.AddFileList(expansion_preload_ini_path);
    //}
    //expansion_preload_ini_path = undefined;

    let chain_ini_path = string_concat(3, expansion_path, FS_CHAR_SEPARATOR, EXPANSIONS_CHAIN_FILENAME);
    let has_chain = await io_resource_exists(chain_ini_path, true, false);

    if (!has_chain) {
        expansions_internal_add_to_chain(chain, expansion_path);
        expansion_path = undefined;
        chain_ini_path = undefined;
        return;
    }

    let unparsed_chain = await io_native_foreground_request(chain_ini_path, IO_REQUEST_TEXT);
    chain_ini_path = undefined;

    if (!unparsed_chain) return;

    let tokenizer = tokenizer_init("\r\n", true, false, unparsed_chain);
    if (!tokenizer) {
        expansions_internal_add_to_chain(chain, expansion_path);
        unparsed_chain = undefined;
        return;
    }

    let name;
    while ((name = tokenizer_read_next(tokenizer)) != null) {
        if (!name || name[0] == EXPANSIONS_COMMENT_CHAR) {
            name = undefined;
            continue;
        }

        if (name == EXPANSIONS_SELF_NAME) {
            expansions_internal_add_to_chain(chain, expansion_path);
        } else if (fs_is_invalid_filename(name)) {
            console.error(`expansions_internal_load_dependency() '${name}' is not a valid folder name`);
        } else {
            // warning: recursive call
            await expansions_internal_load_dependency(chain, name);
        }

        name = undefined;
    }

    expansions_internal_add_to_chain(chain, expansion_path);

    tokenizer_destroy(tokenizer);
    unparsed_chain = undefined;
    tokenizer = undefined;
}

function expansions_internal_add_to_chain(chain, expansion_path) {
    let lowercase_expansion_name = string_to_lowercase(expansion_path);

    for (let expansion of arraylist_iterate4(chain)) {
        if (expansion == expansion_path) return;

        let lowercase_expansion = string_to_lowercase(expansion);
        let equals = lowercase_expansion == lowercase_expansion_name;

        lowercase_expansion = undefined;

        if (equals) {
            lowercase_expansion_name = undefined;
            return;
        }
    }

    lowercase_expansion_name = undefined;
    arraylist_add(chain, expansion_path);
}

async function expansions_internal_update_window(expansion_path, window_title, window_icon) {
    document.title = window_title ?? "";
    if ("main_layout_visor" in window) document.title = "[Layout visor] " + document.title;

    var /**@type {HTMLLinkElement} */ link = document.querySelector("link[rel~='icon']");

    if (!link) {
        if (!window_icon) return;
        link = document.createElement('link');
        link.rel = 'icon';
        document.head.appendChild(link);
    }

    if (!window_icon) {
        link.remove();
        return;
    }

    window_icon = fs_resolve_path(expansion_path + FS_CHAR_SEPARATOR + window_icon);
    link.href = await io_native_get_path(window_icon, true, false, false);
}

