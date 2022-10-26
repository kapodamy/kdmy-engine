#ifndef __linkedlist_h
#define __linkedlist_h

#include <stdlib.h>
#include <stdint.h>

/*
union _Linkedlist_node_value {
    double value_double;
    void* value_pointer;
    float value_float;
    int64_t value_long;
    int32_t value_integer;
    int16_t value_short;
};
*/

typedef struct __Linkedlist_node_t {
    struct __Linkedlist_node_t* next;
    void* item;
} _Linkedlist_node_t;

typedef _Linkedlist_node_t* _Linkedlist_node;


typedef struct _Linkedlist {
    _Linkedlist_node tail;
    _Linkedlist_node head;
    int32_t count;
} *Linkedlist;

typedef void (Linkedlist_free)(void*);

/*#define FOREACH_LINKEDLIST(linkedlist, type, var) type var = \
    linkedlist->head ? (type)linkedlist->head->item : NULL; \
    for ( \
        _Linkedlist_node current ## _ ## linkedlist ## _ ## var = linkedlist->head; \
        current ## _ ## linkedlist ## _ ## var; \
        current ## _ ## linkedlist ## _ ## var = \
        current ## _ ## linkedlist ## _ ## var->next, \
        var = current ## _ ## linkedlist ## _ ## var ? (type)linkedlist->head->item : NULL)*/

#define ITERATE_LINKEDLIST(linkedlist, type, tempname, var, code) \
    _Linkedlist_node tempname  = linkedlist->head; \
    while (tempname) { \
        type var = tempname->item; \
        tempname = tempname->next; \
        code \
    } \



Linkedlist linkedlist_init();

void linkedlist_destroy(Linkedlist* linkedlist);

void linkedlist_destroy2(Linkedlist* linkedlist, Linkedlist_free release_function);

int32_t linkedlist_add_item(Linkedlist linkedlist, void* obj_ptr);

int linkedlist_has_item(Linkedlist linkedlist, void* obj_ptr);

static inline int32_t linkedlist_count(Linkedlist linkedlist) {
    return linkedlist->count;
}

void* linkedlist_get_by_index(Linkedlist linkedlist, int32_t index);

int32_t linkedlist_remove_item(Linkedlist linkedlist, void* obj_ptr);

void* linkedlist_remove_item_at(Linkedlist linkedlist, int32_t index);

#endif

