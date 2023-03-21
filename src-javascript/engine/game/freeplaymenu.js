"use strict";

const FREEPLAYMENU_BACKGROUND_ANIM_OR_ATLAS_ENTRY_NAME = "freeplay-background";
const FREEPLAYMENU_PERSONAL_BEST = "PERSONAL BEST SCORE: $l";
const FREEPLAYMENU_INFO = "$s  -  $s  -  $s";
const FREEPLAYMENU_BG_INFO_NAME = "background_song_info";
const FREEPLAYMENU_LAYOUT = "/assets/common/image/freeplay-menu/layout.xml";
const FREEPLAYMENU_LAYOUT_DREAMCAST = "/assets/common/image/freeplay-menu/layout~dreamcast.xml";
const FREEPLAYMENU_MODDING_SCRIPT = "/assets/common/data/scripts/freeplaymenu.lua";

const FREEPLAYMENU_MENU_SONGS = {
    parameters: {
        suffix_selected: null,// unused
        suffix_idle: null,// unused
        suffix_choosen: null,// unused
        suffix_discarded: null,// unused
        suffix_rollback: null,// unused
        suffix_in: null,// unused
        suffix_out: null,// unused

        atlas: null,
        animlist: "/assets/common/anims/freeplay-menu.xml",

        anim_selected: "menu_item_selected",
        anim_idle: "menu_item_idle",
        anim_choosen: null,// unused
        anim_discarded: null,// unused
        anim_rollback: null,// unused
        anim_in: null,// unused
        anim_out: null,// unused

        anim_transition_in_delay: 0,// unused
        anim_transition_out_delay: 0,// unused

        font: null,
        font_glyph_suffix: null,// unused
        font_color_by_difference: 0,// unused
        font_size: 28.0,
        font_color: 0xFFFFFF,
        font_border_color: 0x000000FF,// unused
        font_border_size: 4,// unused

        is_sparse: 0,// unused
        is_vertical: 1,
        is_per_page: 0,
        static_index: 1,

        items_align: ALIGN_START,
        items_gap: 58.0,
        items_dimmen: 0.0,// unused
        texture_scale: NaN,// unused
        enable_horizontal_text_correction: 1// unused
    },
    items: null,
    items_size: 0
};

