#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "malloc_utils.h"
#include "map.h"

#define MAP_DEAFULT_CAPACITY 16


typedef struct {
    int32_t id;
    void* obj;
} MapEntry;

struct Map_s {
    uint32_t capacity;
    MapEntry* table;
};


Map map_init() {
    Map map = malloc_chk(sizeof(struct Map_s));
    malloc_assert(map, Map);

    map->capacity = MAP_DEAFULT_CAPACITY;
    map->table = malloc_for_array(MapEntry, MAP_DEAFULT_CAPACITY);

    MapEntry* table = map->table;
    for (uint32_t i = 0; i < map->capacity; i++) {
        table->id = -1;
        table->obj = NULL;
        table++;
    }

    return map;
}

void map_destroy(Map* map) {
    if (!*map) return;
    free_chk((*map)->table);
    free_chk(*map);
    *map = NULL;
}


void map_add(Map map, int id, void* obj) {
    assert(id >= 0);

    MapEntry* table = map->table;
    for (uint32_t i = 0; i < map->capacity; i++) {
        if (table->id < 0) {
            table->id = id;
            table->obj = obj;
            return;
        }
        table++;
    }

    uint32_t new_capacity = map->capacity + MAP_DEAFULT_CAPACITY;
    MapEntry* new_table = realloc_for_array(map->table, new_capacity, MapEntry);

    table = new_table + map->capacity;
    table->id = id;
    table->obj = obj;
    table++;

    for (uint32_t i = map->capacity + 1; i < new_capacity; i++) {
        table->id = -1;
        table->obj = NULL;
        table++;
    }

    map->capacity = new_capacity;
    map->table = new_table;
}

void map_delete(Map map, int id) {
    assert(id >= 0);

    MapEntry* table = map->table;
    for (uint32_t i = 0; i < map->capacity; i++) {
        if (table->id == id) {
            table->id = -1;
            table->obj = NULL;
            return;
        }
        table++;
    }
}

bool map_has(Map map, int id) {
    assert(id >= 0);

    MapEntry* table = map->table;
    for (uint32_t i = 0; i < map->capacity; i++) {
        if (table->id == id) {
            return true;
        }
        table++;
    }

    return false;
}

void map_delete2(Map map, void* obj) {
    assert(obj);

    MapEntry* table = map->table;
    for (uint32_t i = 0; i < map->capacity; i++) {
        if (table->obj == obj) {
            table->id = -1;
            table->obj = NULL;
            return;
        }
        table++;
    }
}


MapIterator map_get_iterator(Map map) {
    size_t count = 0;
    MapEntry* table = map->table;
    for (uint32_t i = 0; i < map->capacity; i++) {
        if (table->id != -1) {
            count++;
        }
        table++;
    }

    return (MapIterator){
        .__run = count > 0,
        .map = map,
        .index = 0
    };
}

bool map_iterate(MapIterator* iterator, void** value) {
    uint32_t index = iterator->index;
    uint32_t capacity = iterator->map->capacity;
    MapEntry* table = iterator->map->table + index;

    while (index < capacity) {
        index++;

        if (table->id == -1) {
            table++;
            continue;
        }

        *value = table->obj;
        iterator->index = index;
        return true;
    }

    return false;
}
