﻿using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#pragma warning disable CS0169 // El campos nunca se usan
#pragma warning disable CS0649 // El campos que nunca se asignan y siempre tendrán su valor predeterminado

namespace Engine.Externals.FontAtlasInterop.FreeTypeInterop;

//
// P/Invoke struct, enum, const and functions for FreeType 2.13.0
// Notes:
//       * some structs that are accessed using pointers are stubbed
//       * some structs are truncated becuase some fields are unused
//       * library update will require revise those modifications
//


/* FreetType primitives typedef */
using FT_Byte = Byte;
using FT_Error = Int32;
using FT_Fixed = Int32;
using FT_Int = Int32;
using FT_Int32 = Int32;
using FT_Long = Int32;
using FT_Pos = Int32;
using FT_Short = Int16;
using FT_String = Byte;
using FT_UInt = UInt32;
using FT_ULong = UInt32;
using FT_UShort = UInt16;
using FT_Glyph_Format = UInt32;

/* struct pointer stubs */
using FT_Generic_Finalizer = nint;
using static System.Runtime.InteropServices.JavaScript.JSType;


[StructLayout(LayoutKind.Sequential)]
internal struct FT_BBox {
    readonly FT_Pos xMin, yMin;
    readonly FT_Pos xMax, yMax;
}

[StructLayout(LayoutKind.Sequential)]
internal struct FT_Generic {
    readonly nint data;
    readonly FT_Generic_Finalizer finalizer;
}


