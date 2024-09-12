#include "game/funkin/healthbar.h"

#include "beatwatcher.h"
#include "drawable.h"
#include "externals/luascript.h"
#include "game/common/funkin.h"
#include "imgutils.h"
#include "linkedlist.h"
#include "malloc_utils.h"
#include "statesprite.h"
#include "stringutils.h"
#include "tweenlerp.h"


typedef struct {
    char* state_name;
    int32_t resolution_bar_width, resolution_bar_height;
    int32_t resolution_icon_width, resolution_icon_height;
} TextureResolution;

struct Healthbar_s {
    float x;
    float y;
    float z;
    float length;
    float dimmen;
    float border;
    float icon_overlap;
    float warn_height;
    float lock_height;
    bool is_vertical;
    bool enable_overlap;
    StateSprite sprite_background;
    StateSprite sprite_bar_opponent;
    StateSprite sprite_bar_player;
    StateSprite sprite_icon_opponent;
    StateSprite sprite_icon_player;
    bool flip_icon_opponent;
    bool flip_icon_player;
    float health_bar_length;
    bool extra_enabled;
    float extra_translation;
    char* selected_state_player;
    char* selected_state_opponent;
    AnimSprite bump_animation_opponent;
    AnimSprite bump_animation_player;
    Modifier bump_modifier_opponent;
    Modifier bump_modifier_player;
    bool enable_bump;
    LinkedList /*<TextureResolution>*/ resolutions_player;
    LinkedList /*<TextureResolution>*/ resolutions_opponent;
    float layout_width;
    float layout_height;
    BeatWatcher beatwatcher;
    bool transition_enabled;
    TweenLerp tweenlerp;
    float last_health;
    float last_health_position_opponent;
    char* prefix_state_player;
    char* prefix_state_opponent;
    AnimSprite drawable_animation;
    StateSprite sprite_warnings;
    bool enable_warnings;
    bool enable_flash_warning;
    float low_health_flash_warning;
    bool has_warning_locked;
    bool has_warning_drain;
    int32_t has_warning_opponent_recover;
    float warning_locked_position;
    float warning_drain_x;
    float warning_drain_y;
    Modifier* modifier;
    Drawable drawable;
    bool first_init;
};



const int32_t HEALTHBAR_DEFAULT_DIMMEN = 20;      // 20px in a 1280x720 screen
const float HEALTHBAR_DEFAULT_BORDER_SIZE = 4.0f; // 4px in a 1280x720 screen

const int32_t HEALTHBAR_DEFAULT_ICON_SIZE = 80; // 80px in a 1280x720 screen

static const float HEALTHBAR_SCALE_TO_DIMMEN_ICON = 80.0f / HEALTHBAR_DEFAULT_DIMMEN;

static const char* HEALTHBAR_ICON_PREFIX_WINNER = "winner";
static const char* HEALTHBAR_ICON_PREFIX_WINNING = "winning";
const char* HEALTHBAR_ICON_PREFIX_NEUTRAL = "neutral";
static const char* HEALTHBAR_ICON_PREFIX_LOOSING = "loosing";
static const char* HEALTHBAR_ICON_PREFIX_LOOSER = "looser";

static const char* HEALTHBAR_WARNING_ALT_SUFFIX = " alt";
static const char* HEALTHBAR_WARNING_FAST_DRAIN = "fastDrain";
static const char* HEALTHBAR_WARNING_SLOW_DRAIN = "slowDrain";
static const char* HEALTHBAR_WARNING_LOCKED = "locked";
static const char* HEALTHBAR_WARNING_OPPONENT_RECOVER = "opponentRecover";

static const float HEALTHBAR_CHARACTER_WARNING_PERCENT = 0.25f; // warn if less or equal to 25%
static const float HEALTHBAR_HEALTH_TRANSITION_RATIO = 8.0f;    // the transition duration in BMP/N

const float HEALTHBAR_RATIO_SIZE_NORMAL = 600.0f; // 600px in a 1280x720 screen
// const float HEALTHBAR_RATIO_SIZE_LONG = 1180.0f; // 1180px in a 1280x70 screen

static RGBA HEALTHBAR_LOW_HEALTH_WARN_COLOR = {1.0f, 0.0f, 0.0f, 0.5f}; // rgba: half-transparent red
static const float HEALTHBAR_LOW_HEALTH_PERCENT = 0.10f;                // warn if less or equal to 10%
static const float HEALTHBAR_LOW_HEALTH_FLASH_RATIO = 8.0f;             // active flash duration in BMP/N
static const char* HEALTHBAR_UI_ICON_BUMP = "healthbar_icon_bump";      // picked from UI animlist

static const char* HEALTHBAR_INTERNAL_STATE_NAME = "healthbar-state";


static void healthbar_internal_calc_dimmensions(Healthbar healthbar);
static void healthbar_internal_calc_health_positions(Healthbar healthbar, float player_health);
static int32_t healthbar_internal_add_chrctr_state(StateSprite icon, StateSprite bar, ModelHolder icn_mdlhldr, ModelHolder hlth_mdlhldr, LinkedList rsltn, const char* state_name);
static int32_t healthbar_internal_add_chrctr_state2(StateSprite icon, StateSprite bar, ModelHolder icn_mdlhldr, ModelHolder hlth_mdlhldr, const char* sub_state_name);
static void healthbar_internal_center_bar(Healthbar healthbar, float width, float height, float border, int32_t rsltn_width, int32_t rsltn_height, StateSprite statesprite);
static void healthbar_internal_center_icon(Healthbar healthbar, Align align, int32_t rsltn_width, int32_t rsltn_height, StateSprite statesprite);
static void healthbar_internal_resize_state(Healthbar healthbar, StateSpriteState* state, int32_t rsltn_width, int32_t rsltn_height, int32_t def_size, bool scale_to_dimmen);
static void healthbar_internal_icon_flip(StateSprite statesprite, bool is_vertical, bool do_flip);
static int32_t healthbar_internal_toggle_chrctr_state(char** slctd_ptr, const char* prefix, const char* state_name, StateSprite bar, StateSprite icon);
static int32_t healthbar_internal_toggle_chrctr_state2(const char* prefix, const char* state_name, StateSprite bar, StateSprite icon);
static void healthbar_internal_set_chrctr_state(const char* prefix, const char* state_name, StateSprite bar, StateSprite icon);
static void healthbar_internal_calc_chrctr(Healthbar healthbar, bool invert, float position, StateSprite bar, StateSprite icon);
static void healthbar_internal_draw_chrctr(PVRContext pvrctx, Modifier* bump_modifier, StateSprite icon);
static bool healthbar_internal_add_warning(StateSprite sprite, ModelHolder modelholder, bool use_alt, float height, const char* state_name);
static void healthbar_internal_save_resolutions(LinkedList linkedlist, const char* state_name, ModelHolder mdlhldr_bar, ModelHolder mdlhldr_icn);
static void healthbar_internal_load_resolutions(LinkedList linkedlist, const char* state_name, int32_t* rsltn_bar_width, int32_t* rsltn_bar_height, int32_t* rsltn_icn_width, int32_t* rsltn_icn_height);


