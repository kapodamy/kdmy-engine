#include <string.h>

#include "arraypointerlist.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "number_format_specifiers.h"


#define ARRAYPOINTERLIST_DEFAULT_CAPACITY 16
#define ARRAYPOINTERLIST_LENGTH_MULTIPLE 8


struct ArrayPointerList_s {
    void** array;
    size_t length;
    size_t size;
};


ArrayPointerList arraypointerlist_init() {
    return arraypointerlist_init2(ARRAYPOINTERLIST_DEFAULT_CAPACITY);
}

ArrayPointerList arraypointerlist_init2(int32_t initial_capacity) {
    if (initial_capacity < 1) initial_capacity = ARRAYPOINTERLIST_DEFAULT_CAPACITY;

    ArrayPointerList arraypointerlist = malloc_chk(sizeof(struct ArrayPointerList_s));
    malloc_assert(arraypointerlist, ArrayPointerList);

    *arraypointerlist = (struct ArrayPointerList_s){
        .array = malloc_for_array(void**, initial_capacity),
        .length = (size_t)initial_capacity,
        .size = 0,
    };

    return arraypointerlist;
}

void arraypointerlist_destroy(ArrayPointerList* arraypointerlist_ptr) {
    ArrayPointerList arraypointerlist = *arraypointerlist_ptr;
    if (!arraypointerlist) return;

    free_chk(arraypointerlist->array);

    free_chk(arraypointerlist);
    *arraypointerlist_ptr = NULL;
}

void arraypointerlist_destroy2(ArrayPointerList* arraypointerlist_ptr, int32_t* size_ptr, void** array_ptr) {
    ArrayPointerList arraypointerlist = *arraypointerlist_ptr;
    if (!arraypointerlist) return;

    if (size_ptr) *size_ptr = (int32_t)arraypointerlist->size;

    if (arraypointerlist->size < 1) {
        free_chk(arraypointerlist->array);
        *array_ptr = NULL;
    } else {
        arraypointerlist_trim(arraypointerlist);
        *array_ptr = arraypointerlist->array;
    }

    free_chk(arraypointerlist);
    *arraypointerlist_ptr = NULL;
}

void arraypointerlist_destroy3(ArrayPointerList* arraypointerlist_ptr, void (*free_function)(void*)) {
    ArrayPointerList arraypointerlist = *arraypointerlist_ptr;
    if (!arraypointerlist) return;

    if (free_function) {
        void** item = arraypointerlist->array;
        for (size_t i = 0; i < arraypointerlist->size; i++) {
            if (*item) free_function(*item);
            item++;
        }
    }

    free_chk(arraypointerlist->array);
    free_chk(arraypointerlist);
    arraypointerlist_ptr = NULL;
}


int32_t arraypointerlist_size(ArrayPointerList arraypointerlist) {
    return (int32_t)arraypointerlist->size;
}


void* arraypointerlist_get(ArrayPointerList arraypointerlist, int32_t index) {
    if (index < 0 || index >= arraypointerlist->size) {
        logger_error("arraypointerlist_get() index of bounds size index=" FMT_I4 " size=%u", index, arraypointerlist->size);
        assert(!arraypointerlist);
    }

    return arraypointerlist->array[index];
}

int32_t arraypointerlist_insert_on_null_slot(ArrayPointerList arraypointerlist, const void* item) {
    void** ptr = arraypointerlist->array;
    for (size_t i = 0; i < arraypointerlist->size; i++) {
        if (*ptr == NULL) {
            *ptr = (void*)item;
            return -1;
        }
        ptr++;
    }

    return arraypointerlist_add(arraypointerlist, item);
}

int32_t arraypointerlist_add(ArrayPointerList arraypointerlist, const void* item) {
    size_t size = arraypointerlist->size + 1;

    if (size > arraypointerlist->length) {
        arraypointerlist->length = size + (ARRAYPOINTERLIST_LENGTH_MULTIPLE - (size % ARRAYPOINTERLIST_LENGTH_MULTIPLE));
        arraypointerlist->array = realloc_for_array(arraypointerlist->array, arraypointerlist->length, void**);
    }

    arraypointerlist->array[arraypointerlist->size] = (void*)item;

    arraypointerlist->size = size;
    return (int32_t)(size - 1);
}

void arraypointerlist_set(ArrayPointerList arraypointerlist, int32_t index, const void* item) {
    if (index < 0 || index >= arraypointerlist->size) {
        logger_error("arraypointerlist_get() index of bounds size index=" FMT_I4 " size=%u", index, arraypointerlist->size);
        assert(!arraypointerlist);
    }

    arraypointerlist->array[index] = (void*)item;
}

