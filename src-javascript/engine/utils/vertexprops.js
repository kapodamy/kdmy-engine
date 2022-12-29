"use strict";

const SPRITE_PROP_X = 0;
const SPRITE_PROP_Y = 1;
const SPRITE_PROP_WIDTH = 2;
const SPRITE_PROP_HEIGHT = 3;
const SPRITE_PROP_ROTATE = 4;
const SPRITE_PROP_SCALE_X = 5
const SPRITE_PROP_SCALE_Y = 6;
const SPRITE_PROP_SKEW_X = 7;
const SPRITE_PROP_SKEW_Y = 8
const SPRITE_PROP_TRANSLATE_X = 9
const SPRITE_PROP_TRANSLATE_Y = 10;
const SPRITE_PROP_ALPHA = 11;
const SPRITE_PROP_Z = 12;
const SPRITE_PROP_VERTEX_COLOR_R = 13;
const SPRITE_PROP_VERTEX_COLOR_G = 14;
const SPRITE_PROP_VERTEX_COLOR_B = 15;
const SPRITE_PROP_VERTEX_COLOR_OFFSET_R = 16;
const SPRITE_PROP_VERTEX_COLOR_OFFSET_G = 17;
const SPRITE_PROP_VERTEX_COLOR_OFFSET_B = 18;
const SPRITE_PROP_VERTEX_COLOR_OFFSET_A = 19;
const SPRITE_PROP_FLIP_X = 20;
const SPRITE_PROP_FLIP_Y = 21;
const SPRITE_PROP_ROTATE_PIVOT_ENABLE = 22;
const SPRITE_PROP_ROTATE_PIVOT_U = 23;
const SPRITE_PROP_ROTATE_PIVOT_V = 24;
const SPRITE_PROP_SCALE_DIRECTION_X = 25;
const SPRITE_PROP_SCALE_DIRECTION_Y = 26;
const SPRITE_PROP_TRANSLATE_ROTATION = 27;
const SPRITE_PROP_SCALE_SIZE = 28;

const SPRITE_PROP_ANIMATIONLOOP = 29;// reserved for layouts
const SPRITE_PROP_FRAMEINDEX = 30;// reserved for macroexecutors

const TEXTSPRITE_PROP_FONT_SIZE = 31;
const TEXTSPRITE_PROP_ALIGN_H = 32;
const TEXTSPRITE_PROP_ALIGN_V = 33;
const TEXTSPRITE_PROP_ALIGN_PARAGRAPH = 34;
const TEXTSPRITE_PROP_FORCE_CASE = 35;
const TEXTSPRITE_PROP_FONT_COLOR = 36;
const TEXTSPRITE_PROP_MAX_LINES = 37;
const TEXTSPRITE_PROP_MAX_WIDTH = 38;
const TEXTSPRITE_PROP_MAX_HEIGHT = 39;
const TEXTSPRITE_PROP_BORDER_ENABLE = 40;
const TEXTSPRITE_PROP_BORDER_SIZE = 41;
const TEXTSPRITE_PROP_BORDER_COLOR_R = 42;
const TEXTSPRITE_PROP_BORDER_COLOR_G = 43;
const TEXTSPRITE_PROP_BORDER_COLOR_B = 44;
const TEXTSPRITE_PROP_BORDER_COLOR_A = 45;
const TEXTSPRITE_PROP_PARAGRAPH_SEPARATION = 46;

const SPRITE_PROP_Z_OFFSET = 47;
const SPRITE_PROP_ANTIALIASING = 48;

const MEDIA_PROP_VOLUME = 49;
const MEDIA_PROP_SEEK = 50;
const MEDIA_PROP_PLAYBACK = 51;

const FONT_PROP_WORDBREAK = 52;
const SPRITE_PROP_SCALE_TRANSLATION = 53;
const SPRITE_PROP_FLIP_CORRECTION = 54;

