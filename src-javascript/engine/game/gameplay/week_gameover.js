"use strict";

const WEEK_GAMEOVER_JUDGEMENT =
    "sicks  $i\ngood   $i\nbads   $i\nshits  $i\n\n" +
    "misses      $i\npenalties   $i\ndifficult  $s\n\n\n" +
    "Lost at $s ($2d% completed)";

const WEEK_GAMEOVER_HELP_RETRY = "retry";
const WEEK_GAMEOVER_HELP_DIFFICULT = "change difficult";
const WEEK_GAMEOVER_HELP_GIVEUP = "giveup";
const WEEK_GAMEOVER_HELP_STATS = "show/hide statistics";

const WEEK_GAMEOVER_DURATION_DIE = "gameover_duration_die";
const WEEK_GAMEOVER_DURATION_RETRY = "gameover_duration_retry";
const WEEK_GAMEOVER_DURATION_GIVEUP = "gameover_duration_giveup";
const WEEK_GAMEOVER_DURATION_BEFORE = "gameover_transition_before";
const WEEK_GAMEOVER_DURATION_BEFORE_FORCE_END = "gameover_transition_before_force_end";

const WEEK_GAMEOVER_LAYOUT_Z_INDEX = 200;
const WEEK_GAMEOVER_LAYOUT_DREAMCAST = "/assets/common/image/week-round/gameover~dreamcast.xml";
const WEEK_GAMEOVER_LAYOUT_WIDESCREEN = "/assets/common/image/week-round/gameover.xml";
const WEEK_GAMEOVER_LAYOUT_VERSION = "/assets/common/image/week-round/gameover_version.txt";
const WEEK_GAMEOVER_MODDING_SCRIPT = "/assets/common/data/scripts/weekgameover.lua";

const WEEK_GAMEOVER_BUTTONS = GAMEPAD_BACK | GAMEPAD_B | GAMEPAD_START | GAMEPAD_T_LR;
const WEEK_GAMEOVER_BUTTONS2 = WEEK_GAMEOVER_BUTTONS | GAMEPAD_AD_LEFT | GAMEPAD_AD_RIGHT;
const WEEK_GAMEOVER_BUTTONS_LEFT = GAMEPAD_AD_LEFT | GAMEPAD_TRIGGER_LEFT;
const WEEK_GAMEOVER_BUTTONS_SELECTOR = GAMEPAD_AD_LEFT | GAMEPAD_AD_RIGHT | GAMEPAD_T_LR;

const WEEK_GAMEOVER_NOMUSIC = 0;
const WEEK_GAMEOVER_NOSFXDIE = 1;
const WEEK_GAMEOVER_NOSFXRETRY = 2;
const WEEK_GAMEOVER_ANIMDURATIONDIE = 3;
const WEEK_GAMEOVER_ANIMDURATIONRETRY = 4;
const WEEK_GAMEOVER_ANIMDURATIONGIVEUP = 5;
const WEEK_GAMEOVER_ANIMDURATIONBEFORE = 6;
const WEEK_GAMEOVER_ANIMDURATIONBEFOREFORCEEND = 7;
const WEEK_GAMEOVER_SETMUSIC = 8;
const WEEK_GAMEOVER_SETSFXDIE = 9;
const WEEK_GAMEOVER_SETSFXRETRY = 10;


