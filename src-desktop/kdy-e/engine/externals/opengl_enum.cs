using System;

namespace Engine.Externals {

    public enum GLenum : uint {
        GL_NONE = 0,
        GL_STATIC_DRAW = 0x88E4,
        GL_PROXY_TEXTURE_2D = 0x8064,
        GL_TEXTURE_MIN_FILTER = 0x2801,
        GL_TEXTURE_MAG_FILTER = 0x2800,
        GL_ARRAY_BUFFER = 0x8892,
        GL_DRAW = 0x88E4,
        GL_CULL_FACE = 0x0B44,
        GL_FLOAT = 0x1406,
        GL_TRIANGLES = 0x0004,
        GL_TEXTURE_WRAP_S = 0x2802,
        GL_TEXTURE_WRAP_T = 0x2803,
        GL_LINK_STATUS = 0x8B82,
        GL_VERTEX_SHADER = 0x8B31,
        GL_FRAGMENT_SHADER = 0x8B30,
        GL_COMPILE_STATUS = 0x8B81,
        GL_COLOR_BUFFER_BIT = 0x00004000,
        GL_LINEAR = 0x2601,
        GL_NEAREST = 0x2600,
        GL_RGBA = 0x1908,
        GL_UNSIGNED_BYTE = 0x1401,
        GL_CLAMP_TO_EDGE = 0x812F,
        GL_SRC_ALPHA = 0x0302,
        GL_ONE_MINUS_SRC_ALPHA = 0x0303,
        GL_TRUE = 0x0001,
        GL_ONE = 0x0001,
        UNPACK_PREMULTIPLY_ALPHA_WEBGL = UInt32.MaxValue,// never used in desktop
        GL_DEPTH_TEST = 0x0B71,
        GL_BLEND = 0x0BE2,
        GL_INFO_LOG_LENGTH = 0x8B84,
        GL_TEXTURE_2D = 0x0DE1,
        GL_VERSION = 0x1F02,
        GL_INVALID_OPERATION = 0x0502,
        GL_SMOOTH = 0x1D01,
        GL_LEQUAL = 0x0203,
        GL_PERSPECTIVE_CORRECTION_HINT = 0x0C50,
        GL_NICEST = 0x1102,
        GL_DEPTH_BUFFER_BIT = 0x00000100,
        GL_PROJECTION = 0x1701,
        GL_MODELVIEW = 0x1700,
        GL_LUMINANCE = 0x1909,
        GL_TEXTURE0 = 0x84C0,
        GL_TEXTURE1 = 0x84C1,
        GL_ELEMENT_ARRAY_BUFFER = 0x8893,
        GL_DYNAMIC_DRAW = 0x88E8,
        GL_UNSIGNED_INT = 0x1405,
        GL_UNSIGNED_SHORT = 0x1403,
        GL_UNPACK_ALIGNMENT = 0x0CF5,
        GL_INVALID_ENUM = 0x0500,
        GL_INVALID_VALUE = 0x0501,
        GL_RED = 0x1903,
        GL_OUT_OF_MEMORY = 0x0505,
        GL_VIEWPORT = 0x0BA2,
        GL_PACK_ALIGNMENT = 0x0D05,
        GL_RGB = 0x1907
    }

    public partial class WebGLRenderingContext {

        public readonly GLenum STATIC_DRAW = GLenum.GL_STATIC_DRAW;
        public readonly GLenum PROXY_TEXTURE_2D = GLenum.GL_PROXY_TEXTURE_2D;
        public readonly GLenum TEXTURE_MIN_FILTER = GLenum.GL_TEXTURE_MIN_FILTER;
        public readonly GLenum TEXTURE_MAG_FILTER = GLenum.GL_TEXTURE_MAG_FILTER;
        public readonly GLenum ARRAY_BUFFER = GLenum.GL_ARRAY_BUFFER;
        public readonly GLenum _DRAW = GLenum.GL_DRAW;
        public readonly GLenum CULL_FACE = GLenum.GL_CULL_FACE;
        public readonly GLenum FLOAT = GLenum.GL_FLOAT;
        public readonly GLenum TRIANGLES = GLenum.GL_TRIANGLES;
        public readonly GLenum TEXTURE_WRAP_S = GLenum.GL_TEXTURE_WRAP_S;
        public readonly GLenum TEXTURE_WRAP_T = GLenum.GL_TEXTURE_WRAP_T;
        public readonly GLenum LINK_STATUS = GLenum.GL_LINK_STATUS;
        public readonly GLenum VERTEX_SHADER = GLenum.GL_VERTEX_SHADER;
        public readonly GLenum FRAGMENT_SHADER = GLenum.GL_FRAGMENT_SHADER;
        public readonly GLenum COMPILE_STATUS = GLenum.GL_COMPILE_STATUS;
        public readonly uint COLOR_BUFFER_BIT = (uint)GLenum.GL_COLOR_BUFFER_BIT;
        public readonly int LINEAR = (int)GLenum.GL_LINEAR;
        public readonly int NEAREST = (int)GLenum.GL_NEAREST;
        public readonly GLenum RGBA = GLenum.GL_RGBA;
        public readonly GLenum UNSIGNED_BYTE = GLenum.GL_UNSIGNED_BYTE;
        public readonly GLenum CLAMP_TO_EDGE = GLenum.GL_CLAMP_TO_EDGE;
        public readonly GLenum SRC_ALPHA = GLenum.GL_SRC_ALPHA;
        public readonly GLenum ONE_MINUS_SRC_ALPHA = GLenum.GL_ONE_MINUS_SRC_ALPHA;
        public readonly GLenum TRUE = GLenum.GL_TRUE;
        public readonly GLenum ONE = GLenum.GL_ONE;
        public readonly GLenum UNPACK_PREMULTIPLY_ALPHA_WEBGL = GLenum.UNPACK_PREMULTIPLY_ALPHA_WEBGL;
        public readonly GLenum DEPTH_TEST = GLenum.GL_DEPTH_TEST;
        public readonly GLenum BLEND = GLenum.GL_BLEND;
        public readonly GLenum INFO_LOG_LENGTH = GLenum.GL_INFO_LOG_LENGTH;
        public readonly GLenum TEXTURE_2D = GLenum.GL_TEXTURE_2D;
        public readonly GLenum LUMINANCE = GLenum.GL_LUMINANCE;
        public readonly GLenum TEXTURE0 = GLenum.GL_TEXTURE0;
        public readonly GLenum TEXTURE1 = GLenum.GL_TEXTURE1;
        public readonly GLenum ELEMENT_ARRAY_BUFFER = GLenum.GL_ELEMENT_ARRAY_BUFFER;
        public readonly GLenum DYNAMIC_DRAW = GLenum.GL_DYNAMIC_DRAW;
        public readonly GLenum UNSIGNED_INT = GLenum.GL_UNSIGNED_INT;
        public readonly GLenum UNSIGNED_SHORT = GLenum.GL_UNSIGNED_SHORT;
        public readonly GLenum UNPACK_ALIGNMENT = GLenum.GL_UNPACK_ALIGNMENT;
        public readonly GLenum RED = GLenum.GL_RED;
        public readonly GLenum OUT_OF_MEMORY = GLenum.GL_OUT_OF_MEMORY;
        public readonly GLenum NONE = GLenum.GL_NONE;
        public readonly GLenum VIEWPORT = GLenum.GL_VIEWPORT;
        public readonly GLenum PACK_ALIGNMENT = GLenum.GL_PACK_ALIGNMENT;
        public readonly GLenum RGB = GLenum.GL_RGB;
    }

}