async function freeplaymenu_main() {
    let src_layout = pvrctx_is_widescreen() ? FREEPLAYMENU_LAYOUT : FREEPLAYMENU_LAYOUT_DREAMCAST;
    let layout = await layout_init(src_layout);
    if (!layout) {
        console.error("freeplay_main() missing layout");
        return;
    }

    let placeholder = layout_get_placeholder(layout, "menu");
    let params = FREEPLAYMENU_MENU_SONGS.parameters;


    params.font = layout_get_attached_value(
        layout, "menu_font", LAYOUT_TYPE_STRING, "/assets/common/font/Alphabet.xml"
    );
    params.font_glyph_suffix = layout_get_attached_value(
        layout, "menu_fontGlyphSuffix", LAYOUT_TYPE_STRING, "bold"
    );
    params.font_size = params.items_dimmen = layout_get_attached_value_as_float(
        layout, "menu_fontSize", 46.0
    );
    params.font_color = layout_get_attached_value(
        layout, "menu_fontColor", LAYOUT_TYPE_HEX, 0xFFFFFF
    );
    params.font_border_color = layout_get_attached_value(
        layout, "menu_fontBorderColor", LAYOUT_TYPE_HEX, 0x000000
    );
    params.font_border_size = layout_get_attached_value_as_float(
        layout, "menu_fontBorderSize", 0.0
    );
    params.items_gap = layout_get_attached_value_as_float(
        layout, "menu_itemsGap", 58.0
    );
    let icons_dimmen = layout_get_attached_value_as_float(
        layout, "menu_iconsDimmen", 70.0
    );

    let dt_playsong = layout_get_attached_value_as_float(layout, "durationTransition_playSong", 0.0);
    let dt_screenout = layout_get_attached_value_as_float(layout, "durationTransition_screenOut", 0.0);

    let bg_info_width = -1.0;
    let bg_info = layout_get_sprite(layout, FREEPLAYMENU_BG_INFO_NAME);
    if (bg_info != null) {
        const draw_size = [0, 0];
        sprite_get_draw_size(bg_info, draw_size);
        bg_info_width = draw_size[0];
    }

    // step 3: count required songs
    let songs = arraylist_init2(weeks_array.size * 3);
    for (let i = 0; i < weeks_array.size; i++) {
        const weekinfo = weeks_array.array[i];
        let is_week_locked = !funkinsave_contains_unlock_directive(weekinfo.unlock_directive);

        for (let j = 0; j < weekinfo.songs_count; j++) {
            const song = weekinfo.songs[j];

            let should_hide = song.freeplay_hide_if_week_locked;
            if (should_hide && is_week_locked) continue;

            should_hide = song.freeplay_hide_if_locked;
            let is_song_locked = !funkinsave_contains_unlock_directive(song.freeplay_unlock_directive);
            if (should_hide && is_song_locked) continue;

            let gameplaymanifest_index = song.freeplay_song_index_in_gameplaymanifest;
            if (gameplaymanifest_index < 0) gameplaymanifest_index = j;

            arraylist_add(songs, {
                song_index: j,
                week_index: i,
                gameplaymanifest_index: gameplaymanifest_index,
                is_locked: is_week_locked || is_song_locked
            });
        }
    }

    // step 4: prepare menu
    let songs_size = arraylist_size(songs);
    FREEPLAYMENU_MENU_SONGS.items_size = songs_size;
    FREEPLAYMENU_MENU_SONGS.items = new Array(songs_size);

    for (let i = 0; i < songs_size; i++) {
        let song = arraylist_get(songs, i);
        let weekinfo = weeks_array.array[song.week_index];

        FREEPLAYMENU_MENU_SONGS.items[i] = {
            name: null,// unused
            text: weekinfo.songs[song.song_index].name,
            placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: 0// unused
        };
    }

    // step 5: build menu
    let menu_songs = await menu_init(
        FREEPLAYMENU_MENU_SONGS,
        placeholder.x,
        placeholder.y,
        placeholder.z,
        placeholder.width,
        placeholder.height
    );
    menu_set_text_force_case(menu_songs, TEXTSPRITE_FORCE_UPPERCASE);
    placeholder.vertex = menu_get_drawable(menu_songs);
    menu_select_index(menu_songs, -1);

    // step 6: drop menu manifests
    //free(FREEPLAYMENU_MENU_SONGS.items);
    FREEPLAYMENU_MENU_SONGS.items = null;

    // step 7: create menu icons
    let songicons = await freeplaymenu_songicons_init(songs, icons_dimmen, params.font_size);
    menu_set_draw_callback(menu_songs, 0, freeplaymenu_songicons_draw_item_icon, songicons);

    // step 8: initialize modding
    let modding = await modding_init(layout, FREEPLAYMENU_MODDING_SCRIPT);
    modding.native_menu = modding.active_menu = menu_songs;
    modding.callback_private_data = null;
    modding.callback_option = null;
    await modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN);


    const state = {
        difficult_index: -1,
        difficult_locked: 0,
        use_alternative: 0,
        map: arraylist_get(songs, 0),
        async_id_operation: 0,
        soundplayer_path: null,
        soundplayer: null,
        background: layout_get_sprite(layout, "custom_background"),
        layout: layout,
        personal_best: layout_get_textsprite(layout, "personal_best"),
        info: layout_get_textsprite(layout, "info"),
        description: layout_get_textsprite(layout, "description"),
        difficulties: null,
        difficulties_size: 0,
        running_threads: 0,
        mutex: {},
        bg_info_width: bg_info_width,
        modding: modding,
        song_preview_volume: layout_get_attached_value_as_float(layout, "song_preview_volume", 0.7)
    };
    mutex_init(state.mutex, MUTEX_TYPE_NORMAL);
    if (state.background) sprite_set_texture(state.background, null, 0);

    let default_bf = await freeplaymenu_helper_get_default_character_manifest(1);
    let default_gf = await freeplaymenu_helper_get_default_character_manifest(0);

    if (background_menu_music) {
        soundplayer_pause(background_menu_music);
    }

    layout_trigger_any(layout, "transition-in");
    await modding_helper_notify_event(modding, "transition-in");

    while (1) {
        modding.has_exit = 0;
        modding.has_halt = 0;

        let map_index = await freeplaymenu_show(menu_songs, state, songs);
        if (map_index < 0) break;// back to main menu

        let difficult = state.difficulties[state.difficult_index].name;
        let weekinfo = weeks_array.array[arraylist_get(songs, map_index).week_index];
        let gameplaymanifest = weekinfo.songs[state.map.song_index].freeplay_gameplaymanifest;
        freeplaymenu_internal_drop_soundplayer(state);

        await freeplaymenu_internal_wait_transition(state, "before-play-song", dt_playsong);

        layout_suspend(layout);
        let ret = await week_main(
            weekinfo, state.use_alternative, difficult, default_bf, default_gf,
            gameplaymanifest, state.map.gameplaymanifest_index
        );
        if (ret == 0) break;// back to main menu

        layout_resume(layout);
        await freeplaymenu_internal_wait_transition(state, "after-play-song", dt_playsong);
        freeplaymenu_internal_song_load(state, 0);
        freeplaymenu_internal_show_info(state);
    }

    // Wait for running threads
    while (1) {
        await pvrctx_wait_ready();
        mutex_lock(state.mutex);
        let exit = state.running_threads < 1;
        mutex_unlock(state.mutex);
        if (exit) break;
    }
    thd_pass();

    await freeplaymenu_internal_wait_transition(state, "transition-out", dt_screenout);
    freeplaymenu_internal_drop_custom_background(state);

    await modding_helper_notify_exit2(modding);

    default_bf = undefined;
    default_gf = undefined;
    state.difficulties = undefined;
    mutex_destroy(state.mutex);
    freeplaymenu_internal_drop_soundplayer(state);
    freeplaymenu_songicons_destroy(songicons);
    menu_destroy(menu_songs);
    arraylist_destroy(songs, 0);
    layout_destroy(layout);
    await modding_destroy(modding);

    if (background_menu_music) {
        soundplayer_play(background_menu_music);
    }

}

