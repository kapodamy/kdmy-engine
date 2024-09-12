#include <string.h>

#include "atlas.h"
#include "cloneutils.h"
#include "float64.h"
#include "logger.h"
#include "macroexecutor.h"
#include "malloc_utils.h"
#include "math.h"
#include "vertexprops.h"


/** count of "SPRITE_PROP_* + TEXTSPRITE_PROP_* + MEDIA_PROP_* + FONT_PROP_* + LAYOUT_PROP_* + CAMERA_PROP_*" */
#define MACROEXECUTOR_VERTEX_PROP_COUNT 74


typedef struct {
    MacroExecutorInstruction* definition;

    float end;
    float duration;
    float steps_bounds[3];
    float64 progress;
    float start;
    int32_t steps_count;
    Align steps_method;
} InStackInstruction;

struct MacroExecutor_s {
    int32_t interpolators_stack_size;
    int32_t instructions_size;
    AtlasEntry* frames;
    int32_t frame_count;
    float random_start;
    float random_end;
    float random_value;
    float speed;
    float registers[MACROEXECUTOR_REGISTER_COUNT];
    InStackInstruction* interpolators_stack;
    MacroExecutorInstruction* instructions;
    int32_t index;
    bool on_yield;
    float sleep_time;
    int32_t frame_index_restart;
    bool frame_allow_change;
    float state[MACROEXECUTOR_VERTEX_PROP_COUNT];
    bool state_flags[MACROEXECUTOR_VERTEX_PROP_COUNT];
    bool last_step;
};


static inline void macroexecutor_clear_flags(MacroExecutor macroexecutor);
static void macroexecutor_state_clear(MacroExecutor macroexecutor);
static inline float macroexecutor_state_load(MacroExecutor macroexecutor, int32_t property);
static void macroexecutor_state_save(MacroExecutor macroexecutor, int32_t property, float value);
static void macroexecutor_apply_frame(MacroExecutor macroexecutor, Sprite sprite);
static void macroexecutor_apply_frame2(MacroExecutor macroexecutor, StateSprite statesprite);
static float macroexecutor_resolve_value(MacroExecutor macroexecutor, MacroExecutorValue* value);
static void macroexecutor_interpolator_stack(MacroExecutor macroexecutor, MacroExecutorInstruction* instruction, bool inmediate);
static int32_t macroexecutor_interpolators_check(MacroExecutor macroexecutor, float elapsed);
static bool macroexecutor_interpolator_run(MacroExecutor macroexecutor, float elapsed, InStackInstruction* interpolator);
static void macroexecutor_write_register(MacroExecutor macroexecutor, MacroExecutorInstruction* instruction);
static void macroexecutor_clear_registers(MacroExecutor macroexecutor);
static void macroexecutor_random_exact(MacroExecutor macroexecutor, MacroExecutorInstruction* instruction);


const float CUBIC_BREZIER_EASE_LINEAR[4] = {0.0f, 0.0f, 1.0f, 1.0f};
const float CUBIC_BREZIER_EASE[4] = {0.25f, 0.1f, 0.25f, 1.0f};
const float CUBIC_BREZIER_EASE_IN[4] = {0.42f, 0.0f, 1.0f, 1.0f};
const float CUBIC_BREZIER_EASE_OUT[4] = {0.0f, 0.0f, 0.58f, 1.0f};
const float CUBIC_BREZIER_EASE_IN_OUT[4] = {0.42f, 0.0f, 0.58f, 1.0f};


