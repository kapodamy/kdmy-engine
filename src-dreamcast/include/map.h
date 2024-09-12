#ifndef _map_h
#define _map_h

#include <stdint.h>
#include <stdbool.h>

#include "foreach.h"


struct Map_s;
typedef struct Map_s* Map;

typedef struct {
    bool __run;
    Map map;
    uint32_t index;
} MapIterator;


#define MAP_ITERATOR(INSTANCE, VARIABLE_TYPE, VARIABLE_NAME) foreach__2(VARIABLE_TYPE, VARIABLE_NAME, INSTANCE, MapIterator, map_get_iterator, map_iterate, (void**))

Map map_init();
void map_destroy(Map* map);

void map_add(Map map, int id, void* obj);
void map_delete(Map map, int id);
bool map_has(Map map, int id);
void map_delete2(Map map, void* obj);

MapIterator map_get_iterator(Map map);
bool map_iterate(MapIterator* iterator, void** value);

#endif
