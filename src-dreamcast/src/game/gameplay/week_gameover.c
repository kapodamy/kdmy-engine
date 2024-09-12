#include "game/gameplay/week_gameover.h"

#include "drawable.h"
#include "fs.h"
#include "game/common/funkin.h"
#include "game/funkin/character.h"
#include "game/gameplay/week.h"
#include "game/gameplay/weekscript.h"
#include "game/helpers/weekselector_difficult.h"
#include "game/helpers/weekselector_helptext.h"
#include "game/helpers/weekselector_weektitle.h"
#include "game/mainmenu.h"
#include "game/modding.h"
#include "game/weekselector.h"
#include "gamepad.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "modelholder.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "soundplayer.h"
#include "stringutils.h"
#include "textsprite.h"


struct WeekGameOver_s {
    Layout layout;
    WeekSelectorHelpText help_retry;
    WeekSelectorHelpText help_difficult;
    WeekSelectorHelpText help_giveup;
    WeekSelectorHelpText help_stats;
    WeekSelectorDifficult selector;
    Drawable drawable;
    bool disabled;
    WeekInfo* weekinfo;
    const char* difficult;
    WeekSelectorWeekTitle choosen_difficult;
    int32_t group_id_help;
    float default_die_duration;
    float default_retry_duration;
    float default_giveup_duration;
    float default_before_duration;
    float default_before_force_end_duration;
    SoundPlayer sfx_die;
    SoundPlayer music_bg;
    SoundPlayer sfx_retry;
    float duration_die;
    float duration_retry;
    float duration_giveup;
    float duration_before;
    float duration_before_force_end;
    Modding modding;
    int32_t modding_decision;
};


static const char* WEEK_GAMEOVER_JUDGEMENT =
    "sicks  $i\ngood   $i\nbads   $i\nshits  $i\n\n"
    "misses      $i\npenalties   $i\ndifficult  $s\n\n\n"
    "Lost at $s ($2d% completed)";

static const char* WEEK_GAMEOVER_HELP_RETRY = "retry";
static const char* WEEK_GAMEOVER_HELP_DIFFICULT = "change difficult";
static const char* WEEK_GAMEOVER_HELP_GIVEUP = "giveup";
static const char* WEEK_GAMEOVER_HELP_STATS = "show/hide statistics";

static const char* WEEK_GAMEOVER_DURATION_DIE = "gameover_duration_die";
static const char* WEEK_GAMEOVER_DURATION_RETRY = "gameover_duration_retry";
static const char* WEEK_GAMEOVER_DURATION_GIVEUP = "gameover_duration_giveup";
static const char* WEEK_GAMEOVER_DURATION_BEFORE = "gameover_transition_before";
static const char* WEEK_GAMEOVER_DURATION_BEFORE_FORCE_END = "gameover_transition_before_force_end";

static const float WEEK_GAMEOVER_LAYOUT_Z_INDEX = 200.0f;
static const char* WEEK_GAMEOVER_LAYOUT_DREAMCAST = "/assets/common/image/week-round/gameover~dreamcast.xml";
static const char* WEEK_GAMEOVER_LAYOUT_WIDESCREEN = "/assets/common/image/week-round/gameover.xml";
static const char* WEEK_GAMEOVER_LAYOUT_VERSION = "/assets/common/image/week-round/gameover_version.txt";
static const char* WEEK_GAMEOVER_MODDING_SCRIPT = "/assets/common/data/scripts/weekgameover.lua";

static const GamepadButtons WEEK_GAMEOVER_BUTTONS = MAINMENU_GAMEPAD_BUTTONS | GAMEPAD_T_LR;
static const GamepadButtons WEEK_GAMEOVER_BUTTONS_SELECTOR_OPEN = GAMEPAD_T_LR;
static const GamepadButtons WEEK_GAMEOVER_BUTTONS_SELECTOR_CLOSE = (MAINMENU_GAMEPAD_OK | MAINMENU_GAMEPAD_CANCEL) & ~GAMEPAD_START;
static const GamepadButtons WEEK_GAMEOVER_BUTTONS_SELECTOR_CHOOSE_LEFT = GAMEPAD_AD_LEFT | GAMEPAD_TRIGGER_LEFT;
static const GamepadButtons WEEK_GAMEOVER_BUTTONS_SELECTOR_CHOOSE_RIGHT = GAMEPAD_AD_RIGHT | GAMEPAD_TRIGGER_RIGHT;


