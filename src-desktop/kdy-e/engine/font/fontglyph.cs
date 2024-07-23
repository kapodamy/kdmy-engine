using System;
using Engine.Game.Common;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Font;

public class FontGlyph : IFont {

    public const byte SPACE = 0x20;
    public const byte HARDSPACE = 0xA0;
    public const byte LINEFEED = 0x0A;
    public const byte TAB = 0x09;
    public const byte CARRIAGERETURN = 0x0D;
    public const float SPACE_WIDTH_RATIO = 0.8f;
    public const byte TABSTOP = 8;
    public const byte LOOKUP_TABLE_LENGTH = 128;// this covers all ascii characters

    private FontGlyph() { }

    private Texture texture;
    private GlyphInfo[] table;
    private int table_size;
    private double frame_time;
    private double frame_progress;
    private byte[] lookup_table;

    public static FontGlyph Init(string src_atlas, string suffix, bool allow_animation) {
        Atlas atlas = Atlas.Init(src_atlas);
        if (atlas == null || StringUtils.IsEmpty(atlas.texture_filename)) {
            if (atlas != null) atlas.Destroy();
            Logger.Warn("fontglyph_init() missing atlas file or texture filename not specified on " + src_atlas);
            return null;
        }

        string texture_path = atlas.GetTexturePath();
        FontGlyph fontglyph;

        Texture texture = Texture.Init(texture_path);
        if (texture != null) {
            fontglyph = FontGlyph.Init2(texture, atlas, suffix, allow_animation);
            if (fontglyph == null) Logger.Warn($"fontglyph_init() failed for {src_atlas}");
        } else {
            fontglyph = null;
            Logger.Error($"fontglyph_init() texture specified by atlas not found: {texture_path}");
        }

        atlas.Destroy();

        return fontglyph;
    }

    public static FontGlyph Init2(Texture texture, Atlas atlas, string suffix, bool allow_animation) {
        FontGlyph fontglyph = new FontGlyph() {
            texture = texture,
            table = EngineUtils.CreateArray<GlyphInfo>(atlas.size),
            table_size = atlas.size,// temporal value

            frame_time = 0f,
            frame_progress = 0f,
            lookup_table = new byte[FontGlyph.LOOKUP_TABLE_LENGTH]
        };

        Array.Fill(fontglyph.lookup_table, FontGlyph.LOOKUP_TABLE_LENGTH, 0, FontGlyph.LOOKUP_TABLE_LENGTH);

        if (allow_animation) {
            if (atlas.glyph_fps > 0)
                fontglyph.frame_time = atlas.glyph_fps;
            else
                fontglyph.frame_time = Funkin.DEFAULT_ANIMATIONS_FRAMERATE;

            fontglyph.frame_time = 1000 / fontglyph.frame_time;
        }

        int table_index = 0;

        // calculate the amount of matching glyphs in the atlas
        for (int i = 0 ; i < atlas.size ; i++) {
            bool result = FontGlyph.InternalParse(
                atlas.entries[i], suffix, allow_animation, fontglyph.table, table_index
            );
            if (result) table_index++;
        }

        if (table_index < 1) {
            Logger.Warn($"fontglyph_init2() failed, there no usable glyphs in the atlas suffix=${suffix}");
            //free(fontglyph.table);
            //free(fontglyph);
            return null;
        }

        // shrink the table if necessary
        if (table_index < fontglyph.table_size) {
            fontglyph.table_size = table_index;
            EngineUtils.ResizeArray(ref fontglyph.table, table_index);
        }

        // count frames of every added glpyh
        int frame_total = 0;
        for (int i = 0 ; i < fontglyph.table_size ; i++) {
            frame_total += fontglyph.table[i].frames_size;
        }

        // allocate frames array and set to zero each glyph frame count
        for (int i = 0, j = 0 ; i < fontglyph.table_size ; i++) {
            int frames_size = fontglyph.table[i].frames_size;
            if (frames_size > 0) {
                fontglyph.table[i].frames = EngineUtils.CreateArray<GlyphFrame>(frames_size);
                fontglyph.table[i].frames_size = 0;
                j += frames_size;
            }
        }

        // add frames to each glyph
        for (int i = 0 ; i < atlas.size ; i++) {
            FontGlyph.InternalParse(
                atlas.entries[i], suffix, allow_animation, fontglyph.table, table_index
            );
        }

        // sort table, place ascii characters first
        EngineUtils.Sort(fontglyph.table, 0, fontglyph.table_size, FontGlyph.InternalTableSort);

        // populate lookup table
        for (byte i = 0 ; i < fontglyph.table_size && i <= FontGlyph.LOOKUP_TABLE_LENGTH ; i++) {
            int code = fontglyph.table[i].code;
            if (code < FontGlyph.LOOKUP_TABLE_LENGTH) {
                fontglyph.lookup_table[code] = i;
            }
        }

        return fontglyph;
    }

