using System;
using Engine.Externals;
using Engine.Externals.FontAtlasInterop;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;
using KallistiOS.MUTEX;

namespace Engine.Font;

public class FontType : IFont {

#if SDF_FONT
    private const byte GLYPHS_HEIGHT = 72;// 72px is enough for SDF
    private const byte GLYPHS_GAP = 16;// space between glyphs in pixels (must be high)
    private const byte GLYPHS_OUTLINE_HEIGHT = GLYPHS_HEIGHT >>> 1;
    private const byte GLYPHS_OUTLINE_GAP = GLYPHS_GAP >>> 1;
    private const byte GLYPHS_OUTLINE_RATIO = GLYPHS_OUTLINE_HEIGHT >>> 2;// 25% of FONTTYPE_GLYPHS_OUTLINE_HEIGHT
    private const float GLYPHS_OUTLINE_THICKNESS = 1.25f;// 125%
#else
    private const byte GLYPHS_HEIGHT = 42;
    private const byte GLYPHS_GAP = 4;// space between glyphs in pixels
#endif
    private const float FAKE_SPACE = 0.9f;// 90% of the height


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
    private FCAtlas atlas_primary;
    private FCAtlas atlas_secondary;
    private byte[] lookup_table;


    public static FontType Init(string src) {
        string full_path = FS.GetFullPathAndOverride(src);

        foreach (FontType instance in FontType.POOL) {
            if (StringUtils.EqualsIgnoreCase(instance.instance_path, full_path)) {
                //free(full_path);
                instance.instance_references++;
                return instance;
            }
        }

        FontType fonttype = new FontType() {
            instance_id = FontType.IDS++,
            instance_references = 1,
            instance_path = full_path,

            font = null,
            fontatlas = null,
            space_width = FontType.GLYPHS_HEIGHT * FontType.FAKE_SPACE,

#if SDF_FONT
            atlas_primary = { map_outline = null, texture_outline = null, map = null, texture = null },
            atlas_secondary = { map_outline = null, texture_outline = null, map = null, texture = null },
#else
            atlas_primary = { map = null, texture = null },
            atlas_secondary = { map = null, texture = null },
#endif

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
        fonttype.InternalCreateAtlas(ref fonttype.atlas_primary, null);

        if (fonttype.atlas_primary.map != null) {
            FontCharData[] char_array = fonttype.atlas_primary.map.char_array;
            int char_array_size = fonttype.atlas_primary.map.char_array_size;

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

        FontType.InternalDestroyAtlas(ref this.atlas_primary);
        FontType.InternalDestroyAtlas(ref this.atlas_secondary);

        if (this.fontatlas != null) this.fontatlas.Destroy();

        //free(this.font);
        //free(this.instance_path);
        //free(this);
    }


    public void Measure(ref FontParams @params, string text, int text_index, int text_length, ref FontLinesInfo info) {
        float scale = @params.height / FontType.GLYPHS_HEIGHT;
        int text_end_index = text_index + text_length;
        float max_width = 0f;
        float width = 0f;
        int index = text_index;
        uint previous_codepoint = 0;
        int line_chars = 0;
        int lines = 1;
        int last_glyph_width_correction = 0;
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
                width = 0f;
                previous_codepoint = grapheme.code;
                line_chars = 0;
                lines++;
                last_glyph_width_correction = 0;
                continue;
            }

            FontCharData fontchardata;
            int map_index = FontType.InternalGetFontchardata2(this.lookup_table, this.atlas_primary, grapheme.code);

            if (map_index < 0) {
                map_index = FontType.InternalGetFontchardata(this.atlas_secondary, grapheme.code);
                if (map_index < 0) {
                    if (grapheme.code == FontGlyph.TAB) {
                        int filler = FontGlyph.InternalCalcTabstop(line_chars);
                        if (filler > 0) {
                            width += this.space_width * filler;
                            line_chars += filler;
                        }
                    } else {
                        // space, hard space or unknown characters
                        width += this.space_width;
                        line_chars++;
                    }
                    last_glyph_width_correction = 0;
                    continue;
                } else {
                    fontchardata = this.atlas_secondary.map.char_array[map_index];
                }
            } else {
                fontchardata = this.atlas_primary.map.char_array[map_index];
            }

            int kerning_x = 0;
            if (previous_codepoint != 0x0000) {
                // compute kerning
                for (int i = 0 ; i < fontchardata.kernings_size ; i++) {
                    if (fontchardata.kernings[i].codepoint == previous_codepoint) {
                        kerning_x = fontchardata.kernings[i].x;
                        break;
                    }
                }
            }

            last_glyph_width_correction = fontchardata.width + fontchardata.offset_x - fontchardata.advancex + kerning_x;
            width += fontchardata.advancex + kerning_x;
            previous_codepoint = grapheme.code;
            line_chars++;
        }

        float line_height = MeasureLineHeight(@params.height);
        info.max_width = (Math.Max(width, max_width) + last_glyph_width_correction) * scale;
        info.total_height = (line_height * lines) + (@params.paragraph_space * (lines - 1));
    }