Healthbar healthbar_init(float x, float y, float z, float length, float dimmen, float border, float icon_overlap, float warn_height, float lock_height) {
    Healthbar healthbar = malloc_chk(sizeof(struct Healthbar_s));
    malloc_assert(healthbar, Healthbar);

    *healthbar = (struct Healthbar_s){
        .x = x, .y = y, .z = z, .length = length, .dimmen = dimmen, .border = border, .icon_overlap = icon_overlap, .warn_height = warn_height, .lock_height = lock_height,

        .is_vertical = false,
        .enable_overlap = true,

        .sprite_background = statesprite_init_from_vertex_color(HEALTHBAR_DEFAULT_COLOR_BACKGROUND),

        .sprite_bar_opponent = statesprite_init_from_vertex_color(HEALTHBAR_DEFAULT_COLOR_DAD),
        .sprite_bar_player = statesprite_init_from_vertex_color(HEALTHBAR_DEFAULT_COLOR_BOYFRIEND),

        .sprite_icon_opponent = statesprite_init_from_texture(NULL),
        .sprite_icon_player = statesprite_init_from_texture(NULL),

        .flip_icon_opponent = false,
        .flip_icon_player = true,

        .health_bar_length = length - (border * 2.0f),
        .extra_enabled = false,
        .extra_translation = 0.0f,

        .selected_state_player = (char*)HEALTHBAR_INTERNAL_STATE_NAME,
        .selected_state_opponent = (char*)HEALTHBAR_INTERNAL_STATE_NAME,

        .bump_animation_opponent = NULL,
        .bump_animation_player = NULL,

        .bump_modifier_opponent = (Modifier){},
        .bump_modifier_player = (Modifier){},
        .enable_bump = true,

        .resolutions_player = linkedlist_init(),
        .resolutions_opponent = linkedlist_init(),

        .layout_width = FUNKIN_SCREEN_RESOLUTION_WIDTH,
        .layout_height = FUNKIN_SCREEN_RESOLUTION_HEIGHT,

        .beatwatcher = (BeatWatcher){},

        .transition_enabled = true,
        .tweenlerp = tweenlerp_init(),

        .last_health = FLOAT_NaN,
        .last_health_position_opponent = FLOAT_NaN,
        .prefix_state_player = (char*)HEALTHBAR_ICON_PREFIX_NEUTRAL,
        .prefix_state_opponent = (char*)HEALTHBAR_ICON_PREFIX_NEUTRAL,

        .drawable_animation = NULL,

        .sprite_warnings = statesprite_init_from_texture(NULL),
        .enable_warnings = true,
        .enable_flash_warning = false,
        .low_health_flash_warning = 0.0f,

        .has_warning_locked = false,
        .has_warning_drain = false,
        .has_warning_opponent_recover = -1,
        .warning_locked_position = 0.0f,
        .warning_drain_x = 0.0f,
        .warning_drain_y = 0.0f,

        .modifier = NULL,
        .drawable = NULL,

        .first_init = true
    };

    beatwatcher_reset(&healthbar->beatwatcher, true, 100.0f);

    pvr_context_helper_clear_modifier(&healthbar->bump_modifier_opponent);
    healthbar->bump_modifier_opponent.x = x;
    healthbar->bump_modifier_opponent.x = y;

    pvr_context_helper_clear_modifier(&healthbar->bump_modifier_player);
    healthbar->bump_modifier_player.x = x;
    healthbar->bump_modifier_player.x = y;

    statesprite_set_draw_location(healthbar->sprite_icon_opponent, x, y);
    statesprite_set_draw_location(healthbar->sprite_icon_player, x, y);
    statesprite_set_draw_location(healthbar->sprite_background, x, y);
    statesprite_set_draw_location(healthbar->sprite_bar_opponent, x, y);
    statesprite_set_draw_location(healthbar->sprite_bar_player, x, y);

    statesprite_set_visible(healthbar->sprite_icon_opponent, false);
    statesprite_set_visible(healthbar->sprite_icon_player, false);
    statesprite_set_visible(healthbar->sprite_background, false);
    statesprite_set_visible(healthbar->sprite_bar_opponent, false);
    statesprite_set_visible(healthbar->sprite_bar_player, false);

    statesprite_crop_enable(healthbar->sprite_bar_opponent, true);
    statesprite_crop_enable(healthbar->sprite_bar_player, true);

    healthbar->drawable = drawable_init(z, healthbar, (DelegateDraw)healthbar_draw, (DelegateAnimate)healthbar_animate);

    healthbar->modifier = drawable_get_modifier(healthbar->drawable);
    healthbar->modifier->x = x;
    healthbar->modifier->y = y;

    float transition_ms = healthbar->beatwatcher.tick / HEALTHBAR_HEALTH_TRANSITION_RATIO;
    tweenlerp_add_easeout(healthbar->tweenlerp, -1, 0.0f, 0.0f, transition_ms);

    return healthbar;
}

void healthbar_destroy(Healthbar* healthbar_ptr) {
    if (!healthbar_ptr || !*healthbar_ptr) return;

    Healthbar healthbar = *healthbar_ptr;

    luascript_drop_shared(healthbar);

    drawable_destroy(&healthbar->drawable);
    if (healthbar->drawable_animation) animsprite_destroy(&healthbar->drawable_animation);

    if (/*pointer equals*/ healthbar->selected_state_player != HEALTHBAR_INTERNAL_STATE_NAME)
        free_chk(healthbar->selected_state_player);
    if (/*pointer equals*/ healthbar->selected_state_opponent != HEALTHBAR_INTERNAL_STATE_NAME)
        free_chk(healthbar->selected_state_opponent);

    tweenlerp_destroy(&healthbar->tweenlerp);

    if (healthbar->bump_animation_opponent) animsprite_destroy(&healthbar->bump_animation_opponent);
    if (healthbar->bump_animation_player) animsprite_destroy(&healthbar->bump_animation_player);

    statesprite_destroy(&healthbar->sprite_background);
    statesprite_destroy(&healthbar->sprite_bar_opponent);
    statesprite_destroy(&healthbar->sprite_bar_player);
    statesprite_destroy(&healthbar->sprite_icon_opponent);
    statesprite_destroy(&healthbar->sprite_icon_player);

    statesprite_destroy(&healthbar->sprite_warnings);

    // dispose the resolution list, list items and state names

    foreach (TextureResolution*, item, LINKEDLIST_ITERATOR, healthbar->resolutions_opponent) {
        free_chk(item->state_name);
        free_chk(item);
    }
    linkedlist_destroy(&healthbar->resolutions_opponent);

    foreach (TextureResolution*, item, LINKEDLIST_ITERATOR, healthbar->resolutions_player) {
        free_chk(item->state_name);
        free_chk(item);
    }
    linkedlist_destroy(&healthbar->resolutions_player);

    free_chk(healthbar);
    *healthbar_ptr = NULL;
}