void arraypointerlist_clear(ArrayPointerList arraypointerlist) {
    memset(arraypointerlist->array, 0x00, arraypointerlist->size * sizeof(void**));
    arraypointerlist->size = 0;
}

int32_t arraypointerlist_trim(ArrayPointerList arraypointerlist) {
    if (arraypointerlist->size == arraypointerlist->length) return (int32_t)arraypointerlist->size;

    if (arraypointerlist->size < 1)
        arraypointerlist->length = ARRAYPOINTERLIST_LENGTH_MULTIPLE;
    else
        arraypointerlist->length = arraypointerlist->size;

    arraypointerlist->array = realloc_for_array(arraypointerlist->array, arraypointerlist->length, void**);

    return (int32_t)arraypointerlist->size;
}

void* arraypointerlist_peek_array(ArrayPointerList arraypointerlist) {
    return arraypointerlist->array;
}

int32_t arraypointerlist_index_of(ArrayPointerList arraypointerlist, const void* item) {
    void** ptr = arraypointerlist->array;
    for (size_t i = 0; i < arraypointerlist->size; i++) {
        if (*ptr == item)
            return (int32_t)i;
        ptr++;
    }
    return -1;
}

bool arraypointerlist_has(ArrayPointerList arraypointerlist, const void* item) {
    void** ptr = arraypointerlist->array;
    for (size_t i = 0; i < arraypointerlist->size; i++) {
        if (*ptr == item)
            return true;
        ptr++;
    }
    return false;
}

int32_t arraypointerlist_remove(ArrayPointerList arraypointerlist, const void* item) {
    size_t size = arraypointerlist->size;
    void** start_ptr = arraypointerlist->array;
    void** end_ptr = start_ptr + size;
    void** item_ptr = NULL;
    size_t occurences = 0;
    size_t removed_count = 0;

L_find_item:
    for (void** ptr = start_ptr; ptr < end_ptr; ptr++) {
        if (*ptr == item) {
            if (occurences < 1) item_ptr = ptr;
            occurences++;
        } else if (occurences > 0) {
            break;
        }
    }

    if (item_ptr) {
        void** next_item_ptr = item_ptr + occurences;
        memmove(item_ptr, next_item_ptr, (size_t)(end_ptr - next_item_ptr));

        end_ptr -= occurences;
        size -= occurences;
        removed_count += occurences;

        start_ptr = item_ptr;
        item_ptr = NULL;
        occurences = 0;

        if (size > 0) goto L_find_item;
    }

    arraypointerlist->size = size;
    return (int32_t)removed_count;
}

bool arraypointerlist_remove_at(ArrayPointerList arraypointerlist, int32_t index) {
    if (index < 0 || index >= arraypointerlist->size) return false;

    int32_t next_index = index + 1;
    size_t new_size = arraypointerlist->size - 1;

    if (next_index >= arraypointerlist->size) {
        goto L_return;
    }

    void** array = arraypointerlist->array;

    void** item_ptr = array + index;
    void** next_item_ptr = item_ptr + 1;

    memmove(item_ptr, next_item_ptr, (arraypointerlist->size - (size_t)index) * sizeof(void**));

L_return:
    arraypointerlist->size = new_size;
    return true;
}

void arraypointerlist_cut_size(ArrayPointerList arraypointerlist, int32_t new_size) {
    if (new_size < arraypointerlist->size) arraypointerlist->size = new_size < 1 ? 0 : (size_t)new_size;
}

ArrayPointerList arraypointerlist_clone(ArrayPointerList arraypointerlist) {
    ArrayPointerList copy = arraypointerlist_init2((int32_t)arraypointerlist->size);
    copy->size = arraypointerlist->size;

    memcpy(copy->array, arraypointerlist->array, arraypointerlist->size * sizeof(void**));

    return copy;
}

void arraypointerlist_sort(ArrayPointerList arraypointerlist, int (*sort_fn)(const void**, const void**)) {
    int (*ptr)(const void*, const void*) = (int (*)(const void*, const void*))sort_fn;
    qsort(arraypointerlist->array, arraypointerlist->size, sizeof(void**), ptr);
}


ArrayPointerListIterator arraypointerlist_get_iterator(ArrayPointerList arraypointerlist) {
    return (ArrayPointerListIterator){
        .__run = arraypointerlist->size > 0,
        .arraypointerlist = arraypointerlist,
        .index = 0
    };
}

bool arraypointerlist_iterate(ArrayPointerListIterator* iterator, void** item) {
    size_t index = iterator->index;
    if (index < iterator->arraypointerlist->size) {
        *item = iterator->arraypointerlist->array[index];
        iterator->index = index + 1;
        return true;
    }

    return false;
}
