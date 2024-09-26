using System;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Font;

public class TextSprite : IVertex {

    private static Map<TextSprite> POOL = new Map<TextSprite>();
    private static int IDS = 0;


    private TextSprite() { }


    private IFont font;
    private string text;
    private bool intern;
    private int force_case;
    private int last_force_case;
    private string text_forced_case;
    private bool visible;
    private float x;
    private float y;
    private float z;
    private float alpha2;
    private float z_offset;
    private Align align_vertical;
    private Align align_horizontal;
    private float max_width;
    private float max_height;
    private int max_lines;
    private Align paragraph_align;
    private Modifier matrix_source;
    private CornerRotation matrix_corner;
    private bool flip_x;
    private bool flip_y;
    private ArrayList<ParagraphInfo> paragraph_array;
    private bool modified_coords;
    private bool modified_string;
    private float last_draw_x;
    private float last_draw_y;
    private float last_draw_width;
    private float last_draw_height;
    private AnimSprite animation_external;
    private AnimSprite animation_selected;
    private PVRFlag antialiasing;
    private int wordbreak;
    private PSShader psshader;
    private bool blend_enabled;
    private Blend blend_src_rgb;
    private Blend blend_dst_rgb;
    private Blend blend_src_alpha;
    private Blend blend_dst_alpha;
    private bool background_enabled;
    private float background_size;
    private float background_offset_x;
    private float background_offset_y;
    private float[] background_rgba;
    private FontParams fontparams;
    private int id;


    public static TextSprite Init(IFont font, bool font_is_truetype, bool color_by_addition, float size, uint rbg8_color) {

        TextSprite textsprite = new TextSprite() {
            font = font,

            text = null,
            intern = false,

            force_case = VertexProps.TEXTSPRITE_FORCE_NONE,
            last_force_case = VertexProps.TEXTSPRITE_FORCE_NONE,
            text_forced_case = null,

            visible = true,
            x = 0.0f,
            y = 0.0f,
            z = 0.0f,
            alpha2 = 1.0f,
            z_offset = 0.0f,

            align_vertical = Align.START,
            align_horizontal = Align.START,
            max_width = -1.0f,
            max_height = -1.0f,

            max_lines = -1,
            paragraph_align = Align.START,

            matrix_source = new Modifier(),
            matrix_corner = { x = 0, y = 0, angle = 0 },
            flip_x = false,
            flip_y = false,

            paragraph_array = new ArrayList<ParagraphInfo>(),

            modified_coords = false,
            modified_string = false,

            last_draw_x = 0.0f,
            last_draw_y = 0.0f,

            last_draw_width = 0.0f,
            last_draw_height = 0.0f,

            animation_external = null,
            animation_selected = null,


            antialiasing = PVRFlag.DEFAULT,

            wordbreak = VertexProps.FONT_WORDBREAK_LOOSE,

            psshader = null,

            blend_enabled = true,// obligatory
            blend_src_rgb = Blend.DEFAULT,
            blend_dst_rgb = Blend.DEFAULT,
            blend_src_alpha = Blend.DEFAULT,
            blend_dst_alpha = Blend.DEFAULT,

            background_enabled = false,
            background_size = 0,
            background_offset_x = 0,
            background_offset_y = 0,
            background_rgba = new float[] { 0.0f, 0.0f, 0.0f, 0.5f },

            fontparams = {
                height = size,
                paragraph_space = 0f,
                color_by_addition = color_by_addition,

                tint_color = new float[] { 1f, 1f, 1f, 1f },

                border_enable = false,
                border_size = 0f,

                border_color = new float[] { 0f, 0f, 0f, 1f },
                border_offset_x = 0f,
                border_offset_y = 0f
            },

            id = TextSprite.IDS++
        };

        Math2D.ColorBytesToFloats(rbg8_color, false, textsprite.fontparams.tint_color);

        textsprite.matrix_source.Clear();

        TextSprite.POOL.Set(textsprite.id, textsprite);
        return textsprite;
    }

    public static TextSprite Init2(FontHolder fontholder, float font_size, uint rbg8_color) {
        return TextSprite.Init(fontholder.font, !fontholder.font_from_atlas, fontholder.font_color_by_addition, font_size, rbg8_color);
    }

