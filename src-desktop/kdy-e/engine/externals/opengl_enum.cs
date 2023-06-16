﻿using System;

namespace Engine.Externals;

public enum GLenum : uint {
    // ---------------- GL_EXT_texture_compression_s3tc ----------------
    // https://registry.khronos.org/OpenGL/extensions/EXT/EXT_texture_compression_s3tc.txt
    GL_COMPRESSED_RGB_S3TC_DXT1_EXT = 0x83F0,
    GL_COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1,
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT = 0x83F2,
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3,
    // ---------------- GL_EXT_texture_compression_s3tc ----------------

    // ---------------- GL_EXT_texture_compression_bptc ----------------
    // https://registry.khronos.org/OpenGL/extensions/EXT/EXT_texture_compression_bptc.txt
    GL_COMPRESSED_RGBA_BPTC_UNORM_ARB = 0x8E8C,
    GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB = 0x8E8D,
    GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB = 0x8E8E,
    GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB = 0x8E8F,
    // ---------------- GL_EXT_texture_compression_bptc ----------------

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
    GL_LINEAR_MIPMAP_LINEAR = 0x2703,
    GL_LINEAR_MIPMAP_NEAREST = 0x2701,
    GL_NEAREST = 0x2600,
    GL_NEAREST_MIPMAP_LINEAR = 0x2702,
    GL_NEAREST_MIPMAP_NEAREST = 0x2700,
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
    GL_RGB = 0x1907,
    GL_FLOAT_MAT2 = 0x8B5A,
    GL_FLOAT_MAT2x3 = 0x8B65,
    GL_FLOAT_MAT2x4 = 0x8B66,
    GL_FLOAT_MAT3 = 0x8B5B,
    GL_FLOAT_MAT3x2 = 0x8B67,
    GL_FLOAT_MAT3x4 = 0x8B68,
    GL_FLOAT_MAT4 = 0x8B5C,
    GL_FLOAT_MAT4x2 = 0x8B69,
    GL_FLOAT_MAT4x3 = 0x8B6A,
    GL_FLOAT_VEC2 = 0x8B50,
    GL_FLOAT_VEC3 = 0x8B51,
    GL_FLOAT_VEC4 = 0x8B52,
    GL_INT = 0x1404,
    GL_INT_VEC2 = 0x8B53,
    GL_INT_VEC3 = 0x8B54,
    GL_INT_VEC4 = 0x8B55,
    GL_BOOL = 0x8B56,
    GL_BOOL_VEC2 = 0x8B57,
    GL_BOOL_VEC3 = 0x8B58,
    GL_BOOL_VEC4 = 0x8B59,
    GL_UNSIGNED_INT_VEC2 = 0x8DC6,
    GL_UNSIGNED_INT_VEC3 = 0x8DC7,
    GL_UNSIGNED_INT_VEC4 = 0x8DC8,
    GL_ACTIVE_UNIFORMS = 0x8B86,
    GL_FRAMEBUFFER = 0x8D40,
    GL_FRAMEBUFFER_COMPLETE = 0x8CD5,
    GL_COLOR_ATTACHMENT0 = 0x8CE0,
    GL_ACTIVE_UNIFORM_MAX_LENGTH = 0x8B87,
    GL_FRAMEBUFFER_BINDING = 0x8CA6,
    GL_ZERO = 0,
    GL_SRC_ALPHA_SATURATE = 0x0308,
    GL_SRC_COLOR = 0x0300,
    GL_ONE_MINUS_CONSTANT_ALPHA = 0x8004,
    GL_ONE_MINUS_CONSTANT_COLOR = 0x8002,
    GL_ONE_MINUS_DST_ALPHA = 0x0305,
    GL_ONE_MINUS_DST_COLOR = 0x0307,
    GL_ONE_MINUS_SRC1_ALPHA = 0x88FB,
    GL_ONE_MINUS_SRC1_COLOR = 0x88FA,
    GL_ONE_MINUS_SRC_COLOR = 0x0301,
    GL_DST_ALPHA = 0x0304,
    GL_DST_COLOR = 0x0306,
    GL_CONSTANT_ALPHA = 0x8003,
    GL_CONSTANT_COLOR = 0x8001,
    GL_PIXEL_UNPACK_BUFFER = 0x88EC,
    GL_STREAM_DRAW = 0x88E0,
    GL_WRITE_ONLY = 0x88B9,
    GL_NUM_EXTENSIONS = 0x821D,
    GL_EXTENSIONS = 0x1F03
}

