"use strict";

const WEEKSELECTOR_SCORE = "WEEK SCORE: ";
const WEEKSELECTOR_SONGS = "TRACKS";
const WEEKSELECTOR_SONGS_MORE = "...";
const WEEKSELECTOR_COMPLETED = "WEEKS COMPLETED";
const WEEKSELECTOR_CHOOSEN = "WEEK CHOOSEN";
const WEEKSELECTOR_ALT_WARNING = "SONGS WARNING";
const WEEKSELECTOR_SENSIBLE_CONTENT = "SENSIBLE CONTENT";
const WEEKSELECTOR_HELP_N_ALT = "USE ALTERNATIVE TRACKS";
const WEEKSELECTOR_HELP_W_ALT = "USE DEFAULT TRACKS";
const WEEKSELECTOR_HELP_GFBF_SWITCH = "SWITCH BOYFRIEND/GIRLFRIEND";
const WEEKSELECTOR_HELP_SELECT_WEEK = "SELECT ANOTHER WEEK";
const WEEKSELECTOR_HELP_BACK = "BACK";
const WEEKSELECTOR_HELP_START = "PLAY THIS WEEK";

const WEEKSELECTOR_BUTTON_DELAY = 200;
const WEEKSELECTOR_BUTTON_X = "x";
const WEEKSELECTOR_BUTTON_B = "b";
const WEEKSELECTOR_BUTTON_LT_RT = "lt_rt";
const WEEKSELECTOR_BUTTON_START = "start";

const WEEKSELECTOR_LOCKED = "locked";

const WEEKSELECTOR_BUTTONS_MODEL = "/assets/common/image/week-selector/dreamcast_buttons.xml";
const WEEKSELECTOR_UI_ICONS_MODEL = "/assets/common/image/week-selector/week_selector_UI.xml";
const WEEKSELECTOR_UI_ANIMS = "/assets/common/image/week-selector/ui_anims.xml";

const WEEKSELECTOR_SND_CANCEL = 1;
const WEEKSELECTOR_SND_SCROLL = 2;
const WEEKSELECTOR_SND_ASTERIK = 3;

const WEEKSELECTOR_ARROW_DISABLED_ALPHA = 0.1;
const WEEKSELECTOR_PREVIEW_DISABLED_ALPHA = 0.7;
const WEEKSELECTOR_ARROW_SPRITE_NAME = "arrow_L push";

const WEEKSELECTOR_BUTTONS_OK = GAMEPAD_X | GAMEPAD_A;

const WEEKSELECTOR_LAYOUT = "/assets/common/image/week-selector/layout.xml";
const WEEKSELECTOR_LAYOUT_DREAMCAST = "/assets/common/image/week-selector/layout~dreamcast.xml";
const WEEKSELECTOR_MODDING_SCRIPT = "/assets/common/data/scripts/weekselector.lua";

class UI {
    /**@type {GamepadKDY} */
    maple_pads;

    helptext_alternate;
    helptext_back;
    weekmsg_sensible;
    weekmsg_alternate;
    weekdifficult;
    weeklist;
    weektitle;
    helptext_bfgf;
    helptext_start;
    weeks_choosen_index;
    week_background_color;

    mdl_boyfriend;
    mdl_girlfriend;
    songs_list;
    score;
    description;

    layout;
    stringbuilder;
}

class STATE {
    weekinfo;

    /**@type {boolean} */
    update_ui = false;
    /**@type {boolean} */
    update_extra_ui = false;
    page_selected_ui = 0;
    play_sound = new Object;

    /**@type {boolean} */
    choosing_boyfriend = true;

    /** @type {boolean} */
    week_unlocked;

    /**@type {boolean} */
    has_choosen_alternate = false;
    difficult = "";
    /**@type {boolean} */
    can_start = false;

    /**@type {boolean} */
    quit = false;
    /**@type {boolean} */
    back_to_main_menu = false;

    default_background_color = 0x00;
    custom_layout;

    /**@type {boolean} */
    bg_music_paused;
    bg_music = {};

    modding = null;
}

