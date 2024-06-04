using System;
using System.Runtime.CompilerServices;
using Engine.Externals;
using Engine.Externals.FontAtlasInterop;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Font;

public class FontType : IFontRender {

    private static Map<FontType> POOL = new Map<FontType>();
    private static int IDS = 0;

    public const byte GLYPHS_HEIGHT = 72;// in the dreamcast use 32px, 64px is enough for SDF
    public const float GLYPHS_OUTLINE_RATIO = 0.086f;// ~6px of outline @ 72px (used in SDF)
    public const float GLYPHS_SMOOTHING_COEFF = 0.245f;// used in SDF, idk how its works
    public const byte GLYPHS_GAPS = 16;// space between glyph in pixels (must be high for SDF)
    public const float FAKE_SPACE = 0.75f;// 75% of the height


    private FontType() { }

    private float lines_separation;
    private float[] color;
    private float alpha;
    private float[] border_color;
    private float border_size;
    private bool border_enable;
    private float border_offset_x;
    private float border_offset_y;
    private int instance_id;
    private int instance_references;
    private string instance_path;
    private FontAtlas fontatlas;
    private float space_width;
    private FontCharMap fontcharmap_primary;
    private Texture fontcharmap_primary_texture;
    private FontCharMap fontcharmap_secondary;
    private Texture fontcharmap_secondary_texture;

    [MethodImpl(MethodImplOptions.Synchronized)]
    public static FontType Init(string src) {
        string full_path = FS.GetFullPathAndOverride(src);

        // C# only, lock the pool
        lock (FontType.POOL) {
            foreach (FontType instance in FontType.POOL) {
                if (instance.instance_path == full_path) {
                    //free(full_path);
                    instance.instance_references++;
                    return instance;
                }
            }
        }

        FontType fonttype = new FontType() {
            lines_separation = 0,

            color = new float[] { 0.0f, 0.0f, 0.0f },
            alpha = 1.0f,

            border_color = new float[] { 1.0f, 1.0f, 1.0f, 1.0f },
            border_size = 0f,
            border_enable = false,
            border_offset_x = 0f,
            border_offset_y = 0f,

            instance_id = FontType.IDS++,
            instance_references = 1,
            instance_path = full_path,

            fontatlas = null,
            space_width = FontType.GLYPHS_HEIGHT * FontType.FAKE_SPACE,

            fontcharmap_primary = null,
            fontcharmap_primary_texture = null,

            fontcharmap_secondary = null,
            fontcharmap_secondary_texture = null,
        };

        // initialize FreeType library
        if (fonttype.InternalInitFreetype(src)) {
            //free(full_path);
            //free(fonttype);
            return null;
        }

        // create a texture atlas and glyphs map with all common letters, numbers and symbols
        fonttype.fontcharmap_primary = fonttype.InternalRetrieveFontcharmap(null);
        fonttype.fontcharmap_primary_texture = FontType.InternalUploadTexture(fonttype.fontcharmap_primary);

        if (fonttype.fontcharmap_primary != null) {
            for (int i = 0 ; i < fonttype.fontcharmap_primary.char_array_size ; i++) {
                if (fonttype.fontcharmap_primary.char_array[i].codepoint == FontGlyph.SPACE) {
                    fonttype.space_width = fonttype.fontcharmap_primary.char_array[i].advancex;
                    break;
                }
            }
        }

        //C# only, lock the pool
        lock (FontType.POOL) {
            FontType.POOL.Set(fonttype.instance_id, fonttype);
        }
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

        //free(this.instance_path);
        //free(fonttype);
    }


