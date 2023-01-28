#include <assert.h>

#include "../ffgraph.h"

void buffering_clear(Buffering* buffering);


Buffering* buffering_init() {
    Buffering* buffering = malloc(sizeof(Buffering));
    buffering->head = NULL;
    buffering->tail = NULL;

    return buffering;
}

void buffering_destroy(Buffering** buffering) {
    buffering_clear(*buffering);
    free(*buffering);
    buffering = NULL;
}

void buffering_add(Buffering* buffering, void* data, int data_size, int64_t seconds) {
    if (seconds < 0 || !data || data_size < 1) return;

    void* data_copy = malloc(data_size);
    assert(data_copy);
    memcpy(data_copy, data, data_size);

    BufferNode* node = malloc(sizeof(BufferNode));
    node->data = data_copy;
    node->data_size = data_size;
    node->seconds = seconds;
    node->next_node = NULL;

    if (buffering->head) {
        buffering->tail->next_node = node;
    } else {
        buffering->head = node;
    }

    buffering->tail = node;
}

bool buffering_get(Buffering* buffering, void** data, int* data_size, int64_t* seconds) {
    if (!buffering->head) return false;

    BufferNode* node = buffering->head;
    BufferNode* next_node = node->next_node;

    *data = node->data;
    *data_size = node->data_size;
    *seconds = node->seconds;

    if (!next_node /* || buffering->head == buffering->tail*/) {
        buffering->tail = NULL;
    }

    buffering->head = next_node;
    free(node);

    return true;
}

void buffering_free_data(void* data) {
    /*if (data) */
    free(data);
}

void buffering_clear(Buffering* buffering) {
    BufferNode* node = buffering->head;

    while (node) {
        node = node->next_node;
        free(node->data);
        free(node);
    }

    buffering->head = buffering->tail = NULL;
}

