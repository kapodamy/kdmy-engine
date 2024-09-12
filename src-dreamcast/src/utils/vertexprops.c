#include <ctype.h>

#include "stringutils.h"
#include "textsprite.h"
#include "vertexprops.h"

#define PARSE_NUMBER(fn, str, val, def_val)                 \
    {                                                       \
        size_t start_index, end_index;                      \
        trim_string(str, &start_index, &end_index);         \
                                                            \
        if (start_index == end_index) {                     \
            val = def_val;                                  \
        } else {                                            \
            const char* expected_str_end = str + end_index; \
            char* end_str;                                  \
                                                            \
            val = fn(str + start_index, &end_str, 10);      \
                                                            \
            if (end_str != expected_str_end) {              \
                val = def_val;                              \
            }                                               \
        }                                                   \
    }

static bool string_eq_ic_ascii(const char* enumerated_value, const char* value) {
    if (!value) return false;

    while (true) {
        char c1 = *enumerated_value++;
        char c2 = *value++;

        if (c1 == '\0' && c2 == '\0') {
            return true;
        }

        c1 = tolower(c1);
        c2 = tolower(c2);

        if (c1 != c2) {
            return false;
        }
    }


    return false;
}

static void print_warn(const char* fn, XmlNode node, const char* name_or_prop, bool miss_or_unk, bool warn) {
    if (!warn) return;

    if (miss_or_unk)
        logger_warn_xml("%s() missing %s attribute:\n", node, fn, name_or_prop);
    else
        logger_warn_xml("%s() unknown property value %s:\n", node, fn, name_or_prop);
}


static void trim_string(const char* str, size_t* out_start, size_t* out_end) {
    size_t index = 0;

    while (true) {
        char c = *str++;
        switch (c) {
            case '\0':
                *out_start = *out_end = 0;
                return;
            case '\r':
            case ' ':
            case '\n':
            case '\t':
            case '\v':
                index++;
                continue;
        }
        break;
    }

    *out_start = index;

    size_t str_length = strlen(str);
    size_t index_end = index + str_length;

    str += str_length - 1;
    while (index_end > index) {
        char c = *str--;
        switch (c) {
            case '\r':
            case '\x20':
            case '\n':
            case '\t':
            case '\v':
                index_end--;
                continue;
        }
        break;
    }

    *out_end = index_end + 1;
}



int32_t vertexprops_parse_sprite_property(XmlNode node, const char* name, bool warn) {
    const char* property = xmlparser_get_attribute_value2(node, name);

    if (!property) {
        print_warn(__func__, node, name, true, warn);
        return -2;
    }

    int32_t id = vertexprops_parse_sprite_property2(property);
    if (id < 0)
        print_warn(__func__, node, property, false, warn);

    return id;
}

