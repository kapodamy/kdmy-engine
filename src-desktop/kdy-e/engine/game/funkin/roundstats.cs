using System;
using System.Text;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Font;
using Engine.Game.Common;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game;

public class RoundStats : IDraw, IAnimate {

    private const uint FONT_COLOR = 0xFFFFFF;// white
    private const uint FONT_BORDER_COLOR = 0x000000FF;// black
    public const float FONT_BORDER_SIZE = 2f;
    private const string SEPARATOR = " | ";


    private TextSprite textsprite;
    private double next_clear;
    private Drawable drawable;
    private AnimSprite drawable_animation;
    private StringBuilder builder;
    private BeatWatcher beatwatcher;
    private TweenKeyframeInfo tweenkeyframe_active;
    private float tweenkeyframe_multiplier;
    private int last_count_hit;
    private int last_count_miss;
    private TweenKeyframeInfo tweenkeyframe_beat;
    private TweenKeyframeInfo tweenkeyframe_hit;
    private TweenKeyframeInfo tweenkeyframe_miss;
    private bool enable_nps;


    public RoundStats(float x, float y, float z, FontHolder fontholder, float font_size, float layout_width) {

        this.textsprite = TextSprite.Init2(fontholder, font_size, RoundStats.FONT_COLOR);
        this.next_clear = 0.0;
        this.drawable = null;
        this.drawable_animation = null;
        this.builder = new StringBuilder(128);

        this.beatwatcher = new BeatWatcher() { };

        this.tweenkeyframe_active = null;// this can be "tweenkeyframe_hit" or "tweenkeyframe_miss"
        this.tweenkeyframe_multiplier = 1.0f;

        this.last_count_hit = 0;
        this.last_count_miss = 0;
        this.enable_nps = false;

        this.tweenkeyframe_beat = new TweenKeyframeInfo() { };
        this.tweenkeyframe_hit = new TweenKeyframeInfo() { };
        this.tweenkeyframe_miss = new TweenKeyframeInfo() { };


        this.beatwatcher.Reset(true, 100.0f);

        this.drawable = new Drawable(z, this, this);

        Modifier modifier = this.drawable.GetModifier();
        modifier.x = x;
        modifier.y = y;

        this.textsprite.BorderEnable(true);
        this.textsprite.BorderSetColorRGBA8(RoundStats.FONT_BORDER_COLOR);
        this.textsprite.BorderSetSize(RoundStats.FONT_BORDER_SIZE);
        this.textsprite.SetVisible(false);
        this.textsprite.SetDrawLocation(x, y);
        this.textsprite.SetMaxDrawSize(layout_width, -1);
        this.textsprite.SetAlign(Align.START, Align.CENTER);

        Reset();

    }

    public void Destroy() {
        Luascript.DropShared(this);

        this.drawable.Destroy();
        if (this.drawable_animation != null) this.drawable_animation.Destroy();
        this.textsprite.Destroy();
        //this.builder.Destroy();
        //free(this);
    }


    public void Hide(bool hide) {
        this.drawable.SetVisible(!hide);
    }

    public void HideNps(bool hide) {
        this.enable_nps = hide;
    }

    public void SetDrawY(float y) {
        Modifier modifier = this.drawable.GetModifier();
        modifier.y = y;
        this.textsprite.SetDrawLocation(Single.NaN, y);
    }

    public void Reset() {
        this.next_clear = 1000.0;
        this.tweenkeyframe_active = null;
        this.tweenkeyframe_multiplier = 1.0f;
        this.last_count_hit = 0;
        this.last_count_miss = 0;

        this.beatwatcher.Reset(true, 100);

        this.drawable.SetAntialiasing(PVRFlag.DEFAULT);

        RoundStats.InternalTweenkeyframeSetup(this.tweenkeyframe_beat, false);
        RoundStats.InternalTweenkeyframeSetup(this.tweenkeyframe_hit, false);
        RoundStats.InternalTweenkeyframeSetup(this.tweenkeyframe_miss, false);
    }

    public Drawable GetDrawable() {
        return this.drawable;
    }