const LAYOUT_PROP_GROUP_VIEWPORT_X = 55;
const LAYOUT_PROP_GROUP_VIEWPORT_Y = 56;
const LAYOUT_PROP_GROUP_VIEWPORT_WIDTH = 57;
const LAYOUT_PROP_GROUP_VIEWPORT_HEIGHT = 58;

const TEXTSPRITE_PROP_BACKGROUND_ENABLED = 59;
const TEXTSPRITE_PROP_BACKGROUND_SIZE = 60;
const TEXTSPRITE_PROP_BACKGROUND_OFFSET_X = 61;
const TEXTSPRITE_PROP_BACKGROUND_OFFSET_Y = 62;
const TEXTSPRITE_PROP_BACKGROUND_COLOR_R = 63;
const TEXTSPRITE_PROP_BACKGROUND_COLOR_G = 64;
const TEXTSPRITE_PROP_BACKGROUND_COLOR_B = 65;
const TEXTSPRITE_PROP_BACKGROUND_COLOR_A = 66;

const CAMERA_PROP_OFFSET_X = 67;
const CAMERA_PROP_OFFSET_Y = 68;
const CAMERA_PROP_OFFSET_Z = 69;

const SPRITE_PROP_ALPHA2 = 70;

const TEXTSPRITE_PROP_BORDER_OFFSET_X = 71;
const TEXTSPRITE_PROP_BORDER_OFFSET_Y = 72;

const TEXTSPRITE_PROP_STRING = 73;// warning: string pointer. DO NOT USE IN MACROEXECUTOR

const CORNER_TOPLEFT = 0;
const CORNER_TOPRIGHT = 1;
const CORNER_BOTTOMLEFT = 2;
const CORNER_BOTTOMRIGHT = 3;

const ALIGN_START = 0;
const ALIGN_CENTER = 1;
const ALIGN_END = 2;
const ALIGN_BOTH = 3;
const ALIGN_NONE = 4;

const PLAYBACK_NONE = 0;
const PLAYBACK_PLAY = 1;
const PLAYBACK_PAUSE = 2;
const PLAYBACK_STOP = 3;
const PLAYBACK_MUTE = 4;
const PLAYBACK_UNMUTE = 5;

const FONT_WORDBREAK_LOOSE = 0;
const FONT_WORDBREAK_NONE = 1;
const FONT_WORDBREAK_BREAK = 2;

const BLEND_DEFAULT = 0;
const BLEND_ZERO = 1;
const BLEND_ONE = 2;
const BLEND_SRC_COLOR = 3;
const BLEND_ONE_MINUS_SRC_COLOR = 4;
const BLEND_DST_COLOR = 5;
const BLEND_ONE_MINUS_DST_COLOR = 6;
const BLEND_SRC_ALPHA = 7;
const BLEND_ONE_MINUS_SRC_ALPHA = 8;
const BLEND_DST_ALPHA = 9;
const BLEND_ONE_MINUS_DST_ALPHA = 10;
const BLEND_CONSTANT_COLOR = 11;
const BLEND_ONE_MINUS_CONSTANT_COLOR = 12;
const BLEND_CONSTANT_ALPHA = 13;
const BLEND_ONE_MINUS_CONSTANT_ALPHA = 14;
const BLEND_SRC_ALPHA_SATURATE = 15;


function vertexprops_parse_sprite_property(node, name, warn) {
    let property = node.getAttribute(name);

    if (!property) {
        console.warn(
            "vertexprops_parse_sprite_property() missing " + name + " attribute", node.outerHTML
        );
        return -2;
    }

    let id = vertexprops_parse_sprite_property2(property);
    if (id < 0 && warn)
        console.warn("vertexprops_parse_sprite_property() unknown property " + property, node.outerHTML);

    return id;
}