async function freeplaymenu_show(menu, state, songs) {
    let map_index = -1;
    let gamepad = gamepad_init(-1);
    gamepad_set_buttons_delay(gamepad, 150);

    let sound_asterik = await soundplayer_init("/assets/common/sound/asterikMenu.ogg");

    if (menu_get_selected_index(menu) < 0 && menu_get_items_count(menu) > 0) {
        menu_select_index(menu, 0);

        state.map = arraylist_get(songs, menu_get_selected_index(menu));
        state.use_alternative = 0;
        freeplaymenu_internal_build_difficulties(state);
        freeplaymenu_internal_show_info(state);
        freeplaymenu_internal_song_load(state, 1);

        await freeplaymenu_internal_trigger_action_menu(state, 1, 0);
        await freeplaymenu_internal_modding_notify_event(state, 1, 1);
    }


    while (!state.modding.has_exit) {
        let elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);

        if (state.running_threads < 1 && state.modding.script != null && state.soundplayer) {
            await weekscript_notify_timersong(state.modding.script, soundplayer_get_position(state.soundplayer));
        }
        if (await modding_helper_handle_custom_menu(state.modding, gamepad, elapsed) != MODDING_HELPER_RESULT_CONTINUE) {
            break;
        }

        layout_animate(state.layout, elapsed);
        layout_draw(state.layout, pvr_context);

        if (state.modding.has_halt) continue;

        let btns = gamepad_has_pressed_delayed(gamepad,
            GAMEPAD_DPAD_UP | GAMEPAD_DPAD_DOWN |
            GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD_RIGHT |
            GAMEPAD_X | GAMEPAD_A | GAMEPAD_B |
            GAMEPAD_BACK | GAMEPAD_START
        );

        let offset;
        let switch_difficult;

        if ((btns & (GAMEPAD_B | GAMEPAD_BACK)) && !await modding_helper_notify_back(state.modding)) {
            break;
        } else if (btns & GAMEPAD_X) {
            let weeinfo = weeks_array.array[state.map.week_index];
            if (!weeinfo.warning_message) {
                state.use_alternative = !state.use_alternative;
                await freeplaymenu_internal_modding_notify_event(state, 0, 1);
            } else if (sound_asterik) {
                soundplayer_replay(sound_asterik);
            }
            continue;
        } else if (btns & GAMEPAD_DPAD_UP) {
            offset = -1;
            switch_difficult = 0;
        } else if (btns & GAMEPAD_DPAD_DOWN) {
            offset = 1;
            switch_difficult = 0;
        } else if (btns & GAMEPAD_DPAD_LEFT) {
            if (state.map.is_locked) continue;
            offset = -1;
            switch_difficult = 1;
        } else if (btns & GAMEPAD_DPAD_RIGHT) {
            if (state.map.is_locked) continue;
            offset = 1;
            switch_difficult = 1;
        } else if (btns & (GAMEPAD_A | GAMEPAD_START)) {
            let index = menu_get_selected_index(menu);
            if (index < 0 || index >= menu_get_items_count(menu) || state.difficult_locked || state.map.is_locked) {
                if (sound_asterik) soundplayer_replay(sound_asterik);
                continue;
            }
            if (await freeplaymenu_internal_modding_notify_option(state, 0)) continue;

            map_index = index;
            break;
        } else {
            continue;
        }

        if (switch_difficult) {
            let new_index = state.difficult_index + offset;
            if (new_index < 0 || new_index >= state.difficulties_size) {
                if (sound_asterik) soundplayer_replay(sound_asterik);
            } else {
                state.difficult_index = new_index;
                state.difficult_locked = state.difficulties[new_index].is_locked;
                freeplaymenu_internal_show_info(state);
                await freeplaymenu_internal_modding_notify_event(state, 1, 0);
            }
            continue;
        }

        let old_index = menu_get_selected_index(menu);
        if (!menu_select_vertical(menu, offset)) {
            let index = menu_get_selected_index(menu);
            if (index < 1)
                index = menu_get_items_count(menu) - 1;
            else
                index = 0;

            menu_select_index(menu, index);
        }

        let selected_index = menu_get_selected_index(menu);
        if (selected_index != old_index) await freeplaymenu_internal_trigger_action_menu(state, 0, 0);

        state.map = arraylist_get(songs, menu_get_selected_index(menu));
        state.use_alternative = 0;
        freeplaymenu_internal_build_difficulties(state);
        freeplaymenu_internal_show_info(state);
        freeplaymenu_internal_song_load(state, 1);

        if (selected_index != old_index) {
            await freeplaymenu_internal_trigger_action_menu(state, 1, 0);
            await freeplaymenu_internal_modding_notify_event(state, 1, 1);
        }
    }

    if (sound_asterik) soundplayer_destroy(sound_asterik);
    gamepad_destroy(gamepad);

    if (map_index >= 0) freeplaymenu_internal_trigger_action_menu(state, 0, 1);

    return map_index;
}