int32_t vertexprops_parse_sprite_property2(const char* property) {
    if (string_eq_ic_ascii("x", property))
        return SPRITE_PROP_X;
    if (string_eq_ic_ascii("y", property))
        return SPRITE_PROP_Y;
    if (string_eq_ic_ascii("width", property))
        return SPRITE_PROP_WIDTH;
    if (string_eq_ic_ascii("height", property))
        return SPRITE_PROP_HEIGHT;
    if (string_eq_ic_ascii("rotate", property))
        return SPRITE_PROP_ROTATE;
    if (string_eq_ic_ascii("scalex", property))
        return SPRITE_PROP_SCALE_X;
    if (string_eq_ic_ascii("scaley", property))
        return SPRITE_PROP_SCALE_Y;
    if (string_eq_ic_ascii("skewx", property))
        return SPRITE_PROP_SKEW_X;
    if (string_eq_ic_ascii("skewy", property))
        return SPRITE_PROP_SKEW_Y;
    if (string_eq_ic_ascii("translatex", property))
        return SPRITE_PROP_TRANSLATE_X;
    if (string_eq_ic_ascii("translatey", property))
        return SPRITE_PROP_TRANSLATE_Y;
    if (string_eq_ic_ascii("alpha", property))
        return SPRITE_PROP_ALPHA;
    if (string_eq_ic_ascii("z", property))
        return SPRITE_PROP_Z;
    if (string_eq_ic_ascii("frameindex", property))
        return SPRITE_PROP_FRAMEINDEX;
    if (string_eq_ic_ascii("vertexcolorr", property))
        return SPRITE_PROP_VERTEX_COLOR_R;
    if (string_eq_ic_ascii("vertexcolorg", property))
        return SPRITE_PROP_VERTEX_COLOR_G;
    if (string_eq_ic_ascii("vertexcolorb", property))
        return SPRITE_PROP_VERTEX_COLOR_B;
    if (string_eq_ic_ascii("offsetcolorr", property))
        return SPRITE_PROP_VERTEX_COLOR_OFFSET_R;
    if (string_eq_ic_ascii("offsetcolorg", property))
        return SPRITE_PROP_VERTEX_COLOR_OFFSET_G;
    if (string_eq_ic_ascii("offsetcolorb", property))
        return SPRITE_PROP_VERTEX_COLOR_OFFSET_B;
    if (string_eq_ic_ascii("offsetcolora", property))
        return SPRITE_PROP_VERTEX_COLOR_OFFSET_A;
    if (string_eq_ic_ascii("rotatepivotenabled", property))
        return SPRITE_PROP_ROTATE_PIVOT_ENABLED;
    if (string_eq_ic_ascii("rotatepivotu", property))
        return SPRITE_PROP_ROTATE_PIVOT_U;
    if (string_eq_ic_ascii("rotatepivotv", property))
        return SPRITE_PROP_ROTATE_PIVOT_V;
    if (string_eq_ic_ascii("scaledirectionx", property))
        return SPRITE_PROP_SCALE_DIRECTION_X;
    if (string_eq_ic_ascii("scaledirectiony", property))
        return SPRITE_PROP_SCALE_DIRECTION_Y;
    if (string_eq_ic_ascii("translaterotation", property))
        return SPRITE_PROP_TRANSLATE_ROTATION;
    if (string_eq_ic_ascii("scalesize", property))
        return SPRITE_PROP_SCALE_SIZE;
    if (string_eq_ic_ascii("scaletranslation", property))
        return SPRITE_PROP_SCALE_TRANSLATION;
    if (string_eq_ic_ascii("zoffset", property))
        return SPRITE_PROP_Z_OFFSET;
    if (string_eq_ic_ascii("flipx", property))
        return SPRITE_PROP_FLIP_X;
    if (string_eq_ic_ascii("flipy", property))
        return SPRITE_PROP_FLIP_Y;
    if (string_eq_ic_ascii("antialiasing", property))
        return SPRITE_PROP_ANTIALIASING;
    if (string_eq_ic_ascii("flipcorrection", property))
        return SPRITE_PROP_FLIP_CORRECTION;
    if (string_eq_ic_ascii("alpha2", property))
        return SPRITE_PROP_ALPHA2;

    return -1;
}


int32_t vertexprops_parse_textsprite_property(XmlNode node, const char* name, bool warn) {
    const char* property = xmlparser_get_attribute_value2(node, name);

    if (!property) {
        print_warn(__func__, node, name, true, warn);
        return -2;
    }

    int32_t id = vertexprops_parse_textsprite_property2(property);
    if (id < 0)
        print_warn(__func__, node, property, false, warn);

    return id;
}

