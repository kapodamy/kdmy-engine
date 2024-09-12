#ifndef _charactermanifest_h
#define _charactermanifest_h

#include <stdbool.h>
#include <stdint.h>

#include "vertexprops_types.h"


typedef struct {
    int32_t sizes;
    char** from;
    char** to;
} CharacterManifestOpposite;

typedef struct {
    char* direction;
    char* model_src;
    char* anim;
    char* anim_hold;
    char* anim_rollback;
    bool rollback;
    bool follow_hold;
    bool full_sustain;
    float stop_after_beats;
    float offset_x;
    float offset_y;
} CharacterManifestSing;

typedef struct {
    char* direction;
    char* model_src;
    float stop_after_beats;
    char* anim;
    float offset_x;
    float offset_y;
} CharacterManifestMiss;

typedef struct {
    char* name;
    char* model_src;
    float stop_after_beats;
    char* anim;
    char* anim_hold;
    char* anim_rollback;
    float offset_x;
    float offset_y;
} CharacterManifestExtra;

typedef struct {
    int32_t sing_size;
    int32_t miss_size;
    int32_t extras_size;
    CharacterManifestSing* sing;
    CharacterManifestSing* sing_alt;
    CharacterManifestMiss* miss;
    CharacterManifestExtra* extras;
    CharacterManifestExtra idle;
    CharacterManifestExtra hey;
    bool has_idle;
    bool has_hey;
} CharacterManifestActions;

typedef struct {
    char* name;
    char* model;
    CharacterManifestActions actions;
} CharacterManifestAdditionalState;

typedef struct CharacterManifest_s {
    char* model_character;
    CharacterManifestActions actions;
    bool left_facing;
    Align align_horizontal;
    Align align_vertical;
    bool actions_apply_chart_speed;
    bool continuous_idle;
    bool has_reference_size;
    bool allow_alternate_idle;
    float reference_width;
    float reference_height;
    char* sing_suffix;
    char* sing_alternate_suffix;
    char* sing_prefix;
    char* sing_alternate_prefix;
    CharacterManifestOpposite opposite_directions;
    char* model_health_icons;
    bool has_healthbar_color;
    uint32_t healthbar_color;
    float offset_x;
    float offset_y;
    char* week_selector_model;
    char* week_selector_idle_anim_name;
    char* week_selector_choosen_anim_name;
    bool week_selector_left_facing;
    bool week_selector_enable_beat;
    CharacterManifestAdditionalState* additional_states;
    int32_t additional_states_size;

}* CharacterManifest;

CharacterManifest charactermanifest_init(const char* src, bool gameplay_required_models_only);
void charactermanifest_destroy(CharacterManifest* character_manifest);

#endif
