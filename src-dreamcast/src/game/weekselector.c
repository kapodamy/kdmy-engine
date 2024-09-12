#include "game/weekselector.h"

#include "beatwatcher.h"
#include "fs.h"
#include "game/common/funkin.h"
#include "game/common/funkinsave.h"
#include "game/common/weekenumerator.h"
#include "game/gameplay/week.h"
#include "game/helpers/weekselector_difficult.h"
#include "game/helpers/weekselector_helptext.h"
#include "game/helpers/weekselector_mdlselect.h"
#include "game/helpers/weekselector_weeklist.h"
#include "game/helpers/weekselector_weekmsg.h"
#include "game/helpers/weekselector_weektitle.h"
#include "game/main.h"
#include "game/modding.h"
#include "game/savemanager.h"
#include "gamepad.h"
#include "layout.h"
#include "malloc_utils.h"
#include "modelholder.h"
#include "preloadcache.h"
#include "pvrcontext_global.h"
#include "soundplayer.h"
#include "sprite.h"
#include "stringbuilder.h"
#include "stringutils.h"
#include "textsprite.h"
#include "texture.h"


static const char* WEEKSELECTOR_SCORE = "WEEK SCORE: ";
static const char* WEEKSELECTOR_SONGS = "TRACKS";
static const char* WEEKSELECTOR_SONGS_MORE = "...";
static const char* WEEKSELECTOR_COMPLETED = "WEEKS COMPLETED";
static const char* WEEKSELECTOR_CHOOSEN = "WEEK CHOOSEN";
static const char* WEEKSELECTOR_ALT_WARNING = "SONGS WARNING";
static const char* WEEKSELECTOR_SENSIBLE_CONTENT = "SENSIBLE CONTENT";
static const char* WEEKSELECTOR_HELP_N_ALT = "USE ALTERNATIVE TRACKS";
static const char* WEEKSELECTOR_HELP_W_ALT = "USE DEFAULT TRACKS";
static const char* WEEKSELECTOR_HELP_GFBF_SWITCH = "SWITCH BOYFRIEND/GIRLFRIEND";
static const char* WEEKSELECTOR_HELP_SELECT_WEEK = "SELECT ANOTHER WEEK";
static const char* WEEKSELECTOR_HELP_BACK = "BACK";
static const char* WEEKSELECTOR_HELP_START = "PLAY THIS WEEK";

const char* WEEKSELECTOR_BUTTON_X = "x";
const char* WEEKSELECTOR_BUTTON_B = "b";
const char* WEEKSELECTOR_BUTTON_LT_RT = "lt_rt";
const char* WEEKSELECTOR_BUTTON_START = "start";

const char* WEEKSELECTOR_LOCKED = "locked";

const char* WEEKSELECTOR_BUTTONS_MODEL = "/assets/common/image/week-selector/dreamcast_buttons.xml";
const char* WEEKSELECTOR_UI_ICONS_MODEL = "/assets/common/image/week-selector/week_selector_UI.xml";
const char* WEEKSELECTOR_UI_ANIMS = "/assets/common/image/week-selector/ui_anims.xml";

const char* WEEKSELECTOR_ARROW_SPRITE_NAME = "arrow_L push";

static const GamepadButtons WEEKSELECTOR_BUTTONS_OK = GAMEPAD_X | GAMEPAD_A;

static const char* WEEKSELECTOR_LAYOUT = "/assets/common/image/week-selector/layout.xml";
static const char* WEEKSELECTOR_LAYOUT_DREAMCAST = "/assets/common/image/week-selector/layout~dreamcast.xml";
static const char* WEEKSELECTOR_MODDING_SCRIPT = "/assets/common/data/scripts/weekselector.lua";


typedef enum {
    SND_CANCEL = 1,
    SND_SCROLL = 2,
    SND_ASTERIK = 3
} SND;

typedef struct {
    Gamepad maple_pads;

    WeekSelectorHelpText helptext_alternate;
    WeekSelectorHelpText helptext_back;
    WeekSelectorWeekMSG weekmsg_sensible;
    WeekSelectorWeekMSG weekmsg_alternate;
    WeekSelectorDifficult weekdifficult;
    WeekSelectorWeekList weeklist;
    WeekSelectorWeekTitle weektitle;
    WeekSelectorHelpText helptext_bfgf;
    WeekSelectorHelpText helptext_start;
    TextSprite weeks_choosen_index;
    Sprite week_background_color;

    WeekSelectorMdlSelect mdl_boyfriend;
    WeekSelectorMdlSelect mdl_girlfriend;
    TextSprite songs_list;
    TextSprite score;
    TextSprite description;

    Layout layout;
    StringBuilder stringbuilder;
} UI;