MacroExecutor macroexecutor_init(const AnimListItem* animlist_item) {
    MacroExecutor macroexecutor = malloc_chk(sizeof(struct MacroExecutor_s));
    malloc_assert(macroexecutor, MacroExecutor);

    macroexecutor->interpolators_stack_size = 0;
    macroexecutor->instructions = CLONE_STRUCT_ARRAY(MacroExecutorInstruction, animlist_item->instructions, animlist_item->instructions_count);
    macroexecutor->instructions_size = animlist_item->instructions_count;

    macroexecutor->frames = CLONE_STRUCT_ARRAY(AtlasEntry, animlist_item->frames, animlist_item->frame_count);
    macroexecutor->frame_count = animlist_item->frame_count;

    macroexecutor->random_start = 0.0f;
    macroexecutor->random_end = 1.0f;
    macroexecutor->random_value = 0.5f;
    macroexecutor->speed = 1.0f;
    memset(macroexecutor->registers, 0x00, sizeof(macroexecutor->registers));

    for (int32_t i = 0; i < macroexecutor->instructions_size; i++) {
        MacroExecutorInstruction* instruction = &macroexecutor->instructions[i];

        // clone values array
        instruction->values = CLONE_STRUCT_ARRAY(MacroExecutorValue, instruction->values, instruction->values_size);

        if (animlist_item->instructions[i].type == AnimMacroType_INTERPOLATOR)
            macroexecutor->interpolators_stack_size++;
    }

    macroexecutor_clear_registers(macroexecutor);

    macroexecutor->interpolators_stack = calloc_for_array(macroexecutor->interpolators_stack_size, InStackInstruction);

    macroexecutor->index = 0;
    macroexecutor->on_yield = false;
    macroexecutor->sleep_time = 0.0f;

    macroexecutor->frame_index_restart = -1;
    macroexecutor->frame_allow_change = false;

    macroexecutor_state_clear(macroexecutor);
    macroexecutor->last_step = true;

    return macroexecutor;
}

void macroexecutor_destroy(MacroExecutor* macroexecutor_ptr) {
    MacroExecutor macroexecutor = *macroexecutor_ptr;
    if (!macroexecutor) return;

    for (int32_t i = 0; i < macroexecutor->instructions_size; i++) {
        free_chk(macroexecutor->instructions[i].values);
    }

    free_chk(macroexecutor->interpolators_stack);
    free_chk(macroexecutor->frames);
    free_chk(macroexecutor->instructions);

    free_chk(macroexecutor);
    *macroexecutor_ptr = NULL;
}

void macroexecutor_set_restart_in_frame(MacroExecutor macroexecutor, int32_t frame_index, bool allow_change_size) {
    if (frame_index < 0 && frame_index >= macroexecutor->frame_count)
        macroexecutor->frame_index_restart = -1;
    else
        macroexecutor->frame_index_restart = frame_index;

    macroexecutor->frame_allow_change = allow_change_size;
}

void macroexecutor_set_speed(MacroExecutor macroexecutor, float speed) {
    macroexecutor->speed = speed;
}

void macroexecutor_restart(MacroExecutor macroexecutor) {
    macroexecutor->index = 0;
    macroexecutor->on_yield = false;
    macroexecutor->sleep_time = 0.0f;
    macroexecutor->last_step = true;

    macroexecutor_state_clear(macroexecutor);

    if (macroexecutor->frame_index_restart >= 0) {
        macroexecutor->state[SPRITE_PROP_FRAMEINDEX] = macroexecutor->frame_index_restart;
        macroexecutor_apply_frame(macroexecutor, NULL);
    }

    for (int32_t i = 0; i < macroexecutor->interpolators_stack_size; i++)
        macroexecutor->interpolators_stack[i].definition = NULL;
}