async function week_gameover_init() {
    let src = pvr_context_is_widescreen() ? WEEK_GAMEOVER_LAYOUT_WIDESCREEN : WEEK_GAMEOVER_LAYOUT_DREAMCAST;
    let button_icons = await modelholder_init(WEEKSELECTOR_BUTTONS_MODEL);
    let ui_icons = await modelholder_init(WEEKSELECTOR_UI_ICONS_MODEL);
    let animlist = await animlist_init(WEEKSELECTOR_UI_ANIMS);
    let layout = await layout_init(src);

    if (!layout) throw new Error("missing or invalid layout for gameover screen");

    let help_retry = weekselector_helptext_init(
        button_icons, layout, 3, false, WEEKSELECTOR_BUTTON_START, WEEK_GAMEOVER_HELP_RETRY, null
    );
    let help_difficult = weekselector_helptext_init(
        button_icons, layout, 2, false, WEEKSELECTOR_BUTTON_LT_RT, WEEK_GAMEOVER_HELP_DIFFICULT, null
    );
    let help_giveup = weekselector_helptext_init(
        button_icons, layout, 1, false, WEEKSELECTOR_BUTTON_B, WEEK_GAMEOVER_HELP_GIVEUP, null
    );
    let help_stats = weekselector_helptext_init(
        button_icons, layout, 1, true, WEEKSELECTOR_BUTTON_X, WEEK_GAMEOVER_HELP_STATS, null
    );
    modelholder_destroy(button_icons);

    let selector = await weekselector_difficult_init(animlist, ui_icons, layout);
    weekselector_difficult_visible(selector, false);
    modelholder_destroy(ui_icons);

    let weekgameover = {
        layout,
        help_retry,
        help_difficult,
        help_giveup,
        help_stats,
        selector,
        drawable: null,
        disabled: true,
        weekinfo: null,
        difficult: null,
        choosen_difficult: weekselector_weektitle_init(layout),
        group_id_help: layout_external_create_group(layout, null, 0),

        default_die_duration: layout_get_attached_value_as_float(
            layout, WEEK_GAMEOVER_DURATION_DIE, -1.0
        ),
        default_retry_duration: layout_get_attached_value_as_float(
            layout, WEEK_GAMEOVER_DURATION_RETRY, -1.0
        ),
        default_giveup_duration: layout_get_attached_value_as_float(
            layout, WEEK_GAMEOVER_DURATION_GIVEUP, -1.0
        ),
        default_before_duration: layout_get_attached_value_as_float(
            layout, WEEK_GAMEOVER_DURATION_BEFORE, 500.0
        ),
        default_before_force_end_duration: layout_get_attached_value_as_float(
            layout, WEEK_GAMEOVER_DURATION_BEFORE_FORCE_END, 500.0
        )
    };

    weekgameover.drawable = drawable_init(
        WEEK_GAMEOVER_LAYOUT_Z_INDEX, weekgameover, week_gameover_draw, week_gameover_animate
    );

    layout_external_vertex_create_entries(layout, 4);
    layout_external_vertex_set_entry(
        layout, 0, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_retry), weekgameover.group_id_help
    );
    layout_external_vertex_set_entry(
        layout, 1, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_difficult), weekgameover.group_id_help
    );
    layout_external_vertex_set_entry(
        layout, 2, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_giveup), weekgameover.group_id_help
    );
    layout_external_vertex_set_entry(
        layout, 3, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_stats), weekgameover.group_id_help
    );

    // load default sounds
    await week_gameover_set_option(weekgameover, WEEK_GAMEOVER_SETMUSIC, NaN, null);
    await week_gameover_set_option(weekgameover, WEEK_GAMEOVER_SETSFXDIE, NaN, null);
    await week_gameover_set_option(weekgameover, WEEK_GAMEOVER_SETSFXRETRY, NaN, null);

    weekgameover.duration_die = weekgameover.default_die_duration;
    weekgameover.duration_retry = weekgameover.default_retry_duration;
    weekgameover.duration_giveup = weekgameover.default_giveup_duration;
    weekgameover.duration_before = weekgameover.default_before_duration;
    weekgameover.duration_before_force_end = weekgameover.default_before_force_end_duration;

    weekgameover.modding = modding_init(layout, WEEK_GAMEOVER_MODDING_SCRIPT);
    weekgameover.modding.callback_private_data = weekgameover;
    weekgameover.modding.callback_option = week_gameover_internal_handle_modding_option;

    return weekgameover;
}

function week_gameover_destroy(weekgameover) {
    layout_destroy(weekgameover.layout);
    weekselector_helptext_destroy(weekgameover.help_retry);
    weekselector_helptext_destroy(weekgameover.help_difficult);
    weekselector_helptext_destroy(weekgameover.help_giveup);
    weekselector_helptext_destroy(weekgameover.help_stats);
    weekselector_difficult_destroy(weekgameover.selector);
    drawable_destroy(weekgameover.drawable);
    weekselector_weektitle_destroy(weekgameover.choosen_difficult);
    if (weekgameover.sfx_die) soundplayer_destroy(weekgameover.sfx_die);
    if (weekgameover.music_bg) soundplayer_destroy(weekgameover.music_bg);
    if (weekgameover.sfx_retry) soundplayer_destroy(weekgameover.sfx_retry);

    weekgameover.modding.has_funkinsave_changes = false;// ignore fukinsave changes
    modding_destroy(weekgameover.modding);

    weekgameover = undefined;
}

async function week_gameover_read_version() {
    const garapheme = { code: 0, size: 0 };
    let version = await fs_readtext(WEEK_GAMEOVER_LAYOUT_VERSION);

    if (version) {
        string_get_character_codepoint(version, 0, garapheme);
    }

    version = undefined;
    return garapheme.code;
}