void healthbar_set_layout_size(Healthbar healthbar, float width, float height) {
    healthbar->layout_width = width;
    healthbar->layout_height = height;
}

void healthbar_enable_extra_length(Healthbar healthbar, bool extra_enabled) {
    healthbar->extra_enabled = extra_enabled;

    // recalculate everything
    float last_health = healthbar->last_health;
    healthbar->last_health = FLOAT_NaN;
    if (math2d_is_float_NaN(last_health)) return;
    healthbar_internal_calc_health_positions(healthbar, last_health);
}

void healthbar_enable_vertical(Healthbar healthbar, bool enable_vertical) {
    healthbar->is_vertical = enable_vertical;
    healthbar_internal_calc_dimmensions(healthbar);
}


int32_t healthbar_state_opponent_add(Healthbar healthbar, ModelHolder icon_mdlhldr, ModelHolder bar_mdlhldr, const char* state_name) {
    return healthbar_internal_add_chrctr_state(
        healthbar->sprite_icon_opponent, healthbar->sprite_bar_opponent,
        icon_mdlhldr, bar_mdlhldr,
        healthbar->resolutions_opponent,
        state_name
    );
}

int32_t healthbar_state_opponent_add2(Healthbar healthbar, ModelHolder icon_mdlhldr, uint32_t bar_color_rgb8, const char* state_name) {
    ModelHolder mdl = modelholder_init3(bar_color_rgb8, NULL, NULL, NULL);

    int32_t ret = healthbar_internal_add_chrctr_state(
        healthbar->sprite_icon_opponent, healthbar->sprite_bar_opponent,
        icon_mdlhldr, mdl,
        healthbar->resolutions_opponent,
        state_name
    );

    modelholder_destroy(&mdl);
    return ret;
}

int32_t healthbar_state_player_add(Healthbar healthbar, ModelHolder icon_mdlhldr, ModelHolder bar_mdlhldr, const char* state_name) {
    return healthbar_internal_add_chrctr_state(
        healthbar->sprite_icon_player, healthbar->sprite_bar_player,
        icon_mdlhldr, bar_mdlhldr,
        healthbar->resolutions_player,
        state_name
    );
}

int32_t healthbar_state_player_add2(Healthbar healthbar, ModelHolder icon_modelholder, uint32_t bar_color_rgb8, const char* state_name) {
    ModelHolder mdl = modelholder_init3(bar_color_rgb8, NULL, NULL, NULL);

    int32_t ret = healthbar_internal_add_chrctr_state(
        healthbar->sprite_icon_player, healthbar->sprite_bar_player,
        icon_modelholder, mdl,
        healthbar->resolutions_player,
        state_name
    );

    modelholder_destroy(&mdl);
    return ret;
}


bool healthbar_state_background_add(Healthbar healthbar, ModelHolder modelholder, const char* state_name) {
    StateSpriteState* state = statesprite_state_add(healthbar->sprite_background, modelholder, state_name, state_name);
    return state != NULL;
}

bool healthbar_state_background_add2(Healthbar healthbar, uint32_t color_rgb8, AnimSprite animsprite, const char* state_name) {
    animsprite = animsprite ? animsprite_clone(animsprite) : NULL;
    StateSpriteState* state = statesprite_state_add2(
        healthbar->sprite_background, NULL, animsprite, NULL, color_rgb8, state_name
    );
    if (!state && animsprite) animsprite_destroy(&animsprite);
    return state != NULL;
}

bool healthbar_load_warnings(Healthbar healthbar, ModelHolder modelholder, bool use_alt_icons) {
    int32_t success = 0;

    success += healthbar_internal_add_warning(
                   healthbar->sprite_warnings, modelholder, use_alt_icons, healthbar->warn_height,
                   HEALTHBAR_WARNING_FAST_DRAIN
               )
                   ? 1
                   : 0;
    success += healthbar_internal_add_warning(
                   healthbar->sprite_warnings, modelholder, use_alt_icons, healthbar->warn_height,
                   HEALTHBAR_WARNING_SLOW_DRAIN
               )
                   ? 1
                   : 0;
    success += healthbar_internal_add_warning(
                   healthbar->sprite_warnings, modelholder, use_alt_icons, healthbar->lock_height,
                   HEALTHBAR_WARNING_LOCKED
               )
                   ? 1
                   : 0;
    success += healthbar_internal_add_warning(
                   healthbar->sprite_warnings, modelholder, use_alt_icons, healthbar->warn_height,
                   HEALTHBAR_WARNING_OPPONENT_RECOVER
               )
                   ? 1
                   : 0;

    return success >= 4;
}


void healthbar_set_opponent_bar_color_rgb8(Healthbar healthbar, uint32_t color_rgb8) {
    statesprite_set_vertex_color_rgb8(healthbar->sprite_bar_opponent, color_rgb8);
}

void healthbar_set_opponent_bar_color(Healthbar healthbar, float r, float g, float b) {
    statesprite_set_vertex_color(healthbar->sprite_bar_opponent, r, g, b);
}

void healthbar_set_player_bar_color_rgb8(Healthbar healthbar, uint32_t color_rgb8) {
    statesprite_set_vertex_color_rgb8(healthbar->sprite_bar_player, color_rgb8);
}

void healthbar_set_player_bar_color(Healthbar healthbar, float r, float g, float b) {
    statesprite_set_vertex_color(healthbar->sprite_bar_player, r, g, b);
}


int32_t healthbar_state_toggle(Healthbar healthbar, const char* state_name) {
    int32_t success = 0;
    success += healthbar_state_toggle_background(healthbar, state_name) ? 1 : 0;
    success += healthbar_state_toggle_player(healthbar, state_name);
    success += healthbar_state_toggle_opponent(healthbar, state_name);

    if (healthbar->first_init) {
        healthbar->first_init = false;
        healthbar_internal_calc_dimmensions(healthbar);
    }

    return success;
}

bool healthbar_state_toggle_background(Healthbar healthbar, const char* state_name) {
    return statesprite_state_toggle(healthbar->sprite_background, state_name);
}

