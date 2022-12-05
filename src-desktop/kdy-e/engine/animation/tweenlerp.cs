using System;
using Engine.Externals.LuaScriptInterop;
using Engine.Utils;

namespace Engine.Animation {

    public class TweenLerp : IAnimate {

        private delegate float TweenLerpCallback(TweenLerpEntry tweenlerp_entry, double progress_percent);

        private class TweenLerpEntry {
            public float duration;
            public float value;
            public int id;
            public float start;
            public float end;
            public float[] brezier_points;
            public float[] steps_bounds;
            public int steps_count;
            public Align steps_dir;
            public TweenLerpCallback callback;

            public TweenLerpEntry Clone() {
                return new TweenLerpEntry() {
                    duration = this.duration,
                    id = this.id,
                    value = this.value,
                    start = this.start,
                    end = this.end,
                    brezier_points = this.brezier_points,
                    steps_bounds = this.steps_bounds,
                    steps_count = this.steps_count,
                    steps_dir = this.steps_dir
                };
            }
        }


        private ArrayList<TweenLerpEntry> arraylist;
        private double progress;
        private bool has_completed;

        private TweenLerp() { }

        public static TweenLerp Init() {
            return new TweenLerp() {
                arraylist = new ArrayList<TweenLerpEntry>(),
                progress = 0.0,
                has_completed = false
            };
        }

        public static TweenLerp Init2(AnimListItem animlist_item) {
            if (!animlist_item.is_tweenlerp) {
                Console.Error.WriteLine("tweenlerp_init2() the animlist item is not a tweenlerp: " + animlist_item.name);
                return null;
            }

            TweenLerp tweenlerp = new TweenLerp() {
                arraylist = new ArrayList<TweenLerpEntry>(animlist_item.tweenlerp_entries_count),
                progress = 0.0,
                has_completed = false
            };

            for (int i = 0 ; i < animlist_item.tweenlerp_entries_count ; i++) {
                tweenlerp.InternalAdd(
                    animlist_item.tweenlerp_entries[i].id,
                    animlist_item.tweenlerp_entries[i].start,
                    animlist_item.tweenlerp_entries[i].end,
                    animlist_item.tweenlerp_entries[i].duration,
                    animlist_item.tweenlerp_entries[i].interp,
                    animlist_item.tweenlerp_entries[i].steps_dir,
                    (int)animlist_item.tweenlerp_entries[i].steps_count
                );
            }

            return tweenlerp;
        }

        public static TweenLerp Init3(AnimList animlist, string tweenlerp_name) {
            AnimListItem animlist_item = null;

            for (int i = 0 ; i < animlist.entries_count ; i++) {
                if (animlist.entries[i].name == tweenlerp_name) {
                    animlist_item = animlist.entries[i];
                    break;
                }
            }

            if (animlist_item == null) {
                Console.Error.WriteLine("tweenlerp_init3() the animlist does not contains: " + tweenlerp_name);
                return null;
            }

            return TweenLerp.Init2(animlist_item);
        }

        public void Destroy() {
            this.arraylist.Destroy(false);
            this.arraylist = null;
            Luascript.DropShared(this);
            //free(tweenlerp);
        }

        public TweenLerp Clone() {
            //if (!this) return null;
            int array_size = this.arraylist.Size();
            TweenLerp copy = new TweenLerp() {
                arraylist = new ArrayList<TweenLerpEntry>(array_size),
                progress = this.progress,
                has_completed = this.has_completed
            };

            TweenLerpEntry[] array_old = this.arraylist.PeekArray();
            TweenLerpEntry[] array_new = copy.arraylist.PeekArray();

            for (int i = 0 ; i < array_size ; i++) {
                array_new[i] = array_old[i].Clone();
            }

            return copy;
        }


        public void End() {
            TweenLerpEntry[] array = this.arraylist.PeekArray();
            int size = this.arraylist.Size();

            for (int i = 0 ; i < size ; i++) {
                if (array[i].duration > this.progress) this.progress = array[i].duration;
                array[i].value = array[i].end;
            }

            this.has_completed = true;
        }

        public void MarkAsCompleted() {
            this.has_completed = true;
        }


        public void Restart() {
            this.progress = 0.0;
            this.has_completed = false;
            AnimatePercent(0.0);
        }

        public int Animate(float elapsed) {
            if (this.has_completed) return 1;

            this.progress += elapsed;

            TweenLerpEntry[] array = this.arraylist.PeekArray();
            int size = this.arraylist.Size();
            int completed = 0;

            for (int i = 0 ; i < size ; i++) {
                if (TweenLerp.InternalAnimateEntry(array[i], this.progress)) completed++;
            }

            if (completed >= size) this.has_completed = true;

            return 0;// keep last "frame" alive
        }

