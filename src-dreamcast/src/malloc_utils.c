#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <arch/arch.h>
#include <arch/spinlock.h>
#include <dc/biosfont.h>
#include <dc/video.h>

#include "logger.h"
#include "malloc_utils.h"


#if defined(MALLOC_CHK_ENABLE)

#define MALLOC_CHK_MAX_BASE_RAM (16 * 1024 * 1024) // 16MiB


typedef struct {
    void* ptr_returned;
    size_t size_allocated;
    size_t id;
#ifdef MALLOC_CHK_BOUNDS_CHECK
    unsigned char* ptr_allocated;
    size_t padding;
    size_t align;
#endif
} MemEntry;


static volatile MemEntry allocs_table[MALLOC_CHK_ALLOCS_TABLE_SIZE];
static volatile uint8_t allocs_table_filter[MALLOC_CHK_ALLOCS_TABLE_SIZE];
static volatile size_t allocs_table_id = 1;
static volatile spinlock_t lock = SPINLOCK_INITIALIZER;


#ifdef MALLOC_CHK_BOUNDS_CHECK

static void fill(unsigned char* ptr, size_t padding) {
    size_t j = 0; // padding;
    // while (j > 0x0FF) j >>= 2;

    for (size_t i = 0; i < padding; i++) {
        *ptr++ = (unsigned char)j;
        j++;
        if (j > 0xFF) j = 0x00;
    }
}

static void check(size_t id, unsigned char* ptr, size_t padding) {
    size_t j = 0; // padding;
    // while (j > 0x0FF) j >>= 2;

    for (size_t i = 0; i < padding; i++) {
        if (*ptr != (unsigned char)j) {
            printf("malloc_chk check() failed at %p, alloc id: %u\n", ptr, id);
            fflush(stdout);
            assert(*ptr == (unsigned char)j);
        }
        ptr++;
        j++;
        if (j > 0xFF) j = 0x00;
    }
}

static void move(unsigned char* dst, unsigned char* src, size_t size) {
    for (ssize_t i = (ssize_t)size - 1; i >= 0; i--) {
        dst[i] = src[i];
    }
}

static void fill_ranges(unsigned char* ptr_allocated, size_t size_allocated, size_t padding) {
    fill(ptr_allocated, padding);
    // memset(ptr_allocated + padding, 0xFF, size_allocated);
    fill(ptr_allocated + padding + size_allocated, padding);

    // if this function never returns, is because allocations are corrupted
    // mallinfo();
}

static void check_ranges(size_t id, unsigned char* ptr_allocated, size_t size_allocated, size_t padding) {
    check(id, ptr_allocated, padding);
    check(id, ptr_allocated + padding + size_allocated, padding);

    // if this function never returns, is because allocations are corrupted
    mallinfo();
}

static void check_alloc_id() {
    //
    // Place here the id to get notified
    // Note: id 0 is never used
    //
    static size_t notify_on_id = 0;

    if (notify_on_id == allocs_table_id) {
        // place a breakpoint here
        printf("check_alloc_id() found %u\n", allocs_table_id);
    }
}


void* malloc_chk(size_t size) {
    assert(size > 0);
    assert(size < MALLOC_CHK_MAX_BASE_RAM);

    // if this function never returns, is because allocations are corrupted
    // mallinfo();
    spinlock_lock(&lock);
    check_alloc_id();

    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (!allocs_table[i].ptr_allocated) {
            allocs_table[i].ptr_allocated = malloc(size * 3);
            allocs_table[i].size_allocated = size;
            allocs_table[i].padding = size;
            allocs_table[i].id = allocs_table_id++;
            allocs_table[i].align = 1;

            if (allocs_table[i].ptr_allocated)
                allocs_table[i].ptr_returned = allocs_table[i].ptr_allocated + allocs_table[i].padding;
            else
                allocs_table[i].ptr_returned = NULL;

            spinlock_unlock(&lock);

#ifdef MALLOC_CHK_ENABLE_VERBOSE
            printf("void* var_%p = malloc(%u);\n", allocs_table[i].ptr_returned, size);
#endif
            if (allocs_table[i].ptr_allocated) {
                fill_ranges(allocs_table[i].ptr_allocated, size, size);
            }
            return allocs_table[i].ptr_returned;
        }
    }

    spinlock_unlock(&lock);
    printf("malloc_chk() failed, allocs_table is full.\n");
    fflush(stdout);
    assert(NULL);
    return NULL;
}

