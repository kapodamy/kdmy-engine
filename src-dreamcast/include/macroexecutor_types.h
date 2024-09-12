#ifndef _macroexecutor_types_h
#define _macroexecutor_types_h

#include "stdint.h"

#include "vertexprops_types.h"


// this must match ANIM_MACRO_SPECIAL_REGISTER* count
#define MACROEXECUTOR_REGISTER_COUNT 4

#define ANIM_MACRO_SPECIAL_REGISTER0 0
#define ANIM_MACRO_SPECIAL_REGISTER1 1
#define ANIM_MACRO_SPECIAL_REGISTER2 2
#define ANIM_MACRO_SPECIAL_REGISTER3 3
#define ANIM_MACRO_SPECIAL_RANDOM 9


typedef enum {
    AnimMacroType_INTERPOLATOR = 0,
    AnimMacroType_SET = 1,
    AnimMacroType_YIELD = 2,
    AnimMacroType_PAUSE = 3,
    AnimMacroType_RESET = 4,
    AnimMacroType_RANDOM_SETUP = 5,
    AnimMacroType_RANDOM_CHOOSE = 6,
    AnimMacroType_RANDOM_EXACT = 7,
    AnimMacroType_REGISTER_PROP = 8,
    AnimMacroType_REGISTER_SET = 9,
} AnimMacroType;

typedef enum {
    AnimInterpolator_EASE = 0,
    AnimInterpolator_EASE_IN = 1,
    AnimInterpolator_EASE_OUT = 2,
    AnimInterpolator_EASE_IN_OUT = 3,
    AnimInterpolator_LINEAR = 4,
    AnimInterpolator_STEPS = 5,
    AnimInterpolator_CUBIC = 6,
    AnimInterpolator_QUAD = 7,
    AnimInterpolator_EXPO = 8,
    AnimInterpolator_SIN = 9
} AnimInterpolator;

typedef enum {
    MacroExecutorValueKind_LITERAL,
    MacroExecutorValueKind_SPECIAL,
    MacroExecutorValueKind_PROPERTY
} MacroExecutorValueKind;

typedef struct {
    float literal;
    int32_t reference;
    MacroExecutorValueKind kind;
} MacroExecutorValue;

typedef struct {
    int32_t property;
    AnimMacroType type;
    MacroExecutorValue value;
    MacroExecutorValue start;
    MacroExecutorValue end;
    int32_t values_size;
    MacroExecutorValue* values;
    int32_t register_index;
    MacroExecutorValue steps_count;
    MacroExecutorValue steps_method;
    MacroExecutorValue duration;
    AnimInterpolator interpolator;
} MacroExecutorInstruction;

typedef void (*PropertySetter)(void* vertex, int32_t property_id, float value);


extern const float CUBIC_BREZIER_EASE_LINEAR[4];
extern const float CUBIC_BREZIER_EASE[4];
extern const float CUBIC_BREZIER_EASE_IN[4];
extern const float CUBIC_BREZIER_EASE_OUT[4];
extern const float CUBIC_BREZIER_EASE_IN_OUT[4];


float macroexecutor_calc_cubicbezier(float elapsed_time, const float points[4]);
float macroexecutor_calc_steps(float elapsed_time, float bounds[3], int32_t count, Align direction);
float macroexecutor_calc_swing(float percent);
float macroexecutor_calc_log(float percent);


#endif