function vertexprops_parse_sprite_property2(property) {
    property = property.toLowerCase();// in C dispose this variable!!!

    switch (property) {
        case "x":
            return SPRITE_PROP_X;
        case "y":
            return SPRITE_PROP_Y;
        case "width":
            return SPRITE_PROP_WIDTH;
        case "height":
            return SPRITE_PROP_HEIGHT;
        case "rotate":
            return SPRITE_PROP_ROTATE;
        case "scalex":
            return SPRITE_PROP_SCALE_X;
        case "scaley":
            return SPRITE_PROP_SCALE_Y;
        case "skewx":
            return SPRITE_PROP_SKEW_X;
        case "skewy":
            return SPRITE_PROP_SKEW_Y;
        case "translatex":
            return SPRITE_PROP_TRANSLATE_X;
        case "translatey":
            return SPRITE_PROP_TRANSLATE_Y;
        case "alpha":
            return SPRITE_PROP_ALPHA;
        case "z":
            return SPRITE_PROP_Z;
        case "frameindex":
            return SPRITE_PROP_FRAMEINDEX;
        case "vertexcolorr":
            return SPRITE_PROP_VERTEX_COLOR_R;
        case "vertexcolorg":
            return SPRITE_PROP_VERTEX_COLOR_G;
        case "vertexcolorb":
            return SPRITE_PROP_VERTEX_COLOR_B;
        case "offsetcolorr":
            return SPRITE_PROP_VERTEX_COLOR_OFFSET_R;
        case "offsetcolorg":
            return SPRITE_PROP_VERTEX_COLOR_OFFSET_G;
        case "offsetcolorb":
            return SPRITE_PROP_VERTEX_COLOR_OFFSET_B;
        case "offsetcolora":
            return SPRITE_PROP_VERTEX_COLOR_OFFSET_A;
        case "pivotenable":
            return SPRITE_PROP_ROTATE_PIVOT_ENABLE;
        case "pivotu":
            return SPRITE_PROP_ROTATE_PIVOT_U;
        case "pivotv":
            return SPRITE_PROP_ROTATE_PIVOT_V;
        case "scaledirectionx":
            return SPRITE_PROP_SCALE_DIRECTION_X;
        case "scaledirectiony":
            return SPRITE_PROP_SCALE_DIRECTION_Y;
        case "translaterotation":
            return SPRITE_PROP_TRANSLATE_ROTATION;
        case "scalesize":
            return SPRITE_PROP_SCALE_SIZE;
        case "scaletranslation":
            return SPRITE_PROP_SCALE_TRANSLATION;
        case "zoffset":
            return SPRITE_PROP_Z_OFFSET;
        case "flipx":
            return SPRITE_PROP_FLIP_X;
        case "flipy":
            return SPRITE_PROP_FLIP_Y;
        case "antialiasing":
            return SPRITE_PROP_ANTIALIASING;
        case "flipcorrection":
            return SPRITE_PROP_FLIP_CORRECTION;
        case "alpha2":
            return SPRITE_PROP_ALPHA2;
    }

    return -1;
}


function vertexprops_parse_textsprite_property(node, name, warn) {
    let property = node.getAttribute(name);

    if (!property) {
        console.warn(
            "vertexprops_parse_textsprite_property: missing " + name + " attribute", node.outerHTML
        );
        return -2;
    }

    let id = vertexprops_parse_textsprite_property2(property);
    if (id < 0 && warn)
        console.warn("vertexprops_parse_sprite_property() unknown property " + property, node.outerHTML);

    return id;
}

