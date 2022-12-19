#include <stdio.h>
#include "linkedlist.h"
#include "assert.h"

Linkedlist linkedlist_init() {
    Linkedlist linkedlist = calloc(1, sizeof(struct _Linkedlist));
    assert(linkedlist);
    return linkedlist;
}

void linkedlist_destroy(Linkedlist* linkedlist) {
    linkedlist_destroy2(linkedlist, NULL);
}

void linkedlist_destroy2(Linkedlist* linkedlist, Linkedlist_free release_function) {
    _Linkedlist_node current = (*linkedlist)->head;

    if (current) {
        if (release_function) {
            while (current) {
                if (current->item != NULL) release_function(current->item);
                _Linkedlist_node next = current->next;
                free(current);
                current = next;
            }
        }
        else {
            while (current) {
                _Linkedlist_node next = current->next;
                free(current);
                current = next;
            }
        }
    }

    free(*linkedlist);
    *linkedlist = NULL;
}


int32_t linkedlist_add_item(Linkedlist linkedlist, void* obj_ptr) {
    _Linkedlist_node node = malloc(sizeof(_Linkedlist_node_t));
    assert(node);

    node->item = obj_ptr;
    node->next = NULL;

    if (!linkedlist->head) {
        linkedlist->head = node;
    }
    else {
        linkedlist->tail->next = node;
    }

    linkedlist->tail = node;
    linkedlist->count++;

    return linkedlist->count;
}

int linkedlist_has_item(Linkedlist linkedlist, void* obj_ptr) {
    _Linkedlist_node current = linkedlist->head;
    while (current) {
        if (current->item == obj_ptr) return 1;
        current = current->next;
    }
    return 0;
}

void* linkedlist_get_by_index(Linkedlist linkedlist, int32_t index) {
    if (index < 0 || index >= linkedlist->count) return NULL;

    _Linkedlist_node current = linkedlist->head;

    for (int32_t i = 0; i < index && current; i++) current = current->next;

    return current ? current->item : NULL;
}

int32_t linkedlist_remove_item(Linkedlist linkedlist, void* obj_ptr) {
    int32_t i = 0;
    _Linkedlist_node current = linkedlist->head;

    while (current) {
        if (current->item == obj_ptr) {
            linkedlist_remove_item_at(linkedlist, i);
            return i;
        }
        current = current->next;
        i++;
    }

    return -1;
}

void* linkedlist_remove_item_at(Linkedlist linkedlist, int32_t index) {
    if (linkedlist->count < 1) return NULL;

    assert(index >= 0 && index < linkedlist->count);

    _Linkedlist_node current;
    void* item;

    if (linkedlist->count == 1) {
        item = linkedlist->head->item;
        free(linkedlist->head);
        linkedlist->head = NULL;
        linkedlist->tail = NULL;
        linkedlist->count = 0;
        return item;
    }

    if (index == 0) {
        current = linkedlist->head->next;
        item = linkedlist->head->item;

        free(linkedlist->head);

        linkedlist->head = current;
        linkedlist->count--;
        return item;
    }

    int32_t i = 1;
    _Linkedlist_node previous = linkedlist->head;
    current = linkedlist->head->next;

    while (current) {
        if (index == i) {
            previous->next = current->next;

            if (linkedlist->tail == current)
                linkedlist->tail = previous;

            linkedlist->count--;
            item = current->item;

            free(current);
            return item;
        }

        i++;
        previous = current;
        current = current->next;
    }

    fprintf(stderr, "linkedlist (%p) is corrupted or in a invalid state\n", linkedlist);
    abort();
}