void* calloc_chk(size_t nitems, size_t size) {
    assert(nitems > 0);
    assert(size > 0);
    assert((nitems * size) < MALLOC_CHK_MAX_BASE_RAM);

    // if this function never returns, is because allocations are corrupted
    // mallinfo();
    spinlock_lock(&lock);
    check_alloc_id();

    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (!allocs_table[i].ptr_allocated) {
            allocs_table[i].ptr_allocated = calloc(nitems, size * 3);
            allocs_table[i].size_allocated = nitems * size;
            allocs_table[i].padding = allocs_table[i].size_allocated;
            allocs_table[i].id = allocs_table_id++;
            allocs_table[i].align = 1;

            if (allocs_table[i].ptr_allocated)
                allocs_table[i].ptr_returned = allocs_table[i].ptr_allocated + allocs_table[i].padding;
            else
                allocs_table[i].ptr_returned = NULL;

            spinlock_unlock(&lock);

#ifdef MALLOC_CHK_ENABLE_VERBOSE
            printf("void* var_%p = calloc(%u, %u);\n", allocs_table[i].ptr_returned, nitems, size);
#endif
            if (allocs_table[i].ptr_allocated) {
                fill_ranges(allocs_table[i].ptr_allocated, allocs_table[i].size_allocated, allocs_table[i].padding);
            }
            return allocs_table[i].ptr_returned;
        }
    }

    spinlock_unlock(&lock);
    printf("calloc_chk() failed, allocs_table is full.\n");
    fflush(stdout);
    assert(NULL);
    return NULL;
}

void free_chk(void* ptr) {
    if (!ptr) return;

    spinlock_lock(&lock);
    check_alloc_id();

    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr_returned == ptr) {
#ifdef MALLOC_CHK_ENABLE_VERBOSE
            printf("free(var_%p);\n", ptr);
#endif

            check_ranges(
                allocs_table[i].id, allocs_table[i].ptr_allocated, allocs_table[i].size_allocated, allocs_table[i].padding
            );

            memset(allocs_table[i].ptr_allocated, 0x00, allocs_table[i].size_allocated + (allocs_table[i].padding * 2));

            // if this function never returns, is because allocations are corrupted
            // mallinfo();

            free(allocs_table[i].ptr_allocated);

            allocs_table[i].ptr_returned = NULL;
            allocs_table[i].ptr_allocated = NULL;
            allocs_table[i].id = allocs_table_id++;

            // disable filter for this entry
            allocs_table_filter[i] = '\0';

            // if this function never returns, is because allocations are corrupted
            // mallinfo();
            spinlock_unlock(&lock);

            return;
        }
    }

    spinlock_unlock(&lock);
    printf("free_chk() failed, %p not found in allocs_table.\n", ptr);

    char tmp_buf[32];
    int sz = snprintf(tmp_buf, sizeof(tmp_buf), "free_chk() failed %p", ptr);
    tmp_buf[sz] = '\0';

    bfont_draw_str(vram_s, vid_mode->width, 1, tmp_buf);

    static bool halt_execution = true;
    while (halt_execution) {
        thd_pass();
    }

    assert(NULL);
}