function vertexprops_parse_textsprite_property2(property) {
    property = property.toLowerCase();// in C dispose this variable!!!

    switch (property) {
        case "fontsize":
            return TEXTSPRITE_PROP_FONT_SIZE;
        case "fontcolor":
            return TEXTSPRITE_PROP_FONT_COLOR;
        case "alignhorizontal":
            return TEXTSPRITE_PROP_ALIGN_H;
        case "alignvertical":
            return TEXTSPRITE_PROP_ALIGN_V;
        case "alignparagraph":
            return TEXTSPRITE_PROP_ALIGN_PARAGRAPH;
        case "forcecase":
            return TEXTSPRITE_PROP_FORCE_CASE;
        case "maxlines":
            return TEXTSPRITE_PROP_MAX_LINES;
        case "maxwidth":
            return TEXTSPRITE_PROP_MAX_WIDTH;
        case "maxheight":
            return TEXTSPRITE_PROP_MAX_HEIGHT;
        case "borderenable":
            return TEXTSPRITE_PROP_BORDER_ENABLE;
        case "bordersize":
            return TEXTSPRITE_PROP_BORDER_SIZE;
        case "bordercolorr":
            return TEXTSPRITE_PROP_BORDER_COLOR_R;
        case "bordercolorg":
            return TEXTSPRITE_PROP_BORDER_COLOR_G;
        case "bordercolorb":
            return TEXTSPRITE_PROP_BORDER_COLOR_B;
        case "bordercolora":
            return TEXTSPRITE_PROP_BORDER_COLOR_A;
        case "paragraphseparation":
            return TEXTSPRITE_PROP_PARAGRAPH_SEPARATION;
        case "string":
            return TEXTSPRITE_PROP_STRING;
        case "wordbreak":
            return FONT_PROP_WORDBREAK;
        case "backgroundenabled":
            return TEXTSPRITE_PROP_BACKGROUND_ENABLED;
        case "backgroundsize":
            return TEXTSPRITE_PROP_BACKGROUND_SIZE;
        case "backgroundoffsetx":
            return TEXTSPRITE_PROP_BACKGROUND_OFFSET_X;
        case "backgroundoffsety":
            return TEXTSPRITE_PROP_BACKGROUND_OFFSET_Y;
        case "backgroundcolorr":
            return TEXTSPRITE_PROP_BACKGROUND_COLOR_R;
        case "backgroundcolorg":
            return TEXTSPRITE_PROP_BACKGROUND_COLOR_G;
        case "backgroundcolorb":
            return TEXTSPRITE_PROP_BACKGROUND_COLOR_B;
        case "backgroundcolora":
            return TEXTSPRITE_PROP_BACKGROUND_COLOR_A;
        case "borderoffsetx":
            return TEXTSPRITE_PROP_BORDER_OFFSET_X;
        case "borderoffsety":
            return TEXTSPRITE_PROP_BORDER_OFFSET_Y;
    }

    return -1;
}

function vertexprops_parse_textsprite_forcecase(node, name, warn) {
    let value = node.getAttribute(name);
    if (!value) return TEXTSPRITE_FORCE_NONE;

    let id = vertexprops_parse_textsprite_forcecase2(value);
    if (id < 0) {
        if (warn) console.log(`Unknown value ${value} in: ${node.outerHTML}`);
        return TEXTSPRITE_FORCE_NONE;
    }

    return id;
}

function vertexprops_parse_textsprite_forcecase2(value) {
    switch (value.toLowerCase()) {
        case "upper":
        case "uppercase":
            return TEXTSPRITE_FORCE_UPPERCASE;
        case "lower":
        case "lowercase":
            return TEXTSPRITE_FORCE_LOWERCASE;
        case "none":
        case "":
            return TEXTSPRITE_FORCE_NONE;
        default:
            return -1;
    }
}


function vertexprops_parse_media_property(node, name, warn) {
    let property = node.getAttribute(name);

    if (!property) {
        console.warn(
            "vertexprops_parse_media_property: missing " + name + " attribute", node.outerHTML
        );
        return -2;
    }

    let id = vertexprops_parse_media_property2(property);
    if (id < 0 && warn)
        console.warn("vertexprops_parse_media_property() unknown property " + property, node.outerHTML);

    return id;
}

