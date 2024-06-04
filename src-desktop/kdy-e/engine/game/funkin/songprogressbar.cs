using System;
using System.Text;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Font;
using Engine.Image;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;

namespace Engine.Game;

public class SongProgressbar : IAnimate, IDraw {

    private const string MODEL_BACKGROUND = "/assets/common/image/week-round/songprogressbar_background.xml";
    private const string MODEL_BAR = "/assets/common/image/week-round/songprogressbar_bar.xml";
    private const string NAME_BACKGROUND = "songprogressbar background";
    private const string NAME_BAR_BACK = "songprogressbar bar back";
    private const string NAME_BAR_FRONT = "songprogressbar bar front";


    private SongPlayer songplayer;
    private double duration;
    private Drawable drawable;
    private bool manual_update;
    private bool show_time;
    private bool show_time_elapsed;
    private AnimSprite drawable_animation;
    private double last_elapsed_seconds;
    private StringBuilder stringbuilder;
    private bool is_vertical;
    private float bar_length;
    private StateSprite statesprite_background;
    private StateSprite statesprite_back;
    private StateSprite statesprite_progress;
    private TextSprite textsprite_time;


    public SongProgressbar(float x, float y, float z, float width, float height, Align align, float border_size, bool is_vertical, bool show_time, FontHolder fontholder, float font_size, float font_border_size, uint color_rgba8_text, uint color_rgba8_background, uint color_rgba8_back, uint color_rgba8_progress) {

        this.songplayer = null;
        this.duration = Double.NaN;
        this.drawable = null;
        this.manual_update = false;
        this.show_time = show_time;
        this.show_time_elapsed = true;
        this.drawable_animation = null;
        this.last_elapsed_seconds = Double.NaN;
        this.stringbuilder = new StringBuilder(16);

        this.is_vertical = is_vertical;
        this.bar_length = is_vertical ? height : width;

        this.statesprite_background = StateSprite.InitFromVertexColor(color_rgba8_background >> 8);
        this.statesprite_back = StateSprite.InitFromVertexColor(color_rgba8_back >> 8);
        this.statesprite_progress = StateSprite.InitFromVertexColor(color_rgba8_progress >> 8);
        this.textsprite_time = TextSprite.Init2(fontholder, font_size, color_rgba8_text >> 8);


        this.drawable = new Drawable(
            z, this, this
        );

        // set alphas
        this.textsprite_time.SetAlpha((color_rgba8_text & 0xFF) / 255f);
        this.statesprite_background.SetAlpha((color_rgba8_background & 0xFF) / 255f);
        this.statesprite_back.SetAlpha((color_rgba8_back & 0xFF) / 255f);
        this.statesprite_progress.SetAlpha((color_rgba8_progress & 0xFF) / 255f);

        // guess border color (invert font color)
        float[] rgba = { 0.0f, 0.0f, 0.0f, 0.0f };
        Math2D.ColorBytesToFloats(color_rgba8_text, true, rgba);
        this.textsprite_time.BorderSetColor(1.0f - rgba[0], 1.0f - rgba[1], 1.0f - rgba[2], rgba[3]);
        this.textsprite_time.BorderSetSize(font_border_size);
        this.textsprite_time.BorderEnable(true);

        // background location&size
        float background_x = x - border_size;
        float background_y = y - border_size;
        float background_width = width + (border_size * 2);
        float background_height = height + (border_size * 2);
        this.statesprite_background.SetDrawLocation(background_x, background_y);
        this.statesprite_background.SetDrawSize(background_width, background_height);

        // bar back+progress location&size
        this.statesprite_back.SetDrawLocation(x, y);
        this.statesprite_back.SetDrawSize(width, height);
        this.statesprite_progress.SetDrawLocation(x, y);
        this.statesprite_progress.SetDrawSize(width, height);


        // set textsprite bounds
        if (is_vertical) {
            this.textsprite_time.SetParagraphAlign(align);
            this.textsprite_time.SetAlign(align, Align.CENTER);
        } else {
            this.textsprite_time.SetParagraphAlign(align);
            this.textsprite_time.SetAlign(Align.CENTER, align);
        }
        this.textsprite_time.SetDrawLocation(x - border_size, y - border_size);
        border_size *= 2f;
        this.textsprite_time.SetMaxDrawSize(width + border_size, height + border_size);

        this.statesprite_progress.CropEnable(true);

        //
        // import models (if exists)
        //
        InternalCalcState(
            this.statesprite_background, SongProgressbar.MODEL_BACKGROUND,
            SongProgressbar.NAME_BACKGROUND, background_width, background_height
        );
        InternalCalcState(
            this.statesprite_back, SongProgressbar.MODEL_BAR,
            SongProgressbar.NAME_BAR_BACK, width, height
        );
        InternalCalcState(
            this.statesprite_back, SongProgressbar.MODEL_BAR,
            SongProgressbar.NAME_BAR_FRONT, width, height
        );

    }

