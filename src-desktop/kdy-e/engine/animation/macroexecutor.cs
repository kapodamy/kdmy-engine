using System;
using Engine.Font;
using Engine.Image;
using Engine.Utils;

namespace Engine.Animation;

public enum AnimMacroType : int {
    INTERPOLATOR = 0,
    SET = 1,
    YIELD = 2,
    PAUSE = 3,
    RESET = 4,
    RANDOM_SETUP = 5,
    RANDOM_CHOOSE = 6,
    RANDOM_EXACT = 7,
    REGISTER_PROP = 8,
    REGISTER_SET = 9,
}

public enum AnimInterpolator : int {
    EASE = 0,
    EASE_IN = 1,
    EASE_OUT = 2,
    EASE_IN_OUT = 3,
    LINEAR = 4,
    STEPS = 5,
    CUBIC = 6,
    QUAD = 7,
    EXPO = 8,
    SIN = 9
}

public class MacroExecutorInstruction {
    public int property;
    public AnimMacroType type;
    internal MacroExecutorValue value;
    internal MacroExecutorValue start;
    internal MacroExecutorValue end;
    internal int values_size;
    internal MacroExecutorValue[] values;
    internal int register_index;
    internal MacroExecutorValue steps_count;
    internal MacroExecutorValue steps_method;
    internal MacroExecutorValue duration;
    internal AnimInterpolator interpolator;
}

public enum MacroExecutorValueKind {
    LITERAL,
    SPECIAL,
    PROPERTY
}

public struct MacroExecutorValue {
    public float literal;
    public int reference;
    public MacroExecutorValueKind kind;
}


public class MacroExecutor : IAnimate {

    public static readonly float[] CUBIC_BREZIER_EASE_LINEAR = { 0.0f, 0.0f, 1.0f, 1.0f };
    public static readonly float[] CUBIC_BREZIER_EASE = { 0.25f, 0.1f, 0.25f, 1.0f };
    public static readonly float[] CUBIC_BREZIER_EASE_IN = { 0.42f, 0.0f, 1.0f, 1.0f };
    public static readonly float[] CUBIC_BREZIER_EASE_OUT = { 0.0f, 0.0f, 0.58f, 1.0f };
    public static readonly float[] CUBIC_BREZIER_EASE_IN_OUT = { 0.42f, 0.0f, 0.58f, 1.0f };

    public const int ANIM_MACRO_SPECIAL_REGISTER0 = 0;
    public const int ANIM_MACRO_SPECIAL_REGISTER1 = 1;
    public const int ANIM_MACRO_SPECIAL_REGISTER2 = 2;
    public const int ANIM_MACRO_SPECIAL_REGISTER3 = 3;
    public const int ANIM_MACRO_SPECIAL_RANDOM = 9;

    /** count of "SPRITE_PROP_* + TEXTSPRITE_PROP_* + MEDIA_PROP_* + FONT_PROP_* + LAYOUT_PROP_* + CAMERA_PROP_*" */
    public const int MACROEXECUTOR_VERTEX_PROP_COUNT = 74;
    public const int MACROEXECUTOR_REGISTER_COUNT = 4;// this must match ANIM_MACRO_SPECIAL_REGISTER* count


    private int interpolators_stack_size;
    private int instructions_size;
    internal AtlasEntry[] frames;
    internal int frame_count;
    private bool frames_dispose;
    private float random_start;
    private float random_end;
    private float random_value;
    private float speed;
    private float[] registers;
    private InStackInstruction[] interpolators_stack;
    private MacroExecutorInstruction[] instructions;
    private int index;
    private bool on_yield;
    private float sleep_time;
    private int frame_index_restart;
    private bool frame_allow_change;
    private float[] state;
    private bool[] state_flags;
    private bool last_step;


    private MacroExecutor() { }