typedef struct {
    WeekInfo* weekinfo;

    bool update_ui;
    bool update_extra_ui;
    int32_t page_selected_ui;
    SND play_sound;

    bool choosing_boyfriend;

    bool week_unlocked;

    bool has_choosen_alternate;
    const char* difficult;
    bool can_start;

    bool quit;
    bool back_to_main_menu;

    uint32_t default_background_color;
    Layout custom_layout;

    bool bg_music_paused;
    SoundPlayer bg_music;
    Modding modding;
} STATE;


static void weekselector_page0(UI* ui, STATE* state);
static void weekselector_page1(UI* ui, STATE* state);
static void weekselector_page2(UI* ui, STATE* state);
static void weekselector_trigger_event(UI* ui, STATE* state, const char* name);


static void weekselector_show_week_info(UI* ui, WeekInfo* weekinfo, int64_t score) {
    float max_draw_width = 0.0f, max_draw_height = 0.0f;
    float font_size = textsprite_get_font_size(ui->songs_list);
    textsprite_get_max_draw_size(ui->songs_list, &max_draw_width, &max_draw_height);

    int32_t songs_count = weekinfo->songs_count;
    int32_t max_lines = (int32_t)(max_draw_height / font_size) - 1;
    bool has_overflow;
    if (songs_count > max_lines) {
        has_overflow = true;
        songs_count = max_lines;
    } else {
        has_overflow = false;
    }

    StringBuilder list = ui->stringbuilder;
    stringbuilder_clear(ui->stringbuilder);

    for (int32_t i = 0; i < songs_count; i++) {
        if (weekinfo->songs[i].freeplay_only) continue;
        stringbuilder_add(list, "\n");
        stringbuilder_add(list, weekinfo->songs[i].name);
    }
    if (has_overflow) {
        stringbuilder_add(list, "\n");
        stringbuilder_add(list, WEEKSELECTOR_SONGS_MORE);
    }

    // set the strings
    textsprite_set_text_formated(ui->score, "$s$l", WEEKSELECTOR_SCORE, score);
    textsprite_set_text_intern(ui->description, true, (const char* const*)&weekinfo->description);
    textsprite_set_text_intern(ui->songs_list, true, stringbuilder_intern(ui->stringbuilder));
}

static void weekselector_change_page(UI* ui, int32_t page) {
    // hide everything first
    layout_set_group_visibility(ui->layout, "ui_week_selector", page == 0);
    layout_set_group_visibility(ui->layout, "ui_difficult_selector", page == 1);
    layout_set_group_visibility(ui->layout, "ui_character_selector", page == 2);

    weekselector_helptext_set_visible(ui->helptext_alternate, false);
    weekselector_helptext_set_visible(ui->helptext_bfgf, false);
    weekselector_helptext_set_visible(ui->helptext_back, page > 0);
    weekselector_helptext_set_visible(ui->helptext_start, false);
}

static void weekselector_trigger_difficult_change(UI* ui, STATE* state, bool empty) {
    weekselector_trigger_event(ui, state, "change-difficult");

    if (empty) return;

    const char* default_difficult = weekselector_difficult_get_selected(ui->weekdifficult);
    if (default_difficult) {
        char* difficult_action = string_concat(2, "selected-difficult-", default_difficult);
        weekselector_trigger_event(ui, state, difficult_action);
        free_chk(difficult_action);
    }
}

static void weekselector_trigger_alternate_change(UI* ui, STATE* state) {
    const char* what = state->has_choosen_alternate ? "selected-no-alternate" : "selected-alternate";
    weekselector_trigger_event(ui, state, what);
}

static void weekselector_load_custom_week_background(STATE* state) {
    if (state->custom_layout) {
        layout_destroy(&state->custom_layout);
        state->custom_layout = NULL;
    }

    if (!state->weekinfo->custom_selector_layout) {
        if (state->bg_music_paused && state->bg_music) soundplayer_play(state->bg_music);
        return;
    }

    bool has_custom_bg_music = false;

    state->custom_layout = layout_init(state->weekinfo->custom_selector_layout);
    if (state->custom_layout) {
        layout_trigger_any(state->custom_layout, "show-principal");
        if (!state->week_unlocked) layout_trigger_any(state->custom_layout, "week-locked");

        has_custom_bg_music = layout_get_attached_value_boolean(
            state->custom_layout, "has_background_music", has_custom_bg_music
        );
    }

    if (state->bg_music) {
        if (!state->bg_music_paused && has_custom_bg_music)
            soundplayer_play(state->bg_music);
        else if (state->bg_music_paused && !has_custom_bg_music)
            soundplayer_pause(state->bg_music);

        state->bg_music_paused = has_custom_bg_music;
    }
}

