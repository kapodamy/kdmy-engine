"use strict";

async function atlas_init(src) {
    if (string_lowercase_ends_with(src, ".txt")) {
        return atlas_parse_from_plain_text(src);
    }

    let atlas = {};
    let xml;

    try {
        xml = await fs_readxml(src);
    } catch (e) {
        if (e instanceof KDMYEngineIOError) {
            console.error(`atlas_init() error loading "${src}"`, e);
            return null;
        }

        // malformed/corrupt xml file
        throw e;
    }

    let xml_atlas = xml.querySelector("TextureAtlas");
    console.assert(xml_atlas != null, xml);

    atlas.size = 0;
    atlas.entries = null;
    //atlas.name = src;

    atlas.glyph_fps = parseFloat(xml_atlas.getAttribute("glyphFps"));
    if (Number.isNaN(atlas.glyph_fps)) atlas.glyph_fps = 0.0;

    atlas.texture_filename = xml_atlas.getAttribute("imagePath");
    atlas.resolution_width = FUNKIN_SCREEN_RESOLUTION_WIDTH;
    atlas.resolution_height = FUNKIN_SCREEN_RESOLUTION_HEIGHT;
    atlas.has_declared_resolution = atlas_parse_resolution(atlas, xml_atlas.getAttribute("resolution"));

    if (atlas.texture_filename != null) {
        if (atlas.texture_filename.length > 0) {
            let path = fs_build_path2(src, atlas.texture_filename);
            atlas.texture_filename = path;
        } else {
            atlas.texture_filename = null;
        }
    }

    let arraylist = arraylist_init();

    for (let unparsed_entry of xml_atlas.children) {
        switch (unparsed_entry.tagName) {
            case "TileSet":
                atlas_parse_tileset(arraylist, unparsed_entry);
                continue;
            case "SubTexture":
                break;
            default:
                console.warn("atlas_init() unknown TextureAtlas entry: " + unparsed_entry.outerHTML);
                continue;
        }

        const atlas_entry = {
            name: unparsed_entry.getAttribute("name"),
            x: Number.parseInt(unparsed_entry.getAttribute("x")),
            y: Number.parseInt(unparsed_entry.getAttribute("y")),
            width: Number.parseInt(unparsed_entry.getAttribute("width")),
            height: Number.parseInt(unparsed_entry.getAttribute("height")),
            frame_x: Number.parseInt(unparsed_entry.getAttribute("frameX")),
            frame_y: Number.parseInt(unparsed_entry.getAttribute("frameY")),
            frame_width: Number.parseInt(unparsed_entry.getAttribute("frameWidth")),
            frame_height: Number.parseInt(unparsed_entry.getAttribute("frameHeight")),
            pivot_x: 0.0,// Number.parseFloat(unparsed_entry.getAttribute("pivotX")),
            pivot_y: 0.0,// Number.parseFloat(unparsed_entry.getAttribute("pivotY")),
        };

        console.assert(!Number.isNaN(atlas_entry.width) && !Number.isNaN(atlas_entry.height), atlas_entry);

        if (Number.isNaN(atlas_entry.frame_x)) atlas_entry.frame_x = 0;
        if (Number.isNaN(atlas_entry.frame_y)) atlas_entry.frame_y = 0;
        if (Number.isNaN(atlas_entry.frame_width)) atlas_entry.frame_width = 0;
        if (Number.isNaN(atlas_entry.frame_height)) atlas_entry.frame_height = 0;
        if (Number.isNaN(atlas_entry.pivot_x)) atlas_entry.pivot_x = 0.0;
        if (Number.isNaN(atlas_entry.pivot_y)) atlas_entry.pivot_y = 0.0;

        arraylist_add(arraylist, atlas_entry);
    }

    xml = undefined;
    arraylist_destroy2(arraylist, atlas, "size", "entries");

    return atlas;
}

function atlas_destroy(atlas) {
    for (let i = 0; i < atlas.size; i++) {
        atlas.entries[i].name = undefined;
    }

    atlas.entries = undefined;
    atlas.size = 0;
    atlas.texture_filename = undefined;
    atlas = undefined;
}

function atlas_get_index_of(atlas, name) {
    for (let i = 0; i < atlas.size; i++) {
        if (atlas.entries[i].name == name)
            return i;
    }

    return -1;
}

function atlas_get_entry(atlas, name) {
    let index = atlas_get_index_of(atlas, name);

    return index >= 0 ? atlas.entries[index] : null;
}

function atlas_get_entry_with_number_suffix(atlas, name_prefix) {
    let start = name_prefix.length;

    for (let i = 0; i < atlas.size; i++) {
        let atlas_entry_name = atlas.entries[i].name;

        if (
            atlas_entry_name.startsWith(name_prefix) &&
            atlas_name_has_number_suffix(atlas_entry_name, start)
        )
            return atlas.entries[i];
    }

    return null;
}