function weekselector_show_week_info(/**@type {UI}*/ui, weekinfo, score) {
    let max_draw_size = [0.0, 0.0];
    let font_size = textsprite_get_font_size(ui.songs_list);
    textsprite_get_max_draw_size(ui.songs_list, max_draw_size);

    let songs_count = weekinfo.songs_count;
    let max_lines = Math.trunc(max_draw_size[1] / font_size) - 1;
    let has_overflow;
    if (songs_count > max_lines) {
        has_overflow = true;
        songs_count = max_lines;
    } else {
        has_overflow = false;
    }

    let list = ui.stringbuilder;
    stringbuilder_clear(ui.stringbuilder);

    for (let i = 0; i < songs_count; i++) {
        if (weekinfo.songs[i].freeplay_only) continue;
        stringbuilder_add(list, "\n");
        stringbuilder_add(list, weekinfo.songs[i].name);
    }
    if (has_overflow) {
        stringbuilder_add(list, "\n");
        stringbuilder_add(list, WEEKSELECTOR_SONGS_MORE);
    }

    // set the strings
    textsprite_set_text_formated(ui.score, "$s$l", WEEKSELECTOR_SCORE, score);
    textsprite_set_text_intern(ui.description, true, weekinfo.description);
    textsprite_set_text_intern(ui.songs_list, true, stringbuilder_intern(ui.stringbuilder));
}

function weekselector_change_page(/**@type {UI}*/ui, page) {
    // hide everything first
    layout_set_group_visibility(ui.layout, "ui_week_selector", page == 0);
    layout_set_group_visibility(ui.layout, "ui_difficult_selector", page == 1);
    layout_set_group_visibility(ui.layout, "ui_character_selector", page == 2);

    weekselector_helptext_set_visible(ui.helptext_alternate, false);
    weekselector_helptext_set_visible(ui.helptext_bfgf, false);
    weekselector_helptext_set_visible(ui.helptext_back, page > 0);
    weekselector_helptext_set_visible(ui.helptext_start, false);
}

async function weekselector_trigger_difficult_change(/**@type {UI}*/ui,/**@type {STATE}*/state, empty) {
    await weekselector_trigger_event(ui, state, "change-difficult");

    if (empty) return;

    let default_difficult = weekselector_difficult_get_selected(ui.weekdifficult);
    if (default_difficult) {
        let difficult_action = string_concat(2, "selected-difficult-", default_difficult);
        await weekselector_trigger_event(ui, state, difficult_action);
        difficult_action = undefined;
    }
}

async function weekselector_trigger_alternate_change(ui, /**@type {STATE}*/state) {
    let what = state.has_choosen_alternate ? "selected-no-alternate" : "selected-alternate";
    await weekselector_trigger_event(ui, state, what);
}

async function weekselector_load_custom_week_background(/**@type {STATE}*/state) {
    if (state.custom_layout) {
        layout_destroy(state.custom_layout);
        state.custom_layout = null;
    }

    if (!state.weekinfo.custom_selector_layout) {
        if (state.bg_music_paused && state.bg_music) soundplayer_play(state.bg_music);
        return;
    }

    let has_custom_bg_music = false;

    state.custom_layout = await layout_init(state.weekinfo.custom_selector_layout);
    if (state.custom_layout) {
        layout_trigger_any(state.custom_layout, "show-principal");
        if (!state.week_unlocked) layout_trigger_any(state.custom_layout, "week-locked");

        has_custom_bg_music = layout_get_attached_value(
            state.custom_layout, "has_background_music", LAYOUT_TYPE_BOOLEAN, has_custom_bg_music
        );
    }

    if (state.bg_music) {
        if (!state.bg_music_paused && has_custom_bg_music)
            soundplayer_play(state.bg_music);
        else if (state.bg_music_paused && !has_custom_bg_music)
            soundplayer_pause(state.bg_music);

        state.bg_music_paused = has_custom_bg_music;
    }
}

function weekselector_set_text(layout, name, format, text_or_integer) {
    let textsprite = layout_get_textsprite(layout, name);
    if (!textsprite) return;
    if (format)
        textsprite_set_text_formated(textsprite, format, text_or_integer);
    else
        textsprite_set_text_intern(textsprite, true, text_or_integer);
}