    public MacroExecutor(MacroExecutorInstruction[] instructions, int instructions_size, AtlasEntry[] frames, int frame_count) {
        this.interpolators_stack_size = 0;
        this.instructions_size = instructions_size;

        this.frames = frames;
        this.frame_count = frame_count;
        this.frames_dispose = false;

        this.random_start = 0f;
        this.random_end = 1.0f;
        this.random_value = 0.5f;
        this.speed = 1.0f;
        this.registers = new float[MACROEXECUTOR_REGISTER_COUNT];

        for (int i = 0 ; i < instructions_size ; i++) {
            if (instructions[i] == null)
                continue;
            if (instructions[i].type == AnimMacroType.INTERPOLATOR)
                this.interpolators_stack_size++;
        }

        ClearRegisters();

        this.interpolators_stack = new InStackInstruction[this.interpolators_stack_size];
        this.instructions = instructions;

        for (int i = 0 ; i < this.interpolators_stack_size ; i++) {
            this.interpolators_stack[i] = new InStackInstruction();
        }

        this.index = 0;
        this.on_yield = false;
        this.sleep_time = 0;

        this.frame_index_restart = -1;
        this.frame_allow_change = false;

        this.state = new float[MACROEXECUTOR_VERTEX_PROP_COUNT];
        this.state_flags = new bool[MACROEXECUTOR_VERTEX_PROP_COUNT];
        StateClear();

        for (int i = 0 ; i < this.interpolators_stack_size ; i++)
            this.interpolators_stack[i] = new InStackInstruction();

        this.last_step = true;
    }

    public void Destroy() {
        //free(this.interpolators_stack);
        //free(this.instructions);
        //free(this);
    }

    public void SetRestartInFrame(int frame_index, bool allow_change_size) {
        if (frame_index < 0 && frame_index >= this.frame_count)
            this.frame_index_restart = -1;
        else
            this.frame_index_restart = frame_index;

        this.frame_allow_change = allow_change_size;
    }

    public void SetSpeed(float speed) {
        this.speed = speed;
    }

    public void Restart() {
        this.index = 0;
        this.on_yield = false;
        this.sleep_time = 0;
        this.last_step = true;

        StateClear();

        if (this.frame_index_restart >= 0) {
            this.state[VertexProps.SPRITE_PROP_FRAMEINDEX] = this.frame_index_restart;
            ApplyFrame(null);
        }

        for (int i = 0 ; i < this.interpolators_stack_size ; i++)
            this.interpolators_stack[i].definition = null;
    }

    public int Animate(float elapsed) {
        if (this.speed != 1.0) elapsed /= this.speed;

        int running = InterpolatorsCheck(elapsed);

        if (this.sleep_time > 0.0 || this.on_yield) {
            if (this.on_yield && running > 0) return 0;

            if (this.sleep_time > 0.0) {
                this.sleep_time -= elapsed;
                if (this.sleep_time > 0) return 0;

                elapsed = -this.sleep_time;
            }

            // pause/yield instruction completed
            this.sleep_time = 0.0f;
            this.on_yield = false;
            this.index++;
        }

        for (; this.index < this.instructions_size ; this.index++) {
            MacroExecutorInstruction instruction = this.instructions[this.index];

            switch (instruction.type) {
                case AnimMacroType.INTERPOLATOR:
                    running++;
                    InterpolatorStack(instruction, false);
                    break;
                case AnimMacroType.SET:
                    float value = ResolveValue(instruction.value);
                    StateSave(instruction.property, value);
                    break;
                case AnimMacroType.YIELD:
                case AnimMacroType.PAUSE:
                    this.on_yield = instruction.type == AnimMacroType.YIELD;
                    this.sleep_time = ResolveValue(instruction.value);
                    // check for ANIM_MACRO_PAUSE with zero duration 
                    if (!this.on_yield && this.sleep_time < 1) {
                        this.sleep_time = 0;
                        this.on_yield = true;
                    }
                    return 0;
                case AnimMacroType.RESET:
                    StateClear();
                    ClearRegisters();
                    ApplyFrame(null);
                    break;
                case AnimMacroType.RANDOM_SETUP:
                    this.random_start = ResolveValue(instruction.start);
                    this.random_end = ResolveValue(instruction.end);
                    break;
                case AnimMacroType.RANDOM_CHOOSE:
                    this.random_value = Math2D.RandomFloatRange(
                        this.random_start,
                        this.random_end
                    );
                    break;
                case AnimMacroType.RANDOM_EXACT:
                    RandomExact(instruction);
                    break;
                case AnimMacroType.REGISTER_PROP:
                case AnimMacroType.REGISTER_SET:
                    WriteRegister(instruction);
                    break;
            }
        }


        //return running < 1;
        if (running > 0) return 0;

        if (this.last_step) {
            // keep this frame
            this.last_step = false;
            return 0;
        }
        return 1;
    }

