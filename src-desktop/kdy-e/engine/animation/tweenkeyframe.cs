using System;
using Engine.Externals.LuaScriptInterop;
using Engine.Utils;

namespace Engine.Animation;

public class TweenKeyframe {

    private ArrayList<Entry> entries;

    private TweenKeyframe() { }


    public static TweenKeyframe Init() {
        return new TweenKeyframe() {
            entries = new ArrayList<Entry>()
        };
    }

    public static TweenKeyframe Init2(AnimListItem animlist_item) {
        if (!animlist_item.is_tweenkeyframe) {
            Logger.Error($"tweenkeyframe_init2() the animlist item is not a tweenkeyframe: {animlist_item.name}");
            return null;
        }

        TweenKeyframe tweenkeyframe = new TweenKeyframe() {
            entries = new ArrayList<Entry>(animlist_item.tweenkeyframe_entries_count)
        };

        for (int i = 0 ; i < animlist_item.tweenkeyframe_entries_count ; i++) {
            tweenkeyframe.InternalAdd(
                animlist_item.tweenkeyframe_entries[i].at,
                animlist_item.tweenkeyframe_entries[i].id,
                animlist_item.tweenkeyframe_entries[i].value,
                animlist_item.tweenkeyframe_entries[i].interpolator,
                animlist_item.tweenkeyframe_entries[i].steps_dir,
                animlist_item.tweenkeyframe_entries[i].steps_count
            );
        }

        return tweenkeyframe;
    }

    public static TweenKeyframe Init3(AnimList animlist, string tweenkeyframe_name) {
        AnimListItem animlist_item = null;

        for (int i = 0 ; i < animlist.entries_count ; i++) {
            if (animlist.entries[i].name == tweenkeyframe_name) {
                animlist_item = animlist.entries[i];
                break;
            }
        }

        if (animlist_item == null) {
            Logger.Warn($"tweenkeyframe_init3() the animlist does not contains: {tweenkeyframe_name}");
            return null;
        }

        return TweenKeyframe.Init2(animlist_item);
    }

    public void Destroy() {
        foreach (Entry entry in this.entries) {
            entry.steps.Destroy(false);
        }
        this.entries.Destroy(false);
        Luascript.DropShared(this);
        //free(this);
    }

    public TweenKeyframe Clone() {
        TweenKeyframe copy = new TweenKeyframe();
        copy.entries = this.entries.Clone();

        foreach (Entry entry in copy.entries) {
            entry.steps = entry.steps.Clone();
        }

        return copy;
    }


    public void AnimatePercent(double percent) {
        Entry[] array = this.entries.PeekArray();
        int entries_size = this.entries.Size();

        float final_percent = (float)Math2D.Clamp(percent, 0.0, 1.0);

        for (int i = 0 ; i < entries_size ; i++) {
            Entry entry = array[i];
            Step[] steps = entry.steps.PeekArray();
            int steps_size = entry.steps.Size();

            for (int j = 0 ; j < steps_size ; j++) {
                Step step = steps[i];

                if (percent >= step.percent_start && percent <= step.percent_end) {
                    entry.value = TweenKeyframe.InternalAnimateEntry(step, final_percent);
                } else if (percent >= step.percent_end) {
                    entry.value = step.value_end;
                }
            }
        }
    }


    public int GetIdsCount() {
        return this.entries.Size();
    }


    public float PeekValue() {
        if (this.entries.Size() < 1) return Single.NaN;
        Entry entry = this.entries.Get(0);
        return entry.value;
    }

    public float PeekValueByIndex(int index) {
        Entry entry = this.entries.Get(index);
        if (entry == null) return Single.NaN;

        return entry.value;
    }

    public bool PeekEntryByIndex(int index, out int id, out float value) {
        Entry entry = this.entries.Get(index);
        if (entry == null) {
            id = -1;
            value = Single.NaN;
            return false;
        }

        id = entry.id;
        value = entry.value;
        return true;
    }