function vertexprops_parse_media_property2(property) {
    property = property.toLowerCase();// in C dispose this variable!!!

    switch (property) {
        case "volume":
            return MEDIA_PROP_VOLUME;
        case "seek":
            return MEDIA_PROP_SEEK;
        case "playback":
            return MEDIA_PROP_PLAYBACK;
    }

    return -1;
}


function vertexprops_parse_layout_property(node, name, warn) {
    let property = node.getAttribute(name);

    if (!property) {
        console.warn(
            "vertexprops_parse_layout_property: missing " + name + " attribute", node.outerHTML
        );
        return -2;
    }

    let id = vertexprops_parse_layout_property2(property);
    if (id < 0 && warn)
        console.warn("vertexprops_parse_layout_property() unknown property " + property, node.outerHTML);

    return id;
}

function vertexprops_parse_layout_property2(property) {
    property = property.toLowerCase();// in C dispose this variable!!!

    switch (property) {
        case "groupviewportx":
            return LAYOUT_PROP_GROUP_VIEWPORT_X;
        case "groupviewporty":
            return LAYOUT_PROP_GROUP_VIEWPORT_Y;
        case "groupviewportwidth":
            return LAYOUT_PROP_GROUP_VIEWPORT_WIDTH;
        case "groupviewportheight":
            return LAYOUT_PROP_GROUP_VIEWPORT_HEIGHT;
    }

    return -1;
}


function vertexprops_parse_camera_property(node, name, warn) {
    let property = node.getAttribute(name);

    if (!property) {
        console.warn("vertexprops_parse_camera_property: missing " + name + " attribute", node.outerHTML);
        return -2;
    }

    let id = vertexprops_parse_camera_property2(property);
    if (id < 0 && warn)
        console.warn("vertexprops_parse_camera_property() unknown property " + property, node.outerHTML);

    return id;
}

function vertexprops_parse_camera_property2(property) {
    if (property) {
        switch (property.toLowerCase()) {
            case "offsetx":
                return CAMERA_PROP_OFFSET_X;
            case "offsety":
                return CAMERA_PROP_OFFSET_Y;
            case "offsetz":
                return CAMERA_PROP_OFFSET_Z;
        }
    }

    return -1;
}


function vertexprops_parse_align(node, name, warn_missing, reject_bothnone) {
    let value = node.getAttribute(name);

    if (!value) {
        if (warn_missing) {
            console.warn(
                "vertexprops_parse_align() missing " + name + " attribute", node.outerHTML
            );
        }
        return ALIGN_START;
    }

    let id = vertexprops_parse_align2(value);

    if (reject_bothnone && (id == ALIGN_BOTH || id == ALIGN_NONE))
        id = -1;
    else if (!reject_bothnone && id == ALIGN_CENTER)
        id = -1;

    if (id == -1) {
        console.warn("vertexprops_parse_align() unknown align value: " + value, node.outerHTML);
        return ALIGN_START;
    }

    return id;
}

function vertexprops_parse_align2(value) {
    if (value) {
        switch (value.toLowerCase()) {
            case "none":
                return ALIGN_NONE;
            case "start":
                return ALIGN_START;
            case "center":
                return ALIGN_CENTER;
            case "end":
                return ALIGN_END;
            case "both":
                return ALIGN_BOTH;
        }
    }
    return -1;
}


function vertexprops_parse_playback(node, name, warn_missing) {
    let value = node.getAttribute(name);

    if (!value) {
        if (warn_missing) {
            console.warn(
                "vertexprops_parse_playback() missing " + name + " attribute", node.outerHTML
            );
        }
        return PLAYBACK_NONE;
    }

    let id = vertexprops_parse_playback2(value);

    if (id == -1) {
        console.warn("vertexprops_parse_playback() unknown align value: " + value, node.outerHTML);
        return PLAYBACK_NONE;
    }

    return id;
}