bool healthbar_state_toggle_player(Healthbar healthbar, const char* state_name) {
    return healthbar_internal_toggle_chrctr_state(
        &healthbar->selected_state_player, healthbar->prefix_state_player,
        state_name, healthbar->sprite_bar_player, healthbar->sprite_icon_player
    );
}

bool healthbar_state_toggle_opponent(Healthbar healthbar, const char* state_name) {
    return healthbar_internal_toggle_chrctr_state(
        &healthbar->selected_state_opponent, healthbar->prefix_state_opponent,
        state_name, healthbar->sprite_bar_opponent, healthbar->sprite_icon_opponent
    );
}


void healthbar_set_bump_animation(Healthbar healthbar, AnimList animlist) {
    if (!animlist) return;
    const AnimListItem* animlist_item = animlist_get_animation(animlist, HEALTHBAR_UI_ICON_BUMP);

    if (!animlist_item) return;
    AnimSprite animsprite = animsprite_init(animlist_item);

    if (healthbar->bump_animation_opponent) animsprite_destroy(&healthbar->bump_animation_opponent);
    if (healthbar->bump_animation_player) animsprite_destroy(&healthbar->bump_animation_player);

    healthbar_set_bump_animation_opponent(healthbar, animsprite);
    healthbar_set_bump_animation_player(healthbar, animsprite);
    if (animsprite != NULL) animsprite_destroy(&animsprite);
}

void healthbar_set_bump_animation_opponent(Healthbar healthbar, AnimSprite animsprite) {
    if (healthbar->bump_animation_opponent) animsprite_destroy(&healthbar->bump_animation_opponent);
    healthbar->bump_animation_opponent = animsprite ? animsprite_clone(animsprite) : NULL;
}

void healthbar_set_bump_animation_player(Healthbar healthbar, AnimSprite animsprite) {
    if (healthbar->bump_animation_player) animsprite_destroy(&healthbar->bump_animation_player);
    healthbar->bump_animation_player = animsprite ? animsprite_clone(animsprite) : NULL;
}

void healthbar_bump_enable(Healthbar healthbar, bool enable_bump) {
    healthbar->enable_bump = enable_bump;
}


void healthbar_set_bpm(Healthbar healthbar, float beats_per_minute) {
    beatwatcher_change_bpm(&healthbar->beatwatcher, beats_per_minute);

    float transition_ms = healthbar->beatwatcher.tick / HEALTHBAR_HEALTH_TRANSITION_RATIO;
    tweenlerp_change_duration_by_index(healthbar->tweenlerp, 0, transition_ms);
}


void healthbar_set_offsetcolor(Healthbar healthbar, float r, float g, float b, float a) {
    drawable_set_offsetcolor(healthbar->drawable, r, g, b, a);
}

void healthbar_set_alpha(Healthbar healthbar, float alpha) {
    drawable_set_alpha(healthbar->drawable, alpha);
}

void healthbar_set_visible(Healthbar healthbar, bool visible) {
    drawable_set_visible(healthbar->drawable, visible);
}

Modifier* healthbar_get_modifier(Healthbar healthbar) {
    return drawable_get_modifier(healthbar->drawable);
}

Drawable healthbar_get_drawable(Healthbar healthbar) {
    return healthbar->drawable;
}


void healthbar_animation_set(Healthbar healthbar, AnimSprite animsprite) {
    if (healthbar->drawable_animation) animsprite_destroy(&healthbar->drawable_animation);
    healthbar->drawable_animation = animsprite ? animsprite_clone(animsprite) : NULL;
}

void healthbar_animation_restart(Healthbar healthbar) {
    if (healthbar->bump_animation_opponent) animsprite_restart(healthbar->bump_animation_opponent);
    if (healthbar->bump_animation_player) animsprite_restart(healthbar->bump_animation_player);
    if (healthbar->drawable_animation) animsprite_restart(healthbar->drawable_animation);

    statesprite_animation_restart(healthbar->sprite_background);
    statesprite_animation_restart(healthbar->sprite_bar_opponent);
    statesprite_animation_restart(healthbar->sprite_icon_opponent);
    statesprite_animation_restart(healthbar->sprite_bar_player);
    statesprite_animation_restart(healthbar->sprite_icon_player);
}

void healthbar_animation_end(Healthbar healthbar) {
    if (healthbar->bump_animation_opponent) {
        animsprite_force_end(healthbar->bump_animation_opponent);
        animsprite_update_modifier(healthbar->bump_animation_opponent, &healthbar->bump_modifier_opponent, true);
    }
    if (healthbar->bump_animation_player) {
        animsprite_force_end(healthbar->bump_animation_player);
        animsprite_update_modifier(healthbar->bump_animation_player, &healthbar->bump_modifier_player, true);
    }
    if (healthbar->drawable_animation) {
        animsprite_force_end(healthbar->drawable_animation);
        animsprite_update_drawable(healthbar->drawable_animation, healthbar->drawable, true);
    }

    statesprite_animation_end(healthbar->sprite_background);
    statesprite_animation_end(healthbar->sprite_bar_opponent);
    statesprite_animation_end(healthbar->sprite_icon_opponent);
    statesprite_animation_end(healthbar->sprite_bar_player);
    statesprite_animation_end(healthbar->sprite_icon_player);
}


int32_t healthbar_animate(Healthbar healthbar, float elapsed) {
    float since_beat = elapsed;
    bool has_bump_opponent = healthbar->enable_bump && (healthbar->bump_animation_opponent != NULL);
    bool has_bump_player = healthbar->enable_bump && (healthbar->bump_animation_opponent != NULL);
    int32_t res = 0;

    if (beatwatcher_poll(&healthbar->beatwatcher)) {
        since_beat = healthbar->beatwatcher.since;

        if (has_bump_opponent && animsprite_is_completed(healthbar->bump_animation_opponent))
            animsprite_restart(healthbar->bump_animation_opponent);

        if (has_bump_player && animsprite_is_completed(healthbar->bump_animation_player))
            animsprite_restart(healthbar->bump_animation_player);

        if (
            healthbar->enable_warnings &&
            healthbar->enable_flash_warning &&
            healthbar->last_health <= HEALTHBAR_LOW_HEALTH_PERCENT
        ) {
            healthbar->low_health_flash_warning = healthbar->beatwatcher.tick / HEALTHBAR_LOW_HEALTH_FLASH_RATIO;
        }

    } else if (healthbar->low_health_flash_warning > 0.0f) {
        healthbar->low_health_flash_warning -= elapsed;
    }

    if (has_bump_opponent) {
        res += animsprite_animate(healthbar->bump_animation_opponent, since_beat);
        animsprite_update_modifier(
            healthbar->bump_animation_opponent, &healthbar->bump_modifier_opponent, true
        );
    }

    if (has_bump_player) {
        res += animsprite_animate(healthbar->bump_animation_player, since_beat);
        animsprite_update_modifier(
            healthbar->bump_animation_player, &healthbar->bump_modifier_player, true
        );
    }

    if (healthbar->drawable_animation) {
        res += animsprite_animate(healthbar->drawable_animation, elapsed);
        animsprite_update_drawable(healthbar->drawable_animation, healthbar->drawable, true);
    }

    if (healthbar->transition_enabled) {
        res += tweenlerp_animate(healthbar->tweenlerp, elapsed);
        float health = tweenlerp_peek_value(healthbar->tweenlerp);
        healthbar_internal_calc_health_positions(healthbar, health);
    }

    res += statesprite_animate(healthbar->sprite_background, elapsed);

    res += statesprite_animate(healthbar->sprite_bar_opponent, elapsed);
    res += statesprite_animate(healthbar->sprite_icon_opponent, elapsed);

    res += statesprite_animate(healthbar->sprite_bar_player, elapsed);
    res += statesprite_animate(healthbar->sprite_icon_player, elapsed);

    res += statesprite_animate(healthbar->sprite_warnings, elapsed);

    return res;
}