    public void StateApply(Sprite sprite, bool no_stack_changes) {
        ApplyFrame(sprite);

        for (int i = 0 ; i < MACROEXECUTOR_VERTEX_PROP_COUNT ; i++) {
            if (this.state_flags[i])
                sprite.SetProperty(i, this.state[i]);
        }

        if (no_stack_changes) return;

        ClearFlags();
    }

    public void StateApply2(StateSprite statesprite, bool no_stack_changes) {
        ApplyFrame2(statesprite);

        for (int i = 0 ; i < MACROEXECUTOR_VERTEX_PROP_COUNT ; i++) {
            if (this.state_flags[i])
                statesprite.SetProperty(i, this.state[i]);
        }

        if (no_stack_changes) return;

        ClearFlags();
    }

    public void StateApply3(Drawable drawable, bool no_stack_changes) {
        for (int i = 0 ; i < MACROEXECUTOR_VERTEX_PROP_COUNT ; i++) {
            if (this.state_flags[i])
                drawable.SetProperty(i, this.state[i]);
        }

        if (no_stack_changes) return;

        ClearFlags();
    }

    public void StateApply4(TextSprite textsprite, bool no_stack_changes) {
        for (int i = 0 ; i < MACROEXECUTOR_VERTEX_PROP_COUNT ; i++) {
            if (this.state_flags[i])
                textsprite.SetProperty(i, this.state[i]);
        }

        if (no_stack_changes) return;

        ClearFlags();
    }

    public void StateApply5(ISetProperty private_data, bool no_stack_changes) {
        for (int i = 0 ; i < MACROEXECUTOR_VERTEX_PROP_COUNT ; i++) {
            if (this.state_flags[i])
                private_data.SetProperty(i, this.state[i]);
        }

        if (no_stack_changes) return;

        ClearFlags();
    }

    public void StateApplyMinimal(Sprite sprite) {
        int[] MACROEXECUTOR_MINIMAL_PROPS = {
            VertexProps.SPRITE_PROP_FRAMEINDEX,
            VertexProps.SPRITE_PROP_ALPHA,
            VertexProps.SPRITE_PROP_VERTEX_COLOR_R,
            VertexProps.SPRITE_PROP_VERTEX_COLOR_G,
            VertexProps.SPRITE_PROP_VERTEX_COLOR_B
        };

        for (int i = 0 ; i < MACROEXECUTOR_MINIMAL_PROPS.Length ; i++) {
            if (this.state_flags[i])
                sprite.SetProperty(i, this.state[i]);
        }
    }

    public void StateToModifier(Modifier modifier, bool no_stack_changes) {
        for (int i = 0 ; i < MACROEXECUTOR_VERTEX_PROP_COUNT ; i++) {
            if (this.state_flags[i])
                modifier.SetProperty(i, this.state[i]);
        }

        if (no_stack_changes) return;

        ClearFlags();
    }

    public void StateFromModifier(Modifier modifier) {
        this.state[VertexProps.SPRITE_PROP_ROTATE] = modifier.rotate;
        this.state[VertexProps.SPRITE_PROP_SCALE_X] = modifier.scale_x;
        this.state[VertexProps.SPRITE_PROP_SCALE_Y] = modifier.scale_y;
        this.state[VertexProps.SPRITE_PROP_TRANSLATE_X] = modifier.translate_x;
        this.state[VertexProps.SPRITE_PROP_TRANSLATE_Y] = modifier.translate_y;
        this.state[VertexProps.SPRITE_PROP_SKEW_X] = modifier.skew_x;
        this.state[VertexProps.SPRITE_PROP_SKEW_Y] = modifier.skew_y;
        this.state[VertexProps.SPRITE_PROP_ROTATE_PIVOT_ENABLED] = modifier.rotate_pivot_enabled == true ? 1f : 0f;
        this.state[VertexProps.SPRITE_PROP_ROTATE_PIVOT_U] = modifier.rotate_pivot_u;
        this.state[VertexProps.SPRITE_PROP_ROTATE_PIVOT_V] = modifier.rotate_pivot_v;
        this.state[VertexProps.SPRITE_PROP_SCALE_DIRECTION_X] = modifier.scale_direction_x;
        this.state[VertexProps.SPRITE_PROP_SCALE_DIRECTION_Y] = modifier.scale_direction_y;
        this.state[VertexProps.SPRITE_PROP_TRANSLATE_ROTATION] = modifier.translate_rotation == true ? 1f : 0f;
        this.state[VertexProps.SPRITE_PROP_SCALE_SIZE] = modifier.scale_size == true ? 1f : 0f;
        this.state[VertexProps.SPRITE_PROP_SCALE_TRANSLATION] = modifier.scale_translation == true ? 1f : 0f;
    }

