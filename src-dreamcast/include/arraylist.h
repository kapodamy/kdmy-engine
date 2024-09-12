#ifndef _arraylist_h
#define _arraylist_h

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "foreach.h"


#define ARRAYLIST_ITERATOR(INSTANCE, VARIABLE_TYPE, VARIABLE_NAME) foreach__2(VARIABLE_TYPE, VARIABLE_NAME, INSTANCE, ArrayListIterator, arraylist_get_iterator, arraylist_iterate, (void**))


typedef struct ArrayList_s* ArrayList;

typedef struct {
    bool __run;
    ArrayList arraylist;
    size_t index;
} ArrayListIterator;

typedef void (*FreeDelegate)(void* item);


ArrayList arraylist_init(size_t element_size);
ArrayList arraylist_init2(size_t element_size, int32_t initial_capacity);
void arraylist_destroy(ArrayList* arraylist);
void arraylist_destroy2(ArrayList* arraylist, int32_t* size_ptr, void** array_ptr);
void arraylist_destroy3(ArrayList* arraylist, FreeDelegate free_function);

int32_t arraylist_size(ArrayList arraylist);

void* arraylist_get(ArrayList arraylist, int32_t index);
int32_t arraylist_insert_on_null_slot(ArrayList arraylist, const void* item);
void* arraylist_add(ArrayList arraylist, const void* item);
void arraylist_set(ArrayList arraylist, int32_t index, const void* item);
void arraylist_clear(ArrayList arraylist);
int32_t arraylist_trim(ArrayList arraylist);
void* arraylist_peek_array(ArrayList arraylist);
int32_t arraylist_index_of(ArrayList arraylist, const void* item);
bool arraylist_has(ArrayList arraylist, const void* item);
int32_t arraylist_remove(ArrayList arraylist, const void* item);
bool arraylist_remove_at(ArrayList arraylist, int32_t index);
void arraylist_cut_size(ArrayList arraylist, int32_t new_size);
ArrayList arraylist_clone(ArrayList arraylist);
void arraylist_sort(ArrayList arraylist, int (*sort_fn)(const void*, const void*));

ArrayListIterator arraylist_get_iterator(ArrayList arraylist);
bool arraylist_iterate(ArrayListIterator* iterator, void** item);

#endif