    public void MeasureChar(uint codepoint, float height, ref FontCharInfo info) {
        float scale = height / FontType.GLYPHS_HEIGHT;

        //override hard-spaces with white-spaces
        if (codepoint == 0xA0) codepoint = 0x20;

        FontCharData fontchardata;
        int map_index = FontType.InternalGetFontchardata2(this.lookup_table, this.atlas_primary, codepoint);

        if (map_index < 0) {
            map_index = FontType.InternalGetFontchardata(this.atlas_secondary, codepoint);
            if (map_index < 0) {
                if (codepoint == FontGlyph.TAB) {
                    int filler = FontGlyph.InternalCalcTabstop(info.line_char_count);
                    if (filler > 0) {
                        info.last_char_width = this.space_width * filler;
                        info.line_char_count += filler;
                    }
                } else {
                    // space, hard space or unknown characters
                    info.last_char_width = this.space_width;
                    info.line_char_count++;
                }
                info.last_char_width *= scale;
                info.last_char_height = height;
                info.last_char_width_end = 0f;
                return;
            } else {
                fontchardata = this.atlas_secondary.map.char_array[map_index];
            }
        } else {
            fontchardata = this.atlas_primary.map.char_array[map_index];
        }

        int kerning_x = 0;
        if (info.previous_codepoint != 0x0000) {
            // compute kerning
            for (int i = 0 ; i < fontchardata.kernings_size ; i++) {
                if (fontchardata.kernings[i].codepoint == info.previous_codepoint) {
                    kerning_x = fontchardata.kernings[i].x;
                    break;
                }
            }
        }

        info.last_char_width = (fontchardata.advancex + kerning_x) * scale;
        info.last_char_height = fontchardata.advancey * scale;
        info.last_char_width_end = (fontchardata.width + fontchardata.offset_x - fontchardata.advancex + kerning_x) * scale;
        info.previous_codepoint = codepoint;
        info.line_char_count++;
    }

    public float MeasureLineHeight(float height) {
        float line_height = (this.atlas_primary.map ?? this.atlas_secondary.map).line_height;

        float scale = height / FontType.GLYPHS_HEIGHT;
        return line_height * scale;
    }