    public void PeekPlayerstats(double song_timestamp, PlayerStats playerstats) {
        StringBuilder builder = this.builder;
        double accuracy = playerstats.GetAccuracy();

        builder.Clear();

        if (this.enable_nps) {
            int nps = playerstats.GetNotesPerSeconds();
            int nps_max = playerstats.GetNotesPerSecondsHighest();
            builder.AddFormatKDY("NPS: $i (Max $i)", nps, nps_max);
            builder.AddKDY(RoundStats.SEPARATOR);
        }

        if (this.next_clear > song_timestamp) {
            this.next_clear += 1000.0;
            playerstats.ResetNotesPerSeconds();
        }

        builder.AddFormatKDY("Score: $l", playerstats.GetScore());
        builder.AddKDY(RoundStats.SEPARATOR);

        builder.AddFormatKDY("CB: $i", playerstats.GetComboBreaks());
        builder.AddKDY(RoundStats.SEPARATOR);

        builder.AddFormatKDY("Accuracy: $2d%", accuracy);
        builder.AddKDY(RoundStats.SEPARATOR);

        if (accuracy > 0) {
            builder.AddFormatKDY(
               " $s $s",
               Funkin.GetLetterRank(playerstats),
               Funkin.GetWife3Accuracy(playerstats)
           );
        } else {
            builder.AddKDY(Funkin.NO_ACCURACY);
        }

        //stringbuilder_add(builder, RoundStats.SEPARATOR);

        // now use the builded stats string
        this.textsprite.SetTextIntern(true, builder.InternKDY());

        //
        // configure the active tweenkeyframe
        //
        int miss_count = playerstats.GetMisses();
        int hit_count = playerstats.GetHits();
        TweenKeyframeInfo tweenkeyframe_active = null;
        float tweenkeyframe_multiplier = 1.0f;

        if (miss_count > this.last_count_miss) {
            tweenkeyframe_active = this.tweenkeyframe_miss;
            tweenkeyframe_multiplier = this.last_count_miss - miss_count;
        } else if (hit_count > this.last_count_hit) {
            tweenkeyframe_active = this.tweenkeyframe_hit;
            tweenkeyframe_multiplier = this.last_count_hit - hit_count;
        } else {
            return;
        }

        this.last_count_miss = miss_count;
        this.last_count_hit = hit_count;

        if (tweenkeyframe_active != this.tweenkeyframe_active) {
            this.tweenkeyframe_active = tweenkeyframe_active;
            this.tweenkeyframe_multiplier = tweenkeyframe_multiplier;
        } else {
            this.tweenkeyframe_multiplier += tweenkeyframe_multiplier;
        }
        RoundStats.InternalTweenkeyframeSetup(tweenkeyframe_active, false);

    }


    public void TweenkeyframeSetOnBeat(TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration) {
        RoundStats.InternalTweenkeyframeSet(this.tweenkeyframe_beat, tweenkeyframe, rollback_beats, beat_duration);
        InternalTweenkeyframeDuration(this.tweenkeyframe_beat, this.beatwatcher.tick);
    }

    public void TweenkeyframeSetOnHit(TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration) {
        RoundStats.InternalTweenkeyframeSet(this.tweenkeyframe_hit, tweenkeyframe, rollback_beats, beat_duration);
        InternalTweenkeyframeDuration(this.tweenkeyframe_hit, this.beatwatcher.tick);
    }

    public void TweenkeyframeSetOnMiss(TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration) {
        RoundStats.InternalTweenkeyframeSet(this.tweenkeyframe_miss, tweenkeyframe, rollback_beats, beat_duration);
        InternalTweenkeyframeDuration(this.tweenkeyframe_miss, this.beatwatcher.tick);
    }

    public void TweenkeyframeSetBpm(float beats_per_minute) {
        this.beatwatcher.ChangeBpm(beats_per_minute);

        InternalTweenkeyframeDuration(this.tweenkeyframe_beat, this.beatwatcher.tick);
        InternalTweenkeyframeDuration(this.tweenkeyframe_hit, this.beatwatcher.tick);
        InternalTweenkeyframeDuration(this.tweenkeyframe_miss, this.beatwatcher.tick);
    }


