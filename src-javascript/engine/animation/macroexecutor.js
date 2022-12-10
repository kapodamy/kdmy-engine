"use strict";

const CUBIC_BREZIER_EASE_LINEAR = [0.0, 0.0, 1.0, 1.0];
const CUBIC_BREZIER_EASE = [0.25, 0.1, 0.25, 1.0];
const CUBIC_BREZIER_EASE_IN = [0.42, 0.0, 1.0, 1.0];
const CUBIC_BREZIER_EASE_OUT = [0.0, 0.0, 0.58, 1.0];
const CUBIC_BREZIER_EASE_IN_OUT = [0.42, 0.0, 0.58, 1.0];

const ANIM_MACRO_SPECIAL_REGISTER0 = 0;
const ANIM_MACRO_SPECIAL_REGISTER1 = 1;
const ANIM_MACRO_SPECIAL_REGISTER2 = 2;
const ANIM_MACRO_SPECIAL_REGISTER3 = 3;
const ANIM_MACRO_SPECIAL_RANDOM = 9;

const ANIM_MACRO_VALUE_KIND_PROPERTY = 0;
const ANIM_MACRO_VALUE_KIND_SPECIAL = 1;
const ANIM_MACRO_VALUE_KIND_LITERAL = 2;

/** count of "SPRITE_PROP_* + TEXTSPRITE_PROP_* + MEDIA_PROP_* + FONT_PROP_* + LAYOUT_PROP_*" */
const MACROEXECUTOR_VERTEX_PROP_COUNT = 67;
const MACROEXECUTOR_REGISTER_COUNT = 4;// this must match ANIM_MACRO_SPECIAL_REGISTER* count


function macroexecutor_init(instructions, instructions_size, frames, frame_count) {
    let macroexecutor = {};
    macroexecutor.interpolators_stack_size = 0;
    macroexecutor.instructions_size = instructions_size;

    macroexecutor.frames = frames;
    macroexecutor.frame_count = frame_count;
    macroexecutor.frames_dispose = 0;

    macroexecutor.random_start = 0;
    macroexecutor.random_end = 1.0;
    macroexecutor.random_value = 0.5;
    macroexecutor.speed = 1.0;
    macroexecutor.registers = new Array(MACROEXECUTOR_REGISTER_COUNT);

    for (let i = 0; i < instructions_size; i++) {
        if (!instructions[i])
            continue;
        if (instructions[i].type == ANIM_MACRO_INTERPOLATOR)
            macroexecutor.interpolators_stack_size++;
    }

    macroexecutor_clear_registers(macroexecutor);

    macroexecutor.interpolators_stack = new Array(macroexecutor.interpolators_stack_size);
    macroexecutor.instructions = instructions;

    macroexecutor.index = 0;
    macroexecutor.on_yield = 0;
    macroexecutor.sleep_time = 0;

    macroexecutor.frame_index_restart = -1;
    macroexecutor.frame_allow_change = 0;

    macroexecutor.state = new Array(MACROEXECUTOR_VERTEX_PROP_COUNT);
    macroexecutor.state_flags = new Array(MACROEXECUTOR_VERTEX_PROP_COUNT);
    macroexecutor_state_clear(macroexecutor);

    for (let i = 0; i < macroexecutor.interpolators_stack_size; i++)
        macroexecutor.interpolators_stack[i] = null;

    macroexecutor.last_step = 1;

    return macroexecutor;
}

function macroexecutor_destroy(macroexecutor) {
    macroexecutor.interpolators_stack = undefined;
    macroexecutor.instructions = undefined;
    macroexecutor = undefined;
}

function macroexecutor_set_restart_in_frame(macroexecutor, frame_index, allow_change_size) {
    if (frame_index < 0 && frame_index >= macroexecutor.frame_count)
        macroexecutor.frame_index_restart = -1;
    else
        macroexecutor.frame_index_restart = frame_index;

    macroexecutor.frame_allow_change = allow_change_size;
}

function macroexecutor_set_speed(macroexecutor, speed) {
    macroexecutor.speed = speed;
}

