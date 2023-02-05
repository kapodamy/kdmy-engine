"use strict";

const HEALTHBAR_LENGTH_NORMAL = "normal";
const HEALTHBAR_LENGTH_EXTRA = "extra";
const HEALTHBAR_LENGTH_LONG = "long";

const HEALTHBAR_DEFAULT_COLOR_BACKGROUND = 0x000000;// black
const HEALTHBAR_DEFAULT_COLOR_DAD = 0xFF0000;// red
const HEALTHBAR_DEFAULT_COLOR_BOYFRIEND = 0x00FF00;// green


const HEALTHBAR_DEFAULT_DIMMEN = 20;// 20px in a 1280x720 screen
const HEALTHBAR_DEFAULT_BORDER_SIZE = 4;// 4px in a 1280x720 screen

const HEALTHBAR_DEFAULT_ICON_SIZE = 80;//80px in a 1280x720 screen

const HEALTHBAR_SCALE_TO_DIMMEN_ICON = 80 / HEALTHBAR_DEFAULT_DIMMEN;
const HEALTHBAR_SCALE_TO_DIMMEN_BORDER = 4 / HEALTHBAR_DEFAULT_DIMMEN;
const HEALTHBAR_SCALE_TO_DIMMEN_BAR = 1.0;

const HEALTHBAR_ICON_PREFIX_WINNER = "winner";
const HEALTHBAR_ICON_PREFIX_WINNING = "winning";
const HEALTHBAR_ICON_PREFIX_NEUTRAL = "neutral";
const HEALTHBAR_ICON_PREFIX_LOOSING = "loosing";
const HEALTHBAR_ICON_PREFIX_LOOSER = "looser";

const HEALTHBAR_WARNING_ALT_SUFFIX = " alt";
const HEALTHBAR_WARNING_FAST_DRAIN = "fastDrain";
const HEALTHBAR_WARNING_SLOW_DRAIN = "slowDrain";
const HEALTHBAR_WARNING_LOCKED = "locked";
const HEALTHBAR_WARNING_OPPONENT_RECOVER = "opponentRecover";

const HEALTHBAR_CHARACTER_WARNING_PERCENT = 0.25;// warn if less or equal to 25%
const HEALTHBAR_HEALTH_TRANSITION_RATIO = 8;// the transition duration in BMP/N

const HEALTHBAR_RATIO_SIZE_NORMAL = 600;// 600px in a 1280x720 screen
const HEALTHBAR_RATIO_SIZE_EXTRA = HEALTHBAR_RATIO_SIZE_NORMAL;
const HEALTHBAR_RATIO_SIZE_LONG = 1180;// 1180px in a 1280x70 screen

const HEALTHBAR_STUB_MODELHOLDER = {
    atlas: MODELHOLDER_STUB_ATLAS,
    animlist: MODELHOLDER_STUB_ANIMLIST,
    vertex_color_rgb8: 0x000000,
    texture: null,
    id: -1,
    instance_references: 0,
    instance_src: null
};

/**@type {RGBA}*/let HEALTHBAR_LOW_HEALTH_WARN_COLOR = [1.0, 0.0, 0.0, 0.5];// rgba: half-transparent red
const HEALTHBAR_LOW_HEALTH_PERCENT = 0.10;// warn if less or equal to 10%
const HEALTHBAR_LOW_HEALTH_FLASH_RATIO = 8;// active flash duration in BMP/N
const HEALTHBAR_UI_ICON_BUMP = "healthbar_icon_bump";// picked from UI animlist
const HEALTHBAR_WARNING_MODEL = "/assets/common/image/week-round/healthbar_warns.xml";