static int32_t week_gameover_animate(WeekGameOver weekgameover, float elapsed);
static void week_gameover_draw(WeekGameOver weekgameover, PVRContext pvrctx);
static void week_gameover_draw2(WeekGameOver weekgameover, RoundContext roundcontext, PVRContext pvrctx);
static bool week_gameover_internal_handle_modding_option(void* obj, const char* option);


WeekGameOver week_gameover_init() {
    const char* src = pvr_context_is_widescreen() ? WEEK_GAMEOVER_LAYOUT_WIDESCREEN : WEEK_GAMEOVER_LAYOUT_DREAMCAST;
    ModelHolder button_icons = modelholder_init(WEEKSELECTOR_BUTTONS_MODEL);
    ModelHolder ui_icons = modelholder_init(WEEKSELECTOR_UI_ICONS_MODEL);
    AnimList animlist = animlist_init(WEEKSELECTOR_UI_ANIMS);
    Layout layout = layout_init(src);

    if (!layout) {
        logger_error("missing or invalid layout for gameover screen");
        assert(layout);
    }

    WeekSelectorHelpText help_retry = weekselector_helptext_init(
        button_icons, layout, 3, false, WEEKSELECTOR_BUTTON_START, WEEK_GAMEOVER_HELP_RETRY, NULL
    );
    WeekSelectorHelpText help_difficult = weekselector_helptext_init(
        button_icons, layout, 2, false, WEEKSELECTOR_BUTTON_LT_RT, WEEK_GAMEOVER_HELP_DIFFICULT, NULL
    );
    WeekSelectorHelpText help_giveup = weekselector_helptext_init(
        button_icons, layout, 1, false, WEEKSELECTOR_BUTTON_B, WEEK_GAMEOVER_HELP_GIVEUP, NULL
    );
    WeekSelectorHelpText help_stats = weekselector_helptext_init(
        button_icons, layout, 1, true, WEEKSELECTOR_BUTTON_X, WEEK_GAMEOVER_HELP_STATS, NULL
    );
    modelholder_destroy(&button_icons);

    WeekSelectorDifficult selector = weekselector_difficult_init(animlist, ui_icons, layout);
    weekselector_difficult_visible(selector, false);
    modelholder_destroy(&ui_icons);
    if (animlist) animlist_destroy(&animlist);

    WeekGameOver weekgameover = malloc_chk(sizeof(struct WeekGameOver_s));
    malloc_assert(weekgameover, WeekGameOver);

    *weekgameover = (struct WeekGameOver_s){
        .layout = layout,
        .help_retry = help_retry,
        .help_difficult = help_difficult,
        .help_giveup = help_giveup,
        .help_stats = help_stats,
        .selector = selector,
        .drawable = NULL,
        .disabled = true,
        .weekinfo = NULL,
        .difficult = NULL,
        .choosen_difficult = weekselector_weektitle_init(layout),
        .group_id_help = layout_external_create_group(layout, NULL, 0),

        .default_die_duration = layout_get_attached_value_as_float(layout, WEEK_GAMEOVER_DURATION_DIE, -1.0f),
        .default_retry_duration = layout_get_attached_value_as_float(layout, WEEK_GAMEOVER_DURATION_RETRY, -1.0f),
        .default_giveup_duration = layout_get_attached_value_as_float(layout, WEEK_GAMEOVER_DURATION_GIVEUP, -1.0f),
        .default_before_duration = layout_get_attached_value_as_float(layout, WEEK_GAMEOVER_DURATION_BEFORE, 500.0f),
        .default_before_force_end_duration = layout_get_attached_value_as_float(layout, WEEK_GAMEOVER_DURATION_BEFORE_FORCE_END, 500.0f)
    };

    weekgameover->drawable = drawable_init(
        WEEK_GAMEOVER_LAYOUT_Z_INDEX, weekgameover, (DelegateDraw)week_gameover_draw, (DelegateAnimate)week_gameover_animate
    );

    layout_external_vertex_create_entries(layout, 4);
    layout_external_vertex_set_entry(
        layout, 0, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_retry), weekgameover->group_id_help
    );
    layout_external_vertex_set_entry(
        layout, 1, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_difficult), weekgameover->group_id_help
    );
    layout_external_vertex_set_entry(
        layout, 2, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_giveup), weekgameover->group_id_help
    );
    layout_external_vertex_set_entry(
        layout, 3, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_stats), weekgameover->group_id_help
    );

    // load default sounds
    week_gameover_set_option(weekgameover, WEEK_GAMEOVER_SETMUSIC, FLOAT_NaN, NULL);
    week_gameover_set_option(weekgameover, WEEK_GAMEOVER_SETSFXDIE, FLOAT_NaN, NULL);
    week_gameover_set_option(weekgameover, WEEK_GAMEOVER_SETSFXRETRY, FLOAT_NaN, NULL);

    weekgameover->duration_die = weekgameover->default_die_duration;
    weekgameover->duration_retry = weekgameover->default_retry_duration;
    weekgameover->duration_giveup = weekgameover->default_giveup_duration;
    weekgameover->duration_before = weekgameover->default_before_duration;
    weekgameover->duration_before_force_end = weekgameover->default_before_force_end_duration;

    weekgameover->modding = modding_init(layout, WEEK_GAMEOVER_MODDING_SCRIPT);
    weekgameover->modding->callback_private_data = weekgameover;
    weekgameover->modding->callback_option = week_gameover_internal_handle_modding_option;

    return weekgameover;
}