function macroexecutor_restart(macroexecutor) {
    macroexecutor.index = 0;
    macroexecutor.on_yield = 0;
    macroexecutor.sleep_time = 0;
    macroexecutor.last_step = 1;

    macroexecutor_state_clear(macroexecutor);

    if (macroexecutor.frame_index_restart >= 0) {
        macroexecutor.state[SPRITE_PROP_FRAMEINDEX] = macroexecutor.frame_index_restart;
        macroexecutor_apply_frame(macroexecutor, null);
    }

    for (let i = 0; i < macroexecutor.interpolators_stack_size; i++)
        macroexecutor.interpolators_stack[i] = null;
}

function macroexecutor_animate(macroexecutor, elapsed) {
    if (macroexecutor.speed != 1.0) elapsed /= macroexecutor.speed;

    let running = macroexecutor_interpolators_check(macroexecutor, elapsed);

    if (macroexecutor.sleep_time > 0.0 || macroexecutor.on_yield) {
        if (macroexecutor.on_yield && running > 0) return;

        if (macroexecutor.sleep_time > 0.0) {
            macroexecutor.sleep_time -= elapsed;
            if (macroexecutor.sleep_time > 0) return;

            elapsed = -macroexecutor.sleep_time;
        }

        // pause/yield instruction completed
        macroexecutor.sleep_time = 0.0;
        macroexecutor.on_yield = 0;
        macroexecutor.index++;
    }

    for (; macroexecutor.index < macroexecutor.instructions_size; macroexecutor.index++) {
        let instruction = macroexecutor.instructions[macroexecutor.index];

        switch (instruction.type) {
            case ANIM_MACRO_INTERPOLATOR:
                running++;
                macroexecutor_interpolator_stack(macroexecutor, instruction, 0);
                break;
            case ANIM_MACRO_SET:
                let value = macroexecutor_resolve_value(macroexecutor, instruction.value);
                macroexecutor_state_save(macroexecutor, instruction.property, value);
                break;
            case ANIM_MACRO_YIELD:
            case ANIM_MACRO_PAUSE:
                macroexecutor.on_yield = instruction.type == ANIM_MACRO_YIELD;
                macroexecutor.sleep_time = macroexecutor_resolve_value(macroexecutor, instruction.value);
                // check for ANIM_MACRO_PAUSE with zero duration 
                if (!macroexecutor.on_yield && macroexecutor.sleep_time < 1) {
                    macroexecutor.sleep_time = 0;
                    macroexecutor.on_yield = 1;
                }
                return 0;
            case ANIM_MACRO_RESET:
                macroexecutor_state_clear(macroexecutor);
                macroexecutor_clear_registers(macroexecutor);
                macroexecutor_apply_frame(macroexecutor, null);
                break;
            case ANIM_MACRO_RANDOM_SETUP:
                macroexecutor.random_start = macroexecutor_resolve_value(macroexecutor, instruction.start);
                macroexecutor.random_end = macroexecutor_resolve_value(macroexecutor, instruction.end);
                break;
            case ANIM_MACRO_RANDOM_CHOOSE:
                macroexecutor.random_value = math2d_random(
                    macroexecutor.random_start,
                    macroexecutor.random_end
                );
                break;
            case ANIM_MACRO_RANDOM_EXACT:
                macroexecutor_random_exact(macroexecutor, instruction);
                break;
            case ANIM_MACRO_REGISTER_PROP:
            case ANIM_MACRO_REGISTER_SET:
                macroexecutor_write_register(macroexecutor, instruction);
                break;
        }
    }


    //return running < 1;
    if (running > 0) return 0;

    if (macroexecutor.last_step) {
        // keep this frame
        macroexecutor.last_step = 0;
        return 0;
    }
    return 1;
}

function macroexecutor_state_apply(macroexecutor, sprite, no_stack_changes) {
    macroexecutor_apply_frame(macroexecutor, sprite);

    for (let i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        if (macroexecutor.state_flags[i])
            sprite_set_property(sprite, i, macroexecutor.state[i]);
    }

    if (no_stack_changes) return;

    macroexecutor_clear_flags(macroexecutor);
}