function healthbar_init(x, y, z, length, dimmen, border, icon_overlap, warn_height, lock_height) {
    let healthbar = {
        x, y, z, length, dimmen, border, icon_overlap, warn_height, lock_height,

        is_vertical: 0,
        enable_overlap: 1,

        sprite_background: statesprite_init_from_vertex_color(HEALTHBAR_DEFAULT_COLOR_BACKGROUND),

        sprite_bar_opponent: statesprite_init_from_vertex_color(HEALTHBAR_DEFAULT_COLOR_DAD),
        sprite_bar_player: statesprite_init_from_vertex_color(HEALTHBAR_DEFAULT_COLOR_BOYFRIEND),

        sprite_icon_opponent: statesprite_init_from_texture(null),
        sprite_icon_player: statesprite_init_from_texture(null),

        flip_icon_opponent: 0,
        flip_icon_player: 1,

        health_bar_length: length - (border * 2),
        extra_enabled: 0,
        extra_translation: 0,

        // note: in C replace "Symbol" with an unique constantselected_state_background: Symbol,
        selected_state_player: Symbol,
        selected_state_opponent: Symbol,

        bump_animation_opponent: null,
        bump_animation_player: null,

        bump_modifier_opponent: {},
        bump_modifier_player: {},
        enable_bump: 1,

        resolutions_player: linkedlist_init(),
        resolutions_opponent: linkedlist_init(),

        layout_width: FUNKIN_SCREEN_RESOLUTION_WIDTH,
        layout_height: FUNKIN_SCREEN_RESOLUTION_HEIGHT,

        beatwatcher: {},

        transition_enabled: 1,
        tweenlerp: tweenlerp_init(),

        last_health: NaN,
        last_health_position_opponent: NaN,
        prefix_state_player: HEALTHBAR_ICON_PREFIX_NEUTRAL,
        prefix_state_opponent: HEALTHBAR_ICON_PREFIX_NEUTRAL,

        drawable_animation: null,

        sprite_warnings: statesprite_init_from_texture(null),
        enable_warnings: 1,
        enable_flash_warning: 0,
        low_health_flash_warning: 0,

        has_warning_locked: 0,
        has_warning_drain: 0,
        has_warning_opponent_recover: -1,
        warning_locked_position: 0,
        warning_drain_x: 0,
        warning_drain_y: 0,

        modifier: null,
        drawable: {},

        first_init: 1
    };

    beatwatcher_reset(healthbar.beatwatcher, 1, 100);

    pvrctx_helper_clear_modifier(healthbar.bump_modifier_opponent);
    healthbar.bump_modifier_opponent.x = x;
    healthbar.bump_modifier_opponent.x = y;

    pvrctx_helper_clear_modifier(healthbar.bump_modifier_player);
    healthbar.bump_modifier_player.x = x;
    healthbar.bump_modifier_player.x = y;

    statesprite_set_draw_location(healthbar.sprite_icon_opponent, x, y);
    statesprite_set_draw_location(healthbar.sprite_icon_player, x, y);
    statesprite_set_draw_location(healthbar.sprite_background, x, y);
    statesprite_set_draw_location(healthbar.sprite_bar_opponent, x, y);
    statesprite_set_draw_location(healthbar.sprite_bar_player, x, y);

    // hide from the PVR backend, draw these sprites manually
    statesprite_set_visible(healthbar.sprite_icon_opponent, 0);
    statesprite_set_visible(healthbar.sprite_icon_player, 0);
    statesprite_set_visible(healthbar.sprite_background, 0);
    statesprite_set_visible(healthbar.sprite_bar_opponent, 0);
    statesprite_set_visible(healthbar.sprite_bar_player, 0);

    statesprite_crop_enable(healthbar.sprite_bar_opponent, 1);
    statesprite_crop_enable(healthbar.sprite_bar_player, 1);

    healthbar.drawable = drawable_init(z, healthbar, healthbar_draw, healthbar_animate);

    healthbar.modifier = drawable_get_modifier(healthbar.drawable);
    healthbar.modifier.x = x;
    healthbar.modifier.y = y;

    let transition_ms = healthbar.beatwatcher.tick / HEALTHBAR_HEALTH_TRANSITION_RATIO;
    tweenlerp_add_easeout(healthbar.tweenlerp, -1, 0, 0, transition_ms);

    return healthbar;
}

function healthbar_destroy(healthbar) {
    ModuleLuaScript.kdmyEngine_drop_shared_object(healthbar);

    drawable_destroy(healthbar.drawable);
    if (healthbar.drawable_animation) animsprite_destroy(healthbar.drawable_animation);

    if (healthbar.selected_state_player != Symbol) healthbar.selected_state_player = undefined;
    if (healthbar.selected_state_opponent != Symbol) healthbar.selected_state_opponent = undefined;

    tweenlerp_destroy(healthbar.tweenlerp);

    if (healthbar.bump_animation_opponent) animsprite_destroy(healthbar.bump_animation_opponent);
    if (healthbar.bump_animation_player) animsprite_destroy(healthbar.bump_animation_player);

    statesprite_destroy(healthbar.sprite_background);
    statesprite_destroy(healthbar.sprite_bar_opponent);
    statesprite_destroy(healthbar.sprite_bar_player);
    statesprite_destroy(healthbar.sprite_icon_opponent);
    statesprite_destroy(healthbar.sprite_icon_player);

    statesprite_destroy(healthbar.sprite_warnings);

    // dispose the resolution list, list items and state names

    for (let item of linkedlist_iterate4(healthbar.resolutions_opponent)) {
        item.state_name = undefined;
        item = undefined;
    }
    linkedlist_destroy(healthbar.resolutions_opponent);

    for (let item of linkedlist_iterate4(healthbar.resolutions_player)) {
        item.state_name = undefined;
        item = undefined;
    }
    linkedlist_destroy(healthbar.resolutions_player);

    healthbar = undefined;
}


function healthbar_set_layout_size(healthbar, width, height) {
    healthbar.layout_width = width;
    healthbar.layout_height = height;
}

function healthbar_enable_extra_length(healthbar, extra_enabled) {
    healthbar.extra_enabled = !!extra_enabled;

    // recalculate everything
    let last_health = healthbar.last_health;
    healthbar.last_health = NaN;
    if (isNaN(last_health)) return;
    healthbar_internal_calc_health_positions(healthbar, last_health);
}

function healthbar_enable_vertical(healthbar, enable_vertical) {
    healthbar.is_vertical = !!enable_vertical;
    healthbar_internal_calc_dimmensions(healthbar);
}


function healthbar_state_opponent_add(healthbar, icon_mdlhldr, bar_mdlhldr, state_name) {
    return healthbar_internal_add_chrctr_state(
        healthbar.sprite_icon_opponent, healthbar.sprite_bar_opponent,
        icon_mdlhldr, bar_mdlhldr,
        healthbar.resolutions_opponent,
        state_name
    );
}

function healthbar_state_opponent_add2(healthbar, icon_mdlhldr, bar_color_rgb8, state_name) {
    HEALTHBAR_STUB_MODELHOLDER.vertex_color_rgb8 = bar_color_rgb8 | 0x00;
    return healthbar_internal_add_chrctr_state(
        healthbar.sprite_icon_opponent, healthbar.sprite_bar_opponent,
        icon_mdlhldr, HEALTHBAR_STUB_MODELHOLDER,
        healthbar.resolutions_opponent,
        state_name
    );
}

function healthbar_state_player_add(healthbar, icon_mdlhldr, bar_mdlhldr, state_name) {
    return healthbar_internal_add_chrctr_state(
        healthbar.sprite_icon_player, healthbar.sprite_bar_player,
        icon_mdlhldr, bar_mdlhldr,
        healthbar.resolutions_player,
        state_name
    );
}

