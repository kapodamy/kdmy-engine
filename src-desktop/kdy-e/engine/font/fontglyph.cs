using System;
using System.Diagnostics;
using Engine.Game.Common;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Font;

public class FontLineInfo {
    public float space_width;
    public float last_char_width;
    public int line_char_count;
    public int previous_codepoint;

    public FontLineInfo() {
        this.space_width = -1;
    }
}

public class FontGlyph : IFontRender {

    public const byte SPACE = 0x20;
    public const byte HARDSPACE = 0xA0;
    public const byte LINEFEED = 0x0A;
    public const byte TAB = 0x09;
    public const byte CARRIAGERETURN = 0x0D;
    public const float SPACE_WIDTH_RATIO = 0.8f;
    public const byte TABSTOP = 8;

    private FontGlyph() { }

    private Texture texture;
    private GlyphInfo[] table;
    private int table_size;
    private float paragraph_separation;
    private float[] tintcolor;
    private float alpha;
    private bool color_by_difference;
    private float[] border_tintcolor;
    private float border_size;
    private bool border_enable;
    private float border_offset_x;
    private float border_offset_y;
    private double frame_time;
    private double frame_progress;

    public static FontGlyph Init(string src_atlas, string suffix, bool allow_animation) {
        Atlas atlas = Atlas.Init(src_atlas);
        if (atlas == null || String.IsNullOrEmpty(atlas.texture_filename)) {
            if (atlas != null) atlas.Destroy();
            Logger.Warn("fontglyph_init() missing atlas file or texture filename not specified");
            return null;
        }

        string texture_path = atlas.GetTexturePath();
        FontGlyph fontglyph;

        Texture texture = Texture.Init(texture_path);
        if (texture != null) {
            fontglyph = FontGlyph.Init2(texture, atlas, suffix, allow_animation);
        } else {
            fontglyph = null;
            Logger.Warn($"fontglyph_init() texture specified by atlas not found: {texture_path}");
        }

        atlas.Destroy();

        return fontglyph;
    }

    public static FontGlyph Init2(Texture texture, Atlas atlas, string suffix, bool allow_animation) {
        FontGlyph fontglyph = new FontGlyph() {
            texture = texture,
            table = new GlyphInfo[atlas.size],
            table_size = atlas.size,// temporal value
            paragraph_separation = 0.0f,
            tintcolor = new float[] { 1.0f, 1.0f, 1.0f },
            alpha = 1.0f,

            color_by_difference = false,

            border_tintcolor = new float[] { 0.0f, 0.0f, 0.0f, 0.0f },
            border_size = 0f,
            border_enable = false,
            border_offset_x = 0f,
            border_offset_y = 0f,

            frame_time = 0f,
            frame_progress = 0f,
        };

        if (allow_animation) {
            if (atlas.glyph_fps > 0)
                fontglyph.frame_time = atlas.glyph_fps;
            else
                fontglyph.frame_time = Funkin.DEFAULT_ANIMATIONS_FRAMERATE;

            fontglyph.frame_time = 1000 / fontglyph.frame_time;
        }

        PVRContext.HelperClearOffsetColor(fontglyph.border_tintcolor);

        int table_index = 0;

        for (int i = 0 ; i < atlas.size ; i++) {
            int result = FontGlyph.InternalParse(
                atlas.entries[i], suffix, allow_animation, fontglyph.table, table_index
            );
            if (result == 1) table_index++;
        }

        // shrink the table if necessary
        if (table_index < fontglyph.table_size) {
            fontglyph.table_size = table_index;
            Array.Resize(ref fontglyph.table, table_index);
        }

        // convert all linkedlists into arrays
        for (int i = 0 ; i < fontglyph.table_size ; i++) {
            fontglyph.table[i].frames_size = fontglyph.table[i].frames_temp.Count();
            fontglyph.table[i].frames = fontglyph.table[i].frames_temp.ToSolidArray( /*sizeof(frame)*/);

            fontglyph.table[i].frames_temp.Destroy2(/*free*/);
            fontglyph.table[i].frames_temp = null;
        }

        return fontglyph;
    }

    public void Destroy() {
        //if (this == null) return;

        //for (int i = 0; i < this.table_size; i++) {
        //    free(this.table[i].frames);
        //}
        //free(this.table);

        this.texture.Destroy();
    }


