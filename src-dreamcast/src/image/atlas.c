#include "atlas.h"
#include "arraylist.h"
#include "fs.h"
#include "game/common/funkin.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "sprite.h"
#include "stringbuilder.h"
#include "stringutils.h"
#include "tokenizer.h"
#include "vertexprops.h"
#include "xmlparser.h"


static bool atlas_parse_resolution(Atlas atlas, const char* resolution_string);
static Atlas atlas_parse_from_plain_text(const char* src_txt);
static void atlas_parse_tileset(ArrayList arraylist, XmlNode unparsed_tileset, char** entries_names_ptr);
static char* atlas_internal_get_name(XmlNode unparsed_entry, char** entries_names_ptr);
static char* atlas_internal_get_name2(const char* name, char** entries_names_ptr);


Atlas atlas_init(const char* src) {
    if (string_lowercase_ends_with(src, ".txt")) {
        return atlas_parse_from_plain_text(src);
    }

    Atlas atlas = malloc_chk(sizeof(struct Atlas_s));
    malloc_assert(atlas, Atlas);

    XmlParser xml = xmlparser_init(src);

    if (!xml) {
        logger_error("atlas_init() error loading '%s'", src);
        return NULL;
    }


    XmlNode xml_atlas = xmlparser_get_root(xml);
    assert(xml_atlas && xmlparser_get_tag_name(xml_atlas));

    atlas->size = 0;
    atlas->entries = NULL;
    // atlas->name = src;

    atlas->glyph_fps = vertexprops_parse_float(xml_atlas, "glyphFps", FLOAT_NaN);
    if (!math2d_is_float_finite(atlas->glyph_fps)) atlas->glyph_fps = 0.0f;

    atlas->texture_filename = xmlparser_get_attribute_value2(xml_atlas, "imagePath");
    atlas->resolution_width = FUNKIN_SCREEN_RESOLUTION_WIDTH;
    atlas->resolution_height = FUNKIN_SCREEN_RESOLUTION_HEIGHT;
    atlas->has_declared_resolution = atlas_parse_resolution(atlas, xmlparser_get_attribute_value2(xml_atlas, "resolution"));

    if (atlas->texture_filename != NULL) {
        if (atlas->texture_filename[0] != '\0') {
            char* path = fs_build_path2(src, atlas->texture_filename);
            atlas->texture_filename = path;
        } else {
            atlas->texture_filename = NULL;
        }
    }

    ArrayList arraylist = arraylist_init(sizeof(AtlasEntry));

    // create a string buffer
    size_t string_buffer_size = 0;
    foreach (XmlNode, unparsed_entry, XMLPARSER_CHILDREN_ITERATOR, xml_atlas) {
        const char* tagName = xmlparser_get_tag_name(unparsed_entry);

        if (string_equals(tagName, "TileSet")) {
            foreach (XmlNode, unparsed_tile, XMLPARSER_CHILDREN_ITERATOR, unparsed_entry) {
                const char* name = xmlparser_get_attribute_value2(unparsed_tile, "name");
                string_buffer_size += string_get_bytelength(name);
            }
        } else if (string_equals(tagName, "SubTexture")) {
            const char* name = xmlparser_get_attribute_value2(unparsed_entry, "name");
            string_buffer_size += string_get_bytelength(name);
        }
    }

    char* entries_names_ptr = malloc_for_array(char, string_buffer_size);
    atlas->entries_names = entries_names_ptr;

    foreach (XmlNode, unparsed_entry, XMLPARSER_CHILDREN_ITERATOR, xml_atlas) {
        const char* tagName = xmlparser_get_tag_name(unparsed_entry);

        if (string_equals(tagName, "TileSet")) {
            atlas_parse_tileset(arraylist, unparsed_entry, &entries_names_ptr);
            continue;
        } else if (!string_equals(tagName, "SubTexture")) {
            logger_warn_xml("atlas_init() unknown TextureAtlas entry:", unparsed_entry);
            continue;
        }

        AtlasEntry atlas_entry = {
            .name = atlas_internal_get_name(unparsed_entry, &entries_names_ptr),
            .x = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_entry, "x"), 0),
            .y = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_entry, "y"), 0),
            .width = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_entry, "width"), 0),
            .height = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_entry, "height"), 0),
            .frame_x = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_entry, "frameX"), 0),
            .frame_y = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_entry, "frameY"), 0),
            .frame_width = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_entry, "frameWidth"), 0),
            .frame_height = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_entry, "frameHeight"), 0),
            .pivot_x = 0.0f, // vertexprops_parse_float2(xmlparser_get_attribute_value2(unparsed_entry, "pivotX"), 0.0f),
            .pivot_y = 0.0f, // vertexprops_parse_float2(xmlparser_get_attribute_value2(unparsed_entry, "pivotY"), 0.0f),
        };

        assert(atlas_entry.width >= 0 && atlas_entry.height >= 0);

        arraylist_add(arraylist, &atlas_entry);
    }

    xmlparser_destroy(&xml);
    arraylist_destroy2(&arraylist, &atlas->size, (void**)&atlas->entries);

    return atlas;
}

