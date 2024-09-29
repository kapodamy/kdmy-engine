#include "io.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include <kos/fs.h>
#include <kos/mutex.h>

#include "expansions.h"
#include "fs.h"
#include "logger.h"
#include "malloc_utils.h"
#include "preloadcache.h"
#include "stringutils.h"


static const char* ENGINE_DIRECTORY = "/cd/";          // GD-ROM root directory
static const char KALLISTIOS_DIR_CHAR_SEPARATOR = '/'; // asummed to be always '/'

static mutex_t io_mutex = MUTEX_INITIALIZER;


bool io_open_folder(const char* absolute_path, IOFolderHandle* out_hnd) {
    _Static_assert(sizeof(file_t) <= sizeof(IOFolderHandle));

    file_t hnd = fs_open(absolute_path, O_RDONLY | O_DIR);

    if (hnd == FILEHND_INVALID) {
        logger_error("io_open_folder() failed to open: %s", absolute_path);
    }

    *out_hnd = (IOFolderHandle)hnd;
    return hnd != FILEHND_INVALID;
}

bool io_read_folder(IOFolderHandle fldrhnd, const char** name, int* size) {
    file_t hnd = (file_t)fldrhnd;
    if (hnd == FILEHND_INVALID) return false;

L_read_dir:
    dirent_t* dirent = fs_readdir(hnd);

    if (dirent) {
        if (string_equals(dirent->name, ".") || string_equals(dirent->name, "..")) {
            // i do not see benefits of returning "." and ".."
            goto L_read_dir;
        }

        *name = dirent->name;
        *size = (dirent->attr & O_DIR) ? -1 : dirent->size;
        return true;
    }

    return false;
}

void io_close_folder(IOFolderHandle* fldrhnd) {
    file_t hnd = (file_t)*fldrhnd;
    if (hnd == FILEHND_INVALID) return;

    fs_close(hnd);

    *fldrhnd = (IOFolderHandle)FILEHND_INVALID;
}


ArrayBuffer io_read_arraybuffer(const char* absolute_path) {
    char* native_path = io_get_native_path(absolute_path, true, false, true);
    ArrayBuffer arraybuffer = preloadcache_adquire(native_path);

    if (arraybuffer) {
        free_chk(native_path);
        return arraybuffer;
    }

    mutex_lock(&io_mutex);
    file_t hnd = fs_open(native_path, O_RDONLY);

    if (hnd != FILEHND_INVALID) {
        size_t file_size = fs_total(hnd);

        if (file_size == 0) {
            arraybuffer = arraybuffer_init(0);
            if (arraybuffer)
                goto L_return;
            else
                goto L_check_data; // this never should happen
        } else if (file_size == (size_t)-1) {
            goto L_check_data;
        }

        arraybuffer = arraybuffer_init(file_size);
        if (!arraybuffer) {
            goto L_check_data;
        }

        uint8_t* ptr = (uint8_t*)arraybuffer->data;
        ssize_t readed;

        while (file_size > 0) {
            readed = fs_read(hnd, ptr, file_size);
            if (readed < 1) break;

            ptr += (size_t)readed;
            file_size -= (size_t)readed;
        }

        if (readed < 0) {
            arraybuffer_destroy(&arraybuffer);
            arraybuffer = NULL;
        }
    }

L_check_data:
    if (!arraybuffer) {
        logger_error(
            "io_read_arraybuffer() failed to read absolute_path=%s native_path=%s",
            absolute_path, native_path
        );
    }

L_return:
    if (hnd != FILEHND_INVALID) fs_close(hnd);
    mutex_unlock(&io_mutex);

    if (arraybuffer && !preloadcache_append(native_path, true, false, arraybuffer)) {
        // not added to the preload cache
        free_chk(native_path);
    }

    return arraybuffer;
}

bool io_resource_exists(const char* absolute_path, bool expect_file, bool expect_folder) {
    char* native_path = io_get_native_path(absolute_path, expect_file, expect_folder, true);

    bool ret;

    if (expect_file && preloadcache_stat(native_path) >= 0) {
        ret = true;
        goto L_return;
    }

    mutex_lock(&io_mutex);
    file_t hnd;

    if (expect_file)
        hnd = fs_open(native_path, O_RDONLY);
    else if (expect_folder)
        hnd = fs_open(native_path, O_DIR);
    else
        hnd = FILEHND_INVALID;

    ret = hnd != FILEHND_INVALID;
    if (ret) fs_close(hnd);
    mutex_unlock(&io_mutex);

L_return:
    free_chk(native_path);
    return ret;
}