    public void Destroy() {
        this.font = null;// set to null only

        //if (!this.intern) free(this.text);

        //if (this.text_forced_case) free(this.text_forced_case);

        this.paragraph_array.Destroy();

        if (this.animation_selected != null) this.animation_selected.Destroy();

        TextSprite.POOL.Delete(this.id);
        Luascript.DropShared(this.matrix_source);
        Luascript.DropShared(this);
        //free(textsprite);
    }


    public void SetText(string text) {
        SetTextIntern(false, text);
    }

    public void SetTextIntern(bool intern, string text) {
        ///if (!this.intern) //free(this.text);

        this.intern = intern;
        //if (intern)
        /**/
        this.text = text;
        //else
        //    this.text = textstrdup(text);

        this.modified_string = true;
    }

    public void SetTextFormated(string format, params object[] values) {
        string text = StringUtils.CreateFormattedString(format, values);

        //if (!this.intern) free(this.text);

        this.intern = false;
        this.text = text;

        this.modified_string = true;
    }

    public void SetTextFormated2(string format, object[] va_args) {
        string text = StringUtils.CreateFormattedString(format, va_args);

        //if (!this.intern) free(this.text);

        this.intern = false;
        this.text = text;

        this.modified_string = true;
    }

    public void SetFontSize(float font_size) {
        this.modified_coords = true;
        this.fontparams.height = font_size;
    }

    public void ForceCase(int none_or_lowercase_or_uppercase) {
        this.modified_coords = true;
        this.force_case = none_or_lowercase_or_uppercase;
    }

    public void SetParagraphAlign(Align align) {
        this.modified_coords = true;
        this.paragraph_align = align;
    }

    public void SetParagraphSpace(float space) {
        this.fontparams.paragraph_space = space;
    }

    public void SetMaxlines(int max_lines) {
        this.modified_coords = true;
        this.max_lines = max_lines;
    }

    public void SetColorRGBA8(uint rbg8_color) {
        Math2D.ColorBytesToFloats(rbg8_color, false, this.fontparams.tint_color);
    }

    public void SetColor(float r, float g, float b) {
        if (r >= 0) this.fontparams.tint_color[0] = r;
        if (g >= 0) this.fontparams.tint_color[1] = g;
        if (b >= 0) this.fontparams.tint_color[2] = b;
    }


    public void SetAlpha(float alpha) {
        this.fontparams.tint_color[3] = alpha;
    }

    public void SetVisible(bool visible) {
        this.visible = visible;
    }

    public void SetDrawLocation(float x, float y) {
        this.modified_coords = true;
        if (!Single.IsNaN(x)) this.x = x;
        if (!Single.IsNaN(y)) this.y = y;
    }

    public void GetDrawLocation(out float draw_x, out float draw_y) {
        draw_x = this.x;
        draw_y = this.y;
    }

    public void SetZIndex(float z_index) {
        this.z = z_index;
    }

    public void SetZOffset(float offset) {
        this.z_offset = offset;
    }

    public float GetZIndex() {
        return this.z + this.z_offset;
    }

    public void SetMaxDrawSize(float max_width, float max_height) {
        this.modified_coords = true;
        if (!Single.IsNaN(max_width)) this.max_width = max_width;
        if (!Single.IsNaN(max_height)) this.max_height = max_height;
    }

    public void MatrixFlip(bool? flip_x, bool? flip_y) {
        this.modified_coords = true;
        if (flip_x != null) this.flip_x = flip_x == true;
        if (flip_y != null) this.flip_y = flip_y == true;
    }


    public void SetAlign(Align align_vertical, Align align_horizontal) {
        this.modified_coords = true;
        if (align_vertical != Align.NONE) {
            if (align_vertical == Align.BOTH) align_vertical = Align.START;
            this.align_vertical = align_vertical;
        }
        if (align_horizontal != Align.NONE) {
            if (align_horizontal == Align.BOTH) align_horizontal = Align.START;
            this.align_horizontal = align_horizontal;
        }
    }

    public void SetAlignVertical(Align align) {
        this.modified_coords = true;
        this.align_vertical = align;
    }

    public void SetAlignHorizontal(Align align) {
        this.modified_coords = true;
        this.align_horizontal = align;
    }


    public Modifier MatrixGetModifier() {
        return this.matrix_source;
    }

    public void MatrixReset() {
        this.matrix_source.Clear();
        this.flip_x = this.flip_x = false;
        this.matrix_corner.Clear();
    }

    public void MatrixTranslate(float translate_x, float translate_y) {
        if (!Single.IsNaN(translate_x))
            this.matrix_source.translate_x = translate_x;
        if (!Single.IsNaN(translate_y))
            this.matrix_source.translate_y = translate_y;
    }

