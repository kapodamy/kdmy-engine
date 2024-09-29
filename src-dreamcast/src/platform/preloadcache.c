#include "preloadcache.h"

#include <errno.h>

#include <arch/arch.h>
#include <arch/spinlock.h>
#include <arch/timer.h>
#include <kos/fs.h>

#include "arraypointerlist.h"
#include "fs.h"
#include "io.h"
#include "malloc_utils.h"
#include "stringutils.h"
#include "tokenizer.h"

#define PRELOAD_COMMENT_CHAR ';'
#define PRELOAD_MAX_DIR_DEEP 32
#define PRELOAD_TABLE_SIZE 1024
#define PRELOAD_MAX_BYTES (7 * 1024 * 1024)                       // 7MiB RAM usage
#define PRELOAD_MAX_BYTES_DBL (20 * 1024 * 1024)                  // 20MiB RAM usage
#define PRELOAD_MAX_DATA_BYTES ((1 * 1024 * 1024) + (512 * 1024)) // 1.5MiB file size
#define PRELOAD_MAX_DATA_BYTES_DBL (4 * 1024 * 1024)              // 4MiB file size

#ifdef MALLOC_CHK_ENABLE
//
// reduce sizes to avoid out of memory
//
#undef PRELOAD_MAX_BYTES
#undef PRELOAD_MAX_BYTES_DBL

#define PRELOAD_MAX_BYTES (3 * 1024 * 1024)     // 3MiB RAM usage
#define PRELOAD_MAX_BYTES_DBL (8 * 1024 * 1024) // 8MiB RAM usage

#endif


typedef struct {
    char* native_path;
    size_t id;
    ArrayBuffer arraybuffer;
} CacheEntry;

typedef struct {
    int dirent_index;
    char* native_path;
} InStackFolder;


static CacheEntry table[PRELOAD_TABLE_SIZE] = {0};
static size_t table_used = 0;
static size_t unique_id = 0;
static volatile spinlock_t lock = SPINLOCK_INITIALIZER;


static int preloadcache_internal_list_files_sort(const void** file1_ptr, const void** file2_ptr) {
    const char* file1 = (const char*)*file1_ptr;
    const char* file2 = (const char*)*file2_ptr;

    return string_natural_comparer(file1, file2);
}

static void preloadcache_internal_release_entry(size_t index) {
    arraybuffer_destroy(&table[index].arraybuffer);
    free_chk(table[index].native_path);
    memset(&table[index], 0x00, sizeof(CacheEntry));
}

static int preloadcache_internal_sort(const void* a_ptr, const void* b_ptr) {
    const CacheEntry* a = (const CacheEntry*)a_ptr;
    const CacheEntry* b = (const CacheEntry*)b_ptr;

    if (!a->arraybuffer && !b->arraybuffer)
        return 0;
    else if (!a->arraybuffer)
        return 1;
    else if (!b->arraybuffer)
        return -1;
    else if (a->id < b->id)
        return -1;
    else if (a->id > b->id)
        return 1;
    else
        return 0;
}

static void preloadcache_internal_trim() {
    qsort(table, PRELOAD_TABLE_SIZE, sizeof(CacheEntry), (int (*)(const void*, const void*))preloadcache_internal_sort);

    for (size_t i = 0, last = table_used; i < last; i++) {
        if (!table[i].arraybuffer) {
            table_used = i;
            return;
        }
    }
    table_used = PRELOAD_TABLE_SIZE;
}

static bool preloadcache_internal_flush(size_t needed_space) {
    size_t released_space = 0;
    size_t entries_total = 0;
    size_t entries_released = 0;

    // try find and remove unused entries
    for (size_t i = 0, last = table_used; i < last && released_space < needed_space; i++) {
        if (!table[i].arraybuffer) continue;
        entries_total++;

        if (table[i].arraybuffer->references < 2) {
            entries_released++;
            released_space += table[i].arraybuffer->length;
        }
        preloadcache_internal_release_entry(i);
    }

    preloadcache_internal_trim();

    if (entries_total < 1 || entries_released >= entries_total) {
        return true;
    }

    return released_space >= needed_space;
}