static void weekselector_set_text_int(Layout layout, const char* name, const char* format, int32_t integer) {
    TextSprite textsprite = layout_get_textsprite(layout, name);
    if (!textsprite) return;

    assert(format);
    textsprite_set_text_formated(textsprite, format, integer);
}

static void weekselector_set_text_string(Layout layout, const char* name, const char** string_ptr) {
    TextSprite textsprite = layout_get_textsprite(layout, name);
    if (!textsprite) return;
    textsprite_set_text_intern(textsprite, true, (const char* const*)string_ptr);
}

static void weekselector_trigger_menu_action(UI* ui, STATE* state, const char* name, bool selected, bool choosen) {
    if (!state->weekinfo) return;
    main_helper_trigger_action_menu(
        ui->layout,
        state->weekinfo->display_name ? state->weekinfo->display_name : state->weekinfo->name,
        name, selected, choosen
    );
}

static void weekselector_trigger_event(UI* ui, STATE* state, const char* name) {
    if (state->custom_layout) layout_trigger_any(state->custom_layout, name);
    layout_trigger_any(ui->layout, name);
    modding_helper_notify_event(state->modding, name);
}


int32_t weekselector_main() {
    Layout layout = layout_init(pvr_context_is_widescreen() ? WEEKSELECTOR_LAYOUT : WEEKSELECTOR_LAYOUT_DREAMCAST);
    AnimList animlist_ui = animlist_init(WEEKSELECTOR_UI_ANIMS);
    ModelHolder modelholder_ui = modelholder_init(WEEKSELECTOR_UI_ICONS_MODEL);
    ModelHolder modelholder_buttons_ui = modelholder_init(WEEKSELECTOR_BUTTONS_MODEL);

    if (!modelholder_ui) {
        logger_error("Failed to load: %s", WEEKSELECTOR_UI_ICONS_MODEL);
        assert(modelholder_ui);
    }
    if (!modelholder_buttons_ui) {
        logger_error("Failed to load: %s", WEEKSELECTOR_BUTTONS_MODEL);
        assert(modelholder_buttons_ui);
    }

    SoundPlayer sound_confirm = soundplayer_init("/assets/common/sound/confirmMenu.ogg");
    SoundPlayer sound_scroll = soundplayer_init("/assets/common/sound/scrollMenu.ogg");
    SoundPlayer sound_cancel = soundplayer_init("/assets/common/sound/cancelMenu.ogg");
    SoundPlayer sound_asterik = soundplayer_init("/assets/common/sound/asterikMenu.ogg");

    beatwatcher_global_set_timestamp_from_kos_timer();

    // store all ui elements here (makes the code more readable)
    UI ui = {
        .week_background_color = layout_get_sprite(layout, "week_background_color"),

        // week score&description
        .score = layout_get_textsprite(layout, "week_score"),
        .description = layout_get_textsprite(layout, "week_description"),

        // week selector ui elements
        .songs_list = layout_get_textsprite(layout, "ui_songs_list"),
        .weeks_choosen_index = layout_get_textsprite(layout, "ui_weeks_choosen_index"),
        .layout = layout,

        // selected week warning messages
        .weekmsg_sensible = weekselector_weekmsg_init(
            layout, WEEKSELECTOR_SENSIBLE_CONTENT,
            "warning_sensible_title", "warning_sensible_message"
        ),
        .weekmsg_alternate = weekselector_weekmsg_init(
            layout, WEEKSELECTOR_ALT_WARNING,
            "warning_alternate_title", "warning_alternate_message"
        ),

        .weeklist = weekselector_weeklist_init(animlist_ui, modelholder_ui, layout),
        .weektitle = weekselector_weektitle_init(layout),

        .mdl_boyfriend = weekselector_mdlselect_init(
            animlist_ui, modelholder_ui, layout, true
        ),
        .mdl_girlfriend = weekselector_mdlselect_init(
            animlist_ui, modelholder_ui, layout, false
        ),

        .weekdifficult = weekselector_difficult_init(animlist_ui, modelholder_ui, layout),

        .helptext_alternate = weekselector_helptext_init(
            modelholder_buttons_ui, layout, 3, false, WEEKSELECTOR_BUTTON_X,
            WEEKSELECTOR_HELP_N_ALT, WEEKSELECTOR_HELP_W_ALT
        ),
        .helptext_bfgf = weekselector_helptext_init(
            modelholder_buttons_ui, layout, 2, false, WEEKSELECTOR_BUTTON_LT_RT,
            WEEKSELECTOR_HELP_GFBF_SWITCH, NULL
        ),
        .helptext_back = weekselector_helptext_init(
            modelholder_buttons_ui, layout, 1, false, WEEKSELECTOR_BUTTON_B,
            WEEKSELECTOR_HELP_BACK, WEEKSELECTOR_HELP_SELECT_WEEK
        ),
        .helptext_start = weekselector_helptext_init(
            modelholder_buttons_ui, layout, 1, true, WEEKSELECTOR_BUTTON_START,
            WEEKSELECTOR_HELP_START, NULL
        ),

        .maple_pads = gamepad_init(-1),
        .stringbuilder = stringbuilder_init(64)
    };

    // dispose unused resources
    modelholder_destroy(&modelholder_ui);
    modelholder_destroy(&modelholder_buttons_ui);
    animlist_destroy(&animlist_ui);

    // funkin stuff
    int32_t weeks_completed = funkinsave_get_completed_count();
    const char* last_difficult_played = funkinsave_get_last_played_difficult();

    // setup text boxes
    weekselector_set_text_string(layout, "ui_songs_title", &WEEKSELECTOR_SONGS);
    weekselector_set_text_string(layout, "ui_weeks_completed_title", &WEEKSELECTOR_COMPLETED);
    weekselector_set_text_int(layout, "ui_weeks_completed_count", "$i", weeks_completed);
    weekselector_set_text_string(layout, "ui_weeks_choosen_title", &WEEKSELECTOR_CHOOSEN);

    // attach help texts to the layout
    int32_t ui_group = layout_external_create_group(layout, NULL, 0);
    layout_external_vertex_create_entries(layout, 4);
    layout_external_vertex_set_entry(
        layout, 0, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(ui.helptext_alternate), ui_group
    );
    layout_external_vertex_set_entry(
        layout, 1, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(ui.helptext_bfgf), ui_group
    );
    layout_external_vertex_set_entry(
        layout, 2, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(ui.helptext_back), ui_group
    );
    layout_external_vertex_set_entry(
        layout, 3, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(ui.helptext_start), ui_group
    );

    gamepad_set_buttons_delay(ui.maple_pads, WEEKSELECTOR_BUTTON_DELAY);

    Modding modding = modding_init(layout, WEEKSELECTOR_MODDING_SCRIPT);
    modding->native_menu = NULL;
    modding->callback_option = NULL;
    modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN, ModdingValueType_string);

    STATE state = {
        .weekinfo = weekselector_weeklist_get_selected(ui.weeklist),
        .difficult = last_difficult_played,

        .week_unlocked = false,
        .play_sound = 0,

        .default_background_color = 0x00,

        .can_start = false,
        .choosing_boyfriend = true,
        .has_choosen_alternate = false,

        .update_extra_ui = true,
        .page_selected_ui = 0,
        .update_ui = true,
        .quit = false,
        .back_to_main_menu = false,

        .custom_layout = NULL,
        .bg_music_paused = false,
        .bg_music = layout_get_soundplayer(layout, "background_music"),
        .modding = modding
    };

    if (!state.bg_music) state.bg_music = background_menu_music;

    if (state.bg_music != background_menu_music && background_menu_music) {
        soundplayer_pause(background_menu_music);
    }

    if (ui.week_background_color)
        state.default_background_color = sprite_get_vertex_color_rgb8(ui.week_background_color);

    weekselector_change_page(&ui, state.page_selected_ui);
    state.week_unlocked = funkinsave_contains_unlock_directive(state.weekinfo->unlock_directive);
    weekselector_load_custom_week_background(&state);
    weekselector_trigger_menu_action(&ui, &state, "weeklist", true, false);

    while (!state.quit) {
        float elapsed = pvrctx_wait_ready();
        int32_t page_selected_ui = state.page_selected_ui;

        beatwatcher_global_set_timestamp_from_kos_timer();

        ModdingHelperResult res = modding_helper_handle_custom_menu(state.modding, ui.maple_pads, elapsed);

        if (state.modding->has_exit || res == ModdingHelperResult_BACK) {
            state.back_to_main_menu = true;
            state.quit = true;
            break;
        }

        if (!state.modding->has_halt) {
            switch (page_selected_ui) {
                case 0:
                    weekselector_page0(&ui, &state);
                    break;
                case 1:
                    weekselector_page1(&ui, &state);
                    break;
                case 2:
                    weekselector_page2(&ui, &state);
                    break;
            }
        }

        if (state.play_sound) {
            if (sound_cancel) soundplayer_stop(sound_cancel);
            if (sound_scroll) soundplayer_stop(sound_scroll);
            if (sound_asterik) soundplayer_stop(sound_asterik);

            SoundPlayer sound = NULL;
            switch (state.play_sound) {
                case SND_CANCEL:
                    sound = sound_cancel;
                    break;
                case SND_SCROLL:
                    sound = sound_scroll;
                    break;
                case SND_ASTERIK:
                    sound = sound_asterik;
                    break;
            }

            state.play_sound = 0;
            if (sound) soundplayer_play(sound);
        }

        pvr_context_reset(&pvr_context);
        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        if (state.custom_layout) {
            layout_animate(state.custom_layout, elapsed);
            layout_draw(state.custom_layout, &pvr_context);
        }

        state.update_ui = page_selected_ui != state.page_selected_ui;
        if (state.update_ui) weekselector_change_page(&ui, state.page_selected_ui);
    }

    Layout outro_layout = layout;
    if (state.back_to_main_menu) {
        if (sound_cancel) soundplayer_replay(sound_cancel);
        weekselector_trigger_event(&ui, &state, "back-to-main-menu");
    } else {
        if (state.bg_music) soundplayer_stop(state.bg_music);
        weekselector_change_page(&ui, false);
        if (sound_confirm) soundplayer_replay(sound_confirm);

        if (state.custom_layout && layout_trigger_any(state.custom_layout, "week-choosen")) {
            outro_layout = state.custom_layout;
        } else {
            // if the custom layout does not contains the "week-choosen" action
            // trigger in the main layout
            layout_trigger_any(layout, "week-choosen");
        }

        modding_helper_notify_event(state.modding, "week-choosen");
        layout_set_group_visibility(layout, "ui_game_progress", false);
        weekselector_weektitle_move_difficult(ui.weektitle, ui.weekdifficult);
        weekselector_mdlselect_toggle_choosen(ui.mdl_boyfriend);
        weekselector_mdlselect_toggle_choosen(ui.mdl_girlfriend);
        weekselector_weeklist_toggle_choosen(ui.weeklist);
    }

    while (true) {
        float elapsed = pvrctx_wait_ready();

        beatwatcher_global_set_timestamp_from_kos_timer();
        modding_helper_notify_frame(modding, elapsed, -1.0);

        pvr_context_reset(&pvr_context);
        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        if (state.custom_layout) {
            layout_animate(state.custom_layout, elapsed);
            layout_draw(state.custom_layout, &pvr_context);
        }

        if (layout_animation_is_completed(outro_layout, "transition_effect")) {
            // flush framebuffer again with last fade frame
            pvrctx_wait_ready();
            break;
        }
    }

    // gameplay parameters
    WeekInfo* gameplay_weekinfo = state.weekinfo;
    const char* gameplay_difficult = weekselector_difficult_get_selected(ui.weekdifficult);
    bool gameplay_alternative_tracks = state.has_choosen_alternate;
    char* gameplay_model_boyfriend = state.back_to_main_menu ? NULL : weekselector_mdlselect_get_manifest(ui.mdl_boyfriend);
    char* gameplay_model_girlfriend = state.back_to_main_menu ? NULL : weekselector_mdlselect_get_manifest(ui.mdl_girlfriend);

    modding_helper_notify_exit2(modding);

    // dispose everything used
    weekselector_weeklist_destroy(&ui.weeklist);
    weekselector_difficult_destroy(&ui.weekdifficult);
    weekselector_weektitle_destroy(&ui.weektitle);

    weekselector_mdlselect_destroy(&ui.mdl_boyfriend);
    weekselector_mdlselect_destroy(&ui.mdl_girlfriend);

    gamepad_destroy(&ui.maple_pads);

    weekselector_helptext_destroy(&ui.helptext_alternate);
    weekselector_helptext_destroy(&ui.helptext_bfgf);
    weekselector_helptext_destroy(&ui.helptext_back);
    weekselector_helptext_destroy(&ui.helptext_start);

    weekselector_weekmsg_destroy(&ui.weekmsg_sensible);
    weekselector_weekmsg_destroy(&ui.weekmsg_alternate);

    if (sound_confirm) soundplayer_destroy(&sound_confirm);
    if (sound_asterik) soundplayer_destroy(&sound_asterik);
    if (sound_scroll) soundplayer_destroy(&sound_scroll);
    if (sound_cancel) soundplayer_destroy(&sound_cancel);

    stringbuilder_destroy(&ui.stringbuilder);

    if (state.custom_layout) layout_destroy(&state.custom_layout);

    layout_destroy(&layout);
    modding_destroy(&modding);

    if (state.back_to_main_menu) {
        savemanager_check_and_save_changes();
        return 0;
    }

    if (background_menu_music) {
        soundplayer_destroy(&background_menu_music);
        background_menu_music = NULL;
    }

    funkinsave_set_last_played(gameplay_weekinfo->name, gameplay_difficult);

    main_helper_draw_loading_screen();

    // before run check if necessary preload files
    char* preload_filelist = weekenumerator_get_asset(gameplay_weekinfo, PRELOADCACHE_PRELOAD_FILENAME);
    preloadcache_add_file_list(preload_filelist);
    free_chk(preload_filelist);

    int32_t week_result = week_main(
        gameplay_weekinfo,
        gameplay_alternative_tracks,
        gameplay_difficult,
        gameplay_model_boyfriend,
        gameplay_model_girlfriend,
        NULL,
        -1,
        NULL
    );

    free_chk(gameplay_model_boyfriend);
    free_chk(gameplay_model_girlfriend);

    background_menu_music = soundplayer_init(FUNKIN_BACKGROUND_MUSIC);
    if (background_menu_music) {
        soundplayer_loop_enable(background_menu_music, true);
        soundplayer_replay(background_menu_music);
    }

    return week_result;
}