void* realloc_chk(void* ptr, size_t size) {
    assert(ptr);
    assert(size > 0);
    assert(size < MALLOC_CHK_MAX_BASE_RAM);

    check_alloc_id();

    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr_returned == ptr) {
            check_ranges(
                allocs_table[i].id, allocs_table[i].ptr_allocated, allocs_table[i].size_allocated, allocs_table[i].padding
            );

            // if this function never returns, is because allocations are corrupted
            // mallinfo();

            if (allocs_table[i].size_allocated == size) {
#ifdef MALLOC_CHK_ENABLE_VERBOSE
                printf("var_%p = realloc(var_%p, %u);// same size\n", ptr, ptr, size);
#endif

                return ptr;
            }

            memmove(allocs_table[i].ptr_allocated, allocs_table[i].ptr_returned, allocs_table[i].size_allocated);

            // if this function never returns, is because allocations are corrupted
            // mallinfo();

            size_t new_padding;
            if (allocs_table[i].align > 1) {
                new_padding = size % allocs_table[i].align;
                if (new_padding < 1)
                    new_padding = size;
                else
                    new_padding = size + (allocs_table[i].align - new_padding);
            } else {
                new_padding = size;
            }

            unsigned char* new_ptr_allocated = realloc(allocs_table[i].ptr_allocated, size + (new_padding * 2));
            unsigned char* new_ptr_returned = new_ptr_allocated + new_padding;

            int out_of_memory = new_ptr_allocated == NULL;
            if (out_of_memory) {
#ifdef MALLOC_CHK_ENABLE_VERBOSE
                printf("var_%p = realloc(var_%p, %u);// failed (out-of-memory)\n", NULL, ptr, size);
#endif

                new_ptr_allocated = allocs_table[i].ptr_allocated;
                new_ptr_returned = allocs_table[i].ptr_returned;
                size = allocs_table[i].size_allocated;
                new_padding = allocs_table[i].padding;
            }

            size_t copy_size = size;
            if (copy_size > allocs_table[i].size_allocated) copy_size = allocs_table[i].size_allocated;

            move(new_ptr_returned, new_ptr_allocated, copy_size);
            fill_ranges(new_ptr_allocated, size, new_padding);

            // if this function never returns, is because allocations are corrupted
            // mallinfo();

            if (out_of_memory) {
                allocs_table[i].id = allocs_table_id++;
                return NULL;
            }

#ifdef MALLOC_CHK_ENABLE_VERBOSE
            if (allocs_table[i].ptr_allocated == new_ptr_allocated)
                printf("var_%p = realloc(var_%p, %u);// same pointer\n", new_ptr_returned, ptr, size);
            else
                printf("var_%p = realloc(var_%p, %u);// changed pointer\n", new_ptr_returned, ptr, size);
#endif

            allocs_table[i].ptr_allocated = new_ptr_allocated;
            allocs_table[i].ptr_returned = new_ptr_returned;
            allocs_table[i].size_allocated = size;
            allocs_table[i].padding = new_padding;
            allocs_table[i].id = allocs_table_id++;

            return allocs_table[i].ptr_returned;
        }
    }

    printf("realloc_chk() failed, %p not found in allocs_table.\n", ptr);
    assert(NULL);
    return NULL;
}

void* memalign_chk(size_t align, size_t size) {
    assert(size > 0);
    assert(size < MALLOC_CHK_MAX_BASE_RAM);
    assert(align >= 2);

    // if this function never returns, is because allocations are corrupted
    // mallinfo();
    spinlock_lock(&lock);
    check_alloc_id();

    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (!allocs_table[i].ptr_allocated) {

            size_t padding = size % align;
            if (padding < 1)
                padding = size;
            else
                padding = size + (align - padding);

            allocs_table[i].ptr_allocated = memalign(align, size + (padding * 2));
            allocs_table[i].size_allocated = size;
            allocs_table[i].padding = padding;
            allocs_table[i].id = allocs_table_id++;
            allocs_table[i].align = align;

            if (allocs_table[i].ptr_allocated)
                allocs_table[i].ptr_returned = allocs_table[i].ptr_allocated + allocs_table[i].padding;
            else
                allocs_table[i].ptr_returned = NULL;

            spinlock_unlock(&lock);

#ifdef MALLOC_CHK_ENABLE_VERBOSE
            printf("void* var_%p = memalign(%u, %u);\n", allocs_table[i].ptr_returned, align, size);
#endif
            if (allocs_table[i].ptr_allocated) {
                fill_ranges(allocs_table[i].ptr_allocated, size, padding);
            }
            return allocs_table[i].ptr_returned;
        }
    }

    spinlock_unlock(&lock);
    printf("memalign_chk() failed, allocs_table is full.\n");
    fflush(stdout);
    assert(NULL);
    return NULL;
}