    public void MatrixSkew(float skew_x, float skew_y) {
        if (!Single.IsNaN(skew_x))
            this.matrix_source.skew_x = skew_x;
        if (!Single.IsNaN(skew_y))
            this.matrix_source.skew_y = skew_y;
    }

    public void MatrixScale(float scale_x, float scale_y) {
        if (!Single.IsNaN(scale_x))
            this.matrix_source.scale_x = scale_x;
        if (!Single.IsNaN(scale_y))
            this.matrix_source.scale_y = scale_y;
    }

    public void MatrixRotate(float degrees_angle) {
        this.matrix_source.rotate = degrees_angle * Math2D.DEG_TO_RAD;
    }


    public void MatrixRotatePivotEnable(bool enable) {
        this.matrix_source.rotate_pivot_enabled = enable;
    }

    public void MatrixRotatePivot(float u, float v) {
        if (!Single.IsNaN(u)) this.matrix_source.rotate_pivot_u = u;
        if (!Single.IsNaN(v)) this.matrix_source.rotate_pivot_v = v;
    }


    public void MatrixScaleOffset(float direction_x, float direction_y) {
        if (!Single.IsNaN(direction_x)) this.matrix_source.scale_direction_x = direction_x;
        if (!Single.IsNaN(direction_y)) this.matrix_source.scale_direction_y = direction_y;
    }


    public void MatrixCornerRotation(Corner corner) {
        this.matrix_corner.HelperCalculateCornerRotation(corner);
    }


    public void MatrixCalculate(PVRContext pvrctx) {
        SIMDMatrix matrix = pvrctx.CurrentMatrix;

        // step 1: flip text (if required)
        if (this.flip_x || this.flip_y) {
            matrix.Scale(
                this.flip_x ? -1 : 1,
                this.flip_y ? -1 : 1
            );
        }

        // step 2: apply self modifier
        matrix.ApplyModifier2(
            this.matrix_source,
            this.last_draw_x, this.last_draw_y,
            this.last_draw_width, this.last_draw_height
        );

        // step 3: apply corner rotation
        if (this.matrix_corner.angle != 0) {
            matrix.CornerRotate(
                this.matrix_corner,
                this.last_draw_x, this.last_draw_y,
                this.last_draw_width, this.last_draw_height
            );
        }

        //pvrctx.Flush();
    }