static void weekselector_page0(UI* ui, STATE* state) {
    int32_t seek_offset = 0;

    if (state->update_ui) weekselector_helptext_set_visible(ui->helptext_start, state->can_start);

    if (state->update_extra_ui) {
        int64_t score = funkinsave_get_week_score(state->weekinfo->name, state->difficult);
        int32_t index = weekselector_weeklist_get_selected_index(ui->weeklist) + 1;
        weekselector_show_week_info(ui, state->weekinfo, score);
        textsprite_set_text_formated(ui->weeks_choosen_index, "$i/$i", index, weeks_array.size);

        if (ui->week_background_color) {
            uint32_t background_color;
            if (state->weekinfo->selector_background_color_override)
                background_color = state->weekinfo->selector_background_color;
            else
                background_color = state->default_background_color;
            sprite_set_vertex_color_rgb8(ui->week_background_color, background_color);
        }

        state->update_extra_ui = false;
    }

    GamepadButtons buttons = gamepad_has_pressed_delayed(
        ui->maple_pads,
        GAMEPAD_AD_UP | GAMEPAD_AD_DOWN | GAMEPAD_X | GAMEPAD_A | GAMEPAD_START | GAMEPAD_B | GAMEPAD_BACK
    );

    if (buttons & GAMEPAD_AD_UP) {
        seek_offset = -1;
    } else if (buttons & GAMEPAD_AD_DOWN) {
        seek_offset = 1;
    } else if (buttons & WEEKSELECTOR_BUTTONS_OK) {
        if (state->week_unlocked) {
            state->play_sound = SND_SCROLL;
            state->page_selected_ui = 1;
        } else {
            state->play_sound = SND_ASTERIK;
        }
    } else if (buttons & GAMEPAD_START) {
        state->quit = state->can_start;
        if (!state->quit && state->week_unlocked) {
            state->play_sound = SND_SCROLL;
            state->page_selected_ui = 1;
        }
    } else if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
        state->quit = true;
        state->back_to_main_menu = true;
        return;
    }

    if (state->page_selected_ui == 1) {
        weekselector_trigger_event(ui, state, "principal-hide");
        weekselector_trigger_event(ui, state, "difficult-selector-show");
    }

    if (seek_offset == 0) return;

    if (!weekselector_weeklist_scroll(ui->weeklist, seek_offset)) {
        state->play_sound = SND_ASTERIK;
        return;
    }

    weekselector_trigger_menu_action(ui, state, "weeklist", false, false);

    state->weekinfo = weekselector_weeklist_get_selected(ui->weeklist);
    state->week_unlocked = funkinsave_contains_unlock_directive(state->weekinfo->unlock_directive);
    state->play_sound = SND_SCROLL;
    state->can_start = false;
    state->update_extra_ui = true;
    state->has_choosen_alternate = false;

    weekselector_helptext_set_visible(ui->helptext_start, false);
    weekselector_mdlselect_select_default(ui->mdl_boyfriend);
    weekselector_mdlselect_select_default(ui->mdl_girlfriend);

    weekselector_load_custom_week_background(state);
    weekselector_trigger_menu_action(ui, state, "weeklist", true, false);
}