function healthbar_state_player_add2(healthbar, icon_modelholder, bar_color_rgb8, state_name) {
    HEALTHBAR_STUB_MODELHOLDER.vertex_color_rgb8 = bar_color_rgb8 | 0x00;
    return healthbar_internal_add_chrctr_state(
        healthbar.sprite_icon_player, healthbar.sprite_bar_player,
        icon_modelholder, HEALTHBAR_STUB_MODELHOLDER,
        healthbar.resolutions_player,
        state_name
    );
}


function healthbar_state_background_add(healthbar, modelholder, state_name) {
    let state = statesprite_state_add(healthbar.sprite_background, modelholder, state_name, state_name);
    return state ? 1 : 0;
}

function healthbar_state_background_add2(healthbar, color_rgb8, animsprite, state_name) {
    animsprite = animsprite ? animsprite_clone(animsprite) : null;
    let state = statesprite_state_add2(
        healthbar.sprite_background, null, animsprite, null, color_rgb8, state_name
    );
    if (!state && animsprite) animsprite_destroy(animsprite);
    return state ? 1 : 0;
}

function healthbar_load_warnings(healthbar, modelholder, use_alt_icons) {
    let success = 0;

    success += healthbar_internal_add_warning(
        healthbar.sprite_warnings, modelholder, use_alt_icons, healthbar.warn_height,
        HEALTHBAR_WARNING_FAST_DRAIN
    );
    success += healthbar_internal_add_warning(
        healthbar.sprite_warnings, modelholder, use_alt_icons, healthbar.warn_height,
        HEALTHBAR_WARNING_SLOW_DRAIN
    );
    success += healthbar_internal_add_warning(
        healthbar.sprite_warnings, modelholder, use_alt_icons, healthbar.lock_height,
        HEALTHBAR_WARNING_LOCKED
    );
    success += healthbar_internal_add_warning(
        healthbar.sprite_warnings, modelholder, use_alt_icons, healthbar.warn_height,
        HEALTHBAR_WARNING_OPPONENT_RECOVER
    );

    return success >= 4;
}


function healthbar_set_opponent_bar_color_rgb8(healthbar, color_rgb8) {
    statesprite_set_vertex_color_rgb8(healthbar.sprite_bar_opponent, color_rgb8);
}

function healthbar_set_opponent_bar_color(healthbar, r, g, b) {
    statesprite_set_vertex_color(healthbar.sprite_bar_opponent, r, g, b);
}

function healthbar_set_player_bar_color_rgb8(healthbar, color_rgb8) {
    statesprite_set_vertex_color_rgb8(healthbar.sprite_bar_player, color_rgb8);
}

function healthbar_set_player_bar_color(healthbar, r, g, b) {
    statesprite_set_vertex_color(healthbar.sprite_bar_player, r, g, b);
}


function healthbar_state_toggle(healthbar, state_name) {
    let success = 0;
    success += healthbar_state_toggle_background(healthbar, state_name);
    success += healthbar_state_toggle_player(healthbar, state_name);
    success += healthbar_state_toggle_opponent(healthbar, state_name);

    if (healthbar.first_init) {
        healthbar.first_init = 0;
        healthbar_internal_calc_dimmensions(healthbar);
    }

    return success;
}

function healthbar_state_toggle_background(healthbar, state_name) {
    return statesprite_state_toggle(healthbar.sprite_background, state_name);
}

function healthbar_state_toggle_player(healthbar, state_name) {
    return healthbar_internal_toggle_chrctr_state(
        healthbar, "selected_state_player", healthbar.prefix_state_player,
        state_name, healthbar.sprite_bar_player, healthbar.sprite_icon_player
    );
}

function healthbar_state_toggle_opponent(healthbar, state_name) {
    return healthbar_internal_toggle_chrctr_state(
        healthbar, "selected_state_opponent", healthbar.prefix_state_opponent,
        state_name, healthbar.sprite_bar_opponent, healthbar.sprite_icon_opponent
    );
}


function healthbar_set_bump_animation(healthbar, animlist) {
    if (!animlist) return;
    let animlist_item = animlist_get_animation(animlist, HEALTHBAR_UI_ICON_BUMP);

    if (!animlist_item) return;
    let animsprite = animsprite_init(animlist_item);

    if (healthbar.bump_animation_opponent) animsprite_destroy(healthbar.bump_animation_opponent);
    if (healthbar.bump_animation_player) animsprite_destroy(healthbar.bump_animation_player);

    healthbar_set_bump_animation_opponent(healthbar, animsprite);
    healthbar_set_bump_animation_player(healthbar, animsprite);
    if (animsprite != null) animsprite_destroy(animsprite);
}

function healthbar_set_bump_animation_opponent(healthbar, animsprite) {
    if (healthbar.bump_animation_opponent) animsprite_destroy(healthbar.bump_animation_opponent);
    healthbar.bump_animation_opponent = animsprite ? animsprite_clone(animsprite) : null;
}

function healthbar_set_bump_animation_player(healthbar, animsprite) {
    if (healthbar.bump_animation_player) animsprite_destroy(healthbar.bump_animation_player);
    healthbar.bump_animation_player = animsprite ? animsprite_clone(animsprite) : null;
}

function healthbar_bump_enable(healthbar, enable_bump) {
    healthbar.enable_bump = !!enable_bump;
}


function healthbar_set_bpm(healthbar, beats_per_minute) {
    beatwatcher_change_bpm(healthbar.beatwatcher, beats_per_minute);

    let transition_ms = healthbar.beatwatcher.tick / HEALTHBAR_HEALTH_TRANSITION_RATIO;
    tweenlerp_change_duration_by_index(healthbar.tweenlerp, 0, transition_ms);
}


