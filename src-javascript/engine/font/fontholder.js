"use strict";


async function fontholder_init(src, glyph_suffix, color_by_addition) {
    let font_instance;
    let is_altas_type = string_lowercase_ends_with(src, ".xml");

    if (is_altas_type)
        font_instance = await fontglyph_init(src, glyph_suffix, true);
    else
        font_instance = await fonttype_init(src);

    return fontholder_init2(font_instance, is_altas_type, color_by_addition);
}

function fontholder_init2(font_instance, is_altas_type, color_by_addition) {
    return {
        font: font_instance,
        font_from_atlas: is_altas_type,
        font_color_by_addition: color_by_addition
    };
}

function fontholder_destroy(fontholder) {
    if (fontholder.font_from_atlas)
        fontglyph_destroy(fontholder.font);
    else
        fonttype_destroy(fontholder.font);

    fontholder = undefined;
}
