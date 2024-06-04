"use strict";

function linkedlist_init() {
    return {
        head: null,
        tail: null,
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

function linkedlist_add_item(linkedlist, item) {
    let node = { next: null, item: item };

    if (!linkedlist.head) {
        linkedlist.head = node;
    } else {
        linkedlist.tail.next = node;
    }

    linkedlist.tail = node;
    linkedlist.count++;

    return linkedlist.count;
}

function linkedlist_add_item_unique(linkedlist, item) {
    if (linkedlist_has_item(linkedlist, item)) return -1;
    return linkedlist_add_item(linkedlist, item);
}

function linkedlist_has_item(linkedlist, item) {
    let current = linkedlist.head;
    while (current) {
        if (current.item == item) return true;
        current = current.next;
    }
    return false;
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
    // Dreamcast only
    /* uint8_t* array = malloc(element_size * linkedlist->count);
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
    */

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

    linkedlist.count = 0;
    return;
    */

    // JS only
    linkedlist.head = null;
    linkedlist.tail = null;
    linkedlist.count = 0;
}


function linkedlist_count(linkedlist) {
    return linkedlist.count;
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

function linkedlist_index_of(linkedlist, item) {
    let index = 0;
    let current = linkedlist.head;
    while (current) {
        if (current.item == item) return index;
        current = current.next;
        index++;
    }
    return -1;
}


function linkedlist_set_item(linkedlist, index, item) {
    if (index < 0 || index >= linkedlist.count) throw new Error("index is out-of-range");

    let i = 0;
    let current = linkedlist.head;
    while (current) {
        if (i == index) {
            current.item = item;
            return;
        }
        current = current.next;
        i++;
    }
}


function linkedlist_remove_item(linkedlist, item) {
    let i = 0;
    for (let current_item of linkedlist_iterate4(linkedlist)) {
        if (current_item == item) {
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