async function freeplaymenu_internal_load_song_async(state) {
    // adquire mutex and declare this thread
    mutex_lock(state.mutex);
    state.running_threads++;

    let weekinfo = weeks_array.array[state.map.week_index];
    let songinfo = weekinfo.songs[state.map.song_index];
    let async_id_operation = state.async_id_operation;
    let path_base = songinfo.freeplay_song_filename;
    let src_is_null = songinfo.freeplay_song_filename == null;
    let seek = songinfo.freeplay_seek_time * 1000.0;

    if (state.map.is_locked || songinfo.name == null) {
        freeplaymenu_internal_drop_soundplayer(state);

        state.running_threads--;
        mutex_unlock(state.mutex);
        return null;
    }

    // shared variables adquired, release mutex
    mutex_unlock(state.mutex);

    // guess the filename
    if (src_is_null) {
        let temp_nospaces = string_replace(songinfo.name, '\x20', '-');
        let temp_lowercase = string_to_lowercase(temp_nospaces);
        temp_nospaces = undefined;
        let temp_relativepath = string_concat(3, FUNKIN_WEEK_SONGS_FOLDER, temp_lowercase, ".ogg");
        temp_lowercase = undefined;
        path_base = weekenumerator_get_asset(weekinfo, temp_relativepath);
        temp_relativepath = undefined;
    }

    // get the path of instrumetal track of the song
    const output_paths = [null, null];
    let is_not_splitted = await songplayer_helper_get_tracks(
        path_base, state.use_alternative, output_paths
    );
    let final_path = is_not_splitted ? path_base : (output_paths[1] ?? output_paths[0]);

    L_load_soundplayer: {
        if (async_id_operation != state.async_id_operation) {
            break L_load_soundplayer;
        }

        if (final_path == state.soundplayer_path) {
            break L_load_soundplayer;
        }
        if (final_path == null) {
            freeplaymenu_internal_drop_soundplayer(state);
            break L_load_soundplayer;
        }

        // instance a soundplayer
        let soundplayer = await soundplayer_init(final_path);
        if (soundplayer == null) final_path = null;

        // check if the user selected another song
        if (async_id_operation != state.async_id_operation) {
            if (soundplayer) soundplayer_destroy(soundplayer);
            break L_load_soundplayer;
        }

        // adquire mutex and swap the soundplayer
        mutex_lock(state.mutex);

        freeplaymenu_internal_drop_soundplayer(state);
        state.soundplayer_path = strdup(final_path);
        state.soundplayer = soundplayer;

        soundplayer_set_volume(soundplayer, state.song_preview_volume);
        if (!Number.isNaN(seek)) soundplayer_seek(soundplayer, seek);
        soundplayer_play(soundplayer);
        soundplayer_fade(soundplayer, 1, 500);

        mutex_unlock(state.mutex);
    }

    mutex_lock(state.mutex);
    if (src_is_null) path_base = undefined;
    output_paths[0] = undefined;
    output_paths[1] = undefined;
    state.running_threads--;
    mutex_unlock(state.mutex);
    return null;
}