    public void Destroy() {
        this.statesprite_background.Destroy();
        this.statesprite_back.Destroy();
        this.statesprite_progress.Destroy();
        this.textsprite_time.Destroy();

        if (this.drawable_animation != null) this.drawable_animation.Destroy();
        this.drawable.Destroy();
        //this.stringbuilder.Destroy();

        Luascript.DropShared(this);
        //free(this);
    }


    public void SetSongplayer(SongPlayer songplayer) {
        this.songplayer = songplayer;
    }

    public void SetDuration(double duration) {
        this.duration = duration;
    }

    public Drawable GetDrawable() {
        return this.drawable;
    }

    public void SetVisible(bool visible) {
        this.drawable.SetVisible(visible);
    }

    public void SetBackgroundColor(float r, float g, float b, float a) {
        this.statesprite_background.SetVertexColor(r, g, b);
        if (!Double.IsNaN(a)) this.statesprite_background.SetAlpha(a);
    }

    public void SetBarBackColor(float r, float g, float b, float a) {
        this.statesprite_back.SetVertexColor(r, g, b);
        if (!Double.IsNaN(a)) this.statesprite_back.SetAlpha(a);
    }

    public void SetBarProgressColor(float r, float g, float b, float a) {
        this.statesprite_progress.SetVertexColor(r, g, b);
        if (!Double.IsNaN(a)) this.statesprite_progress.SetAlpha(a);
    }

    public void SetTextColor(float r, float g, float b, float a) {
        this.textsprite_time.SetColor(r, g, b);
        if (!Double.IsNaN(a)) this.textsprite_time.SetAlpha(a);
    }

    public void HideTime(bool hidden) {
        this.show_time = !hidden;
    }

    public void ShowElapsed(bool elapsed_or_remain_time) {
        this.show_time_elapsed = !!elapsed_or_remain_time;
    }

    public void ManualUpdateEnable(bool enabled) {
        this.manual_update = !!enabled;
        this.last_elapsed_seconds = Double.NaN;
    }

    public void ManualSetText(string text) {
        this.last_elapsed_seconds = Double.NaN;// force update
        this.textsprite_time.SetText(text);
    }

    public float ManualSetPosition(double elapsed, double duration, bool should_update_time_text) {
        double percent = elapsed / duration;
        if (Double.IsNaN(percent)) percent = 0.0;

        if (should_update_time_text) {
            double elapsed_seconds = Math.Truncate(elapsed / 1000.0);
            if (elapsed_seconds != this.last_elapsed_seconds) {
                this.last_elapsed_seconds = elapsed_seconds;

                if (!this.show_time_elapsed) elapsed_seconds = (duration / 1000.0) - elapsed_seconds;
                elapsed_seconds = Math2D.Clamp(elapsed_seconds, 0.0, duration);

                // compute text
                InternalSecondsToString(this.stringbuilder, elapsed_seconds);
                string str = this.stringbuilder.InternKDY();

                this.textsprite_time.SetTextIntern(true, str);
            }
        }

        // calculate bar cropping
        float length = (float)(this.bar_length * Math2D.Clamp(percent, 0.0, 1.0));
        float crop_width = -1.0f, crop_height = -1.0f;

        if (this.is_vertical)
            crop_height = length;
        else
            crop_width = length;

        this.statesprite_progress.Crop(0.0f, 0.0f, crop_width, crop_height);

        return (float)percent;
    }