int32_t vertexprops_parse_textsprite_property2(const char* property) {
    if (string_eq_ic_ascii("fontsize", property))
        return TEXTSPRITE_PROP_FONT_SIZE;
    if (string_eq_ic_ascii("fontcolor", property))
        return TEXTSPRITE_PROP_FONT_COLOR;
    if (string_eq_ic_ascii("alignhorizontal", property))
        return TEXTSPRITE_PROP_ALIGN_H;
    if (string_eq_ic_ascii("alignvertical", property))
        return TEXTSPRITE_PROP_ALIGN_V;
    if (string_eq_ic_ascii("alignparagraph", property))
        return TEXTSPRITE_PROP_ALIGN_PARAGRAPH;
    if (string_eq_ic_ascii("forcecase", property))
        return TEXTSPRITE_PROP_FORCE_CASE;
    if (string_eq_ic_ascii("maxlines", property))
        return TEXTSPRITE_PROP_MAX_LINES;
    if (string_eq_ic_ascii("maxwidth", property))
        return TEXTSPRITE_PROP_MAX_WIDTH;
    if (string_eq_ic_ascii("maxheight", property))
        return TEXTSPRITE_PROP_MAX_HEIGHT;
    if (string_eq_ic_ascii("borderenable", property))
        return TEXTSPRITE_PROP_BORDER_ENABLE;
    if (string_eq_ic_ascii("bordersize", property))
        return TEXTSPRITE_PROP_BORDER_SIZE;
    if (string_eq_ic_ascii("bordercolorr", property))
        return TEXTSPRITE_PROP_BORDER_COLOR_R;
    if (string_eq_ic_ascii("bordercolorg", property))
        return TEXTSPRITE_PROP_BORDER_COLOR_G;
    if (string_eq_ic_ascii("bordercolorb", property))
        return TEXTSPRITE_PROP_BORDER_COLOR_B;
    if (string_eq_ic_ascii("bordercolora", property))
        return TEXTSPRITE_PROP_BORDER_COLOR_A;
    if (string_eq_ic_ascii("paragraphseparation", property))
        return TEXTSPRITE_PROP_PARAGRAPH_SEPARATION;
    if (string_eq_ic_ascii("string", property))
        return TEXTSPRITE_PROP_STRING;
    if (string_eq_ic_ascii("wordbreak", property))
        return FONT_PROP_WORDBREAK;
    if (string_eq_ic_ascii("backgroundenabled", property))
        return TEXTSPRITE_PROP_BACKGROUND_ENABLED;
    if (string_eq_ic_ascii("backgroundsize", property))
        return TEXTSPRITE_PROP_BACKGROUND_SIZE;
    if (string_eq_ic_ascii("backgroundoffsetx", property))
        return TEXTSPRITE_PROP_BACKGROUND_OFFSET_X;
    if (string_eq_ic_ascii("backgroundoffsety", property))
        return TEXTSPRITE_PROP_BACKGROUND_OFFSET_Y;
    if (string_eq_ic_ascii("backgroundcolorr", property))
        return TEXTSPRITE_PROP_BACKGROUND_COLOR_R;
    if (string_eq_ic_ascii("backgroundcolorg", property))
        return TEXTSPRITE_PROP_BACKGROUND_COLOR_G;
    if (string_eq_ic_ascii("backgroundcolorb", property))
        return TEXTSPRITE_PROP_BACKGROUND_COLOR_B;
    if (string_eq_ic_ascii("backgroundcolora", property))
        return TEXTSPRITE_PROP_BACKGROUND_COLOR_A;
    if (string_eq_ic_ascii("borderoffsetx", property))
        return TEXTSPRITE_PROP_BORDER_OFFSET_X;
    if (string_eq_ic_ascii("borderoffsety", property))
        return TEXTSPRITE_PROP_BORDER_OFFSET_Y;

    return -1;
}

int32_t vertexprops_parse_textsprite_forcecase(XmlNode node, const char* name, bool warn) {
    const char* value = xmlparser_get_attribute_value2(node, name);
    if (!value) return TEXTSPRITE_FORCE_NONE;

    int32_t id = vertexprops_parse_textsprite_forcecase2(value);
    if (id < 0) {
        print_warn(__func__, node, value, false, warn);
        return TEXTSPRITE_FORCE_NONE;
    }

    return id;
}

