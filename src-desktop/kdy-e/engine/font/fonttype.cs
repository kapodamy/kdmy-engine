using System;
using System.Runtime.CompilerServices;
using Engine.Externals;
using Engine.Externals.FontAtlasInterop;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;
using KallistiOS.MUTEX;

namespace Engine.Font;

public class FontType : IFont {

    private const byte GLYPHS_HEIGHT = 72;// in the dreamcast use 32px, 64px is enough for SDF
    private const byte GLYPHS_SDF_SIZE = GLYPHS_HEIGHT >> 2;// 25% of FONTTYPE_GLYPHS_HEIGHT
    private const byte GLYPHS_GAPS = 16;// space between glyph in pixels (must be high for SDF)
    private const float FAKE_SPACE = 0.75f;// 75% of the height

    private static Map<FontType> POOL = new Map<FontType>();
    private static int IDS = 0;
    private static mutex_t fontatlas_mutex = mutex.INITIALIZER();

    private FontType() { }

    private int instance_id;
    private int instance_references;
    private string instance_path;
    private FontAtlas fontatlas;
    private byte[] font;
    private float space_width;
    private FontCharMap fontcharmap_primary;
    private Texture fontcharmap_primary_texture;
    private FontCharMap fontcharmap_secondary;
    private Texture fontcharmap_secondary_texture;
    private byte[] lookup_table;


    public static FontType Init(string src) {
        string full_path = FS.GetFullPathAndOverride(src);

        foreach (FontType instance in FontType.POOL) {
            if (instance.instance_path == full_path) {
                //free(full_path);
                instance.instance_references++;
                return instance;
            }
        }

        FontType fonttype = new FontType() {
            instance_id = FontType.IDS++,
            instance_references = 1,
            instance_path = full_path,

            fontatlas = null,
            space_width = FontType.GLYPHS_HEIGHT * FontType.FAKE_SPACE,

            fontcharmap_primary = null,
            fontcharmap_primary_texture = null,

            fontcharmap_secondary = null,
            fontcharmap_secondary_texture = null,

            lookup_table = new byte[FontGlyph.LOOKUP_TABLE_LENGTH]
        };

        Array.Fill(fonttype.lookup_table, FontGlyph.LOOKUP_TABLE_LENGTH, 0, FontGlyph.LOOKUP_TABLE_LENGTH);

        // initialize FreeType library
        if (fonttype.InternalInitFreetype(src)) {
            //free(full_path);
            //free(fonttype.font);
            //free(fonttype);
            return null;
        }

        // create a texture atlas and glyphs map with all common letters, numbers and symbols
        fonttype.fontcharmap_primary = fonttype.InternalRetrieveFontcharmap(null);
        fonttype.fontcharmap_primary_texture = FontType.InternalUploadTexture(fonttype.fontcharmap_primary);

        if (fonttype.fontcharmap_primary != null) {
            FontCharData[] char_array = fonttype.fontcharmap_primary.char_array;
            int char_array_size = fonttype.fontcharmap_primary.char_array_size;

            for (int i = 0 ; i < char_array_size ; i++) {
                if (char_array[i].codepoint == FontGlyph.SPACE) {
                    fonttype.space_width = char_array[i].advancex;
                    break;
                }
            }

            for (byte i = 0 ; i < char_array_size && i <= FontGlyph.LOOKUP_TABLE_LENGTH ; i++) {
                uint codepoint = char_array[i].codepoint;
                if (codepoint < FontGlyph.LOOKUP_TABLE_LENGTH) {
                    fonttype.lookup_table[codepoint] = i;
                }
            }
        }

        FontType.POOL.Set(fonttype.instance_id, fonttype);
        return fonttype;
    }

    public void Destroy() {
        //if (this == null) return;

        this.instance_references--;
        if (this.instance_references > 0) return;
        FontType.POOL.Delete(this.instance_id);

        if (this.fontcharmap_primary != null) {
            this.fontcharmap_primary.Destroy();
            if (this.fontcharmap_primary_texture != null) this.fontcharmap_primary_texture.Destroy();
        }

        if (this.fontcharmap_secondary != null) {
            this.fontcharmap_secondary.Destroy();
            if (this.fontcharmap_secondary_texture != null) this.fontcharmap_secondary_texture.Destroy();
        }

        if (this.fontatlas != null) this.fontatlas.Destroy();


        //free(this.font);
        //free(this.instance_path);
        //free(this);
    }