int32_t macroexecutor_animate(MacroExecutor macroexecutor, float elapsed) {
    if (macroexecutor->speed != 1.0f) elapsed /= macroexecutor->speed;

    int32_t running = macroexecutor_interpolators_check(macroexecutor, elapsed);

    if (macroexecutor->sleep_time > 0.0f || macroexecutor->on_yield) {
        if (macroexecutor->on_yield && running > 0) return 0;

        if (macroexecutor->sleep_time > 0.0f) {
            macroexecutor->sleep_time -= elapsed;
            if (macroexecutor->sleep_time > 0) return 0;

            elapsed = -macroexecutor->sleep_time;
        }

        // pause/yield instruction completed
        macroexecutor->sleep_time = 0.0f;
        macroexecutor->on_yield = false;
        macroexecutor->index++;
    }

    for (; macroexecutor->index < macroexecutor->instructions_size; macroexecutor->index++) {
        MacroExecutorInstruction* instruction = &macroexecutor->instructions[macroexecutor->index];

        switch (instruction->type) {
            case AnimMacroType_INTERPOLATOR:
                running++;
                macroexecutor_interpolator_stack(macroexecutor, instruction, 0);
                break;
            case AnimMacroType_SET:
                float value = macroexecutor_resolve_value(macroexecutor, &instruction->value);
                macroexecutor_state_save(macroexecutor, instruction->property, value);
                break;
            case AnimMacroType_YIELD:
            case AnimMacroType_PAUSE:
                macroexecutor->on_yield = instruction->type == AnimMacroType_YIELD;
                macroexecutor->sleep_time = macroexecutor_resolve_value(macroexecutor, &instruction->value);
                // check for AnimMacroType_PAUSE with zero duration
                if (!macroexecutor->on_yield && macroexecutor->sleep_time <= 0.0f) {
                    macroexecutor->sleep_time = 0.0f;
                    macroexecutor->on_yield = true;
                }
                return 0;
            case AnimMacroType_RESET:
                macroexecutor_state_clear(macroexecutor);
                macroexecutor_clear_registers(macroexecutor);
                macroexecutor_apply_frame(macroexecutor, NULL);
                break;
            case AnimMacroType_RANDOM_SETUP:
                macroexecutor->random_start = macroexecutor_resolve_value(macroexecutor, &instruction->start);
                macroexecutor->random_end = macroexecutor_resolve_value(macroexecutor, &instruction->end);
                break;
            case AnimMacroType_RANDOM_CHOOSE:
                macroexecutor->random_value = math2d_random_float_range(
                    macroexecutor->random_start,
                    macroexecutor->random_end
                );
                break;
            case AnimMacroType_RANDOM_EXACT:
                macroexecutor_random_exact(macroexecutor, instruction);
                break;
            case AnimMacroType_REGISTER_PROP:
            case AnimMacroType_REGISTER_SET:
                macroexecutor_write_register(macroexecutor, instruction);
                break;
        }
    }


    // return running < 1;
    if (running > 0) return 0;

    if (macroexecutor->last_step) {
        // keep this frame
        macroexecutor->last_step = 0;
        return 0;
    }
    return 1;
}

void macroexecutor_state_apply(MacroExecutor macroexecutor, Sprite sprite, bool no_stack_changes) {
    macroexecutor_apply_frame(macroexecutor, sprite);

    for (int32_t i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        if (macroexecutor->state_flags[i])
            sprite_set_property(sprite, i, macroexecutor->state[i]);
    }

    if (no_stack_changes) return;

    macroexecutor_clear_flags(macroexecutor);
}

void macroexecutor_state_apply2(MacroExecutor macroexecutor, StateSprite statesprite, bool no_stack_changes) {
    macroexecutor_apply_frame2(macroexecutor, statesprite);

    for (int32_t i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        if (macroexecutor->state_flags[i])
            statesprite_set_property(statesprite, i, macroexecutor->state[i]);
    }

    if (no_stack_changes) return;

    macroexecutor_clear_flags(macroexecutor);
}

void macroexecutor_state_apply3(MacroExecutor macroexecutor, Drawable drawable, bool no_stack_changes) {
    for (int32_t i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        if (macroexecutor->state_flags[i])
            drawable_set_property(drawable, i, macroexecutor->state[i]);
    }

    if (no_stack_changes) return;

    macroexecutor_clear_flags(macroexecutor);
}

void macroexecutor_state_apply4(MacroExecutor macroexecutor, TextSprite textsprite, bool no_stack_changes) {
    for (int32_t i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        if (macroexecutor->state_flags[i])
            textsprite_set_property(textsprite, i, macroexecutor->state[i]);
    }

    if (no_stack_changes) return;

    macroexecutor_clear_flags(macroexecutor);
}

void macroexecutor_state_apply5(MacroExecutor macroexecutor, void* private_data, PropertySetter setter_callback, bool no_stack_changes) {
    for (int32_t i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        if (macroexecutor->state_flags[i])
            setter_callback(private_data, i, macroexecutor->state[i]);
    }

    if (no_stack_changes) return;

    macroexecutor_clear_flags(macroexecutor);
}

