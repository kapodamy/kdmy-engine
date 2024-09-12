#include "game/freeplaymenu.h"

#include <kos/mutex.h>
#include <kos/thread.h>

#include "arraylist.h"
#include "fs.h"
#include "game/common/funkin.h"
#include "game/common/funkinsave.h"
#include "game/common/menu.h"
#include "game/common/weekenumerator.h"
#include "game/freeplaymenu_types.h"
#include "game/gameplay/helpers/menumanifest.h"
#include "game/gameplay/week.h"
#include "game/gameplay/weekscript.h"
#include "game/helpers/freeplaymenu_songicons.h"
#include "game/helpers/weekselector_mdlselect.h"
#include "game/main.h"
#include "game/modding.h"
#include "game/savemanager.h"
#include "gamepad.h"
#include "jsonparser.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "modelholder.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "songplayer.h"
#include "soundplayer.h"
#include "sprite.h"
#include "stringutils.h"
#include "textsprite.h"


typedef struct {
    const char* name;
    bool is_locked;
} Difficult;

typedef struct {
    int32_t difficult_index;
    bool difficult_locked;
    bool use_alternative;
    MappedSong* map;

    volatile int32_t async_id_operation;

    volatile SoundPlayer soundplayer;
    volatile char* soundplayer_path;

    mutex_t mutex;
    volatile int32_t running_threads;

    volatile Sprite background;
    Layout layout;

    TextSprite personal_best;
    TextSprite info;
    TextSprite description;

    Difficult* difficulties;
    int32_t difficulties_size;

    float bg_info_width;
    Modding modding;
    float song_preview_volume;
} State;


static const char* FREEPLAYMENU_BACKGROUND_ANIM_OR_ATLAS_ENTRY_NAME = "freeplay-background";
static const char* FREEPLAYMENU_PERSONAL_BEST = "PERSONAL BEST SCORE: $l";
static const char* FREEPLAYMENU_INFO = "$s  -  $s  -  $s";
static const char* FREEPLAYMENU_BG_INFO_NAME = "background_song_info";
static const char* FREEPLAYMENU_LAYOUT = "/assets/common/image/freeplay-menu/layout.xml";
static const char* FREEPLAYMENU_LAYOUT_DREAMCAST = "/assets/common/image/freeplay-menu/layout~dreamcast.xml";
static const char* FREEPLAYMENU_MODDING_SCRIPT = "/assets/common/data/scripts/freeplaymenu.lua";

static struct MenuManifest_s FREEPLAYMENU_MENU_SONGS = {
    .parameters = {
        .suffix_selected = NULL,  // unused
        .suffix_idle = NULL,      // unused
        .suffix_choosen = NULL,   // unused
        .suffix_discarded = NULL, // unused
        .suffix_rollback = NULL,  // unused
        .suffix_in = NULL,        // unused
        .suffix_out = NULL,       // unused

        .atlas = NULL,
        .animlist = "/assets/common/anims/freeplay-menu.xml",

        .anim_selected = "menu_item_selected",
        .anim_idle = "menu_item_idle",
        .anim_choosen = NULL,   // unused
        .anim_discarded = NULL, // unused
        .anim_rollback = NULL,  // unused
        .anim_in = NULL,        // unused
        .anim_out = NULL,       // unused

        .anim_transition_in_delay = 0.0f,  // unused
        .anim_transition_out_delay = 0.0f, // unused

        .font = NULL,
        .font_glyph_suffix = NULL,       // unused
        .font_color_by_addition = false, // unused
        .font_size = 28.0f,
        .font_color = 0xFFFFFF,
        .font_border_color = 0x000000FF, // unused
        .font_border_size = 4.0f,        // unused

        .is_sparse = false, // unused
        .is_vertical = true,
        .is_per_page = false,
        .static_index = 1,

        .items_align = ALIGN_START,
        .items_gap = 58.0f,
        .items_dimmen = 0.0f,                     // unused
        .texture_scale = FLOAT_NaN,               // unused
        .enable_horizontal_text_correction = true // unused
    },
    .items = NULL,
    .items_size = 0
};