    public float DrawText(PVRContext pvrctx, ref FontParams @params, float x, float y, int text_index, int text_length, string text) {
        if (StringUtils.IsEmpty(text)) return 0f;

        Grapheme grapheme = new Grapheme();
        bool has_border = @params.border_enable && @params.border_color[3] > 0f && @params.border_size >= 0f;
        float outline_size = @params.border_size * 2f;
        float scale_glyph = @params.height / FontType.GLYPHS_HEIGHT;
        int text_end_index = text_index + text_length;
        float ascender = (this.atlas_primary.map ?? this.atlas_secondary.map).ascender;
        float line_height = (this.atlas_primary.map ?? this.atlas_secondary.map).line_height;

#if SDF_FONT
        float scale_outline = @params.height / FontType.GLYPHS_OUTLINE_HEIGHT;

        if (has_border) {
            // calculate sdf padding
            float padding;
            padding = @params.border_size / FontType.GLYPHS_OUTLINE_RATIO;
            padding /= @params.height / FontType.GLYPHS_OUTLINE_HEIGHT;

            GlyphRenderer.SetParamsSDF(pvrctx, FontType.GLYPHS_OUTLINE_RATIO, padding, FontType.GLYPHS_OUTLINE_THICKNESS);
        } else {
            GlyphRenderer.SetParamsSDF(pvrctx, FontType.GLYPHS_OUTLINE_RATIO, -1f, -1f);
        }
#endif

        float draw_glyph_x = 0f;
        float draw_glyph_y = ascender;
        int line_chars = 0;
        int lines = 1;

        int index = text_index;
        uint previous_codepoint = 0x0000;
        int total_glyphs = 0;
        int map_index;

        pvrctx.Save();
        pvrctx.SetVertexAlpha(@params.tint_color[3]);
        pvrctx.SetGlobalOffsetColor(PVRContext.DEFAULT_OFFSET_COLOR);
        pvrctx.SetVertexAntialiasing(PVRFlag.DEFAULT);

        // count the amount of glyph required
        while (index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, ref grapheme)) {
            index += grapheme.size;

            if (grapheme.code == 0xA0) continue;

            map_index = FontType.InternalGetFontchardata2(this.lookup_table, this.atlas_primary, grapheme.code);
            if (map_index >= 0) {
                if (this.atlas_primary.map.char_array[map_index].has_atlas_entry) total_glyphs++;
                continue;
            }
            map_index = FontType.InternalGetFontchardata(this.atlas_secondary, grapheme.code);
            if (map_index >= 0) {
                if (this.atlas_secondary.map.char_array[map_index].has_atlas_entry) total_glyphs++;
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
                draw_glyph_x = 0f;
                draw_glyph_y += line_height + (@params.paragraph_space / scale_glyph);
                previous_codepoint = grapheme.code;
                line_chars = 0;
                lines++;
                continue;
            }

            ref FCAtlas atlas = ref this.atlas_primary;
            bool is_secondary = false;
            map_index = FontType.InternalGetFontchardata2(this.lookup_table, atlas, grapheme.code);

            if (map_index < 0) {
                atlas = ref this.atlas_secondary;
                is_secondary = true;
                map_index = FontType.InternalGetFontchardata(atlas, grapheme.code);
            }

            if (map_index < 0) {
                // codepoint not mapped or FontType.measure() was not called previously to map it
                if (grapheme.code == FontGlyph.TAB) {
                    int filler = FontGlyph.InternalCalcTabstop(line_chars);
                    if (filler > 0) {
                        draw_glyph_x += this.space_width * filler;
                        line_chars += filler;
                    }
                } else {
                    // space, hard space or unknown characters
                    draw_glyph_x += this.space_width;
                    line_chars++;
                }

                previous_codepoint = grapheme.code;
                continue;
            }

            FontCharData fontchardata_glyph = atlas.map.char_array[map_index];

            // apply kerning before continue
            if (previous_codepoint != 0x0000) {
                for (int i = 0 ; i < fontchardata_glyph.kernings_size ; i++) {
                    if (fontchardata_glyph.kernings[i].codepoint == previous_codepoint) {
                        draw_glyph_x += fontchardata_glyph.kernings[i].x;
                        break;
                    }
                }
            }

            if (fontchardata_glyph.has_atlas_entry) {
                // compute draw location and size
                float dx = x + ((draw_glyph_x + fontchardata_glyph.offset_x) * scale_glyph);
                float dy = y + ((draw_glyph_y + fontchardata_glyph.offset_y) * scale_glyph);
                float dw = fontchardata_glyph.width * scale_glyph;
                float dh = fontchardata_glyph.height * scale_glyph;

                if (has_border) {
                    float sdx, sdy, sdw, sdh;
                    Texture texture_outline;
                    FontCharData fontchardata_outline;

#if SDF_FONT
                    texture_outline = atlas.texture_outline;
                    fontchardata_outline = atlas.map_outline.char_array[map_index];

                    sdw = dw + ((fontchardata_outline.width * scale_outline) - dw);
                    sdh = dh + ((fontchardata_outline.height * scale_outline) - dh);
                    sdx = dx - ((sdw - dw) / 2f);
                    sdy = dy - ((sdh - dh) / 2f);
#else
                    texture_outline = atlas.texture;
                    fontchardata_outline = fontchardata_glyph;

                    // compute border location and outline size
                    sdw = dw + outline_size;
                    sdh = dh + outline_size;
                    sdx = dx - @params.border_size;
                    sdy = dy - @params.border_size;
#endif

                    sdx += @params.border_offset_x;
                    sdy += @params.border_offset_y;

                    // queue outlined glyph for batch rendering
                    GlyphRenderer.AppendGlyph(
                        texture_outline, is_secondary, true,
                        fontchardata_outline.atlas_entry.x, fontchardata_outline.atlas_entry.y,
                        fontchardata_outline.width, fontchardata_outline.height,
                        sdx, sdy, sdw, sdh
                    );
                    added++;
                }

                // queue glyph for batch rendering
                GlyphRenderer.AppendGlyph(
                    atlas.texture, is_secondary, false,
                    fontchardata_glyph.atlas_entry.x, fontchardata_glyph.atlas_entry.y,
                    fontchardata_glyph.width, fontchardata_glyph.height,
                    dx, dy, dw, dh
                );
                added++;
            }

            draw_glyph_x += fontchardata_glyph.advancex;
            line_chars++;
        }