void macroexecutor_state_apply_minimal(MacroExecutor macroexecutor, Sprite sprite) {
    static const int32_t MACROEXECUTOR_MINIMAL_PROPS[] = {
        SPRITE_PROP_FRAMEINDEX, SPRITE_PROP_ALPHA,
        SPRITE_PROP_VERTEX_COLOR_R, SPRITE_PROP_VERTEX_COLOR_G, SPRITE_PROP_VERTEX_COLOR_B
    };
    static const int32_t MACROEXECUTOR_MINIMAL_PROPS_LENGTH = sizeof(MACROEXECUTOR_MINIMAL_PROPS) / sizeof(int32_t);

    for (int32_t i = 0; i < MACROEXECUTOR_MINIMAL_PROPS_LENGTH; i++) {
        if (macroexecutor->state_flags[i])
            sprite_set_property(sprite, i, macroexecutor->state[i]);
    }
}

void macroexecutor_state_to_modifier(MacroExecutor macroexecutor, Modifier* modifier, bool no_stack_changes) {
    for (int32_t i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        if (macroexecutor->state_flags[i])
            pvr_context_helper_set_modifier_property(modifier, i, macroexecutor->state[i]);
    }

    if (no_stack_changes) return;

    macroexecutor_clear_flags(macroexecutor);
}

void macroexecutor_state_from_modifier(MacroExecutor macroexecutor, Modifier* modifier) {
    macroexecutor->state[SPRITE_PROP_ROTATE] = modifier->rotate;
    macroexecutor->state[SPRITE_PROP_SCALE_X] = modifier->scale_x;
    macroexecutor->state[SPRITE_PROP_SCALE_Y] = modifier->scale_y;
    macroexecutor->state[SPRITE_PROP_TRANSLATE_X] = modifier->translate_x;
    macroexecutor->state[SPRITE_PROP_TRANSLATE_Y] = modifier->translate_y;
    macroexecutor->state[SPRITE_PROP_SKEW_X] = modifier->skew_x;
    macroexecutor->state[SPRITE_PROP_SKEW_Y] = modifier->skew_y;
    macroexecutor->state[SPRITE_PROP_ROTATE_PIVOT_ENABLED] = modifier->rotate_pivot_enabled == true ? 1.0f : 0.0f;
    macroexecutor->state[SPRITE_PROP_ROTATE_PIVOT_U] = modifier->rotate_pivot_u;
    macroexecutor->state[SPRITE_PROP_ROTATE_PIVOT_V] = modifier->rotate_pivot_v;
    macroexecutor->state[SPRITE_PROP_SCALE_DIRECTION_X] = modifier->scale_direction_x;
    macroexecutor->state[SPRITE_PROP_SCALE_DIRECTION_Y] = modifier->scale_direction_y;
    macroexecutor->state[SPRITE_PROP_TRANSLATE_ROTATION] = modifier->translate_rotation == true ? 1.0f : 0.0f;
    macroexecutor->state[SPRITE_PROP_SCALE_SIZE] = modifier->scale_size == true ? 1.0f : 0.0f;
    macroexecutor->state[SPRITE_PROP_SCALE_TRANSLATION] = modifier->scale_translation == true ? 1.0f : 0.0f;
}

bool macroexecutor_is_completed(MacroExecutor macroexecutor) {
    if (!macroexecutor->instructions) return false;
    if (macroexecutor->index < macroexecutor->instructions_size) return false;

    for (int32_t i = 0; i < macroexecutor->interpolators_stack_size; i++) {
        if (!macroexecutor->interpolators_stack[i].definition) continue;
        return false;
    }

    return true;
}

