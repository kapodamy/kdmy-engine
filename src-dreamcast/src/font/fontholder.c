#include "fontholder.h"

#include "fontglyph.h"
#include "fonttype.h"
#include "malloc_utils.h"
#include "stringutils.h"


FontHolder fontholder_init(const char* src, float default_size_px, const char* glyph_suffix, bool color_by_addition) {
    void* font_instance;
    bool is_altas_type = string_lowercase_ends_with(src, ".xml");

    if (is_altas_type)
        font_instance = fontglyph_init(src, glyph_suffix, true);
    else
        font_instance = fonttype_init(src);

    return fontholder_init2(font_instance, is_altas_type, default_size_px, color_by_addition);
}

FontHolder fontholder_init2(void* font_instance, bool is_altas_type, float default_size_px, bool color_by_addition) {
    FontHolder fontholder = malloc_chk(sizeof(struct FontHolder_s));
    malloc_assert(fontholder, FontHolder);

    *fontholder = (struct FontHolder_s){
        .font = font_instance,
        .font_from_atlas = is_altas_type,
        .font_size = default_size_px > 0.0f ? default_size_px : FONTHOLDER_DEFAULT_SIZE_PIXELS,
        .font_color_by_addition = color_by_addition
    };

    return fontholder;
}

void fontholder_destroy(FontHolder* fontholder_ptr) {
    FontHolder fontholder = *fontholder_ptr;
    if (!fontholder) return;

    if (fontholder->font_from_atlas)
        fontglyph_destroy((FontGlyph*)&fontholder->font);
    else
        fonttype_destroy((FontType*)&fontholder->font);

    free_chk(fontholder);
    *fontholder_ptr = NULL;
}
