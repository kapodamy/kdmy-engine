#include "commons.h"
#include "modelholder.h"

static ModelHolder_t modelholder = {};
static Atlas_t atlas = {};
static AnimSprite_t animsprite = {};
static AnimList_t animlist = {};
static AtlasEntry_t atlasentry = {};


ModelHolder modelholder_init(const char* src) {
    print_stub("modelholder_init", "src=%s", src);
    return &modelholder;
}
ModelHolder modelholder_init2(uint32_t vertex_color_rgb8, const char* atlas_src, const char* animlist_src) {
    print_stub("modelholder_init2", "vertex_color_rgb8=%u atlas_src=%s animlist_src=%s", vertex_color_rgb8, atlas_src, animlist_src);
    return &modelholder;
}
void modelholder_destroy(ModelHolder* modelholder) {
    print_stub("modelholder_destroy", "modelholder=%p", modelholder);
}
bool modelholder_is_invalid(ModelHolder modelholder) {
    print_stub("modelholder_is_invalid", "modelholder=%p", modelholder);
    return 0;
}
bool modelholder_has_animlist(ModelHolder modelholder) {
    print_stub("modelholder_has_animlist", "modelholder=%p", modelholder);
    return 0;
}
AnimSprite modelholder_create_animsprite(ModelHolder modelholder, const char* animation_name, bool fallback_static, bool no_return_null) {
    print_stub("modelholder_create_animsprite", "modelholder=%p animation_name=%s fallback_static=(bool)%i no_return_null=(bool)%i", modelholder, animation_name, fallback_static, no_return_null);
    return &animsprite;
}
Atlas modelholder_get_atlas(ModelHolder modelholder) {
    print_stub("modelholder_get_atlas", "modelholder=%p", modelholder);
    return &atlas;
}
uint32_t modelholder_get_vertex_color(ModelHolder modelholder) {
    print_stub("modelholder_get_vertex_color", "modelholder=%p", modelholder);
    return 0;
}
AnimList modelholder_get_animlist(ModelHolder modelholder) {
    print_stub("modelholder_get_animlist", "modelholder=%p", modelholder);
    return &animlist;
}
AtlasEntry modelholder_get_atlas_entry(ModelHolder modelholder, const char* atlas_entry_name, bool return_copy) {
    print_stub("modelholder_get_atlas_entry", "modelholder=%p atlas_entry_name=%s return_copy=(bool)%i", modelholder, atlas_entry_name, return_copy);
    return &atlasentry;
}
AtlasEntry modelholder_get_atlas_entry2(ModelHolder modelholder, const char* atlas_entry_name, bool return_copy) {
    print_stub("modelholder_get_atlas_entry2", "modelholder=%p atlas_entry_name=%s return_copy=(bool)%i", modelholder, atlas_entry_name, return_copy);
    return &atlasentry;
}
void modelholder_get_texture_resolution(ModelHolder modelholder, int* resolution_width, int* resolution_height) {
    *resolution_width = 123;
    *resolution_height = 456;
    print_stub("modelholder_get_texture_resolution", "modelholder=%p resolution_width=%p resolution_height=%p", modelholder, resolution_width, resolution_height);
}
bool modelholder_utils_is_known_extension(const char* filename) {
    print_stub("modelholder_utils_is_known_extension", "filename=%p", filename);
    return 0;
}