void week_gameover_destroy(WeekGameOver* weekgameover_ptr) {
    if (!weekgameover_ptr || !*weekgameover_ptr) return;

    WeekGameOver weekgameover = *weekgameover_ptr;

    layout_destroy(&weekgameover->layout);
    weekselector_helptext_destroy(&weekgameover->help_retry);
    weekselector_helptext_destroy(&weekgameover->help_difficult);
    weekselector_helptext_destroy(&weekgameover->help_giveup);
    weekselector_helptext_destroy(&weekgameover->help_stats);
    weekselector_difficult_destroy(&weekgameover->selector);
    drawable_destroy(&weekgameover->drawable);
    weekselector_weektitle_destroy(&weekgameover->choosen_difficult);
    if (weekgameover->sfx_die) soundplayer_destroy(&weekgameover->sfx_die);
    if (weekgameover->music_bg) soundplayer_destroy(&weekgameover->music_bg);
    if (weekgameover->sfx_retry) soundplayer_destroy(&weekgameover->sfx_retry);

    modding_destroy(&weekgameover->modding);

    free_chk(weekgameover);
    *weekgameover_ptr = NULL;
}

uint32_t week_gameover_read_version() {
    Grapheme garapheme = {.code = 0, .size = 0};
    char* version = fs_readtext(WEEK_GAMEOVER_LAYOUT_VERSION);

    if (string_is_not_empty(version)) {
        string_get_character_codepoint(version, 0, strlen(version), &garapheme);
    }

    free_chk(version);
    return garapheme.code;
}


void week_gameover_hide(WeekGameOver weekgameover) {
    weekgameover->disabled = true;
    weekgameover->weekinfo = NULL;
    weekgameover->difficult = NULL;
    weekselector_difficult_load(weekgameover->selector, NULL, NULL);
    drawable_set_visible(weekgameover->drawable, false);
}

