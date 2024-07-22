"use strict";

class NotImplementedError extends Error {
    constructor(message) {
        super(message);
        this.name = "NotImplementedError";
    }
}

class NaNArgumentError extends Error {
    constructor(message) {
        super(message);
        this.name = "NaNArgumentError";
    }
}


// C STUB
/**
 * Clones a struct
 * @param {any} struct_ptr pointer to the struct to clone
 * @param {number} struct_size size of the struct in bytes (a.k.a "sizeof(struct)")
 * @returns pointer to an allocated memory with the cloned struct data
 */
function clone_struct(struct_ptr, struct_size = NaN) {
    if (struct_ptr == null) return null;
    if (struct_ptr instanceof Array) new TypeError("(javacript) clone_struct called with array");
    let new_struct = {};
    for (const field in struct_ptr) new_struct[field] = struct_ptr[field];
    return new_struct;
}

// C STUB
/**
 * Clones an array of structs
 * @param {any[]} array_ptr pointer to the array to clone
 * @param {number} elements amount of elements on the array
 * @param {number} struct_size size of each element (a.k.a "sizeof(struct)")
 * @returns pointer to an allocated memory with the cloned array data
 */
function clone_array(array_ptr, elements, struct_size = NaN) {
    if (elements < 1 || !array_ptr) return null;
    if (!array_ptr) throw new Error("array_ptr is null");

    let array = new Array(elements);
    for (let i = 0; i < array.length; i++)  array[i] = clone_object(array_ptr[i]);
    return array;
}

// C STUB
/**
 * Clone a struct on every array entry
 * @param {array[any]} array_ptr pointer to array
 * @param {number} elements amount of elements on the array to set
 * @param {any} struct_ptr pointer to the struct to clone
 * @param {number?} struct_size size of the struct in bytes
 * @returns {boolean} returns false if the array was null
 */
function clone_struct_as_array_items(array_ptr, elements, struct_ptr, struct_size = NaN) {
    if (array_ptr == null) return false;
    if (!(array_ptr instanceof Array))
        new TypeError("(javacript) clone_struct_as_array_items expected array");

    for (let i = 0; i < elements; i++)
        array_ptr[i] = clone_struct(struct_ptr, struct_size);

    return true;
}

/**
 * Copy the contents of struct into another
 * @param {object} struct_src source struct pointer
 * @param {object} struct_dest  destination struct pointer
 * @param {number} struct_size the size of the struct
 * @returns {boolean} false if one of the struct was null
 */
function clone_struct_to(struct_src, struct_dest, struct_size = NaN) {
    if (struct_src instanceof Array)
        new TypeError("(javacript) clone_struct_to struct_src was an array");

    if (struct_dest instanceof Array)
        new TypeError("(javacript) clone_struct_to struct_dest was an array");

    if (struct_src === struct_dest) {
        console.warn('clone_struct_to() "struct_src" and "struct_dest" are the same');
        return false;
    }

    if (struct_src == null || struct_dest == null) return false;

    for (const prop in struct_src) {
        struct_dest[prop] = struct_src[prop];
    }

    return true;
}

/**
 * @param {T[]} array 
 * @param {number} size 
 * @template T
 * @returns T[]
 */
function realloc_for_array(array, size) {
    if (array == null)
        return new Array(size);
    else if (size > 0)
        return realloc(array, size);
    else if (size < 0)
        throw new Error("size was negative");
    else
        return null;
}

/**
 * @param {number} size
 */
function malloc_for_array(size) {
    if (size > 0)
        return new Array(size);
    else if (size < 0)
        throw new Error("size was negative");
    else
        return null;
}
