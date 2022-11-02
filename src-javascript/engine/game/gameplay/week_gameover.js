"use strict";

const WEEK_GAMEOVER_JUDGEMENT =
    "sicks  $i\ngood   $i\nbads   $i\nshits  $i\n\n" +
    "misses      $i\npenalties   $i\ndifficult  $s\n\n\n" +
    "Lost at $s ($2d% completed)";

const WEEK_GAMEOVER_HELP_RETRY = "retry";
const WEEK_GAMEOVER_HELP_DIFFICULT = "change difficult";
const WEEK_GAMEOVER_HELP_GIVEUP = "giveup";

const WEEK_GAMEOVER_DURATION_DIE = "gameover_duration_die";
const WEEK_GAMEOVER_DURATION_RETRY = "gameover_duration_retry";
const WEEK_GAMEOVER_DURATION_GIVEUP = "gameover_duration_giveup";

const WEEK_GAMEOVER_LAYOUT_Z_INDEX = 200;
const WEEK_GAMEOVER_LAYOUT_DREAMCAST = "/assets/common/image/week-round/gameover~dreamcast.xml";
const WEEK_GAMEOVER_LAYOUT_WIDESCREEN = "/assets/common/image/week-round/gameover.xml";
const WEEK_GAMEOVER_LAYOUT_VERSION = "/assets/common/image/week-round/gameover_version.txt";

const WEEK_GAMEOVER_BUTTONS = GAMEPAD_B | GAMEPAD_START | GAMEPAD_T_LR;
const WEEK_GAMEOVER_BUTTONS2 = WEEK_GAMEOVER_BUTTONS | GAMEPAD_AD_LEFT | GAMEPAD_AD_RIGHT;
const WEEK_GAMEOVER_BUTTONS_LEFT = GAMEPAD_AD_LEFT | GAMEPAD_TRIGGER_LEFT;
const WEEK_GAMEOVER_BUTTONS_SELECTOR = GAMEPAD_AD_LEFT | GAMEPAD_AD_RIGHT | GAMEPAD_T_LR;


async function week_gameover_init() {
    let src = pvrctx_output_is_widescreen() ? WEEK_GAMEOVER_LAYOUT_WIDESCREEN : WEEK_GAMEOVER_LAYOUT_DREAMCAST;
    let button_icons = await modelholder_init(WEEKSELECTOR_BUTTONS_MODEL);
    let ui_icons = await modelholder_init(WEEKSELECTOR_UI_ICONS_MODEL);
    let animlist = await animlist_init(WEEKSELECTOR_UI_ANIMS);
    let layout = await layout_init(src);

    if (!layout) throw new Error("missing or invalid layout for gameover screen");

    let help_retry = weekselector_helptext_init(
        button_icons, layout, 3, 0, WEEKSELECTOR_BUTTON_START, WEEK_GAMEOVER_HELP_RETRY, null
    );
    let help_difficult = weekselector_helptext_init(
        button_icons, layout, 2, 0, WEEKSELECTOR_BUTTON_LT_RT, WEEK_GAMEOVER_HELP_DIFFICULT, null
    );
    let help_giveup = weekselector_helptext_init(
        button_icons, layout, 1, 0, WEEKSELECTOR_BUTTON_B, WEEK_GAMEOVER_HELP_GIVEUP, null
    );
    modelholder_destroy(button_icons);

    let selector = await weekselector_difficult_init(animlist, ui_icons, layout);
    weekselector_difficult_visible(selector, 0);
    modelholder_destroy(ui_icons);

    let weekgameover = {
        layout, help_retry, help_difficult, help_giveup, selector,
        drawable: null,
        disabled: 1,
        weekinfo: null,
        difficult: null,
        choosen_difficult: weekselector_weektitle_init(layout),
        group_id_help: layout_external_create_group(layout, null, 0),

        default_die_duration: layout_get_attached_value_as_float(
            layout, WEEK_GAMEOVER_DURATION_DIE, 3000
        ),
        default_retry_duration: layout_get_attached_value_as_float(
            layout, WEEK_GAMEOVER_DURATION_RETRY, 2000
        ),
        default_giveup_duration: layout_get_attached_value_as_float(
            layout, WEEK_GAMEOVER_DURATION_GIVEUP, 500
        )
    };

    weekgameover.drawable = drawable_init(
        WEEK_GAMEOVER_LAYOUT_Z_INDEX, weekgameover, week_gameover_draw, week_gameover_animate
    );

    layout_external_vertex_create_entries(layout, 3);
    layout_external_vertex_set_entry(
        layout, 0, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_retry), weekgameover.group_id_help
    );
    layout_external_vertex_set_entry(
        layout, 1, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_difficult), weekgameover.group_id_help
    );
    layout_external_vertex_set_entry(
        layout, 2, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_giveup), weekgameover.group_id_help
    );

    return weekgameover;
}