function week_gameover_hide(weekgameover) {
    weekgameover.disabled = true;
    weekgameover.weekinfo = null;
    weekgameover.difficult = null;
    weekselector_difficult_load(weekgameover.selector, null, null);
    drawable_set_visible(weekgameover.drawable, false);
}

function week_gameover_display(weekgameover, timestamp, duration, playerstats, weekinfo, difficult) {
    let percent = Math.min((timestamp / duration) * 100.0, 100.0);

    layout_set_group_visibility_by_id(weekgameover.layout, weekgameover.group_id_help, false);

    weekselector_difficult_visible(weekgameover.selector, false);
    layout_trigger_any(weekgameover.layout, null);// restart any animation and/or camera
    drawable_set_visible(weekgameover.drawable, true);

    let title = layout_get_textsprite(weekgameover.layout, "title");
    if (title) {
        const display_name = weekinfo.display_name ? weekinfo.display_name : weekinfo.name;
        textsprite_set_text_intern(title, true, display_name);
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
    weekgameover.disabled = true;
}

async function week_gameover_display_selector(weekgameover) {
    await weekselector_difficult_load(
        weekgameover.selector, weekgameover.weekinfo, weekgameover.difficult
    );
    weekselector_difficult_relayout(weekgameover.selector, false);// obligatory
    weekselector_difficult_visible(weekgameover.selector, true);
    layout_trigger_any(weekgameover.layout, "selector_show");
    await modding_helper_notify_event(weekgameover.modding, "selector_show");
}

async function week_gameover_display_choosen_difficult(weekgameover) {
    layout_trigger_any(weekgameover.layout, "changed_difficult");
    weekselector_weektitle_move_difficult(weekgameover.choosen_difficult, weekgameover.selector);
    await modding_helper_notify_event(weekgameover.modding, "changed_difficult");
}

function week_gameover_get_drawable(weekgameover) {
    return weekgameover.drawable;
}

function week_gameover_get_difficult(weekgameover) {
    return weekgameover.difficult;
}

async function week_gameover_helper_ask_to_player(weekgameover, roundcontext) {
    const pressed = [0x00];
    const layout = roundcontext.layout ?? roundcontext.ui_layout;

    // trigger any initial Action
    if (roundcontext.settings.layout_rollback)
        layout_trigger_any(weekgameover.layout, null);
    else
        layout_trigger_any(weekgameover.layout, "hide_judgement");

    // match antialiasing with the stage layout
    layout_disable_antialiasing(weekgameover.layout, layout_is_antialiasing_disabled(layout));
    layout_set_layout_antialiasing(weekgameover.layout, layout_get_layout_antialiasing(layout));

    let controller = gamepad_init(-1);
    gamepad_set_buttons_delay(controller, WEEKSELECTOR_BUTTON_DELAY);
    gamepad_clear_buttons(controller);

    // pick values from the current stage layout (if exists)
    let die_animation_duration = layout_get_attached_value_as_float(
        layout, WEEK_GAMEOVER_DURATION_DIE, weekgameover.duration_die
    );
    let retry_animation_duration = layout_get_attached_value_as_float(
        layout, WEEK_GAMEOVER_DURATION_RETRY, weekgameover.duration_retry
    );
    let giveup_animation_duration = layout_get_attached_value_as_float(
        layout, WEEK_GAMEOVER_DURATION_GIVEUP, weekgameover.duration_giveup
    );
    let before_duration = layout_get_attached_value_as_float(
        layout, WEEK_GAMEOVER_DURATION_BEFORE, weekgameover.duration_giveup
    );
    let before_force_end_duration = layout_get_attached_value_as_float(
        layout, WEEK_GAMEOVER_DURATION_BEFORE_FORCE_END, weekgameover.duration_giveup
    );
    let stage_has_gameover = layout_get_attached_value(
        layout, "gameover_with_stage", LAYOUT_TYPE_BOOLEAN, false
    );

    // ¿which player is dead?    
    let dead_player_index = roundcontext.players_size < 2 ? 0 : 1;
    let dead_character = null;
    let commited_anims = -1;
    let wait_animation = false;

    if (roundcontext.girlfriend) {
        character_play_extra(roundcontext.girlfriend, "cry", false);
    }

    weekgameover.modding_decision = 1; // giveup by default
    weekgameover.modding.has_exit = false;
    weekgameover.modding.has_halt = false;
    weekgameover.modding.native_menu = null;
    await modding_helper_notify_init(weekgameover.modding, MODDING_NATIVE_MENU_SCREEN);

    if (!layout_trigger_any(layout, "camera_gameover")) {
        for (let i = 0; i < roundcontext.players_size; i++) {
            if (!roundcontext.players[i].playerstats) continue;
            if (roundcontext.players[i].is_opponent) {
                character_play_extra(roundcontext.players[i].character, FUNKIN_OPPONENT_VICTORY, false);
                continue;
            }

            if (playerstats_is_dead(roundcontext.players[i].playerstats)) {
                dead_player_index = i;
                dead_character = roundcontext.players[i].character;

                if (character_play_extra(dead_character, FUNKIN_PLAYER_DIES, false)) {
                    wait_animation = die_animation_duration < 0.0;
                    commited_anims = character_get_commited_animations_count(dead_character);
                }

                let target = roundcontext.players[i].is_opponent ? WEEKROUND_CAMERA_OPONNENT : WEEKROUND_CAMERA_PLAYER;
                week_camera_focus_guess(roundcontext, target, i);
            } else {
                character_play_extra(roundcontext.players[i].character, FUNKIN_PLAYER_SCARED, false);
            }
        }
    }

    if (weekgameover.sfx_die) soundplayer_replay(weekgameover.sfx_die);

    // try draw only the dead player
    let character_name = week_internal_concat_suffix(WEEKROUND_CHARACTER_PREFIX, dead_player_index);
    layout_set_single_item_to_draw(layout, character_name);
    character_name = undefined;

    // trigger layout (normally shows the player only with a black background)
    if (stage_has_gameover) layout_trigger_any(layout, "gameover");

    let decision = 0;
    let selector_unloaded = true;
    let selector_buttons = GAMEPAD_T_LR;
    let ui_buttons = WEEK_GAMEOVER_BUTTONS;
    let total = 0.0;
    let gameoverloop_notified = false;

    while (true) {
        if (roundcontext.scriptcontext.halt_flag) {
            await week_halt(roundcontext, true);
            continue;
        }

        let elapsed = await pvrctx_wait_ready();

        if (roundcontext.script) await weekscript_notify_frame(roundcontext.script, elapsed);

        let res = await modding_helper_handle_custom_menu(weekgameover.modding, controller, elapsed);
        if (res != MODDING_HELPER_RESULT_CONTINUE || weekgameover.modding.has_exit) {
            decision = weekgameover.modding_decision;
            break;
        }
        if (weekgameover.modding.has_halt) {
            gamepad_clear_buttons(controller);
            continue;
        }

        if (roundcontext.scriptcontext.force_end_flag) {
            gamepad_destroy(controller);
            layout_set_single_item_to_draw(layout, null);

            modding_helper_notify_event(
                weekgameover.modding,
                roundcontext.scriptcontext.force_end_loose_or_win ? "force_loose" : "force_win"
            );
            modding_helper_notify_exit2(weekgameover.modding);

            return roundcontext.scriptcontext.force_end_loose_or_win ? 1 : 2;
        }

        // animate & draw first to minimize suttering
        pvr_context_reset(pvr_context);
        layout_animate(layout, elapsed);

        week_gameover_draw2(weekgameover, roundcontext, pvr_context);

        let buttons = gamepad_has_pressed_delayed(controller, ui_buttons);

        if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
            // giveup
            decision = 1;
            break;
        } else if (buttons & GAMEPAD_START) {
            // retry
            decision = 2;
            weekgameover.disabled = false;
            if (selector_unloaded) break;

            // the string returned is constant
            const difficult = weekselector_difficult_get_selected(weekgameover.selector);
            if (difficult == null) break;// is locked or not selected

            if (difficult != weekgameover.difficult) {
                weekgameover.difficult = difficult;
                await week_gameover_display_choosen_difficult(weekgameover);
            }

            decision = 2;
            break;
        } else if (buttons & selector_buttons) {
            // change difficult
            if (selector_unloaded) {
                // show the selector and wait until the custom difficulties are readed from disk
                gamepad_clear_buttons(controller);
                await week_gameover_display_selector(weekgameover);
                selector_unloaded = false;
                selector_buttons = WEEK_GAMEOVER_BUTTONS_SELECTOR;
                ui_buttons = WEEK_GAMEOVER_BUTTONS2;
            } else {
                // switch difficult
                let offset = buttons & WEEK_GAMEOVER_BUTTONS_LEFT ? -1 : 1;
                weekselector_difficult_scroll(weekgameover.selector, offset);
            }
        } else if (buttons & GAMEPAD_X) {
            let is_visible = layout_get_group_visibility(weekgameover.layout, "judgement");
            layout_trigger_any(weekgameover.layout, is_visible ? "hide_judgement" : "show_judgement");
        }

        if (wait_animation) {
            if (commited_anims == character_get_commited_animations_count(dead_character)) continue;

            wait_animation = false;
            total = Infinity;
        }

        total += elapsed;
        if (weekgameover.disabled && total > die_animation_duration) {
            weekgameover.disabled = false;
            layout_set_group_visibility_by_id(weekgameover.layout, weekgameover.group_id_help, true);
            if (weekgameover.music_bg) {
                soundplayer_replay(weekgameover.music_bg);
                soundplayer_loop_enable(weekgameover.music_bg, true);
            }
            if (weekgameover.sfx_die) soundplayer_stop(weekgameover.sfx_die);

            gameoverloop_notified = true;
            if (roundcontext.script) await weekscript_notify_gameoverloop(roundcontext.script);
            await modding_helper_notify_event(weekgameover.modding, "gameover_loop");
        }
    }

    if (!gameoverloop_notified) {
        if (roundcontext.script) await weekscript_notify_gameoverloop(roundcontext.script);
        await modding_helper_notify_event(weekgameover.modding, "gameover_loop");
    }

    switch (decision) {
        case 1:
            await modding_helper_notify_event(weekgameover.modding, "giveup");
            break;
        case 2:
            await modding_helper_notify_event(weekgameover.modding, "retry");
            break;
    }

    if (dead_character) {
        const anim = decision == 2 ? FUNKIN_PLAYER_RETRY : FUNKIN_PLAYER_GIVEUP;
        if (!character_play_extra(dead_character, anim, false)) {
            // avoid waiting for retry/giveup animations
            dead_character = null;
        }
    }

    //if (weekgameover.sfx_die) soundplayer_stop(weekgameover.sfx_die);
    layout_set_group_visibility_by_id(weekgameover.layout, weekgameover.group_id_help, false);

    if (decision == 2) {
        layout_trigger_any(weekgameover.layout, "hide_stats");
        if (weekgameover.music_bg) soundplayer_stop(weekgameover.music_bg);
        if (weekgameover.sfx_retry) soundplayer_replay(weekgameover.sfx_retry);
    }

    if (roundcontext.script) {
        const new_difficult = weekgameover.difficult == roundcontext.song_difficult ? null : weekgameover.difficult;
        await weekscript_notify_gameoverdecision(roundcontext.script, decision == 2, new_difficult);
        await week_halt(roundcontext, true);
    }

    if (dead_character) commited_anims = character_get_commited_animations_count(dead_character);

    let before = decision == 1 ? before_force_end_duration : before_duration;
    let trigger_transition = true;

    total = decision == 2 ? retry_animation_duration : giveup_animation_duration;
    wait_animation = total < 0.0 && dead_character;

    if (before < 0.0) before = 0.0;

    if (wait_animation)
        total = Infinity;
    else if (total < 0.0)
        total = before;

    while (total >= 0.0) {
        let elapsed = await pvrctx_wait_ready();

        for (let i = 0; i < roundcontext.players_size; i++) {
            let controller_player = roundcontext.players[i].controller;
            if (controller_player && gamepad_get_managed_presses(controller_player, true, pressed)) {
                if (roundcontext.script) await weekscript_notify_buttons(roundcontext.script, i, pressed[0]);
            }
        }

        if (roundcontext.script) await weekscript_notify_frame(roundcontext.script, elapsed);

        // animate & draw first to minimize suttering
        pvr_context_reset(pvr_context);
        layout_animate(layout, elapsed);

        week_gameover_draw2(weekgameover, roundcontext, pvr_context);

        if (gamepad_has_pressed_delayed(controller, GAMEPAD_START | GAMEPAD_A | GAMEPAD_X)) {
            layout_trigger_any(weekgameover.layout, "transition_force_end");
            break;
        }

        // wait for character animation ends (if required)
        if (wait_animation && commited_anims != character_get_commited_animations_count(dead_character)) {
            wait_animation = false;

            if (decision == 2 && weekgameover.sfx_retry && !soundplayer_has_ended(weekgameover.sfx_retry)) {
                // wait for retry sound effect (gameOverEnd.ogg is 7 seconds long)
                total = soundplayer_get_duration(weekgameover.sfx_retry) - soundplayer_get_position(weekgameover.sfx_retry);
                if (total <= 0.0) total = before;// ignore
            } else {
                total = before;// ignore
            }
        }

        if (trigger_transition) {
            total -= elapsed;
            if (total <= before) {
                total = Infinity;
                trigger_transition = false;
                layout_trigger_any(weekgameover.layout, decision == 2 ? "transition" : "transition_giveup");
                if (decision == 1 && weekgameover.music_bg) soundplayer_fade(weekgameover.music_bg, false, total);
            }
        } else if (layout_animation_is_completed(weekgameover.layout, "transition_effect")) {
            break;
        }
    }

    if (weekgameover.sfx_die) soundplayer_stop(weekgameover.sfx_die);
    if (weekgameover.sfx_retry) soundplayer_stop(weekgameover.sfx_retry);

    gamepad_destroy(controller);
    layout_set_single_item_to_draw(layout, null);
    await modding_helper_notify_exit2(weekgameover.modding);

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

function week_gameover_draw2(weekgameover, roundcontext, pvrctx) {
    sprite_draw(roundcontext.screen_background, pvrctx);
    layout_draw(roundcontext.layout ?? roundcontext.ui_layout, pvrctx);
    if (weekgameover.disabled) return;
    layout_draw(weekgameover.layout, pvrctx);
}

async function week_gameover_set_option(weekgameover, option, nro, str) {
    switch (option) {
        case WEEK_GAMEOVER_NOMUSIC:
            if (weekgameover.music_bg) {
                soundplayer_destroy(weekgameover.music_bg);
                weekgameover.music_bg = null;
            }
            return;
        case WEEK_GAMEOVER_NOSFXDIE:
            if (weekgameover.sfx_die) {
                soundplayer_destroy(weekgameover.sfx_die);
                weekgameover.sfx_die = null;
            }
            return;
        case WEEK_GAMEOVER_NOSFXRETRY:
            if (weekgameover.sfx_retry) {
                soundplayer_destroy(weekgameover.sfx_retry);
                weekgameover.sfx_retry = null;
            }
            return;
        case WEEK_GAMEOVER_ANIMDURATIONDIE:
            weekgameover.duration_die = Number.isNaN(nro) ? weekgameover.default_die_duration : nro;
            return;
        case WEEK_GAMEOVER_ANIMDURATIONRETRY:
            weekgameover.duration_retry = Number.isNaN(nro) ? weekgameover.default_retry_duration : nro;
            return;
        case WEEK_GAMEOVER_ANIMDURATIONGIVEUP:
            weekgameover.duration_giveup = Number.isNaN(nro) ? weekgameover.default_giveup_duration : nro;
            return;
        case WEEK_GAMEOVER_ANIMDURATIONBEFORE:
            weekgameover.duration_before = Number.isNaN(nro) ? weekgameover.default_before_duration : nro;
            return;
        case WEEK_GAMEOVER_ANIMDURATIONBEFOREFORCEEND:
            weekgameover.duration_before_force_end = Number.isNaN(nro) ? weekgameover.default_before_force_end_duration : nro;
            return;
        case WEEK_GAMEOVER_SETMUSIC:
            if (weekgameover.music_bg) soundplayer_destroy(weekgameover.music_bg);
            weekgameover.music_bg = await soundplayer_init(str ?? "/assets/common/music/gameOver.ogg");
            return;
        case WEEK_GAMEOVER_SETSFXDIE:
            if (weekgameover.sfx_die) soundplayer_destroy(weekgameover.sfx_die);
            weekgameover.sfx_die = await soundplayer_init(str ?? "/assets/common/sound/loss_sfx.ogg");
            return;
        case WEEK_GAMEOVER_SETSFXRETRY:
            if (weekgameover.sfx_retry) soundplayer_destroy(weekgameover.sfx_retry);
            weekgameover.sfx_retry = await soundplayer_init(str ?? "/assets/common/sound/gameOverEnd.ogg");
            return;
    }
}

function week_gameover_get_layout(weekgameover) {
    return weekgameover.layout;
}


function week_gameover_internal_handle_modding_option(weekgameover, option) {

    if (option == "giveup") {
        weekgameover.modding_decision = 1;
    } else if (option == "retry") {
        weekgameover.modding_decision = 2;
    } else {
        // unknown option
        return false;
    }

    return true;
}