void healthbar_draw(Healthbar healthbar, PVRContext pvrctx) {
    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(healthbar->drawable, pvrctx);

    int32_t has_low_warning_flash = healthbar->low_health_flash_warning > 0.0f;
    if (has_low_warning_flash) {
        pvr_context_save(pvrctx);
        pvr_context_set_global_offsetcolor(pvrctx, HEALTHBAR_LOW_HEALTH_WARN_COLOR);
    }

    if (healthbar->extra_enabled) {
        float x, y;
        if (healthbar->is_vertical) {
            x = 0.0f;
            y = healthbar->extra_translation;
        } else {
            x = healthbar->extra_translation;
            y = 0.0f;
        }
        sh4matrix_translate(pvrctx->current_matrix, x, y);
    }

    statesprite_draw(healthbar->sprite_background, pvrctx);
    statesprite_draw(healthbar->sprite_bar_opponent, pvrctx);
    statesprite_draw(healthbar->sprite_bar_player, pvrctx);

    healthbar_internal_draw_chrctr(
        pvrctx, &healthbar->bump_modifier_player, healthbar->sprite_icon_player
    );
    healthbar_internal_draw_chrctr(
        pvrctx, &healthbar->bump_modifier_opponent, healthbar->sprite_icon_opponent
    );

    if (healthbar->enable_warnings && healthbar->has_warning_drain) {
        statesprite_set_draw_location(
            healthbar->sprite_warnings, healthbar->warning_drain_x, healthbar->warning_drain_y
        );
        statesprite_draw(healthbar->sprite_warnings, pvrctx);
    }

    if (healthbar->enable_warnings && healthbar->has_warning_locked) {
        const float half_dimmen = healthbar->dimmen / 2.0f;
        float x, y;
        if (healthbar->is_vertical) {
            x = healthbar->modifier->y + half_dimmen;
            y = healthbar->warning_locked_position;
        } else {
            x = healthbar->warning_locked_position;
            y = healthbar->modifier->y + half_dimmen;
        }
        statesprite_set_draw_location(healthbar->sprite_warnings, x, y);
        statesprite_draw(healthbar->sprite_warnings, pvrctx);
    }

    if (healthbar->enable_warnings && healthbar->has_warning_opponent_recover > healthbar->beatwatcher.count) {
        statesprite_set_draw_location(
            healthbar->sprite_warnings, healthbar->warning_drain_x, healthbar->warning_drain_y
        );
        statesprite_draw(healthbar->sprite_warnings, pvrctx);
    }

    if (has_low_warning_flash) pvr_context_restore(pvrctx);
    pvr_context_restore(pvrctx);
}


void healthbar_disable_progress_animation(Healthbar healthbar, bool disable) {
    healthbar->transition_enabled = !disable;
}

float healthbar_set_health_position(Healthbar healthbar, float max_health, float health, bool opponent_recover) {
    // calculate the health percent
    health = math2d_inverselerp(0.0f, max_health, health);

    if (opponent_recover) {
        opponent_recover =
            healthbar->enable_warnings &&
            health < healthbar->last_health &&
            !healthbar->has_warning_drain &&
            !healthbar->has_warning_locked &&
            statesprite_state_toggle(healthbar->sprite_warnings, HEALTHBAR_WARNING_OPPONENT_RECOVER);
    }

    healthbar->has_warning_opponent_recover = opponent_recover ? (healthbar->beatwatcher.count + 2) : -1;

    if (!healthbar->transition_enabled || math2d_is_float_NaN(healthbar->last_health)) {
        tweenlerp_change_bounds_by_index(healthbar->tweenlerp, 0, -1.0f, health);
        tweenlerp_end(healthbar->tweenlerp);
        healthbar_internal_calc_health_positions(healthbar, health);
        return health;
    }

    bool is_completed = tweenlerp_change_bounds_by_index(
        healthbar->tweenlerp, 0, healthbar->last_health, health
    );

    if (is_completed) tweenlerp_restart(healthbar->tweenlerp);

    return health;
}

void healthbar_set_health_position2(Healthbar healthbar, float percent) {
    percent = math2d_clamp_float(percent, 0.0f, 1.0f);
    tweenlerp_change_bounds_by_index(healthbar->tweenlerp, 0, -1.0f, percent);
    tweenlerp_end(healthbar->tweenlerp);
    healthbar_internal_calc_health_positions(healthbar, percent);
}

void healthbar_disable_icon_overlap(Healthbar healthbar, bool disable) {
    healthbar->enable_overlap = !disable;
    healthbar_internal_calc_dimmensions(healthbar);

    float last_health = healthbar->last_health;
    healthbar->last_health = FLOAT_NaN;
    healthbar_internal_calc_health_positions(healthbar, last_health);
}

void healthbar_disable_warnings(Healthbar healthbar, bool disable) {
    healthbar->enable_warnings = !disable;
}

void healthbar_enable_low_health_flash_warning(Healthbar healthbar, bool enable) {
    healthbar->enable_flash_warning = !enable;
}



void healthbar_hide_warnings(Healthbar healthbar) {
    healthbar->has_warning_drain = false;
    healthbar->has_warning_locked = false;
    healthbar->low_health_flash_warning = false;
}

void healthbar_show_drain_warning(Healthbar healthbar, bool use_fast_drain) {
    healthbar_hide_warnings(healthbar);
    healthbar->has_warning_drain = statesprite_state_toggle(
        healthbar->sprite_warnings,
        use_fast_drain ? HEALTHBAR_WARNING_FAST_DRAIN : HEALTHBAR_WARNING_SLOW_DRAIN
    );
}