int64_t io_file_size(const char* absolute_path) {
    char* native_path = io_get_native_path(absolute_path, true, false, true);

    int64_t ret = preloadcache_stat(native_path);
    if (ret >= 0) goto L_return;

    mutex_lock(&io_mutex);
    file_t hnd = fs_open(native_path, O_RDONLY);

    if (hnd != FILEHND_INVALID) {
        size_t size = fs_total(hnd);
        fs_close(hnd);

        if (size == (size_t)-1)
            ret = -1;
        else
            ret = (int64_t)size;
    }
    mutex_unlock(&io_mutex);

L_return:
    free_chk(native_path);
    return ret;
}

char* io_get_native_path(const char* absolute_path, bool is_file, bool is_folder, bool resolve_expansion) {
    // assert(is_file != is_folder);

    const char* base_path = ENGINE_DIRECTORY;
    char* path = (char*)absolute_path;
    int index = 0;

    if (resolve_expansion && string_starts_with(absolute_path, "/assets")) {
        path = expansions_resolve_path(path, is_file, is_folder);
    }

    if (string_starts_with(path, "/~assets") || string_starts_with(path, "/~expansions")) {
        index = 2;
    } else {
        switch (path[0]) {
            case '/':
            case '\\':
                index = 1;
                break;
        }

        if (!string_starts_with(path + index, "assets") && !string_starts_with(path + index, "expansions")) {
            logger_warn("io_get_native_path() path outside of 'assets' or 'expansions' folder: %s", path);
            base_path = "assets/";
        }
    }

    char* new_path;
    if (is_file)
        new_path = string_concat(3, base_path, path + index, IO_4Z);
    else
        new_path = string_concat(2, base_path, path + index);

    if (/*pointer equals*/ path != absolute_path) free_chk(path);

    for (char* ptr = new_path; *ptr != '\0'; ptr++) {
        if (*ptr == FS_CHAR_SEPARATOR) {
            *ptr = KALLISTIOS_DIR_CHAR_SEPARATOR;
        }
    }

    if (is_file) {
        io_override_file_extension(new_path, is_folder);
    }

    return new_path;
}

void io_override_file_extension(char* native_path_4Z, bool check_if_folder) {
    // replace extension if necessary
    size_t new_path_length = strlen(native_path_4Z);
    native_path_4Z[new_path_length - 1] = '\0';
    native_path_4Z[new_path_length - 2] = '\0';
    native_path_4Z[new_path_length - 3] = '\0';
    native_path_4Z[new_path_length - 4] = '\0';

    if (check_if_folder) {
        // check if is a folder before continue
        file_t hnd = fs_open(native_path_4Z, O_RDONLY | O_DIR);
        bool exists = hnd != FILEHND_INVALID;
        fs_close(hnd);

        if (exists) {
            return;
        }
    }

    char* ext = NULL;
    for (char* ptr = native_path_4Z + new_path_length - 4; ptr > native_path_4Z; ptr--) {
        if (*ptr == FS_CHAR_SEPARATOR) break;
        if (*ptr == '.') {
            ext = ptr + 1;
            break;
        }
    }

    if (!ext) {
        return;
    }

    const char* new_ext = NULL;
    if (string_equals_ignore_case(ext, "json")) {
        new_ext = "jbno";
    } else if (string_equals_ignore_case(ext, "xml")) {
        new_ext = "bno";
    } else if (
        string_equals_ignore_case(ext, "jpeg") ||
        string_equals_ignore_case(ext, "jpg") ||
        string_equals_ignore_case(ext, "png") ||
        string_equals_ignore_case(ext, "ico") ||
        string_equals_ignore_case(ext, "bmp") ||
        string_equals_ignore_case(ext, "webp") ||
        string_equals_ignore_case(ext, "dds")
    ) {
        new_ext = "kdt";
    } else if (
        string_equals_ignore_case(ext, "mp4") ||
        string_equals_ignore_case(ext, "m4v") ||
        string_equals_ignore_case(ext, "3gp") ||
        string_equals_ignore_case(ext, "webm") ||
        string_equals_ignore_case(ext, "mkv")
    ) {
        new_ext = "kdm";
    } else if (string_equals_ignore_case(ext, "ogg")) {
        new_ext = "wav";
        // check if the ogg file was replaced as sfx in wav format
        char* test_path = string_concat_with_substring(native_path_4Z, 0, (int32_t)(ext - native_path_4Z), 0, 1, new_ext);

        if (preloadcache_stat(test_path) < 0) {
            mutex_lock(&io_mutex);
            file_t hnd = fs_open(test_path, O_RDONLY);
            bool ret = hnd != FILEHND_INVALID;

            if (ret)
                fs_close(hnd);
            else
                new_ext = NULL;
            mutex_unlock(&io_mutex);
        }

        free_chk(test_path);
    }

    if (!new_ext) {
        return;
    }

    size_t new_ext_length = strlen(new_ext);
    memcpy(ext, new_ext, new_ext_length);
    ext[new_ext_length] = '\0';
}