        // commit draw
#if SDF_FONT
        if (has_border) {
            GlyphRenderer.Draw(
                pvrctx,
                @params.tint_color, @params.border_color,
                false, true,
                this.atlas_primary.texture, this.atlas_secondary.texture,
                this.atlas_primary.texture_outline, this.atlas_secondary.texture_outline
            );
        } else {
#else
        {
#endif
            GlyphRenderer.Draw(
                pvrctx,
                @params.tint_color, @params.border_color,
                false, true,
                this.atlas_primary.texture, this.atlas_secondary.texture,
                null, null
            );
        }

        pvrctx.Restore();
        return (line_height * lines) + (@params.paragraph_space * (lines - 1));
    }

    public void MapCodepoints(string text, int text_index, int text_length) {
        Grapheme grapheme = new Grapheme();

        int codepoints_length;
        int codepoints_used;
        uint[] codepoints;
        int existing_count;

        if (this.atlas_secondary.map != null) {
            codepoints_used = this.atlas_secondary.map.char_array_size;
            codepoints_length = codepoints_used + 32;
            codepoints = EngineUtils.CreateArray<uint>(codepoints_length);
            existing_count = codepoints_used;

            // add existing secondary codepoints
            for (int i = 0 ; i < this.atlas_secondary.map.char_array_size ; i++) {
                codepoints[i] = this.atlas_secondary.map.char_array[i].codepoint;
            }
        } else {
            codepoints_used = 0;
            codepoints_length = 64;
            codepoints = EngineUtils.CreateArray<uint>(codepoints_length);
            existing_count = 0;
        }

// step 1: add all unmapped codepoints
L_find_unmaped_codepoints:
        while (text_index < text_length && StringUtils.GetCharacterCodepoint(text, text_index, ref grapheme)) {
            text_index += grapheme.size;

            switch (grapheme.code) {
                case FontGlyph.LINEFEED:
                case FontGlyph.CARRIAGERETURN:
                    continue;
            }

            if (FontType.InternalGetFontchardata2(this.lookup_table, this.atlas_primary, grapheme.code) >= 0)
                continue;
            if (FontType.InternalGetFontchardata(this.atlas_secondary, grapheme.code) >= 0)
                continue;

            // check if the codepoint is already added
            for (int i = 0 ; i < codepoints_used ; i++) {
                if (codepoints[i] == grapheme.code) goto L_find_unmaped_codepoints;
            }

            codepoints[codepoints_used++] = grapheme.code;

            if ((codepoints_used + 1) >= codepoints_length) {
                codepoints_length += 16;
                EngineUtils.ResizeArray(ref codepoints, codepoints_length);
            }
        }

        if (codepoints_used <= existing_count) {
            // nothing to do
            //free(codepoints);
            return;
        }

        codepoints[codepoints_used++] = 0x00000000;

        // step 3: rebuild the secondary char map
        FontType.InternalDestroyAtlas(ref this.atlas_secondary);
        InternalCreateAtlas(ref this.atlas_secondary, codepoints);
        //free(codepoints);
    }

    public int Animate(float elapsed) {
        // not used
        return 1;
    }


    private bool InternalInitFreetype(string src) {
        //if (this.font_ptr) throw new Error("The font is already initialized");

        byte[] font = FS.ReadArrayBuffer(src);
        if (font == null) return true;

        mutex.Lock(FontType.fontatlas_mutex);

        // Important: keep the font data allocated, required for FreeType library
        this.font = font;
        this.fontatlas = FontAtlas.Init(font, font.Length);

        mutex.Unlock(FontType.fontatlas_mutex);

        return this.fontatlas == null;
    }