    public bool IsCompleted() {
        if (this.instructions == null) return false;
        if (this.index < this.instructions_size) return false;

        int running = 0;
        for (int i = 0 ; i < this.interpolators_stack_size ; i++) {
            InStackInstruction interpolator = this.interpolators_stack[i];
            if (interpolator == null || interpolator.definition == null) continue;

            running++;
        }

        return running > 0 ? false : true;
    }

    public void ForceEnd(Sprite sprite) {
        // end all pending interpolators
        for (int i = 0 ; i < this.interpolators_stack_size ; i++) {
            InStackInstruction interpolator = this.interpolators_stack[i];
            if (interpolator == null || interpolator.definition == null) continue;

            StateSave(interpolator.definition.property, interpolator.end);

            this.interpolators_stack[i].definition = null;
        }

        // execute all instructions in "one cycle"
        for (; this.index < this.instructions_size ; this.index++) {
            MacroExecutorInstruction instruction = this.instructions[this.index];
            if (instruction == null) continue;

            switch (instruction.type) {
                case AnimMacroType.INTERPOLATOR:
                    InterpolatorStack(instruction, true);
                    break;
                case AnimMacroType.SET:
                    float value = ResolveValue(instruction.value);
                    StateSave(instruction.property, value);
                    break;
                case AnimMacroType.YIELD:
                case AnimMacroType.PAUSE:
                    continue;
                case AnimMacroType.RESET:
                    StateClear();
                    ClearRegisters();
                    ApplyFrame(null);
                    break;
                case AnimMacroType.RANDOM_SETUP:
                    this.random_start = ResolveValue(instruction.start);
                    this.random_end = ResolveValue(instruction.end);
                    break;
                case AnimMacroType.RANDOM_CHOOSE:
                    this.random_value = Math2D.RandomFloatRange(
                        this.random_start, this.random_end
                    );
                    break;
                case AnimMacroType.RANDOM_EXACT:
                    RandomExact(instruction);
                    break;
                case AnimMacroType.REGISTER_PROP:
                case AnimMacroType.REGISTER_SET:
                    WriteRegister(instruction);
                    break;
            }
        }

        this.sleep_time = 0.0f;
        this.on_yield = false;

        if (sprite != null)
            StateApply(sprite, true);
    }

    public MacroExecutor Clone(bool clone_frames) {
        //if (!this) return null;

        MacroExecutor copy = new MacroExecutor() {
            interpolators_stack_size = this.interpolators_stack_size,
            instructions_size = this.instructions_size,
            frames = this.frames,
            frame_count = this.frame_count,
            frames_dispose = this.frames_dispose,
            random_start = this.random_start,
            random_end = this.random_end,
            random_value = this.random_value,
            speed = this.speed,
            registers = this.registers,
            interpolators_stack = this.interpolators_stack,
            instructions = this.instructions,
            index = this.index,
            on_yield = this.on_yield,
            sleep_time = this.sleep_time,
            frame_index_restart = this.frame_index_restart,
            frame_allow_change = this.frame_allow_change,
            state = this.state,
            state_flags = this.state_flags,
            last_step = this.last_step
        };
        //if (!copy) return null;

        if (clone_frames) {
            copy.frames_dispose = true;
            copy.frames = CloneUtils.CloneArray(copy.frames, copy.frame_count);
        } else {
            copy.frame_count = 0;
            copy.frames = null;
        }

        copy.interpolators_stack = new InStackInstruction[copy.interpolators_stack_size];
        for (int i = 0 ; i < copy.interpolators_stack_size ; i++) {
            copy.interpolators_stack[i] = new InStackInstruction() {
                definition = this.interpolators_stack[i].definition,
                end = this.interpolators_stack[i].end,
                duration = this.interpolators_stack[i].duration,
                steps_bounds = this.interpolators_stack[i].steps_bounds,
                progress = this.interpolators_stack[i].progress,
                start = this.interpolators_stack[i].start,
                steps_count = this.interpolators_stack[i].steps_count,
                steps_method = this.interpolators_stack[i].steps_method,
            };
        }

        copy.instructions = CloneUtils.CloneArray(copy.instructions, copy.instructions_size);

        // C# and JS only
        copy.registers = CloneUtils.CloneArray(copy.registers, MACROEXECUTOR_REGISTER_COUNT);
        copy.state = CloneUtils.CloneArray(copy.state, MACROEXECUTOR_VERTEX_PROP_COUNT);
        copy.state_flags = CloneUtils.CloneArray(copy.state_flags, MACROEXECUTOR_VERTEX_PROP_COUNT);

        return copy;
    }