    public void CalculateParagraphAlignment() {
        Grapheme grapheme = new Grapheme();
        FontCharInfo char_info = new FontCharInfo();

        if (!this.modified_string && !this.modified_coords) return;

        // check if case VertexProps.TEXTSPRITE_FORCE conversion is required
        if (this.modified_string || this.last_force_case != this.force_case) {
            this.last_force_case = this.force_case;

            //if (this.text_forced_case) free(this.text_forced_case);

            switch (this.force_case) {
                case VertexProps.TEXTSPRITE_FORCE_UPPERCASE:
                    this.text_forced_case = StringUtils.ToUpperCase(this.text);
                    break;
                case VertexProps.TEXTSPRITE_FORCE_LOWERCASE:
                    this.text_forced_case = StringUtils.ToLowerCase(this.text);
                    break;
                //case VertexProps.TEXTSPRITE_FORCE_NONE:
                default:
                    this.text_forced_case = null;
                    break;
            }
        }

        string text = this.text_forced_case ?? this.text;
        this.paragraph_array.Clear();

        if (StringUtils.IsEmpty(text)) {
            this.modified_string = false;
            this.last_draw_width = 0.0f;
            this.last_draw_height = 0.0f;
            return;
        }

        int text_length = text.Length;
        if (this.modified_string) {
            this.font.MapCodepoints(text, 0, text_length);
        }

        // step 1: count amount of required paragraphs
        float line_height = this.font.MeasureLineHeight(this.fontparams.height);
        float max_height;
        if (this.max_lines > 0 || this.max_height >= 0f) {
            float limit1 = this.max_lines < 0 ? Single.PositiveInfinity : (line_height * this.max_lines);
            float limit2 = this.max_height < 0f ? Single.PositiveInfinity : this.max_height;
            max_height = Math.Min(limit1, limit2);
        } else {
            max_height = Single.PositiveInfinity;
        }

        // step 2: build paragraph info array and store in paragraph info offset the paragraph width
        int index = 0;
        int index_previous = 0;
        int index_current_line = 0;
        float accumulated_width = 0f;
        int index_last_detected_break = 0;
        bool last_break_was_dotcommatab = false;
        float calculated_text_height = 0f;
        float max_width = this.max_width < 0f ? Single.PositiveInfinity : this.max_width;
        float max_line_width = 0f;
        int last_known_break_index = 0;
        int loose_index = 0;
        float last_known_break_width = 0f;

        while (true) {
            bool eof_reached = !StringUtils.GetCharacterCodepoint(text, index, ref grapheme);

            if (grapheme.code == FontGlyph.LINEFEED || eof_reached || loose_index > 0) {
                int new_index = index + grapheme.size;
                if (loose_index > 0) {
                    new_index = index = loose_index;
                    loose_index = 0;
                }

                // note: do not ignore empty lines
                this.paragraph_array.Add(new ParagraphInfo() {
                    index = index_current_line,
                    length = index - index_current_line,
                    offset = accumulated_width// temporal
                });

                index_last_detected_break = index_current_line = new_index;
                last_break_was_dotcommatab = true;

                float line_width = accumulated_width + char_info.last_char_width_end;
                if (line_width >= max_line_width) max_line_width = line_width;

                calculated_text_height += line_height;
                if (calculated_text_height >= max_height) break;

                char_info.line_char_count = 0;
                char_info.previous_codepoint = 0x0000;
                index_previous = index;
                index = new_index;
                accumulated_width = 0;
                last_known_break_index = index;
                last_known_break_width = -1;

                if (eof_reached) break;
                continue;
            }

            if (grapheme.code == FontGlyph.CARRIAGERETURN) {
                index += grapheme.size;
                continue;
            }

            // measure char width
            this.font.MeasureChar(grapheme.code, this.fontparams.height, ref char_info);

            // check if the current codepoint is breakable
            bool current_is_break = false;
            int break_in_index = -1;
            int break_char_count = 1;
            uint break_codepoint = grapheme.code;
            float break_width_end = char_info.last_char_width_end;
            float break_width = char_info.last_char_width;

            switch (grapheme.code) {
                case FontGlyph.SPACE:
                    if (last_break_was_dotcommatab) {
                        last_known_break_width = accumulated_width;
                        current_is_break = true;
                    }
                    last_break_was_dotcommatab = false;
                    break;
                case FontGlyph.TAB:
                case 0x2C:// comma
                case 0x2E:// dot
                case 0x3A:// dot dot
                case 0x3B:// dot comma
                    last_known_break_width = accumulated_width;
                    current_is_break = true;
                    last_break_was_dotcommatab = true;
                    break;
                default:
                    last_break_was_dotcommatab = true;
                    break;
            }

            accumulated_width += char_info.last_char_width;

            if (accumulated_width > max_width) {
                if (current_is_break) {
                    break_in_index = index;
                    break_char_count = 0;
                    break_codepoint = 0x0000;
                    break_width = 0;
                } else {
                    switch (this.wordbreak) {
                        case VertexProps.FONT_WORDBREAK_NONE:
                            // do nothing, wait for the next line break
                            break;
                        case VertexProps.FONT_WORDBREAK_LOOSE:
                            // fallback to FONT_WORDBREAK_BREAK if the current word is to long
                            if (index_current_line != index_last_detected_break) {
                                // break and reparse the overflowing word
                                loose_index = last_known_break_index;
                                if (last_known_break_width >= 0f) accumulated_width = last_known_break_width;
                                continue;
                            }
                            goto case VertexProps.FONT_WORDBREAK_BREAK;
                        case VertexProps.FONT_WORDBREAK_BREAK:
                            // break in previous character
                            break_in_index = index_previous;
                            break;
                    }
                }
            } else if (current_is_break) {
                index_last_detected_break = index;
                last_known_break_index = index;
            }

            // commit wordbreak (if required)
            if (break_in_index >= 0) {
                this.paragraph_array.Add(new ParagraphInfo() {
                    index = index_current_line,
                    length = break_in_index - index_current_line,
                    offset = accumulated_width// temporal
                });

                float line_width = accumulated_width + break_width_end;
                if (line_width >= max_line_width) max_line_width = line_width;

                calculated_text_height += line_height;
                if (calculated_text_height >= max_height) break;

                index_last_detected_break = index_current_line = break_in_index;
                last_break_was_dotcommatab = false;

                char_info.line_char_count = break_char_count;
                char_info.previous_codepoint = break_codepoint;
                accumulated_width = break_width;
                last_known_break_index = break_in_index;
                last_known_break_width = -1;
            }

            index_previous = index;
            index += grapheme.size;
        }

        // step 3: align paragraphs
        bool align_to_start = this.paragraph_align == Align.START;
        bool align_to_center = this.paragraph_align == Align.CENTER;

        if (max_line_width == 0f || align_to_start) {
            foreach (ParagraphInfo paragraphinfo in this.paragraph_array) {
                // Align.START is used, put all offsets in zero
                paragraphinfo.offset = 0f;
            }
        } else {
            // step 5: calculate paragraph offsets
            foreach (ParagraphInfo paragraphinfo in this.paragraph_array) {
                if (max_line_width == paragraphinfo.offset) {
                    paragraphinfo.offset = 0f;
                    continue;
                }

                paragraphinfo.offset = max_line_width - paragraphinfo.offset;// align to end
                if (align_to_center) paragraphinfo.offset /= 2f;// align to center
            }
        }

        this.modified_string = false;
        this.last_draw_width = max_line_width;
        this.last_draw_height = calculated_text_height;
    }