function healthbar_set_offsetcolor(healthbar, r, g, b, a) {
    drawable_set_offsetcolor(healthbar.drawable, r, g, b, a);
}

function healthbar_set_alpha(healthbar, alpha) {
    drawable_set_alpha(healthbar.drawable, alpha);
}

function healthbar_set_visible(healthbar, visible) {
    drawable_set_visible(healthbar.drawable, visible);
}

function healthbar_get_modifier(healthbar) {
    return drawable_get_modifier(healthbar.drawable);
}

function healthbar_get_drawable(healthbar) {
    return healthbar.drawable;
}


function healthbar_animation_set(healthbar, animsprite) {
    if (healthbar.drawable_animation) animsprite_destroy(healthbar.drawable_animation);
    healthbar.drawable_animation = animsprite ? animsprite_clone(animsprite) : null;
}

function healthbar_animation_restart(healthbar) {
    if (healthbar.bump_animation_opponent) animsprite_restart(healthbar.bump_animation_opponent);
    if (healthbar.bump_animation_player) animsprite_restart(healthbar.bump_animation_player);
    if (healthbar.drawable_animation) animsprite_restart(healthbar.drawable_animation);

    statesprite_animation_restart(healthbar.sprite_background);
    statesprite_animation_restart(healthbar.sprite_bar_opponent);
    statesprite_animation_restart(healthbar.sprite_icon_opponent);
    statesprite_animation_restart(healthbar.sprite_bar_player);
    statesprite_animation_restart(healthbar.sprite_icon_player);
}

function healthbar_animation_end(healthbar) {
    if (healthbar.bump_animation_opponent) {
        animsprite_force_end(healthbar.bump_animation_opponent);
        animsprite_update_modifier(this.bump_animation_opponent, this.bump_modifier_opponent, 1);
    }
    if (healthbar.bump_animation_player) {
        animsprite_force_end(healthbar.bump_animation_player);
        animsprite_update_modifier(this.bump_animation_player, this.bump_modifier_player, 1);
    }
    if (healthbar.drawable_animation) {
        animsprite_force_end(healthbar.drawable_animation);
        animsprite_update_drawable(this.drawable_animation, this.drawable, 1);
    }

    statesprite_animation_end(healthbar.sprite_background);
    statesprite_animation_end(healthbar.sprite_bar_opponent);
    statesprite_animation_end(healthbar.sprite_icon_opponent);
    statesprite_animation_end(healthbar.sprite_bar_player);
    statesprite_animation_end(healthbar.sprite_icon_player);
}


function healthbar_animate(healthbar, elapsed) {
    let since_beat = elapsed;
    let has_bump_opponent = healthbar.enable_bump && healthbar.bump_animation_opponent != null;
    let has_bump_player = healthbar.enable_bump && healthbar.bump_animation_opponent != null;
    let res = 0;

    if (beatwatcher_poll(healthbar.beatwatcher)) {
        since_beat = healthbar.beatwatcher.since;

        if (has_bump_opponent && animsprite_is_completed(healthbar.bump_animation_opponent))
            animsprite_restart(healthbar.bump_animation_opponent);

        if (has_bump_player && animsprite_is_completed(healthbar.bump_animation_player))
            animsprite_restart(healthbar.bump_animation_player);

        if (
            healthbar.enable_warnings &&
            healthbar.enable_flash_warning &&
            healthbar.last_health <= HEALTHBAR_LOW_HEALTH_PERCENT
        ) {
            healthbar.low_health_flash_warning = healthbar.beatwatcher.tick / HEALTHBAR_LOW_HEALTH_FLASH_RATIO;
        }

    } else if (healthbar.low_health_flash_warning > 0) {
        healthbar.low_health_flash_warning -= elapsed;
    }

    if (has_bump_opponent) {
        res += animsprite_animate(healthbar.bump_animation_opponent, since_beat);
        animsprite_update_modifier(
            healthbar.bump_animation_opponent, healthbar.bump_modifier_opponent, 1
        );
    }

    if (has_bump_player) {
        res += animsprite_animate(healthbar.bump_animation_player, since_beat);
        animsprite_update_modifier(
            healthbar.bump_animation_player, healthbar.bump_modifier_player, 1
        );
    }

    if (healthbar.drawable_animation) {
        res += animsprite_animate(healthbar.drawable_animation, elapsed);
        animsprite_update_drawable(healthbar.drawable_animation, healthbar.drawable, 1);
    }

    if (healthbar.transition_enabled) {
        res += tweenlerp_animate(healthbar.tweenlerp, elapsed);
        let health = tweenlerp_peek_value(healthbar.tweenlerp);
        healthbar_internal_calc_health_positions(healthbar, health);
    }

    res += statesprite_animate(healthbar.sprite_background, elapsed);

    res += statesprite_animate(healthbar.sprite_bar_opponent, elapsed);
    res += statesprite_animate(healthbar.sprite_icon_opponent, elapsed);

    res += statesprite_animate(healthbar.sprite_bar_player, elapsed);
    res += statesprite_animate(healthbar.sprite_icon_player, elapsed);

    res += statesprite_animate(healthbar.sprite_warnings, elapsed);

    return res;
}

