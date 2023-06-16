using System;
using System.Globalization;
using Engine.Platform;

namespace Engine.Utils;

public enum Align : int {
    INVALID = -1,
    START = 0,
    CENTER = 1,
    END = 2,
    BOTH = 3,
    NONE = 4
}

public enum Blend : int {
    INVALID_VALUE = -1,
    DEFAULT = 0,
    ZERO = 1,
    ONE = 2,
    SRC_COLOR = 3,
    ONE_MINUS_SRC_COLOR = 4,
    DST_COLOR = 5,
    ONE_MINUS_DST_COLOR = 6,
    SRC_ALPHA = 7,
    ONE_MINUS_SRC_ALPHA = 8,
    DST_ALPHA = 9,
    ONE_MINUS_DST_ALPHA = 10,
    CONSTANT_COLOR = 11,
    ONE_MINUS_CONSTANT_COLOR = 12,
    CONSTANT_ALPHA = 13,
    ONE_MINUS_CONSTANT_ALPHA = 14,
    SRC_ALPHA_SATURATE = 15
}

public static class VertexProps {
    private const NumberStyles FLAGS_FLOAT = NumberStyles.AllowLeadingSign | NumberStyles.AllowDecimalPoint | NumberStyles.AllowExponent | NumberStyles.Integer;
    private const NumberStyles FLAGS_INT = NumberStyles.Integer | NumberStyles.AllowLeadingSign;


    public const int SPRITE_PROP_X = 0;
    public const int SPRITE_PROP_Y = 1;
    public const int SPRITE_PROP_WIDTH = 2;
    public const int SPRITE_PROP_HEIGHT = 3;
    public const int SPRITE_PROP_ROTATE = 4;
    public const int SPRITE_PROP_SCALE_X = 5;
    public const int SPRITE_PROP_SCALE_Y = 6;
    public const int SPRITE_PROP_SKEW_X = 7;
    public const int SPRITE_PROP_SKEW_Y = 8;
    public const int SPRITE_PROP_TRANSLATE_X = 9;
    public const int SPRITE_PROP_TRANSLATE_Y = 10;
    public const int SPRITE_PROP_ALPHA = 11;
    public const int SPRITE_PROP_Z = 12;
    public const int SPRITE_PROP_VERTEX_COLOR_R = 13;
    public const int SPRITE_PROP_VERTEX_COLOR_G = 14;
    public const int SPRITE_PROP_VERTEX_COLOR_B = 15;
    public const int SPRITE_PROP_VERTEX_COLOR_OFFSET_R = 16;
    public const int SPRITE_PROP_VERTEX_COLOR_OFFSET_G = 17;
    public const int SPRITE_PROP_VERTEX_COLOR_OFFSET_B = 18;
    public const int SPRITE_PROP_VERTEX_COLOR_OFFSET_A = 19;
    public const int SPRITE_PROP_FLIP_X = 20;
    public const int SPRITE_PROP_FLIP_Y = 21;
    public const int SPRITE_PROP_ROTATE_PIVOT_ENABLED = 22;
    public const int SPRITE_PROP_ROTATE_PIVOT_U = 23;
    public const int SPRITE_PROP_ROTATE_PIVOT_V = 24;
    public const int SPRITE_PROP_SCALE_DIRECTION_X = 25;
    public const int SPRITE_PROP_SCALE_DIRECTION_Y = 26;
    public const int SPRITE_PROP_TRANSLATE_ROTATION = 27;
    public const int SPRITE_PROP_SCALE_SIZE = 28;

    public const int SPRITE_PROP_ANIMATIONLOOP = 29;// reserved for layouts
    public const int SPRITE_PROP_FRAMEINDEX = 30;// reserved for macroexecutors

    public const int TEXTSPRITE_PROP_FONT_SIZE = 31;
    public const int TEXTSPRITE_PROP_ALIGN_H = 32;
    public const int TEXTSPRITE_PROP_ALIGN_V = 33;
    public const int TEXTSPRITE_PROP_ALIGN_PARAGRAPH = 34;
    public const int TEXTSPRITE_PROP_FORCE_CASE = 35;
    public const int TEXTSPRITE_PROP_FONT_COLOR = 36;
    public const int TEXTSPRITE_PROP_MAX_LINES = 37;
    public const int TEXTSPRITE_PROP_MAX_WIDTH = 38;
    public const int TEXTSPRITE_PROP_MAX_HEIGHT = 39;
    public const int TEXTSPRITE_PROP_BORDER_ENABLE = 40;
    public const int TEXTSPRITE_PROP_BORDER_SIZE = 41;
    public const int TEXTSPRITE_PROP_BORDER_COLOR_R = 42;
    public const int TEXTSPRITE_PROP_BORDER_COLOR_G = 43;
    public const int TEXTSPRITE_PROP_BORDER_COLOR_B = 44;
    public const int TEXTSPRITE_PROP_BORDER_COLOR_A = 45;
    public const int TEXTSPRITE_PROP_PARAGRAPH_SEPARATION = 46;