static ArrayPointerList preloadcache_internal_list_files_of_folder(char* root_dir_native_path, bool resolve_files_with_io) {
    ArrayPointerList files = arraypointerlist_init2(32);
    InStackFolder stack[PRELOAD_MAX_DIR_DEEP];
    size_t stack_used = 0;

    // add root directory to the stack
    stack[stack_used++] = (InStackFolder){.native_path = root_dir_native_path, .dirent_index = 0};

    while (stack_used > 0) {
        InStackFolder* current = &stack[stack_used - 1];
        file_t dir_hnd = fs_open(current->native_path, O_RDONLY | O_DIR);

        if (dir_hnd == FILEHND_INVALID) {
            stack_used--;
            free_chk(current->native_path);
            continue;
        }

        int dir_entry_index = 0;
        dirent_t* dir_entry;
        while ((dir_entry = fs_readdir(dir_hnd)) != NULL) {
            int index = dir_entry_index;
            dir_entry_index++;

            if (index < current->dirent_index) {
                // this entry is already processed
                continue;
            }

            if (string_equals(dir_entry->name, ".") || string_equals(dir_entry->name, "..")) {
                continue;
            }

            bool is_file = dir_entry->size >= 0 && dir_entry->attr == 0;

            char* entry_path;
            if (is_file && resolve_files_with_io) {
                entry_path = string_concat(4, current->native_path, FS_STRING_SEPARATOR, dir_entry->name, IO_4Z);
                io_override_file_extension(entry_path, false);
            } else {
                entry_path = string_concat(3, current->native_path, FS_STRING_SEPARATOR, dir_entry->name);
            }

            if (is_file) {
                arraypointerlist_add(files, entry_path);
            } else if (dir_entry->size < 0 && dir_entry->attr == O_DIR) {
                if (stack_used >= PRELOAD_MAX_DIR_DEEP) {
                    logger_warn("preloadcache_internal_list_files_of_folder() deep limit reached for %s", entry_path);
                    free_chk(entry_path);
                    continue;
                }

                // add folder to the stack and pause current directory processing
                stack[stack_used++] = (InStackFolder){.native_path = entry_path, .dirent_index = 0};
                break;
            } else {
                // not a file or directory
                free_chk(entry_path);
            }
        }

        fs_close(dir_hnd);

        if (dir_entry == NULL) {
            // all directory entries processed, remove from the stack
            free_chk(current->native_path);
            stack_used--;
        } else {
            // resume processing later
            current->dirent_index = dir_entry_index;
        }
    }

    // sort files ¿is this step really necessary?
    arraypointerlist_sort(files, preloadcache_internal_list_files_sort);

    return files;
}


#ifdef MALLOC_CHK_ENABLE
static bool malloc_chk_autofilter = false;
void preloadcache_malloc_chk_enable_autofilter_entries(bool enabled) {
    malloc_chk_autofilter = enabled;
}
#endif


void preloadcache_clear() {
    spinlock_lock(&lock);
    for (size_t i = 0, last = table_used; i < last; i++) {
        if (!table[i].arraybuffer) continue;
        preloadcache_internal_release_entry(i);
    }

    preloadcache_internal_trim();
    spinlock_unlock(&lock);
}

bool preloadcache_flush(size_t needed_space) {
    spinlock_lock(&lock);
    bool ret = preloadcache_internal_flush(needed_space);
    spinlock_unlock(&lock);
    return ret;
}