    public float PeekValueById(int id) {
        foreach (Entry entry in this.entries) {
            if (entry.id == id) return entry.value;
        }

        return Single.NaN;
    }


    public int AddEase(float at, int id, float value) {
        return InternalAdd(
            at, id, value, AnimInterpolator.EASE, Align.NONE, -1
        );
    }

    public int AddEaseIn(float at, int id, float value) {
        return InternalAdd(
            at, id, value, AnimInterpolator.EASE_IN, Align.NONE, -1
        );
    }

    public int AddEaseOut(float at, int id, float value) {
        return InternalAdd(
            at, id, value, AnimInterpolator.EASE_OUT, Align.NONE, -1
        );
    }

    public int AddEaseInOut(float at, int id, float value) {
        return InternalAdd(
            at, id, value, AnimInterpolator.EASE_IN_OUT, Align.NONE, -1
        );
    }

    public int AddLinear(float at, int id, float value) {
        return InternalAdd(
            at, id, value, AnimInterpolator.LINEAR, Align.NONE, -1
        );
    }

    public int AddSteps(float at, int id, float value, int steps_count, Align steps_method) {
        return InternalAdd(
            at, id, value, AnimInterpolator.STEPS,
            steps_method, steps_count
        );
    }

    public int AddCubic(float at, int id, float value) {
        return InternalAdd(
            at, id, value, AnimInterpolator.CUBIC, Align.NONE, -1
        );
    }

    public int AddQuad(float at, int id, float value) {
        return InternalAdd(
            at, id, value, AnimInterpolator.QUAD, Align.NONE, -1
        );
    }

    public int AddExpo(float at, int id, float value) {
        return InternalAdd(
            at, id, value, AnimInterpolator.EXPO, Align.NONE, -1
        );
    }

    public int AddSin(float at, int id, float value) {
        return InternalAdd(
            at, id, value, AnimInterpolator.SIN, Align.NONE, -1
        );
    }

    public int AddInterpolator(float at, int id, float value, AnimInterpolator type) {
        return InternalAdd(
            at, id, value, type, Align.NONE, -1
        );
    }


    public void VertexSetProperties(ISetProperty vertex) {
        Entry[] array = this.entries.PeekArray();
        int size = this.entries.Size();

        for (int i = 0 ; i < size ; i++) {
            if (array[i].id < 0 || array[i].id == VertexProps.TEXTSPRITE_PROP_STRING) continue;
            if (Single.IsNaN(array[i].value)) continue;
            vertex.SetProperty(array[i].id, array[i].value);
        }
    }


    private static float InternalByBrezier(Step step, float percent) {
        return MacroExecutor.CalcCubicBezier(percent, step.brezier_points);
    }

    private static float InternalByLinear(Step step, float percent) {
        return percent;
    }

    private static float InternalBySteps(Step step, float percent) {
        return MacroExecutor.CalcSteps(percent, step.steps_bounds, step.steps_count, step.steps_dir);
    }

    private static float InternalByCubic(Step step, float progress) {
        return Math2D.LerpCubic(progress);
    }

    private static float InternalByQuad(Step step, float progress) {
        return Math2D.LerpQuad(progress);
    }

    private static float InternalByExpo(Step step, float progress) {
        return Math2D.LerpExpo(progress);
    }

    private static float InternalBySin(Step step, float progress) {
        return Math2D.LerpSin(progress);
    }


    private static float InternalAnimateEntry(Step step, float percent) {
        float interp_percent = step.callback(step, percent);
        float value = Math2D.Lerp(step.value_start, step.value_end, interp_percent);

        return value;
    }