void atlas_destroy(Atlas* atlas_ptr) {
    Atlas atlas = *atlas_ptr;
    if (!atlas) return;

    free_chk(atlas->entries_names);

    free_chk(atlas->entries);
    atlas->size = 0;
    free_chk((char*)atlas->texture_filename);

    free_chk(atlas);
    *atlas_ptr = NULL;
}

int32_t atlas_get_index_of(Atlas atlas, const char* name) {
    for (uint32_t i = 0; i < atlas->size; i++) {
        if (string_equals(atlas->entries[i].name, name))
            return (int32_t)i;
    }

    return -1;
}

const AtlasEntry* atlas_get_entry(Atlas atlas, const char* name) {
    int32_t index = atlas_get_index_of(atlas, name);

    return index >= 0 ? &atlas->entries[index] : NULL;
}

const AtlasEntry* atlas_get_entry_with_number_suffix(Atlas atlas, const char* name_prefix) {
    size_t start = strlen(name_prefix);

    for (uint32_t i = 0; i < atlas->size; i++) {
        const char* atlas_entry_name = atlas->entries[i].name;

        if (
            string_starts_with(atlas_entry_name, name_prefix) &&
            atlas_name_has_number_suffix(atlas_entry_name, start)
        )
            return &atlas->entries[i];
    }

    return NULL;
}

float atlas_get_glyph_fps(Atlas atlas) {
    return atlas->glyph_fps;
}

const char* atlas_get_texture_path(Atlas atlas) {
    return atlas->texture_filename;
}

bool atlas_apply(Atlas atlas, Sprite sprite, const char* name, bool override_draw_size) {
    int32_t i = atlas_get_index_of(atlas, name);
    if (i < 0) return true;

    atlas_apply_from_entry(sprite, &atlas->entries[i], override_draw_size);
    return false;
}

void atlas_apply_from_entry(Sprite sprite, const AtlasEntry* atlas_entry, bool override_draw_size) {
    sprite_set_offset_source(
        sprite,
        atlas_entry->x, atlas_entry->y,
        atlas_entry->width, atlas_entry->height
    );
    sprite_set_offset_frame(
        sprite,
        atlas_entry->frame_x, atlas_entry->frame_y,
        atlas_entry->frame_width, atlas_entry->frame_height
    );
    sprite_set_offset_pivot(
        sprite,
        atlas_entry->pivot_x,
        atlas_entry->pivot_y
    );

    if (override_draw_size) {
        float width = atlas_entry->frame_width > 0 ? atlas_entry->frame_width : atlas_entry->width;
        float height = atlas_entry->frame_height > 0 ? atlas_entry->frame_height : atlas_entry->height;
        sprite_set_draw_size(sprite, width, height);
    }
}

