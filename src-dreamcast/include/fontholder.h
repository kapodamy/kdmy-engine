#ifndef _fontholder_h
#define _fontholder_h

#include <stdbool.h>
#include <stdint.h>


typedef struct FontHolder_s {
    void* font;
    bool font_from_atlas;
    bool font_color_by_addition;
}* FontHolder;


FontHolder fontholder_init(const char* src, const char* glyph_suffix, bool color_by_addition);
FontHolder fontholder_init2(void* font_instance, bool is_altas_type, bool color_by_addition);
void fontholder_destroy(FontHolder* fontholder);

#endif