static int32_t freeplaymenu_show(Menu menu, State* state, ArrayList songs);
static void freeplaymenu_internal_show_info(State* state);
static void freeplaymenu_internal_drop_soundplayer(State* state, bool wait_for_background_threads);
static void freeplaymenu_internal_drop_custom_background(State* state);
static void freeplaymenu_internal_build_difficulties(State* state);
static void freeplaymenu_internal_song_load(State* state, bool with_bg);
static void freeplaymenu_internal_wait_transition(State* state, const char* what, float duration);
static void freeplaymenu_internal_trigger_action_menu(State* state, bool selected, bool choosen);
static bool freeplaymenu_internal_modding_notify_option(State* state, bool selected_or_choosen);
static void freeplaymenu_internal_modding_notify_event(State* state, bool difficult, bool alt_tracks);
static char* freeplaymenu_internal_get_song_path(WeekInfo* weekinfo, bool use_alternative, int32_t mapped_song_index);


void freeplaymenu_main() {
    const char* src_layout = pvr_context_is_widescreen() ? FREEPLAYMENU_LAYOUT : FREEPLAYMENU_LAYOUT_DREAMCAST;
    Layout layout = layout_init(src_layout);
    if (!layout) {
        logger_error("freeplay_main() missing layout");
        return;
    }

    LayoutPlaceholder* placeholder = layout_get_placeholder(layout, "menu");
    MenuManifestParameters* params = &FREEPLAYMENU_MENU_SONGS.parameters;


    params->font = (char*)layout_get_attached_value_string(
        layout, "menu_font", "/assets/common/font/Alphabet.xml"
    );
    params->font_glyph_suffix = (char*)layout_get_attached_value_string(
        layout, "menu_fontGlyphSuffix", "bold"
    );
    params->font_size = params->items_dimmen = layout_get_attached_value_as_float(
        layout, "menu_fontSize", 46.0f
    );
    params->font_color = layout_get_attached_value_hex(
        layout, "menu_fontColor", 0xFFFFFF
    );
    params->font_border_color = layout_get_attached_value_hex(
        layout, "menu_fontBorderColor", 0x000000
    );
    params->font_border_size = layout_get_attached_value_as_float(
        layout, "menu_fontBorderSize", 0.0f
    );
    params->items_gap = layout_get_attached_value_as_float(
        layout, "menu_itemsGap", 58.0f
    );
    float icons_dimmen = layout_get_attached_value_as_float(
        layout, "menu_iconsDimmen", 70.0f
    );

    float dt_playsong = layout_get_attached_value_as_float(layout, "durationTransition_playSong", 0.0f);
    float dt_screenout = layout_get_attached_value_as_float(layout, "durationTransition_screenOut", 0.0f);

    float bg_info_width = -1.0f;
    Sprite bg_info = layout_get_sprite(layout, FREEPLAYMENU_BG_INFO_NAME);
    if (bg_info) {
        float draw_width = 0.0f, draw_height = 0.0f;
        sprite_get_draw_size(bg_info, &draw_width, &draw_height);
        bg_info_width = draw_width;
    }

    // step 3: count required songs
    ArrayList songs = arraylist_init2(sizeof(MappedSong), weeks_array.size * 3);
    for (int32_t i = 0; i < weeks_array.size; i++) {
        WeekInfo* weekinfo = &weeks_array.array[i];
        bool is_week_locked = !funkinsave_contains_unlock_directive(weekinfo->unlock_directive);

        for (int32_t j = 0; j < weekinfo->songs_count; j++) {
            WeekInfoSong* song = &weekinfo->songs[j];

            bool should_hide = song->freeplay_hide_if_week_locked;
            if (should_hide && is_week_locked) continue;

            should_hide = song->freeplay_hide_if_locked;
            bool is_song_locked = !funkinsave_contains_unlock_directive(song->freeplay_unlock_directive);
            if (should_hide && is_song_locked) continue;

            int32_t gameplaymanifest_index = song->freeplay_song_index_in_gameplaymanifest;
            if (gameplaymanifest_index < 0) gameplaymanifest_index = j;

            arraylist_add(
                songs,
                &(MappedSong){
                    .song_index = j,
                    .week_index = i,
                    .gameplaymanifest_index = gameplaymanifest_index,
                    .is_locked = is_week_locked || is_song_locked,
                }
            );
        }
    }

    // step 4: prepare menu
    int32_t songs_size = arraylist_size(songs);
    FREEPLAYMENU_MENU_SONGS.items_size = songs_size;
    FREEPLAYMENU_MENU_SONGS.items = malloc_for_array(MenuManifestItem, songs_size);

    for (int32_t i = 0; i < songs_size; i++) {
        MappedSong* song = arraylist_get(songs, i);
        WeekInfo* weekinfo = &weeks_array.array[song->week_index];

        FREEPLAYMENU_MENU_SONGS.items[i] = (MenuManifestItem){
            .name = NULL, // unused
            .text = (char*)weekinfo->songs[song->song_index].name,
            .placement = {x : 0.0f, y : 0.0f, dimmen : 0.0f, gap : 0.0f}, // unused
            .anim_selected = NULL,                                        // unused
            .anim_choosen = NULL,                                         // unused
            .anim_discarded = NULL,                                       // unused
            .anim_idle = NULL,                                            // unused
            .anim_rollback = NULL,                                        // unused
            .anim_in = NULL,                                              // unused
            .anim_out = NULL,                                             // unused
            .hidden = false                                               // unused
        };
    }

    // step 5: build menu
    Menu menu_songs = menu_init(
        &FREEPLAYMENU_MENU_SONGS,
        placeholder->x,
        placeholder->y,
        placeholder->z,
        placeholder->width,
        placeholder->height
    );
    menu_set_text_force_case(menu_songs, TEXTSPRITE_FORCE_UPPERCASE);
    placeholder->vertex = menu_get_drawable(menu_songs);
    menu_select_index(menu_songs, -1);

    // step 6: drop menu manifests
    free_chk(FREEPLAYMENU_MENU_SONGS.items);
    FREEPLAYMENU_MENU_SONGS.items = NULL;

    // step 7: create menu icons
    FreeplaySongIcons songicons = freeplaymenu_songicons_init(songs, icons_dimmen, params->font_size);
    menu_set_draw_callback(menu_songs, false, (MenuDrawCallback)freeplaymenu_songicons_draw_item_icon, songicons);

    // step 8: initialize modding
    Modding modding = modding_init(layout, FREEPLAYMENU_MODDING_SCRIPT);
    modding->native_menu = modding->active_menu = menu_songs;
    modding->callback_private_data = NULL;
    modding->callback_option = NULL;
    modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN, ModdingValueType_string);


    State state = {
        .difficult_index = -1,
        .difficult_locked = false,
        .use_alternative = false,
        .map = arraylist_get(songs, 0),
        .async_id_operation = 0,
        .soundplayer_path = NULL,
        .soundplayer = NULL,
        .background = layout_get_sprite(layout, "custom_background"),
        .layout = layout,
        .personal_best = layout_get_textsprite(layout, "personal_best"),
        .info = layout_get_textsprite(layout, "info"),
        .description = layout_get_textsprite(layout, "description"),
        .difficulties = NULL,
        .difficulties_size = 0,
        .running_threads = 0,
        .mutex = {},
        .bg_info_width = bg_info_width,
        .modding = modding,
        .song_preview_volume = layout_get_attached_value_as_float(layout, "song_preview_volume", 0.7f)
    };
    mutex_init(&state.mutex, MUTEX_TYPE_NORMAL);
    if (state.background) sprite_set_texture(state.background, NULL, false);

    char* default_bf = freeplaymenu_helper_get_default_character_manifest(true);
    char* default_gf = freeplaymenu_helper_get_default_character_manifest(false);

    if (background_menu_music) {
        soundplayer_pause(background_menu_music);
    }

    layout_trigger_any(layout, "transition-in");
    modding_helper_notify_event(modding, "transition-in");

    while (true) {
        modding->has_exit = false;
        modding->has_halt = false;

        int32_t map_index = freeplaymenu_show(menu_songs, &state, songs);
        if (map_index < 0) break; // back to main menu

        const char* difficult = state.difficulties[state.difficult_index].name;
        WeekInfo* weekinfo = &weeks_array.array[((MappedSong*)arraylist_get(songs, map_index))->week_index];
        const char* gameplaymanifest = weekinfo->songs[state.map->song_index].freeplay_gameplaymanifest;
        freeplaymenu_internal_drop_soundplayer(&state, true);

        freeplaymenu_internal_wait_transition(&state, "before-play-song", dt_playsong);

        layout_suspend(layout);

        main_helper_draw_loading_screen();

        int32_t ret = week_main(
            weekinfo, state.use_alternative, difficult, default_bf, default_gf,
            gameplaymanifest, state.map->gameplaymanifest_index, "RETURN TO FREEPLAY MENU"
        );
        if (ret == 0) break; // back to main menu

        layout_resume(layout);
        freeplaymenu_internal_wait_transition(&state, "after-play-song", dt_playsong);
        freeplaymenu_internal_song_load(&state, false);
        freeplaymenu_internal_show_info(&state);
    }

    state.async_id_operation++;
    while (state.running_threads > 0) {
        // wait until all async operations are done
        thd_pass();
    }

    freeplaymenu_internal_wait_transition(&state, "transition-out", dt_screenout);
    freeplaymenu_internal_drop_custom_background(&state);

    modding_helper_notify_exit2(modding);

    free_chk(default_bf);
    free_chk(default_gf);
    free_chk(state.difficulties);
    mutex_destroy(&state.mutex);
    freeplaymenu_internal_drop_soundplayer(&state, false);
    freeplaymenu_songicons_destroy(&songicons);
    menu_destroy(&menu_songs);
    arraylist_destroy(&songs);
    layout_destroy(&layout);
    modding_destroy(&modding);

    savemanager_check_and_save_changes();

    if (background_menu_music) {
        soundplayer_play(background_menu_music);
    }
}