    public void SetProperty(int property_id, float value) {
        switch (property_id) {
            case VertexProps.TEXTSPRITE_PROP_FONT_SIZE:
                SetFontSize(value);
                break;
            case VertexProps.TEXTSPRITE_PROP_ALIGN_H:
                SetAlignHorizontal((Align)((int)value));
                break;
            case VertexProps.TEXTSPRITE_PROP_ALIGN_V:
                SetAlignVertical((Align)((int)value));
                break;
            case VertexProps.TEXTSPRITE_PROP_FORCE_CASE:
                ForceCase((int)value);
                break;
            case VertexProps.TEXTSPRITE_PROP_MAX_LINES:
                SetMaxlines((int)value);
                break;
            case VertexProps.TEXTSPRITE_PROP_ALIGN_PARAGRAPH:
                SetParagraphAlign((Align)((int)value));
                break;
            case VertexProps.TEXTSPRITE_PROP_BORDER_ENABLE:
                this.fontparams.border_enable = value >= 1.0f;
                break;
            case VertexProps.TEXTSPRITE_PROP_BORDER_SIZE:
                this.fontparams.border_size = value;
                break;
            case VertexProps.TEXTSPRITE_PROP_BORDER_COLOR_R:
                this.fontparams.border_color[0] = value;
                break;
            case VertexProps.TEXTSPRITE_PROP_BORDER_COLOR_G:
                this.fontparams.border_color[1] = value;
                break;
            case VertexProps.TEXTSPRITE_PROP_BORDER_COLOR_B:
                this.fontparams.border_color[2] = value;
                break;
            case VertexProps.TEXTSPRITE_PROP_BORDER_COLOR_A:
                this.fontparams.border_color[3] = value;
                break;
            case VertexProps.TEXTSPRITE_PROP_BORDER_OFFSET_X:
                this.fontparams.border_offset_x = value;
                break;
            case VertexProps.TEXTSPRITE_PROP_BORDER_OFFSET_Y:
                this.fontparams.border_offset_y = value;
                break;
            /////////////////////////////////////////////////////////////////////////////////////////////////
            case VertexProps.SPRITE_PROP_X:
                this.x = value;
                break;
            case VertexProps.SPRITE_PROP_Y:
                this.y = value;
                break;
            //case VertexProps.SPRITE_PROP_WIDTH:
            case VertexProps.TEXTSPRITE_PROP_MAX_WIDTH:
                this.max_width = value;
                break;
            //case VertexProps.SPRITE_PROP_HEIGHT:
            case VertexProps.TEXTSPRITE_PROP_MAX_HEIGHT:
                this.max_height = value;
                break;
            case VertexProps.SPRITE_PROP_ROTATE:
                MatrixRotate(value);
                break;
            case VertexProps.SPRITE_PROP_SCALE_X:
                MatrixScale(value, Single.NaN);
                break;
            case VertexProps.SPRITE_PROP_SCALE_Y:
                MatrixScale(Single.NaN, value);
                break;
            case VertexProps.SPRITE_PROP_SKEW_X:
                MatrixSkew(value, Single.NaN);
                break;
            case VertexProps.SPRITE_PROP_SKEW_Y:
                MatrixSkew(Single.NaN, value);
                break;
            case VertexProps.SPRITE_PROP_TRANSLATE_X:
                MatrixTranslate(value, Single.NaN);
                break;
            case VertexProps.SPRITE_PROP_TRANSLATE_Y:
                MatrixTranslate(Single.NaN, value);
                break;
            case VertexProps.SPRITE_PROP_ALPHA:
                this.fontparams.tint_color[3] = Math2D.Clamp(value, 0.0f, 1.0f);
                break;
            case VertexProps.SPRITE_PROP_Z:
                this.z = value;
                break;
            case VertexProps.SPRITE_PROP_VERTEX_COLOR_R:
                this.fontparams.tint_color[0] = Math2D.Clamp(value, 0.0f, 1.0f);
                break;
            case VertexProps.SPRITE_PROP_VERTEX_COLOR_G:
                this.fontparams.tint_color[1] = Math2D.Clamp(value, 0.0f, 1.0f);
                break;
            case VertexProps.SPRITE_PROP_VERTEX_COLOR_B:
                this.fontparams.tint_color[2] = Math2D.Clamp(value, 0.0f, 1.0f);
                break;
            case VertexProps.SPRITE_PROP_ANIMATIONLOOP:
                if (this.animation_selected != null)
                    this.animation_selected.SetLoop((int)value);
                break;
            case VertexProps.SPRITE_PROP_ROTATE_PIVOT_ENABLED:
                MatrixRotatePivotEnable(value >= 1.0f);
                break;
            case VertexProps.SPRITE_PROP_ROTATE_PIVOT_U:
                MatrixRotatePivot(value, Single.NaN);
                break;
            case VertexProps.SPRITE_PROP_ROTATE_PIVOT_V:
                MatrixRotatePivot(Single.NaN, value);
                break;
            case VertexProps.SPRITE_PROP_SCALE_DIRECTION_X:
                this.matrix_source.scale_direction_x = value;
                break;
            case VertexProps.SPRITE_PROP_SCALE_DIRECTION_Y:
                this.matrix_source.scale_direction_y = value;
                break;
            case VertexProps.SPRITE_PROP_TRANSLATE_ROTATION:
                this.matrix_source.translate_rotation = value >= 1.0f;
                break;
            case VertexProps.SPRITE_PROP_SCALE_SIZE:
                this.matrix_source.scale_size = value >= 1.0f;
                break;
            case VertexProps.SPRITE_PROP_SCALE_TRANSLATION:
                this.matrix_source.scale_translation = value >= 1.0f;
                break;
            case VertexProps.SPRITE_PROP_Z_OFFSET:
                this.z_offset = value;
                break;
            case VertexProps.TEXTSPRITE_PROP_PARAGRAPH_SEPARATION:
                this.fontparams.paragraph_space = value;
                break;
            case VertexProps.SPRITE_PROP_ANTIALIASING:
                this.antialiasing = (PVRFlag)((int)value);
                break;
            case VertexProps.FONT_PROP_WORDBREAK:
                this.wordbreak = (int)value;
                break;
            case VertexProps.TEXTSPRITE_PROP_BACKGROUND_ENABLED:
                this.background_enabled = value >= 1.0f;
                break;
            case VertexProps.TEXTSPRITE_PROP_BACKGROUND_SIZE:
                this.background_size = value;
                break;
            case VertexProps.TEXTSPRITE_PROP_BACKGROUND_OFFSET_X:
                this.background_offset_x = value;
                break;
            case VertexProps.TEXTSPRITE_PROP_BACKGROUND_OFFSET_Y:
                this.background_offset_y = value;
                break;
            case VertexProps.TEXTSPRITE_PROP_BACKGROUND_COLOR_R:
                this.background_rgba[0] = value;
                break;
            case VertexProps.TEXTSPRITE_PROP_BACKGROUND_COLOR_G:
                this.background_rgba[1] = value;
                break;
            case VertexProps.TEXTSPRITE_PROP_BACKGROUND_COLOR_B:
                this.background_rgba[2] = value;
                break;
            case VertexProps.TEXTSPRITE_PROP_BACKGROUND_COLOR_A:
                this.background_rgba[3] = value;
                break;
            case VertexProps.SPRITE_PROP_ALPHA2:
                this.alpha2 = value;
                break;
        }

        // check if the coordinates was modified
        switch (property_id) {
            case VertexProps.SPRITE_PROP_X:
            case VertexProps.SPRITE_PROP_Y:
            case VertexProps.SPRITE_PROP_WIDTH:
            case VertexProps.SPRITE_PROP_HEIGHT:
            case VertexProps.TEXTSPRITE_PROP_MAX_WIDTH:
            case VertexProps.TEXTSPRITE_PROP_MAX_HEIGHT:
            case VertexProps.TEXTSPRITE_PROP_PARAGRAPH_SEPARATION:
            case VertexProps.FONT_PROP_WORDBREAK:
            case VertexProps.TEXTSPRITE_PROP_FONT_SIZE:
                this.modified_coords = true;
                break;
        }

    }