    public const int SPRITE_PROP_Z_OFFSET = 47;
    public const int SPRITE_PROP_ANTIALIASING = 48;

    public const int MEDIA_PROP_VOLUME = 49;
    public const int MEDIA_PROP_SEEK = 50;
    public const int MEDIA_PROP_PLAYBACK = 51;

    public const int FONT_PROP_WORDBREAK = 52;

    public const int SPRITE_PROP_SCALE_TRANSLATION = 53;
    public const int SPRITE_PROP_FLIP_CORRECTION = 54;

    public const int LAYOUT_PROP_GROUP_VIEWPORT_X = 55;
    public const int LAYOUT_PROP_GROUP_VIEWPORT_Y = 56;
    public const int LAYOUT_PROP_GROUP_VIEWPORT_WIDTH = 57;
    public const int LAYOUT_PROP_GROUP_VIEWPORT_HEIGHT = 58;

    public const int TEXTSPRITE_PROP_BACKGROUND_ENABLED = 59;
    public const int TEXTSPRITE_PROP_BACKGROUND_SIZE = 60;
    public const int TEXTSPRITE_PROP_BACKGROUND_OFFSET_X = 61;
    public const int TEXTSPRITE_PROP_BACKGROUND_OFFSET_Y = 62;
    public const int TEXTSPRITE_PROP_BACKGROUND_COLOR_R = 63;
    public const int TEXTSPRITE_PROP_BACKGROUND_COLOR_G = 64;
    public const int TEXTSPRITE_PROP_BACKGROUND_COLOR_B = 65;
    public const int TEXTSPRITE_PROP_BACKGROUND_COLOR_A = 66;

    public const int CAMERA_PROP_OFFSET_X = 67;
    public const int CAMERA_PROP_OFFSET_Y = 68;
    public const int CAMERA_PROP_OFFSET_Z = 69;

    public const int SPRITE_PROP_ALPHA2 = 70;

    public const int TEXTSPRITE_PROP_BORDER_OFFSET_X = 71;
    public const int TEXTSPRITE_PROP_BORDER_OFFSET_Y = 72;

    public const int CAMERA_PROP_OFFSET_ZOOM = 73;

    public const int TEXTSPRITE_PROP_STRING = 74;// warning: string pointer. DO NOT USE IN MACROEXECUTOR

    public const int PLAYBACK_NONE = 0;
    public const int PLAYBACK_PLAY = 1;
    public const int PLAYBACK_PAUSE = 2;
    public const int PLAYBACK_STOP = 3;
    public const int PLAYBACK_MUTE = 4;
    public const int PLAYBACK_UNMUTE = 5;

    public const int FONT_WORDBREAK_LOOSE = 0;
    public const int FONT_WORDBREAK_NONE = 1;
    public const int FONT_WORDBREAK_BREAK = 2;

    public const int TEXTSPRITE_FORCE_NONE = 0;
    public const int TEXTSPRITE_FORCE_LOWERCASE = 1;
    public const int TEXTSPRITE_FORCE_UPPERCASE = 2;



    public static int ParseSpriteProperty(XmlParserNode node, string name, bool warn) {
        string property = node.GetAttribute(name);

        if (String.IsNullOrEmpty(property)) {
            Console.Error.WriteLine(
                "[WARN] vertexprops_parse_sprite_property() missing " + name + " attribute" + "\n" + node.OuterHTML
            );
            return -2;
        }

        int id = ParseSpriteProperty2(property);
        if (id < 0 && warn)
            Console.Error.WriteLine("[WARN] vertexprops_parse_sprite_property() unknown property " + property + "\n" + node.OuterHTML);

        return id;
    }