    public void Destroy() {
        //if (this == null) return;

        //for (int i = 0; i < this.table_size; i++) {
        //    free(this.table[i].frames);
        //}
        //free(this.frames_array);
        //free(this.table);

        this.texture.Destroy();
    }


    public float Measure(ref FontParams @params, string text, int text_index, int text_length) {
        Grapheme grapheme = new Grapheme();
        int text_end_index = text_index + text_length;

        //Debug.Assert(text_end_index <= text_length, "invalid text_index/text_size (overflow)");

        float width = 0;
        float max_width = 0;
        int line_chars = 0;
        float space_width = InternalFindSpaceWidth(@params.height);

        for (int i = text_index ; i < text_end_index ; i++) {
            if (!StringUtils.GetCharacterCodepoint(text, i, ref grapheme)) continue;
            i += grapheme.size - 1;

            if (grapheme.code == FontGlyph.LINEFEED) {
                if (width > max_width) max_width = width;
                width = 0;
                line_chars = 0;
                continue;
            }

            GlyphInfo info = null;

            if (grapheme.code < FontGlyph.LOOKUP_TABLE_LENGTH) {
                if (this.lookup_table[grapheme.code] != FontGlyph.LOOKUP_TABLE_LENGTH) {
                    byte index = this.lookup_table[grapheme.code];
                    info = this.table[index];
                    goto L_measure;
                }
            }

            for (int j = 0 ; j < this.table_size ; j++) {
                if (this.table[j].code == grapheme.code) {
                    info = this.table[j];
                    goto L_measure;
                }
            }

            if (grapheme.code == FontGlyph.TAB) {
                int filler = FontGlyph.InternalCalcTabstop(line_chars);
                if (filler > 0) {
                    width += space_width * filler;
                    line_chars += filler;
                }
            } else {
                // space, hard space or unknown character
                width += @params.height * FontGlyph.SPACE_WIDTH_RATIO;
                line_chars++;
            }

            continue;

L_measure:
            GlyphFrame frame = info.frames[info.actual_frame];
            width += frame.glyph_width_ratio * @params.height;
            line_chars++;
        }

        return Math.Max(width, max_width);
    }

    public void MeasureChar(int codepoint, float height, ref FontLineInfo lineinfo) {
        if (lineinfo.space_width < 0) {
            lineinfo.space_width = InternalFindSpaceWidth(height);
        }

        GlyphInfo info = null;

        if (codepoint < FontGlyph.LOOKUP_TABLE_LENGTH) {
            if (this.lookup_table[codepoint] != FontGlyph.LOOKUP_TABLE_LENGTH) {
                byte index = this.lookup_table[codepoint];
                info = this.table[index];
                goto L_measure;
            }
        }

        for (int i = 0 ; i < this.table_size ; i++) {
            if (this.table[i].code == codepoint) {
                info = this.table[i];
                goto L_measure;
            }
        }

        if (codepoint == FontGlyph.TAB) {
            int filler = FontGlyph.InternalCalcTabstop(lineinfo.line_char_count);
            if (filler > 0) {
                lineinfo.last_char_width = lineinfo.space_width * filler;
                lineinfo.line_char_count += filler;
            }
        } else {
            // space, hard space or unknown character
            lineinfo.last_char_width = height * FontGlyph.SPACE_WIDTH_RATIO;
            lineinfo.line_char_count++;
        }

        return;

L_measure:
        GlyphFrame frame = info.frames[info.actual_frame];
        lineinfo.last_char_width = frame.glyph_width_ratio * height;
        lineinfo.line_char_count++;
    }