function macroexecutor_state_apply2(macroexecutor, statesprite, no_stack_changes) {
    macroexecutor_apply_frame2(macroexecutor, statesprite);

    for (let i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        if (macroexecutor.state_flags[i])
            statesprite_set_property(statesprite, i, macroexecutor.state[i]);
    }

    if (no_stack_changes) return;

    macroexecutor_clear_flags(macroexecutor);
}

function macroexecutor_state_apply3(macroexecutor, drawable, no_stack_changes) {
    for (let i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        if (macroexecutor.state_flags[i])
            drawable_set_property(drawable, i, macroexecutor.state[i]);
    }

    if (no_stack_changes) return;

    macroexecutor_clear_flags(macroexecutor);
}

function macroexecutor_state_apply4(macroexecutor, textsprite, no_stack_changes) {
    for (let i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        if (macroexecutor.state_flags[i])
            textsprite_set_property(textsprite, i, macroexecutor.state[i]);
    }

    if (no_stack_changes) return;

    macroexecutor_clear_flags(macroexecutor);
}

function macroexecutor_state_apply5(macroexecutor, private_data, setter_callback, no_stack_changes) {
    for (let i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        if (macroexecutor.state_flags[i])
            setter_callback(private_data, i, macroexecutor.state[i]);
    }

    if (no_stack_changes) return;

    macroexecutor_clear_flags(macroexecutor);
}

function macroexecutor_state_apply_minimal(macroexecutor, sprite) {
    const MACROEXECUTOR_MINIMAL_PROPS = [
        SPRITE_PROP_FRAMEINDEX, SPRITE_PROP_ALPHA,
        SPRITE_PROP_VERTEX_COLOR_R, SPRITE_PROP_VERTEX_COLOR_G, SPRITE_PROP_VERTEX_COLOR_B
    ];

    for (let i = 0; i < MACROEXECUTOR_MINIMAL_PROPS.length; i++) {
        if (macroexecutor.state_flags[i])
            sprite_set_property(sprite, i, macroexecutor.state[i]);
    }
}

function macroexecutor_state_to_modifier(macroexecutor, modifier, no_stack_changes) {
    for (let i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        if (macroexecutor.state_flags[i])
            pvrctx_helper_set_modifier_property(modifier, i, macroexecutor.state[i]);
    }

    if (no_stack_changes) return;

    macroexecutor_clear_flags(macroexecutor);
}

function macroexecutor_state_from_modifier(macroexecutor, modifier) {
    macroexecutor.state[SPRITE_PROP_ROTATE] = modifier.rotate;
    macroexecutor.state[SPRITE_PROP_SCALE_X] = modifier.scale_x;
    macroexecutor.state[SPRITE_PROP_SCALE_Y] = modifier.scale_y;
    macroexecutor.state[SPRITE_PROP_TRANSLATE_X] = modifier.translate_x;
    macroexecutor.state[SPRITE_PROP_TRANSLATE_Y] = modifier.translate_y;
    macroexecutor.state[SPRITE_PROP_SKEW_X] = modifier.skew_x;
    macroexecutor.state[SPRITE_PROP_SKEW_Y] = modifier.skew_y;
    macroexecutor.state[SPRITE_PROP_ROTATE_PIVOT_ENABLE] = modifier.rotate_pivot_enabled;
    macroexecutor.state[SPRITE_PROP_ROTATE_PIVOT_U] = modifier.rotate_pivot_u;
    macroexecutor.state[SPRITE_PROP_ROTATE_PIVOT_V] = modifier.rotate_pivot_v;
    macroexecutor.state[SPRITE_PROP_SCALE_DIRECTION_X] = modifier.scale_direction_x;
    macroexecutor.state[SPRITE_PROP_SCALE_DIRECTION_Y] = modifier.scale_direction_y;
    macroexecutor.state[SPRITE_PROP_TRANSLATE_ROTATION] = modifier.translate_rotation;
    macroexecutor.state[SPRITE_PROP_SCALE_SIZE] = modifier.scale_size;
    macroexecutor.state[SPRITE_PROP_SCALE_TRANSLATION] = modifier.scale_translation;
}