function weekselector_trigger_menu_action(ui, state, name, selected, choosen) {
    if (!state.weekinfo) return;
    main_helper_trigger_action_menu(
        ui.layout,
        state.weekinfo.display_name ?? state.weekinfo.name,
        name, selected, choosen
    );
}

async function weekselector_trigger_event(ui, state, name) {
    if (state.custom_layout) layout_trigger_any(state.custom_layout, name);
    layout_trigger_any(ui.layout, name);
    await modding_helper_notify_event(state.modding, name);
}


async function weekselector_main() {
    let layout = await layout_init(pvr_context_is_widescreen() ? WEEKSELECTOR_LAYOUT : WEEKSELECTOR_LAYOUT_DREAMCAST);
    let animlist_ui = await animlist_init(WEEKSELECTOR_UI_ANIMS);
    let modelholder_ui = await modelholder_init(WEEKSELECTOR_UI_ICONS_MODEL);
    let modelholder_buttons_ui = await modelholder_init(WEEKSELECTOR_BUTTONS_MODEL);

    if (!modelholder_ui) throw new Error("Failed to load: " + WEEKSELECTOR_UI_ICONS_MODEL);
    if (!modelholder_buttons_ui) throw new Error("Failed to load: " + WEEKSELECTOR_BUTTONS_MODEL);

    let texpool = texturepool_init(8 * 1024 * 1024);// 8MiB

    let sound_confirm = await soundplayer_init("/assets/common/sound/confirmMenu.ogg");
    let sound_scroll = await soundplayer_init("/assets/common/sound/scrollMenu.ogg");
    let sound_cancel = await soundplayer_init("/assets/common/sound/cancelMenu.ogg");
    let sound_asterik = await soundplayer_init("/assets/common/sound/asterikMenu.ogg");

    beatwatcher_global_set_timestamp_from_kos_timer();

    // store all ui elements here (makes the code more readable)
    /** @type {UI} */
    const ui = {
        week_background_color: layout_get_sprite(layout, "week_background_color"),

        // week score&description
        score: layout_get_textsprite(layout, "week_score"),
        description: layout_get_textsprite(layout, "week_description"),

        // week selector ui elements
        songs_list: layout_get_textsprite(layout, "ui_songs_list"),
        weeks_choosen_index: layout_get_textsprite(layout, "ui_weeks_choosen_index"),
        layout: layout,

        // selected week warning messages
        weekmsg_sensible: weekselector_weekmsg_init(
            layout, WEEKSELECTOR_SENSIBLE_CONTENT,
            "warning_sensible_title", "warning_sensible_message"
        ),
        weekmsg_alternate: weekselector_weekmsg_init(
            layout, WEEKSELECTOR_ALT_WARNING,
            "warning_alternate_title", "warning_alternate_message"
        ),

        weeklist: await weekselector_weeklist_init(animlist_ui, modelholder_ui, layout, texpool),
        weektitle: weekselector_weektitle_init(layout),

        mdl_boyfriend: await weekselector_mdlselect_init(
            animlist_ui, modelholder_ui, layout, texpool, true
        ),
        mdl_girlfriend: await weekselector_mdlselect_init(
            animlist_ui, modelholder_ui, layout, texpool, false
        ),

        weekdifficult: await weekselector_difficult_init(animlist_ui, modelholder_ui, layout),

        helptext_alternate: weekselector_helptext_init(
            modelholder_buttons_ui, layout, 3, false, WEEKSELECTOR_BUTTON_X,
            WEEKSELECTOR_HELP_N_ALT, WEEKSELECTOR_HELP_W_ALT
        ),
        helptext_bfgf: weekselector_helptext_init(
            modelholder_buttons_ui, layout, 2, false, WEEKSELECTOR_BUTTON_LT_RT,
            WEEKSELECTOR_HELP_GFBF_SWITCH, null
        ),
        helptext_back: weekselector_helptext_init(
            modelholder_buttons_ui, layout, 1, false, WEEKSELECTOR_BUTTON_B,
            WEEKSELECTOR_HELP_BACK, WEEKSELECTOR_HELP_SELECT_WEEK
        ),
        helptext_start: weekselector_helptext_init(
            modelholder_buttons_ui, layout, 1, true, WEEKSELECTOR_BUTTON_START,
            WEEKSELECTOR_HELP_START, null
        ),

        maple_pads: gamepad_init(-1),
        stringbuilder: stringbuilder_init(64)
    };

    // dispose unused resources
    modelholder_destroy(modelholder_ui);
    modelholder_destroy(modelholder_buttons_ui);
    animlist_destroy(animlist_ui);

    // funkin stuff
    let weeks_completed = funkinsave_get_completed_count();
    let last_difficult_played = funkinsave_get_last_played_difficult();

    // setup text boxes
    weekselector_set_text(layout, "ui_songs_title", null, WEEKSELECTOR_SONGS);
    weekselector_set_text(layout, "ui_weeks_completed_title", null, WEEKSELECTOR_COMPLETED);
    weekselector_set_text(layout, "ui_weeks_completed_count", "$i", weeks_completed);
    weekselector_set_text(layout, "ui_weeks_choosen_title", null, WEEKSELECTOR_CHOOSEN);

    // attach help texts to the layout
    let ui_group = layout_external_create_group(layout, null, 0);
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

    let modding = await modding_init(layout, WEEKSELECTOR_MODDING_SCRIPT);
    modding.native_menu = null;
    modding.callback_option = null;
    await modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN);

    /** @type {STATE} */
    const state = {
        weekinfo: weekselector_weeklist_get_selected(ui.weeklist),
        difficult: last_difficult_played,

        week_unlocked: false,
        play_sound: 0,

        default_background_color: 0x00,

        can_start: false,
        choosing_boyfriend: true,
        has_choosen_alternate: false,

        update_extra_ui: true,
        page_selected_ui: 0,
        update_ui: true,
        quit: false,
        back_to_main_menu: false,

        custom_layout: null,
        bg_music_paused: false,
        bg_music: layout_get_soundplayer(layout, "background_music") ?? background_menu_music,
        modding: modding
    };

    if (state.bg_music != background_menu_music && background_menu_music) {
        soundplayer_pause(background_menu_music);
    }

    if (ui.week_background_color)
        state.default_background_color = sprite_get_vertex_color_rgb8(ui.week_background_color);

    weekselector_change_page(ui, state.page_selected_ui);
    state.week_unlocked = funkinsave_contains_unlock_directive(state.weekinfo.unlock_directive);
    await weekselector_load_custom_week_background(state);
    weekselector_trigger_menu_action(ui, state, "weeklist", true, false);

    while (!state.quit) {
        let elapsed = await pvrctx_wait_ready();
        let page_selected_ui = state.page_selected_ui;

        beatwatcher_global_set_timestamp_from_kos_timer();

        let res = await modding_helper_handle_custom_menu(state.modding, ui.maple_pads, elapsed);

        if (state.modding.has_exit || res == MODDING_HELPER_RESULT_BACK) {
            state.back_to_main_menu = true;
            state.quit = true;
            break;
        }

        if (!state.modding.has_halt) {
            switch (page_selected_ui) {
                case 0:
                    await weekselector_page0(ui, state);
                    break;
                case 1:
                    await weekselector_page1(ui, state);
                    break;
                case 2:
                    await weekselector_page2(ui, state);
                    break;
            }
        }

        if (state.play_sound) {
            if (sound_cancel) soundplayer_stop(sound_cancel);
            if (sound_scroll) soundplayer_stop(sound_scroll);
            if (sound_asterik) soundplayer_stop(sound_asterik);

            let sound = null;
            switch (state.play_sound) {
                case WEEKSELECTOR_SND_CANCEL:
                    sound = sound_cancel;
                    break;
                case WEEKSELECTOR_SND_SCROLL:
                    sound = sound_scroll;
                    break;
                case WEEKSELECTOR_SND_ASTERIK:
                    sound = sound_asterik;
                    break;
            }

            state.play_sound = 0;
            if (sound) soundplayer_play(sound);
        }

        pvr_context_reset(pvr_context);
        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        if (state.custom_layout) {
            layout_animate(state.custom_layout, elapsed);
            layout_draw(state.custom_layout, pvr_context);
        }

        state.update_ui = page_selected_ui != state.page_selected_ui;
        if (state.update_ui) weekselector_change_page(ui, state.page_selected_ui);
    }

    let outro_layout = layout;
    if (state.back_to_main_menu) {
        if (sound_cancel) soundplayer_replay(sound_cancel);
        await weekselector_trigger_event(ui, state, "back-to-main-menu");
    } else {
        if (state.bg_music) soundplayer_stop(state.bg_music);
        weekselector_change_page(ui, false);
        if (sound_confirm) soundplayer_replay(sound_confirm);

        if (state.custom_layout && layout_trigger_any(state.custom_layout, "week-choosen")) {
            outro_layout = state.custom_layout;
        } else {
            // if the custom layout does not contains the "week-choosen" action
            // trigger in the main layout
            layout_trigger_any(layout, "week-choosen");
        }

        await modding_helper_notify_event(state.modding, "week-choosen");
        layout_set_group_visibility(layout, "ui_game_progress", false);
        weekselector_weektitle_move_difficult(ui.weektitle, ui.weekdifficult);
        weekselector_mdlselect_toggle_choosen(ui.mdl_boyfriend);
        weekselector_mdlselect_toggle_choosen(ui.mdl_girlfriend);
        weekselector_weeklist_toggle_choosen(ui.weeklist);
    }

    while (true) {
        let elapsed = await pvrctx_wait_ready();

        beatwatcher_global_set_timestamp_from_kos_timer();
        await modding_helper_notify_frame(modding, elapsed, -1.0);

        pvr_context_reset(pvr_context);
        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        if (state.custom_layout) {
            layout_animate(state.custom_layout, elapsed);
            layout_draw(state.custom_layout, pvr_context);
        }

        if (layout_animation_is_completed(outro_layout, "transition_effect")) {
            // flush framebuffer again with last fade frame
            await pvrctx_wait_ready();
            break;
        }
    }

    // gameplay parameters
    let gameplay_weekinfo = state.weekinfo;
    let gameplay_difficult = weekselector_difficult_get_selected(ui.weekdifficult);
    let gameplay_alternative_tracks = state.has_choosen_alternate;
    let gameplay_model_boyfriend = state.back_to_main_menu ? null : weekselector_mdlselect_get_manifest(ui.mdl_boyfriend);
    let gameplay_model_girlfriend = state.back_to_main_menu ? null : weekselector_mdlselect_get_manifest(ui.mdl_girlfriend);

    await modding_helper_notify_exit2(modding);

    // dispose everything used
    weekselector_weeklist_destroy(ui.weeklist);
    weekselector_difficult_destroy(ui.weekdifficult);
    weekselector_weektitle_destroy(ui.weektitle);

    weekselector_mdlselect_destroy(ui.mdl_boyfriend);
    weekselector_mdlselect_destroy(ui.mdl_girlfriend);

    gamepad_destroy(ui.maple_pads);

    weekselector_helptext_destroy(ui.helptext_alternate);
    weekselector_helptext_destroy(ui.helptext_bfgf);
    weekselector_helptext_destroy(ui.helptext_back);
    weekselector_helptext_destroy(ui.helptext_start);

    weekselector_weekmsg_destroy(ui.weekmsg_sensible);
    weekselector_weekmsg_destroy(ui.weekmsg_alternate);

    if (sound_confirm) soundplayer_destroy(sound_confirm);
    if (sound_asterik) soundplayer_destroy(sound_asterik);
    if (sound_scroll) soundplayer_destroy(sound_scroll);
    if (sound_cancel) soundplayer_destroy(sound_cancel);

    stringbuilder_destroy(ui.stringbuilder);

    if (state.custom_layout) layout_destroy(state.custom_layout);

    layout_destroy(layout);
    texturepool_destroy(texpool);
    modding_destroy(modding);

    if (state.back_to_main_menu) {
        await savemanager_check_and_save_changes();
        return 0;
    }

    if (background_menu_music) {
        soundplayer_destroy(background_menu_music);
        background_menu_music = null;
    }

    funkinsave_set_last_played(gameplay_weekinfo.name, gameplay_difficult);

    await main_helper_draw_loading_screen();

    let week_result = await week_main(
        gameplay_weekinfo,
        gameplay_alternative_tracks,
        gameplay_difficult,
        gameplay_model_boyfriend,
        gameplay_model_girlfriend,
        null,
        -1,
        null
    );

    background_menu_music = await soundplayer_init(FUNKIN_BACKGROUND_MUSIC);
    if (background_menu_music) {
        soundplayer_loop_enable(background_menu_music, true);
        soundplayer_replay(background_menu_music);
    }

    return week_result;
}