    public float StatePeek(int property, float default_value) {
        if (property < 0 || property >= MACROEXECUTOR_VERTEX_PROP_COUNT)
            throw new ArgumentOutOfRangeException("property", "macroexecutor_state_peek() invalid property enumeration");

        if (this.state_flags[property])
            return default_value;
        else
            return this.state[property];
    }

    public void RegistersWrite(int id, float value) {
        if (id < 0 || id >= MACROEXECUTOR_REGISTER_COUNT) return;
        if (Single.IsNaN(value)) return;
        this.registers[id] = value;
    }

    public float RegistersRead(int id) {
        if (id < 0 || id >= MACROEXECUTOR_REGISTER_COUNT) return Single.NaN;
        return this.registers[id];
    }




    ///////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////// private functions ////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////


    private void ClearFlags() {
        for (int i = 0 ; i < MACROEXECUTOR_VERTEX_PROP_COUNT ; i++)
            this.state_flags[i] = false;
    }


    private void StateClear() {
        for (int i = 0 ; i < MACROEXECUTOR_VERTEX_PROP_COUNT ; i++) {
            float value;
            switch (i) {
                case VertexProps.SPRITE_PROP_SCALE_X:
                case VertexProps.SPRITE_PROP_SCALE_Y:
                case VertexProps.SPRITE_PROP_ALPHA:
                    value = 1.0f;
                    break;
                default:
                    value = 0.0f;
                    break;
            }
            this.state[i] = value;
        }
        ClearFlags();
    }

    private float StateLoad(int property) {
        return this.state[property];
    }

    private void StateSave(int property, float value) {
        switch (property) {
            case VertexProps.SPRITE_PROP_ALPHA:
            case VertexProps.SPRITE_PROP_VERTEX_COLOR_R:
            case VertexProps.SPRITE_PROP_VERTEX_COLOR_G:
            case VertexProps.SPRITE_PROP_VERTEX_COLOR_B:
                value = Math2D.Clamp(value, 0.0f, 1.0f);
                break;
            case VertexProps.SPRITE_PROP_FRAMEINDEX:
                value = (int)Math2D.Clamp(value, 0, this.frame_count);
                ApplyFrame(null);
                break;
        }
        this.state[property] = value;
        this.state_flags[property] = true;
    }


    private void ApplyFrame(Sprite sprite) {
        if (this.frame_count < 1) return;

        float[] state = this.state;
        int frame_index = (int)state[VertexProps.SPRITE_PROP_FRAMEINDEX];

        AtlasEntry frame = this.frames[frame_index];

        if (this.frame_allow_change) {
            state[VertexProps.SPRITE_PROP_WIDTH] = frame.frame_width > 0 ? frame.frame_width : frame.width;
            state[VertexProps.SPRITE_PROP_HEIGHT] = frame.frame_height > 0 ? frame.frame_height : frame.height;

            if (sprite != null)
                sprite.SetDrawSize(state[VertexProps.SPRITE_PROP_WIDTH], state[VertexProps.SPRITE_PROP_HEIGHT]);
        }

        if (sprite == null) return;
        sprite.SetOffsetSource(frame.x, frame.y, frame.width, frame.height);
        sprite.SetOffsetFrame(frame.frame_x, frame.frame_y, frame.frame_width, frame.frame_height);
    }