        public int AnimateTimestamp(double timestamp) {
            this.progress = timestamp;

            TweenLerpEntry[] array = this.arraylist.PeekArray();
            int size = this.arraylist.Size();
            int completed = 0;

            for (int i = 0 ; i < size ; i++) {
                if (TweenLerp.InternalAnimateEntry(array[i], timestamp)) completed++;
            }

            this.has_completed = completed >= size;
            return completed;
        }

        public int AnimatePercent(double percent) {
            this.progress = -1;// undefined behavoir

            TweenLerpEntry[] array = this.arraylist.PeekArray();
            int size = this.arraylist.Size();
            int completed = 0;

            percent = Math2D.Clamp(percent, 0.0, 1.0);

            for (int i = 0 ; i < size ; i++) {
                if (TweenLerp.InternalAnimateEntryAbsolute(array[i], percent)) completed++;
            }

            this.has_completed = completed >= size;
            return completed;
        }

        public bool IsCompleted() {
            return this.has_completed;
        }

        public double GetElapsed() {
            return this.progress;
        }

        public int GetEntryCount() {
            return this.arraylist.Size();
        }


        public float PeekValue() {
            return this.arraylist.Get(0).value;
        }

        public float PeekValueByIndex(int index) {
            TweenLerpEntry entry = this.arraylist.Get(index);
            if (entry == null) return Single.NaN;
            return entry.value;
        }

        public bool PeekEntryByIndex(int index, out int id, out float value, out float duration) {
            TweenLerpEntry entry = this.arraylist.Get(index);
            if (entry == null) {
                id = -1;
                value = Single.NaN;
                duration = Single.NaN;
                return false;
            }
            id = entry.id;
            value = entry.value;
            duration = entry.duration;
            return true;
        }

        public float PeekValueById(int id) {
            TweenLerpEntry[] array = this.arraylist.PeekArray();
            int size = this.arraylist.Size();

            for (int i = 0 ; i < size ; i++) {
                if (array[i].id == id) return array[i].value;
            }

            return Single.NaN;
        }

        public bool ChangeBoundsByIndex(int index, float new_start, float new_end) {
            TweenLerpEntry tweenlerp_entry = this.arraylist.Get(index);

            if (!Single.IsNaN(new_start)) tweenlerp_entry.start = new_start;
            if (!Single.IsNaN(new_end)) tweenlerp_entry.end = new_end;

            return TweenLerp.InternalAnimateEntry(tweenlerp_entry, this.progress);
        }

        public bool OverrideStartWithEndByIndex(int index) {
            TweenLerpEntry tweenlerp_entry = this.arraylist.Get(index);

            tweenlerp_entry.start = tweenlerp_entry.end;

            return TweenLerp.InternalAnimateEntry(tweenlerp_entry, this.progress);
        }

        public bool ChangeBoundsById(int id, float new_start, float new_end) {
            TweenLerpEntry[] array = this.arraylist.PeekArray();
            int size = this.arraylist.Size();

            for (int i = 0 ; i < size ; i++) {
                if (array[i].id != id) continue;
                array[i].start = new_start;
                array[i].end = new_end;
                TweenLerp.InternalAnimateEntry(array[i], this.progress);
                return true;
            }

            return false;
        }

        public bool ChangeDurationByIndex(int index, float new_duration) {
            TweenLerpEntry tweenlerp_entry = this.arraylist.Get(index);
            tweenlerp_entry.duration = new_duration;

            return TweenLerp.InternalAnimateEntry(tweenlerp_entry, this.progress);
        }

        public bool SwapBoundsByIndex(int index) {
            TweenLerpEntry tweenlerp_entry = this.arraylist.Get(index);
            float temp = tweenlerp_entry.start;

            tweenlerp_entry.start = tweenlerp_entry.end;
            tweenlerp_entry.end = temp;

            return TweenLerp.InternalAnimateEntry(tweenlerp_entry, this.progress);
        }


        public int AddEase(int id, float start, float end, float duration) {
            return InternalAdd(
                id, start, end, duration, AnimInterpolator.EASE, Align.INVALID, -1
            );
        }

        public int TweenLerpAddEaseIn(int id, float start, float end, float duration) {
            return InternalAdd(
                id, start, end, duration, AnimInterpolator.EASE_IN, Align.INVALID, -1
            );
        }

        public int AddEaseOut(int id, float start, float end, float duration) {
            return InternalAdd(
                id, start, end, duration, AnimInterpolator.EASE_OUT, Align.INVALID, -1
            );
        }

        public int AddEaseInOut(int id, float start, float end, float duration) {
            return InternalAdd(
                id, start, end, duration, AnimInterpolator.EASE_IN_OUT, Align.INVALID, -1
            );
        }