void healthbar_show_locked_warning(Healthbar healthbar) {
    healthbar_hide_warnings(healthbar);
    healthbar->has_warning_locked = statesprite_state_toggle(
        healthbar->sprite_warnings,
        HEALTHBAR_WARNING_LOCKED
    );
}


void healthbar_get_bar_midpoint(Healthbar healthbar, float* x, float* y) {
    float last_health_position_opponent = healthbar->last_health_position_opponent;

    if (math2d_is_float_NaN(last_health_position_opponent)) last_health_position_opponent = 0.0f;

    float dimmen = healthbar->dimmen / 2.0f;
    if (healthbar->extra_enabled) last_health_position_opponent += healthbar->extra_translation;

    *x = healthbar->modifier->x + healthbar->border;
    *y = healthbar->modifier->y + healthbar->border;

    if (healthbar->is_vertical) {
        *x += dimmen;
        *y += last_health_position_opponent;
    } else {
        *x += last_health_position_opponent;
        *y += dimmen;
    }
}

float healthbar_get_percent(Healthbar healthbar) {
    float health;
    if (healthbar->transition_enabled)
        health = tweenlerp_peek_value(healthbar->tweenlerp);
    else
        health = healthbar->last_health;

    return math2d_is_float_NaN(health) ? 0.0f : health;
}


static void healthbar_internal_calc_dimmensions(Healthbar healthbar) {
    int32_t resolution_bar_width = 0, resolution_bar_height = 0;
    int32_t resolution_icon_width = 0, resolution_icon_height = 0;

    float width, height;

    if (healthbar->is_vertical) {
        width = healthbar->dimmen;
        height = healthbar->length;
    } else {
        width = healthbar->length;
        height = healthbar->dimmen;
    }

    healthbar->modifier->width = width;
    healthbar->modifier->height = height;

    // resize & center background in the screen
    healthbar_internal_center_bar(
        healthbar, width, height, 0.0f,
        -1, -1, healthbar->sprite_background
    );

    // resize & center opponent health bar and icon
    healthbar_internal_load_resolutions(
        healthbar->resolutions_opponent, healthbar->selected_state_opponent,
        &resolution_bar_width, &resolution_bar_height, &resolution_icon_width, &resolution_icon_height
    );
    healthbar_internal_center_bar(
        healthbar, width, height, healthbar->border, resolution_bar_width, resolution_bar_height, healthbar->sprite_bar_opponent
    );
    healthbar_internal_center_icon(
        healthbar, true, resolution_icon_width, resolution_icon_height, healthbar->sprite_icon_opponent
    );

    // resize & center player health bar and icon
    healthbar_internal_load_resolutions(
        healthbar->resolutions_player, healthbar->selected_state_player,
        &resolution_bar_width, &resolution_bar_height, &resolution_icon_width, &resolution_icon_height
    );
    healthbar_internal_center_bar(
        healthbar, width, height, healthbar->border, resolution_bar_width, resolution_bar_height, healthbar->sprite_bar_player
    );
    healthbar_internal_center_icon(
        healthbar, false, resolution_icon_width, resolution_icon_height, healthbar->sprite_icon_player
    );

    // apply again the selected state
    statesprite_state_apply(healthbar->sprite_background, NULL);
    statesprite_state_apply(healthbar->sprite_bar_opponent, NULL);
    statesprite_state_apply(healthbar->sprite_bar_player, NULL);
    statesprite_state_apply(healthbar->sprite_icon_opponent, NULL);
    statesprite_state_apply(healthbar->sprite_icon_player, NULL);

    // flip icons (if was necessary), the player icon should be always flipped
    healthbar_internal_icon_flip(
        healthbar->sprite_icon_opponent, healthbar->is_vertical, healthbar->flip_icon_opponent
    );
    healthbar_internal_icon_flip(
        healthbar->sprite_icon_player, healthbar->is_vertical, healthbar->flip_icon_player
    );
}

static void healthbar_internal_calc_health_positions(Healthbar healthbar, float player_health) {
    if (player_health == healthbar->last_health) return;

    healthbar->last_health = player_health;

    if (healthbar->extra_enabled) {
        player_health = math2d_lerp(0.0f, 2.0f, player_health);
        if (player_health > 1.0f) {
            float extra_percent = player_health - 1.0f;
            float extra_length = healthbar->length / 2.0f;
            healthbar->extra_translation = math2d_lerp(0.0f, extra_length, extra_percent) * -1.0f;
            player_health = 1.0f;
        }
    }

    float opponent_health = 1.0f - player_health;

    float health_position_opponent = math2d_clamp_float(
        healthbar->health_bar_length * opponent_health,
        0.0f,
        healthbar->health_bar_length
    );
    float health_position_player = math2d_clamp_float(
        healthbar->health_bar_length * player_health,
        0.0f,
        healthbar->health_bar_length
    );


    const char *player_health_state, *opponent_health_state;

    if (player_health >= 1.0f) {
        player_health_state = HEALTHBAR_ICON_PREFIX_WINNER;
        opponent_health_state = HEALTHBAR_ICON_PREFIX_LOOSER;
    } else if (player_health <= HEALTHBAR_CHARACTER_WARNING_PERCENT) {
        player_health_state = HEALTHBAR_ICON_PREFIX_LOOSING;
        opponent_health_state = HEALTHBAR_ICON_PREFIX_WINNING;
    } else if (opponent_health <= HEALTHBAR_CHARACTER_WARNING_PERCENT) {
        player_health_state = HEALTHBAR_ICON_PREFIX_WINNING;
        opponent_health_state = HEALTHBAR_ICON_PREFIX_LOOSING;
    } else if (opponent_health >= 1.0f) {
        player_health_state = HEALTHBAR_ICON_PREFIX_LOOSER;
        opponent_health_state = HEALTHBAR_ICON_PREFIX_WINNER;
    } else {
        player_health_state = HEALTHBAR_ICON_PREFIX_NEUTRAL;
        opponent_health_state = HEALTHBAR_ICON_PREFIX_NEUTRAL;
    }

    // set the character new state
    healthbar_internal_set_chrctr_state(
        opponent_health_state, healthbar->selected_state_opponent,
        healthbar->sprite_bar_opponent, healthbar->sprite_icon_opponent
    );
    healthbar_internal_set_chrctr_state(
        player_health_state, healthbar->selected_state_player,
        healthbar->sprite_bar_player, healthbar->sprite_icon_player
    );

    // calculate the icon & bar positions
    healthbar_internal_calc_chrctr(
        healthbar, false,
        health_position_opponent, healthbar->sprite_bar_opponent, healthbar->sprite_icon_opponent
    );
    healthbar_internal_calc_chrctr(
        healthbar, true,
        health_position_player, healthbar->sprite_bar_player, healthbar->sprite_icon_player
    );

    // calc locked icon position on the bar
    healthbar->warning_locked_position = health_position_opponent + healthbar->border;
    if (healthbar->is_vertical)
        healthbar->warning_locked_position += healthbar->modifier->y;
    else
        healthbar->warning_locked_position += healthbar->modifier->x;

    // calc the top-right corner of the player icon
    float temp0 = 0.0f, temp1 = 0.0f;
    statesprite_get_draw_location(healthbar->sprite_icon_player, &temp0, &temp1);
    healthbar->warning_drain_x = temp0;
    healthbar->warning_drain_y = temp1;
    statesprite_get_draw_size(healthbar->sprite_icon_player, &temp0, &temp1);
    healthbar->warning_drain_x += temp0;

    if (healthbar->enable_overlap) {
        if (healthbar->is_vertical)
            healthbar->warning_drain_y -= healthbar->icon_overlap;
        else
            healthbar->warning_drain_x -= healthbar->icon_overlap;
    }

    healthbar->last_health_position_opponent = health_position_opponent;
}


