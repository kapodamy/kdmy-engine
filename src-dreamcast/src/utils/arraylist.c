#include <string.h>

#include "arraylist.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "number_format_specifiers.h"


#define ARRAYLIST_DEFAULT_CAPACITY 16
#define ARRAYLIST_LENGTH_MULTIPLE 8

#define SET_ITEM(arraylist, index, item)                           \
    {                                                              \
        uint8_t* array_item_ptr = arraylist->array;                \
        array_item_ptr += (size_t)index * arraylist->element_size; \
        if (item) {                                                \
            memcpy(array_item_ptr, item, arraylist->element_size); \
        } else {                                                   \
            memset(array_item_ptr, 0x00, arraylist->element_size); \
        }                                                          \
    }


struct ArrayList_s {
    uint8_t* array;
    size_t length;
    size_t size;
    size_t element_size;
};


ArrayList arraylist_init(size_t element_size) {
    return arraylist_init2(element_size, ARRAYLIST_DEFAULT_CAPACITY);
}

ArrayList arraylist_init2(size_t element_size, int32_t initial_capacity) {
    assert(element_size > 0);
    if (initial_capacity < 1) initial_capacity = ARRAYLIST_DEFAULT_CAPACITY;

    ArrayList arraylist = malloc_chk(sizeof(struct ArrayList_s));
    malloc_assert(arraylist, ArrayList);

    *arraylist = (struct ArrayList_s){
        .array = malloc_chk((size_t)initial_capacity * element_size),
        .length = (size_t)initial_capacity,
        .size = 0,
        .element_size = element_size
    };

    malloc_assert(arraylist->array, void*[]);

    return arraylist;
}

void arraylist_destroy(ArrayList* arraylist_ptr) {
    ArrayList arraylist = *arraylist_ptr;
    if (!arraylist) return;

    free_chk(arraylist->array);

    free_chk(arraylist);
    *arraylist_ptr = NULL;
}

void arraylist_destroy2(ArrayList* arraylist_ptr, int32_t* size_ptr, void** array_ptr) {
    ArrayList arraylist = *arraylist_ptr;
    if (!arraylist) return;

    if (size_ptr) *size_ptr = (int32_t)arraylist->size;

    if (arraylist->size < 1) {
        free_chk(arraylist->array);
        *array_ptr = NULL;
    } else {
        arraylist_trim(arraylist);
        *array_ptr = arraylist->array;
    }

    free_chk(arraylist);
    *arraylist_ptr = NULL;
}

void arraylist_destroy3(ArrayList* arraylist_ptr, void (*free_function)(void*)) {
    ArrayList arraylist = *arraylist_ptr;
    if (!arraylist) return;

    if (free_function) {
        uint8_t* item = arraylist->array;
        for (size_t i = 0; i < arraylist->size; i++) {
            free_function(item);
            item += arraylist->element_size;
        }
    }

    free_chk(arraylist->array);
    free_chk(arraylist);
    arraylist_ptr = NULL;
}


int32_t arraylist_size(ArrayList arraylist) {
    return (int32_t)arraylist->size;
}


void* arraylist_get(ArrayList arraylist, int32_t index) {
    if (index < 0 || index >= arraylist->size) {
        logger_error("arraylist_get() index of bounds size index=" FMT_I4 " size=%u", index, arraylist->size);
        assert(!arraylist);
    }

    return arraylist->array + (arraylist->element_size * (size_t)index);
}

void* arraylist_add(ArrayList arraylist, const void* item) {
    size_t size = arraylist->size + 1;

    if (size > arraylist->length) {
        arraylist->length = size + (ARRAYLIST_LENGTH_MULTIPLE - (size % ARRAYLIST_LENGTH_MULTIPLE));
        arraylist->array = realloc_chk(arraylist->array, arraylist->length * arraylist->element_size);
        if (!arraylist->array) {
            logger_error("arraylist_add() out of memory length=%u", arraylist->length);
            assert(arraylist->array);
        }
    }

    void* item_in_array = arraylist->array + (arraylist->size * arraylist->element_size);

    SET_ITEM(arraylist, arraylist->size, item);
    arraylist->size = size;

    return item_in_array;
}

void arraylist_set(ArrayList arraylist, int32_t index, const void* item) {
    if (index < 0 || index >= arraylist->size) {
        logger_error("arraylist_get() index of bounds size index=" FMT_I4 " size=%u", index, arraylist->size);
        assert(!arraylist);
    }

    SET_ITEM(arraylist, index, item);
}

void arraylist_clear(ArrayList arraylist) {
    memset(arraylist->array, 0x00, arraylist->size * arraylist->element_size);
    arraylist->size = 0;
}