    public float Measure(ref FontParams @params, string text, int text_index, int text_length) {
        float scale = @params.height / FontType.GLYPHS_HEIGHT;
        int text_end_index = text_index + text_length;

        float max_width = 0;
        float width = 0;
        int index = text_index;
        int previous_codepoint = 0;
        int line_chars = 0;
        Grapheme grapheme = new Grapheme();

        while (index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, ref grapheme)) {
            index += grapheme.size;

            //override hard-spaces with white-spaces
            if (grapheme.code == 0xA0) grapheme.code = 0x20;

            // ignore "\r" characters
            if (grapheme.code == FontGlyph.CARRIAGERETURN) {
                previous_codepoint = grapheme.code;
                continue;
            }

            if (grapheme.code == FontGlyph.LINEFEED) {
                if (width > max_width) max_width = width;
                width = 0;
                previous_codepoint = grapheme.code;
                line_chars = 0;
                continue;
            }

            FontCharData fontchardata = InternalGetFontchardata2(this.lookup_table, this.fontcharmap_primary, grapheme.code);
            if (fontchardata == null) {
                fontchardata = InternalGetFontchardata(this.fontcharmap_secondary, grapheme.code);
                if (fontchardata == null) {
                    if (grapheme.code == FontGlyph.TAB) {
                        int filler = FontGlyph.InternalCalcTabstop(line_chars);
                        if (filler > 0) {
                            width += this.space_width * filler * scale;
                            line_chars += filler;
                        }
                    } else {
                        // space, hard space or unknown characters
                        width += this.space_width * scale;
                        line_chars++;
                    }
                }

                continue;
            }

            if (previous_codepoint != 0x0000) {
                // compute kerning
                for (int i = 0 ; i < fontchardata.kernings_size ; i++) {
                    if (fontchardata.kernings[i].codepoint == previous_codepoint) {
                        width += fontchardata.kernings[i].x * scale;
                        break;
                    }
                }
            }

            width += fontchardata.advancex * scale;
            previous_codepoint = grapheme.code;
            line_chars++;
        }

