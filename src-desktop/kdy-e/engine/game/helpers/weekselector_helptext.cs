using Engine.Font;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game.Helpers;

public class WeekSelectorHelpText : IDraw {

    private const uint TEXTCOLOR = 0xFFFFFF;// white
    private const float Z_INDEX = 210f;// draw on top of anything
    private const float ROW_PADDING_RATIO = 0.5f;


    private Sprite icon;
    private string string_1;
    private string string_2;
    private TextSprite textsprite;
    private float max_width;
    private float icon_width;
    private float icon_padding;
    private bool start;
    private float offset_x;
    private Drawable drawable;


    public WeekSelectorHelpText(ModelHolder modelholder, Layout layout, int row, bool start, string icon_name, string string1, string string2) {
        float font_size = layout.GetAttachedValueAsFloat("help_text_font_size", 12f);
        string font_name = (string)layout.GetAttachedValue("help_text_font_name", AttachedValueType.STRING, null);

        float padding_x = layout.GetAttachedValueAsFloat("help_text_padding_x", 0);
        float padding_y = layout.GetAttachedValueAsFloat("help_text_padding_y", 0);

        FontHolder layout_font = layout.GetAttachedFont(font_name);

        float size_width, size_height;
        layout.GetViewportSize(out size_width, out size_height);

        float max_width = size_width / 2.0f;


        this.icon = null;
        this.string_1 = string1;
        this.string_2 = string2;
        this.textsprite = null;
        this.max_width = max_width - padding_x;
        this.icon_width = 0;
        this.icon_padding = font_size / 2.0f;
        this.start = start;
        this.offset_x = 0;
        this.drawable = null;


        this.drawable = new Drawable(
            WeekSelectorHelpText.Z_INDEX, this, null
        );

        if (layout_font == null) return;

        float row_padding_x = padding_x * WeekSelectorHelpText.ROW_PADDING_RATIO;
        float draw_x = padding_x * (start ? 2.0f : 1.0f);
        float draw_y = size_height - ((font_size + row_padding_x) * row) - padding_y + row_padding_x;

        if (!start) draw_x += this.max_width;

        if (modelholder != null) {
            this.icon = Sprite.Init(modelholder.GetTexture(true));

            AtlasEntry atlas_entry = modelholder.GetAtlasEntry(icon_name);
            if (atlas_entry != null) Atlas.ApplyFromEntry(this.icon, atlas_entry, true);

            this.icon.SetVisible(false);
            this.icon.ResizeDrawSize(-1f, font_size, out size_width, out size_height);
            this.icon.SetDrawLocation(draw_x, draw_y);
            this.icon_width = size_width;
        }

        this.textsprite = TextSprite.Init2(layout_font, font_size, WeekSelectorHelpText.TEXTCOLOR);
        this.textsprite.SetVisible(false);

        if (start) {
            if (this.icon != null) draw_x += this.icon_width + this.icon_padding;
            this.textsprite.SetDrawLocation(draw_x, draw_y);
        } else {
            this.textsprite.SetAlignHorizontal(Align.END);
            this.textsprite.SetMaxDrawSize(this.max_width, -1);
            this.textsprite.SetDrawLocation(draw_x, draw_y);
        }

        UseAlt(false);
    }

    public void Destroy() {
        if (this.icon != null) this.icon.DestroyFull();
        if (this.textsprite != null) this.textsprite.Destroy();
        this.drawable.Destroy();
        //free(this);
    }

    public void SetVisible(bool visible) {
        this.drawable.SetVisible(visible);
    }

    public void UseAlt(bool use_alt) {
        // intern strings, they are static
        this.textsprite.SetTextIntern(true, use_alt ? this.string_2 : this.string_1);
        if (this.icon == null || this.start) return;
        float draw_width, draw_height;
        this.textsprite.GetDrawSize(out draw_width, out draw_height);
        this.offset_x = this.max_width - draw_width - this.icon_width - this.icon_padding;
    }

    public Drawable GetDrawable() {
        return this.drawable;
    }

    public void Draw(PVRContext pvrctx) {
        pvrctx.Save();
        this.textsprite.Draw(pvrctx);
        if (this.icon != null) {
            pvrctx.CurrentMatrix.TranslateX(this.offset_x);
            //pvrctx.Flush();
            this.icon.Draw(pvrctx);
        }
        pvrctx.Restore();
    }

}