ArrayBuffer arraybuffer_init(size_t data_size) {
    size_t length = sizeof(struct ArrayBuffer_s) + data_size;
    ArrayBuffer arraybuffer = malloc_chk(length);

    if (!arraybuffer) {
        // try again by flushing unused entries in the preload cache
        bool success = preloadcache_flush(length);
        if (success) {
            arraybuffer = malloc_chk(length);
            success = arraybuffer != NULL;
        }

        if (!success) {
            // try again by clearing the preload cache
            preloadcache_clear(true);
            arraybuffer = malloc_chk(length);
        }
    }

    if (arraybuffer) {
        arraybuffer->length = data_size;
        arraybuffer->references = 1;
    } else {
        logger_warn("arraybuffer_init() out-of-memory, can not allocate " FMT_ZU " bytes", data_size);
    }

    return arraybuffer;
}

ArrayBuffer arraybuffer_init2(const char* native_file_path, size_t max_file_size) {
    errno = 0;

    file_t hnd = fs_open(native_file_path, O_RDONLY);
    if (!hnd) {
        return NULL;
    }

    size_t file_size = fs_total(hnd);
    if (file_size == (size_t)-1) {
        fs_close(hnd);
        return NULL;
    }

    if (file_size >= max_file_size) {
        errno = EFBIG;
        fs_close(hnd);
        return NULL;
    }

    ArrayBuffer arraybuffer = arraybuffer_init(file_size + 1);
    if (!arraybuffer) {
        fs_close(hnd);
        return NULL;
    }

    uint8_t* ptr = (uint8_t*)arraybuffer->data;
    ssize_t readed = -1;

    while (file_size > 0) {
        readed = fs_read(hnd, ptr, file_size);
        if (readed < 1) break;

        ptr += (size_t)readed;
        file_size -= (size_t)readed;
    }

    fs_close(hnd);

    if (readed < 0 || (readed == 0 && file_size > 0)) {
        arraybuffer_destroy(&arraybuffer);
        return NULL;
    }

    // if reading a text file, add a NULL terminator character
    *((char*)ptr) = '\0';
    arraybuffer->length--;

    return arraybuffer;
}

void arraybuffer_destroy(ArrayBuffer* arraybuffer_ptr) {
    if (!arraybuffer_ptr || !*arraybuffer_ptr) return;

    ArrayBuffer arraybuffer = *arraybuffer_ptr;

    arraybuffer->references--;
    if (arraybuffer->references > 0) {
        *arraybuffer_ptr = NULL;
        return;
    }

    arraybuffer_force_destroy(arraybuffer_ptr);
}

void* arraybuffer_destroy2(ArrayBuffer* arraybuffer_ptr, bool add_null_terminator) {
    if (!arraybuffer_ptr || !*arraybuffer_ptr) return NULL;

    ArrayBuffer arraybuffer = *arraybuffer_ptr;

    uint8_t* data;
    size_t data_size = arraybuffer->length;

    arraybuffer->references--;

    if (arraybuffer->references > 0) {
        // still in use, return a copy
        data = malloc_chk(data_size + (add_null_terminator ? sizeof(char) : 0));
        malloc_assert(data, uint8_t[]);
        memcpy(data, arraybuffer->data, data_size);
    } else {
        // not stored in preload cache, reuse allocated space
        _Static_assert(sizeof(struct ArrayBuffer_s) >= sizeof(char));
        data = (uint8_t*)(void*)arraybuffer;
        memmove(data, arraybuffer->data, data_size);
    }

    if (add_null_terminator) {
        char* ptr = (char*)&data[data_size];
        *ptr = '\0';
    }

    *arraybuffer_ptr = NULL;
    return data;
}

void arraybuffer_force_destroy(ArrayBuffer* arraybuffer_ptr) {
    if (!arraybuffer_ptr || !*arraybuffer_ptr) return;

    ArrayBuffer arraybuffer = *arraybuffer_ptr;
    arraybuffer->references = -1;

    free_chk(arraybuffer);
    *arraybuffer_ptr = NULL;
}

ArrayBuffer arraybuffer_share_reference(ArrayBuffer arraybuffer) {
    if (arraybuffer) arraybuffer->references++;
    return arraybuffer;
}