int32_t vertexprops_parse_textsprite_forcecase2(const char* value) {
    if (string_eq_ic_ascii("upper", value) || string_eq_ic_ascii("uppercase", value))
        return TEXTSPRITE_FORCE_UPPERCASE;
    if (string_eq_ic_ascii("lower", value) || string_eq_ic_ascii("lowercase", value))
        return TEXTSPRITE_FORCE_LOWERCASE;
    if (string_eq_ic_ascii("none", value) || string_eq_ic_ascii("", value))
        return TEXTSPRITE_FORCE_NONE;
    return -1;
}


int32_t vertexprops_parse_media_property(XmlNode node, const char* name, bool warn) {
    const char* property = xmlparser_get_attribute_value2(node, name);

    if (!property) {
        print_warn(__func__, node, name, true, warn);
        return -2;
    }

    int32_t id = vertexprops_parse_media_property2(property);
    if (id < 0)
        print_warn(__func__, node, property, false, warn);

    return id;
}

int32_t vertexprops_parse_media_property2(const char* property) {
    if (string_eq_ic_ascii("volume", property))
        return MEDIA_PROP_VOLUME;
    if (string_eq_ic_ascii("seek", property))
        return MEDIA_PROP_SEEK;
    if (string_eq_ic_ascii("playback", property))
        return MEDIA_PROP_PLAYBACK;

    return -1;
}


int32_t vertexprops_parse_layout_property(XmlNode node, const char* name, bool warn) {
    const char* property = xmlparser_get_attribute_value2(node, name);

    if (!property) {
        print_warn(__func__, node, name, true, warn);
        return -2;
    }

    int32_t id = vertexprops_parse_layout_property2(property);
    if (id < 0)
        print_warn(__func__, node, property, false, warn);

    return id;
}

int32_t vertexprops_parse_layout_property2(const char* property) {
    if (string_eq_ic_ascii("groupviewportx", property))
        return LAYOUT_PROP_GROUP_VIEWPORT_X;
    if (string_eq_ic_ascii("groupviewporty", property))
        return LAYOUT_PROP_GROUP_VIEWPORT_Y;
    if (string_eq_ic_ascii("groupviewportwidth", property))
        return LAYOUT_PROP_GROUP_VIEWPORT_WIDTH;
    if (string_eq_ic_ascii("groupviewportheight", property))
        return LAYOUT_PROP_GROUP_VIEWPORT_HEIGHT;

    return -1;
}


int32_t vertexprops_parse_camera_property(XmlNode node, const char* name, bool warn) {
    const char* property = xmlparser_get_attribute_value2(node, name);

    if (!property) {
        print_warn(__func__, node, name, true, warn);
        return -2;
    }

    int32_t id = vertexprops_parse_camera_property2(property);
    if (id < 0)
        print_warn(__func__, node, property, false, warn);

    return id;
}

int32_t vertexprops_parse_camera_property2(const char* property) {
    if (property) {
        if (string_eq_ic_ascii("offsetx", property))
            return CAMERA_PROP_OFFSET_X;
        if (string_eq_ic_ascii("offsety", property))
            return CAMERA_PROP_OFFSET_Y;
        if (string_eq_ic_ascii("offsetz", property))
            return CAMERA_PROP_OFFSET_Z;
        if (string_eq_ic_ascii("offsetzoom", property))
            return CAMERA_PROP_OFFSET_ZOOM;
    }

    return -1;
}


Align vertexprops_parse_align(XmlNode node, const char* name, bool warn_missing, bool reject_bothnone) {
    const char* value = xmlparser_get_attribute_value2(node, name);

    if (!value) {
        print_warn(__func__, node, name, true, warn_missing);
        return ALIGN_START;
    }

    int32_t id = vertexprops_parse_align2(value);

    if (reject_bothnone && (id == ALIGN_BOTH || id == ALIGN_NONE))
        id = -1;
    else if (!reject_bothnone && id == ALIGN_CENTER)
        id = -1;

    if (id == -1) {
        print_warn(__func__, node, value, false, warn_missing);
        return ALIGN_START;
    }

    return id;
}