async function weekselector_page0(/**@type {UI}*/ui, /**@type {STATE}*/state) {
    let seek_offset = 0;

    if (state.update_ui) weekselector_helptext_set_visible(ui.helptext_start, state.can_start);

    if (state.update_extra_ui) {
        let score = funkinsave_get_week_score(state.weekinfo.name, state.difficult);
        let index = weekselector_weeklist_get_selected_index(ui.weeklist) + 1;
        weekselector_show_week_info(ui, state.weekinfo, score);
        textsprite_set_text_formated(ui.weeks_choosen_index, "$i/$i", index, weeks_array.size);

        if (ui.week_background_color) {
            let background_color;
            if (state.weekinfo.selector_background_color_override)
                background_color = state.weekinfo.selector_background_color;
            else
                background_color = state.default_background_color;
            sprite_set_vertex_color_rgb8(ui.week_background_color, background_color);
        }

        state.update_extra_ui = false;
    }

    let buttons = gamepad_has_pressed_delayed(
        ui.maple_pads,
        GAMEPAD_AD_UP | GAMEPAD_AD_DOWN | GAMEPAD_X | GAMEPAD_A | GAMEPAD_START | GAMEPAD_B | GAMEPAD_BACK
    );

    if (buttons & GAMEPAD_AD_UP) {
        seek_offset = -1;
    } else if (buttons & GAMEPAD_AD_DOWN) {
        seek_offset = 1;
    } else if (buttons & WEEKSELECTOR_BUTTONS_OK) {
        if (state.week_unlocked) {
            state.play_sound = WEEKSELECTOR_SND_SCROLL;
            state.page_selected_ui = 1;
        } else {
            state.play_sound = WEEKSELECTOR_SND_ASTERIK;
        }
    } else if (buttons & GAMEPAD_START) {
        state.quit = state.can_start;
        if (!state.quit && state.week_unlocked) {
            state.play_sound = WEEKSELECTOR_SND_SCROLL;
            state.page_selected_ui = 1;
        }
    } else if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
        state.quit = true;
        state.back_to_main_menu = true;
        return;
    }

    if (state.page_selected_ui == 1) {
        await weekselector_trigger_event(ui, state, "principal-hide");
        await weekselector_trigger_event(ui, state, "difficult-selector-show");
    }

    if (seek_offset == 0) return;

    if (!await weekselector_weeklist_scroll(ui.weeklist, seek_offset)) {
        state.play_sound = WEEKSELECTOR_SND_ASTERIK;
        return;
    }

    weekselector_trigger_menu_action(ui, state, "weeklist", false, false);

    state.weekinfo = weekselector_weeklist_get_selected(ui.weeklist);
    state.week_unlocked = funkinsave_contains_unlock_directive(state.weekinfo.unlock_directive);
    state.play_sound = WEEKSELECTOR_SND_SCROLL;
    state.can_start = false;
    state.update_extra_ui = true;
    state.has_choosen_alternate = false;

    weekselector_helptext_set_visible(ui.helptext_start, false);
    weekselector_mdlselect_select_default(ui.mdl_boyfriend);
    weekselector_mdlselect_select_default(ui.mdl_girlfriend);

    await weekselector_load_custom_week_background(state);
    weekselector_trigger_menu_action(ui, state, "weeklist", true, false);
}