function macroexecutor_is_completed(macroexecutor) {
    if (!macroexecutor.instructions) return 0;
    if (macroexecutor.index < macroexecutor.instructions_size) return 0;

    let running = 0;
    for (let i = 0; i < macroexecutor.interpolators_stack_size; i++) {
        let interpolator = macroexecutor.interpolators_stack[i];
        if (!interpolator || !interpolator.definition) continue;

        running++;
    }

    return running > 0 ? 0 : 1;
}

function macroexecutor_force_end(macroexecutor, sprite) {
    // end all pending interpolators
    for (let i = 0; i < macroexecutor.interpolators_stack_size; i++) {
        let interpolator = macroexecutor.interpolators_stack[i];
        if (!interpolator || !interpolator.definition) continue;

        macroexecutor_state_save(macroexecutor, interpolator.definition.property, interpolator.end);

        macroexecutor.interpolators_stack[i].definition = null;
    }

    // execute all instructions in "one cycle"
    for (; macroexecutor.index < macroexecutor.instructions_size; macroexecutor.index++) {
        let instruction = macroexecutor.instructions[macroexecutor.index];
        if (!instruction) continue;

        switch (instruction.type) {
            case ANIM_MACRO_INTERPOLATOR:
                macroexecutor_interpolator_stack(macroexecutor, instruction, 1);
                break;
            case ANIM_MACRO_SET:
                let value = macroexecutor_resolve_value(macroexecutor, instruction.value);
                macroexecutor_state_save(macroexecutor, instruction.property, value);
                break;
            case ANIM_MACRO_YIELD:
            case ANIM_MACRO_PAUSE:
                continue;
            case ANIM_MACRO_RESET:
                macroexecutor_state_clear(macroexecutor);
                macroexecutor_clear_registers(macroexecutor);
                macroexecutor_apply_frame(macroexecutor, null);
                break;
            case ANIM_MACRO_RANDOM_SETUP:
                macroexecutor.random_start = macroexecutor_resolve_value(macroexecutor, instruction.start);
                macroexecutor.random_end = macroexecutor_resolve_value(macroexecutor, instruction.end);
                break;
            case ANIM_MACRO_RANDOM_CHOOSE:
                macroexecutor.random_value = math2d_random(
                    macroexecutor.random_start, macroexecutor.random_end
                );
                break;
            case ANIM_MACRO_RANDOM_EXACT:
                macroexecutor_random_exact(macroexecutor, instruction);
                break;
            case ANIM_MACRO_REGISTER_PROP:
            case ANIM_MACRO_REGISTER_SET:
                macroexecutor_write_register(macroexecutor, instruction);
                break;
        }
    }

    macroexecutor.sleep_time = 0.0;
    macroexecutor.on_yield = 0;

    if (sprite)
        macroexecutor_state_apply(macroexecutor, sprite, 1);
}

function macroexecutor_clone(macroexecutor, clone_frames) {
    if (!macroexecutor) return null;

    let copy = clone_struct(macroexecutor);
    if (!copy) return null;

    if (clone_frames) {
        copy.frames_dispose = 1;
        copy.frames = clone_array(copy.frames, copy.frame_count);
    } else {
        copy.frame_count = 0;
        copy.frames = null;
    }

    copy.interpolators_stack = clone_array(copy.interpolators_stack, copy.interpolators_stack_size);
    copy.instructions = clone_array(copy.instructions, copy.instructions_size);

    // C# and JS only
    copy.registers = clone_array(copy.registers, MACROEXECUTOR_REGISTER_COUNT);
    copy.state = clone_array(copy.state, MACROEXECUTOR_VERTEX_PROP_COUNT);
    copy.state_flags = clone_array(copy.state_flags, MACROEXECUTOR_VERTEX_PROP_COUNT);

    return copy;
}