[StructLayout(LayoutKind.Sequential)]
internal struct FT_Library { }

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct FT_Face {
    readonly FT_Long num_faces;
    readonly FT_Long face_index;

    readonly FT_Long face_flags;
    readonly FT_Long style_flags;

    readonly FT_Long num_glyphs;

    readonly FT_String* family_name;
    readonly FT_String* style_name;

    readonly FT_Int num_fixed_sizes;
    readonly nint available_sizes;

    readonly FT_Int num_charmaps;
    readonly nint charmaps;

    readonly FT_Generic generic;

    readonly FT_BBox bbox;

    readonly FT_UShort units_per_EM;
    public FT_Short ascender;
    readonly FT_Short descender;
    readonly FT_Short height;

    readonly FT_Short max_advance_width;
    readonly FT_Short max_advance_height;

    readonly FT_Short underline_position;
    readonly FT_Short underline_thickness;

    public FT_GlyphSlot* glyph;

    /*  truncated struct  truncated struct  truncated struct  truncated struct  */
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct FT_Glyph_Metrics {
    public FT_Pos width;
    public FT_Pos height;

    public FT_Pos horiBearingX;
    public FT_Pos horiBearingY;
    public FT_Pos horiAdvance;

    public FT_Pos vertBearingX;
    public FT_Pos vertBearingY;
    public FT_Pos vertAdvance;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct FT_Vector {
    public FT_Pos x;
    public FT_Pos y;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct FT_Bitmap {
    public uint rows;
    public uint width;
    readonly int pitch;
    public byte* buffer;
    readonly ushort num_grays;
    public FT_Pixel_Mode pixel_mode;

    /*  truncated struct  truncated struct  truncated struct  truncated struct  */
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct FT_GlyphSlot {
    readonly FT_Library* library;
    readonly FT_Face* face;
    readonly FT_GlyphSlot* next;
    readonly FT_UInt glyph_index;
    readonly FT_Generic generic;

    public FT_Glyph_Metrics metrics;
    readonly FT_Fixed linearHoriAdvance;
    readonly FT_Fixed linearVertAdvance;
    readonly FT_Vector advance;

    readonly FT_Glyph_Format format;

    public FT_Bitmap bitmap;
    public FT_Int bitmap_left;
    public FT_Int bitmap_top;

    /*  truncated struct  truncated struct  truncated struct  truncated struct  */
}


internal enum FT_Pixel_Mode : byte {
    NONE = 0,
    MONO,
    GRAY,
    GRAY2,
    GRAY4,
    LCD,
    LCD_V,
    BGRA,

    MAX
}

internal enum FT_Encoding : uint {
    UNICODE = ('u' << 24) | ('n' << 16) | ('i' << 8) | 'c'
}

internal enum FT_Render_Mode : uint {
    NORMAL = 0,
    LIGHT,
    MONO,
    LCD,
    LCD_V,
    SDF,

    MAX
}

internal enum FT_Kerning_Mode : uint {
    DEFAULT = 0,
    UNFITTED,
    UNSCALED
}


internal static unsafe partial class FreeType {

    private const string DLL = "freetype";// "freetype.dll"
    public const FT_Int32 FT_LOAD_RENDER = 1 << 2;


    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern FT_Error FT_Init_FreeType(FT_Library** alibrary);

    /*[LibraryImport(DLL)]
    [UnmanagedCallConv(CallConvs = new[] { typeof(CallConvCdecl) })]
    [return: MarshalAs(UnmanagedType.LPStr)]
    public static partial string FT_Error_String(FT_Error error_code);*/

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern FT_Error FT_New_Memory_Face(FT_Library* library, FT_Byte* file_base, FT_Long file_size, FT_Long face_index, FT_Face** aface);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern FT_Error FT_Select_Charmap(FT_Face* face, FT_Encoding encoding);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern FT_Error FT_Done_Face(FT_Face* face);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern FT_Error FT_Done_FreeType(FT_Library* library);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern FT_ULong FT_Get_First_Char(FT_Face* face, FT_UInt* agindex);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern FT_ULong FT_Get_Next_Char(FT_Face* face, FT_ULong char_code, FT_UInt* agindex);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern FT_Error FT_Load_Glyph(FT_Face* face, FT_UInt glyph_index, FT_Int32 load_flags);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern FT_Error FT_Render_Glyph(FT_GlyphSlot* slot, FT_Render_Mode render_mode);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern FT_Error FT_Get_Kerning(FT_Face* face, FT_UInt left_glyph, FT_UInt right_glyph, FT_Kerning_Mode kern_mode, FT_Vector* akerning);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern FT_Error FT_Set_Pixel_Sizes(FT_Face* face, FT_UInt pixel_width, FT_UInt pixel_height);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void FT_Library_Version(FT_Library* library, FT_Int* amajor, FT_Int* aminor, FT_Int* apatch);
    
    public static string FT_Error_String(FT_Error error_code) {
        switch (error_code) {
            case 0x00:
                return "no error";

            case 0x01:
                return "cannot open resource";
            case 0x02:
                return "unknown file format";
            case 0x03:
                return "broken file";
            case 0x04:
                return "invalid FreeType version";
            case 0x05:
                return "module version is too low";
            case 0x06:
                return "invalid argument";
            case 0x07:
                return "unimplemented feature";
            case 0x08:
                return "broken table";
            case 0x09:
                return "broken offset within table";
            case 0x0A:
                return "array allocation size too large";
            case 0x0B:
                return "missing module";
            case 0x0C:
                return "missing property";

            /* glyph/character errors */

            case 0x10:
                return "invalid glyph index";
            case 0x11:
                return "invalid character code";
            case 0x12:
                return "unsupported glyph image format";
            case 0x13:
                return "cannot render this glyph format";
            case 0x14:
                return "invalid outline";
            case 0x15:
                return "invalid composite glyph";
            case 0x16:
                return "too many hints";
            case 0x17:
                return "invalid pixel size";
            case 0x18:
                return "invalid SVG document";

            /* handle errors */

            case 0x20:
                return "invalid object handle";
            case 0x21:
                return "invalid library handle";
            case 0x22:
                return "invalid module handle";
            case 0x23:
                return "invalid face handle";
            case 0x24:
                return "invalid size handle";
            case 0x25:
                return "invalid glyph slot handle";
            case 0x26:
                return "invalid charmap handle";
            case 0x27:
                return "invalid cache manager handle";
            case 0x28:
                return "invalid stream handle";

            /* driver errors */

            case 0x30:
                return "too many modules";
            case 0x31:
                return "too many extensions";

            /* memory errors */

            case 0x40:
                return "out of memory";
            case 0x41:
                return "unlisted object";

            /* stream errors */

            case 0x51:
                return "cannot open stream";
            case 0x52:
                return "invalid stream seek";
            case 0x53:
                return "invalid stream skip";
            case 0x54:
                return "invalid stream read";
            case 0x55:
                return "invalid stream operation";
            case 0x56:
                return "invalid frame operation";
            case 0x57:
                return "nested frame access";
            case 0x58:
                return "invalid frame read";

            /* raster errors */

            case 0x60:
                return "raster uninitialized";
            case 0x61:
                return "raster corrupted";
            case 0x62:
                return "raster overflow";
            case 0x63:
                return "negative height while rastering";

            /* cache errors */

            case 0x70:
                return "too many registered caches";

            /* TrueType and SFNT errors */

            case 0x80:
                return "invalid opcode";
            case 0x81:
                return "too few arguments";
            case 0x82:
                return "stack overflow";
            case 0x83:
                return "code overflow";
            case 0x84:
                return "bad argument";
            case 0x85:
                return "division by zero";
            case 0x86:
                return "invalid reference";
            case 0x87:
                return "found debug opcode";
            case 0x88:
                return "found ENDF opcode in execution stream";
            case 0x89:
                return "nested DEFS";
            case 0x8A:
                return "invalid code range";
            case 0x8B:
                return "execution context too long";
            case 0x8C:
                return "too many function definitions";
            case 0x8D:
                return "too many instruction definitions";
            case 0x8E:
                return "SFNT font table missing";
            case 0x8F:
                return "horizontal header (hhea) table missing";
            case 0x90:
                return "locations (loca) table missing";
            case 0x91:
                return "name table missing";
            case 0x92:
                return "character map (cmap) table missing";
            case 0x93:
                return "horizontal metrics (hmtx) table missing";
            case 0x94:
                return "PostScript (post) table missing";
            case 0x95:
                return "invalid horizontal metrics";
            case 0x96:
                return "invalid character map (cmap) format";
            case 0x97:
                return "invalid ppem value";
            case 0x98:
                return "invalid vertical metrics";
            case 0x99:
                return "could not find context";
            case 0x9A:
                return "invalid PostScript (post) table format";
            case 0x9B:
                return "invalid PostScript (post) table";
            case 0x9C:
                return "found FDEF or IDEF opcode in glyf bytecode";
            case 0x9D:
                return "missing bitmap in strike";
            case 0x9E:
                return "SVG hooks have not been set";

            /* CFF, CID, and Type 1 errors */

            case 0xA0:
                return "opcode syntax error";
            case 0xA1:
                return "argument stack underflow";
            case 0xA2:
                return "ignore";
            case 0xA3:
                return "no Unicode glyph name found";
            case 0xA4:
                return "glyph too big for hinting";

            /* BDF errors */

            case 0xB0:
                return "`STARTFONT' field missing";
            case 0xB1:
                return "`FONT' field missing";
            case 0xB2:
                return "`SIZE' field missing";
            case 0xB3:
                return "`FONTBOUNDINGBOX' field missing";
            case 0xB4:
                return "`CHARS' field missing";
            case 0xB5:
                return "`STARTCHAR' field missing";
            case 0xB6:
                return "`ENCODING' field missing";
            case 0xB7:
                return "`BBX' field missing";
            case 0xB8:
                return "`BBX' too big";
            case 0xB9:
                return "Font header corrupted or missing fields";
            case 0xBA:
                return "Font glyphs corrupted or missing fields";
        }

        return null;
    }
}
