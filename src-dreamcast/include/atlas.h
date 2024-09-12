#ifndef _atlas_h
#define _atlas_h

#include <stdbool.h>
#include <stdint.h>

//  forward reference
typedef struct Sprite_s* Sprite;

typedef struct AtlasEntry_s {
    /** @brief Atlas entry name, this field is only valid during the Atlas lifetime */
    const char* name;
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
} AtlasEntry;


typedef struct Atlas_s {
    int32_t size;
    AtlasEntry* entries;
    float glyph_fps;
    const char* texture_filename;
    int32_t resolution_width;
    int32_t resolution_height;
    bool has_declared_resolution;
    /** @private @brief contains all entries names*/
    void* entries_names;
}* Atlas;


Atlas atlas_init(const char* src);
void atlas_destroy(Atlas* atlas);
int32_t atlas_get_index_of(Atlas atlas, const char* name);
const AtlasEntry* atlas_get_entry(Atlas atlas, const char* name);
const AtlasEntry* atlas_get_entry_with_number_suffix(Atlas atlas, const char* name_prefix);
float atlas_get_glyph_fps(Atlas atlas);
const char* atlas_get_texture_path(Atlas atlas);
bool atlas_apply(Atlas atlas, Sprite sprite, const char* name, bool override_draw_size);
void atlas_apply_from_entry(Sprite sprite, const AtlasEntry* atlas_entry, bool override_draw_size);
bool atlas_name_has_number_suffix(const char* atlas_entry_name, uint32_t start_index);
bool atlas_get_texture_resolution(Atlas atlas, int32_t* out_width, int32_t* out_height);

bool atlas_utils_is_known_extension(const char* src);

#endif