void macroexecutor_force_end(MacroExecutor macroexecutor, Sprite sprite) {
    // end all pending interpolators
    for (int32_t i = 0; i < macroexecutor->interpolators_stack_size; i++) {
        InStackInstruction* stack_entry = &macroexecutor->interpolators_stack[i];
        if (!stack_entry->definition) continue;

        macroexecutor_state_save(macroexecutor, stack_entry->definition->property, stack_entry->end);
        stack_entry->definition = NULL;
    }

    // execute all instructions in "one cycle"
    for (; macroexecutor->index < macroexecutor->instructions_size; macroexecutor->index++) {
        MacroExecutorInstruction* instruction = &macroexecutor->instructions[macroexecutor->index];

        switch (instruction->type) {
            case AnimMacroType_INTERPOLATOR:
                macroexecutor_interpolator_stack(macroexecutor, instruction, true);
                break;
            case AnimMacroType_SET:
                float value = macroexecutor_resolve_value(macroexecutor, &instruction->value);
                macroexecutor_state_save(macroexecutor, instruction->property, value);
                break;
            case AnimMacroType_YIELD:
            case AnimMacroType_PAUSE:
                continue;
            case AnimMacroType_RESET:
                macroexecutor_state_clear(macroexecutor);
                macroexecutor_clear_registers(macroexecutor);
                macroexecutor_apply_frame(macroexecutor, NULL);
                break;
            case AnimMacroType_RANDOM_SETUP:
                macroexecutor->random_start = macroexecutor_resolve_value(macroexecutor, &instruction->start);
                macroexecutor->random_end = macroexecutor_resolve_value(macroexecutor, &instruction->end);
                break;
            case AnimMacroType_RANDOM_CHOOSE:
                macroexecutor->random_value = math2d_random_float_range(
                    macroexecutor->random_start, macroexecutor->random_end
                );
                break;
            case AnimMacroType_RANDOM_EXACT:
                macroexecutor_random_exact(macroexecutor, instruction);
                break;
            case AnimMacroType_REGISTER_PROP:
            case AnimMacroType_REGISTER_SET:
                macroexecutor_write_register(macroexecutor, instruction);
                break;
        }
    }

    macroexecutor->sleep_time = 0.0f;
    macroexecutor->on_yield = false;

    if (sprite)
        macroexecutor_state_apply(macroexecutor, sprite, true);
}

MacroExecutor macroexecutor_clone(MacroExecutor macroexecutor) {
    MacroExecutor copy = CLONE_STRUCT(struct MacroExecutor_s, MacroExecutor, macroexecutor);

    copy->interpolators_stack = CLONE_STRUCT_ARRAY(InStackInstruction, macroexecutor->interpolators_stack, macroexecutor->interpolators_stack_size);
    copy->frames = CLONE_STRUCT_ARRAY(AtlasEntry, macroexecutor->frames, macroexecutor->frame_count);
    copy->instructions = CLONE_STRUCT_ARRAY(MacroExecutorInstruction, macroexecutor->instructions, macroexecutor->instructions_size);

    // clone values array
    for (int32_t i = 0; i < copy->instructions_size; i++) {
        MacroExecutorInstruction* instruction = &copy->instructions[i];
        instruction->values = CLONE_STRUCT_ARRAY(MacroExecutorValue, instruction->values, instruction->values_size);
    }

    for (int32_t i = 0; i < macroexecutor->interpolators_stack_size; i++) {
        MacroExecutorInstruction* stack_entry = macroexecutor->interpolators_stack[i].definition;
        if (!stack_entry) continue;

        for (int32_t j = 0; j < macroexecutor->instructions_size; j++) {
            if (stack_entry == &macroexecutor->instructions[j]) {
                copy->interpolators_stack[i].definition = &copy->instructions[j];
                break;
            }
        }
    }

    return copy;
}

int32_t macroexecutor_get_frame_count(MacroExecutor macroexecutor) {
    return macroexecutor->frame_count;
}

const AtlasEntry* macroexecutor_get_frame(MacroExecutor macroexecutor, int32_t index) {
    if (macroexecutor->frame_count < 1) return NULL;
    if (index < 0) index = (int32_t)macroexecutor->state[SPRITE_PROP_FRAMEINDEX];

    return &macroexecutor->frames[index];
}




///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// private functions ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////


static inline void macroexecutor_clear_flags(MacroExecutor macroexecutor) {
    memset(macroexecutor->state_flags, 0x00, MACROEXECUTOR_VERTEX_PROP_COUNT);
}