function vertexprops_parse_playback2(value) {
    if (value) {
        switch (value.toLowerCase()) {
            case "play":
                return PLAYBACK_PLAY;
            case "pause":
                return PLAYBACK_PAUSE;
            case "stop":
                return PLAYBACK_STOP;
            case "mute":
                return PLAYBACK_MUTE;
            case "unmute":
                return PLAYBACK_UNMUTE;
        }
    }

    return -1;
}


function vertexprops_parse_boolean(node, attr_name, def_value) {
    let value = node.getAttribute(attr_name);
    if (value == null) return def_value;

    return vertexprops_parse_boolean2(value, def_value);
}

function vertexprops_parse_boolean2(value, def_value) {
    if (value != null) {
        switch (value.toLowerCase()) {
            case "1":
            case "true":
                return 1;
            case "0":
            case "false":
                return 0;
        }
    }
    return def_value;
}


function vertexprops_parse_integer(node, attr_name, def_value) {
    let value = node.getAttribute(attr_name);
    if (value == null || value.length < 1) return def_value;

    return vertexprops_parse_integer2(value, def_value);
}

function vertexprops_parse_integer2(value, def_value) {
    if (value == null) return def_value;
    let regex = /^-?[0-9]+$/;
    if (!regex.test(value)) return def_value;
    let result = parseInt(value);
    return Number.isFinite(result) ? result : def_value;
}


function vertexprops_parse_flag(node, attr_name, def_value) {
    let value = node.getAttribute(attr_name);
    if (value == null) return def_value;

    return vertexprops_parse_flag2(value, def_value);
}

function vertexprops_parse_flag2(value, def_value) {
    if (value != null) {
        switch (value.toLowerCase()) {
            case "enabled":
            case "enable":
                //case "true":
                //case "1":
                return PVR_FLAG_ENABLE;
            case "disabled":
            case "disable":
                //case "false":
                //case "0":
                return PVR_FLAG_DISABLE;
            case "default":
                //case "inherit":
                //case "unset":
                //case "2":
                return PVR_FLAG_DEFAULT;
        }
    }
    return def_value;
}


function vertexprops_parse_unsigned_integer(string, default_value) {
    let regex = /^[0-9]+$/;
    if (regex.test(string))
        return parseInt(string, 10);
    else
        return default_value;
}