    public float DrawText(PVRContext pvrctx, ref FontParams @params, float x, float y, int text_index, int text_length, string text) {
        Grapheme grapheme = new Grapheme();
        bool by_add = @params.color_by_addition;
        bool has_border = @params.border_enable && @params.border_color[3] > 0 && @params.border_size >= 0;
        float outline_size = @params.border_size * 2;
        int text_end_index = text_index + text_length;

        //Debug.Assert(text_end_index <= text_length, "invalid text_index/text_size (overflow)");

        float draw_x = 0;
        float draw_y = 0;
        int index = text_index;
        int total_glyphs = 0;
        int line_chars = 0;

        // get space glyph width (if present)
        float space_width = InternalFindSpaceWidth(@params.height);

        this.texture.UploadToPVR();
        pvrctx.Save();
        pvrctx.SetVertexAlpha(@params.tint_color[3]);

        // count required glyphs
        while (index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, ref grapheme)) {
            index += grapheme.size;

            switch (grapheme.code) {
                case FontGlyph.CARRIAGERETURN:
                case FontGlyph.LINEFEED:
                    continue;
            }

            if (grapheme.code < FontGlyph.LOOKUP_TABLE_LENGTH && this.lookup_table[grapheme.code] != FontGlyph.LOOKUP_TABLE_LENGTH) {
                total_glyphs++;
                continue;
            }

            for (int i = 0 ; i < this.table_size ; i++) {
                if (this.table[i].code == grapheme.code) {
                    total_glyphs++;
                    break;
                }
            }
        }

        // prepare vertex buffer
        if (has_border) total_glyphs *= 2;
        int added = 0;
        int maximum = GlyphRenderer.Prepare(total_glyphs, has_border);