    private int InternalAdd(float at, int id, float value, AnimInterpolator interp, Align steps_dir, int steps_count) {
        Entry entry = null;
        Step step = null;

        // find the requested entry
        foreach (Entry existing_entry in this.entries) {
            if (existing_entry.id == id) {
                entry = existing_entry;
                break;
            }
        }

        // create new entry (if necessary)
        if (entry == null) {
            entry = this.entries.Add(new Entry() {
                id = id,
                value = 0f,
                steps = new ArrayList<Step>()
            });
        }

        // find a duplicated step and replace if exists
        foreach (Step existing_step in entry.steps) {
            if (existing_step.percent_start == at) {
                step = existing_step;
                break;
            }
        }

        // create new step (if necessary)
        if (step == null) {
            step = entry.steps.Add(new Step {
                steps_dir = steps_dir,
                steps_count = steps_count,
                steps_bounds = new float[] { 0.0f, 0.0f, 0.0f },

                brezier_points = null,
                callback = null,

                value_start = value,
                value_end = value,

                percent_start = at,
                percent_end = 1.0f
            });
        }

        switch (interp) {
            case AnimInterpolator.EASE:
                step.callback = TweenKeyframe.InternalByBrezier;
                step.brezier_points = MacroExecutor.CUBIC_BREZIER_EASE;
                break;
            case AnimInterpolator.EASE_IN:
                step.callback = TweenKeyframe.InternalByBrezier;
                step.brezier_points = MacroExecutor.CUBIC_BREZIER_EASE_IN;
                break;
            case AnimInterpolator.EASE_OUT:
                step.callback = TweenKeyframe.InternalByBrezier;
                step.brezier_points = MacroExecutor.CUBIC_BREZIER_EASE_OUT;
                break;
            case AnimInterpolator.EASE_IN_OUT:
                step.callback = TweenKeyframe.InternalByBrezier;
                step.brezier_points = MacroExecutor.CUBIC_BREZIER_EASE_IN_OUT;
                break;
            case AnimInterpolator.STEPS:
                step.callback = TweenKeyframe.InternalBySteps;
                break;
            case AnimInterpolator.LINEAR:
                step.callback = TweenKeyframe.InternalByLinear;
                break;
            case AnimInterpolator.CUBIC:
                step.callback = TweenKeyframe.InternalByCubic;
                break;
            case AnimInterpolator.QUAD:
                step.callback = TweenKeyframe.InternalByQuad;
                break;
            case AnimInterpolator.EXPO:
                step.callback = TweenKeyframe.InternalByExpo;
                break;
            case AnimInterpolator.SIN:
                step.callback = TweenKeyframe.InternalBySin;
                break;
            default:
#if DEBUG
                Logger.Error("tweenkeyframe_internal_add() unknown interpolator provided");
#endif
                // this never should happen
                entry.steps.Remove(step);
                return -1;
        }

        TweenKeyframe.InternalCalculateEnds(entry);

        return this.entries.IndexOf(entry);
    }


    private static void InternalCalculateEnds(Entry entry) {
        // sort steps by starting percent
        entry.steps.Sort(TweenKeyframe.InternalSort);

        Step[] steps = entry.steps.PeekArray();
        int steps_count = entry.steps.Size();

        for (int i = 0, j = 1 ; i < steps_count ; i++, j++) {
            if (j < steps_count) {
                steps[i].value_end = steps[j].value_start;
                steps[i].percent_end = steps[j].percent_start;
            } else {
                steps[i].value_end = steps[i].value_start;
                steps[i].percent_end = 1f;
            }
        }
    }

    private static int InternalSort(Step a, Step b) {
        float percent_a = a.percent_start;
        float percent_b = b.percent_start;
        return percent_a.CompareTo(percent_b);
    }



    private delegate float Callback(Step step, float progress_percent);

    private class Step : ICloneable {
        public float percent_start;
        public float percent_end;
        public float value_start;
        public float value_end;
        public float[] brezier_points;
        public float[] steps_bounds;
        public int steps_count;
        public Align steps_dir;
        public Callback callback;

        public object Clone() {
            Step copy = (Step)this.MemberwiseClone();
            copy.steps_bounds = EngineUtils.CloneStructArray(this.steps_bounds, this.steps_bounds.Length);
            return copy;
        }
    }

    private class Entry : ICloneable {
        public int id;
        public float value;
        public ArrayList<Step> steps;

        public object Clone() => this.MemberwiseClone();
    }


}
