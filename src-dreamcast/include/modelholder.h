#ifndef _modelholder_h
#define _modelholder_h

#include "animlist.h"
#include "animsprite.h"
#include "atlas.h"
#include "sprite.h"


typedef struct ModelHolder_s* ModelHolder;


ModelHolder modelholder_init(const char* src);
ModelHolder modelholder_init2(uint32_t vertex_color_rgb8, const char* atlas_src, const char* animlist_src);
ModelHolder modelholder_init3(uint32_t vertex_color_rgb8, const Texture texture, const Atlas atlas, const AnimList animlist);

void modelholder_destroy(ModelHolder* modelholder);

bool modelholder_is_invalid(ModelHolder modelholder);
bool modelholder_has_animlist(ModelHolder modelholder);
Atlas modelholder_get_atlas(ModelHolder modelholder);
Texture modelholder_get_texture(ModelHolder modelholder, bool increment_reference);
uint32_t modelholder_get_vertex_color(ModelHolder modelholder);
AnimList modelholder_get_animlist(ModelHolder modelholder);
Sprite modelholder_create_sprite(ModelHolder modelholder, const char* atlas_entry_name, bool with_animation);
AnimSprite modelholder_create_animsprite(ModelHolder modelholder, const char* animation_name, bool fallback_static, bool no_return_null);
const AtlasEntry* modelholder_get_atlas_entry(ModelHolder modelholder, const char* atlas_entry_name);
const AtlasEntry* modelholder_get_atlas_entry2(ModelHolder modelholder, const char* atlas_entry_name);
bool modelholder_get_texture_resolution(ModelHolder modelholder, int32_t* resolution_width, int32_t* resolution_height);
bool modelholder_utils_is_known_extension(const char* filename);


#endif
