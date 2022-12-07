#ifndef __atlas_h
#define __atlas_h

#include <stdbool.h>
#include <stdint.h>


typedef struct {
    int dummy;
} Atlas_t;

typedef Atlas_t* Atlas;


typedef struct {
    char* name;
    float x;
    float y;
    float width;
    float height;
    float frame_x;
    float frame_y;
    float frame_width;
    float frame_height;
    float pivot_x;
    float pivot_y;
} AtlasEntry_t;

typedef AtlasEntry_t* AtlasEntry;

Atlas atlas_init(const char* src);
void atlas_destroy(Atlas* atlas);
int32_t atlas_get_index_of(Atlas atlas, const char* name);
AtlasEntry atlas_get_entry(Atlas atlas, const char* name);
AtlasEntry atlas_get_entry_with_number_suffix(Atlas atlas, const char* name_prefix);
float atlas_get_glyph_fps(Atlas atlas);
void atlas_get_texture_resolution(Atlas atlas, int32_t* resolution_width, int32_t* resolution_height);
bool atlas_utils_is_known_extension(const char* src);

#endif