static int32_t freeplaymenu_show(Menu menu, State* state, ArrayList songs) {
    int32_t map_index = -1;
    Gamepad gamepad = gamepad_init(-1);
    gamepad_set_buttons_delay(gamepad, 170);

    SoundPlayer sound_asterik = soundplayer_init("/assets/common/sound/asterikMenu.ogg");

    if (menu_get_selected_index(menu) < 0 && menu_get_items_count(menu) > 0) {
        menu_select_index(menu, 0);

        state->map = arraylist_get(songs, 0);
        state->use_alternative = false;
        freeplaymenu_internal_build_difficulties(state);
        freeplaymenu_internal_show_info(state);
        freeplaymenu_internal_song_load(state, true);

        freeplaymenu_internal_trigger_action_menu(state, true, false);
        freeplaymenu_internal_modding_notify_event(state, true, true);
    }


    while (!state->modding->has_exit) {
        float elapsed = pvrctx_wait_ready();
        pvr_context_reset(&pvr_context);

        if (state->running_threads < 1 && state->modding->script && state->soundplayer) {
            weekscript_notify_timersong(state->modding->script, soundplayer_get_position(state->soundplayer));
        }
        if (modding_helper_handle_custom_menu(state->modding, gamepad, elapsed) != ModdingHelperResult_CONTINUE) {
            break;
        }

        layout_animate(state->layout, elapsed);
        layout_draw(state->layout, &pvr_context);

        if (state->modding->has_halt) continue;

        GamepadButtons btns = gamepad_has_pressed_delayed(gamepad, GAMEPAD_DPAD_UP | GAMEPAD_DPAD_DOWN | GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD_RIGHT | GAMEPAD_X | GAMEPAD_A | GAMEPAD_B | GAMEPAD_BACK | GAMEPAD_START);

        int32_t offset;
        bool switch_difficult;

        if ((btns & (GAMEPAD_B | GAMEPAD_BACK)) && !modding_helper_notify_back(state->modding)) {
            break;
        } else if (btns & GAMEPAD_X) {
            WeekInfo* weekinfo = &weeks_array.array[state->map->week_index];
            if (!weekinfo->warning_message) {
                state->use_alternative = !state->use_alternative;
                freeplaymenu_internal_modding_notify_event(state, false, true);
            } else if (sound_asterik) {
                soundplayer_replay(sound_asterik);
            }
            continue;
        } else if (btns & GAMEPAD_DPAD_UP) {
            offset = -1;
            switch_difficult = false;
        } else if (btns & GAMEPAD_DPAD_DOWN) {
            offset = 1;
            switch_difficult = false;
        } else if (btns & GAMEPAD_DPAD_LEFT) {
            if (state->map->is_locked) continue;
            offset = -1;
            switch_difficult = true;
        } else if (btns & GAMEPAD_DPAD_RIGHT) {
            if (state->map->is_locked) continue;
            offset = 1;
            switch_difficult = true;
        } else if (btns & (GAMEPAD_A | GAMEPAD_START)) {
            int32_t index = menu_get_selected_index(menu);
            if (index < 0 || index >= menu_get_items_count(menu) || state->difficult_locked || state->map->is_locked) {
                if (sound_asterik) soundplayer_replay(sound_asterik);
                continue;
            }
            if (freeplaymenu_internal_modding_notify_option(state, false)) continue;

            map_index = index;
            break;
        } else {
            continue;
        }

        if (switch_difficult) {
            int32_t new_index = state->difficult_index + offset;
            if (new_index < 0 || new_index >= state->difficulties_size) {
                if (sound_asterik) soundplayer_replay(sound_asterik);
            } else {
                state->difficult_index = new_index;
                state->difficult_locked = state->difficulties[new_index].is_locked;
                freeplaymenu_internal_show_info(state);
                freeplaymenu_internal_modding_notify_event(state, true, false);
            }
            continue;
        }

        int32_t old_index = menu_get_selected_index(menu);
        if (!menu_select_vertical(menu, offset)) {
            int32_t index = menu_get_selected_index(menu);
            if (index < 1)
                index = menu_get_items_count(menu) - 1;
            else
                index = 0;

            menu_select_index(menu, index);
        }

        int32_t selected_index = menu_get_selected_index(menu);
        if (selected_index != old_index) freeplaymenu_internal_trigger_action_menu(state, false, false);

        state->map = arraylist_get(songs, menu_get_selected_index(menu));
        state->use_alternative = false;
        freeplaymenu_internal_build_difficulties(state);
        freeplaymenu_internal_show_info(state);
        freeplaymenu_internal_song_load(state, true);

        if (selected_index != old_index) {
            freeplaymenu_internal_trigger_action_menu(state, true, false);
            freeplaymenu_internal_modding_notify_event(state, true, true);
        }
    }

    if (sound_asterik) soundplayer_destroy(&sound_asterik);
    gamepad_destroy(&gamepad);

    if (map_index >= 0) freeplaymenu_internal_trigger_action_menu(state, false, true);

    return map_index;
}

