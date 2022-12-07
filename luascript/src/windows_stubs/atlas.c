#include "commons.h"
#include "atlas.h"

static Atlas_t atlas = {};

Atlas atlas_init(const char* src) {
    print_stub("atlas_init", "src=%s", src);
    return &atlas;
}
void atlas_destroy(Atlas* atlas) {
    print_stub("atlas_destroy", "atlas=%p", atlas);
}
int32_t atlas_get_index_of(Atlas atlas, const char* name) {
    print_stub("atlas_get_index_of", "atlas=%p name=%s", atlas, name);
    return 0;
}
AtlasEntry atlas_get_entry(Atlas atlas, const char* name) {
    print_stub("atlas_get_entry", "atlas=%p name=%s", atlas, name);
    return NULL;
}
AtlasEntry atlas_get_entry_with_number_suffix(Atlas atlas, const char* name_prefix) {
    print_stub("atlas_get_entry_with_number_suffix", "atlas=%p name_prefix=%s", atlas, name_prefix);
    return NULL;
}
float atlas_get_glyph_fps(Atlas atlas) {
    print_stub("atlas_get_glyph_fps", "atlas=%p", atlas);
    return 0;
}
void atlas_get_texture_resolution(Atlas atlas, int32_t* resolution_width, int32_t* resolution_height) {
    print_stub("atlas_get_texture_resolution", "atlas=%p resolution_width=%p resolution_height=%p", atlas, resolution_width, resolution_height);
}
bool atlas_utils_is_known_extension(const char* src) {
    print_stub("atlas_utils_is_known_extension", "src=%s", src);
    return 0;
}
