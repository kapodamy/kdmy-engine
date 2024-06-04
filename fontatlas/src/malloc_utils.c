#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <arch/arch.h>

#include "malloc_utils.h"
#include "logger.h"


typedef struct {
    void* ptr;
    size_t size;
    size_t id;
} MemEntry;


#define ALLOCS_TABLE_SIZE 1024
static MemEntry allocs_table[ALLOCS_TABLE_SIZE];


void __attribute__((constructor)) __ctor_chk() {
    memset(allocs_table, 0x00, sizeof(allocs_table));
}


#if defined(MALLOC_CHK_ENABLE_NO_BOUNDS_CHECK)

static size_t allocs_table_id = 0;


static void add_to_table(void* old_ptr, void* new_ptr, size_t req_size) {
    for (size_t i = 0; i < ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr == old_ptr) continue;
        allocs_table[i].ptr = new_ptr;
        allocs_table[i].size = req_size;
        allocs_table[i].id = allocs_table_id++;
        return;
    }

    assert("malloc_utils: add_to_table() failed");
}


void* malloc_chk(size_t size) {
    assert(size > 0);
    assert(size < (16 * 1024 * 1024));

    void* ret = malloc(size);
    add_to_table(NULL, ret, size);

    return ret;
}

void* calloc_chk(size_t nitems, size_t size) {
    assert(size > 0);
    assert(nitems > 0);
    assert((nitems * size) < (16 * 1024 * 1024));

    void* ret = calloc(nitems, size);
    add_to_table(NULL, ret, nitems * size);

    return ret;
}

void free_chk(void* ptr) {
    if (ptr)
        add_to_table(ptr, NULL, 0);

    free(ptr);
}

void* realloc_chk(void* ptr, size_t size) {
    assert(ptr);
    assert(size > 0);
    assert(size < (16 * 1024 * 1024));

    void* ret = realloc(ptr, size);

    if (ptr != ret)
        add_to_table(ptr, ret, size);

    return ret;
}

void* memalign_chk(size_t align, size_t size) {
    assert(size > 0);
    assert(size < (16 * 1024 * 1024));

    void* ret = memalign(align, size);
    add_to_table(NULL, ret, size);

    return ret;
}

#elif defined(MALLOC_CHK_ENABLE)

static void fill(unsigned char* ptr, size_t padding) {
    for (size_t i = 0, j = 0; i < padding; i++) {
        *ptr++ = (unsigned char)j;
        j++;
        if (j > 0xFF) j = 0x00;
    }
}