    public static int ParseSpriteProperty2(string property) {
        property = property.ToLower();

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
            case "rotatepivotenabled":
                return SPRITE_PROP_ROTATE_PIVOT_ENABLED;
            case "rotatepivotu":
                return SPRITE_PROP_ROTATE_PIVOT_U;
            case "rotatepivotv":
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


    public static int ParseTextSpriteProperty(XmlParserNode node, string name, bool warn) {
        string property = node.GetAttribute(name);

        if (String.IsNullOrEmpty(property)) {
            Console.Error.WriteLine(
                "[WARN] vertexprops_parse_textsprite_property: missing " + name + " attribute" + "\n" + node.OuterHTML
            );
            return -2;
        }

        int id = ParseTextSpriteProperty2(property);
        if (id < 0 && warn)
            Console.Error.WriteLine("[WARN] vertexprops_parse_sprite_property() unknown property " + property + "\n" + node.OuterHTML);

        return id;
    }

    public static int ParseTextSpriteProperty2(string property) {
        property = property.ToLower();

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

    public static int ParseTextSpriteForceCase(XmlParserNode node, string name, bool warn) {
        string value = node.GetAttribute(name);
        if (String.IsNullOrEmpty(value)) return VertexProps.TEXTSPRITE_FORCE_NONE;

        int id = ParseTextSpriteForceCase2(value);
        if (id < 0) {
            if (warn) Console.Error.WriteLine("Unknown value " + value + " in: " + node.OuterHTML);
            return TEXTSPRITE_FORCE_NONE;
        }

        return id;
    }

    public static int ParseTextSpriteForceCase2(string value) {
        if (value == null) return -1;
        switch (value.ToLower()) {
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


    public static int ParseMediaProperty(XmlParserNode node, string name, bool warn) {
        string property = node.GetAttribute(name);

        if (String.IsNullOrEmpty(property)) {
            Console.Error.WriteLine(
                "[WARN] vertexprops_parse_media_property: missing " + name + " attribute" + "\n" + node.OuterHTML
            );
            return -2;
        }

        int id = ParseMediaProperty2(property);
        if (id < 0 && warn)
            Console.Error.WriteLine("[WARN] vertexprops_parse_media_property() unknown property " + property + "\n" + node.OuterHTML);

        return id;
    }

    public static int ParseMediaProperty2(string property) {
        property = property.ToLower();

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


    public static int ParseLayoutProperty(XmlParserNode node, string name, bool warn) {
        string property = node.GetAttribute(name);

        if (String.IsNullOrEmpty(property)) {
            Console.Error.WriteLine(
                "[WARN] vertexprops_parse_layout_property: missing " + name + " attribute" + "\n" + node.OuterHTML
            );
            return -2;
        }

        int id = ParseLayoutProperty2(property);
        if (id < 0 && warn)
            Console.Error.WriteLine("[WARN] vertexprops_parse_layout_property() unknown property " + property + "\n" + node.OuterHTML);

        return id;
    }

    public static int ParseLayoutProperty2(string property) {
        property = property.ToLowerInvariant();// in C dispose this variable!!!

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


    public static int ParseCameraProperty(XmlParserNode node, string name, bool warn) {
        string property = node.GetAttribute(name);

        if (String.IsNullOrEmpty(property)) {
            Console.Error.WriteLine(
                "[WARN] vertexprops_parse_camera_property: missing " + name + " attribute" + "\n" + node.OuterHTML
            );
            return -2;
        }

        int id = ParseCameraProperty2(property);
        if (id < 0 && warn)
            Console.Error.WriteLine("[WARN] vertexprops_parse_camera_property() unknown property " + property + "\n" + node.OuterHTML);

        return id;
    }

    public static int ParseCameraProperty2(string property) {
        if (!String.IsNullOrWhiteSpace(property)) {
            switch (property.ToLowerInvariant()) {
                case "offsetx":
                    return CAMERA_PROP_OFFSET_X;
                case "offsety":
                    return CAMERA_PROP_OFFSET_Y;
                case "offsetz":
                    return CAMERA_PROP_OFFSET_Z;
                case "offsetzoom":
                    return CAMERA_PROP_OFFSET_ZOOM;
            }
        }

        return -1;
    }


    public static Align ParseAlign(XmlParserNode node, string name, bool warn_missing, bool reject_bothnone) {
        string value = node.GetAttribute(name);

        if (String.IsNullOrEmpty(value)) {
            if (warn_missing) {
                Console.Error.WriteLine(
                    "[WARN] vertexprops_parse_align() missing " + name + " attribute" + "\n" + node.OuterHTML
                );
            }
            return Align.START;
        }

        Align id = ParseAlign2(value);

        if (reject_bothnone && (id == Align.BOTH || id == Align.NONE))
            id = Align.INVALID;
        else if (!reject_bothnone && id == Align.CENTER)
            id = Align.INVALID;

        if (id == Align.INVALID) {
            Console.Error.WriteLine("[WARN] vertexprops_parse_align() unknown align value: " + value + "\n" + node.OuterHTML);
            return Align.START;
        }

        return id;
    }

    public static Align ParseAlign2(string value) {
        if (!String.IsNullOrEmpty(value)) {
            switch (value.ToLower()) {
                case "none":
                    return Align.NONE;
                case "start":
                    return Align.START;
                case "center":
                    return Align.CENTER;
                case "end":
                    return Align.END;
                case "both":
                    return Align.BOTH;
            }
        }
        return Align.INVALID;
    }


    public static int ParsePlayback(XmlParserNode node, string name, bool warn_missing) {
        string value = node.GetAttribute(name);

        if (String.IsNullOrEmpty(value)) {
            if (warn_missing) {
                Console.Error.WriteLine(
                    "[WARN] vertexprops_parse_playback() missing " + name + " attribute" + "\n" + node.OuterHTML
                );
            }
            return PLAYBACK_NONE;
        }

        int id = ParsePlayback2(value);

        if (id == -1) {
            Console.Error.WriteLine("[WARN] vertexprops_parse_playback() unknown align value: " + value + "\n" + node.OuterHTML);
            return PLAYBACK_NONE;
        }

        return id;
    }

    public static int ParsePlayback2(string value) {
        if (!String.IsNullOrEmpty(value)) {
            switch (value.ToLower()) {
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


    public static bool ParseBoolean(XmlParserNode node, string attr_name, bool def_value) {
        string value = node.GetAttribute(attr_name);
        if (value == null) return def_value;

        return ParseBoolean2(value, def_value);
    }

    public static bool ParseBoolean2(string value, bool def_value) {
        if (value != null) {
            switch (value.ToLowerInvariant()) {
                case "1":
                case "true":
                    return true;
                case "0":
                case "false":
                    return false;
            }
        }
        return def_value;
    }


    public static int ParseInteger(XmlParserNode node, string attr_name, int def_value) {
        string value = node.GetAttribute(attr_name);
        if (String.IsNullOrEmpty(value)) return def_value;

        return ParseInteger2(value, def_value);
    }

    public static int ParseInteger2(string value, int def_value) {
        if (value == null) return def_value;

        int val;
        if (Int32.TryParse(value, FLAGS_INT, CultureInfo.InvariantCulture, out val))
            return val;
        else
            return def_value;
    }


    public static PVRContextFlag ParseFlag(XmlParserNode node, string attr_name, PVRContextFlag def_value) {
        string value = node.GetAttribute(attr_name);
        if (value == null) return def_value;

        return ParseFlag2(value, def_value);
    }

    public static PVRContextFlag ParseFlag2(string value, PVRContextFlag def_value) {
        if (value != null) {
            switch (value.ToLower()) {
                case "enabled":
                case "enable":
                    //case "true":
                    //case "1":
                    return PVRContextFlag.ENABLE;
                case "disabled":
                case "disable":
                    //case "false":
                    //case "0":
                    return PVRContextFlag.DISABLE;
                case "default":
                    //case "inherit":
                    //case "unset":
                    //case "2":
                    return PVRContextFlag.DEFAULT;
            }
        }
        return def_value;
    }


    public static uint ParseUnsignedInteger(string str, uint default_value) {
        uint val;
        if (UInt32.TryParse(str, NumberStyles.Integer, CultureInfo.InvariantCulture, out val))
            return val;
        else
            return default_value;
    }


    public static bool ParseHex(string str, out uint value, bool only_if_prefixed) {
        if (String.IsNullOrWhiteSpace(str)) goto L_invalid;

        int trim_index = 0;

        if (str.Length > 1 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
            trim_index = 2;
        } else if (str[0] == '#') {
            trim_index = 1;
        }

        if (only_if_prefixed && trim_index < 1) goto L_invalid;

        str = str.Substring(trim_index);
        if (str.Length < 1) goto L_invalid;

        if (!UInt32.TryParse(str, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out value))
            goto L_invalid;

        // add alpha component
        /*int string_length = matches[1].Length;
        if (string_length == 6) {
            value <<= 8;
            value |= 0xFF;
        }*/

        return true;

L_invalid:
        value = 0x0000;
        return false;
    }

    public static uint ParseHex2(string str, uint default_value, bool only_if_prefixed) {
        uint value;
        if (ParseHex(str, out value, only_if_prefixed))
            return value;
        else
            return default_value;
    }


    public static int ParseWordbreak(XmlParserNode node, string name, bool warn_missing) {
        string value = node.GetAttribute(name);

        if (String.IsNullOrEmpty(value)) {
            if (warn_missing) {
                Console.Error.WriteLine(
                    "[WARN] vertexprops_parse_wordbreak() missing " + name + " attribute" + "\n" + node.OuterHTML
                );
            }
            return FONT_WORDBREAK_BREAK;
        }

        int id = ParseWordbreak2(value);

        if (id == -1) {
            Console.Error.WriteLine("vertexprops_parse_wordbreak() unknown wordbreak value: " + value + "\n" + node.OuterHTML);
            return FONT_WORDBREAK_BREAK;
        }

        return id;
    }

    public static int ParseWordbreak2(string value) {
        if (!String.IsNullOrWhiteSpace(value)) {
            switch (value.ToLower()) {
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


    public static float ParseFloat(XmlParserNode node, string name, float default_value) {
        return ParseFloat2(node.GetAttribute(name), default_value);
    }

    public static float ParseFloat2(string value, float default_value) {
        float val;
        if (Single.TryParse(value, FLAGS_FLOAT, CultureInfo.InvariantCulture, out val)) return val;
        return default_value;
    }

    public static double ParseDouble(XmlParserNode node, string name, double def_value) {
        string value = node.GetAttribute(name);
        if (value == null || value.Length < 1) return def_value;
        return VertexProps.ParseDouble2(value, def_value);
    }

    public static double ParseDouble2(string value, double def_value) {
        double val;
        Double.TryParse(value, FLAGS_FLOAT, CultureInfo.InvariantCulture, out val);

        if (Double.IsNaN(val)) {
            Console.Error.WriteLine("[ERROR] vertexprops_parse_double2(): invalid value: " + value);
            return def_value;
        }

        return val;
    }

    public static Blend ParseBlending(string value) {
        switch (value) {
            case "ZERO":
                return Blend.ZERO;
            case "ONE":
                return Blend.ONE;
            case "SRC_COLOR":
                return Blend.SRC_COLOR;
            case "ONE_MINUS_SRC_COLOR":
                return Blend.ONE_MINUS_SRC_COLOR;
            case "DST_COLOR":
                return Blend.DST_COLOR;
            case "ONE_MINUS_DST_COLOR":
                return Blend.ONE_MINUS_DST_COLOR;
            case "SRC_ALPHA":
                return Blend.SRC_ALPHA;
            case "ONE_MINUS_SRC_ALPHA":
                return Blend.ONE_MINUS_SRC_ALPHA;
            case "DST_ALPHA":
                return Blend.DST_ALPHA;
            case "ONE_MINUS_DST_ALPHA":
                return Blend.ONE_MINUS_DST_ALPHA;
            case "CONSTANT_COLOR":
                return Blend.CONSTANT_COLOR;
            case "ONE_MINUS_CONSTANT_COLOR":
                return Blend.ONE_MINUS_CONSTANT_COLOR;
            case "CONSTANT_ALPHA":
                return Blend.CONSTANT_ALPHA;
            case "ONE_MINUS_CONSTANT_ALPHA":
                return Blend.ONE_MINUS_CONSTANT_ALPHA;
            case "SRC_ALPHA_SATURATE":
                return Blend.SRC_ALPHA_SATURATE;
            case null:
                return Blend.DEFAULT;
        }
        Console.Error.WriteLine("vertexprops_parse_blending() unknown blending: " + value);
        return Blend.INVALID_VALUE;
    }




    public static bool IsPropertyEnumerable(int property_id) {
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

    public static bool IsPropertyBoolean(int property_id) {
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

    public static bool IsValueInteger(string value) {
        if (value == null) return false;
        return Int64.TryParse(value, FLAGS_INT, CultureInfo.InvariantCulture, out _);
    }

    public static bool IsValueBoolean(string value) {
        if (value != null) {
            switch (value.ToLowerInvariant()) {
                case "true":
                case "0":
                case "1":
                case "false":
                    return true;
            }
        }
        return false;
    }

    public static long ParseLongInteger2(string value, long def_value) {
        if (String.IsNullOrEmpty(value)) return def_value;

        long val;
        if (Int64.TryParse(value, FLAGS_INT, CultureInfo.InvariantCulture, out val)) return val;

        Console.Error.WriteLine("[ERROR] layout_parse_integer2(): invalid value: " + value);
        return def_value;
    }


}