static void* freeplaymenu_internal_load_soundplayer_async(State* state) {
    int32_t async_id_operation = state->async_id_operation;

    // adquire mutex and adquire shared variables
    mutex_lock(&state->mutex);

    MappedSong* map = state->map;
    WeekInfo* weekinfo = &weeks_array.array[map->week_index];
    WeekInfoSong* songinfo = &weekinfo->songs[map->song_index];
    float seek = songinfo->freeplay_seek_time * 1000.0f;

    if (map->is_locked || songinfo->name == NULL) {
        // nothing to load
        freeplaymenu_internal_drop_soundplayer(state, false);
        mutex_unlock(&state->mutex);
        state->running_threads--;
        return NULL;
    }

    // shared variables adquired, release mutex
    mutex_unlock(&state->mutex);

    char* song_path = freeplaymenu_internal_get_song_path(
        weekinfo, state->use_alternative, map->song_index
    );

    if (async_id_operation != state->async_id_operation) {
        // another song was selected
        free_chk(song_path);
        state->running_threads--;
        return NULL;
    }

    if (song_path == NULL) {
        // song not available
        mutex_lock(&state->mutex);
        freeplaymenu_internal_drop_soundplayer(state, false);
        mutex_unlock(&state->mutex);
        state->running_threads--;
        return NULL;
    }

    if (string_equals(song_path, (char*)state->soundplayer_path)) {
        // the same song is used, seek and fade previous song
        if (state->soundplayer != NULL) {
            mutex_lock(&state->mutex);
            if (!math2d_is_float_NaN(seek)) soundplayer_seek(state->soundplayer, seek);
            soundplayer_play(state->soundplayer);
            soundplayer_fade(state->soundplayer, true, 500.0f);
            mutex_unlock(&state->mutex);
        }

        free_chk(song_path);
        state->running_threads--;
        return NULL;
    }

    // instance a new soundplayer
    SoundPlayer soundplayer = soundplayer_init(song_path);

    if (async_id_operation == state->async_id_operation) {
        // swap soundplayer
        mutex_lock(&state->mutex);
        freeplaymenu_internal_drop_soundplayer(state, false);

        if (soundplayer) {
            state->soundplayer_path = song_path;
            state->soundplayer = soundplayer;

            soundplayer_set_volume(soundplayer, state->song_preview_volume);
            if (!math2d_is_float_NaN(seek)) soundplayer_seek(soundplayer, seek);
            soundplayer_play(soundplayer);
            soundplayer_fade(soundplayer, true, 500.0f);
        } else {
            free_chk(song_path);
        }
        mutex_unlock(&state->mutex);
    } else {
        // another song was selected
        if (soundplayer) soundplayer_destroy(&soundplayer);
        free_chk(song_path);
    }

    state->running_threads--;
    return NULL;
}