    public float Measure(float height, string text, int text_index, int text_size) {
        Grapheme grapheme = new Grapheme();
        int text_length = text.Length;
        int text_end_index = text_index + text_size;

        Debug.Assert(text_end_index <= text_length, "invalid text_index/text_size (overflow)");

        float width = 0;
        float max_width = 0;
        int line_chars = 0;
        float space_width = InternalFindSpaceWidth(height);

        for (int i = text_index ; i < text_end_index ; i++) {
            if (!StringUtils.GetCharacterCodepoint(text, i, ref grapheme)) continue;
            i += grapheme.size - 1;

            if (grapheme.code == FontGlyph.LINEFEED) {
                if (width > max_width) max_width = width;
                width = 0;
                line_chars = 0;
                continue;
            }

            bool found = false;

            for (int j = 0 ; j < this.table_size ; j++) {
                if (this.table[j].code == grapheme.code) {
                    GlyphFrame frame = this.table[j].frames[this.table[j].actual_frame];
                    width += frame.glyph_width_ratio * height;
                    line_chars++;
                    found = true;
                    break;
                }
            }

            if (found) continue;

            if (grapheme.code == FontGlyph.TAB) {
                int filler = FontGlyph.InternalCalcTabstop(line_chars);
                if (filler > 0) {
                    width += space_width * filler;
                    line_chars += filler;
                }
            } else {
                // space, hard space or unknown character
                width += height * FontGlyph.SPACE_WIDTH_RATIO;
                line_chars++;
            }
        }

        return Math.Max(width, max_width);
    }