    public int Animate(float elapsed) {
        if (this.drawable_animation != null) this.drawable_animation.Animate(elapsed);

        bool completed = true;
        bool ignore_beat = this.tweenkeyframe_beat.tweenkeyframe == null;// ignore if there no tweenkeyframe for beats
        bool beat_rollback_active = this.tweenkeyframe_beat.rollback_active;

        if (this.tweenkeyframe_active != null) {
            completed = InternalTweenkeyframeRun(this.tweenkeyframe_active, elapsed);

            // if the active tweenkeyframe is completed, do rollback
            if (!this.tweenkeyframe_active.rollback_active) {
                if (completed) {
                    RoundStats.InternalTweenkeyframeSetup(this.tweenkeyframe_active, true);
                } else if (!ignore_beat && !beat_rollback_active) {
                    // still running, ignore beat tweernlerp
                    beat_rollback_active = true;
                    this.tweenkeyframe_beat.rollback_active = true;
                }
            }
        }

        // beat check
        if (this.beatwatcher.Poll()) {
            elapsed += this.beatwatcher.since;
            RoundStats.InternalTweenkeyframeSetup(this.tweenkeyframe_beat, false);// run again
        }

        if (!ignore_beat) {
            completed = InternalTweenkeyframeRun(this.tweenkeyframe_beat, elapsed);
            if (completed && !beat_rollback_active)
                RoundStats.InternalTweenkeyframeSetup(this.tweenkeyframe_beat, true);// do rollback
        }

        float draw_width, draw_height;
        Modifier modifier = this.drawable.GetModifier();
        this.textsprite.GetDrawSize(out draw_width, out draw_height);
        modifier.width = draw_width;
        modifier.height = draw_height;

        return completed ? 1 : 0;
    }

    public void Draw(PVRContext pvrctx) {
        pvrctx.Save();
        this.drawable.HelperApplyInContext(pvrctx);
        this.textsprite.Draw(pvrctx);
        pvrctx.Restore();
    }



    private static void InternalTweenkeyframeSet(TweenKeyframeInfo tweenkeyframe_info, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration) {
        if (tweenkeyframe_info.tweenkeyframe != null) tweenkeyframe_info.tweenkeyframe.Destroy();
        tweenkeyframe_info.tweenkeyframe = tweenkeyframe.Clone();
        tweenkeyframe_info.rollback_beats = rollback_beats;
        tweenkeyframe_info.beat_duration = beat_duration;
    }

    private static void InternalTweenkeyframeDuration(TweenKeyframeInfo tweenkeyframe_info, float beat_duration) {
        tweenkeyframe_info.rollback_duration = tweenkeyframe_info.rollback_beats * beat_duration;
        tweenkeyframe_info.beat_duration = beat_duration * tweenkeyframe_info.beat_duration;
        RoundStats.InternalTweenkeyframeSetup(tweenkeyframe_info, tweenkeyframe_info.rollback_active);
    }

    private bool InternalTweenkeyframeRun(TweenKeyframeInfo tweenkeyframe_info, float elapsed) {
        int entry_id; float entry_value;

        if (tweenkeyframe_info.tweenkeyframe == null || tweenkeyframe_info.is_completed) return true;

        tweenkeyframe_info.target_elapsed += elapsed;

        float percent = tweenkeyframe_info.target_elapsed / tweenkeyframe_info.target_duration;
        tweenkeyframe_info.is_completed = percent >= 1f;

        if (percent > 1f) percent = 1f;

        if (tweenkeyframe_info.rollback_active) percent = 1f - percent;

        tweenkeyframe_info.tweenkeyframe.AnimatePercent(percent);

        int count = tweenkeyframe_info.tweenkeyframe.GetIdsCount();

        for (int i = 0 ; i < count ; i++) {
            tweenkeyframe_info.tweenkeyframe.PeekEntryByIndex(i, out entry_id, out entry_value);
            if (entry_id == VertexProps.TEXTSPRITE_PROP_STRING) continue;// illegal property

            if (!VertexProps.IsPropertyEnumerable(entry_id))
                entry_value *= this.tweenkeyframe_multiplier;

            this.drawable.SetProperty(entry_id, entry_value);
        }

        return tweenkeyframe_info.is_completed;
    }

    private static bool InternalTweenkeyframeSetup(TweenKeyframeInfo tweenkeyframe_info, bool rollback) {
        tweenkeyframe_info.is_completed = false;
        tweenkeyframe_info.rollback_active = rollback;
        tweenkeyframe_info.target_elapsed = 0f;
        tweenkeyframe_info.target_duration = rollback ? tweenkeyframe_info.rollback_duration : tweenkeyframe_info.beat_duration;
        return tweenkeyframe_info.tweenkeyframe == null;
    }


    private class TweenKeyframeInfo {
        public TweenKeyframe tweenkeyframe;
        public float rollback_beats;
        public float rollback_duration;
        public float beat_duration;
        public bool rollback_active;
        public float target_elapsed;
        public float target_duration;
        public bool is_completed;
    }


}