public partial class WebGL2RenderingContext {

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
    public readonly GLenum LINEAR = GLenum.GL_LINEAR;
    public readonly GLenum LINEAR_MIPMAP_LINEAR = GLenum.GL_LINEAR_MIPMAP_LINEAR;
    public readonly GLenum LINEAR_MIPMAP_NEAREST = GLenum.GL_LINEAR_MIPMAP_NEAREST;
    public readonly GLenum NEAREST = GLenum.GL_NEAREST;
    public readonly GLenum NEAREST_MIPMAP_LINEAR = GLenum.GL_NEAREST_MIPMAP_LINEAR;
    public readonly GLenum NEAREST_MIPMAP_NEAREST = GLenum.GL_NEAREST_MIPMAP_NEAREST;
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
    public readonly GLenum FLOAT_MAT2 = GLenum.GL_FLOAT_MAT2;
    public readonly GLenum FLOAT_MAT2x3 = GLenum.GL_FLOAT_MAT2x3;
    public readonly GLenum FLOAT_MAT2x4 = GLenum.GL_FLOAT_MAT2x4;
    public readonly GLenum FLOAT_MAT3 = GLenum.GL_FLOAT_MAT3;
    public readonly GLenum FLOAT_MAT3x2 = GLenum.GL_FLOAT_MAT3x2;
    public readonly GLenum FLOAT_MAT3x4 = GLenum.GL_FLOAT_MAT3x4;
    public readonly GLenum FLOAT_MAT4 = GLenum.GL_FLOAT_MAT4;
    public readonly GLenum FLOAT_MAT4x2 = GLenum.GL_FLOAT_MAT4x2;
    public readonly GLenum FLOAT_MAT4x3 = GLenum.GL_FLOAT_MAT4x3;
    public readonly GLenum FLOAT_VEC2 = GLenum.GL_FLOAT_VEC2;
    public readonly GLenum FLOAT_VEC3 = GLenum.GL_FLOAT_VEC3;
    public readonly GLenum FLOAT_VEC4 = GLenum.GL_FLOAT_VEC4;
    public readonly GLenum INT = GLenum.GL_INT;
    public readonly GLenum INT_VEC2 = GLenum.GL_INT_VEC2;
    public readonly GLenum INT_VEC3 = GLenum.GL_INT_VEC3;
    public readonly GLenum INT_VEC4 = GLenum.GL_INT_VEC4;
    public readonly GLenum BOOL = GLenum.GL_BOOL;
    public readonly GLenum BOOL_VEC2 = GLenum.GL_BOOL_VEC2;
    public readonly GLenum BOOL_VEC3 = GLenum.GL_BOOL_VEC3;
    public readonly GLenum BOOL_VEC4 = GLenum.GL_BOOL_VEC4;
    public readonly GLenum UNSIGNED_INT_VEC2 = GLenum.GL_UNSIGNED_INT_VEC2;
    public readonly GLenum UNSIGNED_INT_VEC3 = GLenum.GL_UNSIGNED_INT_VEC3;
    public readonly GLenum UNSIGNED_INT_VEC4 = GLenum.GL_UNSIGNED_INT_VEC4;
    public readonly GLenum ACTIVE_UNIFORMS = GLenum.GL_ACTIVE_UNIFORMS;
    public readonly GLenum FRAMEBUFFER = GLenum.GL_FRAMEBUFFER;
    public readonly GLenum FRAMEBUFFER_COMPLETE = GLenum.GL_FRAMEBUFFER_COMPLETE;
    public readonly GLenum COLOR_ATTACHMENT0 = GLenum.GL_COLOR_ATTACHMENT0;
    public readonly GLenum ACTIVE_UNIFORM_MAX_LENGTH = GLenum.GL_ACTIVE_UNIFORM_MAX_LENGTH;
    public readonly GLenum FRAMEBUFFER_BINDING = GLenum.GL_FRAMEBUFFER_BINDING;
    public readonly GLenum ZERO = GLenum.GL_ZERO;
    public readonly GLenum SRC_ALPHA_SATURATE = GLenum.GL_SRC_ALPHA_SATURATE;
    public readonly GLenum SRC_COLOR = GLenum.GL_SRC_COLOR;
    public readonly GLenum ONE_MINUS_CONSTANT_ALPHA = GLenum.GL_ONE_MINUS_CONSTANT_ALPHA;
    public readonly GLenum ONE_MINUS_CONSTANT_COLOR = GLenum.GL_ONE_MINUS_CONSTANT_COLOR;
    public readonly GLenum ONE_MINUS_DST_ALPHA = GLenum.GL_ONE_MINUS_DST_ALPHA;
    public readonly GLenum ONE_MINUS_DST_COLOR = GLenum.GL_ONE_MINUS_DST_COLOR;
    public readonly GLenum ONE_MINUS_SRC1_ALPHA = GLenum.GL_ONE_MINUS_SRC1_ALPHA;
    public readonly GLenum ONE_MINUS_SRC1_COLOR = GLenum.GL_ONE_MINUS_SRC1_COLOR;
    public readonly GLenum ONE_MINUS_SRC_COLOR = GLenum.GL_ONE_MINUS_SRC_COLOR;
    public readonly GLenum DST_ALPHA = GLenum.GL_DST_ALPHA;
    public readonly GLenum DST_COLOR = GLenum.GL_DST_COLOR;
    public readonly GLenum CONSTANT_ALPHA = GLenum.GL_CONSTANT_ALPHA;
    public readonly GLenum CONSTANT_COLOR = GLenum.GL_CONSTANT_COLOR;
    public readonly GLenum PIXEL_UNPACK_BUFFER = GLenum.GL_PIXEL_UNPACK_BUFFER;
    public readonly GLenum STREAM_DRAW = GLenum.GL_STREAM_DRAW;
    public readonly GLenum WRITE_ONLY = GLenum.GL_WRITE_ONLY;
}