function macroexecutor_state_peek(macroexecutor, property, default_value) {
    if (property < 0 || property >= MACROEXECUTOR_VERTEX_PROP_COUNT)
        throw new Error("macroexecutor_state_peek() invalid property enumeration");

    if (macroexecutor.state_flags[property])
        return default_value;
    else
        return macroexecutor.state[property];
}

function macroexecutor_registers_write(macroexecutor, id, value) {
    if (!Number.isFinite(id) || id < 0 || id >= MACROEXECUTOR_REGISTER_COUNT) return;
    if (!Number.isFinite(value)) return;
    macroexecutor.registers[Math.trunc(id)] = value;
}

function macroexecutor_registers_read(macroexecutor, id) {
    if (!Number.isFinite(id) || id < 0 || id >= MACROEXECUTOR_REGISTER_COUNT) return;
    return macroexecutor.registers[Math.trunc(id)];
}




///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// private functions ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////


function macroexecutor_clear_flags(macroexecutor) {
    for (let i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++)
        macroexecutor.state_flags[i] = 0;
}


function macroexecutor_state_clear(macroexecutor) {
    for (let i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        let value;
        switch (i) {
            case SPRITE_PROP_SCALE_X:
            case SPRITE_PROP_SCALE_Y:
            case SPRITE_PROP_ALPHA:
                value = 1.0;
                break;
            default:
                value = 0.0;
        }
        macroexecutor.state[i] = value;
    }
    macroexecutor_clear_flags(macroexecutor);
}

function macroexecutor_state_load(macroexecutor, property) {
    return macroexecutor.state[property];
}

function macroexecutor_state_save(macroexecutor, property, value) {
    switch (property) {
        case SPRITE_PROP_ALPHA:
        case SPRITE_PROP_VERTEX_COLOR_R:
        case SPRITE_PROP_VERTEX_COLOR_G:
        case SPRITE_PROP_VERTEX_COLOR_B:
            value = math2d_clamp(value, 0, 1.0);
            break;
        case SPRITE_PROP_FRAMEINDEX:
            value = Math.trunc(math2d_clamp(value, 0, macroexecutor.frame_count - 1));
            macroexecutor_apply_frame(macroexecutor, null);
            break;
    }
    macroexecutor.state[property] = value;
    macroexecutor.state_flags[property] = 1;
}


function macroexecutor_apply_frame(macroexecutor, sprite) {
    if (macroexecutor.frame_count < 1) return;

    let state = macroexecutor.state;
    let frame_index = state[SPRITE_PROP_FRAMEINDEX];

    let frame = macroexecutor.frames[frame_index];

    if (macroexecutor.frame_allow_change) {
        state[SPRITE_PROP_WIDTH] = frame.frame_width > 0 ? frame.frame_width : frame.width;
        state[SPRITE_PROP_HEIGHT] = frame.frame_height > 0 ? frame.frame_height : frame.height;

        if (sprite)
            sprite_set_draw_size(sprite, state[SPRITE_PROP_WIDTH], state[SPRITE_PROP_HEIGHT]);
    }

    if (!sprite) return;
    sprite_set_offset_source(sprite, frame.x, frame.y, frame.width, frame.height);
    sprite_set_offset_frame(sprite, frame.frame_x, frame.frame_y, frame.frame_width, frame.frame_height);
}

function macroexecutor_apply_frame2(macroexecutor, statesprite) {
    if (macroexecutor.frame_count < 1) return;

    let state = macroexecutor.state;
    let frame_index = state[SPRITE_PROP_FRAMEINDEX];

    let frame = macroexecutor.frames[frame_index];

    if (macroexecutor.frame_allow_change) {
        state[SPRITE_PROP_WIDTH] = frame.frame_width > 0 ? frame.frame_width : frame.width;
        state[SPRITE_PROP_HEIGHT] = frame.frame_height > 0 ? frame.frame_height : frame.height;

        if (statesprite)
            statesprite_set_draw_size(statesprite, state[SPRITE_PROP_WIDTH], state[SPRITE_PROP_HEIGHT]);
    }

    if (!statesprite) return;
    statesprite_set_offset_source(
        statesprite, frame.x, frame.y, frame.width, frame.height
    );
    statesprite_set_offset_frame(
        statesprite, frame.frame_x, frame.frame_y, frame.frame_width, frame.frame_height
    );
}

