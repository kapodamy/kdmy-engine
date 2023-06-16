using System;
using Engine.Externals.LuaScriptInterop;
using Engine.Utils;

namespace Engine.Animation;

public class TweenKeyframe {

    private ArrayList<KeyframeEntry> arraylist_keyframes;
    private ArrayList<KeyframeValue> arraylist_values;
    private AnimInterpolator default_interpolator;

    private TweenKeyframe() { }


    public static TweenKeyframe Init() {
        return new TweenKeyframe() {
            arraylist_keyframes = new ArrayList<KeyframeEntry>(),
            arraylist_values = new ArrayList<KeyframeValue>(),
            default_interpolator = AnimInterpolator.LINEAR
        };
    }

    public static TweenKeyframe Init2(AnimListItem animlist_item) {
        if (!animlist_item.is_tweenkeyframe) {
            Console.Error.WriteLine("[ERROR] tweenkeyframe_init2() the animlist item is not a tweenkeyframe: " + animlist_item.name);
            return null;
        }

        TweenKeyframe tweenkeyframe = new TweenKeyframe() {
            arraylist_keyframes = new ArrayList<KeyframeEntry>(animlist_item.tweenkeyframe_entries_count),
            arraylist_values = new ArrayList<KeyframeValue>(),
            default_interpolator = animlist_item.tweenkeyframe_default_interpolator
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
            Console.Error.WriteLine("[WARN] tweenkeyframe_init3() the animlist does not contains: " + tweenkeyframe_name);
            return null;
        }

        return TweenKeyframe.Init2(animlist_item);
    }

    public void Destroy() {
        this.arraylist_keyframes.Destroy(false);
        this.arraylist_values.Destroy(false);
        Luascript.DropShared(this);
        //free(this);
    }

    public TweenKeyframe Clone() {
        TweenKeyframe copy = new TweenKeyframe() {
            arraylist_keyframes = this.arraylist_keyframes.Clone(),
            arraylist_values = this.arraylist_values.Clone()
        };

        //  (JS & C# only) clone steps_bounds
        foreach (KeyframeEntry entry in copy.arraylist_keyframes) {
            entry.steps_bounds = new float[] { entry.steps_bounds[0], entry.steps_bounds[1], entry.steps_bounds[2] };
        }

        return copy;
    }


    public void AnimatePercent(double percent) {
        var array = this.arraylist_keyframes.PeekArray();
        var size = this.arraylist_keyframes.Size();

        percent = Math2D.Clamp(percent, 0.0, 1.0);

        for (int i = 0 ; i < size ; i++) {
            KeyframeEntry keyframe_entry = array[i];
            if (percent >= keyframe_entry.percent_start && percent <= keyframe_entry.percent_end) {
                TweenKeyframe.InternalAnimateEntry(keyframe_entry, (float)percent);
            } else if (percent >= keyframe_entry.percent_end) {
                keyframe_entry.keyframe_value.value = keyframe_entry.value_end;
            }
        }
    }


    public int GetIdsCount() {
        return this.arraylist_keyframes.Size();
    }


    public float PeekValue() {
        if (this.arraylist_values.Size() < 1) return Single.NaN;
        return this.arraylist_values.Get(0).value;
    }

    public float PeekValueByIndex(int index) {
        KeyframeValue keyframe_value = this.arraylist_values.Get(index);
        if (keyframe_value == null) return Single.NaN;

        return keyframe_value.value;
    }

    public bool PeekEntryByIndex(int index, out int id, out float value) {
        KeyframeValue keyframe_value = this.arraylist_values.Get(index);
        if (keyframe_value == null) {
            id = -1;
            value = Single.NaN;
            return false;
        }

        id = keyframe_value.id;
        value = keyframe_value.value;
        return true;
    }