        index = text_index;
        while (added < maximum && index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, ref grapheme)) {
            index += grapheme.size;

            // ignore "\r" characters
            if (grapheme.code == FontGlyph.CARRIAGERETURN) continue;

            if (grapheme.code == FontGlyph.LINEFEED) {
                draw_y += @params.height + @params.paragraph_space;
                draw_x = 0;
                line_chars = 0;
                continue;
            }

            GlyphFrame frame = null;

            if (grapheme.code < FontGlyph.LOOKUP_TABLE_LENGTH && this.lookup_table[grapheme.code] != FontGlyph.LOOKUP_TABLE_LENGTH) {
                byte in_table_index = this.lookup_table[grapheme.code];
                frame = this.table[in_table_index].frames[this.table[in_table_index].actual_frame];
            } else {
                for (int i = 0 ; i < this.table_size ; i++) {
                    if (this.table[i].code == grapheme.code) {
                        frame = this.table[i].frames[this.table[i].actual_frame];
                        break;
                    }
                }
            }

            if (frame == null) {
                if (grapheme.code == FontGlyph.TAB) {
                    int filler = FontGlyph.InternalCalcTabstop(line_chars);
                    if (filler > 0) {
                        draw_x += space_width * filler;
                        line_chars += filler;
                    }
                } else {
                    // space, hard space or unknown characters
                    draw_x += @params.height * FontGlyph.SPACE_WIDTH_RATIO;
                    line_chars++;
                }
                continue;
            }

            // calculate glyph bounds
            float ratio_width, ratio_height;
            float dx = x + draw_x;
            float dy = y + draw_y;
            float dh = @params.height;
            float dw = dh * frame.glyph_width_ratio;

            if (frame.frame_width > 0) {
                ratio_width = dw / frame.frame_width;
                dw = frame.width * ratio_width;
            } else {
                ratio_width = dw / frame.width;
            }

            if (frame.frame_height > 0) {
                ratio_height = dh / frame.frame_height;
                dh = frame.height * ratio_height;
            } else {
                ratio_height = dh / frame.height;
            }

            dx += (frame.pivot_x + frame.frame_x) * ratio_width;
            dy += (frame.pivot_y + frame.frame_y) * ratio_height;

            if (has_border) {
                float sdx = dx - @params.border_size;
                float sdy = dy - @params.border_size;
                float sdw = dw + outline_size;
                float sdh = dh + outline_size;

                sdx += @params.border_offset_x;
                sdy += @params.border_offset_y;

                GlyphRenderer.AppendGlyph(
                    this.texture, false, true,
                    frame.x, frame.y, frame.width, frame.height,
                    sdx, sdy, sdw, sdh
                );
            }

            GlyphRenderer.AppendGlyph(
                this.texture, false, false,
                frame.x, frame.y, frame.width, frame.height,
                dx, dy, dw, dh
            );

            draw_x += dw;
            line_chars++;
        }

        GlyphRenderer.Draw(pvrctx, @params.tint_color, @params.border_color, by_add, false, this.texture, null);

        pvrctx.Restore();
        return draw_y + @params.height;
    }

    public int Animate(float elapsed) {
        if (this.frame_time <= 0) return 1;

        int frame_index = (int)(this.frame_progress / this.frame_time);

        for (int i = 0 ; i < this.table_size ; i++) {
            if (this.table[i].frames_size < 2) continue;
            this.table[i].actual_frame = frame_index % this.table[i].frames_size;
        }

        this.frame_progress += elapsed;
        return 0;
    }

    public void MapCodepoints(string text, int text_index, int text_length) {
        // unused
    }


    private static bool InternalParse(AtlasEntry atlas_entry, string match_suffix, bool allow_animation, GlyphInfo[] table, int table_index) {
        string atlas_entry_name = atlas_entry.name;
        int atlas_entry_name_length = atlas_entry_name.Length;
        Grapheme grapheme = new Grapheme();

        // read character info
        if (!StringUtils.GetCharacterCodepoint(atlas_entry_name, 0, ref grapheme)) {
            // eof reached
            return false;
        }

        int index = grapheme.size;

        if (StringUtils.IsNotEmpty(match_suffix)) {
            int match_suffix_length = match_suffix.Length;
            int number_suffix_start = index + match_suffix_length + 1;

            if (number_suffix_start > atlas_entry_name_length) {
                // suffix not present
                return false;
            }

            switch (atlas_entry_name.CodePointAtKDY(index)) {
                //case FontGlyph.HARDSPACE:
                case FontGlyph.SPACE:
                    index++;
                    break;
                default:
                    // suffix not present
                    return false;
            }

            // check if the suffix matchs
            if (!atlas_entry_name.StartsWithKDY(match_suffix, index)) {
                // suffix not present
                return false;
            }

            index += match_suffix_length;
        }

        // check if this atlas entry is an animation frame
        if (index < atlas_entry_name_length && !Atlas.NameHasNumberSuffix(atlas_entry_name, index)) {
            // missing number suffix
            return false;
        }

        // check if already exists an entry with this unicode code point
        int codepoint_index = -1;
        for (int i = 0 ; i < table_index ; i++) {
            if (table[i].code == grapheme.code) {
                if (!allow_animation && table[i].frames_size > 0) {
                    // glyph animation is disabled, do not add more frames
                    return false;
                }
                codepoint_index = i;
                break;
            }
        }

        if (codepoint_index < 0) {
            // create entry for this unicode code point
            table[table_index] = new GlyphInfo() {
                code = grapheme.code,
                actual_frame = 0,
                frames = null,
                frames_size = 0
            };
        } else {
            table_index = codepoint_index;
        }

        FontGlyph.InternalAddFrame(atlas_entry, table[table_index]);
        table[table_index].frames_size++;

        // returns true if an entry was added to the table
        return codepoint_index < 0;
    }

    private static void InternalAddFrame(AtlasEntry atlas_entry, GlyphInfo glyph_info) {
        if (glyph_info.frames == null) return;

        float height = atlas_entry.frame_height > 0 ? atlas_entry.frame_height : atlas_entry.height;
        float glyph_width_ratio = 0.0f;
        if (height > 0) {
            // cache this frame width
            float width = atlas_entry.frame_width > 0 ? atlas_entry.frame_width : atlas_entry.width;
            glyph_width_ratio = width / height;
        }

        glyph_info.frames[glyph_info.frames_size] = new GlyphFrame() {
            x = atlas_entry.x,
            y = atlas_entry.y,
            width = atlas_entry.width,
            height = atlas_entry.height,
            frame_x = atlas_entry.frame_x,
            frame_y = atlas_entry.frame_y,
            frame_width = atlas_entry.frame_width,
            frame_height = atlas_entry.frame_height,
            pivot_x = atlas_entry.pivot_x,
            pivot_y = atlas_entry.pivot_y,

            glyph_width_ratio = glyph_width_ratio
        };
    }

    internal static int InternalCalcTabstop(int characters_in_the_line) {
        int space = characters_in_the_line % FontGlyph.TABSTOP;
        if (space == 0) return 0;
        return FontGlyph.TABSTOP - space;
    }

    private float InternalFindSpaceWidth(float height) {
        for (int i = 0 ; i < this.table_size ; i++) {
            if (this.table[i].code == FontGlyph.SPACE || this.table[i].code == FontGlyph.HARDSPACE) {
                GlyphFrame frame = this.table[i].frames[this.table[i].actual_frame];
                return height * frame.glyph_width_ratio;
            }
        }
        return height * FontGlyph.SPACE_WIDTH_RATIO; ;
    }

    private static int InternalTableSort(GlyphInfo x, GlyphInfo y) {
        return x.code.CompareTo(y.code);
    }


    private class GlyphInfo {
        public GlyphFrame[] frames;
        public int frames_size;
        public int code;
        public int actual_frame;
    }

    private class GlyphFrame {
        public float x;
        public float y;
        public float width;
        public float height;
        public float frame_x;
        public float frame_y;
        public float frame_width;
        public float frame_height;
        public float pivot_x;
        public float pivot_y;
        public float glyph_width_ratio;
    }

}