async function weekselector_page1(/**@type {UI}*/ui, /**@type {STATE}*/state) {
    let seek_offset = 0;

    if (state.update_ui) {
        let weekinfo = state.weekinfo;
        let has_alternate = weekinfo.warning_message != null;
        let has_warnings = has_alternate && !!weekinfo.sensible_content_message;

        weekselector_helptext_set_visible(
            ui.helptext_bfgf,
            !(weekinfo.disallow_custom_boyfriend && weekinfo.disallow_custom_girlfriend)
        );

        await weekselector_difficult_load(ui.weekdifficult, weekinfo, state.difficult);
        weekselector_weektitle_move(ui.weektitle, ui.weeklist, has_warnings);

        weekselector_helptext_set_visible(ui.helptext_alternate, has_alternate);
        weekselector_weekmsg_set_message(ui.weekmsg_sensible, weekinfo.sensible_content_message);
        weekselector_weekmsg_set_message(ui.weekmsg_alternate, weekinfo.warning_message);
        weekselector_weekmsg_disabled(ui.weekmsg_alternate, state.has_choosen_alternate);
        weekselector_difficult_relayout(ui.weekdifficult, has_warnings);

        weekselector_helptext_set_visible(ui.helptext_start, !has_warnings);
        await weekselector_trigger_difficult_change(ui, state, false);
        await weekselector_trigger_alternate_change(ui, state);
    }

    let buttons = gamepad_has_pressed_delayed(
        ui.maple_pads,
        GAMEPAD_AD_LEFT | GAMEPAD_AD_RIGHT | GAMEPAD_X | GAMEPAD_B |
        GAMEPAD_T_LR | GAMEPAD_A | GAMEPAD_START | GAMEPAD_BACK
    );

    if (buttons & GAMEPAD_AD_LEFT) {
        seek_offset = -1;
    } else if (buttons & GAMEPAD_AD_RIGHT) {
        seek_offset = 1;
    } else if (buttons & GAMEPAD_X) {
        if (state.weekinfo.warning_message) {
            state.has_choosen_alternate = !state.has_choosen_alternate;
            weekselector_helptext_use_alt(ui.helptext_alternate, state.has_choosen_alternate);
            weekselector_weekmsg_disabled(ui.weekmsg_alternate, state.has_choosen_alternate);
            state.play_sound = WEEKSELECTOR_SND_SCROLL;
            await weekselector_trigger_alternate_change(ui, state);
        }
    } else if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
        state.page_selected_ui = 0;
        state.has_choosen_alternate = false;
        state.play_sound = WEEKSELECTOR_SND_CANCEL;
    } else if (buttons & GAMEPAD_T_LR) {
        if (state.weekinfo.disallow_custom_boyfriend && state.weekinfo.disallow_custom_girlfriend) {
            state.play_sound = WEEKSELECTOR_SND_ASTERIK;
        } else {
            state.choosing_boyfriend = (buttons & GAMEPAD_TRIGGER_LEFT) == GAMEPAD_TRIGGER_LEFT;
            state.page_selected_ui = 2;
            state.play_sound = WEEKSELECTOR_SND_SCROLL;
        }
    } else if (buttons & GAMEPAD_A) {
        if (weekselector_difficult_selected_is_locked(ui.weekdifficult)) {
            state.play_sound = WEEKSELECTOR_SND_ASTERIK;
        } else {
            state.page_selected_ui = 0;
            state.can_start = true;
        }
    } else if (buttons & GAMEPAD_START) {
        if (weekselector_difficult_selected_is_locked(ui.weekdifficult))
            state.play_sound = WEEKSELECTOR_SND_ASTERIK;
        else
            state.quit = true;
    }

    if (seek_offset == 0) return;

    if (state.page_selected_ui == 0) {
        await weekselector_trigger_event(ui, state, "difficult-selector-hide");
        await weekselector_trigger_event(ui, state, "principal-show");
    }

    if (weekselector_difficult_scroll(ui.weekdifficult, seek_offset)) {
        state.play_sound = WEEKSELECTOR_SND_SCROLL;
        state.can_start = true;
        state.difficult = weekselector_difficult_get_selected(ui.weekdifficult);

        await weekselector_trigger_difficult_change(ui, state, false);
        weekselector_show_week_info(
            ui, state.weekinfo, funkinsave_get_week_score(state.weekinfo.name, state.difficult)
        );
    } else {
        state.play_sound = WEEKSELECTOR_SND_ASTERIK;
    }
}