static void* freeplaymenu_internal_load_background_async(State* state) {
    int32_t async_id_operation = state->async_id_operation;

    if (state->background == NULL) return NULL;

    // adquire mutex and get shared variables
    mutex_lock(&state->mutex);
    WeekInfo* weekinfo = &weeks_array.array[state->map->week_index];
    const char* src = weekinfo->songs[state->map->song_index].freeplay_background;
    ModelHolder modelholder = NULL;
    Texture texture = NULL;

    // check if the selected song has a custom background
    if (string_is_empty(src)) {
        freeplaymenu_internal_drop_custom_background(state);
        state->running_threads--;
        mutex_unlock(&state->mutex);
        return NULL;
    }

    if (modelholder_utils_is_known_extension(src)) {
        modelholder = modelholder_init(src);
    } else {
        // assume is a image file
        texture = texture_init(src);
    }

    mutex_lock(&state->mutex);

    // if the user has no changed the song, set the background
    if (async_id_operation == state->async_id_operation) {
        AnimSprite sprite_anim = NULL;
        Texture sprite_tex = NULL;

        if (modelholder) {
            sprite_tex = modelholder_get_texture(modelholder, true);
            sprite_anim = modelholder_create_animsprite(
                modelholder, FREEPLAYMENU_BACKGROUND_ANIM_OR_ATLAS_ENTRY_NAME, true, false
            );
        } else if (texture) {
            sprite_tex = texture_share_reference(texture);
        }

        freeplaymenu_internal_drop_custom_background(state);
        if (sprite_tex) sprite_set_texture(state->background, sprite_tex, true);
        if (sprite_anim) sprite_external_animation_set(state->background, sprite_anim);

        if (!sprite_tex)
            layout_trigger_any(state->layout, "song-background-hide");
        else
            layout_trigger_any(state->layout, "song-background-set");
    }

    if (modelholder) modelholder_destroy(&modelholder);
    if (texture) texture_destroy(&texture);

    state->running_threads--;
    mutex_unlock(&state->mutex);
    return NULL;
}