        public int AddLinear(int id, float start, float end, float duration) {
            return InternalAdd(
                id, start, end, duration, AnimInterpolator.LINEAR, Align.INVALID, -1
            );
        }

        public int AddSteps(int id, float start, float end, float duration, int steps_count, Align steps_method) {
            return InternalAdd(
                id, start, end, duration, AnimInterpolator.STEPS,
                steps_method, steps_count
            );
        }

        public int AddInterpolator(int id, float start, float end, float duration, AnimInterpolator type) {
            return InternalAdd(id, start, end, duration, type, Align.NONE, 2);
        }


        public void VertexSetProperties(ISetProperty vertex) {
            TweenLerpEntry[] array = this.arraylist.PeekArray();
            int size = this.arraylist.Size();

            for (int i = 0 ; i < size ; i++) {
                if (array[i].id < 0 || array[i].id == VertexProps.TEXTSPRITE_PROP_STRING) continue;
                vertex.SetProperty(array[i].id, array[i].value);
            }
        }


        private static float InternalByBrezier(TweenLerpEntry tweenlerp_entry, double progress) {
            return MacroExecutor.CalcCubicBezier((float)progress, tweenlerp_entry.brezier_points);
        }

        private static float InternalByLinear(TweenLerpEntry tweenlerp_entry, double progress) {
            return (float)progress;
        }

        private static float InternalBySteps(TweenLerpEntry tweenlerp_entry, double progress) {
            float completed = Math2D.InverseLerp(0, tweenlerp_entry.duration, (float)progress);
            return MacroExecutor.CalcSteps(
                completed, tweenlerp_entry.steps_bounds, tweenlerp_entry.steps_count, tweenlerp_entry.steps_dir
            );
        }

        private static bool InternalAnimateEntry(TweenLerpEntry tweenlerp_entry, double progress) {
            // calculate the completed percent
            if (progress > tweenlerp_entry.duration) progress = tweenlerp_entry.duration;

            float progress_percent = Math2D.InverseLerp(0, tweenlerp_entry.duration, (float)progress);
            float percent = tweenlerp_entry.callback(tweenlerp_entry, progress_percent);

            tweenlerp_entry.value = Math2D.Lerp(tweenlerp_entry.start, tweenlerp_entry.end, percent);
            return progress >= tweenlerp_entry.duration;
        }

        private static bool InternalAnimateEntryAbsolute(TweenLerpEntry tweenlerp_entry, double progress_percent) {
            float percent = tweenlerp_entry.callback(tweenlerp_entry, progress_percent);
            tweenlerp_entry.value = Math2D.Lerp(tweenlerp_entry.start, tweenlerp_entry.end, percent);
            return percent >= 1.0;
        }

        private int InternalAdd(int id, float start, float end, float duration, AnimInterpolator interp, Align steps_dir, int steps_count) {
            TweenLerpEntry tweenlerp_entry = new TweenLerpEntry() {
                steps_dir = steps_dir,
                steps_count = steps_count,
                steps_bounds = new float[] { 0.0f, 0.0f, 0.0f },

                brezier_points = null,
                callback = null,

                id = id,
                value = start,

                start = start,
                end = end,
                duration = duration
            };

            switch (interp) {
                case AnimInterpolator.EASE:
                    tweenlerp_entry.callback = TweenLerp.InternalByBrezier;
                    tweenlerp_entry.brezier_points = MacroExecutor.CUBIC_BREZIER_EASE;
                    break;
                case AnimInterpolator.EASE_IN:
                    tweenlerp_entry.callback = TweenLerp.InternalByBrezier;
                    tweenlerp_entry.brezier_points = MacroExecutor.CUBIC_BREZIER_EASE_IN;
                    break;
                case AnimInterpolator.EASE_OUT:
                    tweenlerp_entry.callback = TweenLerp.InternalByBrezier;
                    tweenlerp_entry.brezier_points = MacroExecutor.CUBIC_BREZIER_EASE_OUT;
                    break;
                case AnimInterpolator.EASE_IN_OUT:
                    tweenlerp_entry.callback = TweenLerp.InternalByBrezier;
                    tweenlerp_entry.brezier_points = MacroExecutor.CUBIC_BREZIER_EASE_IN_OUT;
                    break;
                case AnimInterpolator.STEPS:
                    tweenlerp_entry.callback = TweenLerp.InternalBySteps;
                    break;
                case AnimInterpolator.LINEAR:
                    tweenlerp_entry.callback = TweenLerp.InternalByLinear;
                    break;
                default:
                    return -1;
            }

            // if all other entries are completed, check if this entry is completed too
            if (this.has_completed)
                this.has_completed = TweenLerp.InternalAnimateEntry(tweenlerp_entry, this.progress);

            return this.arraylist.Add(tweenlerp_entry) - 1;
        }

    }

}