    private void ApplyFrame2(StateSprite statesprite) {
        if (this.frame_count < 1) return;

        float[] state = this.state;
        int frame_index = (int)state[VertexProps.SPRITE_PROP_FRAMEINDEX];

        AtlasEntry frame = this.frames[frame_index];

        if (this.frame_allow_change) {
            state[VertexProps.SPRITE_PROP_WIDTH] = frame.frame_width > 0 ? frame.frame_width : frame.width;
            state[VertexProps.SPRITE_PROP_HEIGHT] = frame.frame_height > 0 ? frame.frame_height : frame.height;

            if (statesprite != null)
                statesprite.SetDrawSize(state[VertexProps.SPRITE_PROP_WIDTH], state[VertexProps.SPRITE_PROP_HEIGHT]);
        }

        if (statesprite == null) return;
        statesprite.SetOffsetSource(
            frame.x, frame.y, frame.width, frame.height
        );
        statesprite.SetOffsetFrame(
            frame.frame_x, frame.frame_y, frame.frame_width, frame.frame_height
        );
    }

    private float ResolveValue(MacroExecutorValue value) {
        switch (value.kind) {
            case MacroExecutorValueKind.LITERAL:
                return value.literal;
            case MacroExecutorValueKind.PROPERTY:
                return this.state[value.reference];
            case MacroExecutorValueKind.SPECIAL:
                break;
        }
        switch (value.reference) {
            case ANIM_MACRO_SPECIAL_RANDOM:
                return this.random_value;
            case ANIM_MACRO_SPECIAL_REGISTER0:
                return this.registers[0];
            case ANIM_MACRO_SPECIAL_REGISTER1:
                return this.registers[1];
            case ANIM_MACRO_SPECIAL_REGISTER2:
                return this.registers[2];
            case ANIM_MACRO_SPECIAL_REGISTER3:
                return this.registers[3];
        }

        //this never should happen
        return 0f;
    }


    private void InterpolatorStack(MacroExecutorInstruction instruction, bool inmediate) {
        int i = 0;

        if (!inmediate) {
            for (; i < this.interpolators_stack_size ; i++) {
                InStackInstruction interpolator = this.interpolators_stack[i];
                if (interpolator.definition != null) continue;
                break;
            }
            if (i >= this.interpolators_stack_size) return;
        }

        float actual_value = StateLoad(instruction.property);

        float start = ResolveValue(instruction.start);
        float end = ResolveValue(instruction.end);
        float duration = ResolveValue(instruction.duration);
        float steps_count = ResolveValue(instruction.steps_count);
        float steps_method = ResolveValue(instruction.steps_method);

        if (Single.IsNaN(start)) start = actual_value;
        if (Single.IsNaN(end)) end = actual_value;

        if (inmediate) {
            StateSave(instruction.property, end);
            return;
        }

        this.interpolators_stack[i].definition = instruction;
        this.interpolators_stack[i].duration = duration;
        this.interpolators_stack[i].steps_bounds[0] = 0.0f;
        this.interpolators_stack[i].steps_bounds[1] = 0.0f;
        this.interpolators_stack[i].steps_bounds[2] = 0.0f;
        this.interpolators_stack[i].progress = 0.0f;
        this.interpolators_stack[i].start = start;
        this.interpolators_stack[i].end = end;
        this.interpolators_stack[i].steps_count = (int)steps_count;
        this.interpolators_stack[i].steps_method = (Align)((int)steps_method);

        InterpolatorRun(0.0f, this.interpolators_stack[i]);
    }

    private int InterpolatorsCheck(float elapsed) {
        int running = 0;

        for (int i = 0 ; i < this.interpolators_stack_size ; i++) {
            InStackInstruction interpolator = this.interpolators_stack[i];
            if (interpolator.definition == null) continue;

            bool done = InterpolatorRun(elapsed, interpolator);

            if (done)
                this.interpolators_stack[i].definition = null;
            else
                running++;
        }

        return running;
    }