function atlas_get_glyph_fps(atlas) {
    return atlas.glyph_fps;
}

function atlas_get_texture_path(atlas) {
    return atlas.texture_filename;
}

function atlas_apply(atlas, sprite, name, override_draw_size) {
    let i = atlas_get_index_of(atlas, name);
    if (i < 0) return true;

    atlas_apply_from_entry(sprite, atlas.entries[i], override_draw_size);
    return false;
}

function atlas_apply_from_entry(sprite, atlas_entry, override_draw_size) {
    sprite_set_offset_source(
        sprite,
        atlas_entry.x, atlas_entry.y,
        atlas_entry.width, atlas_entry.height
    );
    sprite_set_offset_frame(
        sprite,
        atlas_entry.frame_x, atlas_entry.frame_y,
        atlas_entry.frame_width, atlas_entry.frame_height
    );
    sprite_set_offset_pivot(
        sprite,
        atlas_entry.pivot_x,
        atlas_entry.pivot_y
    );

    if (override_draw_size) {
        let width = atlas_entry.frame_width > 0 ? atlas_entry.frame_width : atlas_entry.width;
        let height = atlas_entry.frame_height > 0 ? atlas_entry.frame_height : atlas_entry.height;
        sprite_set_draw_size(sprite, width, height);
    }
}

function atlas_name_has_number_suffix(atlas_entry_name, start_index) {
    let string_length = atlas_entry_name.length;
    let ignore_space = true;

    if (start_index >= string_length) return false;

    for (let j = start_index; j < string_length; j++) {
        let code = atlas_entry_name.charCodeAt(j);

        if (code < 0x30 || code > 0x39) {
            if (ignore_space) {
                switch (code) {
                    case 0x09:// tabulation
                    case 0x20:// white space
                    //case 0xff:// hard space
                        // case 0x5F:// underscore (used in plain-text atlas)
                        ignore_space = false;
                        continue;
                }
            }

            // the name does not end with numbers
            return false;
        }
    }

    return true;
}

function atlas_get_texture_resolution(atlas, output_resolution) {
    output_resolution[0] = atlas.resolution_width;
    output_resolution[1] = atlas.resolution_height;
    return atlas.has_declared_resolution;
}

function atlas_parse_resolution(atlas, resolution_string) {
    if (resolution_string == null || resolution_string.length < 1) return false;

    let index = -1;
    let length = resolution_string.length;

    for (let i = 0; i < length; i++) {
        let char = resolution_string.charAt(i);
        if (char == 'x' || char == 'X') {
            index = i;
            break;
        }
    }

    if (index < 0) {
        console.error("atlas_parse_resolution() invalid resolution", resolution_string);
        return false;
    }

    let width = vertexprops_parse_unsigned_integer(
        resolution_string.substring(0, index), NaN
    );
    let height = vertexprops_parse_unsigned_integer(
        resolution_string.substring(index + 1, resolution_string.length), NaN
    );

    if (Number.isNaN(width) || Number.isNaN(height)) {
        console.error("atlas_parse_resolution() invalid resolution", resolution_string);
        return false;
    }

    atlas.resolution_width = width;
    atlas.resolution_height = height;
    return true;
}

function atlas_utils_is_known_extension(src) {
    return string_lowercase_ends_with(src, ".xml") || string_lowercase_ends_with(src, ".txt");
}

