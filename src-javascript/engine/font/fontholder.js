"use strict";

const FONTHOLDER_DEFAULT_SIZE_PIXELS = 8;// in pixels


async function fontholder_init(src, default_size_px, glyph_suffix) {
    let font_instance;
    let is_altas_type = string_lowercase_ends_with(src, ".xml");

    if (is_altas_type)
        font_instance = await fontglyph_init(src, glyph_suffix, true);
    else
        font_instance = await fonttype_init(src);

    return fontholder_init2(font_instance, is_altas_type, default_size_px);
}

function fontholder_init2(font_instance, is_altas_type, default_size_px) {
    return {
        font: font_instance,
        font_from_atlas: is_altas_type,
        font_size: default_size_px > 0.0 ? default_size_px : FONTHOLDER_DEFAULT_SIZE_PIXELS
    };
}

function fontholder_destroy(fontholder) {
    if (fontholder.font_from_atlas)
        fontglyph_destroy(fontholder.font);
    else
        fonttype_destroy(fontholder.font);

    fontholder = undefined;
}