        return Math.Max(width, max_width);
    }

    public void MeasureChar(int codepoint, float height, ref FontLineInfo lineinfo) {
        float scale = height / FontType.GLYPHS_HEIGHT;

        //override hard-spaces with white-spaces
        if (codepoint == 0xA0) codepoint = 0x20;

        FontCharData fontchardata = FontType.InternalGetFontchardata2(this.lookup_table, this.fontcharmap_primary, codepoint);
        if (fontchardata == null) {
            fontchardata = FontType.InternalGetFontchardata(this.fontcharmap_secondary, codepoint);
            if (fontchardata == null) {
                if (codepoint == FontGlyph.TAB) {
                    int filler = FontGlyph.InternalCalcTabstop(lineinfo.line_char_count);
                    if (filler > 0) {
                        lineinfo.last_char_width = this.space_width * filler * scale;
                        lineinfo.line_char_count += filler;
                    }
                } else {
                    // space, hard space or unknown characters
                    lineinfo.last_char_width = this.space_width * scale;
                    lineinfo.line_char_count++;
                }
            }
            return;
        }

        if (lineinfo.previous_codepoint != 0x0000) {
            // compute kerning
            for (int i = 0 ; i < fontchardata.kernings_size ; i++) {
                if (fontchardata.kernings[i].codepoint == lineinfo.previous_codepoint) {
                    lineinfo.last_char_width = fontchardata.kernings[i].x * scale;
                    break;
                }
            }
        }

        lineinfo.last_char_width = fontchardata.advancex * scale;
        lineinfo.previous_codepoint = codepoint;
        lineinfo.line_char_count++;
    }

    public float DrawText(PVRContext pvrctx, ref FontParams @params, float x, float y, int text_index, int text_length, string text) {
        if (text == null || text_length < 1) return 0;

        Grapheme grapheme = new Grapheme();
        FontCharMap primary = this.fontcharmap_primary;
        Texture primary_texture = this.fontcharmap_primary_texture;
        FontCharMap secondary = this.fontcharmap_secondary;
        Texture secondary_texture = this.fontcharmap_secondary_texture;
        bool has_border = @params.border_enable && @params.border_color[3] > 0f && @params.border_size >= 0f;
        float outline_size = @params.border_size * 2;
        float scale = @params.height / FontType.GLYPHS_HEIGHT;
        float ascender = ((primary ?? secondary).ascender / 2f) * scale;// FIXME: ¿why does dividing by 2 works?
        int text_end_index = text_index + text_length;

#if SDF_FONT
        if (has_border) {
            // calculate sdf padding
            float padding;
            padding = @params.border_size / FontType.GLYPHS_SDF_SIZE;
            padding /= @params.height / FontType.GLYPHS_HEIGHT;

            GlyphRenderer.SetParamsSDF(pvrctx, FontType.GLYPHS_SDF_SIZE, padding);
        } else {
            GlyphRenderer.SetParamsSDF(pvrctx, FontType.GLYPHS_SDF_SIZE, -1f);
        }
#endif

        float draw_x = 0;
        float draw_y = 0 - ascender;
        int line_chars = 0;

        int index = text_index;
        int previous_codepoint = 0x0000;
        int total_glyphs = 0;
        FontCharData fontchardata;

        pvrctx.Save();
        pvrctx.SetVertexAlpha(@params.tint_color[3]);
        pvrctx.SetGlobalOffsetColor(PVRContext.DEFAULT_OFFSET_COLOR);
        pvrctx.SetVertexAntialiasing(PVRFlag.DEFAULT);


        // count the amount of glyph required
        while (index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, ref grapheme)) {
            index += grapheme.size;

            if (grapheme.code == 0xA0) continue;

            if ((fontchardata = InternalGetFontchardata2(this.lookup_table, primary, grapheme.code)) != null) {
                if (fontchardata.has_atlas_entry) total_glyphs++;
                continue;
            }
            if ((fontchardata = InternalGetFontchardata(secondary, grapheme.code)) != null) {
                if (fontchardata.has_atlas_entry) total_glyphs++;
                continue;
            }
        }

        // prepare vertex buffer
        if (has_border) total_glyphs *= 2;
        int added = 0;
        int maximum = GlyphRenderer.Prepare(total_glyphs, has_border);

        // add glyphs to the vertex buffer
        index = text_index;
        while (added < maximum && index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, ref grapheme)) {
            index += grapheme.size;

            //override hard-spaces with white-spaces
            if (grapheme.code == 0xA0) grapheme.code = 0x20;

            // ignore "\r" characters
            if (grapheme.code == FontGlyph.CARRIAGERETURN) {
                previous_codepoint = grapheme.code;
                continue;
            }

            if (grapheme.code == FontGlyph.LINEFEED) {
                draw_x = 0;
                draw_y += @params.height + @params.paragraph_space - ascender;
                previous_codepoint = grapheme.code;
                line_chars = 0;
                continue;
            }

            fontchardata = FontType.InternalGetFontchardata2(this.lookup_table, primary, grapheme.code);
            bool is_secondary = false;
            Texture texture = primary_texture;

            if (fontchardata == null) {
                fontchardata = FontType.InternalGetFontchardata(secondary, grapheme.code);
                is_secondary = true;
                texture = secondary_texture;
            }

            if (fontchardata == null) {
                // codepoint not mapped or FontType.measure() was not called previously to map it
                if (grapheme.code == FontGlyph.TAB) {
                    int filler = FontGlyph.InternalCalcTabstop(line_chars);
                    if (filler > 0) {
                        draw_x += this.space_width * filler * scale;
                        line_chars += filler;
                    }
                } else {
                    // space, hard space or unknown characters
                    draw_x += this.space_width * scale;
                    line_chars++;
                }

                previous_codepoint = grapheme.code;
                continue;
            }

            // apply kerking before continue
            if (previous_codepoint != 0x0000) {
                for (int i = 0 ; i < fontchardata.kernings_size ; i++) {
                    if (fontchardata.kernings[i].codepoint == previous_codepoint) {
                        draw_x += fontchardata.kernings[i].x * scale;
                        break;
                    }
                }
            }

            if (fontchardata.has_atlas_entry) {
                // compute draw location and size
                float dx = x + draw_x + (fontchardata.offset_x * scale);
                float dy = y + draw_y + (fontchardata.offset_y * scale);
                float dw = fontchardata.width * scale;
                float dh = fontchardata.height * scale;

                if (has_border) {
                    float sdx, sdy, sdw, sdh;
#if SDF_FONT
                    sdx = dx;
                    sdy = dy;
                    sdw = dw;
                    sdh = dh;
#else
                    // compute border location and outline size
                    sdx = dx - this.border_size;
                    sdy = dy - this.border_size;
                    sdw = dw + outline_size;
                    sdh = dh + outline_size;
#endif

                    sdx += @params.border_offset_x;
                    sdy += @params.border_offset_y;

                    // queue outlined glyph for batch rendering
                    GlyphRenderer.AppendGlyph(
                        texture, is_secondary, true,
                        fontchardata.atlas_entry.x, fontchardata.atlas_entry.y, fontchardata.width, fontchardata.height,
                        sdx, sdy, sdw, sdh
                    );
                    added++;
                }

                // queue glyph for batch rendering
                GlyphRenderer.AppendGlyph(
                    texture, is_secondary, false,
                    fontchardata.atlas_entry.x, fontchardata.atlas_entry.y, fontchardata.width, fontchardata.height,
                    dx, dy, dw, dh
                );
                added++;
            }

            draw_x += fontchardata.advancex * scale;
            line_chars++;
        }

        // commit draw
        GlyphRenderer.Draw(pvrctx, @params.tint_color, @params.border_color, false, true, primary_texture, secondary_texture);

        pvrctx.Restore();
        return draw_y + @params.height;
    }

    public void MapCodepoints(string text, int text_index, int text_length) {
        int actual = this.fontcharmap_secondary != null ? this.fontcharmap_secondary.char_array_size : 0;
        int new_codepoints = 0;
        Grapheme grapheme = new Grapheme();
        int index = text_index;

        // step 1: count all unmapped codepoints
        while (index < text_length && StringUtils.GetCharacterCodepoint(text, index, ref grapheme)) {
            index += grapheme.size;

            switch (grapheme.code) {
                case FontGlyph.LINEFEED:
                case FontGlyph.CARRIAGERETURN:
                    continue;
            }

            if (FontType.InternalGetFontchardata2(this.lookup_table, this.fontcharmap_primary, grapheme.code) != null)
                continue;
            if (FontType.InternalGetFontchardata(this.fontcharmap_secondary, grapheme.code) != null)
                continue;

            // not present, count it
            new_codepoints++;
        }

        if (new_codepoints < 1) return;// nothing to do

        // step 2: allocate codepoints array
        int codepoints_size = actual + new_codepoints + 1;
        uint[] codepoints = EngineUtils.CreateArray<uint>(codepoints_size);
        codepoints[actual + new_codepoints] = 0x00000000;

        if (this.fontcharmap_secondary != null) {
            // add existing secondary codepoints
            for (int i = 0 ; i < this.fontcharmap_secondary.char_array_size ; i++) {
                codepoints[i] = this.fontcharmap_secondary.char_array[i].codepoint;
            }
        }

        index = text_index;
        new_codepoints = actual;
        while (index < text_length && StringUtils.GetCharacterCodepoint(text, index, ref grapheme)) {
            index += grapheme.size;

            if (FontType.InternalGetFontchardata2(this.lookup_table, this.fontcharmap_primary, grapheme.code) != null)
                continue;
            if (FontType.InternalGetFontchardata(this.fontcharmap_secondary, grapheme.code) != null)
                continue;

            codepoints[new_codepoints++] = (uint)grapheme.code;
        }

        // step 3: rebuild the secondary char map
        if (this.fontcharmap_secondary != null) {
            // dispose previous instance
            this.fontcharmap_secondary.Destroy();
            //free(this.fontcharmap_secondary.char_array);
            //free(this.fontcharmap_secondary);
            if (this.fontcharmap_secondary_texture != null) this.fontcharmap_secondary_texture.Destroy();
        }

        // build map and upload texture
        this.fontcharmap_secondary = InternalRetrieveFontcharmap(codepoints);
        this.fontcharmap_secondary_texture = FontType.InternalUploadTexture(this.fontcharmap_secondary);

        // dispose secondary codepoints array
        //free(codepoints);
    }

    public int Animate(float elapsed) {
        // not used
        return 1;
    }


    private bool InternalInitFreetype(string src) {
        //if (this.font_ptr) throw new Error("The font is already initialized");

        byte[] font = FS.ReadArrayBuffer(src);
        if (font == null) return false;

        mutex.Lock(FontType.fontatlas_mutex);

        // Important: keep the font data allocated, required for FreeType library
        this.font = font;
        this.fontatlas = FontAtlas.Init(font, font.Length);

        mutex.Unlock(FontType.fontatlas_mutex);

        return this.fontatlas == null;
    }

    private FontCharMap InternalRetrieveFontcharmap(uint[] characters_map) {
        FontCharMap fontcharmap = null;

#if SDF_FONT
        FontAtlas.EnableSDF(true);
#endif

        mutex.Lock(FontType.fontatlas_mutex);

        if (characters_map != null) {
            fontcharmap = this.fontatlas.AtlasBuild(
                FontType.GLYPHS_HEIGHT, FontType.GLYPHS_GAPS, characters_map
            );
        } else {
            fontcharmap = this.fontatlas.AtlasBuildComplete(
                FontType.GLYPHS_HEIGHT, FontType.GLYPHS_GAPS
            );
        }

        mutex.Unlock(FontType.fontatlas_mutex);

        return fontcharmap;
    }

    private static Texture InternalUploadTexture(FontCharMap fontcharmap) {
        if (fontcharmap == null) return null;

        WebGL2RenderingContext gl = PVRContext.global_context.webopengl.gl;

        //int unpack_alignment = gl.getParameter(gl.UNPACK_ALIGNMENT);
        //gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);


        WebGLTexture texture = gl.createTexture();
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, texture);