function healthbar_draw(healthbar, pvrctx) {
    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(healthbar.drawable, pvrctx);

    let has_low_warning_flash = healthbar.low_health_flash_warning > 0;
    if (has_low_warning_flash) {
        pvr_context_save(pvrctx);
        pvr_context_set_global_offsetcolor(pvrctx, HEALTHBAR_LOW_HEALTH_WARN_COLOR);
    }

    if (healthbar.extra_enabled) {
        let x, y;
        if (healthbar.is_vertical) {
            x = 0;
            y = healthbar.extra_translation;
        } else {
            x = healthbar.extra_translation;
            y = 0;
        }
        sh4matrix_translate(pvrctx.current_matrix, x, y);
    }

    statesprite_draw(healthbar.sprite_background, pvrctx);
    statesprite_draw(healthbar.sprite_bar_opponent, pvrctx);
    statesprite_draw(healthbar.sprite_bar_player, pvrctx);

    healthbar_internal_draw_chrctr(
        pvrctx, healthbar.bump_modifier_player, healthbar.sprite_icon_player
    );
    healthbar_internal_draw_chrctr(
        pvrctx, healthbar.bump_modifier_opponent, healthbar.sprite_icon_opponent
    );

    if (healthbar.enable_warnings && healthbar.has_warning_drain) {
        statesprite_set_draw_location(
            healthbar.sprite_warnings, healthbar.warning_drain_x, healthbar.warning_drain_y
        );
        statesprite_draw(healthbar.sprite_warnings, pvrctx);
    }

    if (healthbar.enable_warnings && healthbar.has_warning_locked) {
        const half_dimmen = healthbar.dimmen / 2;
        let x, y;
        if (healthbar.is_vertical) {
            x = healthbar.modifier.y + half_dimmen;
            y = healthbar.warning_locked_position;
        } else {
            x = healthbar.warning_locked_position;
            y = healthbar.modifier.y + half_dimmen;
        }
        statesprite_set_draw_location(healthbar.sprite_warnings, x, y);
        statesprite_draw(healthbar.sprite_warnings, pvrctx);
    }

    if (healthbar.enable_warnings && healthbar.has_warning_opponent_recover > healthbar.beatwatcher.count) {
        statesprite_set_draw_location(
            healthbar.sprite_warnings, healthbar.warning_drain_x, healthbar.warning_drain_y
        );
        statesprite_draw(healthbar.sprite_warnings, pvrctx);
    }

    if (has_low_warning_flash) pvr_context_restore(pvrctx);
    pvr_context_restore(pvrctx);
}


function healthbar_disable_progress_animation(healthbar, disable) {
    healthbar.transition_enabled = !disable;
}

function healthbar_set_health_position(healthbar, max_health, health, opponent_recover) {
    // calculate the health percent
    health = math2d_inverselerp(0.0, max_health, health);

    if (opponent_recover) {
        opponent_recover =
            healthbar.enable_warnings &&
            health < healthbar.last_health &&
            !healthbar.has_warning_drain &&
            !healthbar.has_warning_locked &&
            statesprite_state_toggle(healthbar.sprite_warnings, HEALTHBAR_WARNING_OPPONENT_RECOVER)
            ;
    }

    healthbar.has_warning_opponent_recover = opponent_recover ? (healthbar.beatwatcher.count + 2) : -1;

    if (!healthbar.transition_enabled || isNaN(healthbar.last_health)) {
        tweenlerp_change_bounds_by_index(healthbar.tweenlerp, 0, -1, health);
        tweenlerp_end(healthbar.tweenlerp);
        healthbar_internal_calc_health_positions(healthbar, health);
        return health;
    }

    let is_completed = tweenlerp_change_bounds_by_index(
        healthbar.tweenlerp, 0, healthbar.last_health, health
    );

    if (is_completed) tweenlerp_restart(healthbar.tweenlerp);

    return health;
}

function healthbar_set_health_position2(healthbar, percent) {
    percent = math2d_clamp(percent, 0.0, 1.0);
    tweenlerp_change_bounds_by_index(healthbar.tweenlerp, 0, -1, percent);
    tweenlerp_end(healthbar.tweenlerp);
    healthbar_internal_calc_health_positions(healthbar, percent);
}

function healthbar_disable_icon_overlap(healthbar, disable) {
    healthbar.enable_overlap = !disable;
    healthbar_internal_calc_dimmensions(healthbar);

    let last_health = healthbar.last_health;
    healthbar.last_health = NaN;
    healthbar_internal_calc_health_positions(healthbar, last_health);
}

function healthbar_disable_warnings(healthbar, disable) {
    healthbar.enable_warnings = !disable;
}

function healthbar_enable_low_health_flash_warning(healthbar, enable) {
    healthbar.enable_flash_warning = !enable;
}



function healthbar_hide_warnings(healthbar) {
    healthbar.has_warning_drain = 0;
    healthbar.has_warning_locked = 0;
    healthbar.low_health_flash_warning = 0;
}

function healthbar_show_drain_warning(healthbar, use_fast_drain) {
    healthbar_hide_warnings(healthbar);
    healthbar.has_warning_drain = statesprite_state_toggle(
        healthbar.sprite_warnings,
        use_fast_drain ? HEALTHBAR_WARNING_FAST_DRAIN : HEALTHBAR_WARNING_SLOW_DRAIN
    );
}

function healthbar_show_locked_warning(healthbar) {
    healthbar_hide_warnings(healthbar);
    healthbar.has_warning_locked = statesprite_state_toggle(
        healthbar.sprite_warnings,
        HEALTHBAR_WARNING_LOCKED
    );
}


function healthbar_get_bar_midpoint(healthbar, output_location) {
    let last_health_position_opponent = healthbar.last_health_position_opponent;

    if (Number.isNaN(last_health_position_opponent)) last_health_position_opponent = 0.0;

    let dimmen = healthbar.dimmen / 2.0;
    if (healthbar.extra_enabled) last_health_position_opponent += healthbar.extra_translation;

    let x = healthbar.modifier.x + healthbar.border;
    let y = healthbar.modifier.y + healthbar.border;

    if (healthbar.is_vertical) {
        x += dimmen;
        y += last_health_position_opponent;
    } else {
        x += last_health_position_opponent;
        y += dimmen;
    }

    output_location[0] = x;
    output_location[1] = y;
}