    private FontCharMap InternalCreateFontcharmap(uint[] characters_map, byte glyphs_height, byte glyphs_gap) {
        FontCharMap fontcharmap = null;

#if SDF_FONT
        FontAtlas.EnableSDF(true);
#endif

        mutex.Lock(FontType.fontatlas_mutex);

        if (characters_map != null) {
            fontcharmap = this.fontatlas.AtlasBuild(
                glyphs_height, glyphs_gap, characters_map
            );
        } else {
            fontcharmap = this.fontatlas.AtlasBuildComplete(
                glyphs_height, glyphs_gap
            );
        }

        mutex.Unlock(FontType.fontatlas_mutex);

        return fontcharmap;
    }

    private static Texture InternalUploadTexture(FontCharMap fontcharmap) {
        if (fontcharmap == null) return null;

        WebGL2RenderingContext gl = PVRContext.global_context.webopengl.gl;

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

        gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);

        // FIXME: calling this method also deallocates the vram texture, �is this a gpu driver bug?
        // now deallocate texture data
        //fontcharmap.DestroyTextureOnly();

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

    private static int InternalGetFontchardata(FCAtlas atlas, uint codepoint) {
        if (atlas.map != null) {
            for (int i = 0 ; i < atlas.map.char_array_size ; i++) {
                if (codepoint == atlas.map.char_array[i].codepoint) return i;
            }
        }
        return -1;
    }

    private static int InternalGetFontchardata2(byte[] lookup_table, FCAtlas atlas, uint codepoint) {
        if (codepoint < FontGlyph.LOOKUP_TABLE_LENGTH) {
            byte index = lookup_table[codepoint];
            if (index < FontGlyph.LOOKUP_TABLE_LENGTH) {
                return index;
            }
        }
        return InternalGetFontchardata(atlas, codepoint);
    }

    private void InternalCreateAtlas(ref FCAtlas atlas, uint[] codepoints) {
        FontCharMap fontcharmap = InternalCreateFontcharmap(codepoints, FontType.GLYPHS_HEIGHT, FontType.GLYPHS_GAP);
        Texture texture = FontType.InternalUploadTexture(fontcharmap);

#if SDF_FONT
        if (fontcharmap != null) {
            FontCharMap fontcharmap_outline = InternalCreateFontcharmap(codepoints, FontType.GLYPHS_OUTLINE_HEIGHT, FontType.GLYPHS_OUTLINE_GAP);
            Texture texture_outline = FontType.InternalUploadTexture(fontcharmap_outline);

            if (fontcharmap_outline == null) {
                throw new NullReferenceException("fontcharmap_outline was null");
            }
            if (fontcharmap.char_array_size != fontcharmap_outline.char_array_size) {
                throw new Exception("char_array_size fontcharmap != fontcharmap_outline");
            }

            for (int i = 0 ; i < fontcharmap.char_array_size ; i++) {
                FontCharData entry = fontcharmap.char_array[i];
                FontCharData entry_outline = fontcharmap_outline.char_array[i];

                if (entry.has_atlas_entry != entry_outline.has_atlas_entry) {
                    throw new Exception("entry.has_atlas_entry != entry_outline.has_atlas_entry");
                }
            }

            atlas.map_outline = fontcharmap_outline;
            atlas.texture_outline = texture_outline;
        }
#endif
        atlas.map = fontcharmap;
        atlas.texture = texture;
    }

    private static void InternalDestroyAtlas(ref FCAtlas atlas) {
#if SDF_FONT
        if (atlas.map_outline != null) {
            atlas.map_outline.Destroy();
            atlas.map_outline = null;
        }
        if (atlas.texture_outline != null) {
            atlas.texture_outline.Destroy();
            atlas.texture_outline = null;
        }
#endif
        if (atlas.map != null) {
            atlas.map.Destroy();
            atlas.map = null;
        }
        if (atlas.texture != null) {
            atlas.texture.Destroy();
            atlas.texture = null;
        }
    }


    private struct FCAtlas {
#if SDF_FONT
        public FontCharMap map_outline;
        public Texture texture_outline;
#endif
        public FontCharMap map;
        public Texture texture;
    }

}