static void freeplaymenu_internal_show_info(State* state) {
    Layout layout = state->layout;
    WeekInfo* weekinfo = &weeks_array.array[state->map->week_index];
    const char** desc = &weekinfo->songs[state->map->song_index].freeplay_description;
    const char* week_name = weekinfo->display_name;
    const char* song_name = weekinfo->songs[state->map->song_index].name;

    if (weekinfo->display_name == NULL) week_name = weekinfo->name;

    const char* difficult;
    bool is_locked;
    int64_t score;
    float bg_info_width = state->bg_info_width;
    float text_width = 0.0f, text_height = 0.0f;

    if (state->difficult_index >= 0 && state->difficult_index < state->difficulties_size) {
        difficult = state->difficulties[state->difficult_index].name;
        score = funkinsave_get_freeplay_score(weekinfo->name, difficult, song_name);
        is_locked = state->difficulties[state->difficult_index].is_locked;
    } else {
        score = 0;
        is_locked = true;
        difficult = NULL;
    }

    if (state->map->is_locked) is_locked = true;

    if (state->personal_best) {
        textsprite_set_text_formated(state->personal_best, FREEPLAYMENU_PERSONAL_BEST, score);
        textsprite_get_draw_size(state->personal_best, &text_width, &text_height);
        if (text_width > bg_info_width) bg_info_width = text_width * 1.1f;
    }

    if (state->info) {
        textsprite_set_text_formated(state->info, FREEPLAYMENU_INFO, week_name, song_name, difficult);
        textsprite_get_draw_size(state->info, &text_width, &text_height);
        if (text_width > bg_info_width) bg_info_width = text_width * 1.1f;
    }

    Sprite bg_info = layout_get_sprite(layout, FREEPLAYMENU_BG_INFO_NAME);
    if (bg_info) sprite_set_draw_size(bg_info, bg_info_width, FLOAT_NaN);

    if (string_is_not_empty(*desc)) {
        if (state->description)
            textsprite_set_text_intern(state->description, true, (const char* const*)desc);
        layout_trigger_any(layout, "description-show");
    } else {
        layout_trigger_any(layout, "description-hide");
    }

    layout_trigger_any(layout, is_locked ? "locked" : "not-locked");

    if (weekinfo->warning_message != NULL)
        layout_trigger_any(layout, state->use_alternative ? "use-alternative" : "not-use-alternative");
    else
        layout_trigger_any(layout, "hide-alternative");
}