Align vertexprops_parse_align2(const char* value) {
    if (value) {
        if (string_eq_ic_ascii("none", value))
            return ALIGN_NONE;
        if (string_eq_ic_ascii("start", value))
            return ALIGN_START;
        if (string_eq_ic_ascii("center", value))
            return ALIGN_CENTER;
        if (string_eq_ic_ascii("end", value))
            return ALIGN_END;
        if (string_eq_ic_ascii("both", value))
            return ALIGN_BOTH;
    }
    return -1;
}


int32_t vertexprops_parse_playback(XmlNode node, const char* name, bool warn_missing) {
    const char* value = xmlparser_get_attribute_value2(node, name);

    if (!value) {
        print_warn(__func__, node, name, true, warn_missing);
        return PLAYBACK_NONE;
    }

    int32_t id = vertexprops_parse_playback2(value);

    if (id == -1) {
        print_warn(__func__, node, value, false, warn_missing);
        return PLAYBACK_NONE;
    }

    return id;
}

int32_t vertexprops_parse_playback2(const char* value) {
    if (value) {
        if (string_eq_ic_ascii("play", value))
            return PLAYBACK_PLAY;
        if (string_eq_ic_ascii("pause", value))
            return PLAYBACK_PAUSE;
        if (string_eq_ic_ascii("stop", value))
            return PLAYBACK_STOP;
        if (string_eq_ic_ascii("mute", value))
            return PLAYBACK_MUTE;
        if (string_eq_ic_ascii("unmute", value))
            return PLAYBACK_UNMUTE;
    }

    return -1;
}


nbool vertexprops_parse_boolean(XmlNode node, const char* attr_name, nbool def_value) {
    const char* value = xmlparser_get_attribute_value2(node, attr_name);
    if (!value) return def_value;

    return vertexprops_parse_boolean2(value, def_value);
}

nbool vertexprops_parse_boolean2(const char* value, nbool def_value) {
    if (value != NULL) {
        if (string_eq_ic_ascii("1", value) || string_eq_ic_ascii("true", value))
            return true;
        if (string_eq_ic_ascii("0", value) || string_eq_ic_ascii("false", value))
            return false;
    }
    return def_value;
}


int64_t vertexprops_parse_integer(XmlNode node, const char* attr_name, int64_t def_value) {
    const char* value = xmlparser_get_attribute_value2(node, attr_name);
    if (!value || value[0] == '\0') return def_value;

    return vertexprops_parse_integer2(value, def_value);
}

int64_t vertexprops_parse_integer2(const char* value, int64_t def_value) {
    long long val;
    PARSE_NUMBER(strtoll, value, val, def_value);

    return val;
}


PVRFlag vertexprops_parse_flag(XmlNode node, const char* attr_name, PVRFlag def_value) {
    const char* value = xmlparser_get_attribute_value2(node, attr_name);
    if (!value) return def_value;

    return vertexprops_parse_flag2(value, def_value);
}

PVRFlag vertexprops_parse_flag2(const char* value, PVRFlag def_value) {
    if (value != NULL) {
        if (
            string_eq_ic_ascii("enabled", value) || string_eq_ic_ascii("enable", value)
            //|| string_eq_ic_ascii("true", value) || string_eq_ic_ascii("1", value)
        )
            return PVRCTX_FLAG_ENABLE;
        if (
            string_eq_ic_ascii("disabled", value) || string_eq_ic_ascii("disable", value)
            //|| (string_eq_ic_ascii("false", value) || (string_eq_ic_ascii("0", value)
        )
            return PVRCTX_FLAG_DISABLE;
        if (
            string_eq_ic_ascii("default", value)
            //|| (string_eq_ic_ascii("inherit", value) || (string_eq_ic_ascii("unset", value)
            //|| (string_eq_ic_ascii("2", value)
        )
            return PVRCTX_FLAG_DEFAULT;
    }
    return def_value;
}


uint32_t vertexprops_parse_unsigned_integer(const char* string, uint32_t default_value) {
    unsigned long value;
    PARSE_NUMBER(strtoul, string, value, default_value);

    return value;
}


