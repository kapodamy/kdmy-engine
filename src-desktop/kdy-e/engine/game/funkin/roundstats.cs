using System;
using System.Text;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Font;
using Engine.Game.Common;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game {

    public class RoundStats : IDraw, IAnimate {

        private const uint FONT_COLOR = 0xFFFFFF;// white
        private const uint FONT_BORDER_COLOR = 0x000000FF;// black
        private const uint FONT_BORDER_SIZE = 1;// black
        private const string SEPARATOR = " | ";


        private TextSprite textsprite;
        private double next_clear;
        private Drawable drawable;
        private AnimSprite drawable_animation;
        private StringBuilder builder;
        private BeatWatcher beatwatcher;
        private TweenLerpInfo tweenlerp_active;
        private float tweenlerp_multiplier;
        private int last_count_hit;
        private int last_count_miss;
        private TweenLerpInfo tweenlerp_beat;
        private TweenLerpInfo tweenlerp_hit;
        private TweenLerpInfo tweenlerp_miss;
        private bool enable_nps;


        public RoundStats(float x, float y, float z, FontHolder fontholder, float font_size, float layout_width) {

            this.textsprite = TextSprite.Init2(fontholder, font_size, RoundStats.FONT_COLOR);
            this.next_clear = 0.0;
            this.drawable = null;
            this.drawable_animation = null;
            this.builder = new StringBuilder(128);

            this.beatwatcher = new BeatWatcher() { };

            this.tweenlerp_active = null;// this can be "tweenlerp_hit" or "tweenlerp_miss"
            this.tweenlerp_multiplier = 1.0f;

            this.last_count_hit = 0;
            this.last_count_miss = 0;
            this.enable_nps = false;

            this.tweenlerp_beat = new TweenLerpInfo() { };
            this.tweenlerp_hit = new TweenLerpInfo() { };
            this.tweenlerp_miss = new TweenLerpInfo() { };


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
            this.tweenlerp_active = null;
            this.tweenlerp_multiplier = 1.0f;
            this.last_count_hit = 0;
            this.last_count_miss = 0;

            this.beatwatcher.Reset(true, 100);

            this.drawable.SetAntialiasing(PVRContextFlag.DEFAULT);

            RoundStats.InternalTweenlerpSetup(this.tweenlerp_beat, false);
            RoundStats.InternalTweenlerpSetup(this.tweenlerp_hit, false);
            RoundStats.InternalTweenlerpSetup(this.tweenlerp_miss, false);
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
            // configure the active tweenlerp
            //
            int miss_count = playerstats.GetMisses();
            int hit_count = playerstats.GetHits();
            TweenLerpInfo tweenlerp_active = null;
            float tweenlerp_multiplier = 1.0f;

            if (miss_count > this.last_count_miss) {
                tweenlerp_active = this.tweenlerp_miss;
                tweenlerp_multiplier = this.last_count_miss - miss_count;
            } else if (hit_count > this.last_count_hit) {
                tweenlerp_active = this.tweenlerp_hit;
                tweenlerp_multiplier = this.last_count_hit - hit_count;
            } else {
                return;
            }

            this.last_count_miss = miss_count;
            this.last_count_hit = hit_count;

            if (tweenlerp_active != this.tweenlerp_active) {
                this.tweenlerp_active = tweenlerp_active;
                this.tweenlerp_multiplier = tweenlerp_multiplier;
                RoundStats.InternalTweenlerpSetup(tweenlerp_active, false);
            } else {
                this.tweenlerp_multiplier += tweenlerp_multiplier;
                if (this.tweenlerp_active.tweenlerp != null) this.tweenlerp_active.tweenlerp.Restart();
            }
        }


        public void TweenlerpSetOnBeat(TweenLerp tweenlerp, float rollback_beats, float beat_duration) {
            RoundStats.InternalTweenlerpSet(this.tweenlerp_beat, tweenlerp, rollback_beats, beat_duration);
            InternalTweenlerpDuration(this.tweenlerp_beat, this.beatwatcher.tick);
        }

        public void TweenlerpSetOnHit(TweenLerp tweenlerp, float rollback_beats, float beat_duration) {
            RoundStats.InternalTweenlerpSet(this.tweenlerp_hit, tweenlerp, rollback_beats, beat_duration);
            InternalTweenlerpDuration(this.tweenlerp_hit, this.beatwatcher.tick);
        }

        public void TweenlerpSetOnMiss(TweenLerp tweenlerp, float rollback_beats, float beat_duration) {
            RoundStats.InternalTweenlerpSet(this.tweenlerp_miss, tweenlerp, rollback_beats, beat_duration);
            InternalTweenlerpDuration(this.tweenlerp_miss, this.beatwatcher.tick);
        }

        public void TweenlerpSetBpm(float beats_per_minute) {
            this.beatwatcher.ChangeBpm(beats_per_minute);

            InternalTweenlerpDuration(this.tweenlerp_beat, this.beatwatcher.tick);
            InternalTweenlerpDuration(this.tweenlerp_hit, this.beatwatcher.tick);
            InternalTweenlerpDuration(this.tweenlerp_miss, this.beatwatcher.tick);
        }


        public int Animate(float elapsed) {
            if (this.drawable_animation != null) this.drawable_animation.Animate(elapsed);

            bool completed = true;
            bool ignore_beat = this.tweenlerp_beat.tweenlerp == null;// ignore if there no tweenlerp for beats
            bool beat_rollback_active = this.tweenlerp_beat.rollback_active;

            if (this.tweenlerp_active != null) {
                completed = InternalTweenlerpRun(this.tweenlerp_active, elapsed);

                // if the active tweenlerp is completed, do rollback
                if (!this.tweenlerp_active.rollback_active) {
                    if (completed) {
                        RoundStats.InternalTweenlerpSetup(this.tweenlerp_active, true);
                    } else if (!ignore_beat && !beat_rollback_active) {
                        // still running, ignore beat tweernlerp
                        beat_rollback_active = true;
                        this.tweenlerp_beat.rollback_active = true;
                        this.tweenlerp_beat.tweenlerp.MarkAsCompleted();
                    }
                }
            }

            // beat check
            if (this.beatwatcher.Poll()) {
                elapsed += this.beatwatcher.since;
                RoundStats.InternalTweenlerpSetup(this.tweenlerp_beat, false);// run again
            }

            if (!ignore_beat) {
                completed = InternalTweenlerpRun(this.tweenlerp_beat, elapsed);
                if (completed && !beat_rollback_active)
                    RoundStats.InternalTweenlerpSetup(this.tweenlerp_beat, true);// do rollback
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



        private static void InternalTweenlerpSet(TweenLerpInfo tweenlerp_info, TweenLerp tweenlerp, float rollback_beats, float beat_duration) {
            if (tweenlerp_info.tweenlerp != null) tweenlerp_info.tweenlerp.Destroy();
            tweenlerp_info.tweenlerp = tweenlerp.Clone();
            tweenlerp_info.rollback_beats = rollback_beats;
            tweenlerp_info.beat_duration = beat_duration;
        }

        private static void InternalTweenlerpDuration(TweenLerpInfo tweenlerp_info, float beat_duration) {
            tweenlerp_info.duration_rollback = tweenlerp_info.rollback_beats * beat_duration;
            tweenlerp_info.beat_duration = beat_duration * tweenlerp_info.beat_duration;
            RoundStats.InternalTweenlerpSetup(tweenlerp_info, tweenlerp_info.rollback_active);
        }

        private bool InternalTweenlerpRun(TweenLerpInfo tweenlerp_info, float elapsed) {
            int entry_id; float entry_value, entry_duration;

            if (tweenlerp_info.tweenlerp == null) return true;
            if (tweenlerp_info.tweenlerp.IsCompleted()) return true;

            int completed = tweenlerp_info.tweenlerp.Animate(elapsed);
            if (completed > 0) return true;

            int count = tweenlerp_info.tweenlerp.GetEntryCount();

            for (int i = 0 ; i < count ; i++) {
                tweenlerp_info.tweenlerp.PeekEntryByIndex(i, out entry_id, out entry_value, out entry_duration);
                if (entry_id == VertexProps.TEXTSPRITE_PROP_STRING) continue;// illegal property

                if (!VertexProps.IsPropertyEnumerable(entry_id))
                    entry_value *= this.tweenlerp_multiplier;

                this.drawable.SetProperty(entry_id, entry_value);
            }

            return false;
        }

        private static bool InternalTweenlerpSetup(TweenLerpInfo tweenlerp_info, bool rollback) {
            int entry_id; float entry_value, entry_duration;

            tweenlerp_info.rollback_active = rollback;

            if (tweenlerp_info.tweenlerp == null) return true;

            int count = tweenlerp_info.tweenlerp.GetEntryCount();
            float new_duration = rollback ? tweenlerp_info.duration_rollback : tweenlerp_info.beat_duration;

            for (int i = 0 ; i < count ; i++) {
                tweenlerp_info.tweenlerp.SwapBoundsByIndex(i);
                tweenlerp_info.tweenlerp.PeekEntryByIndex(i, out entry_id, out entry_value, out entry_duration);

                // only modify entries with negative duration
                if (entry_duration < 0) tweenlerp_info.tweenlerp.ChangeDurationByIndex(i, new_duration);
            }

            tweenlerp_info.tweenlerp.Restart();
            return false;
        }

        private class TweenLerpInfo {
            public TweenLerp tweenlerp;
            public float rollback_beats;
            public float duration_rollback;
            public float beat_duration;
            public bool rollback_active;
        }

    }

}