function vertexprops_parse_hex(string, output_value, only_if_prefixed) {
    const regex_opt_prefix = /^(?:0x|0X|#)?([a-fA-F0-9]+)$/;
    const regex_obl_prefix = /^(?:0x|0X|#)([a-fA-F0-9]+)$/;

    let regex = only_if_prefixed ? regex_obl_prefix : regex_opt_prefix;
    let matches = regex.exec(string);
    if (!matches) return 0;

    let val = Number.parseInt(matches[1], 16);

    if (!Number.isFinite(val)) return 0;

    // add alpha component
    /*let string_length = matches[1].length;
    if (string_length == 6) {
        val <<= 8;
        val |= 0xFF;
    }*/

    output_value[0] = val;
    return 1;
}

function vertexprops_parse_hex2(string, default_value, only_if_prefixed) {
    const value = [NaN];
    if (vertexprops_parse_hex(string, value, only_if_prefixed)) return value[0];
    return default_value;
}


function vertexprops_parse_wordbreak(node, name, warn_missing) {
    let value = node.getAttribute(name);

    if (!value) {
        if (warn_missing) {
            console.warn(
                "vertexprops_parse_wordbreak() missing " + name + " attribute", node.outerHTML
            );
        }
        return FONT_WORDBREAK_BREAK;
    }

    let id = vertexprops_parse_wordbreak2(value);

    if (id == -1) {
        console.warn("vertexprops_parse_wordbreak() unknown wordbreak value: " + value, node.outerHTML);
        return FONT_WORDBREAK_BREAK;
    }

    return id;
}

function vertexprops_parse_wordbreak2(value) {
    if (value) {
        switch (value.toLowerCase()) {
            case "loose":
                return FONT_WORDBREAK_LOOSE;
            case "none":
                return FONT_WORDBREAK_NONE;
            case "break":
                return FONT_WORDBREAK_BREAK;
        }
    }
    return -1;
}

function vertexprops_parse_float(node, name, def_value) {
    let value = node.getAttribute(name);
    return vertexprops_parse_float2(value, def_value);
}

function vertexprops_parse_float2(value, def_value) {
    let val = Number.parseFloat(value);
    return Number.isNaN(val) ? def_value : val;
}

function vertexprops_is_property_enumerable(property_id) {
    switch (property_id) {
        case SPRITE_PROP_FLIP_X:
        case SPRITE_PROP_FLIP_Y:
        case SPRITE_PROP_ROTATE_PIVOT_ENABLE:
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
            return 1;
    }

    return 0;
}

function vertexprops_is_property_boolean(property_id) {
    switch (property_id) {
        case SPRITE_PROP_FLIP_X:
        case SPRITE_PROP_FLIP_Y:
        case SPRITE_PROP_FLIP_CORRECTION:
        case SPRITE_PROP_ROTATE_PIVOT_ENABLE:
        case SPRITE_PROP_TRANSLATE_ROTATION:
        case SPRITE_PROP_SCALE_SIZE:
        case SPRITE_PROP_SCALE_TRANSLATION:
        case TEXTSPRITE_PROP_BORDER_ENABLE:
        case TEXTSPRITE_PROP_BACKGROUND_ENABLED:
            return 1;
        default:
            return 0;
    }
}

function vertexprops_parse_boolean(node, attr_name, def_value) {
    let value = node.getAttribute(attr_name);

    switch (value) {
        case null:
            return def_value;
        case "1":
        case "true":
        case "TRUE":
            return 1;
    }

    return 0;
}

function vertexprops_parse_double(node, name, def_value) {
    let value = node.getAttribute(name);
    if (value == null || value.length < 1) return def_value;
    return vertexprops_parse_double2(value, def_value);
}

function vertexprops_parse_double2(value, def_value) {
    let val = Number.parseFloat(value);
    return Number.isNaN(val) ? def_value : val;
}

function vertexprops_parse_blending(value) {
    switch (value) {
        case "ZERO":
            return BLEND_ZERO;
        case "ONE":
            return BLEND_ONE;
        case "SRC_COLOR":
            return BLEND_SRC_COLOR;
        case "ONE_MINUS_SRC_COLOR":
            return BLEND_ONE_MINUS_SRC_COLOR;
        case "DST_COLOR":
            return BLEND_DST_COLOR;
        case "ONE_MINUS_DST_COLOR":
            return BLEND_ONE_MINUS_DST_COLOR;
        case "SRC_ALPHA":
            return BLEND_SRC_ALPHA;
        case "ONE_MINUS_SRC_ALPHA":
            return BLEND_ONE_MINUS_SRC_ALPHA;
        case "DST_ALPHA":
            return BLEND_DST_ALPHA;
        case "ONE_MINUS_DST_ALPHA":
            return BLEND_ONE_MINUS_DST_ALPHA;
        case "CONSTANT_COLOR":
            return BLEND_CONSTANT_COLOR;
        case "ONE_MINUS_CONSTANT_COLOR":
            return BLEND_ONE_MINUS_CONSTANT_COLOR;
        case "CONSTANT_ALPHA":
            return BLEND_CONSTANT_ALPHA;
        case "ONE_MINUS_CONSTANT_ALPHA":
            return BLEND_ONE_MINUS_CONSTANT_ALPHA;
        case "SRC_ALPHA_SATURATE":
            return BLEND_SRC_ALPHA_SATURATE;
        case null:
            return BLEND_DEFAULT;
    }
    console.warn("vertexprops_parse_blending() unknown blending: " + value);
    return BLEND_DEFAULT;
}
