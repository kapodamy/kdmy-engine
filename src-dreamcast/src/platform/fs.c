#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <kos/tls.h>

#include "arraylist.h"
#include "expansions.h"
#include "fs.h"
#include "io.h"
#include "linkedlist.h"
#include "malloc_utils.h"
#include "stringbuilder.h"
#include "stringutils.h"


typedef struct {
    char* fs_cwd;
    LinkedList fs_stk;
} FSTLS;


const char FS_CHAR_SEPARATOR = '/';
const char FS_CHAR_SEPARATOR_REJECT = '\\';
const char* FS_FILENAME_INVALID_CHARS = "<>:\"/\\|?*";
const size_t FS_TEMP_MAX_LENGTH = 192;
const char* FS_ASSETS_FOLDER = "/assets";
const char* FS_ASSETS_FOLDER_NO_OVERRIDE = "/~assets/";
const char* FS_EXPANSIONS_FOLDER = "/expansions";
const char* FS_ASSETS_COMMON_FOLDER = "/assets/common/";
const char* FS_NO_OVERRIDE_COMMON = "/~assets/common/";

static kthread_key_t fs_tls_key = 0; // this value is never used by KallistiOS
static char* fs_cod = NULL;


static void fs_destroy(FSTLS* fs_tls);
static bool fs_internal_enumerate_to(ArrayList entries, const char* target_folder, bool resolve_expansion);
static int fs_internal_folder_sort(const void* a, const void* b);


void fs_init_kdy() {
    if (fs_tls_key == 0) {
        // first run, initialize the thread local storage key
        kthread_key_create(&fs_tls_key, (void (*)(void*))fs_destroy);
    }

    if (kthread_getspecific(fs_tls_key)) {
        logger_error("Duplicate call to %s()", __func__);
        assert(!kthread_getspecific(fs_tls_key));
    }

    FSTLS* fs_tls = malloc_chk(sizeof(FSTLS));
    malloc_assert(fs_tls, FSTLS);

    *fs_tls = (FSTLS){
        .fs_cwd = string_duplicate(FS_ASSETS_FOLDER),
        .fs_stk = linkedlist_init()
    };

    kthread_setspecific(fs_tls_key, fs_tls);
}

static void fs_destroy(FSTLS* fs_tls) {
    free_chk(fs_tls->fs_cwd);
    linkedlist_destroy2(&fs_tls->fs_stk, free);
    free_chk(fs_tls);
}


char* fs_readtext(const char* src) {
    ArrayBuffer ret = fs_readarraybuffer(src);

    if (ret)
        return arraybuffer_destroy2(&ret, true);
    else
        return NULL;
}

ArrayBuffer fs_readarraybuffer(const char* src) {
    char* path = fs_get_full_path_and_override(src);
    ArrayBuffer ret = io_read_arraybuffer(path);

    free_chk(path);
    return ret;
}


bool fs_file_exists(const char* src) {
    char* path = fs_get_full_path_and_override(src);
    bool ret = io_resource_exists(path, true, false);

    free_chk(path);
    return ret;
}

bool fs_folder_exists(const char* src) {
    char* path = fs_get_full_path_and_override(src);
    bool ret = io_resource_exists(path, false, true);

    free_chk(path);
    return ret;
}

int64_t fs_file_length(const char* src) {
    char* path = fs_get_full_path_and_override(src);
    int64_t ret = io_file_size(path);

    free_chk(path);
    return ret;
}

