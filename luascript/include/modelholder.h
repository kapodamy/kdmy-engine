#ifndef __modelholder_h
#define __modelholder_h

#include "animlist.h"
#include "animsprite.h"
#include "atlas.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct _ModelHolder_t {
    int dummy;
} ModelHolder_t;

typedef ModelHolder_t* ModelHolder;


ModelHolder modelholder_init(const char* src);
ModelHolder modelholder_init2(uint32_t vertex_color_rgb8, const char* atlas_src, const char* animlist_src);
void modelholder_destroy(ModelHolder* modelholder);
bool modelholder_is_invalid(ModelHolder modelholder);
bool modelholder_has_animlist(ModelHolder modelholder);
AnimSprite modelholder_create_animsprite(ModelHolder modelholder, const char* animation_name, bool fallback_static, bool no_return_null);
Atlas modelholder_get_atlas(ModelHolder modelholder);
uint32_t modelholder_get_vertex_color(ModelHolder modelholder);
AnimList modelholder_get_animlist(ModelHolder modelholder);
AtlasEntry modelholder_get_atlas_entry(ModelHolder modelholder, const char* atlas_entry_name, bool return_copy);
AtlasEntry modelholder_get_atlas_entry2(ModelHolder modelholder, const char* atlas_entry_name, bool return_copy);
void modelholder_get_texture_resolution(ModelHolder modelholder, int32_t* resolution_width, int32_t* resolution_height);
bool modelholder_utils_is_known_extension(const char* filename);

#endif