int32_t arraylist_trim(ArrayList arraylist) {
    if (arraylist->size == arraylist->length) return (int32_t)arraylist->size;

    if (arraylist->size < 1)
        arraylist->length = ARRAYLIST_LENGTH_MULTIPLE;
    else
        arraylist->length = arraylist->size;

    arraylist->array = realloc_chk(arraylist->array, arraylist->length * arraylist->element_size);
    if (!arraylist->array) {
        logger_error("arraylist_trim() out of memory length=%u", arraylist->length);
        assert(arraylist->array);
    }

    return (int32_t)arraylist->size;
}

void* arraylist_peek_array(ArrayList arraylist) {
    return arraylist->array;
}

int32_t arraylist_index_of(ArrayList arraylist, const void* item) {
    const size_t element_size = arraylist->element_size;

    if (item) {
        uint8_t* ptr = arraylist->array;
        for (size_t i = 0; i < arraylist->size; i++) {
            if (memcmp(ptr, item, element_size) == 0)
                return (int32_t)i;
            ptr += element_size;
        }
    } else {
        uint8_t empty_item[element_size];
        // memset(empty_item, 0x00, element_size);

        uint8_t* ptr = arraylist->array;
        for (size_t i = 0; i < arraylist->size; i++) {
            if (memcmp(ptr, empty_item, element_size) == 0)
                return (int32_t)i;
            ptr += element_size;
        }
    }

    return -1;
}

bool arraylist_has(ArrayList arraylist, const void* item) {
    return arraylist_index_of(arraylist, item) != -1;
}

int32_t arraylist_remove(ArrayList arraylist, const void* item) {
    const size_t element_size = arraylist->element_size;

    uint8_t empty_item[element_size];
    // memset(empty_item, 0x00, element_size);

    if (!item) item = empty_item;

    size_t size = arraylist->size;
    uint8_t* start_ptr = arraylist->array;
    uint8_t* end_ptr = start_ptr + (size * element_size);
    uint8_t* item_ptr = NULL;
    size_t occurences = 0;
    size_t removed_count = 0;

L_find_item:
    for (uint8_t* ptr = start_ptr; ptr < end_ptr; ptr += element_size) {
        if (memcmp(ptr, item, element_size) == 0) {
            if (occurences < 1) item_ptr = ptr;
            occurences++;
        } else if (occurences > 0) {
            break;
        }
    }

    if (item_ptr) {
        size_t elements_size = element_size * occurences;
        uint8_t* next_item_ptr = item_ptr + elements_size;
        memmove(item_ptr, next_item_ptr, (size_t)(end_ptr - next_item_ptr));

        end_ptr -= elements_size;
        size -= occurences;
        removed_count += occurences;

        start_ptr = item_ptr;
        item_ptr = NULL;
        occurences = 0;

        if (size > 0) goto L_find_item;
    }

    arraylist->size = size;
    return (int32_t)removed_count;
}

bool arraylist_remove_at(ArrayList arraylist, int32_t index) {
    if (index < 0 || index >= arraylist->size) return false;

    int32_t next_index = index + 1;
    size_t new_size = arraylist->size - 1;

    if (next_index >= arraylist->size) {
        goto L_return;
    }

    uint8_t* array = arraylist->array;

    uint8_t* item_ptr = array + ((size_t)index * arraylist->element_size);
    uint8_t* next_item_ptr = item_ptr + arraylist->element_size;

    memmove(item_ptr, next_item_ptr, (arraylist->size - (size_t)index) * arraylist->element_size);

L_return:
    arraylist->size = new_size;
    return true;
}

void arraylist_cut_size(ArrayList arraylist, int32_t new_size) {
    if (new_size < arraylist->size) arraylist->size = new_size < 1 ? 0 : (size_t)new_size;
}

ArrayList arraylist_clone(ArrayList arraylist) {
    ArrayList copy = arraylist_init2(arraylist->element_size, (int32_t)arraylist->size);
    copy->size = arraylist->size;

    memcpy(copy->array, arraylist->array, arraylist->size * arraylist->element_size);

    return copy;
}

void arraylist_sort(ArrayList arraylist, int (*sort_fn)(const void*, const void*)) {
    qsort(arraylist->array, arraylist->size, arraylist->element_size, sort_fn);
}


ArrayListIterator arraylist_get_iterator(ArrayList arraylist) {
    return (ArrayListIterator){
        .__run = arraylist->size > 0,
        .arraylist = arraylist,
        .index = 0
    };
}

bool arraylist_iterate(ArrayListIterator* iterator, void** item) {
    size_t index = iterator->index;
    if (index < iterator->arraylist->size) {
        *item = iterator->arraylist->array + (index * iterator->arraylist->element_size);
        iterator->index = index + 1;
        return true;
    }

    return false;
}
