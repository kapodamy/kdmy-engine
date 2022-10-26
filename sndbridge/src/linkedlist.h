#ifndef __linkedlist_h
#define __linkedlist_h

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>


#define MACRO_STRCONCAT(a, b) MACRO_STRCONCAT_HELPER(a, b)
#define MACRO_STRCONCAT_HELPER(a, b) a##b
#define MACRO_UNQNAME(name) MACRO_STRCONCAT(__##name, __LINE__)

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


typedef struct {
    _Linkedlist_node tail;
    _Linkedlist_node head;
    int32_t count;
} Linkedlist_t;

typedef Linkedlist_t* Linkedlist;

typedef void(Linkedlist_free)(void*);

/*#define ITERATE_LINKEDLIST(linkedlist, type, tempname, var, code) \
    _Linkedlist_node tempname = linkedlist->head;                 \
    while (tempname) {                                            \
        type var = tempname->item;                                \
        tempname = tempname->next;                                \
        code                                                      \
    }*/

#define LINKEDLIST_FOREACH(TYPE, ITEM_NAME, linkedlist)                                                                                                                                                                     \
    for (_Linkedlist_node MACRO_UNQNAME(tmp) = (linkedlist)->head, MACRO_UNQNAME(flag) = (_Linkedlist_node)1; MACRO_UNQNAME(tmp); MACRO_UNQNAME(tmp) = MACRO_UNQNAME(tmp)->next, MACRO_UNQNAME(flag) = (_Linkedlist_node)1) \
        for (TYPE ITEM_NAME = MACRO_UNQNAME(tmp)->item; MACRO_UNQNAME(flag); MACRO_UNQNAME(flag) = (_Linkedlist_node)0)


Linkedlist linkedlist_init();
void linkedlist_destroy(Linkedlist* linkedlist);
void linkedlist_destroy2(Linkedlist* linkedlist, Linkedlist_free release_function);
int32_t linkedlist_add_item(Linkedlist linkedlist, void* obj_ptr);
bool linkedlist_has_item(Linkedlist linkedlist, void* obj_ptr);
void* linkedlist_get_by_index(Linkedlist linkedlist, int32_t index);
int32_t linkedlist_remove_item(Linkedlist linkedlist, void* obj_ptr);
void* linkedlist_remove_item_at(Linkedlist linkedlist, int32_t index);

static inline int32_t linkedlist_count(Linkedlist linkedlist) {
    return linkedlist->count;
}

static inline void* linkedlist_get_first_item(Linkedlist linkedlist) {
    return linkedlist->head;
}

#endif