static void weekselector_page1(UI* ui, STATE* state) {
    int32_t seek_offset = 0;

    if (state->update_ui) {
        WeekInfo* weekinfo = state->weekinfo;
        bool has_alternate = weekinfo->warning_message != NULL;
        bool has_warnings = has_alternate && string_is_not_empty(weekinfo->sensible_content_message);

        weekselector_helptext_set_visible(
            ui->helptext_bfgf,
            !(weekinfo->disallow_custom_boyfriend && weekinfo->disallow_custom_girlfriend)
        );

        weekselector_difficult_load(ui->weekdifficult, weekinfo, state->difficult);
        weekselector_weektitle_move(ui->weektitle, ui->weeklist, has_warnings);

        weekselector_helptext_set_visible(ui->helptext_alternate, has_alternate);
        weekselector_weekmsg_set_message(ui->weekmsg_sensible, weekinfo->sensible_content_message);
        weekselector_weekmsg_set_message(ui->weekmsg_alternate, weekinfo->warning_message);
        weekselector_weekmsg_disabled(ui->weekmsg_alternate, state->has_choosen_alternate);
        weekselector_difficult_relayout(ui->weekdifficult, has_warnings);

        weekselector_helptext_set_visible(ui->helptext_start, !has_warnings);
        weekselector_trigger_difficult_change(ui, state, false);
        weekselector_trigger_alternate_change(ui, state);
    }

    GamepadButtons buttons = gamepad_has_pressed_delayed(
        ui->maple_pads,
        GAMEPAD_AD_LEFT | GAMEPAD_AD_RIGHT | GAMEPAD_X | GAMEPAD_B |
            GAMEPAD_T_LR | GAMEPAD_A | GAMEPAD_START | GAMEPAD_BACK
    );

    if (buttons & GAMEPAD_AD_LEFT) {
        seek_offset = -1;
    } else if (buttons & GAMEPAD_AD_RIGHT) {
        seek_offset = 1;
    } else if (buttons & GAMEPAD_X) {
        if (state->weekinfo->warning_message) {
            state->has_choosen_alternate = !state->has_choosen_alternate;
            weekselector_helptext_use_alt(ui->helptext_alternate, state->has_choosen_alternate);
            weekselector_weekmsg_disabled(ui->weekmsg_alternate, state->has_choosen_alternate);
            state->play_sound = SND_SCROLL;
            weekselector_trigger_alternate_change(ui, state);
        }
    } else if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
        state->page_selected_ui = 0;
        state->has_choosen_alternate = false;
        state->play_sound = SND_CANCEL;
    } else if (buttons & GAMEPAD_T_LR) {
        if (state->weekinfo->disallow_custom_boyfriend && state->weekinfo->disallow_custom_girlfriend) {
            state->play_sound = SND_ASTERIK;
        } else {
            state->choosing_boyfriend = (buttons & GAMEPAD_TRIGGER_LEFT) == GAMEPAD_TRIGGER_LEFT;
            state->page_selected_ui = 2;
            state->play_sound = SND_SCROLL;
        }
    } else if (buttons & GAMEPAD_A) {
        if (weekselector_difficult_selected_is_locked(ui->weekdifficult)) {
            state->play_sound = SND_ASTERIK;
        } else {
            state->page_selected_ui = 0;
            state->can_start = true;
        }
    } else if (buttons & GAMEPAD_START) {
        if (weekselector_difficult_selected_is_locked(ui->weekdifficult))
            state->play_sound = SND_ASTERIK;
        else
            state->quit = true;
    }

    if (seek_offset == 0) return;

    if (state->page_selected_ui == 0) {
        weekselector_trigger_event(ui, state, "difficult-selector-hide");
        weekselector_trigger_event(ui, state, "principal-show");
    }

    if (weekselector_difficult_scroll(ui->weekdifficult, seek_offset)) {
        state->play_sound = SND_SCROLL;
        state->can_start = true;
        state->difficult = weekselector_difficult_get_selected(ui->weekdifficult);

        weekselector_trigger_difficult_change(ui, state, false);
        weekselector_show_week_info(
            ui, state->weekinfo, funkinsave_get_week_score(state->weekinfo->name, state->difficult)
        );
    } else {
        state->play_sound = SND_ASTERIK;
    }
}