function macroexecutor_resolve_value(macroexecutor, value) {
    switch (value.kind) {
        case ANIM_MACRO_VALUE_KIND_LITERAL:
            return value.literal;
        case ANIM_MACRO_VALUE_KIND_PROPERTY:
            return macroexecutor.state[value.reference];
        case ANIM_MACRO_VALUE_KIND_SPECIAL:
            break;
    }
    switch (value.reference) {
        case ANIM_MACRO_SPECIAL_RANDOM:
            return macroexecutor.random_value;
        case ANIM_MACRO_SPECIAL_REGISTER0:
            return macroexecutor.registers[0];
        case ANIM_MACRO_SPECIAL_REGISTER1:
            return macroexecutor.registers[1];
        case ANIM_MACRO_SPECIAL_REGISTER2:
            return macroexecutor.registers[2];
        case ANIM_MACRO_SPECIAL_REGISTER3:
            return macroexecutor.registers[3];
    }

    //this never should happen
    return 0;
}


function macroexecutor_interpolator_stack(macroexecutor, instruction, inmediate) {
    let i = 0;

    if (!inmediate) {
        for (; i < macroexecutor.interpolators_stack_size; i++) {
            let interpolator = macroexecutor.interpolators_stack[i];
            if (interpolator && interpolator.definition) continue;
            break;
        }
        if (i >= macroexecutor.interpolators_stack_size) return;
    }

    let actual_value = macroexecutor_state_load(macroexecutor, instruction.property);

    let start = macroexecutor_resolve_value(macroexecutor, instruction.start);
    let end = macroexecutor_resolve_value(macroexecutor, instruction.end);
    let duration = macroexecutor_resolve_value(macroexecutor, instruction.duration);
    let steps_count = macroexecutor_resolve_value(macroexecutor, instruction.steps_count);
    let steps_method = macroexecutor_resolve_value(macroexecutor, instruction.steps_method);

    if (Number.isNaN(start)) start = actual_value;
    if (Number.isNaN(end)) end = actual_value;

    if (inmediate) {
        macroexecutor_state_save(macroexecutor, instruction.property, end);
        return;
    }

    macroexecutor.interpolators_stack[i] = {
        definition: instruction,
        duration: duration,
        steps_bounds: [0.0, 0.0, 0.0],
        progress: 0.0,
        start: start,
        end: end,
        steps_count: steps_count,
        steps_method: steps_method
    };

    macroexecutor_interpolator_run(macroexecutor, 0.0, macroexecutor.interpolators_stack[i]);
}

function macroexecutor_interpolators_check(macroexecutor, elapsed) {
    let running = 0;

    for (let i = 0; i < macroexecutor.interpolators_stack_size; i++) {
        let interpolator = macroexecutor.interpolators_stack[i];
        if (!interpolator || !interpolator.definition) continue;

        let done = macroexecutor_interpolator_run(macroexecutor, elapsed, interpolator);

        if (done)
            macroexecutor.interpolators_stack[i].definition = null;
        else
            running++;
    }

    return running;
}