FSFolderEnumerator fs_folder_enumerate(const char* src) {
    FSFolderEnumerator folder_enumerator = {
        .__run = false,
        .___index = 0,
        .___length = 0,
        .___entries = NULL
    };

    ArrayList entries = arraylist_init(sizeof(FSFolderEnumeratorEntry));
    malloc_assert(entries, FSFolderEnumeratorEntry);

    // this is a disaster, enumerate src under "/expansions" too
    if (!string_starts_with(src, FS_ASSETS_FOLDER_NO_OVERRIDE)) {
        for (int8_t i = expansions_chain_array_size - 1; i >= 0; i--) {
            char* path = expansions_get_path_from_expansion(src, i);
            if (path == NULL) continue;

            if (fs_internal_enumerate_to(entries, path, true)) {
                goto L_failed;
            }
            free_chk(path);
        }
    }

    // now enumerate the resqueted folder
    if (fs_internal_enumerate_to(entries, src, false)) {
        goto L_failed;
    }

    if (arraylist_size(entries) < 1) {
        arraylist_destroy(&entries);
        folder_enumerator.__run = false;
        return folder_enumerator;
    }

    folder_enumerator.___index = 0;
    arraylist_destroy2(&entries, &folder_enumerator.___length, (void**)&folder_enumerator.___entries);

    // sort filenames if one or more expansions was enumerated
    qsort(
        folder_enumerator.___entries,
        (size_t)folder_enumerator.___length, sizeof(FSFolderEnumeratorEntry),
        fs_internal_folder_sort
    );

    folder_enumerator.__run = true;
    return folder_enumerator;

L_failed:
    logger_warn("fs_folder_enumerate() failed on: %s", src);

    free_chk(folder_enumerator.___entries);
    folder_enumerator.__run = false;

    return folder_enumerator;
}

bool fs_folder_enumerate_next(FSFolderEnumerator* folder_enumerator, FSFolderEnumeratorEntry** folder_entry) {
    if (!folder_enumerator->___entries) return false;
    if (folder_enumerator->___index >= folder_enumerator->___length) return false;

    *folder_entry = folder_enumerator->___entries + folder_enumerator->___index;

    folder_enumerator->___index++;
    return true;
}

bool fs_folder_enumerate_close(FSFolderEnumerator* folder_enumerator) {
    FSFolderEnumeratorEntry* entry = folder_enumerator->___entries;
    for (int32_t i = 0; i < folder_enumerator->___length; i++) {
        free_chk((char*)entry->name);
        entry++;
    }

    free_chk(folder_enumerator->___entries);
    return false;
}

bool fs_is_invalid_filename(const char* filename) {
    for (size_t i = 0; i < FS_FILENAME_INVALID_CHARS[i]; i++) {
        for (size_t j = 0; j < filename[j]; j++) {
            if (filename[j] == FS_FILENAME_INVALID_CHARS[i]) return true;
        }
    }
    return false;
}

char* fs_get_parent_folder(const char* filename) {
    assert(filename);

    int32_t index = string_last_index_of_char(filename, FS_CHAR_SEPARATOR);
    if (index < 1) return string_duplicate("");
    return string_substring(filename, 0, index);
}

char* fs_build_path(const char* base_path, const char* filename) {
    assert(filename);

    if (!base_path || filename[0] == FS_CHAR_SEPARATOR) return string_duplicate(filename);

    size_t base_index = strlen(base_path);
    if (base_path[base_index - 1] == FS_CHAR_SEPARATOR) base_index--;

    return string_concat_with_substring(base_path, 0, (int32_t)base_index, 0, 2, FS_STRING_SEPARATOR, filename);
}

char* fs_build_path2(const char* reference_filename, const char* filename) {
    assert(filename);

    FSTLS* fs_tls = (FSTLS*)kthread_getspecific(fs_tls_key);

    if (!reference_filename || filename[0] == FS_CHAR_SEPARATOR) return string_duplicate(filename);

    int32_t reference_parent_index = string_last_index_of_char(reference_filename, FS_CHAR_SEPARATOR);
    if (reference_parent_index < 0) {
        // the "reference_filename" does not have any parent folder, use the working directory
        return fs_build_path(fs_tls->fs_cwd, filename);
    }

    // build the path using the reference parent folder
    // example: reference=/foobar/atlas.xml filename=texture.png result=/foobar/texture.png
    return string_concat_with_substring(reference_filename, 0, reference_parent_index + 1, 0, 1, filename);
}