void week_gameover_display(WeekGameOver weekgameover, float64 timestamp, float64 duration, PlayerStats playerstats, WeekInfo* weekinfo, const char* difficult) {
    float64 percent = math2d_min_double((timestamp / duration) * 100.0, 100.0);

    layout_set_group_visibility_by_id(weekgameover->layout, weekgameover->group_id_help, false);

    weekselector_difficult_visible(weekgameover->selector, false);
    layout_trigger_any(weekgameover->layout, NULL); // restart any animation and/or camera
    drawable_set_visible(weekgameover->drawable, true);

    TextSprite title = layout_get_textsprite(weekgameover->layout, "title");
    if (title) {
        const char** display_name = weekinfo->display_name ? &weekinfo->display_name : &weekinfo->name;
        textsprite_set_text_intern(title, true, (const char* const*)display_name);
    }

    TextSprite jugement = layout_get_textsprite(weekgameover->layout, "judgement");
    if (jugement) {
        char* time = math2d_timestamp_to_string(timestamp);
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
        free_chk(time);
    }

    weekgameover->weekinfo = weekinfo;
    weekgameover->difficult = difficult;
    weekgameover->disabled = true;
}

void week_gameover_display_selector(WeekGameOver weekgameover, bool load, bool visible) {
    if (visible && load) {
        weekselector_difficult_load(
            weekgameover->selector, weekgameover->weekinfo, weekgameover->difficult
        );
        weekselector_difficult_relayout(weekgameover->selector, false); // obligatory
    }

    weekselector_difficult_visible(weekgameover->selector, visible);
    layout_trigger_any(weekgameover->layout, visible ? "selector_show" : "selector_hide");
    modding_helper_notify_event(weekgameover->modding, visible ? "selector_show" : "selector_hide");
}

void week_gameover_display_choosen_difficult(WeekGameOver weekgameover) {
    layout_trigger_any(weekgameover->layout, "changed_difficult");
    weekselector_weektitle_move_difficult(weekgameover->choosen_difficult, weekgameover->selector);
    modding_helper_notify_event(weekgameover->modding, "changed_difficult");
}

Drawable week_gameover_get_drawable(WeekGameOver weekgameover) {
    return weekgameover->drawable;
}

const char* week_gameover_get_difficult(WeekGameOver weekgameover) {
    return weekgameover->difficult;
}