function week_gameover_destroy(weekgameover) {
    layout_destroy(weekgameover.layout);
    weekselector_helptext_destroy(weekgameover.help_retry);
    weekselector_helptext_destroy(weekgameover.help_difficult);
    weekselector_helptext_destroy(weekgameover.help_giveup);
    weekselector_difficult_destroy(weekgameover.selector);
    drawable_destroy(weekgameover.drawable);
    weekselector_weektitle_destroy(weekgameover.choosen_difficult);
    weekgameover = undefined;
}

async function week_gameover_read_version() {
    const garapheme = { code: 0, size: 0 };
    let version = await fs_readtext(WEEK_GAMEOVER_LAYOUT_VERSION);
    string_get_character_codepoint(version, 0, garapheme);
    version = undefined;
    return garapheme.code;
}


function week_gameover_hide(weekgameover) {
    weekgameover.disabled = 1;
    weekgameover.weekinfo = null;
    weekgameover.difficult = null;
    weekselector_difficult_load(weekgameover.selector, null, null);
    drawable_set_visible(weekgameover.drawable, 0);
}

function week_gameover_display(weekgameover, timestamp, duration, playerstats, weekinfo, difficult) {
    let percent = Math.min((timestamp / duration) * 100, 100.00);

    layout_set_group_visibility_by_id(weekgameover.layout, weekgameover.group_id_help, 0);

    weekselector_difficult_visible(weekgameover.selector, 0);
    layout_trigger_any(weekgameover.layout, null);// restart any animation and/or camera
    drawable_set_visible(weekgameover.drawable, 1);

    let title = layout_get_textsprite(weekgameover.layout, "title");
    if (title) {
        const display_name = weekinfo.display_name ? weekinfo.display_name : weekinfo.name;
        textsprite_set_text_intern(title, 1, display_name);
    }

    let jugement = layout_get_textsprite(weekgameover.layout, "judgement");
    if (jugement) {
        let time = math2d_timestamp_to_string(timestamp);
        textsprite_set_text_formated(
            jugement, WEEK_GAMEOVER_JUDGEMENT,

            playerstats_get_sicks(playerstats),
            playerstats_get_goods(playerstats),
            playerstats_get_bads(playerstats),
            playerstats_get_shits(playerstats),

            playerstats_get_misses(playerstats),
            playerstats_get_penalties(playerstats),
            difficult,

            time,
            percent
        );
        time = undefined;
    }

    weekgameover.weekinfo = weekinfo;
    weekgameover.difficult = difficult;
    weekgameover.disabled = 1;
}

async function week_gameover_display_selector(weekgameover) {
    await weekselector_difficult_load(
        weekgameover.selector, weekgameover.weekinfo, weekgameover.difficult
    );
    weekselector_difficult_relayout(weekgameover.selector, 0);// obligatory
    weekselector_difficult_visible(weekgameover.selector, 1);
    layout_trigger_any(weekgameover.layout, "selector_show");
}

function week_gameover_display_choosen_difficult(weekgameover) {
    layout_trigger_any(weekgameover.layout, "changed_difficult");
    weekselector_weektitle_move_difficult(weekgameover.choosen_difficult, weekgameover.selector);
}

function week_gameover_get_drawable(weekgameover) {
    return weekgameover.drawable;
}

function week_gameover_get_difficult(weekgameover) {
    return weekgameover.difficult;
}