async function weekselector_page2(/**@type {UI}*/ui, /**@type {STATE}*/state) {
    let seek_offset = 0;
    let character_model_seek = 0;
    let no_custom_boyfriend = state.weekinfo.disallow_custom_boyfriend;
    let no_custom_girlfriend = state.weekinfo.disallow_custom_girlfriend;

    if (state.update_ui) {
        if (state.choosing_boyfriend && no_custom_boyfriend) state.choosing_boyfriend = false;
        if (!state.choosing_boyfriend && no_custom_girlfriend) state.choosing_boyfriend = true;
        weekselector_mdlselect_select_default(ui.mdl_boyfriend);
        weekselector_mdlselect_select_default(ui.mdl_girlfriend);
        weekselector_mdlselect_enable_arrows(ui.mdl_boyfriend, state.choosing_boyfriend);
        weekselector_mdlselect_enable_arrows(ui.mdl_girlfriend, !state.choosing_boyfriend);

        await weekselector_trigger_event(ui, state, "model-change-show");
        await weekselector_trigger_event(
            ui, state, state.choosing_boyfriend ? "model-change-bf" : "model-change-gf"
        );
    }

    let buttons = gamepad_has_pressed_delayed(
        ui.maple_pads,
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
        let has_locked = false;
        if (weekselector_mdlselect_is_selected_locked(ui.mdl_boyfriend)) {
            has_locked = true;
            weekselector_mdlselect_select_default(ui.mdl_boyfriend);
        }
        if (weekselector_mdlselect_is_selected_locked(ui.mdl_girlfriend)) {
            has_locked = true;
            weekselector_mdlselect_select_default(ui.mdl_girlfriend);
        }
        state.page_selected_ui = 1;
        state.play_sound = has_locked ? WEEKSELECTOR_SND_ASTERIK : WEEKSELECTOR_SND_CANCEL;
    } else if (buttons & GAMEPAD_A) {
        let has_locked = weekselector_mdlselect_is_selected_locked(ui.mdl_boyfriend);
        if (weekselector_mdlselect_is_selected_locked(ui.mdl_girlfriend)) has_locked = true;

        if (has_locked) {
            state.play_sound = WEEKSELECTOR_SND_ASTERIK;
        } else {
            state.page_selected_ui = 0;
            state.can_start = true;
        }
    } else if (buttons & GAMEPAD_START) {
        let has_locked = weekselector_mdlselect_is_selected_locked(ui.mdl_boyfriend);
        if (weekselector_mdlselect_is_selected_locked(ui.mdl_girlfriend)) has_locked = true;

        if (has_locked)
            state.play_sound = WEEKSELECTOR_SND_ASTERIK;
        else
            state.quit = true;
    }

    if (state.page_selected_ui != 2) await weekselector_trigger_event(ui, state, "model-change-hide");

    if (character_model_seek != 0) {
        let old_choose_boyfriend = state.choosing_boyfriend;
        state.choosing_boyfriend = character_model_seek < 1;

        if (state.choosing_boyfriend != old_choose_boyfriend) {
            if (no_custom_boyfriend || no_custom_girlfriend) {
                state.play_sound = WEEKSELECTOR_SND_ASTERIK;
                state.choosing_boyfriend = old_choose_boyfriend;
                return;
            }

            state.play_sound = WEEKSELECTOR_SND_SCROLL;
            weekselector_mdlselect_enable_arrows(ui.mdl_boyfriend, state.choosing_boyfriend);
            weekselector_mdlselect_enable_arrows(ui.mdl_girlfriend, !state.choosing_boyfriend);

            await weekselector_trigger_event(
                ui, state, state.choosing_boyfriend ? "model-change-bf" : "model-change-gf"
            );
        }
    }

    if (seek_offset == 0) return;

    let mdlselect = state.choosing_boyfriend ? ui.mdl_boyfriend : ui.mdl_girlfriend;

    if (weekselector_mdlselect_scroll(mdlselect, seek_offset)) {
        state.play_sound = WEEKSELECTOR_SND_SCROLL;
        state.can_start = true;
    } else {
        state.play_sound = WEEKSELECTOR_SND_ASTERIK;
    }

}