function healthbar_get_percent(healthbar) {
    let health;
    if (healthbar.transition_enabled)
        health = tweenlerp_peek_value(healthbar.tweenlerp);
    else
        health = healthbar.last_health;

    return Number.isNaN(health) ? 0.0 : health;
}


function healthbar_internal_calc_dimmensions(healthbar) {
    const resolution_bar = [0, 0];
    const resolution_icon = [0, 0];

    let width, height;

    if (healthbar.is_vertical) {
        width = healthbar.dimmen;
        height = healthbar.length;
    } else {
        width = healthbar.length;
        height = healthbar.dimmen;
    }

    healthbar.modifier.width = width;
    healthbar.modifier.height = height;

    // resize & center background in the screen
    const null_resolution = [-1, -1];
    healthbar_internal_center_bar(
        healthbar, width, height, 0,
        null_resolution, healthbar.sprite_background
    );

    // resize & center opponent health bar and icon
    healthbar_internal_load_resolutions(
        healthbar.resolutions_opponent, healthbar.selected_state_opponent,
        resolution_bar, resolution_icon
    );
    healthbar_internal_center_bar(
        healthbar, width, height, healthbar.border, resolution_bar, healthbar.sprite_bar_opponent
    );
    healthbar_internal_center_icon(
        healthbar, 1, resolution_icon, healthbar.sprite_icon_opponent
    );

    // resize & center player health bar and icon
    healthbar_internal_load_resolutions(
        healthbar.resolutions_player, healthbar.selected_state_player,
        resolution_bar, resolution_icon
    );
    healthbar_internal_center_bar(
        healthbar, width, height, healthbar.border, resolution_bar, healthbar.sprite_bar_player
    );
    healthbar_internal_center_icon(
        healthbar, 0, resolution_icon, healthbar.sprite_icon_player
    );

    // apply again the selected state
    statesprite_state_apply(healthbar.sprite_background, null);
    statesprite_state_apply(healthbar.sprite_bar_opponent, null);
    statesprite_state_apply(healthbar.sprite_bar_player, null);
    statesprite_state_apply(healthbar.sprite_icon_opponent, null);
    statesprite_state_apply(healthbar.sprite_icon_player, null);

    // flip icons (if was necessary), the player icon should be always flipped
    healthbar_internal_icon_flip(
        healthbar.sprite_icon_opponent, healthbar.is_vertical, healthbar.flip_icon_opponent
    );
    healthbar_internal_icon_flip(
        healthbar.sprite_icon_player, healthbar.is_vertical, healthbar.flip_icon_player
    );
}

function healthbar_internal_calc_health_positions(healthbar, player_health) {
    if (player_health == healthbar.last_health) return;

    healthbar.last_health = player_health;

    if (healthbar.extra_enabled) {
        player_health = math2d_lerp(0.0, 2.0, player_health);
        if (player_health > 1.0) {
            let extra_percent = player_health - 1.0;
            let extra_length = healthbar.length / 2;
            healthbar.extra_translation = math2d_lerp(0, extra_length, extra_percent) * -1.0;
            player_health = 1.0;
        }
    }

    let opponent_health = 1.0 - player_health;

    let health_position_opponent = math2d_clamp(
        healthbar.health_bar_length * opponent_health,
        0,
        healthbar.health_bar_length
    );
    let health_position_player = math2d_clamp(
        healthbar.health_bar_length * player_health,
        0,
        healthbar.health_bar_length
    );


    let player_health_state, opponent_health_state;

    if (player_health >= 1.0) {
        player_health_state = HEALTHBAR_ICON_PREFIX_WINNER;
        opponent_health_state = HEALTHBAR_ICON_PREFIX_LOOSER;
    } else if (player_health <= HEALTHBAR_CHARACTER_WARNING_PERCENT) {
        player_health_state = HEALTHBAR_ICON_PREFIX_LOOSING;
        opponent_health_state = HEALTHBAR_ICON_PREFIX_WINNING;
    } else if (opponent_health <= HEALTHBAR_CHARACTER_WARNING_PERCENT) {
        player_health_state = HEALTHBAR_ICON_PREFIX_WINNING;
        opponent_health_state = HEALTHBAR_ICON_PREFIX_LOOSING;
    } else if (opponent_health >= 1.0) {
        player_health_state = HEALTHBAR_ICON_PREFIX_LOOSER;
        opponent_health_state = HEALTHBAR_ICON_PREFIX_WINNER;
    } else {
        player_health_state = HEALTHBAR_ICON_PREFIX_NEUTRAL;
        opponent_health_state = HEALTHBAR_ICON_PREFIX_NEUTRAL;
    }

    // set the character new state
    healthbar_internal_set_chrctr_state(
        opponent_health_state, healthbar.selected_state_opponent,
        healthbar.sprite_bar_opponent, healthbar.sprite_icon_opponent
    );
    healthbar_internal_set_chrctr_state(
        player_health_state, healthbar.selected_state_player,
        healthbar.sprite_bar_player, healthbar.sprite_icon_player
    );

    // calculate the icon & bar positions
    healthbar_internal_calc_chrctr(
        healthbar, 0,
        health_position_opponent, healthbar.sprite_bar_opponent, healthbar.sprite_icon_opponent
    );
    healthbar_internal_calc_chrctr(
        healthbar, 1,
        health_position_player, healthbar.sprite_bar_player, healthbar.sprite_icon_player
    );

    // calc locked icon position on the bar
    healthbar.warning_locked_position = health_position_opponent + healthbar.border;
    if (healthbar.is_vertical) healthbar.warning_locked_position += healthbar.modifier.y;
    else healthbar.warning_locked_position += healthbar.modifier.x;

    // calc the top-right corner of the player icon
    const temp = [0, 0];
    statesprite_get_draw_location(healthbar.sprite_icon_player, temp);
    healthbar.warning_drain_x = temp[0];
    healthbar.warning_drain_y = temp[1];
    statesprite_get_draw_size(healthbar.sprite_icon_player, temp);
    healthbar.warning_drain_x += temp[0];

    if (healthbar.enable_overlap) {
        if (healthbar.is_vertical) healthbar.warning_drain_y -= healthbar.icon_overlap;
        else healthbar.warning_drain_x -= healthbar.icon_overlap;
    }

    healthbar.last_health_position_opponent = health_position_opponent;
}