async function week_gameover_helper_ask_to_player(weekgameover, roundcontext) {
    const pressed = [0x00];
    const layout = roundcontext.layout;

    // match antialiasing with the stage layout
    layout_disable_antialiasing(weekgameover.layout, layout_is_antialiasing_disabled(layout));
    layout_set_layout_antialiasing(weekgameover.layout, layout_get_layout_antialiasing(layout));

    let controller = gamepad_init(-1);
    gamepad_set_buttons_delay(controller, WEEKSELECTOR_BUTTON_DELAY);
    gamepad_clear_buttons(controller);

    // ¿which player is dead?    
    let dead_player_index = roundcontext.players_size < 2 ? 0 : 1;

    if (roundcontext.girlfriend) {
        character_play_extra(roundcontext.girlfriend, "cry", 0);
    }

    for (let i = 0; i < roundcontext.players_size; i++) {
        if (!roundcontext.players[i].playerstats) continue;
        if (roundcontext.players[i].is_opponent) {
            character_play_extra(roundcontext.players[i].character, FUNKIN_OPPONENT_VICTORY, 0);
            continue;
        }

        if (playerstats_is_dead(roundcontext.players[i].playerstats)) {
            dead_player_index = i;
            character_play_extra(roundcontext.players[i].character, FUNKIN_PLAYER_DIES, 0);
        } else {
            character_play_extra(roundcontext.players[i].character, FUNKIN_PLAYER_SCARED, 0);
        }
    }

    // durations
    let die_animation_duration = layout_get_attached_value_as_float(
        layout, WEEK_GAMEOVER_DURATION_DIE, weekgameover.default_die_duration
    );
    let retry_animation_duration = layout_get_attached_value_as_float(
        layout, WEEK_GAMEOVER_DURATION_RETRY, weekgameover.default_retry_duration
    );
    let giveup_animation_duration = layout_get_attached_value_as_float(
        layout, WEEK_GAMEOVER_DURATION_GIVEUP, weekgameover.default_giveup_duration
    );

    let sfx_die = await soundplayer_init("/assets/common/sound/loss_sfx.ogg");
    let music_bg = await soundplayer_init("/assets/common/music/gameOver.ogg");
    let sfx_end = await soundplayer_init("/assets/common/sound/gameOverEnd.ogg");

    if (sfx_die != null) soundplayer_replay(sfx_die);

    // focus dead player
    if (!layout_trigger_any(roundcontext.layout, "camera_gameover")) {
        let camera_name = week_internal_concat_suffix(WEEKROUND_CAMERA_PLAYER, dead_player_index);
        if (!layout_trigger_any(roundcontext.layout, camera_name)) {
            layout_trigger_any(roundcontext.layout, WEEKROUND_CAMERA_PLAYER);
        }
        camera_name = undefined;
    }

    // try draw only the dead player
    let character_name = week_internal_concat_suffix(WEEKROUND_CHARACTER_PREFIX, dead_player_index);
    let has_focus = layout_set_single_item_to_draw(roundcontext.layout, character_name);
    character_name = undefined;

    // trigger layout (normally shows the player only with a black background)
    let stage_has_gameover = layout_get_attached_value(
        roundcontext.layout, "gameover_with_stage", LAYOUT_TYPE_BOOLEAN, 0
    );
    if (stage_has_gameover) layout_trigger_any(layout, "gameover");

    let decision = 0;
    let selector_unloaded = 1;
    let selector_buttons = GAMEPAD_T_LR;
    let ui_buttons = WEEK_GAMEOVER_BUTTONS;
    let total = 0;
    let dead_player_index2 = has_focus ? -1 : dead_player_index;

    while (1) {
        let elapsed = await pvrctx_wait_ready();

        if (roundcontext.script) await weekscript_notify_frame(roundcontext.script, elapsed);

        // animate & draw first to minimize suttering
        pvr_context_reset(pvr_context);
        layout_animate(layout, elapsed);

        if (stage_has_gameover) layout_draw(layout, pvr_context);
        else week_gameover_draw2(weekgameover, dead_player_index2, roundcontext, pvr_context);

        let buttons = gamepad_has_pressed_delayed(controller, ui_buttons);

        if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
            // giveup
            decision = 1;
            break;
        } else if (buttons & GAMEPAD_START) {
            // retry
            decision = 2;
            weekgameover.disabled = 0;
            if (selector_unloaded) break;

            // the string returned is constant
            const difficult = weekselector_difficult_get_selected(weekgameover.selector);
            if (difficult == null) break;// is locked or not selected

            if (difficult != weekgameover.difficult) {
                weekgameover.difficult = difficult;
                week_gameover_display_choosen_difficult(weekgameover);
            }

            decision = 2;
            break;
        } else if (buttons & selector_buttons) {
            // change difficult
            if (selector_unloaded) {
                // show the selector and wait until the custom difficulties are readed from disk
                gamepad_clear_buttons(controller);
                await week_gameover_display_selector(weekgameover);
                selector_unloaded = 0;
                selector_buttons = WEEK_GAMEOVER_BUTTONS_SELECTOR;
                ui_buttons = WEEK_GAMEOVER_BUTTONS2;
            } else {
                // switch difficult
                let offset = buttons & WEEK_GAMEOVER_BUTTONS_LEFT ? -1 : 1;
                weekselector_difficult_scroll(weekgameover.selector, offset);
            }
        }

        total += elapsed;
        if (weekgameover.disabled && total > die_animation_duration) {
            weekgameover.disabled = 0;
            layout_set_group_visibility_by_id(weekgameover.layout, weekgameover.group_id_help, 1);
            if (music_bg) {
                soundplayer_replay(music_bg);
                soundplayer_loop_enable(music_bg, 1);
            }
            if (sfx_die != null) soundplayer_stop(sfx_die);
        }
    }

    if (roundcontext.players_size > 0) {
        const anim = decision == 2 ? FUNKIN_PLAYER_RETRY : FUNKIN_PLAYER_GIVEUP;
        character_play_extra(roundcontext.players[dead_player_index].character, anim, 0);
    }

    if (sfx_die) soundplayer_stop(sfx_die);
    layout_set_group_visibility_by_id(weekgameover.layout, weekgameover.group_id_help, 0);

    if (decision == 2) {
        layout_trigger_any(weekgameover.layout, "hide_stats");
        if (music_bg) soundplayer_stop(music_bg);
        if (sfx_end) soundplayer_replay(sfx_end);
    }

    total = decision == 2 ? retry_animation_duration : giveup_animation_duration;
    let trigger_transition = 1;
    while (total > 0) {
        let elapsed = await pvrctx_wait_ready();

        for (let i = 0; i < roundcontext.players_size; i++) {
            let controller_player = roundcontext.players[i].controller;
            if (controller_player && gamepad_get_managed_presses(controller_player, 1, pressed)) {
                if (roundcontext.script) await weekscript_notify_buttons(roundcontext.script, i, pressed[0]);
            }
        }

        if (roundcontext.script) await weekscript_notify_frame(roundcontext.script, elapsed);

        // animate & draw first to minimize suttering
        pvr_context_reset(pvr_context);
        layout_animate(layout, elapsed);

        if (stage_has_gameover) layout_draw(layout, pvr_context);
        else week_gameover_draw2(weekgameover, dead_player_index2, roundcontext, pvr_context);

        if (gamepad_has_pressed_delayed(controller, GAMEPAD_START | GAMEPAD_A | GAMEPAD_X)) {
            layout_trigger_any(weekgameover.layout, "transition_force_end");
            break;
        }

        if (trigger_transition && total <= 500) {
            trigger_transition = 0;
            layout_trigger_any(weekgameover.layout, "transition");
            if (decision == 1 && music_bg) soundplayer_fade(music_bg, 0, total);
        } else if (trigger_transition) {
            total -= elapsed;
        } else if (layout_animation_is_completed(weekgameover.layout, "transition_effect")) {
            break;
        }
    }

    if (sfx_die) soundplayer_destroy(sfx_die);
    if (sfx_end) soundplayer_destroy(sfx_end);
    if (music_bg) soundplayer_destroy(music_bg);
    gamepad_destroy(controller);
    layout_set_single_item_to_draw(roundcontext.layout, null);

    return decision;
}


function week_gameover_animate(weekgameover, elapsed) {
    if (weekgameover.disabled) return 1;
    return layout_animate(weekgameover.layout, elapsed);
}

function week_gameover_draw(weekgameover, pvrctx) {
    if (weekgameover.disabled) return;
    layout_draw(weekgameover.layout, pvrctx);
}

function week_gameover_draw2(weekgameover, dead_player_index, roundcontext, pvrctx) {
    sprite_draw(roundcontext.screen_background, pvrctx);

    if (dead_player_index < 0) {
        layout_draw(roundcontext.layout, pvrctx);
    } else {
        pvr_context_save(pvrctx);
        let matrix = pvrctx.current_matrix;
        sh4matrix_apply_modifier(matrix, layout_get_modifier_viewport(roundcontext.layout));
        sh4matrix_apply_modifier(matrix, layout_get_modifier_camera(roundcontext.layout));
        pvr_context_set_global_antialiasing(pvrctx, layout_get_layout_antialiasing(roundcontext.layout));
        character_draw(roundcontext.players[dead_player_index].character, pvrctx);
        pvr_context_restore(pvrctx);
    }

    if (weekgameover.disabled) return;
    layout_draw(weekgameover.layout, pvrctx);

}