static void freeplaymenu_internal_drop_soundplayer(State* state, bool wait_for_background_threads) {
    // wait for running threads
    while (wait_for_background_threads) {
        thd_pass();
        mutex_lock(&state->mutex);
        bool exit = state->running_threads < 1;
        mutex_unlock(&state->mutex);
        if (exit) break;
    }

    if (state->soundplayer) {
        soundplayer_stop(state->soundplayer);
        soundplayer_destroy((SoundPlayer*)&state->soundplayer);
        state->soundplayer = NULL;
    }

    if (state->soundplayer_path) {
        free_chk((char*)state->soundplayer_path);
        state->soundplayer_path = NULL;
    }
}

static void freeplaymenu_internal_drop_custom_background(State* state) {
    if (!state->background) return;
    Texture tex_old = sprite_set_texture(state->background, NULL, false);
    AnimSprite anim_old = sprite_external_animation_set(state->background, NULL);
    if (tex_old) texture_destroy(&tex_old);
    if (anim_old) animsprite_destroy(&anim_old);
}

static void freeplaymenu_internal_build_difficulties(State* state) {
    if (state->difficulties) free_chk(state->difficulties);

    WeekInfo* weekinfo = &weeks_array.array[state->map->week_index];
    int32_t count = 3 + weekinfo->custom_difficults_size;

    state->difficulties = malloc_for_array(Difficult, count);
    state->difficulties_size = 0;

    if (weekinfo->has_difficulty_easy) {
        state->difficulties[state->difficulties_size++] = (Difficult){
            .name = FUNKIN_DIFFICULT_EASY,
            .is_locked = false
        };
    }
    if (weekinfo->has_difficulty_normal) {
        state->difficulties[state->difficulties_size++] = (Difficult){
            .name = FUNKIN_DIFFICULT_NORMAL,
            .is_locked = false
        };
    }
    if (weekinfo->has_difficulty_hard) {
        state->difficulties[state->difficulties_size++] = (Difficult){
            .name = FUNKIN_DIFFICULT_HARD,
            .is_locked = false
        };
    }

    for (int32_t i = 0; i < weekinfo->custom_difficults_size; i++) {
        bool is_locked = funkinsave_contains_unlock_directive(weekinfo->custom_difficults[i].unlock_directive);
        state->difficulties[state->difficulties_size++] = (Difficult){
            .name = weekinfo->custom_difficults[i].name,
            .is_locked = is_locked
        };
    }

    // choose default difficult
    state->difficult_index = 0;
    const char* default_difficult = weekinfo->default_difficulty ? weekinfo->default_difficulty : FUNKIN_DIFFICULT_NORMAL;
    for (int32_t i = 0; i < state->difficulties_size; i++) {
        if (string_equals(state->difficulties[i].name, default_difficult)) {
            state->difficult_index = i;
            break;
        }
    }
}

char* freeplaymenu_helper_get_default_character_manifest(bool is_boyfriend) {
    const char* src = is_boyfriend ? WEEKSELECTOR_MDLSELECT_MODELS_BF : WEEKSELECTOR_MDLSELECT_MODELS_GF;
    JSONToken json = json_load_from(src);

    if (!json) return NULL;

    int32_t array_length = json_read_array_length(json);
    char* manifest_src = NULL;

    for (int32_t i = 0; i < array_length; i++) {
        JSONToken item = json_read_array_item_object(json, i);
        if (!item) continue;

        const char* unlock_directive = json_read_string(item, "unlockDirectiveName", NULL);
        if (!funkinsave_contains_unlock_directive(unlock_directive)) continue;

        const char* model = json_read_string(item, "manifest", NULL);
        if (string_is_empty(model)) continue;

        manifest_src = fs_build_path2(src, model);
        break;
    }

    json_destroy(&json);
    return manifest_src;
}

static void freeplaymenu_internal_song_load(State* state, bool with_bg) {
    mutex_lock(&state->mutex);

    state->async_id_operation++;

    if (with_bg) {
        state->running_threads++;
        main_thd_helper_spawn(true, (AsyncThread)freeplaymenu_internal_load_background_async, state);
    }

    state->running_threads++;
    main_thd_helper_spawn(true, (AsyncThread)freeplaymenu_internal_load_soundplayer_async, state);

    mutex_unlock(&state->mutex);
}

