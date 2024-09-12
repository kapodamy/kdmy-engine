#ifndef _malloc_utils_h
#define _malloc_utils_h

#include <assert.h>
#include <stdlib.h>


#ifndef __wasm__

//
// ***** mallochk switches *****
//  MALLOC_CHK_ENABLE:
//          tracks allocations on a table, and remove them when released.
//
//  MALLOC_CHK_BOUNDS_CHECK:
//          enables underflow/overflow writes check, requires 66% more RAM per allocation.
//
//  MALLOC_CHK_ENABLE_VERBOSE:
//          logs to stdout every call to malloc, free, realloc, calloc and memalign.
//
//  MALLOC_CHK_ALLOCS_TABLE_SIZE:
//          maximum amount of tracked allocations, asserts when the table is full.
//

// #define MALLOC_CHK_ENABLE
// #define MALLOC_CHK_BOUNDS_CHECK
// #define MALLOC_CHK_ENABLE_VERBOSE
// #define MALLOC_CHK_ALLOCS_TABLE_SIZE 8192

#endif


#ifdef MALLOC_CHK_ENABLE
void free_chk(void* ptr);
void* malloc_chk(size_t size) __attribute__((malloc(free_chk, 1)));
void* memalign_chk(size_t align, size_t size) __attribute__((malloc(free_chk, 1))) __attribute__((alloc_align(1)));

#ifdef MALLOC_CHK_BOUNDS_CHECK
void* calloc_chk(size_t nitems, size_t size) __attribute__((malloc(free_chk, 1)));
void* realloc_chk(void* ptr, size_t size) __attribute__((malloc(free_chk, 1)));
#else
void* calloc_chk(size_t nitems, size_t size) __attribute__((malloc(free_chk, 1))) __attribute__((alloc_size(1, 2)));
void* realloc_chk(void* ptr, size_t size) __attribute__((malloc(free_chk, 1))) __attribute__((alloc_size(2)));
#endif

void mallocstats_chk();
void malloctable_chk();

size_t mallocfilter_add_chk(void* ptr);
size_t mallocfilter_remove_chk(void* ptr);
void mallocfilter_clear_chk();
void mallocfilter_all_chk();
void mallocfilter_rebuild_chk();
void mallocfilter_stats();

void mallocstats_info_of(void* ptr);

#else

#include <malloc.h>

#define free_chk free         // alias to free()
#define malloc_chk malloc     // alias to malloc()
#define calloc_chk calloc     // alias to calloc()
#define realloc_chk realloc   // alias to realloc()
#define memalign_chk memalign // alias to memalign()

#endif

#ifdef __wasm__
#define __assert(_FILE, _LINE, _MSG, _UNUSED, _FUNC) __assert_fail(_MSG, _FILE, _LINE, _FUNC)
#endif

#define nameof(TYPE) #TYPE

#define malloc_warn(TYPE) malloc_warn2(nameof(TYPE))

#define malloc_assert(VARIABLE_NAME, TYPE)                                \
    if (!(VARIABLE_NAME)) {                                               \
        malloc_warn2(#TYPE);                                              \
        __assert(__FILE__, __LINE__, #VARIABLE_NAME, (char*)0, __func__); \
    }

#define assert_for_array_type(VARIABLE, TYPE)                         \
    if (!(VARIABLE)) {                                                \
        malloc_warn2(#TYPE "[]");                                     \
        __assert(__FILE__, __LINE__, #TYPE "[]", (char*)0, __func__); \
    }

#define malloc_for_array(TYPE, ELEMENTS_COUNT) ({                               \
    void* ___tmp_array_ptr;                                                     \
    if (ELEMENTS_COUNT > 0) {                                                   \
        ___tmp_array_ptr = malloc_chk(sizeof(TYPE) * (size_t)(ELEMENTS_COUNT)); \
        assert_for_array_type(___tmp_array_ptr, TYPE);                          \
    } else {                                                                    \
        ___tmp_array_ptr = NULL;                                                \
    }                                                                           \
    ___tmp_array_ptr;                                                           \
})

#define realloc_for_array(ARRAY_PTR, ELEMENTS_COUNT, TYPE) ({                                   \
    void* ___tmp_array_ptr;                                                                     \
    if (ELEMENTS_COUNT > 0) {                                                                   \
        if (ARRAY_PTR != NULL) {                                                                \
            ___tmp_array_ptr = realloc_chk(ARRAY_PTR, sizeof(TYPE) * (size_t)(ELEMENTS_COUNT)); \
        } else {                                                                                \
            ___tmp_array_ptr = malloc_chk(sizeof(TYPE) * (size_t)(ELEMENTS_COUNT));             \
        }                                                                                       \
        assert_for_array_type(___tmp_array_ptr, TYPE);                                          \
    } else {                                                                                    \
        if (ARRAY_PTR != NULL) {                                                                \
            free_chk(ARRAY_PTR);                                                                \
        }                                                                                       \
        ___tmp_array_ptr = NULL;                                                                \
    }                                                                                           \
    ___tmp_array_ptr;                                                                           \
})

#define calloc_for_array(ELEMENTS_COUNT, TYPE) ({                              \
    void* ___tmp_array_ptr;                                                    \
    if ((ELEMENTS_COUNT) > 0) {                                                \
        ___tmp_array_ptr = calloc_chk((size_t)(ELEMENTS_COUNT), sizeof(TYPE)); \
        assert_for_array_type(___tmp_array_ptr, TYPE);                         \
    } else {                                                                   \
        ___tmp_array_ptr = NULL;                                               \
    }                                                                          \
    ___tmp_array_ptr;                                                          \
})

#define calloc_for_type(TYPE) calloc_chk(1, sizeof(TYPE))
#define malloc_for_type(TYPE) malloc_chk(sizeof(TYPE))

void malloc_warn2(const char* type_name);
void malloc_warn3(const char* msg);

#ifdef _arch_dreamcast

#ifdef MALLOC_CHK_BOUNDS_CHECK
// disabled DBL_MEM check due malloc_chk, even if the system has 32MiB of RAM
#define DBL_MEM_CHECK 0
#else
// allows check "DBL_MEM" macro (requires "#include <arch/arch.h>" before usage)
#define DBL_MEM_CHECK DBL_MEM
#endif

#else

#define DBL_MEM_CHECK 0 // not available on the current platform

#endif

// allows choose between two macros depeding the amount of RAM installed
#define DBL_CHOOSE(MACRO) (DBL_MEM_CHECK != 0x00 ? MACRO##_DBL : MACRO)


#endif