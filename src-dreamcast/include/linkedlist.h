#ifndef _linkedlist_h
#define _linkedlist_h

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "foreach.h"


#define LINKEDLIST_ITERATOR(INSTANCE, VARIABLE_TYPE, VARIABLE_NAME) foreach__2(VARIABLE_TYPE, VARIABLE_NAME, INSTANCE, LinkedListIterator, linkedlist_get_iterator, linkedlist_iterate, (void**))


typedef struct LinkedListNode_s* LinkedListNode;
typedef struct LinkedList_s* LinkedList;

typedef struct {
    bool __run;
    LinkedListNode current;
} LinkedListIterator;

typedef void (*FreeDelegate)(void* item);


LinkedList linkedlist_init();
void linkedlist_destroy(LinkedList* linkedlist);
void linkedlist_destroy2(LinkedList* linkedlist, FreeDelegate release_function);
int32_t linkedlist_add_item(LinkedList linkedlist, const void* item);
int32_t linkedlist_add_item_unique(LinkedList linkedlist, const void* item);
bool linkedlist_has_item(LinkedList linkedlist, const void* item);
void** linkedlist_to_array(LinkedList linkedlist);
void* linkedlist_to_solid_array(LinkedList linkedlist, size_t element_size);
void linkedlist_clear(LinkedList linkedlist, FreeDelegate release_function);

int32_t linkedlist_count(LinkedList linkedlist);
LinkedListIterator linkedlist_get_iterator(LinkedList linkedlist);
bool linkedlist_iterate(LinkedListIterator* iterator, void** item);
void* linkedlist_get_first_item(LinkedList linkedlist);
void* linkedlist_get_last_item(LinkedList linkedlist);
void* linkedlist_get_by_index(LinkedList linkedlist, int32_t index);
int32_t linkedlist_index_of(LinkedList linkedlist, const void* item);

void linkedlist_set_item(LinkedList linkedlist, int32_t index, const void* item);

int32_t linkedlist_remove_item(LinkedList linkedlist, const void* item);
void* linkedlist_remove_item_at(LinkedList linkedlist, int32_t index);

int32_t linkedlist_add_item_unique2(LinkedList linkedlist, const char* string_item);
bool linkedlist_has_item2(LinkedList linkedlist, const char* string_item);
int32_t linkedlist_index_of2(LinkedList linkedlist, const char* string_item);

#endif