    private bool InterpolatorRun(float elapsed, InStackInstruction interpolator) {
        float percent = (float)Math2D.InverseLerp(0, interpolator.duration, interpolator.progress);

        interpolator.progress += elapsed;
        bool completed = interpolator.progress >= interpolator.duration;
        if (completed || percent > 1.0) percent = 1;

        float value = 0;
        switch (interpolator.definition.interpolator) {
            case AnimInterpolator.EASE:
                value = CalcCubicBezier(percent, CUBIC_BREZIER_EASE);
                break;
            case AnimInterpolator.EASE_IN:
                value = CalcCubicBezier(percent, CUBIC_BREZIER_EASE_IN);
                break;
            case AnimInterpolator.EASE_OUT:
                value = CalcCubicBezier(percent, CUBIC_BREZIER_EASE_OUT);
                break;
            case AnimInterpolator.EASE_IN_OUT:
                value = CalcCubicBezier(percent, CUBIC_BREZIER_EASE_IN_OUT);
                break;
            case AnimInterpolator.LINEAR:
                value = percent;
                break;
            case AnimInterpolator.STEPS:
                value = CalcSteps(
                    percent,
                    interpolator.steps_bounds,
                    interpolator.steps_count,
                    interpolator.steps_method
                );
                break;
            case AnimInterpolator.CUBIC:
                value = Math2D.LerpCubic(percent);
                break;
            case AnimInterpolator.QUAD:
                value = Math2D.LerpQuad(percent);
                break;
            case AnimInterpolator.EXPO:
                value = Math2D.LerpExpo(percent);
                break;
            case AnimInterpolator.SIN:
                value = Math2D.LerpSin(percent);
                break;
        }


        if (Single.IsInfinity(value) || Single.IsNaN(value))
            throw new Exception("macroexecutor: infinite/NaN value found");

        // commit interpolation in the specific range
        value = Math2D.Lerp(interpolator.start, interpolator.end, value);

        StateSave(interpolator.definition.property, value);
        return completed;
    }



    public static float CalcCubicBezier(float elapsed_time, float[] points) {
        float input = Math2D.CubicBezier(elapsed_time, 0.0f, points[0], points[1], 1.0f);
        float output = Math2D.CubicBezier(elapsed_time, 0.0f, points[2], points[3], 1.0f);

        if (input == 0.0f || output == 0.0f) return 0.0f;// avoid NaN and division by zero exceptions

        float res = (elapsed_time * input) / output;
        //res = Math2D.Clamp(res, 0.0f, 1.0f);

        return res;
    }

    public static float CalcSteps(float elapsed_time, float[] bounds, int count, Align direction) {
        // initialize steps bounds, this is done once
        if (bounds[0] == 0.0f) {
            // workaround (tested in "none" direction only)
            count++;

            float step_in = 1.0f / count;
            float offset = direction == Align.BOTH ? 1f : -1f;
            float step_out = 1.0f / (count + offset);
            float step_mul = direction == Align.END ? step_in : step_out;

            bounds[0] = step_in;
            bounds[1] = step_out;
            bounds[2] = step_mul;
        }

        elapsed_time = Math2D.NearestDown(elapsed_time, bounds[0]);

        float res;
        if (direction == Align.START) {
            res = elapsed_time + bounds[0];
        } else {
            res = (elapsed_time * bounds[2]) / bounds[0];
            if (direction == Align.BOTH) res += bounds[1];
        }

        return Math2D.Clamp(res, 0.0f, 1.0f);
    }

    public static float CalcSwing(float percent) {
        return (float)(0.5 - Math.Cos(percent * Math.PI) / 2.0);
    }

    public static float CalcLog(float percent) {
        float value = (float)(Math.Log(percent * 100f) / Math2D.LOG100);
        return value < 0f ? 0f : value;
    }


    private void WriteRegister(MacroExecutorInstruction instruction) {
        // resolve the value to store
        float value;
        if (instruction.type == AnimMacroType.REGISTER_PROP)
            value = this.state[instruction.property];
        else
            value = ResolveValue(instruction.value);

        if (instruction.register_index == -1) {
            // put the value in all registers
            for (int i = 0 ; i < MACROEXECUTOR_REGISTER_COUNT ; i++)
                this.registers[i] = value;
            return;
        }

        // store the value in a specific register
        this.registers[instruction.register_index] = value;
    }

    private void ClearRegisters() {
        for (int i = 0 ; i < MACROEXECUTOR_REGISTER_COUNT ; i++)
            this.registers[i] = 0.0f;
    }

    private void RandomExact(MacroExecutorInstruction instruction) {
        if (instruction.values_size < 1) {
            // this never should happen
            this.random_value = 0f;
            return;
        }

        int index = Math2D.RandomInt(0, instruction.values_size);
        MacroExecutorValue item = instruction.values[index];

        // resolve the item value
        this.random_value = ResolveValue(item);
    }


    private class InStackInstruction {
        public MacroExecutorInstruction definition;

        public float end;
        public float duration;
        public float[] steps_bounds;
        public double progress;
        public float start;
        public int steps_count;
        public Align steps_method;

        public InStackInstruction() {
            this.steps_bounds = new float[3];
        }
    }

}