function healthbar_internal_add_chrctr_state(icon, bar, icn_mdlhldr, hlth_mdlhldr, rsltn, state_name) {
    let name;
    let success = 0;

    name = string_concat_for_state_name(2, HEALTHBAR_ICON_PREFIX_WINNING, state_name);
    success += healthbar_internal_add_chrctr_state2(icon, bar, icn_mdlhldr, hlth_mdlhldr, name);
    name = undefined;

    name = string_concat_for_state_name(2, HEALTHBAR_ICON_PREFIX_NEUTRAL, state_name);
    success += healthbar_internal_add_chrctr_state2(icon, bar, icn_mdlhldr, hlth_mdlhldr, name);
    name = undefined;

    name = string_concat_for_state_name(2, HEALTHBAR_ICON_PREFIX_LOOSING, state_name);
    success += healthbar_internal_add_chrctr_state2(icon, bar, icn_mdlhldr, hlth_mdlhldr, name);
    name = undefined;

    if (success > 0)
        healthbar_internal_save_resolutions(rsltn, state_name, hlth_mdlhldr, icn_mdlhldr);

    return success;
}

function healthbar_internal_add_chrctr_state2(icon, bar, icn_mdlhldr, hlth_mdlhldr, sub_state_name) {
    let success = 0;

    if (icn_mdlhldr && statesprite_state_add(icon, icn_mdlhldr, sub_state_name, sub_state_name))
        success++;
    if (hlth_mdlhldr && statesprite_state_add(bar, hlth_mdlhldr, sub_state_name, sub_state_name))
        success++;

    return success;
}


function healthbar_internal_center_bar(healthbar, width, height, border, resolution, statesprite) {
    let has_borders = border > 0;
    let double_border = border * 2;

    for (let state of linkedlist_iterate4(statesprite_state_list(statesprite))) {
        // if this state does not have texture or there are borders resize as-is
        if (has_borders || !state.texture) {
            // there borders in the four sides of the bar
            state.offset_x = state.offset_y = border;

            state.draw_width = width - double_border;
            state.draw_height = height - double_border;
            continue;
        }

        // textured bar, resize properly
        healthbar_internal_resize_state(
            healthbar, state, resolution, HEALTHBAR_DEFAULT_DIMMEN, HEALTHBAR_SCALE_TO_DIMMEN_ICON
        );

        state.offset_x = (width - state.draw_width) / 2;
        state.offset_y = (height - state.draw_height) / 2;
    }
}

function healthbar_internal_center_icon(healthbar, align, resolution, statesprite) {
    let icon_overlap = healthbar.enable_overlap ? healthbar.icon_overlap : 0;

    for (let state of linkedlist_iterate4(statesprite_state_list(statesprite))) {

        healthbar_internal_resize_state(
            healthbar, state, resolution, HEALTHBAR_DEFAULT_ICON_SIZE, HEALTHBAR_SCALE_TO_DIMMEN_ICON
        );

        if (healthbar.is_vertical) {
            state.offset_x = (healthbar.dimmen - state.draw_width) / 2.0;
            if (align) {
                state.offset_y = -state.draw_height;
                state.offset_y += icon_overlap;
            } else {
                state.offset_y = -icon_overlap;
            }
        } else {
            state.offset_y = (healthbar.dimmen - state.draw_height) / 2.0;
            if (align) {
                state.offset_x = -state.draw_width;
                state.offset_x += icon_overlap;
            } else {
                state.offset_x = -icon_overlap;
            }
        }
    }
}

function healthbar_internal_resize_state(healthbar, state, resolution, def_size, scale_to_dimmen) {
    const orig_size = [def_size, def_size];

    imgutils_get_statesprite_original_size(state, orig_size);

    if (resolution[0] >= 0 && resolution[1] >= 0) {
        // resize using the display resolution
        let scale = healthbar.layout_width / resolution[0];
        state.draw_width = orig_size[0] * scale;
        state.draw_height = orig_size[1] * scale;
    } else {
        // resize using the ratio of 1:N the health bar size
        let icon_dimmen = healthbar.dimmen * scale_to_dimmen;
        let width = -1;
        let height = -1;

        if (healthbar.is_vertical) width = icon_dimmen;
        else height = icon_dimmen;

        imgutils_calc_size(orig_size[0], orig_size[1], width, height, orig_size);
        state.draw_width = orig_size[0];
        state.draw_height = orig_size[1];
    }
}

function healthbar_internal_icon_flip(statesprite, is_vertical, do_flip) {
    let x, y;
    if (is_vertical) {
        x = 0;
        y = do_flip;
    } else {
        x = do_flip;
        y = 0;
    }
    statesprite_flip_texture(statesprite, x, y);
}

