using Engine.Platform;

namespace Engine.Game.Gameplay.Helpers;

public class AutoUICosmetics : IDraw, IAnimate {

    public static readonly LayoutPlaceholder PLACEHOLDER_STREAK = new LayoutPlaceholder() {
        name = null,
        type = PVRContextVertex.DRAWABLE,
        vertex = null,
        width = 0f,
        height = 80f
    };
    public static readonly LayoutPlaceholder PLACEHOLDER_RANK = new LayoutPlaceholder() {
        name = null,
        type = PVRContextVertex.DRAWABLE,
        vertex = null,
        width = -1f,
        height = 110f
    };
    public static readonly LayoutPlaceholder PLACEHOLDER_ACCURACY = new LayoutPlaceholder() {
        name = null,
        type = PVRContextVertex.DRAWABLE,
        vertex = null,
        width = -1f,
        height = 32f
    };


    public Drawable drawable_self;
    public int layout_group_id;
    private Drawable drawable_streak;
    private Drawable drawable_rank;
    private Drawable drawable_accuracy;


    public AutoUICosmetics() {
        this.drawable_self = new Drawable(-1, this, this);
        this.layout_group_id = 0;
        this.drawable_streak = null;
        this.drawable_rank = null;
        this.drawable_accuracy = null;
    }

    public void Destroy() {
        this.drawable_self.Destroy();

        this.drawable_streak = null;
        this.drawable_rank = null;
        this.drawable_accuracy = null;

        AutoUICosmetics.PLACEHOLDER_STREAK.vertex = null;
        AutoUICosmetics.PLACEHOLDER_RANK.vertex = null;
        AutoUICosmetics.PLACEHOLDER_ACCURACY.vertex = null;

        //free(this);
    }


    public bool PreparePlaceholders(Layout layout) {
        this.drawable_streak = null;
        this.drawable_rank = null;
        this.drawable_accuracy = null;

        LayoutPlaceholder ui_autoplace = layout.GetPlaceholder("ui_autoplace_cosmetics");
        if (ui_autoplace == null) {
            this.layout_group_id = -1;
            return false;
        }

        this.drawable_self.HelperUpdateFromPlaceholder(ui_autoplace);
        this.layout_group_id = ui_autoplace.group_id;

        //
        // prepare stubs
        //
        AutoUICosmetics.PLACEHOLDER_STREAK.z = ui_autoplace.z + 0.1f;
        AutoUICosmetics.PLACEHOLDER_RANK.z = ui_autoplace.z + 0.2f;
        AutoUICosmetics.PLACEHOLDER_ACCURACY.z = ui_autoplace.z + 0.3f;

        AutoUICosmetics.PLACEHOLDER_STREAK.group_id = ui_autoplace.group_id;
        AutoUICosmetics.PLACEHOLDER_RANK.group_id = ui_autoplace.group_id;
        AutoUICosmetics.PLACEHOLDER_ACCURACY.group_id = ui_autoplace.group_id;

        AutoUICosmetics.PLACEHOLDER_STREAK.x = ui_autoplace.x + ui_autoplace.width / 4f;
        AutoUICosmetics.PLACEHOLDER_STREAK.y = ui_autoplace.y + ui_autoplace.height / 2f;
        AutoUICosmetics.PLACEHOLDER_RANK.x = ui_autoplace.x;
        AutoUICosmetics.PLACEHOLDER_RANK.y = ui_autoplace.y;
        AutoUICosmetics.PLACEHOLDER_ACCURACY.x = ui_autoplace.x + ui_autoplace.width;
        AutoUICosmetics.PLACEHOLDER_ACCURACY.y = ui_autoplace.y + ui_autoplace.height;

        return true;
    }

    public void PickDrawables() {
        this.drawable_streak = (Drawable)AutoUICosmetics.PLACEHOLDER_STREAK.vertex;
        AutoUICosmetics.PLACEHOLDER_STREAK.vertex = null;

        this.drawable_rank = (Drawable)AutoUICosmetics.PLACEHOLDER_RANK.vertex;
        AutoUICosmetics.PLACEHOLDER_RANK.vertex = null;

        this.drawable_accuracy = (Drawable)AutoUICosmetics.PLACEHOLDER_ACCURACY.vertex;
        AutoUICosmetics.PLACEHOLDER_ACCURACY.vertex = null;
    }

    public void Draw(PVRContext pvrctx) {
        if (this.drawable_streak != null) this.drawable_streak.Draw(pvrctx);
        if (this.drawable_rank != null) this.drawable_rank.Draw(pvrctx);
        if (this.drawable_accuracy != null) this.drawable_accuracy.Draw(pvrctx);
    }

    public int Animate(float elapsed) {
        int completed = 0;
        if (this.drawable_streak != null) completed += this.drawable_streak.Animate(elapsed);
        if (this.drawable_rank != null) completed += this.drawable_rank.Animate(elapsed);
        if (this.drawable_accuracy != null) completed += this.drawable_accuracy.Animate(elapsed);
        return completed;
    }

}