bool vertexprops_parse_hex(const char* string, uint32_t* output_value, bool only_if_prefixed) {
    size_t start_index, end_index;
    trim_string(string, &start_index, &end_index);

    if (start_index == end_index) {
        return false;
    }

    if (string[start_index + 0] == '#') {
        start_index++;
    } else if (string[start_index + 0] == '0' && tolower(string[start_index + 1]) == 'x') {
        start_index += 2;
    } else if (only_if_prefixed) {
        return false;
    }

    if (start_index == end_index) {
        return false;
    }

    const char* expected_str_end = string + end_index;
    char* end_str;
    unsigned long value = strtoul(string + start_index, &end_str, 16);

    if (end_str != expected_str_end) {
        return false;
    }

    *output_value = value;
    return true;
}

uint32_t vertexprops_parse_hex2(const char* string, uint32_t default_value, bool only_if_prefixed) {
    uint32_t value = 0x00;
    if (vertexprops_parse_hex(string, &value, only_if_prefixed)) return value;
    return default_value;
}


int32_t vertexprops_parse_wordbreak(XmlNode node, const char* name, bool warn_missing) {
    const char* value = xmlparser_get_attribute_value2(node, name);

    if (!value) {
        print_warn(__func__, node, name, true, warn_missing);
        return FONT_WORDBREAK_BREAK;
    }

    int32_t id = vertexprops_parse_wordbreak2(value);

    if (id == -1) {
        print_warn(__func__, node, value, false, warn_missing);
        return FONT_WORDBREAK_BREAK;
    }

    return id;
}

int32_t vertexprops_parse_wordbreak2(const char* value) {
    if (value) {
        if (string_eq_ic_ascii("loose", value))
            return FONT_WORDBREAK_LOOSE;
        if (string_eq_ic_ascii("none", value))
            return FONT_WORDBREAK_NONE;
        if (string_eq_ic_ascii("break", value))
            return FONT_WORDBREAK_BREAK;
    }
    return -1;
}

float vertexprops_parse_float(XmlNode node, const char* name, float def_value) {
    const char* value = xmlparser_get_attribute_value2(node, name);
    return vertexprops_parse_float2(value, def_value);
}

float vertexprops_parse_float2(const char* value, float def_value) {
    return (float)vertexprops_parse_double2(value, (float64)def_value);
}

bool vertexprops_is_property_enumerable(int32_t property_id) {
    switch (property_id) {
        case SPRITE_PROP_FLIP_X:
        case SPRITE_PROP_FLIP_Y:
        case SPRITE_PROP_ROTATE_PIVOT_ENABLED:
        case SPRITE_PROP_ROTATE_PIVOT_U:
        case SPRITE_PROP_ROTATE_PIVOT_V:
        case SPRITE_PROP_SCALE_DIRECTION_X:
        case SPRITE_PROP_SCALE_DIRECTION_Y:
        case SPRITE_PROP_TRANSLATE_ROTATION:
        case SPRITE_PROP_SCALE_SIZE:
        case SPRITE_PROP_SCALE_TRANSLATION:

        case SPRITE_PROP_ANIMATIONLOOP:
        case SPRITE_PROP_FRAMEINDEX:

        case TEXTSPRITE_PROP_ALIGN_H:
        case TEXTSPRITE_PROP_ALIGN_V:
        case TEXTSPRITE_PROP_ALIGN_PARAGRAPH:
        case TEXTSPRITE_PROP_FORCE_CASE:
        case TEXTSPRITE_PROP_BORDER_ENABLE:
        case TEXTSPRITE_PROP_BACKGROUND_ENABLED:
            return true;
    }

    return false;
}