    public float PeekValueById(int id) {
        foreach (KeyframeValue keyframe_value in this.arraylist_values) {
            if (keyframe_value.id == id) return id;
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
        var array = this.arraylist_values.PeekArray();
        var size = this.arraylist_values.Size();

        for (int i = 0 ; i < size ; i++) {
            if (array[i].id < 0 || array[i].id == VertexProps.TEXTSPRITE_PROP_STRING) continue;
            if (Single.IsNaN(array[i].value)) continue;
            vertex.SetProperty(array[i].id, array[i].value);
        }
    }


    private static float InternalByBrezier(KeyframeEntry tweenkeyframe_entry, float percent) {
        return MacroExecutor.CalcCubicBezier(percent, tweenkeyframe_entry.brezier_points);
    }

    private static float InternalByLinear(KeyframeEntry tweenkeyframe_entry, float percent) {
        return percent;
    }

    private static float InternalBySteps(KeyframeEntry tweenkeyframe_entry, float percent) {
        return MacroExecutor.CalcSteps(
            percent, tweenkeyframe_entry.steps_bounds, tweenkeyframe_entry.steps_count, tweenkeyframe_entry.steps_dir
        );
    }

    private static float InternalByCubic(KeyframeEntry tweenkeyframe_entry, float progress) {
        return Math2D.LerpCubic(progress);
    }

    private static float InternalByQuad(KeyframeEntry tweenkeyframe_entry, float progress) {
        return Math2D.LerpQuad(progress);
    }

    private static float InternalByExpo(KeyframeEntry tweenkeyframe_entry, float progress) {
        return Math2D.LerpExpo(progress);
    }

    private static float InternalBySin(KeyframeEntry tweenkeyframe_entry, float progress) {
        return Math2D.LerpSin(progress);
    }


    private static void InternalAnimateEntry(KeyframeEntry tweenkeyframe_entry, float percent) {
        float interp_percent = tweenkeyframe_entry.callback(tweenkeyframe_entry, percent);
        float value = Math2D.Lerp(tweenkeyframe_entry.value_start, tweenkeyframe_entry.value_end, interp_percent);
        tweenkeyframe_entry.keyframe_value.value = value;
    }

    private int InternalAdd(float at, int id, float value, AnimInterpolator? interp, Align steps_dir, int steps_count) {
        KeyframeEntry keyframe_entry = null;
        int keyframe_entry_index = 0;

        // find a duplicated entry and replace
        foreach (KeyframeEntry entry in this.arraylist_keyframes) {
            if (entry.id == id && entry.percent_start == at) {
                keyframe_entry = entry;
                break;
            }
            keyframe_entry_index++;
        }

        if (keyframe_entry == null) {
            KeyframeEntry new_keyframe_entry = new KeyframeEntry() {
                steps_dir = steps_dir,
                steps_count = steps_count,
                steps_bounds = new float[] { 0.0f, 0.0f, 0.0f },

                brezier_points = null,
                callback = null,

                id = id,

                value_start = value,
                value_end = value,

                percent_start = at,
                percent_end = 1.0f,

                keyframe_value = null
            };

            this.arraylist_keyframes.Add(new_keyframe_entry);
            keyframe_entry = new_keyframe_entry;
        }

        switch (interp) {
            case AnimInterpolator.EASE:
                keyframe_entry.callback = TweenKeyframe.InternalByBrezier;
                keyframe_entry.brezier_points = MacroExecutor.CUBIC_BREZIER_EASE;
                break;
            case AnimInterpolator.EASE_IN:
                keyframe_entry.callback = TweenKeyframe.InternalByBrezier;
                keyframe_entry.brezier_points = MacroExecutor.CUBIC_BREZIER_EASE_IN;
                break;
            case AnimInterpolator.EASE_OUT:
                keyframe_entry.callback = TweenKeyframe.InternalByBrezier;
                keyframe_entry.brezier_points = MacroExecutor.CUBIC_BREZIER_EASE_OUT;
                break;
            case AnimInterpolator.EASE_IN_OUT:
                keyframe_entry.callback = TweenKeyframe.InternalByBrezier;
                keyframe_entry.brezier_points = MacroExecutor.CUBIC_BREZIER_EASE_IN_OUT;
                break;
            case AnimInterpolator.STEPS:
                keyframe_entry.callback = TweenKeyframe.InternalBySteps;
                break;
            case AnimInterpolator.LINEAR:
                keyframe_entry.callback = TweenKeyframe.InternalByLinear;
                break;
            case AnimInterpolator.CUBIC:
                keyframe_entry.callback = TweenKeyframe.InternalByCubic;
                break;
            case AnimInterpolator.QUAD:
                keyframe_entry.callback = TweenKeyframe.InternalByQuad;
                break;
            case AnimInterpolator.EXPO:
                keyframe_entry.callback = TweenKeyframe.InternalByExpo;
                break;
            case AnimInterpolator.SIN:
                keyframe_entry.callback = TweenKeyframe.InternalBySin;
                break;
            case null:
                interp = this.default_interpolator;
                return InternalAdd(at, id, value, interp, steps_dir, steps_count);
            default:
                this.arraylist_keyframes.Remove(keyframe_entry);
                return -1;
        }

        // store id for values
        int value_index = 0;
        foreach (KeyframeValue value_entry in this.arraylist_values) {
            if (value_entry.id == id) {
                keyframe_entry.keyframe_value = value_entry;
                goto L_calculate_ends_and_return;
            }
            value_index++;
        }

        KeyframeValue new_value_entry = new KeyframeValue() { id = id, value = Single.NaN };
        value_index = this.arraylist_values.Add(new_value_entry) - 1;
        keyframe_entry.keyframe_value = new_value_entry;

L_calculate_ends_and_return:
        InternalCalculateEnds();
        return value_index;
    }


    private void InternalCalculateEnds() {
        this.arraylist_keyframes.Sort(TweenKeyframe.InternalSort);

        var array = this.arraylist_keyframes.PeekArray();
        int last_index = this.arraylist_keyframes.Size() - 1;

        foreach (KeyframeValue value in this.arraylist_values) {
            KeyframeEntry last_entry = null;

            for (int i = last_index ; i >= 0 ; i--) {
                if (array[i].id != value.id) continue;

                if (last_entry != null) {
                    array[i].value_end = last_entry.value_start;
                    array[i].percent_end = last_entry.percent_start;
                } else {
                    array[i].value_end = array[i].value_start;
                    array[i].percent_end = 1f;
                }
                last_entry = array[i];
            }
        }
    }

    private static int InternalSort(KeyframeEntry a, KeyframeEntry b) {
        float combo_a = a.percent_start + a.id;
        float combo_b = b.percent_start + b.id;
        return combo_a.CompareTo(combo_b);
    }



    private delegate float Callback(KeyframeEntry tweenkeyframe_entry, float progress_percent);

    private class KeyframeEntry {
        public int id;
        public float percent_start;
        public float percent_end;
        public float value_start;
        public float value_end;
        public float[] brezier_points;
        public float[] steps_bounds;
        public int steps_count;
        public Align steps_dir;
        public KeyframeValue keyframe_value;
        public Callback callback;

        public KeyframeEntry Clone() {
            return new KeyframeEntry() {
                id = this.id,
                percent_start = this.percent_start,
                percent_end = this.percent_end,
                value_start = this.value_start,
                value_end = this.value_end,
                brezier_points = this.brezier_points,
                steps_bounds = this.steps_bounds,
                steps_count = this.steps_count,
                steps_dir = this.steps_dir,
                keyframe_value = null
            };
        }
    }

    private class KeyframeValue {
        public int id;
        public float value;
    }


}