#if SDF_MIPMAPS
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR_MIPMAP_NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
#else
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
#endif

        gl.texImage2D(
            gl.TEXTURE_2D, 0, GLenum.GL_RED,
            fontcharmap.texture_width, fontcharmap.texture_height, 0,
            GLenum.GL_RED, gl.UNSIGNED_BYTE, fontcharmap.texture
        );

#if SDF_MIPMAPS
        gl.generateMipmap(gl.TEXTURE_2D);
#endif

        //gl.pixelStorei(gl.UNPACK_ALIGNMENT, unpack_alignment);

        fontcharmap.texture = 0x00;

        Texture tex = Texture.InitFromRAW(
            texture, (int)fontcharmap.texture_byte_size, true,
            fontcharmap.texture_width, fontcharmap.texture_height,
            fontcharmap.texture_width, fontcharmap.texture_height
        );

#if SDF_MIPMAPS
        tex.has_mipmaps = true;
#endif

        return tex;
    }

    private static FontCharData InternalGetFontchardata(FontCharMap fontcharmap, int codepoint) {
        if (fontcharmap != null) {
            for (int i = 0 ; i < fontcharmap.char_array_size ; i++) {
                if (codepoint == fontcharmap.char_array[i].codepoint) return fontcharmap.char_array[i];
            }
        }
        return null;
    }

    private static FontCharData InternalGetFontchardata2(byte[] lookup_table, FontCharMap fontcharmap, int codepoint) {
        if (codepoint < FontGlyph.LOOKUP_TABLE_LENGTH) {
            byte index = lookup_table[codepoint];
            if (index < FontGlyph.LOOKUP_TABLE_LENGTH) {
                return fontcharmap.char_array[index];
            }
        }
        return InternalGetFontchardata(fontcharmap, codepoint);
    }

}