char* fs_get_filename_without_extension(const char* filename) {
    if (filename == NULL) return NULL;

    size_t filename_length = strlen(filename);
    if (filename_length < 1) return string_duplicate("");

    int32_t reference_parent_index = string_last_index_of_char(filename, FS_CHAR_SEPARATOR);
    int32_t extension_index = string_last_index_of_char(filename, '.');

    if (reference_parent_index < 0)
        reference_parent_index = 0;
    else
        reference_parent_index++;

    if (extension_index < reference_parent_index) extension_index = (int32_t)filename_length;

    return string_substring(filename, reference_parent_index, extension_index);
}

char* fs_resolve_path(const char* src) {
    assert(src);

    int32_t src_length = (int32_t)strlen(src);
    if (src_length < 1) return string_duplicate("");

    if (string_index_of_char(src, 0, FS_CHAR_SEPARATOR_REJECT) >= 0) {
        logger_error("fs_resolve_path() path has invalid separator char: %s", src);
        assert(!src);
    }

    if (src_length == 1) {
        if (src[0] == '.') {
            // path is "." (current directory) or root directory "/"
            return string_duplicate("");
        } else {
            // match "/" and single-character files and/or folders
            return string_duplicate(src);
        }
    }

    int32_t stack_length = string_occurrences_of_char(src, FS_CHAR_SEPARATOR) + 2;
    int32_t stack[stack_length];
    int32_t stack_size = 0;
    int32_t index = 0;
    int32_t last_index = 0;
    int32_t size;

    while (index >= 0 && index < src_length) {
        index = string_index_of_char(src, last_index, FS_CHAR_SEPARATOR);

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
                if (src[last_index] == '.') {
                    // current directory token
                    last_index = index;
                    continue;
                }
                break;
            case 2:
                if (src[last_index] == '.' && src[last_index + 1] == '.') {
                    // goto parent directory
                    stack_size--;
                    if (stack_size < 0) goto L_check_stack;
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

L_check_stack:
    if (stack_size < 0) {
        // the path probably points ouside of assets folders
        return string_duplicate(FS_STRING_SEPARATOR);
    }

    // compute the final path
    StringBuilder builder = stringbuilder_init((size_t)src_length + 2);

    if (src[0] == FS_CHAR_SEPARATOR) stringbuilder_add_char(builder, FS_CHAR_SEPARATOR);

    for (size_t i = 0; i < stack_size; i++) {
        int32_t index = string_index_of_char(src, stack[i], FS_CHAR_SEPARATOR);
        if (index < 0) index = src_length;

        if (i > 0) stringbuilder_add_char(builder, FS_CHAR_SEPARATOR);
        stringbuilder_add_substring(builder, src, stack[i], index);
    }

    char* path = stringbuilder_finalize(&builder);

    return path;
}



void fs_set_working_folder(const char* base_path, bool get_parent_of_base_path) {
    FSTLS* fs_tls = (FSTLS*)kthread_getspecific(fs_tls_key);

    if (!base_path) {
        free_chk(fs_tls->fs_cwd);
        fs_tls->fs_cwd = string_duplicate(FS_ASSETS_COMMON_FOLDER);
        return;
    }

    char* temp_path;
    if (get_parent_of_base_path)
        temp_path = fs_get_parent_folder(base_path);
    else
        temp_path = string_duplicate(base_path);

    // check if the parent directory is the current working directory
    if (strlen(temp_path) < 1) {
        free_chk(temp_path);
        return;
    }

    char* resolved_path;
    if (temp_path[0] != FS_CHAR_SEPARATOR)
        resolved_path = fs_build_path(fs_tls->fs_cwd, temp_path);
    else
        resolved_path = fs_build_path(FS_ASSETS_FOLDER, temp_path);

    free_chk(temp_path);
    temp_path = fs_resolve_path(resolved_path);
    free_chk(resolved_path);

    if (!string_starts_with(temp_path, FS_ASSETS_FOLDER) && !string_starts_with(temp_path, FS_EXPANSIONS_FOLDER)) {
        logger_error("fs_set_working_folder() failed for: %s", base_path);
        assert(!base_path);
    }

    free_chk(fs_tls->fs_cwd);
    fs_tls->fs_cwd = temp_path;
}

void fs_set_working_subfolder(const char* sub_path) {
    FSTLS* fs_tls = (FSTLS*)kthread_getspecific(fs_tls_key);

    char* temp_path = fs_build_path(fs_tls->fs_cwd, sub_path);
    char* resolved_path = fs_resolve_path(temp_path);
    free_chk(temp_path);

    if (!string_starts_with(resolved_path, FS_ASSETS_FOLDER) && !string_starts_with(resolved_path, FS_EXPANSIONS_FOLDER)) {
        logger_error("fs_set_working_subfolder() failed, cwd=%s sub_path=%s", fs_tls->fs_cwd, sub_path);
    }

    free_chk(fs_tls->fs_cwd);
    fs_tls->fs_cwd = resolved_path;
}

char* fs_get_full_path(const char* path) {
    if (!path || path[0] == '\0') return string_duplicate(FS_ASSETS_FOLDER);

    FSTLS* fs_tls = (FSTLS*)kthread_getspecific(fs_tls_key);

    char* temp_path = fs_build_path(fs_tls->fs_cwd, path);
    char* resolved_path = fs_resolve_path(temp_path);
    free_chk(temp_path);

    if (resolved_path[0] == '\0') {
        free_chk(resolved_path);
        return string_duplicate(fs_tls->fs_cwd);
    }

    if (resolved_path[0] != FS_CHAR_SEPARATOR) {
        // expected "/***"
        logger_error("fs_get_full_path() failed cwd=%s path=%s", fs_tls->fs_cwd, path);
        assert(!resolved_path);
    }

    return resolved_path;
}

char* fs_get_full_path_and_override(const char* path) {
    if (!path || path[0] == '\0') return string_duplicate(FS_ASSETS_FOLDER);

    FSTLS* fs_tls = (FSTLS*)kthread_getspecific(fs_tls_key);

    char* temp_path = fs_build_path(fs_tls->fs_cwd, path);
    char* resolved_path = fs_resolve_path(temp_path);
    free_chk(temp_path);

    if (resolved_path[0] == '\0') {
        free_chk(resolved_path);
        return string_duplicate(fs_tls->fs_cwd);
    }

    if (resolved_path[0] != FS_CHAR_SEPARATOR) {
        // expected "/***"
        logger_error("fs_get_full_path_and_override() failed cwd=%s path=%s", fs_tls->fs_cwd, path);
    }

    bool no_override = string_starts_with(resolved_path, FS_NO_OVERRIDE_COMMON);

    if (fs_cod && !no_override && string_starts_with(resolved_path, FS_ASSETS_COMMON_FOLDER)) {
        // override "/assets/common/***" --> "/assets/weeks/abc123/custom_common/***"
        char* custom_path = string_concat_with_substring(
            resolved_path, (int32_t)strlen(FS_ASSETS_COMMON_FOLDER), (int32_t)strlen(resolved_path),
            2,
            2,
            fs_cod,
            FS_STRING_SEPARATOR
        );

        //
        // Note: the resource in origin must match the type in custom
        //       if the file does not exists in origin the behavior is undefined
        //
        bool is_file = true;
        bool is_folder = true;
        if (io_resource_exists(resolved_path, true, false))
            is_folder = false;
        else if (io_resource_exists(resolved_path, false, true))
            is_file = false;

        if (io_resource_exists(custom_path, is_file, is_folder)) {
#ifdef DEBUG
            if (is_file && is_folder) {
                logger_warn(
                    "fs_get_full_path_and_override() '%s' does not exist and will be overrided by '%s'",
                    resolved_path, custom_path
                );
            }
#endif
            free_chk(resolved_path);
            return custom_path;
        }

        // the file to override does not exist in the custom common folder
        free_chk(custom_path);
    } else if (no_override) {
        // replace "/~assets/common/file.txt" --> "/assets/common/file.txt"
        temp_path = string_concat_with_substring(resolved_path, 2, (int32_t)strlen(resolved_path), 1, 1, FS_STRING_SEPARATOR);
        free_chk(resolved_path);
        return temp_path;
    }

    return resolved_path;
}

void fs_override_common_folder(const char* base_path) {
    free_chk(fs_cod);

    FSTLS* fs_tls = (FSTLS*)kthread_getspecific(fs_tls_key);

    if (!base_path) {
        fs_cod = NULL;
        return;
    } else if (string_starts_with(base_path, FS_ASSETS_FOLDER)) {
        int32_t last_index = (int32_t)strlen(base_path);
        if (base_path[last_index - 1] == FS_CHAR_SEPARATOR) last_index = last_index - 1;
        fs_cod = string_substring(base_path, 0, last_index);
    } else {
        char* temp_path = fs_build_path(fs_tls->fs_cwd, base_path);
        fs_cod = fs_resolve_path(temp_path);
        free_chk(temp_path);

        if (string_starts_with(fs_cod, FS_ASSETS_FOLDER)) return;

        logger_error(
            "fs_override_common_folder() failed base_path=%s cwd=%s", base_path, fs_tls->fs_cwd
        );
    }
}

void fs_folder_stack_push() {
    FSTLS* fs_tls = (FSTLS*)kthread_getspecific(fs_tls_key);
    linkedlist_add_item(fs_tls->fs_stk, string_duplicate(fs_tls->fs_cwd));
}

void fs_folder_stack_pop() {
    FSTLS* fs_tls = (FSTLS*)kthread_getspecific(fs_tls_key);

    int32_t count = linkedlist_count(fs_tls->fs_stk);
    if (count < 1) {
        logger_warn("fs_folder_stack_pop() failed, folder stack was empty");
        return;
    }

    free_chk(fs_tls->fs_cwd);
    fs_tls->fs_cwd = linkedlist_remove_item_at(fs_tls->fs_stk, count - 1);
}


static void fs_internal_add_unique(ArrayList entries, const char* name, int length) {
    foreach (FSFolderEnumeratorEntry*, entry, ARRAYLIST_ITERATOR, entries) {
        if (string_equals_ignore_case(entry->name, name)) {
            return;
        }
    }

    const FSFolderEnumeratorEntry entry = {
        .name = string_duplicate(name),
        .length = length
    };

    arraylist_add(entries, (void*)&entry);
}

static bool fs_internal_enumerate_to(ArrayList entries, const char* target_folder, bool resolve_expansion) {
    char* path = io_get_native_path(target_folder, false, true, resolve_expansion);

    IOFolderHandle dir;
    bool ret = io_open_folder(path, &dir);
    free_chk(path);

    if (!ret) {
        logger_error("fs_internal_enumerate_to() failed on: %s", target_folder);
        return true;
    }

    const char* name;
    int size;

    while (io_read_folder(dir, &name, &size)) {
        fs_internal_add_unique(entries, name, size);
    }

    io_close_folder(&dir);
    return false;
}

static int fs_internal_folder_sort(const void* a, const void* b) {
    const FSFolderEnumeratorEntry* a_ptr = (const FSFolderEnumeratorEntry*)a;
    const FSFolderEnumeratorEntry* b_ptr = (const FSFolderEnumeratorEntry*)b;

    return string_natural_comparer(a_ptr->name, b_ptr->name);
}