int32_t week_gameover_helper_ask_to_player(WeekGameOver weekgameover, RoundContext roundcontext) {
    GamepadButtons pressed = 0x00;
    Layout layout = roundcontext->layout ? roundcontext->layout : roundcontext->ui_layout;

    // trigger any initial Action
    if (roundcontext->settings.layout_rollback)
        layout_trigger_any(weekgameover->layout, NULL);
    else
        layout_trigger_any(weekgameover->layout, "hide_judgement");

    // match antialiasing with the stage layout
    layout_disable_antialiasing(weekgameover->layout, layout_is_antialiasing_disabled(layout));
    layout_set_layout_antialiasing(weekgameover->layout, layout_get_layout_antialiasing(layout));

    Gamepad controller = gamepad_init(-1);
    gamepad_set_buttons_delay(controller, WEEKSELECTOR_BUTTON_DELAY);
    gamepad_clear_buttons(controller);

    // pick values from the current stage layout (if exists)
    float die_animation_duration = layout_get_attached_value_as_float(
        layout, WEEK_GAMEOVER_DURATION_DIE, weekgameover->duration_die
    );
    float retry_animation_duration = layout_get_attached_value_as_float(
        layout, WEEK_GAMEOVER_DURATION_RETRY, weekgameover->duration_retry
    );
    float giveup_animation_duration = layout_get_attached_value_as_float(
        layout, WEEK_GAMEOVER_DURATION_GIVEUP, weekgameover->duration_giveup
    );
    float before_duration = layout_get_attached_value_as_float(
        layout, WEEK_GAMEOVER_DURATION_BEFORE, weekgameover->duration_giveup
    );
    float before_force_end_duration = layout_get_attached_value_as_float(
        layout, WEEK_GAMEOVER_DURATION_BEFORE_FORCE_END, weekgameover->duration_giveup
    );
    bool stage_has_gameover = layout_get_attached_value_boolean(
        layout, "gameover_with_stage", false
    );

    // ¿which player is dead?
    int32_t dead_player_index = roundcontext->players_size < 2 ? 0 : 1;
    Character dead_character = NULL;
    int32_t commited_anims = -1;
    bool wait_animation = false;

    if (roundcontext->girlfriend) {
        character_play_extra(roundcontext->girlfriend, "cry", false);
    }

    weekgameover->modding_decision = 1; // giveup by default
    weekgameover->modding->has_exit = false;
    weekgameover->modding->has_halt = false;
    weekgameover->modding->native_menu = NULL;
    modding_helper_notify_init(weekgameover->modding, MODDING_NATIVE_MENU_SCREEN, ModdingValueType_string);

    if (!layout_trigger_any(layout, "camera_gameover")) {
        for (int32_t i = 0; i < roundcontext->players_size; i++) {
            if (!roundcontext->players[i].playerstats) continue;
            if (roundcontext->players[i].is_opponent) {
                character_play_extra(roundcontext->players[i].character, FUNKIN_OPPONENT_VICTORY, false);
                continue;
            }

            if (playerstats_is_dead(roundcontext->players[i].playerstats)) {
                dead_player_index = i;
                dead_character = roundcontext->players[i].character;

                if (character_play_extra(dead_character, FUNKIN_PLAYER_DIES, false)) {
                    wait_animation = die_animation_duration < 0.0f;
                    commited_anims = character_get_commited_animations_count(dead_character);
                }

                const char* target = roundcontext->players[i].is_opponent ? WEEKROUND_CAMERA_OPONNENT : WEEKROUND_CAMERA_PLAYER;
                week_camera_focus_guess(roundcontext, target, i);
            } else {
                character_play_extra(roundcontext->players[i].character, FUNKIN_PLAYER_SCARED, false);
            }
        }
    }

    if (weekgameover->sfx_die) soundplayer_replay(weekgameover->sfx_die);

    // try draw only the dead player
    char* character_name = week_internal_concat_suffix(WEEKROUND_CHARACTER_PREFIX, dead_player_index);
    layout_set_single_item_to_draw(layout, character_name);
    free_chk(character_name);

    // trigger layout (normally shows the player only with a black background)
    if (stage_has_gameover) layout_trigger_any(layout, "gameover");

    int32_t decision = 0;
    bool difficult_selector_unloaded = true;
    bool difficult_selector_visible = false;
    float64 total = 0.0;
    bool gameoverloop_notified = false;

    while (true) {
        if (roundcontext->scriptcontext.halt_flag) {
            week_halt(roundcontext, true);
            continue;
        }

        float elapsed = pvrctx_wait_ready();

        if (roundcontext->script) weekscript_notify_frame(roundcontext->script, elapsed);

        ModdingHelperResult res = modding_helper_handle_custom_menu(weekgameover->modding, controller, elapsed);
        if (res != ModdingHelperResult_CONTINUE || weekgameover->modding->has_exit) {
            decision = weekgameover->modding_decision;
            break;
        }
        if (weekgameover->modding->has_halt) {
            gamepad_clear_buttons(controller);
            continue;
        }

        if (roundcontext->scriptcontext.force_end_flag) {
            gamepad_destroy(&controller);
            layout_set_single_item_to_draw(layout, NULL);

            modding_helper_notify_event(
                weekgameover->modding,
                roundcontext->scriptcontext.force_end_loose_or_win ? "force_loose" : "force_win"
            );
            modding_helper_notify_exit2(weekgameover->modding);

            return roundcontext->scriptcontext.force_end_loose_or_win ? 1 : 2;
        }

        // animate & draw first to minimize suttering
        pvr_context_reset(&pvr_context);
        layout_animate(layout, elapsed);

        week_gameover_draw2(weekgameover, roundcontext, &pvr_context);

        GamepadButtons buttons = gamepad_has_pressed_delayed(controller, WEEK_GAMEOVER_BUTTONS);

        if (difficult_selector_visible) {
            if (buttons & WEEK_GAMEOVER_BUTTONS_SELECTOR_CHOOSE_LEFT) {
                weekselector_difficult_scroll(weekgameover->selector, -1);
                buttons = 0x00;
            } else if (buttons & WEEK_GAMEOVER_BUTTONS_SELECTOR_CHOOSE_RIGHT) {
                weekselector_difficult_scroll(weekgameover->selector, 1);
                buttons = 0x00;
            } else if (buttons & WEEK_GAMEOVER_BUTTONS_SELECTOR_CLOSE) {
                // hide only if valid
                const char* difficult = weekselector_difficult_get_selected(weekgameover->selector);
                if (difficult != NULL) {
                    week_gameover_display_selector(weekgameover, false, false);
                    difficult_selector_visible = false;
                }
                buttons = 0x00;
            } else if (buttons & GAMEPAD_START) {
                // retry with the choosen difficult (if valid)
                const char* difficult = weekselector_difficult_get_selected(weekgameover->selector);
                if (difficult == NULL) {
                    buttons = 0x00;
                }
            } else {
                // nothing to do
                buttons = 0x00;
            }
        }

        if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
            // giveup
            decision = 1;
            break;
        } else if (buttons & GAMEPAD_START) {
            // retry
            decision = 2;
            weekgameover->disabled = false;

            if (!difficult_selector_unloaded) {
                // the string returned is constant
                const char* difficult = weekselector_difficult_get_selected(weekgameover->selector);

                if (difficult != NULL && !string_equals(difficult, weekgameover->difficult)) {
                    weekgameover->difficult = difficult;
                    week_gameover_display_choosen_difficult(weekgameover);
                }
            }
            break;
        } else if (buttons & WEEK_GAMEOVER_BUTTONS_SELECTOR_OPEN) {
            // show the selector and wait until the custom difficulties are readed from disk
            gamepad_clear_buttons(controller);
            week_gameover_display_selector(weekgameover, difficult_selector_unloaded, true);
            difficult_selector_unloaded = false;
            difficult_selector_visible = true;
        } else if (buttons & GAMEPAD_X) {
            bool is_visible = layout_get_group_visibility(weekgameover->layout, "judgement");
            layout_trigger_any(weekgameover->layout, is_visible ? "hide_judgement" : "show_judgement");
        }

        if (wait_animation) {
            if (commited_anims == character_get_commited_animations_count(dead_character)) continue;

            wait_animation = false;
            total = DOUBLE_Inf;
        }

        total += elapsed;
        if (weekgameover->disabled && total > die_animation_duration) {
            weekgameover->disabled = false;
            layout_set_group_visibility_by_id(weekgameover->layout, weekgameover->group_id_help, true);
            if (weekgameover->music_bg) {
                soundplayer_replay(weekgameover->music_bg);
                soundplayer_loop_enable(weekgameover->music_bg, true);
            }
            if (weekgameover->sfx_die) soundplayer_stop(weekgameover->sfx_die);

            gameoverloop_notified = true;
            if (roundcontext->script) weekscript_notify_gameoverloop(roundcontext->script);
            modding_helper_notify_event(weekgameover->modding, "gameover_loop");
        }
    }

    if (!gameoverloop_notified) {
        if (roundcontext->script) weekscript_notify_gameoverloop(roundcontext->script);
        modding_helper_notify_event(weekgameover->modding, "gameover_loop");
    }

    switch (decision) {
        case 1:
            modding_helper_notify_event(weekgameover->modding, "giveup");
            break;
        case 2:
            modding_helper_notify_event(weekgameover->modding, "retry");
            break;
    }

    if (dead_character) {
        const char* anim = decision == 2 ? FUNKIN_PLAYER_RETRY : FUNKIN_PLAYER_GIVEUP;
        if (!character_play_extra(dead_character, anim, false)) {
            // avoid waiting for retry/giveup animations
            dead_character = NULL;
        }
    }

    // if (weekgameover->sfx_die) soundplayer_stop(weekgameover->sfx_die);
    layout_set_group_visibility_by_id(weekgameover->layout, weekgameover->group_id_help, false);

    if (decision == 2) {
        layout_trigger_any(weekgameover->layout, "hide_stats");
        if (weekgameover->music_bg) soundplayer_stop(weekgameover->music_bg);
        if (weekgameover->sfx_retry) soundplayer_replay(weekgameover->sfx_retry);
    }

    if (roundcontext->script) {
        const char* new_difficult = string_equals(weekgameover->difficult, roundcontext->song_difficult) ? NULL : weekgameover->difficult;
        weekscript_notify_gameoverdecision(roundcontext->script, decision == 2, new_difficult);
        week_halt(roundcontext, true);
    }

    if (dead_character) commited_anims = character_get_commited_animations_count(dead_character);

    float before = decision == 1 ? before_force_end_duration : before_duration;
    bool trigger_transition = true;

    total = decision == 2 ? retry_animation_duration : giveup_animation_duration;
    wait_animation = total < 0.0 && dead_character;

    if (before < 0.0f) before = 0.0f;

    if (wait_animation)
        total = DOUBLE_Inf;
    else if (total < 0.0)
        total = before;

    while (total >= 0.0) {
        float elapsed = pvrctx_wait_ready();

        for (int32_t i = 0; i < roundcontext->players_size; i++) {
            Gamepad controller_player = roundcontext->players[i].controller;
            if (controller_player && gamepad_get_managed_presses(controller_player, true, &pressed)) {
                if (roundcontext->script) weekscript_notify_buttons(roundcontext->script, i, pressed);
            }
        }

        if (roundcontext->script) weekscript_notify_frame(roundcontext->script, elapsed);

        // animate & draw first to minimize suttering
        pvr_context_reset(&pvr_context);
        layout_animate(layout, elapsed);

        week_gameover_draw2(weekgameover, roundcontext, &pvr_context);

        if (gamepad_has_pressed_delayed(controller, GAMEPAD_START | GAMEPAD_A | GAMEPAD_X)) {
            layout_trigger_any(weekgameover->layout, "transition_force_end");
            break;
        }

        // wait for character animation ends (if required)
        if (wait_animation && commited_anims != character_get_commited_animations_count(dead_character)) {
            wait_animation = false;

            if (decision == 2 && weekgameover->sfx_retry && !soundplayer_has_ended(weekgameover->sfx_retry)) {
                // wait for retry sound effect (gameOverEnd.ogg is 7 seconds long)
                total = soundplayer_get_duration(weekgameover->sfx_retry) - soundplayer_get_position(weekgameover->sfx_retry);
                if (total <= 0.0) total = before; // ignore
            } else {
                total = before; // ignore
            }
        }

        if (trigger_transition) {
            total -= elapsed;
            if (total <= before) {
                total = DOUBLE_Inf;
                trigger_transition = false;
                layout_trigger_any(weekgameover->layout, decision == 2 ? "transition" : "transition_giveup");
                if (decision == 1 && weekgameover->music_bg) soundplayer_fade(weekgameover->music_bg, false, (float)total);
            }
        } else if (layout_animation_is_completed(weekgameover->layout, "transition_effect")) {
            break;
        }
    }

    if (weekgameover->sfx_die) soundplayer_stop(weekgameover->sfx_die);
    if (weekgameover->sfx_retry) soundplayer_stop(weekgameover->sfx_retry);

    gamepad_destroy(&controller);
    layout_set_single_item_to_draw(layout, NULL);
    modding_helper_notify_exit2(weekgameover->modding);

    return decision;
}