async function atlas_parse_from_plain_text(src_txt) {
    // create fake path to texture, assume the format is PNG
    let path = fs_get_full_path(src_txt);
    let index = path.lastIndexOf('.');
    let subpath = path.substring(0, index);
    let fake_texture_filename = string_concat(2, subpath, ".png");
    path = undefined;
    subpath = undefined;

    let text = await fs_readtext(src_txt);
    if (!text) {
        fake_texture_filename = undefined;
        return null;
    }

    let atlas = {
        //name : src,
        glyph_fps: 0.0,
        texture_filename: fake_texture_filename,
        resolution_width: FUNKIN_SCREEN_RESOLUTION_WIDTH,
        resolution_height: FUNKIN_SCREEN_RESOLUTION_HEIGHT,
        has_declared_resolution: false,
        size: 0,
        entries: null
    };

    let unparsed_entry, temp;
    let x, y, width, height;
    let tokenizer_entries = tokenizer_init("\r\n", true, false, text);
    let arraylist = arraylist_init2(tokenizer_count_occurrences(tokenizer_entries));

    while ((unparsed_entry = tokenizer_read_next(tokenizer_entries)) != null) {
        let tokenizer_field = tokenizer_init("\x3D", true, false, unparsed_entry);

        temp = tokenizer_read_next(tokenizer_field);
        let name = string_trim(temp, false, true);
        temp = undefined;

        temp = tokenizer_read_next(tokenizer_field);
        let unparsed_coords = string_trim(temp, true, true);
        temp = undefined;

        tokenizer_destroy(tokenizer_field);
        unparsed_entry = undefined;

        if (!name) continue;

        if (!unparsed_coords) {
            name = undefined;
            continue;
        }

        let tokenizer_coords = tokenizer_init("\u00A0\x20\x09", true, false, unparsed_coords);

        temp = tokenizer_read_next(tokenizer_coords);
        x = vertexprops_parse_integer2(temp, MATH2D_MAX_INT32);
        temp = undefined;

        temp = tokenizer_read_next(tokenizer_coords);
        y = vertexprops_parse_integer2(temp, MATH2D_MAX_INT32);
        temp = undefined;

        temp = tokenizer_read_next(tokenizer_coords);
        width = vertexprops_parse_integer2(temp, MATH2D_MAX_INT32);
        temp = undefined;

        temp = tokenizer_read_next(tokenizer_coords);
        height = vertexprops_parse_integer2(temp, MATH2D_MAX_INT32);
        temp = undefined;

        tokenizer_destroy(tokenizer_coords);
        unparsed_coords = undefined;

        L_parse_fields: {
            if (x == MATH2D_MAX_INT32) break L_parse_fields;
            if (y == MATH2D_MAX_INT32) break L_parse_fields;
            if (width == MATH2D_MAX_INT32) break L_parse_fields;
            if (height == MATH2D_MAX_INT32) break L_parse_fields;

            arraylist_add(arraylist, {
                name, x, y, width, height,
                frame_x: 0.0, frame_y: 0.0, frame_width: 0.0, frame_height: 0.0, pivot_x: 0.0, pivot_y: 0.0,
            });

            continue;
        }

        name = undefined;
    }

    text = undefined;
    tokenizer_destroy(tokenizer_entries);

    arraylist_destroy2(arraylist, atlas, "size", "entries");

    temp = fs_get_filename_without_extension(src_txt);
    if (temp != null) {
        atlas.texture_filename = string_concat(2, temp, ".png");
        temp = undefined;
    }

    return atlas;
}

function atlas_parse_tileset(arraylist, unparsed_tileset) {
    let sub_x = Number.parseInt(unparsed_tileset.getAttribute("x"));
    let sub_y = Number.parseInt(unparsed_tileset.getAttribute("y"));
    let sub_width = Number.parseInt(unparsed_tileset.getAttribute("subTextureWidth"));
    let sub_height = Number.parseInt(unparsed_tileset.getAttribute("subTextureHeight"));
    let tile_width = Number.parseInt(unparsed_tileset.getAttribute("tileWidth"));
    let tile_height = Number.parseInt(unparsed_tileset.getAttribute("tileHeight"));

    if (
        Number.isNaN(sub_width) || Number.isNaN(sub_height) ||
        Number.isNaN(tile_width) || Number.isNaN(tile_height)
    ) {
        console.warn("atlas_parse_tileset() missing fields in TileSet: " + unparsed_tileset.outerHTML);
        return;
    }

    if (Number.isNaN(sub_x)) sub_x = 0;
    if (Number.isNaN(sub_y)) sub_y = 0;

    // (JS Only) results must be truncated
    let rows = Math.trunc(sub_width / tile_width);
    let columns = Math.trunc(sub_height / tile_height);

    let index = 0;

    for (let unparsed_tile of unparsed_tileset.children) {
        if (unparsed_tile.tagName != "Tile") {
            console.warn("atlas_parse_tileset() unknown TileSet entry: " + unparsed_tile.outerHTML);
            continue;
        } else if (!unparsed_tile.hasAttribute("name")) {
            console.warn("atlas_parse_tileset() missing tile name: " + unparsed_tile.outerHTML);
            index++;
            continue;
        }

        let tile_index = vertexprops_parse_integer(unparsed_tile, "index", -1);
        if (tile_index < 0) tile_index = index;

        let tile_name = unparsed_tile.getAttribute("name");
        let tile_x = tile_index % rows;
        let tile_y = Math.trunc(tile_index / rows) % columns;

        tile_x *= tile_width;
        tile_y *= tile_height;

        tile_x += sub_x;
        tile_y += sub_y;

        arraylist_add(arraylist, {
            name: tile_name,
            x: tile_x,
            y: tile_y,
            width: tile_width,
            height: tile_height,
            frame_x: 0.0, frame_y: 0.0, frame_width: 0.0, frame_height: 0.0, pivot_x: 0.0, pivot_y: 0.0,
        });

        index++;
    }

}

