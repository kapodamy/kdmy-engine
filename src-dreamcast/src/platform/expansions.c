#include "expansions.h"
#include "arraylist.h"
#include "fs.h"
#include "io.h"
#include "jsonparser.h"
#include "malloc_utils.h"
#include "preloadcache.h"
#include "stringutils.h"
#include "tokenizer.h"

const char* EXPANSIONS_PATH = "/expansions";
const char* EXPANSIONS_SYMBOLIC_PATH = "/~expansions/";
const char* EXPANSIONS_FUNKIN_EXPANSION_NAME = "funkin";
const char* EXPANSIONS_FUNKIN_EXPANSION_PATH = "/~expansions/funkin";
const char* EXPANSIONS_CHAIN_FILENAME = "chain.ini";
const char* EXPANSIONS_ABOUT_FILENAME = "expansion.json";
const char EXPANSIONS_COMMENT_CHAR = ';';
const char* EXPANSIONS_SELF_NAME = "/self/";

char** expansions_chain_array = NULL;
int32_t expansions_chain_array_size = 0;
char* expansions_overrided_weeks_folder = NULL;


void expansions_internal_load_dependency(ArrayList chain, const char* expansion_name);
void expansions_internal_add_to_chain(ArrayList chain, char* expansion_path);


void expansions_load(const char* expansion_name) {
    if (expansion_name && fs_is_invalid_filename(expansion_name)) {
        logger_error("expansions_load() invalid expansion name, ilegal filename: %s", expansion_name);
        return;
    }

    for (int32_t i = 0; i < expansions_chain_array_size; i++) {
        if (!string_equals(expansions_chain_array[i], EXPANSIONS_FUNKIN_EXPANSION_PATH)) {
            free_chk(expansions_chain_array[i]);
        }
    }
    free_chk(expansions_chain_array);
    expansions_chain_array_size = 0;

    // clear preload cache
    preloadcache_clear(false);

    ArrayList chain = arraylist_init(sizeof(char*));

    if (io_resource_exists(EXPANSIONS_FUNKIN_EXPANSION_PATH, false, true)) {
        expansions_internal_load_dependency(chain, EXPANSIONS_FUNKIN_EXPANSION_NAME);
    }

    expansions_internal_load_dependency(chain, expansion_name);

    if (expansions_overrided_weeks_folder) free_chk(expansions_overrided_weeks_folder);
    expansions_overrided_weeks_folder = NULL;

    char* expansion_base_path = string_concat(3, EXPANSIONS_SYMBOLIC_PATH, expansion_name, FS_STRING_SEPARATOR);
    char* about_path = string_concat(2, expansion_base_path, EXPANSIONS_ABOUT_FILENAME);
    bool has_about = io_resource_exists(about_path, true, false);

    if (has_about) {
        char* about_json_path = string_concat(2, expansion_base_path, EXPANSIONS_ABOUT_FILENAME);

        free_chk(about_path);
        about_path = io_get_native_path(about_json_path, true, false, false);

        JSONToken json = json_load_direct(about_path);
        bool override_weeks_folder = json_read_boolean(json, "overrideWeeksFolder", false);

        json_destroy(&json);
        free_chk(about_json_path);

        if (override_weeks_folder) {
            expansions_overrided_weeks_folder = string_concat(2, expansion_base_path, "weeks");
        }
    }

    free_chk(expansion_base_path);
    free_chk(about_path);

    arraylist_destroy2(&chain, &expansions_chain_array_size, (void**)&expansions_chain_array);
}

char* expansions_resolve_path(const char* path, bool is_file, bool is_folder) {
    if (path == NULL) return NULL;
    if (!string_starts_with(path, FS_ASSETS_FOLDER)) return string_duplicate(path);

    size_t path_length = strlen(path);
    int32_t index = string_get_length(FS_ASSETS_FOLDER);
    if (index < path_length && path[index] == FS_CHAR_SEPARATOR) index++;

    char* relative_path = string_substring(path, index, (int32_t)path_length);
    char* last_overrided_path = string_duplicate(path);

    for (int32_t i = 0; i < expansions_chain_array_size; i++) {
        char* overrided_path = string_concat(
            3, expansions_chain_array[i], FS_STRING_SEPARATOR, relative_path
        );

        if (io_resource_exists(overrided_path, is_file, is_folder)) {
            if (last_overrided_path) free_chk(last_overrided_path);
            last_overrided_path = overrided_path;
            continue;
        }

        free_chk(overrided_path);
    }

    free_chk(relative_path);

    return last_overrided_path;
}

