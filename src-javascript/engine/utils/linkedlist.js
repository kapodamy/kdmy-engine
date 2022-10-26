"use strict";

function linkedlist_init() {
    return {
        tail: null,
        head: null,
        count: 0
    };
}

/**
 * Destroys this linkedlist instance
 * @param {*} linkedlist the linkedlist pointer
 */
function linkedlist_destroy(linkedlist) {
    linkedlist_destroy2(linkedlist, null);
}

/**
 * Destroys this linkedlist instance and release the items using a specified callback function
 * @param {object} linkedlist the linkedlist pointer
 * @param {function} release_function if specified, executes the function on each non-null item 
 */
function linkedlist_destroy2(linkedlist, release_function) {
    // Dreamcast only
    /*
    let current = linkedlist.head;
    while (current) {
        if (release_function && current.item != null) release_function(current.item);
        let next = current.next;
        free(current);
        current = next;
    }
    */

    // JS only
    linkedlist.head = null;
    linkedlist.tail = null;
    linkedlist.count = 0;

    linkedlist = undefined;
}

function linkedlist_add_item(linkedlist, obj) {
    let item = { next: null, item: obj };

    if (!linkedlist.head) {
        linkedlist.head = item;
    } else {
        linkedlist.tail.next = item;
    }

    linkedlist.tail = item;
    linkedlist.count++;

    return linkedlist.count;
}

function linkedlist_add_item_unique(linkedlist, obj) {
    if (linkedlist_has_item(linkedlist, obj)) return -1;
    return linkedlist_add_item(linkedlist, obj);
}

function linkedlist_has_item(linkedlist, obj) {
    let current = linkedlist.head;
    while (current) {
        if (current.item == obj) return 1;
        current = current.next;
    }
    return 0;
}

function linkedlist_to_array(linkedlist) {
    if (linkedlist.count < 1) return null;

    let i = 0;
    let array = new Array(linkedlist.count);
    let current = linkedlist.head;

    while (current) {
        array[i++] = current.item;
        current = current.next;
    }

    return array;
}

function linkedlist_to_solid_array(linkedlist, element_size = NaN) {
	if (linkedlist.count < 1) return null;
    let array = linkedlist_to_array(linkedlist);
    for (let i = 0; i < array.length; i++) array[i] = clone_object(array[i]);
    return array;
}

function linkedlist_clear(linkedlist, release_function) {
    // Dreamcast only
    /*
    let current = linkedlist.head;
    while (current) {
        if (release_function && current.item != null) release_function(current.item);
        let next = current.next;
        free(current);
        current = next;
    }
    */

    // JS only
    linkedlist.head = null;
    linkedlist.tail = null;
    linkedlist.count = 0;
}


function linkedlist_count(linkedlist) {
    return linkedlist.count;
}

/**
 * @deprecated use linkedlist_iterate3() instead
 */
function linkedlist_iterate(linkedlist, linkedlist_iterator) {
    if (!linkedlist_iterator.init) {
        linkedlist_iterator.init = 1;
        linkedlist_iterator.has_next = 1;
        linkedlist_iterator.next = linkedlist.head;
    }

    if (linkedlist_iterator.next == null) {
        linkedlist_iterator.has_next = 0;
        return null;
    }

    let item = linkedlist_iterator.next.item;
    linkedlist_iterator.next = linkedlist_iterator.next.next;
    linkedlist_iterator.value = item;

    return item;
}

/**
 * @deprecated use linkedlist_iterate3() instead
 */
function linkedlist_iterate2(linkedlist, linkedlist_iterator) {
    if (!linkedlist_iterator.init) {
        linkedlist_iterator.init = 1;
        linkedlist_iterator.has_next = 1;
        linkedlist_iterator.next = linkedlist.head;
    } else if (!linkedlist_iterator.has_next) {
        return 0;
    }

    if (linkedlist_iterator.next) {
        linkedlist_iterator.item = linkedlist_iterator.next.item;
        linkedlist_iterator.next = linkedlist_iterator.next.next;
    } else {
        linkedlist_iterator.has_next = 0;
    }

    return linkedlist_iterator.has_next;
}

function linkedlist_iterate3(linkedlist, linkedlist_iterator) {
    if (!linkedlist_iterator.init) {
        linkedlist_iterator.init = 1;
        linkedlist_iterator.next_entry = linkedlist.head;
    } else if (!linkedlist_iterator.next_entry) {
        return 0;
    }

    if (linkedlist_iterator.next_entry) {
        linkedlist_iterator.item = linkedlist_iterator.next_entry.item;
        linkedlist_iterator.next_entry = linkedlist_iterator.next_entry.next;
        return 1;
    }

    //linkedlist_iterator.item = null;
    return 0;
}

function linkedlist_iterator_prepare(linkedlist_iterator) {
    linkedlist_iterator.init = 0;
}

function* linkedlist_iterate4(linkedlist) {
    let current = linkedlist.head;
    while (current) {
        let item = current.item;
        current = current.next;
        yield item;
    }
}

function linkedlist_get_first_item(linkedlist) {
    if (linkedlist.head)
        return linkedlist.head.item;
    else
        return null;
}

function linkedlist_get_last_item(linkedlist) {
    if (linkedlist.tail)
        return linkedlist.tail.item;
    else
        return null;
}

function linkedlist_get_by_index(linkedlist, index) {
    if (index < 0 || index >= linkedlist.count) return null;

    let current = linkedlist.head;

    for (let i = 0; i < index && current; i++) current = current.next;

    return current ? current.item : null;
}

function linkedlist_index_of(linkedlist, obj) {
    let index = 0;
    let current = linkedlist.head;
    while (current) {
        if (current.item == obj) return index;
        current = current.next;
        index++;
    }
    return -1;
}


function linkedlist_remove_item(linkedlist, item) {
    let iterator = { item: null };
    let i = 0;

    while (linkedlist_iterate3(linkedlist, iterator)) {
        if (iterator.item == item) {
            linkedlist_remove_item_at(linkedlist, i);
            return i;
        }
        i++;
    }
}

function linkedlist_remove_item_at(linkedlist, index) {
    if (linkedlist.count < 1) return null;

    console.assert(index >= 0 && index < linkedlist.count, "index >= 0 && index < linkedlist.count");

    let current, item;

    if (linkedlist.count == 1) {
        item = linkedlist.head.item;
        linkedlist.head = undefined;
        linkedlist.head = null;
        linkedlist.tail = null;
        linkedlist.count = 0;
        return item;
    }

    if (index == 0) {
        current = linkedlist.head.next;
        item = linkedlist.head.item;

        linkedlist.head = undefined;

        linkedlist.head = current;
        linkedlist.count--;
        return item;
    }


    let i = 1;
    let previous = linkedlist.head;
    current = linkedlist.head.next;

    while (current) {
        if (index == i) {
            previous.next = current.next;

            if (linkedlist.tail == current)
                linkedlist.tail = previous;

            linkedlist.count--;
            item = current.item;

            current = undefined;
            return item;
        }

        i++;
        previous = current;
        current = current.next;
    }

    throw new Error("linkedlist is corrupted or in a invalid state");
}