static int32_t healthbar_internal_add_chrctr_state(StateSprite icon, StateSprite bar, ModelHolder icn_mdlhldr, ModelHolder hlth_mdlhldr, LinkedList rsltn, const char* state_name) {
    char* name;
    int32_t success = 0;

    name = string_concat_for_state_name(2, HEALTHBAR_ICON_PREFIX_WINNING, state_name);
    success += healthbar_internal_add_chrctr_state2(icon, bar, icn_mdlhldr, hlth_mdlhldr, name);
    free_chk(name);

    name = string_concat_for_state_name(2, HEALTHBAR_ICON_PREFIX_NEUTRAL, state_name);
    success += healthbar_internal_add_chrctr_state2(icon, bar, icn_mdlhldr, hlth_mdlhldr, name);
    free_chk(name);

    name = string_concat_for_state_name(2, HEALTHBAR_ICON_PREFIX_LOOSING, state_name);
    success += healthbar_internal_add_chrctr_state2(icon, bar, icn_mdlhldr, hlth_mdlhldr, name);
    free_chk(name);

    if (success > 0)
        healthbar_internal_save_resolutions(rsltn, state_name, hlth_mdlhldr, icn_mdlhldr);

    return success;
}

static int32_t healthbar_internal_add_chrctr_state2(StateSprite icon, StateSprite bar, ModelHolder icn_mdlhldr, ModelHolder hlth_mdlhldr, const char* sub_state_name) {
    int32_t success = 0;

    if (icn_mdlhldr && statesprite_state_add(icon, icn_mdlhldr, sub_state_name, sub_state_name))
        success++;
    if (hlth_mdlhldr && statesprite_state_add(bar, hlth_mdlhldr, sub_state_name, sub_state_name))
        success++;

    return success;
}


static void healthbar_internal_center_bar(Healthbar healthbar, float width, float height, float border, int32_t rsltn_width, int32_t rsltn_height, StateSprite statesprite) {
    bool has_borders = border > 0.0f;
    float double_border = border * 2.0f;

    foreach (StateSpriteState*, state, LINKEDLIST_ITERATOR, statesprite_state_list(statesprite)) {
        // if this state does not have texture or there are borders resize as-is
        if (has_borders || !state->texture) {
            // there borders in the four sides of the bar
            state->offset_x = state->offset_y = border;

            state->draw_width = width - double_border;
            state->draw_height = height - double_border;
            continue;
        }

        // textured bar, resize properly
        healthbar_internal_resize_state(
            healthbar, state, rsltn_width, rsltn_height, HEALTHBAR_DEFAULT_DIMMEN, HEALTHBAR_SCALE_TO_DIMMEN_ICON
        );

        state->offset_x = (width - state->draw_width) / 2.0f;
        state->offset_y = (height - state->draw_height) / 2.0f;
    }
}

static void healthbar_internal_center_icon(Healthbar healthbar, Align align, int32_t rsltn_width, int32_t rsltn_height, StateSprite statesprite) {
    float icon_overlap = healthbar->enable_overlap ? healthbar->icon_overlap : 0.0f;

    foreach (StateSpriteState*, state, LINKEDLIST_ITERATOR, statesprite_state_list(statesprite)) {

        healthbar_internal_resize_state(
            healthbar, state, rsltn_width, rsltn_height, HEALTHBAR_DEFAULT_ICON_SIZE, HEALTHBAR_SCALE_TO_DIMMEN_ICON
        );

        if (healthbar->is_vertical) {
            state->offset_x = (healthbar->dimmen - state->draw_width) / 2.0f;
            if (align) {
                state->offset_y = -state->draw_height;
                state->offset_y += icon_overlap;
            } else {
                state->offset_y = -icon_overlap;
            }
        } else {
            state->offset_y = (healthbar->dimmen - state->draw_height) / 2.0f;
            if (align) {
                state->offset_x = -state->draw_width;
                state->offset_x += icon_overlap;
            } else {
                state->offset_x = -icon_overlap;
            }
        }
    }
}

static void healthbar_internal_resize_state(Healthbar healthbar, StateSpriteState* state, int32_t rsltn_width, int32_t rsltn_height, int32_t def_size, bool scale_to_dimmen) {
    float orig_width = def_size, orig_height = def_size;

    imgutils_get_statesprite_original_size(state, &orig_width, &orig_height);

    if (rsltn_width >= 0 && rsltn_height >= 0) {
        // resize using the display resolution
        float scale = healthbar->layout_width / (float)rsltn_width;
        state->draw_width = orig_width * scale;
        state->draw_height = orig_height * scale;
    } else {
        // resize using the ratio of 1:N the health bar size
        float icon_dimmen = healthbar->dimmen * scale_to_dimmen;
        float width = -1.0f;
        float height = -1.0f;

        if (healthbar->is_vertical)
            width = icon_dimmen;
        else
            height = icon_dimmen;

        imgutils_calc_size(orig_width, orig_height, width, height, &orig_width, &orig_height);
        state->draw_width = orig_width;
        state->draw_height = orig_height;
    }
}

static void healthbar_internal_icon_flip(StateSprite statesprite, bool is_vertical, bool do_flip) {
    bool x, y;
    if (is_vertical) {
        x = false;
        y = do_flip;
    } else {
        x = do_flip;
        y = false;
    }
    statesprite_flip_texture(statesprite, x, y);
}

static int32_t healthbar_internal_toggle_chrctr_state(char** slctd_ptr, const char* prefix, const char* state_name, StateSprite bar, StateSprite icon) {
    if (/*pointer equals*/ *slctd_ptr != HEALTHBAR_INTERNAL_STATE_NAME) free_chk(*slctd_ptr);
    *slctd_ptr = string_duplicate(state_name);

    return healthbar_internal_toggle_chrctr_state2(prefix, state_name, bar, icon);
}