static void macroexecutor_state_clear(MacroExecutor macroexecutor) {
    for (int32_t i = 0; i < MACROEXECUTOR_VERTEX_PROP_COUNT; i++) {
        float value;
        switch (i) {
            case SPRITE_PROP_SCALE_X:
            case SPRITE_PROP_SCALE_Y:
            case SPRITE_PROP_ALPHA:
                value = 1.0f;
                break;
            default:
                value = 0.0f;
                break;
        }
        macroexecutor->state[i] = value;
    }
    macroexecutor_clear_flags(macroexecutor);
}

static inline float macroexecutor_state_load(MacroExecutor macroexecutor, int32_t property) {
    return macroexecutor->state[property];
}

static void macroexecutor_state_save(MacroExecutor macroexecutor, int32_t property, float value) {
    switch (property) {
        case SPRITE_PROP_ALPHA:
        case SPRITE_PROP_VERTEX_COLOR_R:
        case SPRITE_PROP_VERTEX_COLOR_G:
        case SPRITE_PROP_VERTEX_COLOR_B:
            value = math2d_clamp_float(value, 0.0f, 1.0f);
            break;
        case SPRITE_PROP_FRAMEINDEX:
            value = truncf(math2d_clamp_float(value, 0.0f, macroexecutor->frame_count - 1));
            macroexecutor_apply_frame(macroexecutor, NULL);
            break;
    }
    macroexecutor->state[property] = value;
    macroexecutor->state_flags[property] = true;
}


static void macroexecutor_apply_frame(MacroExecutor macroexecutor, Sprite sprite) {
    if (macroexecutor->frame_count < 1) return;

    float* state = macroexecutor->state;
    int32_t frame_index = (int32_t)state[SPRITE_PROP_FRAMEINDEX];

    const AtlasEntry* frame = &macroexecutor->frames[frame_index];

    if (macroexecutor->frame_allow_change) {
        state[SPRITE_PROP_WIDTH] = frame->frame_width > 0.0f ? frame->frame_width : frame->width;
        state[SPRITE_PROP_HEIGHT] = frame->frame_height > 0.0f ? frame->frame_height : frame->height;

        if (sprite)
            sprite_set_draw_size(sprite, state[SPRITE_PROP_WIDTH], state[SPRITE_PROP_HEIGHT]);
    }

    if (!sprite) return;
    sprite_set_offset_source(sprite, frame->x, frame->y, frame->width, frame->height);
    sprite_set_offset_frame(sprite, frame->frame_x, frame->frame_y, frame->frame_width, frame->frame_height);
}

static void macroexecutor_apply_frame2(MacroExecutor macroexecutor, StateSprite statesprite) {
    if (macroexecutor->frame_count < 1) return;

    float* state = macroexecutor->state;
    int32_t frame_index = (int32_t)state[SPRITE_PROP_FRAMEINDEX];

    const AtlasEntry* frame = &macroexecutor->frames[frame_index];

    if (macroexecutor->frame_allow_change) {
        state[SPRITE_PROP_WIDTH] = frame->frame_width > 0.0f ? frame->frame_width : frame->width;
        state[SPRITE_PROP_HEIGHT] = frame->frame_height > 0.0f ? frame->frame_height : frame->height;

        if (statesprite)
            statesprite_set_draw_size(statesprite, state[SPRITE_PROP_WIDTH], state[SPRITE_PROP_HEIGHT]);
    }

    if (!statesprite) return;
    statesprite_set_offset_source(
        statesprite, frame->x, frame->y, frame->width, frame->height
    );
    statesprite_set_offset_frame(
        statesprite, frame->frame_x, frame->frame_y, frame->frame_width, frame->frame_height
    );
}

static float macroexecutor_resolve_value(MacroExecutor macroexecutor, MacroExecutorValue* value) {
    switch (value->kind) {
        case MacroExecutorValueKind_LITERAL:
            return value->literal;
        case MacroExecutorValueKind_PROPERTY:
            return macroexecutor->state[value->reference];
        case MacroExecutorValueKind_SPECIAL:
            break;
    }
    switch (value->reference) {
        case ANIM_MACRO_SPECIAL_RANDOM:
            return macroexecutor->random_value;
        case ANIM_MACRO_SPECIAL_REGISTER0:
            return macroexecutor->registers[0];
        case ANIM_MACRO_SPECIAL_REGISTER1:
            return macroexecutor->registers[1];
        case ANIM_MACRO_SPECIAL_REGISTER2:
            return macroexecutor->registers[2];
        case ANIM_MACRO_SPECIAL_REGISTER3:
            return macroexecutor->registers[3];
    }

    // this never should happen
    return 0.0f;
}