function healthbar_internal_toggle_chrctr_state(healthbar, slctd_ptr, prefix, state_name, bar, icon) {
    if (state_name == Symbol) return 0;

    // JS only
    if (healthbar[slctd_ptr]) healthbar[slctd_ptr] = undefined;
    healthbar[slctd_ptr] = strdup(state_name);

    // C version
    //if (*slctd_ptr) string_free(*slctd_ptr);
    //*slctd_ptr = strdup(state_name);

    return healthbar_internal_toggle_chrctr_state2(prefix, state_name, bar, icon);
}

function healthbar_internal_toggle_chrctr_state2(prefix, state_name, bar, icon) {
    let success = 0;
    let sub_state_name = string_concat_for_state_name(2, prefix, state_name);

    success += statesprite_state_toggle(bar, sub_state_name);
    success += statesprite_state_toggle(icon, sub_state_name);

    sub_state_name = undefined;
    return success;
}

function healthbar_internal_set_chrctr_state(prefix, state_name, bar, icon) {
    //
    // confusing part, set the states in this order:
    //      neutral -> winning/loosing -> winner/looser
    //
    // must be done in this way in case there missing icons for non-neutral states
    // 

    // step 1: default to "neutral"
    healthbar_internal_toggle_chrctr_state2(HEALTHBAR_ICON_PREFIX_NEUTRAL, state_name, bar, icon);

    if (prefix == HEALTHBAR_ICON_PREFIX_NEUTRAL) return;

    // step 2: default to "winning" or "loosing"
    if (prefix == HEALTHBAR_ICON_PREFIX_WINNER)
        healthbar_internal_toggle_chrctr_state2(HEALTHBAR_ICON_PREFIX_WINNING, state_name, bar, icon);
    else if (prefix == HEALTHBAR_ICON_PREFIX_LOOSER)
        healthbar_internal_toggle_chrctr_state2(HEALTHBAR_ICON_PREFIX_LOOSING, state_name, bar, icon);

    // step 3: set the state "winner" or "looser"
    healthbar_internal_toggle_chrctr_state2(prefix, state_name, bar, icon);
}

function healthbar_internal_calc_chrctr(healthbar, invert, position, bar, icon) {
    let crop_width = -1;
    let crop_height = -1;
    let crop_x = 0;
    let crop_y = 0;

    if (invert) {
        // player bar, invert the position
        position = healthbar.health_bar_length - position;

        if (healthbar.is_vertical)
            crop_y = position;
        else
            crop_x = position;
    } else {
        if (healthbar.is_vertical)
            crop_height = position;
        else
            crop_width = position;
    }
    statesprite_crop(bar, crop_x, crop_y, crop_width, crop_height);


    let x = healthbar.modifier.x;
    let y = healthbar.modifier.y;

    //let half_dimmen = healthbar.dimmen / -2;
    let half_dimmen = 0;// mimics Funkin behavior

    if (healthbar.is_vertical) {
        x += half_dimmen;
        y += position;
    } else {
        x += position;
        y += half_dimmen;
    }
    statesprite_set_draw_location(icon, x, y);
}

function healthbar_internal_draw_chrctr(pvrctx, bump_modifier, icon) {
    pvr_context_save(pvrctx);

    const draw_size = [0, 0];
    const draw_location = [0, 0];
    const offsets = [0, 0];

    statesprite_get_draw_size(icon, draw_size);
    statesprite_get_draw_location(icon, draw_location);
    statesprite_state_get_offsets(icon, offsets);

    pvr_context_apply_modifier2(
        pvrctx, bump_modifier,
        draw_location[0] + offsets[0],
        draw_location[1] + offsets[1],
        draw_size[0],
        draw_size[1]
    );
    statesprite_draw(icon, pvrctx);

    pvr_context_restore(pvrctx);
}

function healthbar_internal_add_warning(sprite, modelholder, use_alt, height, state_name) {
    let anim_name;

    if (use_alt)
        anim_name = state_name.concat(HEALTHBAR_WARNING_ALT_SUFFIX);
    else
        anim_name = state_name;

    statesprite_state_remove(sprite, state_name);
    let state = statesprite_state_add(sprite, modelholder, anim_name, state_name);

    if (use_alt) anim_name = undefined;

    if (!state) return 0;

    const temp = [0, 0];
    imgutils_get_statesprite_original_size(state, temp);
    imgutils_calc_size(temp[0], temp[1], -1, height, temp);

    state.draw_width = temp[0];
    state.draw_height = temp[1];
    state.offset_x = state.draw_width / -2;
    state.offset_y = state.draw_height / -2;

    return 1;
}

function healthbar_internal_save_resolutions(linkedlist, state_name, mdlhldr_bar, mdlhldr_icn) {
    let item = {
        state_name: strdup(state_name), resolution_bar: [0, 0], resolution_icon: [0, 0]
    };

    modelholder_get_texture_resolution(mdlhldr_icn, item.resolution_bar);
    modelholder_get_texture_resolution(mdlhldr_bar, item.resolution_icon);

    linkedlist_add_item(linkedlist, item);
}

function healthbar_internal_load_resolutions(linkedlist, state_name, rsltn_bar, rsltn_icn) {
    for (let item of linkedlist_iterate4(linkedlist)) {
        if (item.state_name == state_name) {
            rsltn_bar[0] = item.resolution_bar[0];
            rsltn_bar[1] = item.resolution_bar[1];
            rsltn_icn[0] = item.resolution_icon[0];
            rsltn_icn[1] = item.resolution_icon[1];
            return;
        }
    }

    // this never should happen
    rsltn_bar[0] = FUNKIN_SCREEN_RESOLUTION_WIDTH;
    rsltn_bar[1] = FUNKIN_SCREEN_RESOLUTION_HEIGHT;
    rsltn_icn[0] = FUNKIN_SCREEN_RESOLUTION_WIDTH;
    rsltn_icn[1] = FUNKIN_SCREEN_RESOLUTION_HEIGHT;
}