static void weekselector_page2(UI* ui, STATE* state) {
    int32_t seek_offset = 0;
    int32_t character_model_seek = 0;
    bool no_custom_boyfriend = state->weekinfo->disallow_custom_boyfriend;
    bool no_custom_girlfriend = state->weekinfo->disallow_custom_girlfriend;

    if (state->update_ui) {
        if (state->choosing_boyfriend && no_custom_boyfriend) state->choosing_boyfriend = false;
        if (!state->choosing_boyfriend && no_custom_girlfriend) state->choosing_boyfriend = true;
        weekselector_mdlselect_select_default(ui->mdl_boyfriend);
        weekselector_mdlselect_select_default(ui->mdl_girlfriend);
        weekselector_mdlselect_enable_arrows(ui->mdl_boyfriend, state->choosing_boyfriend);
        weekselector_mdlselect_enable_arrows(ui->mdl_girlfriend, !state->choosing_boyfriend);

        weekselector_trigger_event(ui, state, "model-change-show");
        weekselector_trigger_event(
            ui, state, state->choosing_boyfriend ? "model-change-bf" : "model-change-gf"
        );
    }

    GamepadButtons buttons = gamepad_has_pressed_delayed(
        ui->maple_pads,
        GAMEPAD_T_LR | GAMEPAD_AD | GAMEPAD_B | GAMEPAD_A | GAMEPAD_START | GAMEPAD_BACK
    );

    if (buttons & (GAMEPAD_TRIGGER_LEFT | GAMEPAD_AD_LEFT)) {
        character_model_seek = -1;
    } else if (buttons & (GAMEPAD_TRIGGER_RIGHT | GAMEPAD_AD_RIGHT)) {
        character_model_seek = 1;
    } else if (buttons & GAMEPAD_AD_UP) {
        seek_offset = -1;
    } else if (buttons & GAMEPAD_AD_DOWN) {
        seek_offset = 1;
    } else if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
        bool has_locked = false;
        if (weekselector_mdlselect_is_selected_locked(ui->mdl_boyfriend)) {
            has_locked = true;
            weekselector_mdlselect_select_default(ui->mdl_boyfriend);
        }
        if (weekselector_mdlselect_is_selected_locked(ui->mdl_girlfriend)) {
            has_locked = true;
            weekselector_mdlselect_select_default(ui->mdl_girlfriend);
        }
        state->page_selected_ui = 1;
        state->play_sound = has_locked ? SND_ASTERIK : SND_CANCEL;
    } else if (buttons & GAMEPAD_A) {
        bool has_locked = weekselector_mdlselect_is_selected_locked(ui->mdl_boyfriend);
        if (weekselector_mdlselect_is_selected_locked(ui->mdl_girlfriend)) has_locked = true;

        if (has_locked) {
            state->play_sound = SND_ASTERIK;
        } else {
            state->page_selected_ui = 0;
            state->can_start = true;
        }
    } else if (buttons & GAMEPAD_START) {
        bool has_locked = weekselector_mdlselect_is_selected_locked(ui->mdl_boyfriend);
        if (weekselector_mdlselect_is_selected_locked(ui->mdl_girlfriend)) has_locked = true;

        if (has_locked)
            state->play_sound = SND_ASTERIK;
        else
            state->quit = true;
    }

    if (state->page_selected_ui != 2) weekselector_trigger_event(ui, state, "model-change-hide");

    if (character_model_seek != 0) {
        bool old_choose_boyfriend = state->choosing_boyfriend;
        state->choosing_boyfriend = character_model_seek < 1;

        if (state->choosing_boyfriend != old_choose_boyfriend) {
            if (no_custom_boyfriend || no_custom_girlfriend) {
                state->play_sound = SND_ASTERIK;
                state->choosing_boyfriend = old_choose_boyfriend;
                return;
            }

            state->play_sound = SND_SCROLL;
            weekselector_mdlselect_enable_arrows(ui->mdl_boyfriend, state->choosing_boyfriend);
            weekselector_mdlselect_enable_arrows(ui->mdl_girlfriend, !state->choosing_boyfriend);

            weekselector_trigger_event(
                ui, state, state->choosing_boyfriend ? "model-change-bf" : "model-change-gf"
            );
        }
    }

    if (seek_offset == 0) return;

    WeekSelectorMdlSelect mdlselect = state->choosing_boyfriend ? ui->mdl_boyfriend : ui->mdl_girlfriend;

    if (weekselector_mdlselect_scroll(mdlselect, seek_offset)) {
        state->play_sound = SND_SCROLL;
        state->can_start = true;
    } else {
        state->play_sound = SND_ASTERIK;
    }
}