static int32_t healthbar_internal_toggle_chrctr_state2(const char* prefix, const char* state_name, StateSprite bar, StateSprite icon) {
    int32_t success = 0;
    char* sub_state_name = string_concat_for_state_name(2, prefix, state_name);

    success += statesprite_state_toggle(bar, sub_state_name) ? 1 : 0;
    success += statesprite_state_toggle(icon, sub_state_name) ? 1 : 0;

    free_chk(sub_state_name);
    return success;
}

static void healthbar_internal_set_chrctr_state(const char* prefix, const char* state_name, StateSprite bar, StateSprite icon) {
    //
    // confusing part, set the states in this order:
    //      neutral -> winning/loosing -> winner/looser
    //
    // must be done in this way in case there missing icons for non-neutral states
    //

    // step 1: default to "neutral"
    healthbar_internal_toggle_chrctr_state2(HEALTHBAR_ICON_PREFIX_NEUTRAL, state_name, bar, icon);

    if (string_equals(prefix, HEALTHBAR_ICON_PREFIX_NEUTRAL)) return;

    // step 2: default to "winning" or "loosing"
    if (string_equals(prefix, HEALTHBAR_ICON_PREFIX_WINNER))
        healthbar_internal_toggle_chrctr_state2(HEALTHBAR_ICON_PREFIX_WINNING, state_name, bar, icon);
    else if (string_equals(prefix, HEALTHBAR_ICON_PREFIX_LOOSER))
        healthbar_internal_toggle_chrctr_state2(HEALTHBAR_ICON_PREFIX_LOOSING, state_name, bar, icon);

    // step 3: set the state "winner" or "looser"
    healthbar_internal_toggle_chrctr_state2(prefix, state_name, bar, icon);
}

static void healthbar_internal_calc_chrctr(Healthbar healthbar, bool invert, float position, StateSprite bar, StateSprite icon) {
    float crop_width = -1.0f;
    float crop_height = -1.0f;
    float crop_x = 0.0f;
    float crop_y = 0.0f;

    if (invert) {
        // player bar, invert the position
        position = healthbar->health_bar_length - position;

        if (healthbar->is_vertical)
            crop_y = position;
        else
            crop_x = position;
    } else {
        if (healthbar->is_vertical)
            crop_height = position;
        else
            crop_width = position;
    }
    statesprite_crop(bar, crop_x, crop_y, crop_width, crop_height);


    float x = healthbar->modifier->x;
    float y = healthbar->modifier->y;

    // float half_dimmen = healthbar->dimmen / -2.0f;
    float half_dimmen = 0.0f; // mimics Funkin behavior

    if (healthbar->is_vertical) {
        x += half_dimmen;
        y += position;
    } else {
        x += position;
        y += half_dimmen;
    }
    statesprite_set_draw_location(icon, x, y);
}

static void healthbar_internal_draw_chrctr(PVRContext pvrctx, Modifier* bump_modifier, StateSprite icon) {
    pvr_context_save(pvrctx);

    float draw_width = 0.0f, draw_height = 0.0f;
    float draw_x = 0.0f, draw_y = 0.0f;
    float offset_x = 0.0f, offset_y = 0.0f;

    statesprite_get_draw_size(icon, &draw_width, &draw_height);
    statesprite_get_draw_location(icon, &draw_x, &draw_y);
    statesprite_state_get_offsets(icon, &offset_x, &offset_y);

    pvr_context_apply_modifier2(
        pvrctx, bump_modifier,
        draw_x + offset_x,
        draw_y + offset_y,
        draw_width,
        draw_height
    );
    statesprite_draw(icon, pvrctx);

    pvr_context_restore(pvrctx);
}

static bool healthbar_internal_add_warning(StateSprite sprite, ModelHolder modelholder, bool use_alt, float height, const char* state_name) {
    char* anim_name;

    if (use_alt)
        anim_name = string_concat(2, state_name, HEALTHBAR_WARNING_ALT_SUFFIX);
    else
        anim_name = string_duplicate(state_name);

    statesprite_state_remove(sprite, state_name);
    StateSpriteState* state = statesprite_state_add(sprite, modelholder, anim_name, state_name);

    free_chk(anim_name);

    if (!state) return false;

    float temp_width = 0.0f, temp_height = 0.0f;
    imgutils_get_statesprite_original_size(state, &temp_width, &temp_height);
    imgutils_calc_size(temp_width, temp_height, -1.0f, height, &temp_width, &temp_height);

    state->draw_width = temp_width;
    state->draw_height = temp_height;
    state->offset_x = state->draw_width / -2.0f;
    state->offset_y = state->draw_height / -2.0f;

    return true;
}

static void healthbar_internal_save_resolutions(LinkedList linkedlist, const char* state_name, ModelHolder mdlhldr_bar, ModelHolder mdlhldr_icn) {
    TextureResolution* item = malloc_chk(sizeof(TextureResolution));
    malloc_assert(item, TextureResolution);

    *item = (TextureResolution){
        .state_name = string_duplicate(state_name),
        .resolution_bar_width = 0,
        .resolution_bar_height = 0,
        .resolution_icon_width = 0,
        .resolution_icon_height = 0
    };

    modelholder_get_texture_resolution(mdlhldr_icn, &item->resolution_bar_width, &item->resolution_bar_height);
    modelholder_get_texture_resolution(mdlhldr_bar, &item->resolution_icon_width, &item->resolution_icon_height);

    linkedlist_add_item(linkedlist, item);
}

static void healthbar_internal_load_resolutions(LinkedList linkedlist, const char* state_name, int32_t* rsltn_bar_width, int32_t* rsltn_bar_height, int32_t* rsltn_icn_width, int32_t* rsltn_icn_height) {
    foreach (TextureResolution*, item, LINKEDLIST_ITERATOR, linkedlist) {
        if (string_equals(item->state_name, state_name)) {
            *rsltn_bar_width = item->resolution_bar_width;
            *rsltn_bar_height = item->resolution_bar_height;
            *rsltn_icn_width = item->resolution_icon_width;
            *rsltn_icn_height = item->resolution_icon_height;
            return;
        }
    }

    // this never should happen
    *rsltn_bar_width = FUNKIN_SCREEN_RESOLUTION_WIDTH;
    *rsltn_bar_height = FUNKIN_SCREEN_RESOLUTION_HEIGHT;
    *rsltn_icn_width = FUNKIN_SCREEN_RESOLUTION_WIDTH;
    *rsltn_icn_height = FUNKIN_SCREEN_RESOLUTION_HEIGHT;
}
