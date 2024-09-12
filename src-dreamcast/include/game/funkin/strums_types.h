#ifndef _strums_types_h
#define _strums_types_h

#include <stdbool.h>
#include <stdint.h>

#include "gamepad_types.h"

//
//  forward references
//

typedef struct PlayerStats_s* PlayerStats;
typedef struct Drawable_s* Drawable;
typedef struct AnimSprite_s* AnimSprite;
typedef struct Chart_s* Chart;



typedef struct {
    char* name;
    char* sick_effect_state_name;
    char* model_src;
    char* custom_sick_effect_model_src;
    float damage_ratio;
    float heal_ratio;
    bool ignore_hit;
    bool ignore_miss;
    bool can_kill_on_hit;
    bool is_special;
} DistributionNote;

typedef struct {
    char* name;
    int32_t* notes_ids;
    int32_t notes_ids_size;
} DistributionStrum;

typedef struct {
    char* name;
    char* model_marker;
    char* model_sick_effect;
    char* model_background;
    char* model_notes;
} DistributionStrumState;

typedef struct {
    int32_t notes_size;
    DistributionNote* notes;
    int32_t strums_size;
    DistributionStrum* strums;
    GamepadButtons* strum_binds;
    bool strum_binds_is_custom;
    DistributionStrumState* states;
    int32_t states_size;
} Distribution;

typedef struct Strums_s* Strums;

#endif