static void macroexecutor_interpolator_stack(MacroExecutor macroexecutor, MacroExecutorInstruction* instruction, bool inmediate) {
    int32_t i = 0;

    if (!inmediate) {
        for (; i < macroexecutor->interpolators_stack_size; i++) {
            if (macroexecutor->interpolators_stack[i].definition) continue;
            break;
        }
        if (i >= macroexecutor->interpolators_stack_size) return;
    }

    float actual_value = macroexecutor_state_load(macroexecutor, instruction->property);

    float start = macroexecutor_resolve_value(macroexecutor, &instruction->start);
    float end = macroexecutor_resolve_value(macroexecutor, &instruction->end);
    float duration = macroexecutor_resolve_value(macroexecutor, &instruction->duration);
    float steps_count = macroexecutor_resolve_value(macroexecutor, &instruction->steps_count);
    float steps_method = macroexecutor_resolve_value(macroexecutor, &instruction->steps_method);

    if (math2d_is_float_NaN(start)) start = actual_value;
    if (math2d_is_float_NaN(end)) end = actual_value;

    if (inmediate) {
        macroexecutor_state_save(macroexecutor, instruction->property, end);
        return;
    }

    InStackInstruction* stack_entry = &macroexecutor->interpolators_stack[i];
    stack_entry->definition = instruction;
    stack_entry->duration = duration;
    stack_entry->steps_bounds[0] = 0.0f;
    stack_entry->steps_bounds[1] = 0.0f;
    stack_entry->steps_bounds[2] = 0.0f;
    stack_entry->progress = 0.0f;
    stack_entry->start = start;
    stack_entry->end = end;
    stack_entry->steps_count = (int32_t)steps_count;
    stack_entry->steps_method = (Align)((int32_t)steps_method);

    macroexecutor_interpolator_run(macroexecutor, 0.0f, &macroexecutor->interpolators_stack[i]);
}

static int32_t macroexecutor_interpolators_check(MacroExecutor macroexecutor, float elapsed) {
    int32_t running = 0;

    for (int32_t i = 0; i < macroexecutor->interpolators_stack_size; i++) {
        InStackInstruction* stack_entry = &macroexecutor->interpolators_stack[i];
        if (!stack_entry->definition) continue;

        bool done = macroexecutor_interpolator_run(macroexecutor, elapsed, stack_entry);

        if (done)
            stack_entry->definition = NULL;
        else
            running++;
    }

    return running;
}

static bool macroexecutor_interpolator_run(MacroExecutor macroexecutor, float elapsed, InStackInstruction* interpolator) {
    float percent = math2d_inverselerp(0.0f, interpolator->duration, (float)interpolator->progress);

    interpolator->progress += elapsed;
    bool completed = interpolator->progress >= interpolator->duration;
    if (completed || percent > 1.0f) percent = 1.0f;

    float value = 0.0f;
    switch (interpolator->definition->interpolator) {
        case AnimInterpolator_EASE:
            value = macroexecutor_calc_cubicbezier(percent, CUBIC_BREZIER_EASE);
            break;
        case AnimInterpolator_EASE_IN:
            value = macroexecutor_calc_cubicbezier(percent, CUBIC_BREZIER_EASE_IN);
            break;
        case AnimInterpolator_EASE_OUT:
            value = macroexecutor_calc_cubicbezier(percent, CUBIC_BREZIER_EASE_OUT);
            break;
        case AnimInterpolator_EASE_IN_OUT:
            value = macroexecutor_calc_cubicbezier(percent, CUBIC_BREZIER_EASE_IN_OUT);
            break;
        case AnimInterpolator_LINEAR:
            value = percent;
            break;
        case AnimInterpolator_STEPS:
            value = macroexecutor_calc_steps(
                percent,
                interpolator->steps_bounds,
                interpolator->steps_count,
                interpolator->steps_method
            );
            break;
        case AnimInterpolator_CUBIC:
            value = math2d_lerp_cubic(percent);
            break;
        case AnimInterpolator_QUAD:
            value = math2d_lerp_quad(percent);
            break;
        case AnimInterpolator_EXPO:
            value = math2d_lerp_expo(percent);
            break;
        case AnimInterpolator_SIN:
            value = math2d_lerp_sin(percent);
            break;
    }


    if (!math2d_is_float_finite(value)) {
        logger_error("macroexecutor_interpolator_run() infinite/NaN value found");
        assert(math2d_is_float_finite(value));
    }

    // commit interpolation in the specific range
    value = math2d_lerp(interpolator->start, interpolator->end, value);

    macroexecutor_state_save(macroexecutor, interpolator->definition->property, value);
    return completed;
}