    public void GetDrawSize(out float draw_width, out float draw_height) {
        CalculateParagraphAlignment();
        draw_width = this.last_draw_width;
        draw_height = this.last_draw_height;
    }

    public float GetFontSize() {
        return this.fontparams.height;
    }

    public void GetMaxDrawSize(out float draw_width_max, out float draw_height_max) {
        draw_width_max = this.max_width;
        draw_height_max = this.max_height;
    }

    public void Draw(PVRContext pvrctx) {
        if (this.fontparams.tint_color[3] <= 0f) return;
        if (StringUtils.IsEmpty(this.text_forced_case) && StringUtils.IsEmpty(this.text)) return;

        // check if all calculations are up-to-date
        if (this.modified_string || this.modified_coords) DrawCalc();

        DrawInternal(pvrctx);
    }


    private void DrawInternal(PVRContext pvrctx) {
        pvrctx.Save();
        pvrctx.SetGlobalAlpha(this.alpha2);
        if (this.psshader != null) pvrctx.AddShader(this.psshader);

        pvrctx.SetVertexBlend(
            this.blend_enabled,
            this.blend_src_rgb,
            this.blend_dst_rgb,
            this.blend_src_alpha,
            this.blend_dst_alpha
        );

        if (this.antialiasing != PVRFlag.DEFAULT) {
            pvrctx.SetGlobalAntialiasing(this.antialiasing);
        }

        MatrixCalculate(pvrctx);

        string text = this.text_forced_case ?? this.text;

        if (this.background_enabled) {
            float size = this.background_size * 2f;
            float x = this.last_draw_x - this.background_size + this.background_offset_x;
            float y = this.last_draw_y - this.background_size + this.background_offset_y;
            float width = this.last_draw_width + size;
            float height = this.last_draw_height + size;

            pvrctx.SetVertexAlpha(this.background_rgba[3]);
            pvrctx.DrawSolidColor(this.background_rgba, x, y, width, height);
        }

        if (this.paragraph_array.Size() < 2) {
            // let the font handle the draw
            this.font.DrawText(
                pvrctx,
                ref this.fontparams,
                this.last_draw_x, this.last_draw_y,
                0, text.Length, text
            );
        } else {
            // paragraph by paragraph draw
            float y = this.last_draw_y;
            float line_height_default = this.fontparams.height + this.fontparams.paragraph_space;

            foreach (ParagraphInfo paragraphinfo in this.paragraph_array) {
                float line_height;
                if (paragraphinfo.length > 0) {
                    line_height = this.font.DrawText(
                        pvrctx, ref this.fontparams,
                        this.last_draw_x + paragraphinfo.offset, y,
                        paragraphinfo.index, paragraphinfo.length, text
                    );
                } else {
                    line_height = line_height_default;
                }

                y += line_height;
            }
        }

        pvrctx.Restore();
    }