bool preloadcache_append(char* native_path, bool allow_flush, bool low_piority, ArrayBuffer arraybuffer) {
    if (!arraybuffer) return false;
    if (arraybuffer->length > DBL_CHOOSE(PRELOAD_MAX_DATA_BYTES)) return false;

#ifdef DEBUG
    assert(native_path);
#endif

    spinlock_lock(&lock);
    size_t last = table_used;

    // check space usage
    size_t cache_length = arraybuffer->length;
    for (size_t i = 0; i < last; i++) {
        if (table[i].arraybuffer == arraybuffer || string_equals_ignore_case(native_path, table[i].native_path)) {
            // nothing to do, already in cache
            spinlock_unlock(&lock);
            free_chk(native_path);
            return true;
        }

        cache_length += table[i].arraybuffer->length;
    }

    // try release used space (if necessary)
    if (cache_length > DBL_CHOOSE(PRELOAD_MAX_BYTES) && (!allow_flush || !preloadcache_internal_flush(arraybuffer->length))) {
        goto L_failed;
    }

    if (last >= PRELOAD_TABLE_SIZE) {
        // try compact the table
        preloadcache_internal_trim();
        last = table_used;

        if (last >= PRELOAD_TABLE_SIZE) {
            // all table entries are in use
            goto L_failed;
        }
    }

    size_t entry_index;
    if (low_piority) {
        // LIFO order
        memmove(&table[1], &table[0], last * sizeof(CacheEntry));
        entry_index = 0;
    } else {
        // FIFO order
        entry_index = last;
    }

    // add entry
    table[entry_index] = (CacheEntry){
        .native_path = native_path,
        .id = unique_id++,
        .arraybuffer = arraybuffer_share_reference(arraybuffer)
    };

    table_used = last + 1;

#ifdef MALLOC_CHK_ENABLE
    if (malloc_chk_autofilter) {
        mallocfilter_add_chk(table[entry_index].native_path);
        mallocfilter_add_chk(table[entry_index].arraybuffer);
    }
#endif

    spinlock_unlock(&lock);
    return true;

L_failed:
    spinlock_unlock(&lock);
    return false;
}

ArrayBuffer preloadcache_adquire(const char* native_path) {
#ifdef DEBUG
    assert(native_path);
#endif

    ArrayBuffer data = NULL;

    spinlock_lock(&lock);
    for (size_t i = 0, last = table_used; i < last; i++) {
        if (!table[i].arraybuffer) continue;

        if (string_equals_ignore_case(table[i].native_path, native_path)) {
            data = arraybuffer_share_reference(table[i].arraybuffer);
            break;
        }
    }
    spinlock_unlock(&lock);

    return data;
}

bool preloadcache_release(ArrayBuffer adquired_arraybuffer) {
    if (!adquired_arraybuffer) return false;

    bool found = false;

    spinlock_lock(&lock);
    for (size_t i = 0, last = table_used; i < last; i++) {
        if (table[i].arraybuffer != adquired_arraybuffer) continue;

        preloadcache_internal_release_entry(i);

        found = true;
        break;
    }
    spinlock_unlock(&lock);

    return found;
}

bool preloadcache_has(ArrayBuffer arraybuffer) {
    if (!arraybuffer) return false;

    spinlock_lock(&lock);
    bool found = false;
    for (size_t i = 0, last = table_used; i < last; i++) {
        if (table[i].arraybuffer == arraybuffer) {
            found = true;
            break;
        }
    }

    spinlock_unlock(&lock);
    return found;
}

int64_t preloadcache_stat(const char* native_path) {
    if (!native_path) return -1;

    spinlock_lock(&lock);
    int64_t size = -1;
    for (size_t i = 0, last = table_used; i < last; i++) {
        if (table[i].arraybuffer && string_equals_ignore_case(table[i].native_path, native_path)) {
            size = (int64_t)table[i].arraybuffer->length;
            break;
        }
    }

    spinlock_unlock(&lock);
    return size;
}