bool atlas_name_has_number_suffix(const char* atlas_entry_name, uint32_t start_index) {
    size_t string_length = strlen(atlas_entry_name);
    bool ignore_space = true;

    if (start_index >= string_length) return false;

    for (size_t j = start_index; j < string_length; j++) {
        char code = atlas_entry_name[j];

        if (code < 0x30 || code > 0x39) {
            if (ignore_space) {
                switch (code) {
                    case 0x09: // tabulation
                    case 0x20: // white space
                               // case 0xff:// hard space
                        //  case 0x5F:// underscore (used in plain-text atlas)
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

bool atlas_get_texture_resolution(Atlas atlas, int32_t* out_width, int32_t* out_height) {
    *out_width = atlas->resolution_width;
    *out_height = atlas->resolution_height;
    return atlas->has_declared_resolution;
}

static bool atlas_parse_resolution(Atlas atlas, const char* resolution_string) {
    if (string_is_empty(resolution_string)) return false;

    int32_t index = -1;
    size_t length = strlen(resolution_string);

    for (int32_t i = 0; i < length; i++) {
        char c = resolution_string[i];
        if (c == 'x' || c == 'X') {
            index = i;
            break;
        }
    }

    if (index < 0) {
        logger_error("atlas_parse_resolution() invalid resolution: %s", resolution_string);
        return false;
    }

    char tmp[length];
    memcpy(tmp, resolution_string, length);
    tmp[index] = '\0';

    uint32_t width = vertexprops_parse_unsigned_integer(
        tmp, 0
    );
    uint32_t height = vertexprops_parse_unsigned_integer(
        resolution_string + index + 1, 0
    );

    if (width < 1 || height < 1 || width > INT32_MAX || height > INT32_MAX) {
        logger_error("atlas_parse_resolution() invalid resolution: %s", resolution_string);
        return false;
    }

    atlas->resolution_width = (int32_t)width;
    atlas->resolution_height = (int32_t)height;
    return true;
}

bool atlas_utils_is_known_extension(const char* src) {
    return string_lowercase_ends_with(src, ".xml") || string_lowercase_ends_with(src, ".txt");
}

static Atlas atlas_parse_from_plain_text(const char* src_txt) {
    // create fake path to texture, assume the format is PNG
    char* path = fs_get_full_path(src_txt);
    int32_t index = string_last_index_of_char(path, '.');
    char* subpath = string_substring(path, 0, index);
    char* fake_texture_filename = string_concat(2, subpath, ".png");
    free_chk(path);
    free_chk(subpath);

    char* text = fs_readtext(src_txt);
    if (!text) {
        free_chk(fake_texture_filename);
        return NULL;
    }

    Atlas atlas = malloc_chk(sizeof(struct Atlas_s));
    malloc_assert(atlas, Atlas);

    *atlas = (struct Atlas_s){
        //.name = src,
        .glyph_fps = 0.0f,
        .texture_filename = fake_texture_filename,
        .resolution_width = FUNKIN_SCREEN_RESOLUTION_WIDTH,
        .resolution_height = FUNKIN_SCREEN_RESOLUTION_HEIGHT,
        .has_declared_resolution = false,
        .size = 0,
        .entries = NULL
    };

    char* unparsed_entry;
    char* temp;
    int32_t x, y, width, height;
    Tokenizer tokenizer_entries = tokenizer_init("\r\n", true, false, text);
    ArrayList arraylist = arraylist_init2(sizeof(AtlasEntry), tokenizer_count_occurrences(tokenizer_entries));

    // count requires bytes
    size_t entries_names_total = 0;
    while ((unparsed_entry = tokenizer_read_next(tokenizer_entries)) != NULL) {
        Tokenizer tokenizer_field = tokenizer_init("\x3D", true, false, unparsed_entry);

        temp = tokenizer_read_next(tokenizer_field);
        char* name = string_trim(temp, false, true);
        free_chk(temp);

        entries_names_total += string_get_bytelength(name);
        free_chk(name);

        tokenizer_destroy(&tokenizer_field);
        free_chk(unparsed_entry);
    }

    char* entries_names_ptr = malloc_for_array(char, entries_names_total);
    atlas->entries_names = entries_names_ptr;
    tokenizer_restart(tokenizer_entries);

    while ((unparsed_entry = tokenizer_read_next(tokenizer_entries)) != NULL) {
        Tokenizer tokenizer_field = tokenizer_init("\x3D", true, false, unparsed_entry);

        temp = tokenizer_read_next(tokenizer_field);
        char* name = string_trim(temp, false, true);
        free_chk(temp);

        temp = tokenizer_read_next(tokenizer_field);
        char* unparsed_coords = string_trim(temp, true, true);
        free_chk(temp);

        tokenizer_destroy(&tokenizer_field);
        free_chk(unparsed_entry);

        if (!name) continue;

        if (!unparsed_coords) {
            free_chk(name);
            continue;
        }

        Tokenizer tokenizer_coords = tokenizer_init("\u00A0\x20\x09", true, false, unparsed_coords);

        temp = tokenizer_read_next(tokenizer_coords);
        x = (int32_t)vertexprops_parse_integer2(temp, MATH2D_MAX_INT32);
        free_chk(temp);

        temp = tokenizer_read_next(tokenizer_coords);
        y = (int32_t)vertexprops_parse_integer2(temp, MATH2D_MAX_INT32);
        free_chk(temp);

        temp = tokenizer_read_next(tokenizer_coords);
        width = (int32_t)vertexprops_parse_integer2(temp, MATH2D_MAX_INT32);
        free_chk(temp);

        temp = tokenizer_read_next(tokenizer_coords);
        height = (int32_t)vertexprops_parse_integer2(temp, MATH2D_MAX_INT32);
        free_chk(temp);

        tokenizer_destroy(&tokenizer_coords);
        free_chk(unparsed_coords);

        if (x == MATH2D_MAX_INT32) goto L_parse_field_failed;
        if (y == MATH2D_MAX_INT32) goto L_parse_field_failed;
        if (width == MATH2D_MAX_INT32) goto L_parse_field_failed;
        if (height == MATH2D_MAX_INT32) goto L_parse_field_failed;

        AtlasEntry entry = {
            .name = atlas_internal_get_name2(name, &entries_names_ptr),
            .x = x,
            .y = y,
            .width = width,
            .height = height,
            .frame_x = 0.0f,
            .frame_y = 0.0f,
            .frame_width = 0.0f,
            .frame_height = 0.0f,
            .pivot_x = 0.0f,
            .pivot_y = 0.0f,
        };
        arraylist_add(arraylist, &entry);

    L_parse_field_failed:
        free_chk(name);
    }

    free_chk(text);
    tokenizer_destroy(&tokenizer_entries);

    arraylist_destroy2(&arraylist, &atlas->size, (void**)&atlas->entries);

    return atlas;
}

static void atlas_parse_tileset(ArrayList arraylist, XmlNode unparsed_tileset, char** entries_names_ptr) {
    int32_t sub_x = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_tileset, "x"), -1);
    int32_t sub_y = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_tileset, "y"), -1);
    int32_t sub_width = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_tileset, "subTextureWidth"), -1);
    int32_t sub_height = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_tileset, "subTextureHeight"), -1);
    int32_t tile_width = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_tileset, "tileWidth"), -1);
    int32_t tile_height = (int32_t)vertexprops_parse_integer2(xmlparser_get_attribute_value2(unparsed_tileset, "tileHeight"), -1);

    if (
        sub_width < 0 || sub_height < 0 ||
        tile_width < 0 || tile_height < 0
    ) {
        logger_warn_xml("atlas_parse_tileset() missing fields in TileSet:", unparsed_tileset);
        return;
    }

    if (sub_x < 0) sub_x = 0;
    if (sub_y < 0) sub_y = 0;

    int32_t rows = sub_width / tile_width;
    int32_t columns = sub_height / tile_height;

    int32_t index = 0;

    foreach (XmlNode, unparsed_tile, XMLPARSER_CHILDREN_ITERATOR, unparsed_tileset) {
        if (!string_equals(xmlparser_get_tag_name(unparsed_tile), "Tile")) {
            logger_warn_xml("atlas_parse_tileset() unknown TileSet entry:", unparsed_tile);
            continue;
        } else if (!xmlparser_has_attribute(unparsed_tile, "name")) {
            logger_warn_xml("atlas_parse_tileset() missing tile name:", unparsed_tile);
            index++;
            continue;
        }

        int32_t tile_index = (int32_t)vertexprops_parse_integer(unparsed_tile, "index", -1);
        if (tile_index < 0) tile_index = index;

        char* tile_name = atlas_internal_get_name(unparsed_tile, entries_names_ptr);
        int32_t tile_x = tile_index % rows;
        int32_t tile_y = (tile_index / rows) % columns;

        tile_x *= tile_width;
        tile_y *= tile_height;

        tile_x += sub_x;
        tile_y += sub_y;

        AtlasEntry entry = {
            .name = tile_name,
            .x = tile_x,
            .y = tile_y,
            .width = tile_width,
            .height = tile_height,
            .frame_x = 0.0,
            .frame_y = 0.0,
            .frame_width = 0.0,
            .frame_height = 0.0,
            .pivot_x = 0.0,
            .pivot_y = 0.0,
        };
        arraylist_add(arraylist, &entry);

        index++;
    }
}


static char* atlas_internal_get_name(XmlNode unparsed_entry, char** entries_names_ptr) {
    const char* name = xmlparser_get_attribute_value2(unparsed_entry, "name");
    if (name == NULL) return NULL;

    char* str = *entries_names_ptr;

    size_t name_length = strlen(name) + 1;
    memcpy(str, name, name_length);

    *entries_names_ptr = str + name_length;

    return str;
}

static char* atlas_internal_get_name2(const char* name, char** entries_names_ptr) {
    if (name == NULL) return NULL;

    char* str = *entries_names_ptr;

    size_t name_length = strlen(name) + 1;
    memcpy(str, name, name_length);

    *entries_names_ptr = str + name_length;

    return str;
}
