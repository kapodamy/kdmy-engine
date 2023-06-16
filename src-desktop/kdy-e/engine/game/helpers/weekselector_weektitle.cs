using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game.Helpers;

public class WeekSelectorWeekTitle : IDraw {

    private const float ALT_ALPHA = 0.5f;


    private LayoutPlaceholder placeholder1;
    private LayoutPlaceholder placeholder2;
    private LayoutPlaceholder placeholder3;
    private Sprite sprite;
    private Drawable drawable;


    public WeekSelectorWeekTitle(Layout layout) {

        this.placeholder1 = layout.GetPlaceholder("ui_week_title_no_warns");
        this.placeholder2 = layout.GetPlaceholder("ui_week_title_warns");
        this.placeholder3 = layout.GetPlaceholder("ui_difficult_selected");
        this.sprite = Sprite.Init(null);
        this.drawable = null;

        this.drawable = new Drawable(-1f, this, null);
        this.sprite.SetAlpha(WeekSelectorWeekTitle.ALT_ALPHA);

    }

    public void Destroy() {
        this.sprite.Destroy();
        this.drawable.Destroy();
        //free(this);
    }

    public void Draw(PVRContext pvrctx) {
        if (!this.sprite.IsTextured()) return;
        pvrctx.Save();
        this.sprite.Draw(pvrctx);
        pvrctx.Restore();
    }

    public void Move(WeekSelectorWeekList weeklist, bool use_warns_placeholder) {
        float[] draw_size = { 0f, 0f };
        float[] draw_location = { 0f, 0f };
        AtlasEntry atlas_entry = new AtlasEntry() { };

        if (this.placeholder1 == null && this.placeholder2 == null) return;

        Sprite title_sprite = weeklist.PeekTitleSprite();
        title_sprite.FillAtlasEntry(atlas_entry);
        title_sprite.GetSourceSize(out draw_size[0], out draw_size[1]);

        LayoutPlaceholder old_placeholder = use_warns_placeholder ? this.placeholder1 : this.placeholder2;
        LayoutPlaceholder placeholder = use_warns_placeholder ? this.placeholder2 : this.placeholder1;

        if (old_placeholder != null) old_placeholder.vertex = null;
        if (placeholder == null) return;

        placeholder.vertex = this.drawable;
        this.drawable.HelperUpdateFromPlaceholder(placeholder);

        ImgUtils.CalcRectangle(
            placeholder.x, placeholder.y, placeholder.width, placeholder.height,
            draw_size[0], draw_size[1], Align.CENTER, Align.CENTER,
            draw_size, draw_location
        );

        this.sprite.SetTexture(title_sprite.GetTexture(), false);
        Atlas.ApplyFromEntry(this.sprite, atlas_entry, false);
        this.sprite.SetDrawSize(draw_size[0], draw_size[1]);
        this.sprite.SetDrawLocation(draw_location[0], draw_location[1]);
    }

    public void MoveDifficult(WeekSelectorDifficult weekdifficult) {
        float[] draw_size = { 0f, 0f };
        float[] draw_location = { 0f, 0f };

        if (this.placeholder1 != null) this.placeholder1.vertex = null;
        if (this.placeholder2 != null) this.placeholder2.vertex = null;
        if (this.placeholder3 != null) this.placeholder3.vertex = this.drawable;
        this.drawable.HelperUpdateFromPlaceholder(this.placeholder3);

        StateSprite difficult_statesprite = weekdifficult.PeekStateSprite();
        StateSpriteState statesprite_state = difficult_statesprite.StateGet();
        if (statesprite_state == null) {
            this.sprite.SetTexture(null, false);
            return;
        }
        ImgUtils.GetStateSpriteOriginalSize(statesprite_state, ref draw_size[0], ref draw_size[1]);

        ImgUtils.CalcRectangle(
            this.placeholder3.x, this.placeholder3.y,
            this.placeholder3.width, this.placeholder3.height,
            draw_size[0], draw_size[1],
            this.placeholder3.align_horizontal, this.placeholder3.align_vertical,
            draw_size, draw_location
        );

        this.sprite.SetTexture(statesprite_state.texture, false);
        Atlas.ApplyFromEntry(this.sprite, statesprite_state.frame_info, false);
        this.sprite.SetDrawSize(draw_size[0], draw_size[1]);
        this.sprite.SetDrawLocation(draw_location[0], draw_location[1]);
        this.sprite.SetAlpha(1.0f);
    }

}