    private void DrawCalc() {
        // step 1: calculate the draw size (lines and the most wide paragraph)
        float draw_width = 0.0f, draw_height = 0.0f;
        GetDrawSize(out draw_width, out draw_height);

        // step 2: calculate the alignment
        float offset_x = 0f;
        float offset_y = 0f;
        if (this.max_height >= 0 && this.align_vertical != Align.START) {
            offset_y = this.max_height - draw_height;
            if (this.align_vertical == Align.CENTER) offset_y /= 2f;
        }
        if (this.max_width >= 0 && this.align_horizontal != Align.START) {
            offset_x = this.max_width - draw_width;
            if (this.align_horizontal == Align.CENTER) offset_x /= 2f;
        }

        // step 3: cache the drawing coordinates
        this.last_draw_x = this.x + offset_x;
        this.last_draw_y = this.y + offset_y;
        this.modified_coords = false;
        this.modified_string = false;
    }

    public AnimSprite AnimationSet(AnimSprite animsprite) {
        AnimSprite old_animation = this.animation_selected;
        this.animation_selected = animsprite;
        return old_animation;
    }

    public void AnimationRestart() {
        if (this.animation_selected != null) this.animation_selected.Restart();
    }

    public void AnimationEnd() {
        if (this.animation_selected == null) return;
        this.animation_selected.ForceEnd();
        this.animation_selected.UpdateTextsprite(this, true);
    }