static void freeplaymenu_internal_wait_transition(State* state, const char* what, float duration) {
    Layout layout = state->layout;
    Modding modding = state->modding;

    modding_helper_notify_event(modding, what);

    if (duration < 1.0f) return;
    if (layout_trigger_any(layout, what) < 1) return;

    while (duration > 0.0f) {
        float elapsed = pvrctx_wait_ready();
        duration -= elapsed;

        modding_helper_notify_frame(modding, elapsed, -1.0);

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);
    }
}

static void freeplaymenu_internal_trigger_action_menu(State* state, bool selected, bool choosen) {
    if (state->map->week_index < 0 || state->map->week_index >= weeks_array.size) return;
    WeekInfo* weekinfo = &weeks_array.array[state->map->week_index];

    if (state->map->song_index < 0 || state->map->song_index >= weekinfo->songs_count) return;
    WeekInfoSong* songs = weekinfo->songs;

    const char* week_name = weekinfo->display_name ? weekinfo->display_name : weekinfo->name;
    const char* song_name = songs[state->map->song_index].name;

    if (selected) freeplaymenu_internal_modding_notify_option(state, true);

    main_helper_trigger_action_menu(state->layout, week_name, song_name, selected, choosen);
}

static bool freeplaymenu_internal_modding_notify_option(State* state, bool selected_or_choosen) {
    WeekInfo* weekinfo = &weeks_array.array[state->map->week_index];
    WeekInfoSong* songs = weekinfo->songs;
    Menu menu = state->modding->native_menu;

    const char* week_name = weekinfo->display_name ? weekinfo->display_name : weekinfo->name;
    const char* song_name = songs[state->map->song_index].name;
    char* name = string_concat(5, week_name, "\n", weekinfo->display_name, "\n", song_name);
    int32_t index = menu_get_selected_index(menu);

    bool ret = modding_helper_notify_option2(state->modding, selected_or_choosen, menu, index, name);
    free_chk(name);

    return ret;
}

static void freeplaymenu_internal_modding_notify_event(State* state, bool difficult, bool alt_tracks) {
    if (difficult && alt_tracks) {
        modding_helper_notify_event(state->modding, state->map->is_locked ? "song-locked" : "song-not-locked");
    }
    if (difficult) {
        modding_helper_notify_event(state->modding, state->difficult_locked ? "difficult-locked" : "difficult-not-locked");

        if (state->difficult_index >= 0 && state->difficult_index < state->difficulties_size)
            modding_helper_notify_event(state->modding, state->difficulties[state->difficult_index].name);
        else
            modding_helper_notify_event(state->modding, NULL);
    }
    if (alt_tracks) {
        modding_helper_notify_event(state->modding, state->use_alternative ? "tracks-alt" : "tracks-not-alt");
    }
}

static char* freeplaymenu_internal_get_song_path(WeekInfo* weekinfo, bool use_alternative, int32_t mapped_song_index) {
    WeekInfoSong* songinfo = &weekinfo->songs[mapped_song_index];

    // guess the song filename or voice/instrumental part
    char* tmp_song_path;

    if (string_is_not_empty(songinfo->freeplay_song_filename)) {
        tmp_song_path = string_duplicate(songinfo->freeplay_song_filename);
    } else {
        char* temp_nospaces = string_replace_char(songinfo->name, '\x20', '-');
        char* temp_lowercase = string_to_lowercase(temp_nospaces);
        free_chk(temp_nospaces);
        char* temp_relativepath = string_concat(3, FUNKIN_WEEK_SONGS_FOLDER, temp_lowercase, ".ogg");
        free_chk(temp_lowercase);
        tmp_song_path = weekenumerator_get_asset(weekinfo, temp_relativepath);
        free_chk(temp_relativepath);
    }

    // get voice and instrumental tracks
    char *path_voices, *path_instrumental;
    bool is_not_splitted = songplayer_helper_get_tracks(
        tmp_song_path, use_alternative, &path_voices, &path_instrumental
    );

    // get the path of instrumetal track of the song (if applicable)
    if (path_instrumental != NULL) {
        free_chk(tmp_song_path);
        free_chk(path_voices);
        return path_instrumental;
    } else if (path_voices != NULL) {
        free_chk(tmp_song_path);
        free_chk(path_instrumental);
        return path_voices;
    } else if (is_not_splitted) {
        return tmp_song_path;
    }

    // missing file
    free_chk(tmp_song_path);
    return NULL;
}