char* expansions_get_path_from_expansion(const char* path, int32_t expansion_index) {
    if (!path) return NULL;
    if (!string_starts_with(path, FS_ASSETS_FOLDER)) return NULL;
    if (expansion_index < 0 || expansion_index >= expansions_chain_array_size) return NULL;

    size_t path_length = strlen(path);
    size_t index = strlen(FS_ASSETS_FOLDER);
    if (index < path_length && path[index] == FS_CHAR_SEPARATOR) index++;

    char* new_path = string_concat_with_substring(
        path, (int32_t)index, (int32_t)path_length, 2,
        2, expansions_chain_array[expansion_index], FS_STRING_SEPARATOR
    );

    return new_path;

    /*if (io_resource_exists(new_path, true, true)) {
        return new_path;
    }

    free_chk(new_path);
    return NULL;*/
}


void expansions_internal_load_dependency(ArrayList chain, const char* expansion_name) {
    if (!expansion_name) return;

    char* expansion_path = string_concat(2, EXPANSIONS_SYMBOLIC_PATH, expansion_name);
    if (!io_resource_exists(expansion_path, false, true)) {
        logger_error("expansions_load() '%s' not found in %s", expansion_name, expansion_path);
        free_chk(expansion_path);
        return;
    }

// not implemented
#if 0
    char* expansion_preload_ini_path = string_concat(3, expansion_path, FS_STRING_SEPARATOR, PRELOADCACHE_PRELOAD_FILENAME);
    if (io_resource_exists(expansion_preload_ini_path, true, false)) {
        preloadcache_add_filelist(expansion_preload_ini_path);
    }
    free_chk(expansion_preload_ini_path);
#endif

    char* chain_ini_path = string_concat(3, expansion_path, FS_STRING_SEPARATOR, EXPANSIONS_CHAIN_FILENAME);
    bool has_chain = io_resource_exists(chain_ini_path, true, false);

    if (!has_chain) {
        expansions_internal_add_to_chain(chain, expansion_path);
        free_chk(expansion_path);
        free_chk(chain_ini_path);
        return;
    }

    ArrayBuffer unparsed_chain = io_read_arraybuffer(chain_ini_path);
    free_chk(chain_ini_path);

    if (!unparsed_chain) return;

    Tokenizer tokenizer = tokenizer_init2("\r\n", true, (const char*)unparsed_chain->data, unparsed_chain->length);
    if (!tokenizer) {
        expansions_internal_add_to_chain(chain, expansion_path);
        free_chk(unparsed_chain);
        return;
    }

    char* name;
    while ((name = tokenizer_read_next(tokenizer)) != NULL) {
        if (!name || name[0] == EXPANSIONS_COMMENT_CHAR) {
            free_chk(name);
            continue;
        }

        if (string_equals(name, EXPANSIONS_SELF_NAME)) {
            expansions_internal_add_to_chain(chain, expansion_path);
        } else if (fs_is_invalid_filename(name)) {
            logger_error("expansions_internal_load_dependency() '%s' is not a valid folder name", name);
        } else {
            // warning: recursive call
            expansions_internal_load_dependency(chain, name);
        }

        free_chk(name);
    }

    expansions_internal_add_to_chain(chain, expansion_path);

    tokenizer_destroy(&tokenizer);
    free_chk(unparsed_chain);
    free_chk(tokenizer);
}

void expansions_internal_add_to_chain(ArrayList chain, char* expansion_path) {
    char* lowercase_expansion_name = string_to_lowercase(expansion_path);

    foreach (char*, expansion, ARRAYLIST_ITERATOR, chain) {
        if (string_equals(expansion, expansion_path)) return;

        char* lowercase_expansion = string_to_lowercase(expansion);
        bool equals = string_equals(lowercase_expansion, lowercase_expansion_name);

        free_chk(lowercase_expansion);

        if (equals) goto L_return;
    }
    arraylist_add(chain, expansion_path);

L_return:
    free_chk(lowercase_expansion_name);
}
