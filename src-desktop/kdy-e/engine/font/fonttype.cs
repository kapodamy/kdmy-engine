using System;
using System.Diagnostics;
using Engine.Externals;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Font {

    public class FontType : IFontRender {

        private static volatile Map<FontType> POOL = new Map<FontType>();
        private static volatile int IDS = 0;

        public const byte GLYPHS_HEIGHT = 72;// in the dreamcast use 64px, 64px is enough for SDF
        public const sbyte GLYPHS_GAPS = 16;// space between glyph in pixels (must be high for SDF)
        public const float FAKE_SPACE = 0.75f;// 75% of the height


        private FontType() { }

        private float lines_separation;
        private float[] color;
        private float alpha;
        private float[] border_color;
        private float border_size;
        private bool border_enable;
        private int instance_id;
        private int instance_references;
        private string instance_path;
        private IntPtr fontatlas;
        private IntPtr font_ptr;
        private float space_width;
        private FontCharMap fontcharmap_primary;
        private IntPtr fontcharmap_primary_ptr;
        private Texture fontcharmap_primary_texture;
        private FontCharMap fontcharmap_secondary;
        private IntPtr fontcharmap_secondary_ptr;
        private Texture fontcharmap_secondary_texture;


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
                border_size = 0,
                border_enable = false,

                instance_id = FontType.IDS++,
                instance_references = 1,
                instance_path = full_path,

                fontatlas = IntPtr.Zero,
                font_ptr = IntPtr.Zero,
                space_width = FontType.GLYPHS_HEIGHT * FontType.FAKE_SPACE,

                fontcharmap_primary = null,
                fontcharmap_primary_ptr = IntPtr.Zero,
                fontcharmap_primary_texture = null,

                fontcharmap_secondary = null,
                fontcharmap_secondary_ptr = IntPtr.Zero,
                fontcharmap_secondary_texture = null,
            };

            // initialize FreeType library
            if (fonttype.InternalInitFreetype(src)) {
                //free(full_path);
                //free(fonttype);
                return null;
            }

            // create a texture atlas and glyphs map with all common letters, numbers and symbols
            fonttype.fontcharmap_primary_ptr = fonttype.InternalRetrieveFontcharmap(null);
            fonttype.fontcharmap_primary = ModuleFontAtlas.kdmyEngine_parseFontCharMap(fonttype.fontcharmap_primary_ptr);
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
                ModuleFontAtlas._fontatlas_atlas_destroy(this.fontcharmap_primary_ptr);
                if (this.fontcharmap_primary_texture != null) this.fontcharmap_primary_texture.Destroy();
            }

            if (this.fontcharmap_secondary != null) {
                ModuleFontAtlas._fontatlas_atlas_destroy(this.fontcharmap_secondary_ptr);
                if (this.fontcharmap_secondary_texture != null) this.fontcharmap_secondary_texture.Destroy();
            }

            if (this.fontatlas != null) ModuleFontAtlas._fontatlas_destroy(this.fontatlas);
            if (this.font_ptr != IntPtr.Zero) ModuleFontAtlas.kdmyEngine_deallocate(this.font_ptr);

            //free(this.instance_path);
            //free(fonttype);
        }


        public float Measure(float height, string text, int text_index, int text_size) {
            float scale = height / FontType.GLYPHS_HEIGHT;
            int text_end_index = text_index + text_size;
            int text_length = text.Length;

            Debug.Assert(text_end_index <= text_length, "invalid text_index/text_size (overflow)");

            // check for unmapped characters and them to the secondary map
            InternalFindUnmapedCodepointsToSecondary(text_index, text_end_index, text);

            float max_width = 0;
            float width = 0;
            int index = text_index;
            int previous_codepoint = 0;
            int line_chars = 0;
            Grapheme grapheme = new Grapheme();

            while (index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, grapheme)) {
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

        public void MeansureChar(int codepoint, float height, FontLineInfo lineinfo) {
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

        public float DrawText(PVRContext pvrctx, float height, float x, float y, int text_index, int text_size, string text) {
            if (text == null || text.Length < 1) return 0;

            Grapheme grapheme = new Grapheme();
            FontCharMap primary = this.fontcharmap_primary;
            Texture primary_texture = this.fontcharmap_primary_texture;
            FontCharMap secondary = this.fontcharmap_secondary;
            Texture secondary_texture = this.fontcharmap_secondary_texture;
            bool has_border = this.border_enable && this.border_color[3] > 0 && this.border_size > 0;
            float outline_size = this.border_size * 2;
            float scale = height / FontType.GLYPHS_HEIGHT;
            float ascender = (primary ?? secondary).ascender * scale;
            int text_end_index = text_index + text_size;
            int text_length = text.Length;

            Debug.Assert(text_end_index <= text_length, "invalid text_index/text_size (overflow)");

            float draw_x = 0;
            float draw_y = 0;//-ascender;
            int line_chars = 0;

            int index = text_index;
            int previous_codepoint = 0x0000;
            int total_glyphs = 0;
            FontCharData fontchardata;

            pvrctx.Save();
            pvrctx.SetVertexAlpha(this.alpha);
            pvrctx.SetGlobalOffsetColor(PVRContext.DEFAULT_OFFSET_COLOR);
            pvrctx.SetVertexAntialiasing(PVRContextFlag.DEFAULT);


            // count the amount of glyph required
            while (index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, grapheme)) {
                index += grapheme.size;

                if (grapheme.code == 0xA0) continue;

                if ((fontchardata = InternalGetFontchardata(primary, grapheme.code)) != null) {
                    if (primary_texture != null && fontchardata.has_entry) total_glyphs++;
                    continue;
                }
                if ((fontchardata = InternalGetFontchardata(secondary, grapheme.code)) != null) {
                    if (secondary_texture != null && fontchardata.has_entry) total_glyphs++;
                    continue;
                }
            }

            // prepare vertex buffer
            if (has_border) total_glyphs *= 2;
            int added = 0;
            int maximum = GlyphRenderer.Prepare(total_glyphs, has_border);

            // add glyphs to the vertex buffer
            index = text_index;
            while (added < maximum && index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, grapheme)) {
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
                    draw_y += height + this.lines_separation/* - ascender*/;
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

                if (texture != null && fontchardata.has_entry) {
                    // compute draw location and size
                    float dx = x + draw_x + (fontchardata.offset_x * scale);
                    float dy = y + draw_y + (fontchardata.offset_y * scale);
                    float dw = fontchardata.width * scale;
                    float dh = fontchardata.height * scale;

                    if (has_border) {
                        // compute border location and outline size
                        float sdx = dx - this.border_size;
                        float sdy = dy - this.border_size;
                        float sdw = dw + outline_size;
                        float sdh = dh + outline_size;

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

#if SDF_FONT
            float width, edge;
            FontType.InternalCalcSDF(height, out width, out edge);
            GlyphRenderer.SetSDFParams(pvrctx, width, edge);
#endif

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
            this.font_ptr = ModuleFontAtlas.kdmyEngine_allocate(font);
            this.fontatlas = ModuleFontAtlas._fontatlas_init(this.font_ptr, font.Length);

            return this.fontatlas == null;
        }

        private IntPtr InternalRetrieveFontcharmap(int[] characters_map) {
            IntPtr fontcharmap_ptr = IntPtr.Zero;

#if SDF_FONT
            ModuleFontAtlas.fontatlas_enable_sdf(true);
#endif

            if (characters_map != null) {
                IntPtr characters_map_ptr = ModuleFontAtlas.kdmyEngine_allocate(characters_map);
                fontcharmap_ptr = ModuleFontAtlas._fontatlas_atlas_build(
                    this.fontatlas, FontType.GLYPHS_HEIGHT, FontType.GLYPHS_GAPS, characters_map_ptr
                );
                ModuleFontAtlas.kdmyEngine_deallocate(characters_map_ptr);
            } else {
                fontcharmap_ptr = ModuleFontAtlas._fontatlas_atlas_build_complete(
                    this.fontatlas, FontType.GLYPHS_HEIGHT, FontType.GLYPHS_GAPS
                );
            }

            return fontcharmap_ptr;
        }

        private static Texture InternalUploadTexture(FontCharMap fontcharmap) {
            if (fontcharmap == null) return null;

            WebGL2RenderingContext gl = PVRContext.global_context.webopengl.gl;

            int unpack_alignment = gl.getParameter(gl.UNPACK_ALIGNMENT);
            gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);


            WebGLTexture texture = gl.createTexture();
            gl.bindTexture(gl.TEXTURE_2D, texture);


            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

            // gl.LUMINANCE in WebGL and GL_RED in OpenGL
            gl.texImage2D(
                gl.TEXTURE_2D, 0, GLenum.GL_RED,
                fontcharmap.texture_width, fontcharmap.texture_height, 0,
                GLenum.GL_RED, gl.UNSIGNED_BYTE, fontcharmap.texture
            );

#if !SDF_FONT
            gl.generateMipmap(gl.TEXTURE_2D);
#endif

            gl.pixelStorei(gl.UNPACK_ALIGNMENT, unpack_alignment);

            fontcharmap.texture = IntPtr.Zero;
            return Texture.InitFromRAW(
                texture, (int)fontcharmap.texture_byte_size, true,
                fontcharmap.texture_width, fontcharmap.texture_height,
                fontcharmap.texture_width, fontcharmap.texture_height
            );
        }

        private void InternalFindUnmapedCodepointsToSecondary(int text_index, int text_end_index, string text) {
            int actual = this.fontcharmap_secondary != null ? this.fontcharmap_secondary.char_array_size : 0;
            int new_codepoints = 0;
            Grapheme grapheme = new Grapheme();
            int index = text_index;

            // step 1: count all unmapped codepoints
            while (index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, grapheme)) {
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
            int[] codepoints = new int[actual + new_codepoints + 1];
            codepoints[actual + new_codepoints] = 0x00000000;

            if (this.fontcharmap_secondary != null) {
                // add existing secondary codepoints
                for (int i = 0 ; i < this.fontcharmap_secondary.char_array_size ; i++) {
                    codepoints[i] = this.fontcharmap_secondary.char_array[i].codepoint;
                }
            }

            index = text_index;
            new_codepoints = actual;
            while (index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, grapheme)) {
                index += grapheme.size;

                if (FontType.InternalGetFontchardata(this.fontcharmap_primary, grapheme.code) != null) continue;
                if (FontType.InternalGetFontchardata(this.fontcharmap_secondary, grapheme.code) != null) continue;

                codepoints[new_codepoints++] = grapheme.code;
            }

            // step 3: rebuild the secondary char map
            if (this.fontcharmap_secondary != null) {
                // dispose previous instance
                ModuleFontAtlas._fontatlas_atlas_destroy(this.fontcharmap_secondary_ptr);
                //free(this.fontcharmap_secondary.char_array);
                //free(this.fontcharmap_secondary);
                if (this.fontcharmap_secondary_texture != null) this.fontcharmap_secondary_texture.Destroy();
            }

            // build map and upload texture
            this.fontcharmap_secondary_ptr = InternalRetrieveFontcharmap(codepoints);
            this.fontcharmap_secondary = ModuleFontAtlas.kdmyEngine_parseFontCharMap(this.fontcharmap_secondary_ptr);
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

#if SDF_FONT
        private static void InternalCalcSDF(float font_height, out float width, out float edge) {
            const float SMALL_PX = 8.0f, SMALL_WIDTH = 0.474f, SMALL_EDGE = 0.12f;
            const float LARGE_PX = 256f, LARGE_WIDTH = 0.510f, LARGE_EDGE = 0.06f;

            float target = Math2D.InverseLerp(SMALL_PX, LARGE_PX, font_height);

            width = Math2D.Lerp(SMALL_WIDTH, LARGE_WIDTH, target);
            edge = Math2D.Lerp(SMALL_EDGE, LARGE_EDGE, target);
        }
#endif


        public int Animate(float elapsed) { return 0; }

        public void EnableColorByDifference(bool enable) { }


    }

}
