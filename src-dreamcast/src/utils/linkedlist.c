#include <stdlib.h>
#include <string.h>

#include "linkedlist.h"
#include "logger.h"
#include "malloc_utils.h"
#include "stringutils.h"


struct LinkedListNode_s {
    void* item;
    LinkedListNode next;
};

struct LinkedList_s {
    LinkedListNode head;
    LinkedListNode tail;
    size_t count;
};


LinkedList linkedlist_init() {
    LinkedList linkedlist = malloc_chk(sizeof(struct LinkedList_s));
    malloc_assert(linkedlist, LinkedList);

    *linkedlist = (struct LinkedList_s){
        .head = NULL,
        .tail = NULL,
        .count = 0
    };

    return linkedlist;
}

void linkedlist_destroy(LinkedList* linkedlist_ptr) {
    linkedlist_destroy2(linkedlist_ptr, NULL);
}

void linkedlist_destroy2(LinkedList* linkedlist_ptr, FreeDelegate release_function) {
    if (!*linkedlist_ptr) return;

    LinkedList linkedlist = *linkedlist_ptr;

    LinkedListNode current = linkedlist->head;
    while (current) {
        if (release_function && current->item != NULL) release_function(current->item);
        LinkedListNode next = current->next;
        free_chk(current);
        current = next;
    }

    free_chk(linkedlist);
    *linkedlist_ptr = NULL;
}

int32_t linkedlist_add_item(LinkedList linkedlist, const void* item) {
    LinkedListNode node = malloc_chk(sizeof(struct LinkedListNode_s));
    malloc_assert(node, LinkedListNode);

    *node = (struct LinkedListNode_s){.next = NULL, .item = (void*)item};

    if (!linkedlist->head) {
        linkedlist->head = node;
    } else {
        linkedlist->tail->next = node;
    }

    linkedlist->tail = node;
    linkedlist->count++;

    return (int32_t)linkedlist->count;
}

int32_t linkedlist_add_item_unique(LinkedList linkedlist, const void* item) {
    if (linkedlist_has_item(linkedlist, item)) return -1;
    return linkedlist_add_item(linkedlist, item);
}

bool linkedlist_has_item(LinkedList linkedlist, const void* item) {
    LinkedListNode current = linkedlist->head;
    while (current) {
        if (current->item == item) return true;
        current = current->next;
    }
    return false;
}

void** linkedlist_to_array(LinkedList linkedlist) {
    if (linkedlist->count < 1) return NULL;

    int32_t i = 0;
    void** array = malloc_for_array(void*, linkedlist->count);
    LinkedListNode current = linkedlist->head;

    while (current) {
        array[i++] = current->item;
        current = current->next;
    }

    return array;
}

void* linkedlist_to_solid_array(LinkedList linkedlist, size_t element_size) {
    if (linkedlist->count < 1) return NULL;

    uint8_t* array = malloc_chk(element_size * linkedlist->count);
    malloc_assert(array, array[element_size]);

    uint8_t* array_ptr = array;
    LinkedListNode current = linkedlist->head;

    while (current) {
        if (current->item)
            memcpy(array_ptr, current->item, element_size);
        else
            memset(array_ptr, 0x00, element_size);

        current = current->next;
        array_ptr += element_size;
    }

    return array;
}

void linkedlist_clear(LinkedList linkedlist, FreeDelegate release_function) {
    LinkedListNode current = linkedlist->head;
    while (current) {
        if (release_function && current->item != NULL) release_function(current->item);
        LinkedListNode next = current->next;
        free_chk(current);
        current = next;
    }

    linkedlist->count = 0;
}


int32_t linkedlist_count(LinkedList linkedlist) {
    return (int32_t)linkedlist->count;
}

LinkedListIterator linkedlist_get_iterator(LinkedList linkedlist) {
    return (LinkedListIterator){
        .__run = linkedlist->head != NULL,
        .current = linkedlist->head
    };
}

bool linkedlist_iterate(LinkedListIterator* iterator, void** item) {
    LinkedListNode current = iterator->current;
    if (!current) return false;

    *item = current->item;
    iterator->current = current->next;

    return true;
}

void* linkedlist_get_first_item(LinkedList linkedlist) {
    if (linkedlist->head)
        return linkedlist->head->item;
    else
        return NULL;
}

void* linkedlist_get_last_item(LinkedList linkedlist) {
    if (linkedlist->tail)
        return linkedlist->tail->item;
    else
        return NULL;
}

void* linkedlist_get_by_index(LinkedList linkedlist, int32_t index) {
    if (index < 0 || index >= linkedlist->count) return NULL;

    LinkedListNode current = linkedlist->head;

    for (int32_t i = 0; i < index && current; i++) current = current->next;

    return current ? current->item : NULL;
}

int32_t linkedlist_index_of(LinkedList linkedlist, const void* item) {
    int32_t index = 0;
    LinkedListNode current = linkedlist->head;
    while (current) {
        if (current->item == item) return index;
        current = current->next;
        index++;
    }
    return -1;
}


void linkedlist_set_item(LinkedList linkedlist, int32_t index, const void* item) {
    assert(index >= 0 && index < linkedlist->count);

    int32_t i = 0;
    LinkedListNode current = linkedlist->head;
    while (current) {
        if (i == index) {
            current->item = (void*)item;
            return;
        }
        current = current->next;
        i++;
    }
}


int32_t linkedlist_remove_item(LinkedList linkedlist, const void* item) {
    int32_t i = 0;

    foreach (void*, current_item, LINKEDLIST_ITERATOR, linkedlist) {
        if (current_item == item) {
            linkedlist_remove_item_at(linkedlist, i);
            return i;
        }
        i++;
    }

    return -1;
}

void* linkedlist_remove_item_at(LinkedList linkedlist, int32_t index) {
    if (linkedlist->count < 1) return NULL;

    assert(index >= 0 && index < linkedlist->count);

    LinkedListNode current;
    void* item;

    if (linkedlist->count == 1) {
        item = linkedlist->head->item;
        free_chk(linkedlist->head);
        linkedlist->head = NULL;
        linkedlist->tail = NULL;
        linkedlist->count = 0;
        return item;
    }

    if (index == 0) {
        current = linkedlist->head->next;
        item = linkedlist->head->item;

        free_chk(linkedlist->head);

        linkedlist->head = current;
        linkedlist->count--;
        return item;
    }


    int32_t i = 1;
    LinkedListNode previous = linkedlist->head;
    current = linkedlist->head->next;

    while (current) {
        if (index == i) {
            previous->next = current->next;

            if (linkedlist->tail == current)
                linkedlist->tail = previous;

            linkedlist->count--;
            item = current->item;

            free_chk(current);
            return item;
        }

        i++;
        previous = current;
        current = current->next;
    }

    logger_error("linkedlist is corrupted or in a invalid state");
    assert(!linkedlist);

    return NULL;
}



int32_t linkedlist_add_item_unique2(LinkedList linkedlist, const char* string_item) {
    if (linkedlist_has_item2(linkedlist, string_item)) return -1;
    return linkedlist_add_item(linkedlist, string_item);
}

bool linkedlist_has_item2(LinkedList linkedlist, const char* string_item) {
    LinkedListNode current = linkedlist->head;
    while (current) {
        if (string_equals(current->item, string_item)) return true;
        current = current->next;
    }
    return false;
}

int32_t linkedlist_index_of2(LinkedList linkedlist, const char* string_item) {
    int32_t index = 0;
    LinkedListNode current = linkedlist->head;
    while (current) {
        if (string_equals(current->item, string_item)) return index;
        current = current->next;
        index++;
    }
    return -1;
}