    public void MeasureChar(int codepoint, float height, FontLineInfo lineinfo) {
        if (lineinfo.space_width < 0) {
            lineinfo.space_width = InternalFindSpaceWidth(height);
        }

        for (int j = 0 ; j < this.table_size ; j++) {
            if (this.table[j].code == codepoint) {
                GlyphFrame frame = this.table[j].frames[this.table[j].actual_frame];
                lineinfo.last_char_width = frame.glyph_width_ratio * height;
                lineinfo.line_char_count++;
                return;
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
    }

    public void SetLinesSeparation(float height) {
        this.paragraph_separation = height;
    }

    public void SetColor(float r, float g, float b) {
        this.tintcolor[0] = r;
        this.tintcolor[1] = g;
        this.tintcolor[2] = b;
    }

    public void SetRGB8Color(uint rbg8_color) {
        Math2D.ColorBytesToFloats(rbg8_color, false, this.tintcolor);
    }

    public void SetAlpha(float alpha) {
        this.alpha = alpha;
    }

    public void SetBorderColorRGBA8(uint rbga8_color) {
        Math2D.ColorBytesToFloats(rbga8_color, true, this.border_tintcolor);
    }

    public void SetBorderColor(float r, float g, float b, float a) {
        if (r >= 0) this.border_tintcolor[0] = r;
        if (g >= 0) this.border_tintcolor[1] = g;
        if (b >= 0) this.border_tintcolor[2] = b;
        if (a >= 0) this.border_tintcolor[3] = a;
    }

    public void SetBorderSize(float size) {
        this.border_size = size;
    }

    public void SetBorderOffset(float x, float y) {
        this.border_offset_x = x;
        this.border_offset_y = y;
    }

    public void EnableBorder(bool enable) {
        this.border_enable = enable;
    }

    public void SetBorder(bool enable, float size, float[] rgba) {
        this.border_enable = enable;
        this.border_size = size;
        SetBorderColor(rgba[0], rgba[1], rgba[2], rgba[3]);
    }

    public void EnableColorByDifference(bool enable) {
        this.color_by_difference = enable;
    }


    public float DrawText(PVRContext pvrctx, float height, float x, float y, int text_index, int text_size, string text) {
        Grapheme grapheme = new Grapheme();
        bool by_diff = this.color_by_difference;
        bool has_border = this.border_enable && this.border_tintcolor[3] > 0 && this.border_size >= 0;
        float outline_size = this.border_size * 2;
        int text_end_index = text_index + text_size;
        int text_length = text.Length;

        Debug.Assert(text_end_index <= text_length, "invalid text_index/text_size (overflow)");

        float draw_x = 0;
        float draw_y = 0;
        int index = text_index;
        int total_glyphs = 0;
        int line_chars = 0;

        // get space glyph width (if present)
        float space_width = InternalFindSpaceWidth(height);

        this.texture.UploadToPVR();
        pvrctx.Save();
        pvrctx.SetVertexAlpha(this.alpha);

        // count required glyphs
        while (index < text_end_index && StringUtils.GetCharacterCodepoint(text, index, ref grapheme)) {
            index += grapheme.size;

            switch (grapheme.code) {
                case FontGlyph.CARRIAGERETURN:
                case FontGlyph.LINEFEED:
                    continue;
            }

            for (int i = 0 ; i < this.table_size ; i++) {
                if (this.table[i].code == grapheme.code) {
                    switch (grapheme.code) {
                        case FontGlyph.CARRIAGERETURN:
                        case FontGlyph.LINEFEED:
                            continue;
                    }
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
                draw_y += height + this.paragraph_separation;
                draw_x = 0;
                line_chars = 0;
                continue;
            }

            GlyphFrame frame = null;
            for (int i = 0 ; i < this.table_size ; i++) {
                if (this.table[i].code == grapheme.code) {
                    frame = this.table[i].frames[this.table[i].actual_frame];
                    break;
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
                    draw_x += height * FontGlyph.SPACE_WIDTH_RATIO;
                    line_chars++;
                }
                continue;
            }

            // calculate glyph bounds
            float ratio_width, ratio_height;
            float dx = x + draw_x;
            float dy = y + draw_y;
            float dh = height;
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
                float sdx = dx - this.border_size;
                float sdy = dy - this.border_size;
                float sdw = dw + outline_size;
                float sdh = dh + outline_size;

                sdx += this.border_offset_x;
                sdy += this.border_offset_y;

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

        GlyphRenderer.Draw(pvrctx, this.tintcolor, this.border_tintcolor, by_diff, false, this.texture, null);

        pvrctx.Restore();
        return draw_y + height;
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


    private static int InternalParse(AtlasEntry atlas_entry, string match_suffix, bool allow_animation, GlyphInfo[] table, int table_index) {
        string atlas_entry_name = atlas_entry.name;
        int atlas_entry_name_length = atlas_entry_name.Length;
        Grapheme grapheme = new Grapheme();

        // read character info
        if (!StringUtils.GetCharacterCodepoint(atlas_entry_name, 0, ref grapheme)) return -1;

        int index = grapheme.size;

        if (!String.IsNullOrEmpty(match_suffix)) {
            int match_suffix_length = match_suffix.Length;
            int number_suffix_start = index + match_suffix_length + 1;

            if (number_suffix_start > atlas_entry_name_length) return 0;// suffix not present

            switch (atlas_entry_name.CodePointAtKDY(index)) {
                case FontGlyph.SPACE:
                case FontGlyph.HARDSPACE:
                    index++;
                    break;
                default:
                    return 0;// suffix not present
            }

            // check if the suffix matchs
            if (!atlas_entry_name.StartsWithKDY(match_suffix, index)) return 0;

            index += match_suffix_length;
        }

        // check if this atlas entry is an animation frame
        if (index < atlas_entry_name_length) {
            if (!Atlas.NameHasNumberSuffix(atlas_entry_name, index)) return 0;// suffix found

            // check if already exists an entry with this unicode code point
            int code_index = -1;
            for (int i = 0 ; i < table_index ; i++) {
                if (table[i].code == grapheme.code) {
                    code_index = i;
                    break;
                }
            }

            if (code_index >= 0) {
                if (!allow_animation) return 0;// reject, animation is disabled
                                               // add another frame
                GlyphFrame glyph_frame = FontGlyph.InternalBuildFrame(atlas_entry);
                table[code_index].frames_temp.AddItem(glyph_frame);
                return 2;
            }
        }

        GlyphFrame frame = FontGlyph.InternalBuildFrame(atlas_entry);

        // create entry for this unicode code point
        table[table_index] = new GlyphInfo() {
            code = grapheme.code,
            actual_frame = 0,
            frames = null,
            frames_size = 0
        };
        table[table_index].actual_frame = 0;// index in the (future) frames array

        table[table_index].frames_temp = new LinkedList<GlyphFrame>();
        table[table_index].frames_temp.AddItem(frame);

        return 1;
    }

    private static GlyphFrame InternalBuildFrame(AtlasEntry atlas_entry) {
        GlyphFrame frame = new GlyphFrame() {
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

            glyph_width_ratio = 0.0f
        };

        float height = atlas_entry.frame_height > 0 ? atlas_entry.frame_height : atlas_entry.height;

        if (height > 0) {
            // cache this frame width
            float width = atlas_entry.frame_width > 0 ? atlas_entry.frame_width : atlas_entry.width;
            frame.glyph_width_ratio = width / height;
        }

        return frame;
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


    private class GlyphInfo {
        public LinkedList<GlyphFrame> frames_temp;
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