#else

static void add_to_table(void* old_ptr, void* new_ptr, size_t req_size) {
    spinlock_lock(&lock);
    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr_returned != old_ptr) continue;

        allocs_table[i].ptr_returned = new_ptr;
        allocs_table[i].size_allocated = req_size;
        allocs_table[i].id = allocs_table_id++;

        if (req_size == 0) {
            allocs_table_filter[i] = '\0';
        }

        spinlock_unlock(&lock);
        return;
    }

    spinlock_unlock(&lock);

    if (old_ptr == NULL) {
        fflush(stdout);
        assert(!"malloc_chk: allocs_table is full");
    } else {
        printf("malloc_chk: pointer %p not found on allocs_table\n", old_ptr);
        fflush(stdout);
        assert(NULL);
    }
}


void* malloc_chk(size_t size) {
    assert(size > 0);
    assert(size < MALLOC_CHK_MAX_BASE_RAM);

    void* ret = malloc(size);
    if (ret) {
#ifdef MALLOC_CHK_ENABLE_VERBOSE
        printf("void* var_%p = malloc(%u);\n", ret, size);
#endif
        add_to_table(NULL, ret, size);
    }

    return ret;
}

void* calloc_chk(size_t nitems, size_t size) {
    assert(size > 0);
    assert(nitems > 0);
    assert((nitems * size) < MALLOC_CHK_MAX_BASE_RAM);

    void* ret = calloc(nitems, size);
    if (ret) {
#ifdef MALLOC_CHK_ENABLE_VERBOSE
        printf("void* var_%p = calloc(%u, %u);\n", ret, nitems, size);
#endif
        add_to_table(NULL, ret, nitems * size);
    }

    return ret;
}

void free_chk(void* ptr) {
    if (!ptr) return;

    if (ptr) {
#ifdef MALLOC_CHK_ENABLE_VERBOSE
        printf("free(var_%p);\n", ptr);
#endif
        add_to_table(ptr, NULL, 0);
    }

    free(ptr);
}

void* realloc_chk(void* ptr, size_t size) {
    assert(ptr);
    assert(size > 0);
    assert(size < MALLOC_CHK_MAX_BASE_RAM);

#ifdef MALLOC_CHK_ENABLE_VERBOSE
    size_t prev_size = 0;
    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr_returned == ptr) {
            prev_size = allocs_table[i].size_allocated;
            break;
        }
    }
#endif

    void* ret = realloc(ptr, size);

#ifdef MALLOC_CHK_ENABLE_VERBOSE
    if (!ret)
        printf("var_%p = realloc(var_%p, %u);// failed (out-of-memory)\n", NULL, ptr, size);
    else if (ret != ptr)
        printf("var_%p = realloc(var_%p, %u);// changed pointer\n", ret, ptr, size);
    else if (prev_size == size)
        printf("var_%p = realloc(var_%p, %u);// same size\n", ret, ptr, size);
    else
        printf("var_%p = realloc(var_%p, %u);// same pointer\n", ret, ptr, size);
#endif

    if (ret) {
        add_to_table(ptr, ret, size);
    }

    return ret;
}

void* memalign_chk(size_t align, size_t size) {
    assert(size > 0);
    assert(size < MALLOC_CHK_MAX_BASE_RAM);

    void* ret = memalign(align, size);

    if (ret) {
#ifdef MALLOC_CHK_ENABLE_VERBOSE
        printf("void* var_%p = memalign(%u, %u);\n", ret, align, size);
#endif
        add_to_table(NULL, ret, size);
    }

    return ret;
}

#endif


