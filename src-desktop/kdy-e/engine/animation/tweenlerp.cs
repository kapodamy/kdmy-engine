using System;
using Engine.Externals.LuaScriptInterop;
using Engine.Utils;

namespace Engine.Animation {

    public class TweenLerp : IAnimate {

        private ArrayList<TweenLerpEntry> arraylist;
        private double progress;
        private bool has_completed;

        public TweenLerp() {
            this.arraylist = new ArrayList<TweenLerpEntry>();
            this.progress = 0;
            this.has_completed = false;
        }

        public void Destroy() {
            this.arraylist.Destroy(false);
            Luascript.DropShared(this);
            //free(this);
        }

        public TweenLerp Clone() {
            TweenLerp copy = new TweenLerp() {
                arraylist = this.arraylist.Clone(),
                progress = this.progress,
                has_completed = this.has_completed
            };

            //  (JS & C# only) clone steps_bounds
            foreach (TweenLerpEntry entry in copy.arraylist) {
                entry.steps_bounds = new float[] { entry.steps_bounds[0], entry.steps_bounds[1], entry.steps_bounds[2] };
            }

            return copy;
        }


        public void End() {
            foreach (TweenLerpEntry entry in this.arraylist) {
                if (entry.duration > this.progress) this.progress = entry.duration;
                entry.value = entry.end;
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

            AnimateTimestamp(this.progress + elapsed);

            return 0;// keep last "frame" alive
        }

        public int AnimateTimestamp(double timestamp) {
            this.progress = timestamp;

            TweenLerpEntry[] array = this.arraylist.PeekArray();
            int size = this.arraylist.Size();
            int completed = 0;

            for (int i = 0 ; i < size ; i++) {
                if (TweenLerp.InternalAnimateEntry(array[i], (float)this.progress)) completed++;
            }

            if (completed >= size) this.has_completed = true;

            return completed;
        }

        public int AnimatePercent(double percent) {
            this.progress = 0.0;// undefined behavoir

            var array = this.arraylist.PeekArray();
            var size = this.arraylist.Size();

            int completed = 0;

            percent = Math2D.Clamp(percent, 0.0, 1.0);

            for (int i = 0 ; i < size ; i++) {
                if (TweenLerp.InternalAnimateEntryAbsolute(array[i], (float)percent)) completed++;
            }

            this.has_completed = percent >= 1.0;
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
            if (this.arraylist.Size() < 1) return Single.NaN;
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

            return TweenLerp.InternalAnimateEntry(tweenlerp_entry, (float)this.progress);
        }

        public bool OverrideStartWithEndByIndex(int index) {
            TweenLerpEntry tweenlerp_entry = this.arraylist.Get(index);

            tweenlerp_entry.start = tweenlerp_entry.end;

            return TweenLerp.InternalAnimateEntry(tweenlerp_entry, (float)this.progress);
        }

        public bool ChangeBoundsById(int id, float new_start, float new_end) {
            var array = this.arraylist.PeekArray();
            var size = this.arraylist.Size();

            for (int i = 0 ; i < size ; i++) {
                if (array[i].id != id) continue;
                array[i].start = new_start;
                array[i].end = new_end;
                TweenLerp.InternalAnimateEntry(array[i], (float)this.progress);
                return true;
            }

            return false;
        }

        public bool ChangeDurationByIndex(int index, float new_duration) {
            TweenLerpEntry tweenlerp_entry = this.arraylist.Get(index);
            tweenlerp_entry.duration = new_duration;

            return TweenLerp.InternalAnimateEntry(tweenlerp_entry, (float)this.progress);
        }

        public bool SwapBoundsByIndex(int index) {
            TweenLerpEntry tweenlerp_entry = this.arraylist.Get(index);
            float temp = tweenlerp_entry.start;

            tweenlerp_entry.start = tweenlerp_entry.end;
            tweenlerp_entry.end = temp;

            return TweenLerp.InternalAnimateEntry(tweenlerp_entry, (float)this.progress);
        }


        public int AddEase(int id, float start, float end, float duration) {
            return InternalAdd(
                id, start, end, duration, AnimInterpolator.EASE, Align.NONE, -1
            );
        }

        public int AddEaseIn(int id, float start, float end, float duration) {
            return InternalAdd(
                id, start, end, duration, AnimInterpolator.EASE_IN, Align.NONE, -1
            );
        }

        public int AddEaseOut(int id, float start, float end, float duration) {
            return InternalAdd(
                id, start, end, duration, AnimInterpolator.EASE_OUT, Align.NONE, -1
            );
        }

        public int AddEaseInOut(int id, float start, float end, float duration) {
            return InternalAdd(
                id, start, end, duration, AnimInterpolator.EASE_IN_OUT, Align.NONE, -1
            );
        }

        public int AddLinear(int id, float start, float end, float duration) {
            return InternalAdd(
                id, start, end, duration, AnimInterpolator.LINEAR, Align.NONE, -1
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


        private static float InternalByBrezier(TweenLerpEntry tweenlerp_entry, float progress) {
            return MacroExecutor.CalcCubicBezier(progress, tweenlerp_entry.brezier_points);
        }

        private static float InternalByLinear(TweenLerpEntry tweenlerp_entry, float progress) {
            return progress;
        }

        private static float InternalBySteps(TweenLerpEntry tweenlerp_entry, float progress) {
            float completed = Math2D.InverseLerp(0, tweenlerp_entry.duration, progress);
            return MacroExecutor.CalcSteps(
                completed, tweenlerp_entry.steps_bounds, tweenlerp_entry.steps_count, tweenlerp_entry.steps_dir
            );
        }

        private static bool InternalAnimateEntry(TweenLerpEntry tweenlerp_entry, float progress) {
            // calculate the completed percent
            if (progress > tweenlerp_entry.duration) progress = tweenlerp_entry.duration;
            float progress_percent = Math2D.InverseLerp(0, tweenlerp_entry.duration, progress);
            float percent = tweenlerp_entry.callback(tweenlerp_entry, progress_percent);

            tweenlerp_entry.value = Math2D.Lerp(tweenlerp_entry.start, tweenlerp_entry.end, percent);
            return progress >= tweenlerp_entry.duration;
        }

        private static bool InternalAnimateEntryAbsolute(TweenLerpEntry tweenlerp_entry, float progress_percent) {
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
                this.has_completed = InternalAnimateEntry(tweenlerp_entry, (float)this.progress);

            return this.arraylist.Add(tweenlerp_entry) - 1;
        }


        private delegate float TweenLerpCallback(TweenLerpEntry tweenlerp_entry, float progress_percent);

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
        }


    }

}