static int32_t week_gameover_animate(WeekGameOver weekgameover, float elapsed) {
    if (weekgameover->disabled) return 1;
    return layout_animate(weekgameover->layout, elapsed);
}

static void week_gameover_draw(WeekGameOver weekgameover, PVRContext pvrctx) {
    if (weekgameover->disabled) return;
    layout_draw(weekgameover->layout, pvrctx);
}

static void week_gameover_draw2(WeekGameOver weekgameover, RoundContext roundcontext, PVRContext pvrctx) {
    sprite_draw(roundcontext->screen_background, pvrctx);
    layout_draw(roundcontext->layout ? roundcontext->layout : roundcontext->ui_layout, pvrctx);
    if (weekgameover->disabled) return;
    layout_draw(weekgameover->layout, pvrctx);
}


void week_gameover_set_option(WeekGameOver weekgameover, int32_t option, float nro, const char* str) {
    switch (option) {
        case WEEK_GAMEOVER_NOMUSIC:
            if (weekgameover->music_bg) {
                soundplayer_destroy(&weekgameover->music_bg);
                weekgameover->music_bg = NULL;
            }
            return;
        case WEEK_GAMEOVER_NOSFXDIE:
            if (weekgameover->sfx_die) {
                soundplayer_destroy(&weekgameover->sfx_die);
                weekgameover->sfx_die = NULL;
            }
            return;
        case WEEK_GAMEOVER_NOSFXRETRY:
            if (weekgameover->sfx_retry) {
                soundplayer_destroy(&weekgameover->sfx_retry);
                weekgameover->sfx_retry = NULL;
            }
            return;
        case WEEK_GAMEOVER_ANIMDURATIONDIE:
            weekgameover->duration_die = math2d_is_float_NaN(nro) ? weekgameover->default_die_duration : nro;
            return;
        case WEEK_GAMEOVER_ANIMDURATIONRETRY:
            weekgameover->duration_retry = math2d_is_float_NaN(nro) ? weekgameover->default_retry_duration : nro;
            return;
        case WEEK_GAMEOVER_ANIMDURATIONGIVEUP:
            weekgameover->duration_giveup = math2d_is_float_NaN(nro) ? weekgameover->default_giveup_duration : nro;
            return;
        case WEEK_GAMEOVER_ANIMDURATIONBEFORE:
            weekgameover->duration_before = math2d_is_float_NaN(nro) ? weekgameover->default_before_duration : nro;
            return;
        case WEEK_GAMEOVER_ANIMDURATIONBEFOREFORCEEND:
            weekgameover->duration_before_force_end = math2d_is_float_NaN(nro) ? weekgameover->default_before_force_end_duration : nro;
            return;
        case WEEK_GAMEOVER_SETMUSIC:
            if (weekgameover->music_bg) soundplayer_destroy(&weekgameover->music_bg);
            weekgameover->music_bg = soundplayer_init(str ? str : "/assets/common/music/gameOver.ogg");
            return;
        case WEEK_GAMEOVER_SETSFXDIE:
            if (weekgameover->sfx_die) soundplayer_destroy(&weekgameover->sfx_die);
            weekgameover->sfx_die = soundplayer_init(str ? str : "/assets/common/sound/loss_sfx.ogg");
            return;
        case WEEK_GAMEOVER_SETSFXRETRY:
            if (weekgameover->sfx_retry) soundplayer_destroy(&weekgameover->sfx_retry);
            weekgameover->sfx_retry = soundplayer_init(str ? str : "/assets/common/sound/gameOverEnd.ogg");
            return;
    }
}

Layout week_gameover_get_layout(WeekGameOver weekgameover) {
    return weekgameover->layout;
}


static bool week_gameover_internal_handle_modding_option(void* obj, const char* option) {
    WeekGameOver weekgameover = (WeekGameOver)obj;

    if (string_equals(option, "giveup")) {
        weekgameover->modding_decision = 1;
    } else if (string_equals(option, "retry")) {
        weekgameover->modding_decision = 2;
    } else {
        // unknown option
        return false;
    }

    return true;
}