async function freeplaymenu_internal_load_background_async(state) {
    if (state.background == null) return null;

    // adquire mutex and declare this thread
    mutex_lock(state.mutex);
    state.running_threads++;

    let weekinfo = weeks_array.array[state.map.week_index];
    let src = weekinfo.songs[state.map.song_index].freeplay_background;
    let async_id_operation = state.async_id_operation;
    let modelholder = null;
    let texture = null;

    // check if the selected song has a custom background
    if (!src) {
        freeplaymenu_internal_drop_custom_background(state);
        state.running_threads--;
        mutex_unlock(state.mutex);
        return null;
    }

    if (string_lowercase_ends_with(src, ".json") || string_lowercase_ends_with(src, ".xml")) {
        modelholder = await modelholder_init(src);
    } else {
        // assume is a image file
        texture = await texture_init(src);
    }

    mutex_lock(state.mutex);

    // if the user has no changed the song, set the background
    if (async_id_operation == state.async_id_operation) {
        let sprite_anim = null;
        let sprite_tex = null;

        if (modelholder) {
            sprite_tex = modelholder_get_texture(modelholder, 1);
            sprite_anim = modelholder_create_animsprite(
                modelholder, FREEPLAYMENU_BACKGROUND_ANIM_OR_ATLAS_ENTRY_NAME, 1, 0
            );
        } else if (texture) {
            sprite_tex = texture_share_reference(texture);
        }

        freeplaymenu_internal_drop_custom_background(state);
        if (sprite_tex) sprite_set_texture(state.background, sprite_tex, 1);
        if (sprite_anim) sprite_external_animation_set(state.background, sprite_anim);

        if (sprite_tex == null)
            layout_trigger_any(state.layout, "song-background-hide");
        else
            layout_trigger_any(state.layout, "song-background-set");
    }

    if (modelholder) modelholder_destroy(modelholder);
    if (texture) texture_destroy(texture);

    state.running_threads--;
    mutex_unlock(state.mutex);
    return null;
}

