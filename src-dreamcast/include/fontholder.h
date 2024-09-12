#ifndef _fontholder_h
#define _fontholder_h

#include <stdbool.h>
#include <stdint.h>


#define FONTHOLDER_DEFAULT_SIZE_PIXELS 8 // in pixels


typedef struct FontHolder_s {
    void* font;
    bool font_from_atlas;
    float font_size;
    bool font_color_by_addition;
}* FontHolder;


FontHolder fontholder_init(const char* src, float default_size_px, const char* glyph_suffix, bool color_by_addition);
FontHolder fontholder_init2(void* font_instance, bool is_altas_type, float default_size_px, bool color_by_addition);
void fontholder_destroy(FontHolder* fontholder);

#endif