static void mallocstats_chk_internal(int filtered_only) {
    printf("mallocs_chk stats:\n");

    size_t total_size = 0;
    uint32_t available = HW_MEMSIZE;
    size_t count = 0, count_filtered = 0;

#ifdef MALLOC_CHK_BOUNDS_CHECK
    size_t total_padding = 0;
#endif

    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr_returned) {
            if (filtered_only && allocs_table_filter[i] == '\1') {
                count_filtered++;
                continue;
            }

#ifdef MALLOC_CHK_BOUNDS_CHECK
            check_ranges(
                allocs_table[i].id, allocs_table[i].ptr_allocated, allocs_table[i].size_allocated, allocs_table[i].padding
            );
            total_padding += allocs_table[i].padding * 2;
#endif

            printf(
                "    var_%p  (%u bytes) (id %u)\n",
                allocs_table[i].ptr_returned, allocs_table[i].size_allocated, allocs_table[i].id
            );

            total_size += allocs_table[i].size_allocated;
            count++;
        }
    }

    printf("-----------\n");

    if (filtered_only)
        printf("entries: %u   filtered: %u   total: %u\n", count, count_filtered, count + count_filtered);
    else
        printf("entries: %u\n", count);

#ifdef MALLOC_CHK_BOUNDS_CHECK
    float percent = ((total_size + total_padding) / (float)available) * 100.0f;
    printf(
        "used: %u bytes   boundaries: %u bytes   total allocated: %u bytes (%.2f%% of %lu bytes)\n",
        total_size, total_padding, total_size + total_padding, percent, available
    );
#else
    float percent = (total_size / (float)available) * 100.0f;
    printf("    %u bytes used (%.2f%% of %lu bytes)\n", total_size, percent, available);
#endif

    malloc_stats();

    printf("-----------\n");
}

void mallocstats_chk() {
    mallocstats_chk_internal(0);
}

void malloctable_chk() {
    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr_returned) {
#ifdef MALLOC_CHK_BOUNDS_CHECK
            check_ranges(
                allocs_table[i].id, allocs_table[i].ptr_allocated, allocs_table[i].size_allocated, allocs_table[i].padding
            );
#endif
        }
    }

    // if this function never returns, is because allocations are corrupted
    mallinfo();
}


size_t mallocfilter_add_chk(void* ptr) {
    assert(ptr);
    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr_returned == ptr) {
            allocs_table_filter[i] = '\1';
            return allocs_table[i].id;
        }
    }
    printf("mallocfilter_add_chk() the var_%p in not on table\n", ptr);
    return 0;
}

size_t mallocfilter_remove_chk(void* ptr) {
    assert(ptr);
    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr_returned == ptr) {
            allocs_table_filter[i] = '\0';
            return allocs_table[i].id;
        }
    }
    printf("mallocfilter_add_chk() the var_%p in not on table\n", ptr);
    return 0;
}

void mallocfilter_clear_chk() {
    spinlock_lock(&lock);
    size_t count = 0;
    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr_returned && allocs_table_filter[i] == '\1') {
            allocs_table_filter[i] = '\0';
            count++;
        }
    }
    spinlock_unlock(&lock);
    printf("\n-----------\n%u entries removed\n-----------\n", count);
}

void mallocfilter_all_chk() {
    spinlock_lock(&lock);
    size_t count = 0;
    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr_returned && allocs_table_filter[i] == '\0') {
            allocs_table_filter[i] = '\1';
            count++;
        }
    }
    spinlock_unlock(&lock);
    printf("\n-----------\n%u entries filtered\n-----------\n", count);
}

void mallocfilter_rebuild_chk() {
    mallocfilter_all_chk();
}

void mallocfilter_stats() {
    mallocstats_chk_internal(1);
}


void mallocstats_info_of(void* ptr) {
    for (size_t i = 0; i < MALLOC_CHK_ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr_returned == ptr) {
            printf(
                "\n-----------\nptr=%p size=%u id=%u filter=%s\n-----------\n",
                allocs_table[i].ptr_returned, allocs_table[i].size_allocated, allocs_table[i].id,
                allocs_table_filter[i] ? "yes" : "no"
            );
            return;
        }
    }

    printf("mallocstats_of_chk() var_%p not found\n", ptr);
}

#endif

void malloc_warn2(const char* type_name) {
    logger_error("Failed to allocate '%s' type, out-of-memory.", type_name);
}

void malloc_warn3(const char* msg) {
    logger_error("Failed to allocate %s, out-of-memory.", msg);
}
