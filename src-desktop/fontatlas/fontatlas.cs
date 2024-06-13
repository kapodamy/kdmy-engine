using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Engine.Externals.FontAtlasInterop.FreeTypeInterop;

namespace Engine.Externals.FontAtlasInterop;


public struct FontCharDataKerning {
    public uint codepoint;
    public int x;
}

public struct FontCharDataAtlasEntry {
    public ushort x;
    public ushort y;
}

public class FontCharData {
    public uint codepoint;
    public short offset_x;
    public short offset_y;
    public short advancex;
    public int width;
    public int height;
    public FontCharDataKerning[] kernings;
    public int kernings_size;
    public FontCharDataAtlasEntry atlas_entry;
    public bool has_atlas_entry;
}

public class FontCharMap {
    public FontCharData[] char_array;
    public int char_array_size;
    public nint texture;
    public ushort texture_width;
    public ushort texture_height;
    public uint texture_byte_size;
    public short ascender;

    public void DestroyTextureOnly() {
        if (this.texture == 0x00) return;

        unsafe {
            NativeMemory.Free((void*)this.texture);
        }

        this.texture = 0x00;
        this.texture_byte_size = 0;
        this.texture_width = 0;
        this.texture_height = 0;
    }

    public void Destroy() {
        //for (int i = 0 ; i < this.char_array_size ; i++) {
        //    if (this.char_array[i].kernings == null) continue;
        //    free(this.char_array[i].kernings);
        //}
        unsafe {
            if (this.texture != 0x00) NativeMemory.Free((void*)this.texture);
        }
        //free(this.char_array);

        //free(this);
    }

}

public partial class FontAtlas {
    private unsafe byte* font;
    private int font_size;
    private unsafe FT_Library* lib;
    private unsafe FT_Face* face;
}

internal unsafe struct TextureAtlas {
    public byte* texture;
    public uint texture_byte_size;
    public int width;
    public int height;
}

internal unsafe struct CharData {
    public uint codepoint;
    public int offset_x;
    public int offset_y;
    public int advancex;
    public uint width;
    public uint height;
    public FontCharDataKerning[] kernings;
    public int kernings_size;
    public FontCharDataAtlasEntry atlas_entry;
    public bool has_atlas_entry;
    public byte* bitmap;
    public uint glyph_index;
}


public partial class FontAtlas {

    private const string FONTATLAS_BASE_LIST_COMMON = "  !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    private const string FONTATLAS_BASE_LIST_EXTENDED = "¿¡¢¥¦¤§¨©ª«»¬®¯°±´³²¹ºµ¶·ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßŸàáâãäåæçèéêëìíîïðñòóôõö×øùúûüýþßÿ";

    private const int MAX_TEXTURE_DIMMEN = 2048;
    private static bool sdf_enabled = false;

    private FontAtlas() { }

    public static void EnableSDF(bool enable) {
        FontAtlas.sdf_enabled = enable;
    }


    private static int Math2D_poweroftwo_calc(int dimmen) {
        int size = 2;
        while (size < 0x80000) {
            if (size >= dimmen) break;
            size *= 2;
        }

        if (size >= 0x80000) {
            throw new ArgumentException($"math2d_poweroftwo_calc() failed for: {dimmen}", nameof(dimmen));
        }
        return size;
    }


    public static FontAtlas Init(byte[] font_data, int font_data_size) {
        if (font_data_size < 1 || font_data == null) return null;

        unsafe {
            FontAtlas fontatlas = new FontAtlas();
            fontatlas.font = (byte*)NativeMemory.Alloc((nuint)font_data_size);
            fontatlas.font_size = font_data_size;

            int error;
            FT_Library* lib;
            FT_Face* face;

            Marshal.Copy(font_data, 0, (nint)fontatlas.font, font_data_size);

            if ((error = FreeType.FT_Init_FreeType(&lib)) != 0) {
                string e = FreeType.FT_Error_String(error);
                Logger.Error($"FontAtlas::Init() could not init FreeType Library: {e}");
                goto L_failed;
            }

            if ((error = FreeType.FT_New_Memory_Face(lib, fontatlas.font, font_data_size, 0, &face)) != 0) {
                string e = FreeType.FT_Error_String(error);
                Logger.Error($"FontAtlas::Init() failed to load font: {e}");
                goto L_failed;
            }

            FreeType.FT_Select_Charmap(face, FT_Encoding.UNICODE);

            fontatlas.lib = lib;
            fontatlas.face = face;

            return fontatlas;

L_failed:
            NativeMemory.Free(fontatlas.font);
            return null;

        }
    }

