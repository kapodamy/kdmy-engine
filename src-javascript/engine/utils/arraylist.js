"use strict";

const ARRAYLIST_DEFAULT_CAPACITY = 16;
const ARRAYLIST_LENGTH_MULTIPLE = 8;


function arraylist_init() {
    return arraylist_init2(ARRAYLIST_DEFAULT_CAPACITY);
}

function arraylist_init2(initial_capacity) {
    if (initial_capacity < 1) initial_capacity = ARRAYLIST_DEFAULT_CAPACITY;
    return {
        array: new Array(initial_capacity),
        length: initial_capacity,
        size: 0,
        iterate_index: 0
    };
}

function arraylist_destroy(arraylist, keep_array_alive) {
    if (!keep_array_alive) arraylist.array = undefined;
    arraylist = undefined;
}

function arraylist_destroy2(arraylist, obj, size_ptr, array_ptr) {
    // javacript only
    if (size_ptr) obj[size_ptr] = arraylist.size;

    if (arraylist.size < 1) {
        obj[array_ptr] = null;
    } else {
        arraylist_trim(arraylist);
        obj[array_ptr] = arraylist.array;
    }

    arraylist = undefined;

    /*
    // C only
    if (size_ptr) *size_ptr = arraylist->size;

    if (arraylist->size < 1) {
        free(arraylist->array);
        *array_ptr = null;
    } else {
        arraylist_trim(arraylist);
        *array_ptr = arraylist->array;
    }
    
    free(arraylist);
    */
}

function arraylist_destroy3(arraylist, free_function) {
    for (let i = 0; i < arraylist.size; i++) free_function(arraylist.array[i]);
    arraylist.array = undefined;
    arraylist = undefined;
}


function arraylist_size(arraylist) {
    return arraylist.size;
}


/**
 * @template T
 * @typedef {object} ArrayList<T>
 * @property {T[]} array
 * @property {number} size
 */
/** 
* @template T
* @param {ArrayList<T>} arraylist
* @returns {T}
*/
function arraylist_get(arraylist, index) {
    if (index < 0 || index >= arraylist.size)
        throw new Error(`arraylist_get() index of bounds size index=${index} size=${arraylist.size}`);

    return arraylist.array[index];
}

function arraylist_insert_on_null_slot(arraylist, item) {
    for (let i = 0; i < arraylist.size; i++) {
        if (arraylist.array[i] == null) {
            arraylist.array[i] = item;
            return -1;
        }
    }

    return arraylist_add(arraylist, item);
}

function arraylist_add(arraylist, item) {
    let size = arraylist.size + 1;

    if (size > arraylist.length) {
        arraylist.length = size + (ARRAYLIST_LENGTH_MULTIPLE - (size % ARRAYLIST_LENGTH_MULTIPLE));
        arraylist.array = realloc(arraylist.array, arraylist.length);
        if (!arraylist.array)
            throw new Error("arraylist_add() out of memory length=" + arraylist.length);
    }

    arraylist.array[arraylist.size] = item;
    arraylist.size = size;
    return size;
}

function arraylist_set(arraylist, index, item) {
    if (index < 0 || index >= arraylist.size)
        throw new Error(`arraylist_get() index of bounds size index=${index} size=${arraylist.size}`);

    arraylist.array[index] = item;
}

function arraylist_clear(arraylist) {
    arraylist.size = 0;
    // JS & C# only
    arraylist.array.fill(null);
}

function arraylist_trim(arraylist) {
    if (arraylist.size == arraylist.length) return arraylist.size;

    if (arraylist.size < 1)
        arraylist.length = ARRAYLIST_LENGTH_MULTIPLE;
    else
        arraylist.length = arraylist.size;

    arraylist.array = realloc(arraylist.array, arraylist.length);
    if (!arraylist.array)
        throw new Error("arraylist_trim() out of memory length=" + arraylist.length);

    return arraylist.size;
}

function arraylist_peek_array(arraylist) {
    return arraylist.array;
}

function arraylist_index_of(arraylist, item) {
    for (let i = 0; i < arraylist.size; i++) {
        if (arraylist.array[i] == item)
            return i;
    }
    return -1;
}

function arraylist_has(arraylist, item) {
    for (let i = 0; i < arraylist.size; i++) {
        if (arraylist.array[i] == item)
            return 1;
    }
    return 0;
}

function arraylist_remove(arraylist, item) {
    let count = 0;
    let j = 0;

    for (let i = 0; i < arraylist.size; i++) {
        if (arraylist.array[i] == item) {
            count++;
            continue;
        }
        if (j != i) arraylist.array[j] = arraylist.array[i];
        j++;
    }

    arraylist.size = j;
    return count;
}

function arraylist_remove_at(arraylist, index) {
    if (index < 0 || index >= arraylist.size) return 0;

    // JS only
    let j = index;
    for (let i = index + 1; i < arraylist.size; i++, j++) {
        arraylist.array[j] = arraylist.array[i];
    }

    arraylist.size = j;
    return 1;

    /*
    
    // dreamcast version
    index *= sizeof(void*);
    arraylist->size--;
    int32 amount = arraylist->size * sizeof(void*);
    memmove(arraylist->array + index, arraylist->array + index + sizeof(void*), amount);
    
    return 1;

    */
}

function arraylist_cut_size(arraylist, new_size) {
    if (new_size < arraylist.size) arraylist.size = Math.max(new_size, 0);
}

function arraylist_clone(arraylist) {
    let copy = arraylist_init2(arraylist.size);
    copy.size = arraylist.size;

    for (let i = 0; i < arraylist.size; i++)
        copy.array[i] = clone_object(arraylist.array[i]);

    return copy;
}

function arraylist_sort(arraylist, sort_fn) {
    qsort(arraylist.array, arraylist.size, NaN, sort_fn);
}



/** @deprecated */
function arraylist_iterate_prepare(arraylist) {
    arraylist.iterate_index = 0;
}

/** @deprecated */
function* arraylist_iterate(arraylist) {
    for (; arraylist.iterate_index < arraylist.size; arraylist.iterate_index++)
        yield arraylist.array[arraylist.iterate_index];
}

function* arraylist_iterate4(arraylist) {
    for (let i = 0; i < arraylist.size; i++)
        yield arraylist.array[i];
}