int32_t preloadcache_add_file_list(const char* src_filelist) {
    spinlock_lock(&lock);

    size_t entries_count = 0;
    size_t cache_length = 0;
    for (size_t i = 0, last = table_used; i < last; i++) {
        if (table[i].arraybuffer) {
            cache_length += table[i].arraybuffer->length;
            entries_count++;
        }
    }

    spinlock_unlock(&lock);

    if (cache_length >= DBL_CHOOSE(PRELOAD_MAX_BYTES) || entries_count >= PRELOAD_TABLE_SIZE) {
        return -1;
    }

    char* absolute_filelist_path = fs_resolve_path(src_filelist);
    if (!io_resource_exists(absolute_filelist_path, true, false)) {
        free_chk(absolute_filelist_path);
        return -1;
    }

    char* filelist_native_path = io_get_native_path(absolute_filelist_path, true, false, false);
    free_chk(absolute_filelist_path);
    ArrayBuffer filelist = arraybuffer_init2(filelist_native_path, DBL_CHOOSE(PRELOAD_MAX_DATA_BYTES));
    free_chk(filelist_native_path);

    if (!filelist) {
        return -1;
    }
    if (string_is_empty(filelist->data)) {
        arraybuffer_destroy(&filelist);
        return -1;
    }

    logger_info("preloadcache_add_file_list() reading %s", src_filelist);

    uint64_t timestamp = timer_ms_gettime64();
    Tokenizer tokenizer = tokenizer_init2("\r\n", true, (const char*)filelist->data, filelist->length);
    int32_t added = 0;
    int32_t ignored = 0;
    char* line;

    while ((line = tokenizer_read_next(tokenizer)) != NULL) {
        if (line[0] == PRELOAD_COMMENT_CHAR) {
            free_chk(line);
            continue;
        } else if (string_index_of_any_char(line, "*?:<>") >= 0) {
            logger_warn("preloadcache_add_file_list() wildcards are not supported: %s", line);
            free_chk(line);
            continue;
        } else if (strlen(line) > 2 && string_ends_with(line, FS_STRING_SEPARATOR)) {
            line[strlen(line) - 1] = '\0';
        }

        char* path = fs_build_path2(src_filelist, line);
        if (string_starts_with(path, FS_ASSETS_FOLDER)) {
            char* tmp = fs_get_full_path_and_override(path);
            free_chk(path);
            path = tmp;
        }

        char* native_path = io_get_native_path(path, false, true, true);

        file_t dir_hnd = fs_open(native_path, O_RDONLY | O_DIR);
        if (dir_hnd != FILEHND_INVALID) {
            fs_close(dir_hnd);
            ArrayPointerList dir_file_list = preloadcache_internal_list_files_of_folder(native_path, true);

            foreach (char*, folder_file_native_path, ARRAYPOINTERLIST_ITERATOR, dir_file_list) {
                ArrayBuffer arraybuffer = arraybuffer_init2(folder_file_native_path, DBL_CHOOSE(PRELOAD_MAX_DATA_BYTES));
                if (!arraybuffer) {
                    ignored++;
                    const char* reason = errno == EFBIG ? "too big" : "not found";
                    logger_warn("preloadcache_add_file_list() file %s %s (resolved as %s)", reason, line, path);
                    free_chk(folder_file_native_path);
                    continue;
                }

                if (preloadcache_append(folder_file_native_path, false, true, arraybuffer)) {
                    added++;
                } else {
                    ignored++;
                    free_chk(folder_file_native_path);
                }
                arraybuffer_destroy(&arraybuffer);
            }

            arraypointerlist_destroy(&dir_file_list);
            continue;
        } else {
            // resolve native path as file
            free_chk(native_path);
            native_path = io_get_native_path(path, true, false, true);
            ArrayBuffer arraybuffer = arraybuffer_init2(native_path, DBL_CHOOSE(PRELOAD_MAX_DATA_BYTES));

            if (!arraybuffer) {
                ignored++;
                free_chk(native_path);
                const char* reason = errno == EFBIG ? "too big" : "not found";
                logger_warn("preloadcache_add_file_list() file %s %s (resolved as %s)", reason, line, path);
                continue;
            }

            if (preloadcache_append(native_path, false, true, arraybuffer)) {
                added++;
            } else {
                ignored++;
                free_chk(native_path);
            }
            arraybuffer_destroy(&arraybuffer);
        }
    }

    float elapsed = (float)(timer_ms_gettime64() - timestamp) / 1000.0f;

    if (ignored > 0) {
        logger_warn("preloadcache_add_file_list() " FMT_I4 " files ignored or unable to open it", ignored);
    }

    if (added < 1)
        logger_info("preloadcache_add_file_list() no files cached (took " FMT_FLT "sec)", elapsed);
    else
        logger_info("preloadcache_add_file_list() " FMT_I4 " files cached in " FMT_FLT "sec", added, elapsed);

    tokenizer_destroy(&tokenizer);
    arraybuffer_destroy(&filelist);

    return added;
}