    public void Destroy() {
        unsafe {
            FreeType.FT_Done_Face(this.face);
            FreeType.FT_Done_FreeType(this.lib);
            NativeMemory.Free(this.font);

            this.face = null;
            this.lib = null;
            this.font = null;
        }
    }


    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private unsafe static bool PickGlyph(FT_Face* face, ref CharData chardata, uint codepoint) {
        uint charcode;
        uint glyph_index = 0x0000;

        charcode = FreeType.FT_Get_First_Char(face, &glyph_index);
        while (glyph_index != 0) {
            if (charcode == codepoint) break;
            charcode = FreeType.FT_Get_Next_Char(face, charcode, &glyph_index);
        }

        if (glyph_index == 0) {
            // no glyph for the codepoint
            goto L_failed;
        }

        int error = FreeType.FT_Load_Glyph(face, glyph_index, FreeType.FT_LOAD_RENDER);
        if (error != 0) {
            string e = FreeType.FT_Error_String(error);
            Logger.Warn($"FontAtlas::PickGlyph() failed to load glyph for codepoint {codepoint}, error: {e}");

            goto L_failed;
        }

        // Note: the glyph data is loaded in "this.face->glyph" field
        chardata.glyph_index = glyph_index;
        chardata.codepoint = codepoint;
        return false;

L_failed:
        chardata.width = chardata.height = 0;
        chardata.codepoint = 0x0000;
        return true;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private unsafe static void PickBitmap(FT_Face* face, ref CharData chardata, FT_GlyphSlot* glyph) {
        int error;

        if (sdf_enabled) {
            // 2 pass rendering. This use "bsdf" renderer which offers better quality
            error = FreeType.FT_Render_Glyph(glyph, FT_Render_Mode.NORMAL);
            if (error == 0) {
                error = FreeType.FT_Render_Glyph(glyph, FT_Render_Mode.SDF);
            }
        } else {
            error = FreeType.FT_Render_Glyph(glyph, FT_Render_Mode.NORMAL);
        }

        if (error != 0) {
            /*
            uint c = chardata.codepoint;
            string e = FreeType.FT_Error_String(error);
            Logger.Warn($"FontAtlas::PickBitmap() failed to pick the glyph bitmap of codepoint {c}, error: {e}");
            */
            chardata.bitmap = null;
            return;
        }

        byte* buffer = glyph->bitmap.buffer;
        uint width = glyph->bitmap.width;
        uint height = glyph->bitmap.rows;
        nuint texture_size = width * height;

        if (texture_size < 1 || buffer == null) {
            chardata.bitmap = null;
            return;
        }

        chardata.bitmap = (byte*)NativeMemory.Alloc(texture_size);
        //Debug.Assert(chardata->bitmap);

        NativeMemory.Copy(buffer, chardata.bitmap, texture_size);

        if (glyph->bitmap.pixel_mode != FT_Pixel_Mode.GRAY) {
            Logger.Warn($"FontAtlas::PickBitmap() glyph pixel_mode is not FT_PIXEL_MODE_GRAY in codepoint {chardata.codepoint}");
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private unsafe static void PickMetrics(FT_GlyphSlot* glyph, ref CharData chardata, int font_height) {
        // just in case, this never should happen
        //Debug.Assert(glyph != null);

        chardata.width = glyph->bitmap.width;// glyph->metrics.width >> 6;
        chardata.height = glyph->bitmap.rows;// glyph->metrics.height >> 6;
        chardata.offset_x = glyph->bitmap_left;
        chardata.offset_y = font_height - (glyph->metrics.horiBearingY >> 6);
        chardata.advancex = glyph->metrics.horiAdvance >> 6;
        chardata.kernings = null;
        chardata.kernings_size = 0;
        chardata.has_atlas_entry = false;
    }


    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private unsafe static void PickKernings(FT_Face* face, CharData[] chardata_array, int chardata_array_size) {
        FT_Vector kerning;

        for (int i = 0 ; i < chardata_array_size ; i++) {
            uint current = chardata_array[i].glyph_index;
            int kernings_count = 0;

            for (int j = 0 ; j < chardata_array_size ; j++) {
                uint previous = chardata_array[j].glyph_index;

                int error = FreeType.FT_Get_Kerning(face, previous, current, FT_Kerning_Mode.DEFAULT, &kerning);
                if (error != 0) {
                    /*
                    uint p = previous;
                    uint c = current;
                    string e = FreeType.FT_Error_String(error);
                    Logger.Warn($"FontAtlas::PickKernings() cannot load kernigs of previous={p} current={c}: {e}");
                    */
                    continue;
                }

                long kerning_x = kerning.x >> 6;
                if (kerning_x == 0) continue;

                kernings_count++;
            }

            if (kernings_count < 1) {
                chardata_array[i].kernings_size = 0;
                chardata_array[i].kernings = null;
                continue;
            }

            int kerning_index = 0;
            FontCharDataKerning[] kernings = new FontCharDataKerning[kernings_count];
            //Debug.Assert(kernings != null);

            for (int j = 0 ; j < chardata_array_size ; j++) {
                uint previous = chardata_array[j].glyph_index;

                if (FreeType.FT_Get_Kerning(face, previous, current, FT_Kerning_Mode.DEFAULT, &kerning) != 0) continue;

                long kerning_x = kerning.x >> 6;
                if (kerning_x == 0) continue;

                kernings[kerning_index].codepoint = chardata_array[j].codepoint;
                kernings[kerning_index].x = kerning.x >> 6;
                kerning_index++;
            }

            chardata_array[i].kernings_size = kernings_count;
            chardata_array[i].kernings = kernings;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static void BuildAtlas(CharData[] array, int array_size, int gaps, int max_dimmen, ref TextureAtlas atlas) {
        int[] rects_index = new int[array_size];

        for (int i = 0 ; i < array_size ; i++) {
            array[i].has_atlas_entry = false;
            array[i].atlas_entry.x = 0;
            array[i].atlas_entry.y = 0;
            rects_index[i] = i;
        }

        Span<int> rects_index_span = new Span<int>(rects_index, 0, array_size);
        rects_index_span.Sort(delegate (int idx_a, int idx_b) {
            uint height_a = array[idx_a].height;
            uint height_b = array[idx_b].height;

            return height_a.CompareTo(height_b);
        });

        int x = 0;
        int y = 0;
        int max_row_height = 0;
        int max_width = 0;
        int max_height = 0;

        for (int i = 0 ; i < array_size ; i++) {
            ref CharData chardata = ref array[rects_index[i]];

            if (chardata.codepoint == 0) continue;
            if (chardata.width < 1 || chardata.height < 1) continue;

            int rect_width = (int)(chardata.width + gaps);
            int rect_height = (int)(chardata.height + gaps);

            int right = x + rect_width;
            if (right > max_dimmen) {
                y += max_row_height;
                x = 0;
                max_row_height = 0;
            }

            int bottom = y + rect_height;
            if (bottom > max_dimmen) break;

            chardata.atlas_entry.x = (ushort)x;
            chardata.atlas_entry.y = (ushort)y;
            x += rect_width;

            if (rect_height > max_row_height) max_row_height = rect_height;
            if (right > max_width) max_width = right;
            if (bottom > max_height) max_height = bottom;

            chardata.has_atlas_entry = true;
        }

        atlas.width = max_width;
        atlas.height = max_height;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private unsafe static void PutInAtlasTexture(ref CharData chardata, ref TextureAtlas atlas) {
        if (atlas.texture == null || !chardata.has_atlas_entry || chardata.bitmap == null) return;

        byte* buffer = chardata.bitmap;
        uint char_width = chardata.width;
        uint char_height = chardata.height;
        int atlas_y = chardata.atlas_entry.y;

        for (uint y = 0 ; y < char_height ; y++) {
            int offset = chardata.atlas_entry.x + (atlas_y * atlas.width);
            if (offset >= atlas.texture_byte_size) break;

            NativeMemory.Copy(buffer, atlas.texture + offset, char_width);

            buffer += char_width;
            atlas_y++;
        }

        NativeMemory.Free(chardata.bitmap);
        chardata.bitmap = null;
    }


    public FontCharMap AtlasBuild(byte font_height, byte gaps, uint[] codepoints_to_add) {
        if (codepoints_to_add.Length < 1 || codepoints_to_add[0] == 0) return null;

        int codepoints_parsed = 0;
        int codepoints_count = 0;
        for (int i = 0 ; codepoints_to_add[i] != 0 ; i++) codepoints_count++;

        unsafe {
            CharData[] chardata = new CharData[codepoints_count];
            int error;
            TextureAtlas atlas = new() { texture_byte_size = 0, texture = null, width = 0, height = 0 };
            FT_GlyphSlot* glyph = this.face->glyph;

            //Debug.Assert(chardata != null);

            if ((error = FreeType.FT_Set_Pixel_Sizes(this.face, 0, font_height)) != 0) {
                string s = FreeType.FT_Error_String(error);
                Logger.Warn($"FontAtlas::AtlasBuild() failed to use font_height {font_height} using 64 instead, error: {s}");

                // fallback to 64px
                FreeType.FT_Set_Pixel_Sizes(this.face, 0, 64);
            }

            for (int i = 0 ; i < codepoints_count ; i++) {
                if (FontAtlas.PickGlyph(this.face, ref chardata[i], codepoints_to_add[i])) continue;
                FontAtlas.PickBitmap(this.face, ref chardata[i], glyph);
                FontAtlas.PickMetrics(glyph, ref chardata[i], font_height);
                codepoints_parsed++;
            }

            for (int i = 0 ; i < codepoints_count ; i++) {
                FontAtlas.PickKernings(this.face, chardata, codepoints_count);
            }

            if (codepoints_parsed < 1) goto L_build_map;

            // calculate texture dimmensions
            FontAtlas.BuildAtlas(chardata, codepoints_count, gaps, MAX_TEXTURE_DIMMEN, ref atlas);
            if (atlas.width < 1 || atlas.height < 1) goto L_build_map;

            // calculate power of two dimmensions
            atlas.width = Math2D_poweroftwo_calc(atlas.width);
            atlas.height = Math2D_poweroftwo_calc(atlas.height);
            if (atlas.width > MAX_TEXTURE_DIMMEN) atlas.width = MAX_TEXTURE_DIMMEN;
            if (atlas.height > MAX_TEXTURE_DIMMEN) atlas.height = MAX_TEXTURE_DIMMEN;

            // allocate texture
            atlas.texture_byte_size = (uint)(atlas.width * atlas.height);
            atlas.texture = (byte*)NativeMemory.AllocZeroed(1, (nuint)atlas.texture_byte_size);
            //Debug.Assert(atlas.texture, "Texture creation failed, not enough memory");

            // place glyph bitmaps in the texture
            for (int i = 0 ; i < codepoints_count ; i++) {
                if (chardata[i].codepoint == 0 || chardata[i].width < 1 || chardata[i].height < 1) continue;
                FontAtlas.PutInAtlasTexture(ref chardata[i], ref atlas);
            }


L_build_map:
            FontCharMap obj = new FontCharMap();
            obj.char_array_size = codepoints_parsed;
            obj.char_array = new FontCharData[codepoints_parsed];
            obj.texture_byte_size = atlas.texture_byte_size;
            obj.texture = (nint)atlas.texture;
            obj.texture_width = (ushort)atlas.width;
            obj.texture_height = (ushort)atlas.height;
            obj.ascender = (short)(this.face->ascender / 64);

            //if (codepoints_parsed > 0) Debug.Assert(obj.char_array != null);

            // place character data in the map
            for (int i = 0, j = 0 ; i < codepoints_count ; i++) {
                if (chardata[i].codepoint == 0) continue;

                obj.char_array[j] = new FontCharData() {
                    advancex = (short)chardata[i].advancex,
                    atlas_entry = chardata[i].atlas_entry,
                    codepoint = chardata[i].codepoint,
                    has_atlas_entry = chardata[i].has_atlas_entry,
                    height = (int)chardata[i].height,
                    kernings = chardata[i].kernings,
                    kernings_size = chardata[i].kernings_size,
                    offset_x = (short)chardata[i].offset_x,
                    offset_y = (short)chardata[i].offset_y,
                    width = (int)chardata[i].width,
                };
                j++;
            }

            return obj;
        }
    }

    public FontCharMap AtlasBuildComplete(byte font_height, byte gaps) {
        int size_common = FONTATLAS_BASE_LIST_COMMON.Length;
        int size_extended = FONTATLAS_BASE_LIST_EXTENDED.Length;
        int size_total = size_extended + size_common;
        uint[] list_complete = new uint[size_total + 1];
        uint j = 0;

        for (int i = 0 ; i < size_common ; i++) list_complete[j++] = (uint)Char.ConvertToUtf32(FONTATLAS_BASE_LIST_COMMON, i);
        for (int i = 0 ; i < size_extended ; i++) list_complete[j++] = (uint)Char.ConvertToUtf32(FONTATLAS_BASE_LIST_EXTENDED, i);
        list_complete[size_total] = 0;

        FontCharMap charmap = this.AtlasBuild(font_height, gaps, list_complete);
        //free(list_complete);

        return charmap;
    }


    public static string GetVersion() {
        unsafe {
            FT_Library* lib;
            int amajor, aminor, apatch;

            int error = FreeType.FT_Init_FreeType(&lib);
            if (error != 0) return null;

            FreeType.FT_Library_Version(lib, &amajor, &aminor, &apatch);
            FreeType.FT_Done_FreeType(lib);

            return $"{amajor}.{aminor}.{apatch}";
        }
    }

}
