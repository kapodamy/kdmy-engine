#ifndef _malloc_utils_h
#define _malloc_utils_h

#include <assert.h>
#include <stdlib.h>


#ifndef __wasm__

#define MALLOC_CHK_ENABLE
#define MALLOC_CHK_ENABLE_NO_BOUNDS_CHECK

#endif


#if defined(MALLOC_CHK_ENABLE)
void free_chk(void* ptr);
void* malloc_chk(size_t size) __attribute__((malloc(free_chk, 1)));
void* memalign_chk(size_t align, size_t size) __attribute__((malloc(free_chk, 1))) __attribute__((alloc_align(1)));

#if defined(MALLOC_CHK_ENABLE_NO_BOUNDS_CHECK)
void* calloc_chk(size_t nitems, size_t size) __attribute__((malloc(free_chk, 1))) __attribute__((alloc_size(1, 2)));
void* realloc_chk(void* ptr, size_t size) __attribute__((malloc(free_chk, 1))) __attribute__((alloc_size(2)));
#else
void* calloc_chk(size_t nitems, size_t size) __attribute__((malloc(free_chk, 1)));
void* realloc_chk(void* ptr, size_t size) __attribute__((malloc(free_chk, 1)));
#endif

void mallocstats_chk();

#else

#include <malloc.h>

#define free_chk free
#define malloc_chk malloc
#define calloc_chk calloc
#define realloc_chk realloc
#define memalign_chk memalign

#endif

#define nameof(TYPE) #TYPE

#define malloc_warn(TYPE) malloc_warn2(nameof(TYPE))

#define malloc_assert(VARIABLE_NAME, TYPE)                                     \
    if (!(VARIABLE_NAME)) {                                                    \
        malloc_warn2(#TYPE);                                                   \
        __assert(__FILE__, __LINE__, #VARIABLE_NAME, (char*)0, __ASSERT_FUNC); \
    }

#define malloc_for_array(TYPE, ELEMENTS_COUNT) malloc_chk(sizeof(TYPE) * (size_t)(ELEMENTS_COUNT))

void malloc_warn2(const char* type_name);
void malloc_warn3(const char* msg);

#ifdef __wasm__
#undef malloc_assert
#define malloc_assert(VARIABLE_NAME, TYPE)                           \
    if (!(VARIABLE_NAME)) {                                          \
        malloc_warn2(#TYPE);                                         \
        __assert_fail(#VARIABLE_NAME, __FILE__, __LINE__, __func__); \
    }

#endif

#endif