static void check(unsigned char* ptr, size_t padding) {
    for (size_t i = 0, j = 0; i < padding; i++) {
        if (*ptr != (unsigned char)j) {
            printf("malloc_chk check() failed at %p\n", ptr);
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


void* malloc_chk(size_t size) {
    assert(size > 0);
    assert(size < (16 * 1024 * 1024));

    for (size_t i = 0; i < ALLOCS_TABLE_SIZE; i++) {
        if (!allocs_table[i].ptr) {
            allocs_table[i].ptr = malloc(size * 3);
            allocs_table[i].size = size;
            allocs_table[i].id = allocs_table_id++;
            assert(allocs_table[i].ptr);

            fill(allocs_table[i].ptr, size);
            fill((unsigned char*)allocs_table[i].ptr + size + size, size);

            allocs_table[i].ptr = (unsigned char*)allocs_table[i].ptr + size;
            printf("void* var_%p = malloc(%u);\n", allocs_table[i].ptr, size);
            return allocs_table[i].ptr;
        }
    }

    printf("malloc_chk() failed, allocs_table is full.\n");
    assert(NULL);
    return NULL;
}

void* calloc_chk(size_t nitems, size_t size) {
    assert(size > 0);
    assert((nitems * size) < (16 * 1024 * 1024));

    if (nitems < 1) {
        return calloc(nitems, size);
    }

    for (size_t i = 0; i < ALLOCS_TABLE_SIZE; i++) {
        if (!allocs_table[i].ptr) {
            allocs_table[i].ptr = calloc(nitems, size * 3);
            allocs_table[i].size = nitems * size;
            allocs_table[i].id = allocs_table_id++;
            assert(allocs_table[i].ptr);

            fill(allocs_table[i].ptr, allocs_table[i].size);
            fill((unsigned char*)allocs_table[i].ptr + allocs_table[i].size + allocs_table[i].size, allocs_table[i].size);

            allocs_table[i].ptr = (unsigned char*)allocs_table[i].ptr + allocs_table[i].size;
            printf("void* var_%p = calloc(%u, %u);\n", allocs_table[i].ptr, nitems, size);
            return allocs_table[i].ptr;
        }
    }

    printf("calloc_chk() failed, allocs_table is full.\n");
    assert(NULL);
    return NULL;
}

void free_chk(void* ptr) {
    if (!ptr) return;

    for (size_t i = 0; i < ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr == ptr) {

            check(
                (unsigned char*)ptr - allocs_table[i].size,
                allocs_table[i].size
            );
            check(
                (unsigned char*)ptr + allocs_table[i].size,
                allocs_table[i].size
            );

            printf("free(var_%p);\n", ptr);

            memset((unsigned char*)allocs_table[i].ptr - allocs_table[i].size, 0x00, allocs_table[i].size);
            free((unsigned char*)allocs_table[i].ptr - allocs_table[i].size);
            allocs_table[i].ptr = NULL;
            allocs_table[i].id = allocs_table_id++;
            return;
        }
    }

    printf("free_chk() failed, %p not found in allocs_table.\n", ptr);
    assert(NULL);
}

void* realloc_chk(void* ptr, size_t size) {
    assert(ptr);
    assert(size > 0);
    assert(size < (16 * 1024 * 1024));

    for (size_t i = 0; i < ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr == ptr) {
            unsigned char* old_ptr = (unsigned char*)ptr;
            check(
                old_ptr - allocs_table[i].size,
                allocs_table[i].size
            );
            check(
                old_ptr + allocs_table[i].size,
                allocs_table[i].size
            );

            if (allocs_table[i].size == size) {
                // nothing to do
                return ptr;
            }

            memmove(old_ptr - allocs_table[i].size, old_ptr, allocs_table[i].size);

            unsigned char* new_ptr = realloc(old_ptr - allocs_table[i].size, size * 3);
            if (!new_ptr) {
                move(old_ptr, old_ptr - allocs_table[i].size, allocs_table[i].size);
                fill(old_ptr - allocs_table[i].size, allocs_table[i].size);
                printf("var_%p = realloc(var_%p, %u);// failed (out-of-memory)\n", ptr, ptr, size);
                return NULL;
            }

            if ((old_ptr - allocs_table[i].size) == new_ptr)
                printf("var_%p = realloc(var_%p, %u);// same pointer\n", ptr, ptr, size);
            else
                printf("var_%p = realloc(var_%p, %u);// new pointer var_%p\n", ptr, ptr, size, new_ptr + size);

            memmove(new_ptr + size, new_ptr, size);

            fill(new_ptr, size);
            fill(new_ptr + size + size, size);

            allocs_table[i].ptr = new_ptr + size;
            allocs_table[i].size = size;
            allocs_table[i].id = allocs_table_id++;

            return allocs_table[i].ptr;
        }
    }

    printf("realloc_chk() failed, %p not found in allocs_table.\n", ptr);
    assert(NULL);
    return NULL;
}

void* memalign_chk(size_t align, size_t size) {
    assert(size > 0);
    assert(size < (16 * 1024 * 1024));

    for (size_t i = 0; i < ALLOCS_TABLE_SIZE; i++) {
        if (!allocs_table[i].ptr) {
            size_t padding = align - (size % align);
            if (padding != align) size += padding;

            allocs_table[i].ptr = memalign(align, size * 3);
            allocs_table[i].size = size;
            allocs_table[i].id = allocs_table_id++;

            assert(allocs_table[i].ptr);

            fill(allocs_table[i].ptr, size);
            fill((unsigned char*)allocs_table[i].ptr + size + size, size);

            allocs_table[i].ptr = (unsigned char*)allocs_table[i].ptr + size;
            printf("void* var_%p = memalign(%u);\n", allocs_table[i].ptr, size);
            return allocs_table[i].ptr;
        }
    }

    printf("memalign_chk() failed, allocs_table is full.\n");
    assert(NULL);
    return NULL;
}

#endif

void mallocstats_chk() {
#if defined(MALLOC_CHK_ENABLE)

    size_t total = 0;
    uint32_t available = HW_MEMSIZE;

#if !defined(MALLOC_CHK_ENABLE_NO_BOUNDS_CHECK)
    printf("mallocstats (note: memalign() allocs can report a different size):\n");
#endif

    for (size_t i = 0; i < ALLOCS_TABLE_SIZE; i++) {
        if (allocs_table[i].ptr) {
#if !defined(MALLOC_CHK_ENABLE_NO_BOUNDS_CHECK)
            check(
                (unsigned char*)allocs_table[i].ptr - allocs_table[i].size,
                allocs_table[i].size
            );
            check(
                (unsigned char*)allocs_table[i].ptr + allocs_table[i].size,
                allocs_table[i].size
            );
#endif

            printf("    var_%p  (%u bytes) (id %u)\n", allocs_table[i].ptr, allocs_table[i].size, allocs_table[i].id);

            total += allocs_table[i].size;
        }
    }

    printf("-----------\n");

#if defined(MALLOC_CHK_ENABLE_NO_BOUNDS_CHECK)
    uint32_t percent = total / available;
    if (percent < 1) percent = 1;
    printf("    %u bytes used (%lu%% of %lu bytes)\n", total, percent, available);
#else
    uint32_t percent = (total * 3) / available;
    if (percent < 1) percent = 1;

    printf(
        "    used: %u bytes  boundaries: %u bytes  total: %u bytes (%lu%% of %lu bytes)\n",
        total, total * 2, total * 3, percent, available
    );
#endif

    printf("-----------\n");

#else
    printf("mallocstats_chk() unavailable, defined(MALLOC_CHK_ENABLE) macro not defined");
#endif
}


void malloc_warn2(const char* type_name) {
    logger_error("Failed to allocate '%s' type, out-of-memory.", type_name);
}

void malloc_warn3(const char* msg) {
    logger_error("Failed to allocate %s, out-of-memory.", msg);
}