float macroexecutor_calc_cubicbezier(float elapsed_time, const float points[4]) {
    float input = math2d_cubicbezier(elapsed_time, 0.0f, points[0], points[1], 1.0);
    float output = math2d_cubicbezier(elapsed_time, 0.0f, points[2], points[3], 1.0);

    if (input == 0.0f || output == 0.0f) return 0.0f; // avoid NaN and division by zero exceptions

    float res = (elapsed_time * input) / output;
    // res = math2d_clamp_float(res, 0.0f, 1.0f);

    return res;
}

float macroexecutor_calc_steps(float elapsed_time, float bounds[3], int32_t count, Align direction) {
    // initialize steps bounds, this is done once
    if (bounds[0] == 0.0f) {
        // workaround (tested in "none" direction only)
        count++;

        float step_in = 1.0f / count;
        float offset = direction == ALIGN_BOTH ? 1.0f : -1.0f;
        float step_out = 1.0f / (count + offset);
        float step_mul = direction == ALIGN_END ? step_in : step_out;

        bounds[0] = step_in;
        bounds[1] = step_out;
        bounds[2] = step_mul;
    }

    elapsed_time = math2d_nearestdown(elapsed_time, bounds[0]);

    float res;
    if (direction == ALIGN_START) {
        res = elapsed_time + bounds[0];
    } else {
        res = (elapsed_time * bounds[2]) / bounds[0];
        if (direction == ALIGN_BOTH) res += bounds[1];
    }

    return math2d_clamp_float(res, 0.0f, 1.0f);
}

float macroexecutor_calc_swing(float percent) {
    return 0.5f - math2d_cos(percent * MATH2D_PI) / 2.0f;
}

float macroexecutor_calc_log(float percent) {
    float value = logf(percent * 100.0f) / MATH2D_LOG100;
    return value < 0.0f ? 0.0f : value;
}


static void macroexecutor_write_register(MacroExecutor macroexecutor, MacroExecutorInstruction* instruction) {
    // resolve the value to store
    float value;
    if (instruction->type == AnimMacroType_REGISTER_PROP)
        value = macroexecutor->state[instruction->property];
    else
        value = macroexecutor_resolve_value(macroexecutor, &instruction->value);

    if (instruction->register_index == -1) {
        // put the value in all registers
        for (int32_t i = 0; i < MACROEXECUTOR_REGISTER_COUNT; i++)
            macroexecutor->registers[i] = value;
        return;
    }

    // store the value in a specific register
    macroexecutor->registers[instruction->register_index] = value;
}

static void macroexecutor_clear_registers(MacroExecutor macroexecutor) {
    memset(macroexecutor->registers, 0x00, sizeof(macroexecutor->registers));
}

static void macroexecutor_random_exact(MacroExecutor macroexecutor, MacroExecutorInstruction* instruction) {
    if (instruction->values_size < 1) {
        // this never should happen
        macroexecutor->random_value = 0;
        return;
    }

    int32_t index = math2d_random_int(0, instruction->values_size);
    MacroExecutorValue* item = &instruction->values[index];

    // resolve the item value
    macroexecutor->random_value = macroexecutor_resolve_value(macroexecutor, item);
}