bool vertexprops_is_property_boolean(int32_t property_id) {
    switch (property_id) {
        case SPRITE_PROP_FLIP_X:
        case SPRITE_PROP_FLIP_Y:
        case SPRITE_PROP_FLIP_CORRECTION:
        case SPRITE_PROP_ROTATE_PIVOT_ENABLED:
        case SPRITE_PROP_TRANSLATE_ROTATION:
        case SPRITE_PROP_SCALE_SIZE:
        case SPRITE_PROP_SCALE_TRANSLATION:
        case TEXTSPRITE_PROP_BORDER_ENABLE:
        case TEXTSPRITE_PROP_BACKGROUND_ENABLED:
            return true;
        default:
            return false;
    }
}


float64 vertexprops_parse_double(XmlNode node, const char* name, float64 def_value) {
    const char* value = xmlparser_get_attribute_value2(node, name);
    if (!value || value[0] == '\0') return def_value;
    return vertexprops_parse_double2(value, def_value);
}

float64 vertexprops_parse_double2(const char* value, float64 def_value) {
    size_t start_index, end_index;
    trim_string(value, &start_index, &end_index);

    if (start_index == end_index) {
        return def_value;
    }

    const char* expected_str_end = value + end_index;
    char* end_str;

    float64 val = strtod(value + start_index, &end_str);

    if (/*pointer equals*/end_str == expected_str_end) {
        return val;
    }

    return def_value;
}

Blend vertexprops_parse_blending(const char* value) {
    if (string_eq_ic_ascii("ZERO", value))
        return BLEND_ZERO;
    if (string_eq_ic_ascii("ONE", value))
        return BLEND_ONE;
    if (string_eq_ic_ascii("SRC_COLOR", value))
        goto L_unsupported; // return BLEND_SRC_COLOR;
    if (string_eq_ic_ascii("ONE_MINUS_SRC_COLOR", value))
        goto L_unsupported; // return BLEND_ONE_MINUS_SRC_COLOR;
    if (string_eq_ic_ascii("DST_COLOR", value))
        return BLEND_DST_COLOR;
    if (string_eq_ic_ascii("ONE_MINUS_DST_COLOR", value))
        return BLEND_ONE_MINUS_DST_COLOR;
    if (string_eq_ic_ascii("SRC_ALPHA", value))
        return BLEND_SRC_ALPHA;
    if (string_eq_ic_ascii("ONE_MINUS_SRC_ALPHA", value))
        return BLEND_ONE_MINUS_SRC_ALPHA;
    if (string_eq_ic_ascii("DST_ALPHA", value))
        return BLEND_DST_ALPHA;
    if (string_eq_ic_ascii("ONE_MINUS_DST_ALPHA", value))
        return BLEND_ONE_MINUS_DST_ALPHA;
    if (string_eq_ic_ascii("CONSTANT_COLOR", value))
        goto L_unsupported; // return BLEND_CONSTANT_COLOR;
    if (string_eq_ic_ascii("ONE_MINUS_CONSTANT_COLOR", value))
        goto L_unsupported; // return BLEND_ONE_MINUS_CONSTANT_COLOR;
    if (string_eq_ic_ascii("CONSTANT_ALPHA", value))
        goto L_unsupported; // return BLEND_CONSTANT_ALPHA;
    if (string_eq_ic_ascii("ONE_MINUS_CONSTANT_ALPHA", value))
        goto L_unsupported; // return BLEND_ONE_MINUS_CONSTANT_ALPHA;
    if (string_eq_ic_ascii("SRC_ALPHA_SATURATE", value))
        goto L_unsupported; // return BLEND_SRC_ALPHA_SATURATE;
    if (!value)
        return BLEND_DEFAULT;

    logger_warn("%s() unknown blending: %s", __func__, value);
    return BLEND_DEFAULT;

L_unsupported:
    logger_warn("%s() the blending '%s' is not supported in the current platform", __func__, value);
    return BLEND_DEFAULT;
}

bool vertexprops_is_integer(const char* value) {
    if (!value) return false;

    if (value[0] == '-') value++;

    for (; *value; value++) {
        if (!isdigit((uint8_t)*value)) {
            return false;
        }
    }

    return true;
}
