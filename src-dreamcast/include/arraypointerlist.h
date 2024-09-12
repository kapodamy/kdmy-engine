#ifndef _arraypointerlist_h
#define _arraypointerlist_h

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "foreach.h"


#define ARRAYPOINTERLIST_ITERATOR(INSTANCE, VARIABLE_TYPE, VARIABLE_NAME) foreach__2(VARIABLE_TYPE, VARIABLE_NAME, INSTANCE, ArrayPointerListIterator, arraypointerlist_get_iterator, arraypointerlist_iterate, (void**))


typedef struct ArrayPointerList_s* ArrayPointerList;

typedef struct {
    bool __run;
    ArrayPointerList arraypointerlist;
    size_t index;
} ArrayPointerListIterator;

typedef void (*FreeDelegate)(void* item);


ArrayPointerList arraypointerlist_init();
ArrayPointerList arraypointerlist_init2(int32_t initial_capacity);
void arraypointerlist_destroy(ArrayPointerList* arraypointerlist);
void arraypointerlist_destroy2(ArrayPointerList* arraypointerlist, int32_t* size_ptr, void** array_ptr);
void arraypointerlist_destroy3(ArrayPointerList* arraypointerlist, FreeDelegate free_function);

int32_t arraypointerlist_size(ArrayPointerList arraypointerlist);

void* arraypointerlist_get(ArrayPointerList arraypointerlist, int32_t index);
int32_t arraypointerlist_insert_on_null_slot(ArrayPointerList arraypointerlist, const void* item);
int32_t arraypointerlist_add(ArrayPointerList arraypointerlist, const void* item);
void arraypointerlist_set(ArrayPointerList arraypointerlist, int32_t index, const void* item);
void arraypointerlist_clear(ArrayPointerList arraypointerlist);
int32_t arraypointerlist_trim(ArrayPointerList arraypointerlist);
void* arraypointerlist_peek_array(ArrayPointerList arraypointerlist);
int32_t arraypointerlist_index_of(ArrayPointerList arraypointerlist, const void* item);
bool arraypointerlist_has(ArrayPointerList arraypointerlist, const void* item);
int32_t arraypointerlist_remove(ArrayPointerList arraypointerlist, const void* item);
bool arraypointerlist_remove_at(ArrayPointerList arraypointerlist, int32_t index);
void arraypointerlist_cut_size(ArrayPointerList arraypointerlist, int32_t new_size);
ArrayPointerList arraypointerlist_clone(ArrayPointerList arraypointerlist);
void arraypointerlist_sort(ArrayPointerList arraypointerlist, int (*sort_fn)(const void**, const void**));

ArrayPointerListIterator arraypointerlist_get_iterator(ArrayPointerList arraypointerlist);
bool arraypointerlist_iterate(ArrayPointerListIterator* iterator, void** item);

#endif