    public float Measure(float height, string text, int text_index, int text_length) {
        float scale = height / FontType.GLYPHS_HEIGHT;
        int text_end_index = text_index + text_length;

        //Debug.Assert(text_end_index <= text_length, "invalid text_index/text_length (overflow)");

        // check for unmapped characters and them to the secondary map
        InternalFindUnmapedCodepointsToSecondary(text_index, text_end_index, text);

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

            FontCharData fontchardata = InternalGetFontchardata(this.fontcharmap_primary, grapheme.code);
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

    public void MeasureChar(int codepoint, float height, FontLineInfo lineinfo) {
        float scale = height / FontType.GLYPHS_HEIGHT;

        //override hard-spaces with white-spaces
        if (codepoint == 0xA0) codepoint = 0x20;

        FontCharData fontchardata = FontType.InternalGetFontchardata(this.fontcharmap_primary, codepoint);
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

    public void SetLinesSeparation(float height) {
        this.lines_separation = height;
    }

    public void SetColor(float r, float g, float b) {
        this.color[0] = r;
        this.color[1] = g;
        this.color[2] = b;
    }

    public void SetRGB8Color(uint rbg8_color) {
        Math2D.ColorBytesToFloats(rbg8_color, false, this.color);
    }

    public void SetAlpha(float alpha) {
        this.alpha = alpha;
    }

    public void SetBorderSize(float border_size) {
        /*#if SDF_FONT
                    // for SDF rendering use half of the border
                    border_size /= 2f;
        #endif*/

        this.border_size = border_size;
    }

    public void SetBorderColor(float r, float g, float b, float a) {
        if (r >= 0) this.border_color[0] = r;
        if (g >= 0) this.border_color[1] = g;
        if (b >= 0) this.border_color[2] = b;
        if (a >= 0) this.border_color[3] = a;
    }

    public void SetBorderColorRGBA8(uint rbga8_color) {
        Math2D.ColorBytesToFloats(rbga8_color, true, this.border_color);
    }

    public void EnableBorder(bool enable) {
        this.border_enable = enable;
    }

    public void SetBorder(bool enable, float size, float[] rgba) {
        this.border_enable = enable;

        /*#if SDF_FONT
                    // for SDF rendering use half of the border
                    size /= 2f;
        #endif*/

        this.border_size = size;
        SetBorderColor(rgba[0], rgba[1], rgba[2], rgba[3]);
    }

    public void SetBorderOffset(float x, float y) {
        this.border_offset_x = x;
        this.border_offset_y = y;
    }

    public float DrawText(PVRContext pvrctx, float height, float x, float y, int text_index, int text_length, string text) {
        if (text == null || text_length < 1) return 0;

        Grapheme grapheme = new Grapheme();
        FontCharMap primary = this.fontcharmap_primary;
        Texture primary_texture = this.fontcharmap_primary_texture;
        FontCharMap secondary = this.fontcharmap_secondary;
        Texture secondary_texture = this.fontcharmap_secondary_texture;
        bool has_border = this.border_enable && this.border_color[3] > 0 && this.border_size >= 0f;
        float outline_size = this.border_size * 2;
        float scale = height / FontType.GLYPHS_HEIGHT;
        float ascender = ((primary ?? secondary).ascender / 2f) * scale;// FIXME: ¿why does dividing by 2 works?
        int text_end_index = text_index + text_length;
        float border_padding1 = 0f, border_padding2 = 0f;

        //Debug.Assert(text_end_index <= text_length, "invalid text_index/text_length (overflow)");

#if SDF_FONT
        // calculate sdf thickness
        if (has_border) {
            if (this.border_size > 0f) {
                float max_border_size = height * FontType.GLYPHS_OUTLINE_RATIO;
                float border_size = Math.Min(this.border_size, max_border_size);
                float thickness = (1f - (border_size / max_border_size)) / 2f;
                GlyphRenderer.SetSDFThickness(pvrctx, thickness);

                /*if (border_size < this.border_size && height > 8f) {
                    // add some padding
                    border_padding1 = this.border_size - border_size;
                    border_padding2 = border_padding1 * 2f;
                }*/
            } else {
                GlyphRenderer.SetSDFThickness(pvrctx, -1f);
            }
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
        pvrctx.SetVertexAlpha(this.alpha);
        pvrctx.SetGlobalOffsetColor(PVRContext.DEFAULT_OFFSET_COLOR);
        pvrctx.SetVertexAntialiasing(PVRFlag.DEFAULT);


        // count the amount of glyph required
        while (index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, ref grapheme)) {
            index += grapheme.size;

            if (grapheme.code == 0xA0) continue;

            if ((fontchardata = InternalGetFontchardata(primary, grapheme.code)) != null) {
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

#if SDF_FONT
        float smoothing = FontType.InternalCalcSmoothing(pvrctx, height);
        GlyphRenderer.SetSDFSmoothing(pvrctx, smoothing);
#endif

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
                draw_y += height + this.lines_separation - ascender;
                previous_codepoint = grapheme.code;
                line_chars = 0;
                continue;
            }

            fontchardata = FontType.InternalGetFontchardata(primary, grapheme.code);
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
                    sdx = dx - border_padding1;
                    sdy = dy - border_padding1;
                    sdw = dw + border_padding2;
                    sdh = dh + border_padding2;
#else
                    // compute border location and outline size
                    sdx = dx - this.border_size;
                    sdy = dy - this.border_size;
                    sdw = dw + outline_size;
                    sdh = dh + outline_size;
#endif

                    sdx += this.border_offset_x;
                    sdy += this.border_offset_y;

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
        GlyphRenderer.Draw(pvrctx, this.color, this.border_color, false, true, primary_texture, secondary_texture);

        pvrctx.Restore();
        return draw_y + height;
    }


    private bool InternalInitFreetype(string src) {
        //if (this.font_ptr) throw new Error("The font is already initialized");

        byte[] font = FS.ReadArrayBuffer(src);
        if (font == null) return false;

        // Important: keep the font data allocated, required for FreeType library
        this.fontatlas = FontAtlas.Init(font, font.Length);

        return this.fontatlas == null;
    }

    [MethodImpl(MethodImplOptions.Synchronized)]
    private FontCharMap InternalRetrieveFontcharmap(uint[] characters_map) {
        FontCharMap fontcharmap = null;

#if SDF_FONT
        FontAtlas.EnableSDF(true);
#endif

        if (characters_map != null) {
            fontcharmap = this.fontatlas.AtlasBuild(
                FontType.GLYPHS_HEIGHT, FontType.GLYPHS_GAPS, characters_map
            );
        } else {
            fontcharmap = this.fontatlas.AtlasBuildComplete(
                FontType.GLYPHS_HEIGHT, FontType.GLYPHS_GAPS
            );
        }

        return fontcharmap;
    }

    private static Texture InternalUploadTexture(FontCharMap fontcharmap) {
        if (fontcharmap == null) return null;

        WebGL2RenderingContext gl = PVRContext.global_context.webopengl.gl;

        //int unpack_alignment = gl.getParameter(gl.UNPACK_ALIGNMENT);
        //gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);


        WebGLTexture texture = gl.createTexture();
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

    private void InternalFindUnmapedCodepointsToSecondary(int text_index, int text_end_index, string text) {
        int actual = this.fontcharmap_secondary != null ? this.fontcharmap_secondary.char_array_size : 0;
        int new_codepoints = 0;
        Grapheme grapheme = new Grapheme();
        int index = text_index;

        // step 1: count all unmapped codepoints
        while (index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, ref grapheme)) {
            index += grapheme.size;

            switch (grapheme.code) {
                case FontGlyph.LINEFEED:
                case FontGlyph.CARRIAGERETURN:
                    continue;
            }

            if (FontType.InternalGetFontchardata(this.fontcharmap_primary, grapheme.code) != null) continue;
            if (FontType.InternalGetFontchardata(this.fontcharmap_secondary, grapheme.code) != null) continue;

            // not present, count it
            new_codepoints++;
        }

        if (new_codepoints < 1) return;// nothing to do

        // step 2: allocate codepoints array
        uint[] codepoints = new uint[actual + new_codepoints + 1];
        codepoints[actual + new_codepoints] = 0x00000000;

        if (this.fontcharmap_secondary != null) {
            // add existing secondary codepoints
            for (int i = 0 ; i < this.fontcharmap_secondary.char_array_size ; i++) {
                codepoints[i] = this.fontcharmap_secondary.char_array[i].codepoint;
            }
        }

        index = text_index;
        new_codepoints = actual;
        while (index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, ref grapheme)) {
            index += grapheme.size;

            if (FontType.InternalGetFontchardata(this.fontcharmap_primary, grapheme.code) != null) continue;
            if (FontType.InternalGetFontchardata(this.fontcharmap_secondary, grapheme.code) != null) continue;

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

    private static FontCharData InternalGetFontchardata(FontCharMap fontcharmap, int codepoint) {
        if (fontcharmap != null) {
            for (int i = 0 ; i < fontcharmap.char_array_size ; i++) {
                if (codepoint == fontcharmap.char_array[i].codepoint) return fontcharmap.char_array[i];
            }
        }
        return null;
    }

    private static float InternalCalcSmoothing(PVRContext pvrctx, float height) {
        SIMDMatrix matrix = pvrctx.CurrentMatrix;

#if SDF_SMOOTHNESS_BY_MATRIX_SCALE_DECOMPOSITION
        double x = matrix[15] * Math.Sqrt(
            (matrix[0] * matrix[0]) +
            (matrix[1] * matrix[1]) +
            (matrix[2] * matrix[2])
        );
        double y = matrix[15] * Math.Sqrt(
            (matrix[4] * matrix[4]) +
            (matrix[5] * matrix[5]) +
            (matrix[6] * matrix[6])
        );

        double scale = (Math.Abs((x + y) / 2.0) * height) / FontType.GLYPHS_HEIGHT;
        double smoothness = FontType.GLYPHS_SMOOTHING_COEFF / (FontType.GLYPHS_HEIGHT * scale);

        return (float)smoothness;

#else
        float x = 1f, y = 1f;
        matrix.MultiplyPoint(ref x, ref y);

        float scale = (MathF.Abs((x + y) / 2f) * height) / FontType.GLYPHS_HEIGHT;
        float smoothness = FontType.GLYPHS_SMOOTHING_COEFF / (FontType.GLYPHS_HEIGHT * scale);

        return smoothness;
#endif
    }


    public int Animate(float elapsed) { return 0; }

    public void EnableColorByAddition(bool enable) { }


}