    public void AnimationSet(AnimSprite animsprite) {
        if (this.drawable_animation != null) this.drawable_animation.Destroy();
        this.drawable_animation = animsprite != null ? animsprite.Clone() : null;
    }

    public void AnimationRestart() {
        if (this.drawable_animation != null) this.drawable_animation.Restart();
        this.statesprite_background.AnimationRestart();
        this.statesprite_back.AnimationRestart();
        this.statesprite_progress.AnimationRestart();
        this.textsprite_time.AnimationRestart();
    }

    public void AnimationEnd() {
        if (this.drawable_animation != null) {
            this.drawable_animation.ForceEnd();
            this.drawable_animation.UpdateDrawable(this.drawable, true);
        }
        this.statesprite_background.AnimationEnd();
        this.statesprite_back.AnimationEnd();
        this.statesprite_progress.AnimationEnd();
        this.textsprite_time.AnimationEnd();
    }


    public int Animate(float elapsed) {
        int completed = 0;

        if (this.drawable_animation != null) {
            completed += this.drawable_animation.Animate(elapsed);
            if (completed < 1) this.drawable_animation.UpdateDrawable(this.drawable, true);
        }

        completed += this.statesprite_background.Animate(elapsed);
        completed += this.statesprite_back.Animate(elapsed);
        completed += this.statesprite_progress.Animate(elapsed);
        completed += this.textsprite_time.Animate(elapsed);

        if (!this.manual_update) {
            double current = 0, duration = 0;

            if (this.songplayer != null) {
                current = this.songplayer.GetTimestamp();
                duration = this.duration >= 0.0 ? this.duration : this.songplayer.GetDuration();
            } else {
                current = BeatWatcher.beatwatcher_global_timestamp;// peek global beatwatcher time
                duration = this.duration;
            }
            ManualSetPosition(current, duration, true);
        }

        return completed;
    }

    public void Draw(PVRContext pvrctx) {
        pvrctx.Save();
        this.drawable.HelperApplyInContext(pvrctx);

        this.statesprite_background.Draw(pvrctx);
        this.statesprite_back.Draw(pvrctx);
        this.statesprite_progress.Draw(pvrctx);
        if (this.show_time) this.textsprite_time.Draw(pvrctx);

        pvrctx.Restore();
    }


    private void InternalCalcState(StateSprite statesprite, string modelholder_src, string name, float width, float height) {
        if (!FS.FileExists(modelholder_src)) return;

        ModelHolder modelholder = ModelHolder.Init(modelholder_src);
        if (modelholder == null) return;
        StateSpriteState statesprite_state = statesprite.StateAdd(modelholder, name, null);
        modelholder.Destroy();
        if (statesprite_state == null) return;

        ImgUtils.CalcRectangleInStateSpriteState(0, 0, width, height, Align.CENTER, Align.CENTER, statesprite_state);
        statesprite.StateToggle(null);
    }

    private static void InternalSecondsToString(StringBuilder stringbuilder, double seconds) {
        const string TIME_SECONDS = "$is";
        const string TIME_MINUTES = "$im";
        const string TIME_HOURS = "$ih";

        if (Double.IsNaN(seconds)) {
            stringbuilder.Clear();
            stringbuilder.AddKDY("--m--s");
            return;
        }

        double h = Math.Floor(seconds / 3600.0);
        double m = Math.Floor((seconds - (h * 3600.0)) / 60.0);
        double s = seconds - (h * 3600.0) - (m * 60.0);

        stringbuilder.Clear();

        //
        // The 0x30 codepoint means the number zero "0" character
        //

        if (h > 0.0) {
            if (h < 10) stringbuilder.AddCharCodepointKDY(0x30);
            stringbuilder.AddFormatKDY(TIME_HOURS, (int)h);
        }

        if (m < 10) stringbuilder.AddCharCodepointKDY(0x30);
        stringbuilder.AddFormatKDY(TIME_MINUTES, (int)m);

        if (s < 10) stringbuilder.AddCharCodepointKDY(0x30);
        stringbuilder.AddFormatKDY(TIME_SECONDS, (int)s);

    }


}