    public void AnimationStop() {
        if (this.animation_selected != null) this.animation_selected.Stop();
    }


    public void AnimationExternalSet(AnimSprite animsprite) {
        this.animation_external = animsprite;
    }

    public void AnimationExternalEnd() {
        if (this.animation_external != null) {
            this.animation_external.ForceEnd();
            this.animation_external.UpdateTextsprite(this, true);
        }
    }

    public void AnimationExternalRestart() {
        if (this.animation_external != null) this.animation_external.Restart();
    }


    public int Animate(float elapsed) {
        int result = 1;

        if (this.animation_selected != null) {
            result = this.animation_selected.Animate(elapsed);
            this.animation_selected.UpdateTextsprite(this, true);
        }

        if (this.animation_external != null) {
            result = this.animation_external.Animate(elapsed);
            this.animation_external.UpdateTextsprite(this, false);
        }

        if (this.modified_string && this.modified_coords) DrawCalc();

        return result;
    }


    public void BorderEnable(bool enable) {
        this.fontparams.border_enable = enable;
    }

    public void BorderSetSize(float border_size) {
        this.fontparams.border_size = Single.IsNaN(border_size) ? 0.0f : border_size;
    }

    public void BorderSetColor(float r, float g, float b, float a) {
        if (r >= 0) this.fontparams.border_color[0] = r;
        if (g >= 0) this.fontparams.border_color[1] = g;
        if (b >= 0) this.fontparams.border_color[2] = b;
        if (a >= 0) this.fontparams.border_color[3] = a;
    }

    public void BorderSetColorRGBA8(uint rbga8_color) {
        Math2D.ColorBytesToFloats(rbga8_color, true, this.fontparams.border_color);
    }

    public void BorderSetOffset(float x, float y) {
        if (!Single.IsNaN(x)) this.fontparams.border_offset_x = x;
        if (!Single.IsNaN(y)) this.fontparams.border_offset_y = y;
    }


    public bool IsVisible() {
        return this.visible;
    }

    public void SetAntialiasing(PVRFlag antialiasing) {
        this.antialiasing = antialiasing;
    }

    public void SetWordbreak(int wordbreak) {
        this.wordbreak = wordbreak;
        this.modified_coords = true;
    }

    public bool HasFont() {
        return this.font != null;
    }

    public void ChangeFont(FontHolder fontholder) {
        if (fontholder == null) throw new ArgumentNullException("fontholder can not be null");
        this.font = fontholder.font;
        this.fontparams.color_by_addition = fontholder.font_color_by_addition;
    }

    public void SetShader(PSShader psshader) {
        this.psshader = psshader;
    }

    public PSShader GetShader() {
        return this.psshader;
    }

    public void BlendEnable(bool enabled) {
        this.blend_enabled = enabled;
    }

    public void BlendSet(Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha) {
        this.blend_src_rgb = src_rgb;
        this.blend_dst_rgb = dst_rgb;
        this.blend_src_alpha = src_alpha;
        this.blend_dst_alpha = dst_alpha;
    }


    public void BackgroundEnable(bool enabled) {
        this.background_enabled = !!enabled;
    }

    public void BackgroundSetSize(float size) {
        this.background_size = size;
    }

    public void BackgroundSetOffets(float offset_x, float offset_y) {
        if (!Single.IsNaN(offset_x)) this.background_offset_x = offset_x;
        if (!Single.IsNaN(offset_y)) this.background_offset_y = offset_y;
    }

    public void BackgroundSetColor(float r, float g, float b, float a) {
        if (!Single.IsNaN(r)) this.background_rgba[0] = r;
        if (!Single.IsNaN(g)) this.background_rgba[1] = g;
        if (!Single.IsNaN(b)) this.background_rgba[2] = b;
        if (!Single.IsNaN(a)) this.background_rgba[3] = a;
    }

    public string GetString() {
        return this.text_forced_case ?? this.text;
    }

    public void EnableColorByAddition(bool enabled) {
        this.fontparams.color_by_addition = enabled;
    }


    private class ParagraphInfo {
        public int length;
        public float offset;
        public int index;
    }

}