function freeplaymenu_internal_show_info(state) {
    let layout = state.layout;
    let weekinfo = weeks_array.array[state.map.week_index];
    let desc = weekinfo.songs[state.map.song_index].freeplay_description;
    let week_name = weekinfo.display_name;
    let song_name = weekinfo.songs[state.map.song_index].name;

    if (!weekinfo.display_name) week_name = weekinfo.name;

    let difficult, is_locked, score;
    let bg_info_width = state.bg_info_width;
    const text_size = [0, 0];

    if (state.difficult_index >= 0 && state.difficult_index < state.difficulties_size) {
        difficult = state.difficulties[state.difficult_index].name;
        score = funkinsave_get_freeplay_score(weekinfo.name, difficult, song_name);
        is_locked = state.difficulties[state.difficult_index].is_locked;
    } else {
        score = 0;
        is_locked = 1;
        difficult = null;
    }

    if (state.map.is_locked) is_locked = 1;

    if (state.personal_best) {
        textsprite_set_text_formated(state.personal_best, FREEPLAYMENU_PERSONAL_BEST, score);
        textsprite_get_draw_size(state.personal_best, text_size);
        if (text_size[0] > bg_info_width) bg_info_width = text_size[0] * 1.1;
    }

    if (state.info) {
        textsprite_set_text_formated(state.info, FREEPLAYMENU_INFO, week_name, song_name, difficult);
        textsprite_get_draw_size(state.info, text_size);
        if (text_size[0] > bg_info_width) bg_info_width = text_size[0] * 1.1;
    }

    let bg_info = layout_get_sprite(layout, FREEPLAYMENU_BG_INFO_NAME);
    if (bg_info) sprite_set_draw_size(bg_info, bg_info_width, NaN);

    if (desc) {
        if (state.description)
            textsprite_set_text_intern(state.description, 1, desc);
        layout_trigger_any(layout, "description-show");
    } else {
        layout_trigger_any(layout, "description-hide");
    }

    layout_trigger_any(layout, is_locked ? "locked" : "not-locked");

    if (weekinfo.warning_message)
        layout_trigger_any(layout, state.use_alternative ? "use-alternative" : "not-use-alternative");
    else
        layout_trigger_any(layout, "hide-alternative");
}

function freeplaymenu_internal_drop_soundplayer(state) {
    if (state.soundplayer) {
        soundplayer_stop(state.soundplayer);
        soundplayer_destroy(state.soundplayer);
        state.soundplayer = null;
    }

    if (state.soundplayer_path) {
        state.soundplayer_path = undefined;
        state.soundplayer_path = null;
    }
}

function freeplaymenu_internal_drop_custom_background(state) {
    if (!state.background) return;
    let tex_old = sprite_set_texture(state.background, null, 0);
    let anim_old = sprite_external_animation_set(state.background, null);
    if (tex_old) texture_destroy(tex_old);
    if (anim_old) animsprite_destroy(anim_old);
}

function freeplaymenu_internal_build_difficulties(state) {
    if (state.difficulties) state.difficulties = undefined;

    let weekinfo = weeks_array.array[state.map.week_index];
    let count = 3 + weekinfo.custom_difficults_size;

    state.difficulties = new Array(count);
    state.difficulties_size = 0;

    if (weekinfo.has_difficulty_easy) {
        state.difficulties[state.difficulties_size++] = {
            name: FUNKIN_DIFFICULT_EASY,
            is_locked: 0
        };
    }
    if (weekinfo.has_difficulty_normal) {
        state.difficulties[state.difficulties_size++] = {
            name: FUNKIN_DIFFICULT_NORMAL,
            is_locked: 0
        };
    }
    if (weekinfo.has_difficulty_hard) {
        state.difficulties[state.difficulties_size++] = {
            name: FUNKIN_DIFFICULT_HARD,
            is_locked: 0
        };
    }

    for (let i = 0; i < weekinfo.custom_difficults_size; i++) {
        let is_locked = funkinsave_contains_unlock_directive(weekinfo.custom_difficults[i].unlock_directive);
        state.difficulties[state.difficulties_size++] = {
            name: weekinfo.custom_difficults[i].name,
            is_locked: is_locked
        };
    }

    // choose default difficult
    state.difficult_index = 0;
    let default_difficult = weekinfo.default_difficulty ?? FUNKIN_DIFFICULT_NORMAL;
    for (let i = 0; i < state.difficulties_size; i++) {
        if (state.difficulties[i].name == default_difficult) {
            state.difficult_index = i;
            break;
        }
    }

}