function macroexecutor_interpolator_run(macroexecutor, elapsed, interpolator) {
    let percent = math2d_inverselerp(0, interpolator.duration, interpolator.progress);

    interpolator.progress += elapsed;
    let completed = interpolator.progress >= interpolator.duration;
    if (completed || percent > 1.0) percent = 1;

    let value = 0;
    switch (interpolator.definition.interpolator) {
        case ANIM_MACRO_INTERPOLATOR_EASE:
            value = macroexecutor_calc_cubicbezier(percent, CUBIC_BREZIER_EASE);
            break;
        case ANIM_MACRO_INTERPOLATOR_EASE_IN:
            value = macroexecutor_calc_cubicbezier(percent, CUBIC_BREZIER_EASE_IN);
            break;
        case ANIM_MACRO_INTERPOLATOR_EASE_OUT:
            value = macroexecutor_calc_cubicbezier(percent, CUBIC_BREZIER_EASE_OUT);
            break;
        case ANIM_MACRO_INTERPOLATOR_EASE_IN_OUT:
            value = macroexecutor_calc_cubicbezier(percent, CUBIC_BREZIER_EASE_IN_OUT);
            break;
        case ANIM_MACRO_INTERPOLATOR_LINEAR:
            value = percent;
            break;
        case ANIM_MACRO_INTERPOLATOR_STEPS:
            value = macroexecutor_calc_steps(
                percent,
                interpolator.steps_bounds,
                interpolator.steps_count,
                interpolator.steps_method
            );
            break;
    }


    if (!Number.isFinite(value) || isNaN(value))
        throw new Error("macroexecutor: infinite/NaN value found");

    // commit interpolation in the specific range
    value = math2d_lerp(interpolator.start, interpolator.end, value);

    macroexecutor_state_save(macroexecutor, interpolator.definition.property, value);
    return completed;
}



function macroexecutor_calc_cubicbezier(elapsed_time, points) {
    let input = math2d_cubicbezier(elapsed_time, 0.0, points[0], points[1], 1.0);
    let output = math2d_cubicbezier(elapsed_time, 0.0, points[2], points[3], 1.0);

    if (input == 0.0 || output == 0.0) return 0.0;// avoid NaN and division by zero exceptions

    let res = (elapsed_time * input) / output;
    //res = math2d_clamp(res, 0.0, 1.0);

    return res;
}

function macroexecutor_calc_steps(elapsed_time, bounds, count, direction) {
    // initialize steps bounds, this is done once
    if (bounds[0] == 0.0) {
        // workaround (tested in "none" direction only)
        count++;

        let step_in = 1.0 / count;
        let offset = direction == ALIGN_BOTH ? 1 : -1;
        let step_out = 1.0 / (count + offset);
        let step_mul = direction == ALIGN_END ? step_in : step_out;

        bounds[0] = step_in;
        bounds[1] = step_out;
        bounds[2] = step_mul;
    }

    elapsed_time = math2d_nearestdown(elapsed_time, bounds[0]);

    let res;
    if (direction == ALIGN_START) {
        res = elapsed_time + bounds[0];
    } else {
        res = (elapsed_time * bounds[2]) / bounds[0];
        if (direction == ALIGN_BOTH) res += bounds[1];
    }

    return math2d_clamp(res, 0.0, 1.0);
}

function macroexecutor_calc_swing(percent) {
    return 0.5 - Math.cos(percent * Math.PI) / 2;
}

function macroexecutor_calc_log(percent) {
    let value = Math.log(percent * 100) / MATH2D_LOG100;
    return value < 0 ? 0 : value;
}


function macroexecutor_write_register(macroexecutor, instruction) {
    // resolve the value to store
    let value;
    if (instruction.type == ANIM_MACRO_REGISTER_PROP)
        value = macroexecutor.state[instruction.property];
    else
        value = macroexecutor_resolve_value(macroexecutor, instruction.value);

    if (instruction.register_index == -1) {
        // put the value in all registers
        for (let i = 0; i < MACROEXECUTOR_REGISTER_COUNT; i++)
            macroexecutor.registers[i] = value;
        return;
    }

    // store the value in a specific register
    macroexecutor.registers[instruction.register_index] = value;
}

function macroexecutor_clear_registers(macroexecutor) {
    for (let i = 0; i < MACROEXECUTOR_REGISTER_COUNT; i++)
        macroexecutor.registers[i] = 0.0;
}

function macroexecutor_random_exact(macroexecutor, instruction) {
    if (instruction.values_size < 1) {
        // this never should happen
        macroexecutor.random_value = 0;
        return;
    }

    let index = math2d_random_int(0, instruction.values_size - 1);
    let item = instruction.values[index];

    // resolve the item value
    macroexecutor.random_value = macroexecutor_resolve_value(macroexecutor, item);
}