async function freeplaymenu_helper_get_default_character_manifest(is_boyfriend) {
    let src = is_boyfriend ? WEEKSELECTOR_MDLSELECT_MODELS_BF : WEEKSELECTOR_MDLSELECT_MODELS_GF;
    let json = await json_load_from(src);

    if (json == null) return null;

    let array_length = json_read_array_length(json);
    let manifest_src = null;

    for (let i = 0; i < array_length; i++) {
        let item = json_read_array_item_object(json, i);
        if (item == null) continue;

        let unlock_directive = json_read_string(item, "unlockDirectiveName", null);
        if (!funkinsave_contains_unlock_directive(unlock_directive)) continue;

        let model = json_read_string(item, "manifest", null);
        if (!model) continue;

        manifest_src = fs_build_path2(src, model);
        break;
    }

    json_destroy(json);
    return manifest_src;
}

function freeplaymenu_internal_song_load(state, with_bg) {
    mutex_lock(state.mutex);

    state.async_id_operation++;
    if (with_bg) thd_helper_spawn(freeplaymenu_internal_load_background_async, state);
    thd_helper_spawn(freeplaymenu_internal_load_song_async, state);

    mutex_unlock(state.mutex);
}

async function freeplaymenu_internal_wait_transition(state, what, duration) {
    const layout = state.layout;
    const modding = state.modding;

    await modding_helper_notify_event(modding, what);

    if (duration < 1) return;
    if (layout_trigger_any(layout, what) < 1) return;

    while (duration > 0) {
        let elapsed = await pvrctx_wait_ready();
        duration -= elapsed;
        await modding_helper_notify_frame(modding, elapsed, -1.0);
        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);
    }
}

async function freeplaymenu_internal_trigger_action_menu(state, selected, choosen) {
    if (state.map.week_index < 0 || state.map.week_index >= weeks_array.size) return;
    const weekinfo = weeks_array.array[state.map.week_index];

    if (state.map.song_index < 0 || state.map.song_index >= weekinfo.songs_count) return;
    const songs = weekinfo.songs;

    let week_name = weekinfo.display_name ?? weekinfo.name;
    let song_name = songs[state.map.song_index].name;

    if (selected) await freeplaymenu_internal_modding_notify_option(state, 1);

    main_helper_trigger_action_menu(state.layout, week_name, song_name, selected, choosen);
}

async function freeplaymenu_internal_modding_notify_option(state, selected_or_choosen) {
    let weekinfo = weeks_array.array[state.map.week_index];
    let songs = weekinfo.songs;
    let menu = state.modding.native_menu;

    let week_name = weekinfo.display_name ?? weekinfo.name;
    let song_name = songs[state.map.song_index].name;
    let name = string_concat(5, week_name, "\n", weekinfo.display_name, "\n", song_name);
    let index = menu_get_selected_index(menu);

    let ret = await modding_helper_notify_option2(state.modding, selected_or_choosen, menu, index, name);
    name = undefined;

    return ret;
}

async function freeplaymenu_internal_modding_notify_event(state, difficult, alt_tracks) {
    if (difficult && alt_tracks) {
        await modding_helper_notify_event(state.modding, state.map.is_locked ? "song-locked" : "song-not-locked");
    }
    if (difficult) {
        await modding_helper_notify_event(state.modding, state.difficult_locked ? "difficult-locked" : "difficult-not-locked");

        if (state.difficult_index >= 0 && state.difficult_index < state.difficulties_size)
            await modding_helper_notify_event(state.modding, state.difficulties[state.difficult_index].name);
        else
            await modding_helper_notify_event(state.modding, null);
    }
    if (alt_tracks) {
        await modding_helper_notify_event(state.modding, state.use_alternative ? "tracks-alt" : "tracks-not-alt");
    }
}

