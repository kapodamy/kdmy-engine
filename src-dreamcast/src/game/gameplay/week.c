#include "game/gameplay/week.h"

#include <arch/timer.h>

#include "beatwatcher.h"
#include "camera.h"
#include "cloneutils.h"
#include "fs.h"
#include "gamepad.h"
#include "layout.h"
#include "linkedlist.h"
#include "logger.h"
#include "malloc_utils.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "sh4matrix.h"
#include "songplayer.h"
#include "sprite.h"
#include "stringutils.h"
#include "textsprite.h"
#include "texture.h"
#include "unused_switch_case.h"

#include "game/common/funkin.h"
#include "game/common/funkinsave.h"
#include "game/common/messagebox.h"
#include "game/common/weekenumerator.h"

#include "game/credits.h"
#include "game/main.h"
#include "game/savemanager.h"

#include "game/funkin/character.h"
#include "game/funkin/conductor.h"
#include "game/funkin/countdown.h"
#include "game/funkin/healthbar.h"
#include "game/funkin/healthwatcher.h"
#include "game/funkin/missnotefx.h"
#include "game/funkin/notepool.h"
#include "game/funkin/rankingcounter.h"
#include "game/funkin/roundstats.h"
#include "game/funkin/songprogressbar.h"
#include "game/funkin/streakcounter.h"
#include "game/funkin/strums.h"

#include "game/gameplay/dialogue.h"
#include "game/gameplay/helpers/autouicosmetics.h"
#include "game/gameplay/helpers/charactermanifest.h"
#include "game/gameplay/helpers/chart.h"
#include "game/gameplay/helpers/gameplaymanifest.h"
#include "game/gameplay/week_gameover.h"
#include "game/gameplay/week_pause.h"
#include "game/gameplay/week_result.h"
#include "game/gameplay/week_types.h"


#define week_internal_read_value_prefix_string(LAYT, SUFX, IDX, DEFT) \
    week_internal_read_value_prefix(LAYT, SUFX, IDX, AttachedValueType_STRING, LYT_T(string, DEFT)).as_string
#define week_internal_read_value_prefix_float(LAYT, SUFX, IDX, DEFT) \
    (float)week_internal_read_value_prefix(LAYT, SUFX, IDX, AttachedValueType_FLOAT, LYT_T(double, DEFT)).as_double
#define week_internal_read_value_prefix_long(LAYT, SUFX, IDX, DEFT) \
    week_internal_read_value_prefix(LAYT, SUFX, IDX, AttachedValueType_INTEGER, LYT_T(long, DEFT)).as_long
#define week_internal_read_value_prefix_hex(LAYT, SUFX, IDX, DEFT) \
    week_internal_read_value_prefix(LAYT, SUFX, IDX, AttachedValueType_HEX, LYT_T(unsigned, DEFT)).as_unsigned
#define week_internal_read_value_prefix_boolean(LAYT, SUFX, IDX, DEFT) \
    week_internal_read_value_prefix(LAYT, SUFX, IDX, AttachedValueType_BOOLEAN, LYT_T(boolean, DEFT)).as_boolean


// from <stdio.h>
extern int snprintf(char* str, size_t n, const char* format, ...);


static const GamepadButtons WEEKROUND_READY_BUTTONS = GAMEPAD_A | GAMEPAD_START;
// static const char* WEEKROUND_CAMERA_GIRLFRIEND = "camera_girlfriend";
static const char* WEEKROUND_CAMERA_CHARACTER = "camera_character";
const char* WEEKROUND_CAMERA_PLAYER = "camera_player";
const char* WEEKROUND_CAMERA_OPONNENT = "camera_opponent";
static const char* WEEKROUND_CAMERA_ROUNDSTART = "camera_roundStart";
static const char* WEEKROUND_CAMERA_ROUNDEND = "camera_roundEnd";
static SH4Matrix WEEKROUND_UI_MATRIX = {};
static SH4Matrix WEEKROUND_UI_MATRIX_CAMERA = {};
static const char* WEEKROUND_UI_GROUP_NAME = "______UI______";   // for internally use only
static const char* WEEKROUND_UI_GROUP_NAME2 = "______UI2______"; // for internally use only

static const char* UI_RANKINGCOUNTER_MODEL = "/assets/common/image/week-round/ranking.xml";
static const char* UI_STREAKCOUNTER_MODEL = "/assets/common/font/numbers.xml";
static const char* UI_COUNTDOWN_MODEL = "/assets/common/image/week-round/countdown.xml";
static const char* UI_ANIMLIST = "/assets/common/anims/week-round.xml";

static const char* UI_LAYOUT_WIDESCREEN = "/assets/common/image/week-round/ui.xml";
static const char* UI_LAYOUT_DREAMCAST = "/assets/common/image/week-round/ui~dreamcast.xml";
static const LayoutPlaceholder UI_STRUMS_LAYOUT_PLACEHOLDER = {.x = 0.0f, .y = 0.0f, .z = 100.0f, .width = 300.0f, .height = 54.0f};
static const LayoutPlaceholder UI_STUB_LAYOUT_PLACEHOLDER = {.x = 0.0f, .y = 0.0f, .z = -1.0f, .width = 100.0f, .height = 100.0f};
static const char* UI_SONGINFO_FORMAT = "$s $s[$s]"; // "$s $s[$s] {kdy $s}";
static const char* UI_SONGINFO_ALT_SUFFIX = "(alt) ";

const char* WEEKROUND_CHARACTER_PREFIX = "character_";

static BeatWatcher WEEK_BEAT_WATCHER = {.count = 0, .since = 0.0f};
static BeatWatcher WEEK_QUARTER_WATCHER = {.count = 0, .since = 0.0f};


static void week_init_ui_layout(const char* src_layout, InitParams* initparams, RoundContext roundcontext);
static void week_pick_inverted_ui_layout_values(RoundContext roundcontext);
static void week_round_prepare(RoundContext roundcontext, GameplayManifest gameplaymanifest);
static bool week_init_healthbar(RoundContext roundcontext, GameplayManifest gameplaymanifest, bool force_update);
static void week_init_girlfriend(RoundContext roundcontext, GameplayManifestGirlfriend* girlfriend_manifest);
static void week_init_stage(RoundContext roundcontext, const char* stage_src);
static void week_init_script(RoundContext roundcontext, const char* script_src);
static bool week_init_chart_and_players(RoundContext roundcontext, GameplayManifest gameplaymanifest, bool new_ui);
static void week_init_ui_cosmetics(RoundContext roundcontext);
static void week_init_ui_gameover(RoundContext roundcontext);
static void week_init_dialogue(RoundContext roundcontext, const char* dialogue_params, bool dialog_ignore_on_freeplay);
static void week_place_in_layout(RoundContext roundcontext);
static void week_toggle_states(RoundContext roundcontext, GameplayManifest gameplaymanifest);
void week_disable_layout_rollback(RoundContext roundcontext, bool disable);
static int32_t week_round(RoundContext roundcontext, bool from_retry, bool show_dialog);
static void week_peek_chart_events(RoundContext roundcontext, float64 timestamp);
static void week_check_directives_round(RoundContext roundcontext, bool completed);
static void week_check_directives_week(RoundContext roundcontext, bool completed);
static void week_change_scroll_direction(RoundContext roundcontext);
static LayoutAttachedValue week_internal_read_value_prefix(Layout layout, const char* name, int32_t number_suffix, AttachedValueType type, LayoutAttachedValue default_value);
static LayoutPlaceholder* week_internal_read_placeholder(Layout layout, const char* prefix_name, int32_t number_suffix);
static void week_internal_place_character(Layout layout, Character character, LayoutCharacter* layout_character);
static GameplayManifest week_internal_load_gameplay_manifest(const char* src);
static void week_internal_reset_players_and_girlfriend(RoundContext roundcontext);
static void week_internal_pick_counters_values_from_layout(RoundContext roundcontext);
static LayoutPlaceholder* week_internal_read_placeholder_counter(Layout layout, const char* name, bool warn);
static void week_internal_check_screen_resolution(RoundContext roundcontext, bool force);
static void week_internal_do_antibounce(RoundContext roundcontext);
static void* week_internal_savemanager_should_show(void* param);


static void week_destroy(RoundContext roundcontext, GameplayManifest gameplaymanifest) {
    InitParams* initparams = &roundcontext->initparams;

    // texture_disable_defering(false);

    gameplaymanifest_destroy(&gameplaymanifest);
    healthwatcher_destroy(&roundcontext->healthwatcher);

    if (roundcontext->rankingcounter) rankingcounter_destroy(&roundcontext->rankingcounter);
    if (roundcontext->streakcounter) streakcounter_destroy(&roundcontext->streakcounter);
    if (roundcontext->healthbar) healthbar_destroy(&roundcontext->healthbar);
    if (roundcontext->layout) layout_destroy(&roundcontext->layout);
    if (roundcontext->roundstats) roundstats_destroy(&roundcontext->roundstats);
    if (roundcontext->songplayer) songplayer_destroy(&roundcontext->songplayer);
    if (roundcontext->script) weekscript_destroy(&roundcontext->script);
    if (roundcontext->countdown) countdown_destroy(&roundcontext->countdown);
    if (roundcontext->girlfriend) character_destroy(&roundcontext->girlfriend);
    if (roundcontext->songinfo) textsprite_destroy(&roundcontext->songinfo);
    if (roundcontext->weekgameover) week_gameover_destroy(&roundcontext->weekgameover);
    if (roundcontext->weekpause) week_pause_destroy(&roundcontext->weekpause);
    if (roundcontext->screen_background) sprite_destroy(&roundcontext->screen_background);
    if (roundcontext->weekresult) week_result_destroy(&roundcontext->weekresult);
    if (roundcontext->messagebox) messagebox_destroy(&roundcontext->messagebox);
    if (roundcontext->ui_camera) camera_destroy(&roundcontext->ui_camera);
    if (roundcontext->missnotefx) missnotefx_destroy(&roundcontext->missnotefx);
    if (roundcontext->dialogue) dialogue_destroy(&roundcontext->dialogue);
    if (roundcontext->songprogressbar) songprogressbar_destroy(&roundcontext->songprogressbar);
    if (roundcontext->autouicosmetics) autouicosmetics_destroy(&roundcontext->autouicosmetics);
    if (roundcontext->initparams.animlist) animlist_destroy(&roundcontext->initparams.animlist);

    free_chk(roundcontext->events);
    free_chk(roundcontext->healthbarparams.player_icon_model);
    free_chk(roundcontext->healthbarparams.opponent_icon_model);

    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        if (roundcontext->players[i].playerstats) playerstats_destroy(&roundcontext->players[i].playerstats);
        if (roundcontext->players[i].character) character_destroy(&roundcontext->players[i].character);
        if (roundcontext->players[i].conductor) conductor_destroy(&roundcontext->players[i].conductor);
        if (roundcontext->players[i].notepool) notepool_destroy(&roundcontext->players[i].notepool);
        if (roundcontext->players[i].strums) strums_destroy(&roundcontext->players[i].strums);
        if (roundcontext->players[i].ddrkeymon) ddrkeymon_destroy(&roundcontext->players[i].ddrkeymon);
        if (roundcontext->players[i].controller) gamepad_destroy(&roundcontext->players[i].controller);
    }
    free_chk(roundcontext->players);

    foreach (ModifiedDirective*, directive_info, LINKEDLIST_ITERATOR, roundcontext->scriptcontext.directives) {
        free_chk(directive_info->name);
        free_chk(directive_info);
    }
    linkedlist_destroy(&roundcontext->scriptcontext.directives);

    if (roundcontext->ui_layout) layout_destroy(&roundcontext->ui_layout);

    free_chk(initparams->layout_strums);
    free_chk(initparams->layout_characters);

    if (/* pointer equals */ roundcontext->settings.camera_name_opponent != WEEKROUND_CAMERA_OPONNENT)
        free_chk(roundcontext->settings.camera_name_opponent);
    if (/* pointer equals */ roundcontext->settings.camera_name_player != WEEKROUND_CAMERA_PLAYER)
        free_chk(roundcontext->settings.camera_name_player);

    if (string_is_not_empty(roundcontext->initparams.weekinfo->custom_folder_gameplay))
        fs_override_common_folder(NULL);
}


int32_t week_main(WeekInfo* weekinfo, bool alt_tracks, const char* difficult, const char* default_bf, const char* default_gf, const char* gameplaymanifest_src, int32_t single_song_index, const char* exit_to_weekselector_label) {

    sh4matrix_clear(WEEKROUND_UI_MATRIX);
    sh4matrix_clear(WEEKROUND_UI_MATRIX_CAMERA);

    struct RoundContext_s roundcontext = {
        .rankingcounter = NULL,
        .streakcounter = NULL,
        .healthbar = NULL,
        .layout = NULL,
        .roundstats = NULL,
        .songplayer = NULL,
        .script = NULL,
        .dialogue = NULL,
        .playerstats_index = -1,
        .healthwatcher = healthwatcher_init(),
        .countdown = NULL,
        .songinfo = NULL,
        .weekgameover = NULL,
        .weekpause = NULL,
        .weekresult = NULL,
        .messagebox = NULL,
        .ui_camera = camera_init(NULL, pvr_context.screen_width, pvr_context.screen_height),
        .missnotefx = NULL,
        .songprogressbar = NULL,
        .autouicosmetics = autouicosmetics_init(),
        .screen_background = sprite_init_from_rgb8(0x00),

        .has_directive_changes = false,

        .settings = {
            .ask_ready = true,
            .do_countdown = true,
            .girlfriend_cry = true,
            .original_bpm = 100.0f,
            .original_speed = 1.0,
            .camera_bumping = true,
            .show_credits = false,
            .no_healthbar = false,

            .bpm = 100.0f,
            .speed = 1.0,

            .camera_name_opponent = (char*)WEEKROUND_CAMERA_OPONNENT,
            .camera_name_player = (char*)WEEKROUND_CAMERA_PLAYER,

            .layout_rollback = true
        },
        .girlfriend = NULL,

        .song_index = 0,
        .round_duration = -1.0,
        .song_difficult = difficult,

        .events = NULL,
        .events_size = 0,
        .events_peek_index = 0,

        .players = NULL,
        .players_size = 0,

        .girlfriend_from_default = true,
        .healthbar_from_default = true,
        .stage_from_default = true,
        .script_from_default = true,
        .dialogue_from_default = true,
        .ui_from_default = true,
        .pause_menu_from_default = false,
        .weekgameover_from_version = 0,

        .healthbarparams = {
            .has_opponent_color = false,
            .has_player_color = false,
            .opponent_color_rgb8 = 0x00,
            .player_color_rgb8 = 0x00,
            .opponent_icon_model = NULL,
            .player_icon_model = NULL,
        },

        .scriptcontext = {
            .halt_flag = false,
            .directives = linkedlist_init(),
            .force_end_flag = false,
            .force_end_round_or_week = false,
            .force_end_loose_or_win = false,
            .no_week_end_result_screen = false,
        },

        .players_from_default = false,
        .distributions_from_default = false,
        .ui_layout = NULL,
        .resolution_changes = 0,

        .initparams = {
            .alt_tracks = alt_tracks,
            .difficult = difficult,
            .default_boyfriend = default_bf,
            .default_girlfriend = default_gf,
            .single_song = single_song_index >= 0,

            .layout_strums = NULL,
            .layout_strums_size = 0,

            .layout_girlfriend = {.placeholder_id = -1},

            .layout_characters = NULL,
            .layout_characters_size = 0,

            .weekinfo = weekinfo,
            .gameplaymanifest = NULL,

            .animlist = NULL,
            .font = NULL,
            .ui_layout_height = 0.0f,
            .ui_layout_width = 0.0f,
            .ui = {
                .healthbar_x = 0.0f,
                .healthbar_y = 0.0f,
                .healthbar_z = 0.0f,
                .healthbar_length = 0.0f,
                .healthbar_dimmen = 0.0f,
                .healthbar_border = 0.0f,
                .healthbar_iconoverlap = 0.0f,
                .healthbar_warnheight = 0.0f,
                .healthbar_lockheight = 0.0f,
                .healthbar_is_vertical = false,
                .healthbar_nowarns = false,
                .roundstats_x = 0.0f,
                .roundstats_y = 0.0f,
                .roundstats_z = 0.0f,
                .roundstats_size = 0.0f,
                .roundstats_fontcolor = 0x00,
                .roundstats_hide = false,
                .streakcounter_comboheight = 0.0f,
                .streakcounter_numbergap = 0.0f,
                .streakcounter_delay = 0.0f,
                .rankingcounter_percentonly = false,
                .songinfo_x = 0.0f,
                .songinfo_y = 0.0f,
                .songinfo_z = 0.0f,
                .songinfo_maxwidth = -1.0f,
                .songinfo_maxheight = -1.0f,
                .songinfo_alignvertical = ALIGN_START,
                .songinfo_alignhorinzontal = ALIGN_START,
                .songinfo_fontcolor = 0x00,
                .songinfo_fontsize = 0.0f,
                .countdown_height = 0.0f,
                .songprogressbar_x = 0.0f,
                .songprogressbar_y = 0.0f,
                .songprogressbar_z = 0.0f,
                .songprogressbar_width = 0.0f,
                .songprogressbar_height = 0.0f,
                .songprogressbar_align = ALIGN_NONE,
                .songprogressbar_bordersize = 0.0f,
                .songprogressbar_fontsize = 0.0f,
                .songprogressbar_fontbordersize = 0.0f,
                .songprogressbar_isvertical = false,
                .songprogressbar_showtime = false,
                .songprogressbar_colorrgba8_text = 0x00,
                .songprogressbar_colorrgba8_background = 0x00,
                .songprogressbar_colorrgba8_barback = 0x00,
                .songprogressbar_colorrgba8_barfront = 0x00,
            },
        }
    };
    InitParams* initparams = &roundcontext.initparams;

    // texture_disable_defering(true);

    // the UI "bump" has one octave of beat as duration (example: 75ms @ 100bpm)
    camera_set_transition_duration(roundcontext.ui_camera, true, 0.125f);

    // screen background
    sprite_set_z_index(roundcontext.screen_background, -FLOAT_Inf);

    // pause menu
    roundcontext.weekpause = week_pause_init(exit_to_weekselector_label);
    // song/week stats
    roundcontext.weekresult = week_result_init();
    // messagebox
    roundcontext.messagebox = messagebox_init();
    messagebox_set_z_index(roundcontext.messagebox, 205);

    // setup custom folder (if exists) and the week folder as current directory
    char* week_folder = weekenumerator_get_week_folder(weekinfo);
    fs_folder_stack_push();
    fs_set_working_folder(week_folder, false);
    free_chk(week_folder);
    custom_style_from_week = weekinfo;

    if (string_is_not_empty(weekinfo->custom_folder_gameplay)) {
        fs_override_common_folder(weekinfo->custom_folder_gameplay);
    } else if (string_is_not_empty(weekinfo->custom_folder)) {
        fs_override_common_folder(weekinfo->custom_folder);
    }

    // step 1: load the gameplay manifest this hosts all engine components behavior
    GameplayManifest gameplaymanifest;
    if (gameplaymanifest_src != NULL) {
        gameplaymanifest = week_internal_load_gameplay_manifest(gameplaymanifest_src);
    } else {
        gameplaymanifest_src = weekenumerator_get_gameplay_manifest(weekinfo);
        gameplaymanifest = week_internal_load_gameplay_manifest(gameplaymanifest_src);
        free_chk((char*)gameplaymanifest_src);
    }
    if (!gameplaymanifest) {
        // texture_disable_defering(false);
        fs_folder_stack_pop();
        return 1;
    }

    // step 2: initialize the first song (round n° 1)
    roundcontext.song_index = 0; // this is very important
    initparams->gameplaymanifest = gameplaymanifest;

    bool gameover = false;
    bool retry = false;
    bool mainmenu = false;
    bool weekselector = false;
    int32_t* songs_attempts = malloc_for_array(int32_t, gameplaymanifest->songs_size);
    bool first_init = true;
    bool reject_completed = false;
    int32_t last_song = gameplaymanifest->songs_size - 1;
    bool single_song = single_song_index >= 0;

    for (int32_t i = 0; i < gameplaymanifest->songs_size; i++) songs_attempts[i] = 0;

    if (single_song) {
        if (single_song_index > gameplaymanifest->songs_size) {
            logger_error("week_main() single_song_index is out of bounds, check your gameplay manifest");
            gameplaymanifest_destroy(&gameplaymanifest);
            free_chk(songs_attempts);
            fs_folder_stack_pop();
            return 1;
        }
        roundcontext.song_index = single_song_index;
    }

    // step 3: start the round cycle
    while (roundcontext.song_index < gameplaymanifest->songs_size) {
        beatwatcher_global_set_timestamp(0);

        if (!retry) {
            week_round_prepare(&roundcontext, gameplaymanifest);
        }

        Layout layout = roundcontext.layout ? roundcontext.layout : roundcontext.ui_layout;

        // before continue set default values
        beatwatcher_reset(&WEEK_BEAT_WATCHER, true, roundcontext.settings.original_bpm);
        beatwatcher_reset(&WEEK_QUARTER_WATCHER, false, roundcontext.settings.original_bpm);
        week_change_character_camera_name(&roundcontext, true, WEEKROUND_CAMERA_OPONNENT);
        week_change_character_camera_name(&roundcontext, false, WEEKROUND_CAMERA_PLAYER);
        camera_to_origin(roundcontext.ui_camera, false);
        sh4matrix_copy_to(WEEKROUND_UI_MATRIX, WEEKROUND_UI_MATRIX_CAMERA);

        week_toggle_states(&roundcontext, gameplaymanifest);
        messagebox_set_image_sprite(roundcontext.messagebox, NULL);
        for (int32_t i = 0; i < roundcontext.players_size; i++) {
            character_use_alternate_sing_animations(roundcontext.players[i].character, false);
            character_freeze_animation(roundcontext.players[i].character, false);
            character_set_visible(roundcontext.players[i].character, true);
        }
        roundcontext.scriptcontext.halt_flag = false;
        layout_set_single_item_to_draw(layout, NULL);
        if (roundcontext.songplayer) songplayer_mute(roundcontext.songplayer, false);

        if (first_init) {
            if (roundcontext.script) {
                weekscript_notify_weekinit(roundcontext.script, single_song ? single_song_index : -1);
                week_halt(&roundcontext, true);
            }
            first_init = false;
        }

        if (roundcontext.scriptcontext.force_end_flag) {
            if (!roundcontext.scriptcontext.force_end_round_or_week) {
                gameover = roundcontext.scriptcontext.force_end_loose_or_win;
                break;
            }
            roundcontext.scriptcontext.force_end_flag = false;
        }

        // set the healthbar position
        if (roundcontext.healthbar) {
            healthbar_set_health_position2(roundcontext.healthbar, 0.5f);
            healthbar_hide_warnings(roundcontext.healthbar);
        }

        // update songprogressbar
        if (roundcontext.songprogressbar) {
            songprogressbar_set_songplayer(roundcontext.songprogressbar, roundcontext.songplayer);
            songprogressbar_set_duration(roundcontext.songprogressbar, roundcontext.round_duration);
        }

        // check if necessary show dialogue if an dialog text is provided
        bool show_dialog = false;
        bool dialog_on_freeplay = !gameplaymanifest->songs[roundcontext.song_index].dialog_ignore_on_freeplay;
        if (!retry && (!single_song || (single_song && dialog_on_freeplay))) {
            char* dialog_text = gameplaymanifest->songs[roundcontext.song_index].dialog_text;
            if (string_is_empty(dialog_text)) {
                // nothing to do
            } else if (!roundcontext.dialogue) {
                logger_error("week_main() can not load '%s' there no dialogue instance", dialog_text);
            } else if (dialogue_show_dialog(roundcontext.dialogue, dialog_text)) {
                if (roundcontext.script) weekscript_notify_dialogue_builtin_open(roundcontext.script, dialog_text);
                show_dialog = true;
            } else {
                logger_error("week_main() failed to read '%s' file", dialog_text);
            }
        }

        uint32_t vram = pvr_mem_available() / 1024;
        logger_info("week_main() pvr memory stats: " FMT_U4 "/8192 KiB used (" FMT_FLT_DCMLS(.2) "%% available)", 8192 - vram, vram / 81.92f);

        // actual gameplay is here
        int32_t current_song_index = roundcontext.song_index;
        int32_t round_result = week_round(&roundcontext, retry, show_dialog);

        retry = false;
        week_check_directives_round(&roundcontext, round_result == 0);

        if (round_result == 0) {
            week_result_add_stats(roundcontext.weekresult, &roundcontext);
        }

        if (roundcontext.scriptcontext.force_end_flag) {
            if (!roundcontext.scriptcontext.force_end_round_or_week) {
                gameover = true;
                break;
            }
            roundcontext.scriptcontext.force_end_flag = false;
        }

        if ((round_result == 0 && roundcontext.song_index != last_song && !single_song) || round_result == 2) {
            if (roundcontext.settings.layout_rollback) {
                layout_stop_all_triggers(layout);
                layout_trigger_any(layout, NULL);
            }
            week_ui_set_visibility(&roundcontext, true);
            week_internal_reset_players_and_girlfriend(&roundcontext);
            if (roundcontext.healthwatcher) healthwatcher_reset_opponents(roundcontext.healthwatcher);
            if (roundcontext.roundstats) roundstats_reset(roundcontext.roundstats);
            if (roundcontext.streakcounter) streakcounter_reset(roundcontext.streakcounter);
        }

        if (round_result == 1) {
            gameover = true;
            break;
        } else if (round_result == 3) {
            mainmenu = true;
            break;
        } else if (round_result == 4) {
            weekselector = true;
            break;
        } else if (round_result == 2) {
            // round loose, retry
            songs_attempts[roundcontext.song_index]++;
            if (roundcontext.songplayer) {
                songplayer_seek(roundcontext.songplayer, 0.0);
                songplayer_mute(roundcontext.songplayer, false);
            }

            week_toggle_states(&roundcontext, gameplaymanifest);

            if (current_song_index != roundcontext.song_index) continue;
            if (roundcontext.song_difficult != initparams->difficult) {
                reject_completed = current_song_index > 0;
                initparams->difficult = roundcontext.song_difficult;
                continue;
            }

            retry = true;
            roundcontext.events_peek_index = 0;
            continue;
        }

        if (single_song) break; // week launched from freeplaymenu

        // round completed, next one
        roundcontext.song_index++;
        retry = false;
    }

    if (mainmenu || weekselector) {
        // notify script
        if (roundcontext.script) weekscript_notify_weekleave(roundcontext.script);

        // flush unlock directives that do not need completed week
        week_check_directives_week(&roundcontext, !gameover);

        if (roundcontext.has_directive_changes) {
            int32_t save_error = savemanager_should_show(true);
            if (save_error != 0) {
                SaveManager savemanager = savemanager_init(true, save_error);

                savemanager_show(savemanager);
                savemanager_destroy(&savemanager);
            }
        }

        // dispose all allocated resources
        free_chk(songs_attempts);
        week_destroy(&roundcontext, gameplaymanifest);
        fs_folder_stack_pop();

        // if false, goto weekselector
        return mainmenu ? 0 : 1;
    }

    if (roundcontext.script) {
        weekscript_notify_weekend(roundcontext.script, gameover);
        week_halt(&roundcontext, true);
    }

    // TODO: check unlockeables
    week_check_directives_week(&roundcontext, !gameover);
    if (!gameover && !reject_completed && weekinfo->emit_directive) {
        // if the week was completed successfully emit the directive
        funkinsave_create_unlock_directive(weekinfo->emit_directive, 0x00);
    }

    // show the whole week stats and wait for the player to press START to return
    if (!gameover) {
        int32_t total_attempts = 0;
        int32_t songs_count = single_song ? 1 : gameplaymanifest->songs_size;

        for (int32_t i = 0; i < gameplaymanifest->songs_size; i++) total_attempts += songs_attempts[i];

        if (!roundcontext.scriptcontext.no_week_end_result_screen) {
            week_result_helper_show_summary(
                roundcontext.weekresult, &roundcontext, total_attempts, songs_count, single_song, reject_completed
            );
        }
        if (roundcontext.script) {
            weekscript_notify_afterresults(roundcontext.script, total_attempts, songs_count, reject_completed);
            week_halt(&roundcontext, true);
        }
    }

    if (!gameover && roundcontext.settings.show_credits) {
        // game ending credits
        credits_main();
    }

    // save progress
    if ((roundcontext.has_directive_changes || !gameover) && !reject_completed) {
        int64_t total_score = 0;
        for (int32_t i = 0; i < roundcontext.players_size; i++) {
            if (roundcontext.players[i].type != CharacterType_PLAYER) continue;
            if (roundcontext.players[i].is_opponent) continue;
            total_score += playerstats_get_score(roundcontext.players[i].playerstats);
        }

        if (single_song) {
            // Warning: the song name declared in "gameplay.json" must be the same as in "about.json"
            const char* song_name = gameplaymanifest->songs[single_song_index].name;
            funkinsave_set_freeplay_score(weekinfo->name, roundcontext.song_difficult, song_name, true, total_score);
        } else {
            funkinsave_set_week_score(weekinfo->name, roundcontext.song_difficult, true, total_score);
        }

        // keep displaying the stage layout until the save is done
        messagebox_use_small_size(roundcontext.messagebox, true);
        messagebox_use_full_title(roundcontext.messagebox, true);
        messagebox_set_title(roundcontext.messagebox, "Saving progress...");
        messagebox_hide_buttons(roundcontext.messagebox);
        messagebox_set_message(roundcontext.messagebox, NULL);
        messagebox_show(roundcontext.messagebox, true);

        // do save
        Layout layout = roundcontext.layout ? roundcontext.layout : roundcontext.ui_layout;
        int32_t save_error = (int32_t)main_spawn_coroutine(layout, week_internal_savemanager_should_show, (void*)true);

        if (save_error) {
            layout_suspend(layout);
            SaveManager savemanager = savemanager_init(true, save_error);
            savemanager_show(savemanager);
            savemanager_destroy(&savemanager);
        }
    }

    // dispose all allocated resources
    free_chk(songs_attempts);
    week_destroy(&roundcontext, gameplaymanifest);
    fs_folder_stack_pop();

    return 1;
}

static void week_init_ui_layout(const char* src_layout, InitParams* initparams, RoundContext roundcontext) {
    float layout_width = 0.0f, layout_height = 0.0f;
    UIParams* ui = &initparams->ui;
    const char* src;
    const LayoutPlaceholder* placeholder;

    if (src_layout)
        src = src_layout;
    else
        src = pvr_context_is_widescreen() ? UI_LAYOUT_WIDESCREEN : UI_LAYOUT_DREAMCAST;

    Layout layout = layout_init(src);
    if (roundcontext->ui_layout) layout_destroy(&roundcontext->ui_layout);

    if (!layout) {
        logger_error("Can not load the ui layout: %s", src);
        assert(layout);
    }

    free_chk(initparams->layout_strums);
    roundcontext->ui_layout = layout;

    layout_get_viewport_size(layout, &layout_width, &layout_height);
    camera_change_viewport(roundcontext->ui_camera, layout_width, layout_height);
    initparams->ui_layout_width = layout_width;
    initparams->ui_layout_height = layout_height;
    ui->countdown_height = initparams->ui_layout_height / 3.0f;

    initparams->font = layout_get_attached_font(layout, "ui_font");

    const char* src_animlist = layout_get_attached_value_string(layout, "ui_animlist", UI_ANIMLIST);
    AnimList old_animlist = initparams->animlist;
    initparams->animlist = src_animlist ? animlist_init(src_animlist) : NULL;
    if (old_animlist) animlist_destroy(&old_animlist);

    initparams->layout_strums_size = (int32_t)layout_get_attached_value_long(layout, "ui_strums_count", 0);
    initparams->layout_strums = malloc_for_array(LayoutStrum, initparams->layout_strums_size);

    for (int32_t i = 0; i < initparams->layout_strums_size; i++) {
        placeholder = week_internal_read_placeholder(layout, "ui_strums", i);
        if (!placeholder) placeholder = &UI_STRUMS_LAYOUT_PLACEHOLDER;

        initparams->layout_strums[i] = (LayoutStrum){
            .marker_dimmen = week_internal_read_value_prefix_float(layout, "ui_strums_markerDimmen", i, 50.0),
            .invdimmen = week_internal_read_value_prefix_float(layout, "ui_strums_markerInvdimmen", i, 50.0),
            .sustain_alpha = week_internal_read_value_prefix_float(layout, "ui_strums_sustainAlpha", i, 1.0),
            .gap = week_internal_read_value_prefix_float(layout, "ui_strums_gap", i, 0.0),
            .keep_marker_scale = week_internal_read_value_prefix_boolean(layout, "ui_strums_keepMarkerScale", i, true),
            .is_vertical = week_internal_read_value_prefix_boolean(layout, "ui_strums_verticalScroll", i, true),
            .length = 0.0f,
            .x = placeholder->x,
            .y = placeholder->y,
            .z = placeholder->z
        };
        initparams->layout_strums[i].length = initparams->layout_strums[i].is_vertical ? placeholder->height : placeholder->width;
    }

    placeholder = layout_get_placeholder(layout, "ui_healthbar");
    if (!placeholder) {
        logger_error("week_init_ui_layout() missing layout ui_healthbar placeholder");
        placeholder = &UI_STUB_LAYOUT_PLACEHOLDER;
    }
    ui->healthbar_is_vertical = layout_get_attached_value_boolean(layout, "ui_healthbar_isVertical", false);
    ui->healthbar_nowarns = !layout_get_attached_value_boolean(layout, "ui_healthbar_showWarnings", true);
    ui->healthbar_border = layout_get_attached_value_as_float(layout, "ui_healthbar_borderSize", 0.0);
    ui->healthbar_iconoverlap = layout_get_attached_value_as_float(layout, "ui_healthbar_iconOverlap", 0.0);
    ui->healthbar_warnheight = layout_get_attached_value_as_float(layout, "ui_healthbar_warningHeight", 0.0);
    ui->healthbar_lockheight = layout_get_attached_value_as_float(layout, "ui_healthbar_lockedHeight", 0.0);
    ui->healthbar_x = placeholder->x;
    ui->healthbar_y = placeholder->y;
    ui->healthbar_z = placeholder->z;
    ui->healthbar_length = placeholder->width;
    ui->healthbar_dimmen = placeholder->height;
    if (ui->healthbar_is_vertical) {
        float temp = ui->healthbar_length;
        ui->healthbar_length = ui->healthbar_dimmen;
        ui->healthbar_dimmen = temp;
    }

    placeholder = layout_get_placeholder(layout, "ui_roundstats");
    if (!placeholder) {
        logger_error("week_init_ui_layout() missing layout ui_roundstats placeholder");
        placeholder = &UI_STUB_LAYOUT_PLACEHOLDER;
    }
    ui->roundstats_x = placeholder->x;
    ui->roundstats_y = placeholder->y;
    ui->roundstats_z = placeholder->z;
    ui->roundstats_hide = layout_get_attached_value_boolean(layout, "ui_roundstats_hidden", false);
    ui->roundstats_size = (float)layout_get_attached_value_double(layout, "ui_roundstats_fontSize", 12.0f);
    ui->roundstats_fontcolor = layout_get_attached_value_hex(layout, "ui_roundstats_fontColor", 0xFFFFFF);


    placeholder = layout_get_placeholder(layout, "ui_songprogressbar");
    if (!placeholder) {
        logger_error("week_init_ui_layout() missing layout ui_songprogressbar placeholder");
        placeholder = &UI_STUB_LAYOUT_PLACEHOLDER;
    }
    ui->songprogressbar_bordersize = (float)layout_get_attached_value_double(layout, "ui_songprogressbar_borderSize", 2.0);
    ui->songprogressbar_fontsize = (float)layout_get_attached_value_double(layout, "ui_songprogressbar_fontSize", 11.0);
    ui->songprogressbar_fontbordersize = (float)layout_get_attached_value_double(layout, "ui_songprogressbar_fontBorderSize", 1.4);
    ui->songprogressbar_isvertical = layout_get_attached_value_boolean(layout, "ui_songprogressbar_isVertical", false);
    ui->songprogressbar_showtime = layout_get_attached_value_boolean(layout, "ui_songprogressbar_showTime", true);
    ui->songprogressbar_colorrgba8_text = layout_get_attached_value_hex(layout, "ui_songprogressbar_colorRGBA8_text", 0xFFFFFFFF);
    ui->songprogressbar_colorrgba8_background = layout_get_attached_value_hex(layout, "ui_songprogressbar_colorRGBA8_background", 0x000000FF);
    ui->songprogressbar_colorrgba8_barback = layout_get_attached_value_hex(layout, "ui_songprogressbar_colorRGBA8_barBack", 0x808080FF);
    ui->songprogressbar_colorrgba8_barfront = layout_get_attached_value_hex(layout, "ui_songprogressbar_colorRGBA8_barFront", 0xFFFFFFFF);
    ui->songprogressbar_x = placeholder->x;
    ui->songprogressbar_y = placeholder->y;
    ui->songprogressbar_z = placeholder->z;
    ui->songprogressbar_width = placeholder->width;
    ui->songprogressbar_height = placeholder->height;
    ui->songprogressbar_align = ui->songprogressbar_isvertical ? placeholder->align_vertical : placeholder->align_horizontal;

    placeholder = layout_get_placeholder(layout, "ui_song_info");
    if (!placeholder) {
        logger_error("week_init_ui_layout() missing layout ui_song_info placeholder");
        placeholder = &UI_STUB_LAYOUT_PLACEHOLDER;
    }
    ui->songinfo_x = placeholder->x;
    ui->songinfo_y = placeholder->y;
    ui->songinfo_z = placeholder->z;
    ui->songinfo_maxwidth = placeholder->width;
    ui->songinfo_maxheight = placeholder->height;
    ui->songinfo_alignvertical = placeholder->align_vertical;
    ui->songinfo_alignhorinzontal = placeholder->align_horizontal;
    ui->songinfo_fontsize = (float)layout_get_attached_value_double(layout, "ui_song_info_fontSize", 10.0);
    ui->songinfo_fontcolor = layout_get_attached_value_hex(layout, "ui_song_info_fontColor", 0xFFFFFF);

    // initialize adaptation of the UI elements in the stage layout
    week_internal_check_screen_resolution(roundcontext, true);

    // pick default counters values
    week_internal_pick_counters_values_from_layout(roundcontext);
}

static void week_pick_inverted_ui_layout_values(RoundContext roundcontext) {
    Layout layout = roundcontext->ui_layout;
    InitParams* initparams = &roundcontext->initparams;
    UIParams* ui = &roundcontext->initparams.ui;
    LayoutPlaceholder* placeholder;

    for (int32_t i = 0; i < initparams->layout_strums_size; i++) {
        placeholder = week_internal_read_placeholder(layout, "ui_strums_inverted", i);
        if (!placeholder) continue;

        initparams->layout_strums[i].x = placeholder->x;
        initparams->layout_strums[i].y = placeholder->y;
        initparams->layout_strums[i].z = placeholder->z;
        initparams->layout_strums[i].length = initparams->layout_strums[i].is_vertical ? placeholder->height : placeholder->width;
    }

    placeholder = layout_get_placeholder(layout, "ui_healthbar_inverted");
    if (placeholder) {
        ui->healthbar_x = placeholder->x;
        ui->healthbar_y = placeholder->y;
        ui->healthbar_z = placeholder->z;
        ui->healthbar_length = placeholder->width;
        ui->healthbar_dimmen = placeholder->height;
        if (ui->healthbar_is_vertical) {
            float temp = ui->healthbar_length;
            ui->healthbar_length = ui->healthbar_dimmen;
            ui->healthbar_dimmen = temp;
        }
    }

    placeholder = layout_get_placeholder(layout, "ui_roundstats_inverted");
    if (placeholder) {
        ui->roundstats_x = placeholder->x;
        ui->roundstats_y = placeholder->y;
        ui->roundstats_z = placeholder->z;
    }

    placeholder = layout_get_placeholder(layout, "ui_songprogressbar_inverted");
    if (placeholder) {
        ui->songprogressbar_x = placeholder->x;
        ui->songprogressbar_y = placeholder->y;
        ui->songprogressbar_z = placeholder->z;
    }

    placeholder = layout_get_placeholder(layout, "ui_song_info_inverted");
    if (placeholder) {
        ui->songinfo_x = placeholder->x;
        ui->songinfo_y = placeholder->y;
        ui->songinfo_z = placeholder->z;
    }
}


static void week_round_prepare(RoundContext roundcontext, GameplayManifest gameplaymanifest) {
    //
    // Note:
    //      Some UI components can be redefined in each week round (song). If the
    //      song does not specify this, the UI component/s are initialized to thier
    //      default settings.
    //
    //      Each song can not use the defined UI component/s of previous songs, if
    //      those songs does not have thier own definitions it will reinitialized to
    //      default.
    //
    GameplayManifestSong* songmanifest = &gameplaymanifest->songs[roundcontext->song_index];
    InitParams* initparams = &roundcontext->initparams;

    bool updated_ui = false;
    bool updated_distributions_or_players = false;
    bool updated_stage = false;

    // initialize layout
    if (songmanifest->has_stage) {
        updated_stage = true;
        roundcontext->stage_from_default = false;
        week_init_stage(roundcontext, songmanifest->stage);
    } else if (!roundcontext->layout || !roundcontext->stage_from_default) {
        updated_stage = true;
        roundcontext->stage_from_default = true;
        week_init_stage(roundcontext, gameplaymanifest->default_->stage);
    }

    // initialize script/stagescript
    if (songmanifest->has_script) {
        roundcontext->script_from_default = false;
        week_init_script(roundcontext, songmanifest->script);
    } else if (!roundcontext->script || !roundcontext->script_from_default) {
        roundcontext->script_from_default = true;
        week_init_script(roundcontext, gameplaymanifest->default_->script);
    }

    if (!roundcontext->missnotefx) {
        roundcontext->missnotefx = missnotefx_init();
    }

    // initialize ui
    if (songmanifest->has_ui_layout) {
        const char* src = songmanifest->ui_layout;
        if (string_is_empty(src)) src = gameplaymanifest->default_->ui_layout;

        week_init_ui_layout(src, initparams, roundcontext);
        roundcontext->ui_from_default = false;
        updated_ui = true;
    } else if (!roundcontext->ui_layout || !roundcontext->ui_from_default) {
        week_init_ui_layout(gameplaymanifest->default_->ui_layout, initparams, roundcontext);
        roundcontext->ui_from_default = true;
        updated_ui = true;
    }

    if (updated_ui && SETTINGS.inverse_strum_scroll) {
        // pick inverted placeholder values
        week_pick_inverted_ui_layout_values(roundcontext);
    }

    // initialize strums, character and controller misc.
    if (week_init_chart_and_players(roundcontext, gameplaymanifest, updated_ui)) {
        roundcontext->playerstats_index = -1;

        // pick playerstats from the first playable character
        for (int32_t i = 0; i < roundcontext->players_size; i++) {
            if (roundcontext->players[i].type != CharacterType_PLAYER) continue;
            roundcontext->playerstats_index = i;
            break;
        }
        if (roundcontext->playerstats_index < 0 && roundcontext->players_size > 0) {
            roundcontext->playerstats_index = 0;
        }

        updated_distributions_or_players = true;
    }

    // float multiplier = roundcontext->song_difficult == FUNKIN_DIFFICULT_EASY ? 1.25f : 1.0f;
    // for (int32_t i = 0; i < roundcontext->players_size; i++) {
    //     if (roundcontext->players[i].strums)
    //         strums_set_marker_duration_multiplier(roundcontext->players[i].strums, multiplier);
    // }

    // rebuild the healthwatcher
    healthwatcher_clear(roundcontext->healthwatcher);
    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        playerstats_reset(roundcontext->players[i].playerstats);

        // untested
        // if (roundcontext->players[i].type == CharacterType_ACTOR) continue;

        if (roundcontext->players[i].is_opponent) {
            healthwatcher_add_opponent(
                roundcontext->healthwatcher,
                roundcontext->players[i].playerstats,
                roundcontext->players[i].can_recover,
                roundcontext->players[i].can_die
            );
        } else {
            healthwatcher_add_player(
                roundcontext->healthwatcher,
                roundcontext->players[i].playerstats,
                roundcontext->players[i].can_recover,
                roundcontext->players[i].can_die
            );
        }
    }

    // initialize healthbar
    if (updated_distributions_or_players && roundcontext->healthbar) {
        updated_stage = week_init_healthbar(roundcontext, gameplaymanifest, updated_distributions_or_players);
    } else if (!roundcontext->healthbar) {
        updated_stage = week_init_healthbar(roundcontext, gameplaymanifest, true);
    }

    // initialize girlfriend
    if (songmanifest->has_girlfriend) {
        updated_stage = true;
        roundcontext->girlfriend_from_default = false;
        week_init_girlfriend(roundcontext, songmanifest->girlfriend);
    } else if (!roundcontext->girlfriend || !roundcontext->girlfriend_from_default) {
        updated_stage = true;
        roundcontext->girlfriend_from_default = true;
        week_init_girlfriend(roundcontext, gameplaymanifest->default_->girlfriend);
    }

    // add additional pause menu
    if (songmanifest->has_pause_menu) {
        roundcontext->pause_menu_from_default = false;
        week_pause_external_set_menu(roundcontext->weekpause, songmanifest->pause_menu);
    } else if (!roundcontext->pause_menu_from_default) {
        roundcontext->pause_menu_from_default = true;
        week_pause_external_set_menu(roundcontext->weekpause, gameplaymanifest->default_->pause_menu);
    }

    // initialize the song
    if (roundcontext->songplayer) songplayer_destroy(&roundcontext->songplayer);
    roundcontext->songplayer = songplayer_init(songmanifest->file, initparams->alt_tracks);

    // initialize the gameover screen
    week_init_ui_gameover(roundcontext);

    // update location&size of UI cosmetics (if required)
    if (updated_ui || updated_stage) week_init_ui_cosmetics(roundcontext);

    textsprite_set_text_formated(
        roundcontext->songinfo,
        UI_SONGINFO_FORMAT,
        songmanifest->name,
        initparams->alt_tracks ? UI_SONGINFO_ALT_SUFFIX : NULL,
        initparams->difficult /*,
         ENGINE_VERSION*/
    );

    // initialize dialogue
    if (songmanifest->dialogue_params) {
        roundcontext->dialogue_from_default = false;
        week_init_dialogue(
            roundcontext,
            songmanifest->dialogue_params,
            songmanifest->dialog_ignore_on_freeplay && initparams->single_song
        );
    } else if (!roundcontext->dialogue || !roundcontext->script_from_default) {
        roundcontext->dialogue_from_default = true;
        week_init_dialogue(
            roundcontext,
            gameplaymanifest->default_->dialogue_params,
            songmanifest->dialog_ignore_on_freeplay && initparams->single_song
        );
    }

    // Incorporates all ui elements in the stage layout
    if (updated_stage || updated_ui || updated_distributions_or_players) {
        week_place_in_layout(roundcontext);
    }

    // toggle states
    week_toggle_states(roundcontext, gameplaymanifest);
    week_update_bpm(roundcontext, roundcontext->settings.original_bpm);
    week_update_speed(roundcontext, roundcontext->settings.original_speed);
    roundcontext->round_duration = songmanifest->duration;
}

static bool week_init_healthbar(RoundContext roundcontext, GameplayManifest gameplaymanifest, bool force_update) {
    InitParams* initparams = &roundcontext->initparams;

    GameplayManifestHealthBar* healthbarmanifest = gameplaymanifest->default_->healthbar;
    HealthBarParams* healthbarparams = &roundcontext->healthbarparams;

    if (gameplaymanifest->songs[roundcontext->song_index].healthbar) {
        roundcontext->healthbar_from_default = false;
        healthbarmanifest = gameplaymanifest->songs[roundcontext->song_index].healthbar;
    } else if (force_update || !roundcontext->healthbar || !roundcontext->healthbar_from_default) {
        roundcontext->healthbar_from_default = true;
    } else {
        // no chages to make
        return false;
    }

    if (roundcontext->healthbar) {
        healthbar_destroy(&roundcontext->healthbar);
        // roundcontext->healthbar = NULL;
    }

    if (!healthbarmanifest) {
        logger_error("the manifest of the healthbar was NULL");
        assert(healthbarmanifest);
    }

    roundcontext->healthbar = healthbar_init(
        initparams->ui.healthbar_x,
        initparams->ui.healthbar_y,
        initparams->ui.healthbar_z,
        initparams->ui.healthbar_length,
        initparams->ui.healthbar_dimmen,
        initparams->ui.healthbar_border,
        initparams->ui.healthbar_iconoverlap,
        initparams->ui.healthbar_warnheight,
        initparams->ui.healthbar_lockheight
    );
    if (healthbarmanifest->has_disable) {
        roundcontext->settings.no_healthbar = healthbarmanifest->disable;
    }
    healthbar_enable_vertical(roundcontext->healthbar, initparams->ui.healthbar_is_vertical);
    healthbar_set_layout_size(
        roundcontext->healthbar, initparams->ui_layout_width, initparams->ui_layout_height
    );
    if (string_is_not_empty(healthbarmanifest->warnings_model)) {
        ModelHolder modelholder_warn_icons = modelholder_init(
            healthbarmanifest->warnings_model
        );
        if (modelholder_warn_icons) {
            healthbar_load_warnings(
                roundcontext->healthbar,
                modelholder_warn_icons,
                healthbarmanifest->use_alt_icons
            );
            modelholder_destroy(&modelholder_warn_icons);
        }
    }
    healthbar_set_visible(roundcontext->healthbar, !roundcontext->settings.no_healthbar);
    healthbar_disable_warnings(roundcontext->healthbar, initparams->ui.healthbar_nowarns);
    healthbar_set_bump_animation(roundcontext->healthbar, initparams->animlist);


    ModelHolder default_icon_model_opponent = NULL;
    ModelHolder default_icon_model_player = NULL;

    if (string_is_not_empty(healthbarparams->opponent_icon_model)) {
        default_icon_model_opponent = modelholder_init(healthbarparams->opponent_icon_model);
        free_chk(healthbarparams->opponent_icon_model);
        healthbarparams->opponent_icon_model = NULL;
    }
    if (string_is_not_empty(healthbarparams->player_icon_model)) {
        default_icon_model_player = modelholder_init(healthbarparams->player_icon_model);
        free_chk(healthbarparams->player_icon_model);
        healthbarparams->player_icon_model = NULL;
    }

    // import healthbar states
    for (int32_t i = 0; i < healthbarmanifest->states_size; i++) {
        GameplayManifestHealthBarState* state = &healthbarmanifest->states[i];

        // temporal variables
        ModelHolder icon_modelholder = NULL;
        uint32_t bar_color = state->opponent.bar_color;
        const char* bar_model = state->opponent.bar_model;

        if (string_is_empty(state->opponent.icon_model) && default_icon_model_opponent) {
            // pick from the player manifest, player 0 shold be always the opponent
            icon_modelholder = default_icon_model_opponent;
        } else if (state->opponent.icon_model) {
            icon_modelholder = modelholder_init(state->opponent.icon_model);
        }

        if (string_is_not_empty(bar_model)) {
            ModelHolder modelholder_bar = modelholder_init(bar_model);
            healthbar_state_opponent_add(
                roundcontext->healthbar, icon_modelholder, modelholder_bar, state->name
            );
            modelholder_destroy(&modelholder_bar);
        } else {
            healthbar_state_opponent_add2(
                roundcontext->healthbar, icon_modelholder, bar_color, state->name
            );
        }
        if (icon_modelholder != default_icon_model_opponent) modelholder_destroy(&icon_modelholder);

        icon_modelholder = NULL;
        bar_color = state->player.bar_color;
        bar_model = state->player.bar_model;
        if (string_is_empty(state->player.icon_model) && default_icon_model_player) {
            // pick from the player manifest, player 0 shold be always the player
            icon_modelholder = default_icon_model_player;
        } else if (string_is_not_empty(state->player.icon_model)) {
            icon_modelholder = modelholder_init(state->player.icon_model);
        }

        if (string_is_not_empty(bar_model)) {
            ModelHolder modelholder_bar = modelholder_init(bar_model);
            healthbar_state_player_add(
                roundcontext->healthbar, icon_modelholder, modelholder_bar, state->name
            );
            modelholder_destroy(&modelholder_bar);
        } else {
            healthbar_state_player_add2(
                roundcontext->healthbar, icon_modelholder, bar_color, state->name
            );
        }
        if (icon_modelholder != default_icon_model_player) modelholder_destroy(&icon_modelholder);

        if (string_is_not_empty(state->background.bar_model)) {
            ModelHolder modelholder_bar = modelholder_init(state->background.bar_model);
            if (modelholder_bar) {
                healthbar_state_background_add(
                    roundcontext->healthbar, modelholder_bar, state->name
                );
                modelholder_destroy(&modelholder_bar);
            }
        } else {
            healthbar_state_background_add2(
                roundcontext->healthbar, state->background.bar_color, NULL, state->name
            );
        }
    }

    // if the healthbar does not have states, assign the bar colors if was specified
    if (healthbarmanifest->states_size < 1) {
        uint32_t opponent_color_rgb8 = HEALTHBAR_DEFAULT_COLOR_DAD;
        uint32_t player_color_rgb8 = HEALTHBAR_DEFAULT_COLOR_BOYFRIEND;

        if (healthbarmanifest->has_opponent_color)
            opponent_color_rgb8 = healthbarmanifest->opponent_color_rgb8;
        else if (healthbarparams->has_opponent_color)
            opponent_color_rgb8 = healthbarparams->opponent_color_rgb8;

        if (healthbarmanifest->has_player_color)
            player_color_rgb8 = healthbarmanifest->player_color_rgb8;
        else if (healthbarparams->has_player_color)
            player_color_rgb8 = healthbarparams->player_color_rgb8;

        healthbar_state_background_add2(
            roundcontext->healthbar, HEALTHBAR_DEFAULT_COLOR_BACKGROUND, NULL, NULL
        );
        healthbar_state_opponent_add2(
            roundcontext->healthbar, default_icon_model_opponent, opponent_color_rgb8, NULL
        );
        healthbar_state_player_add2(
            roundcontext->healthbar, default_icon_model_player, player_color_rgb8, NULL
        );
        healthbar_state_toggle(roundcontext->healthbar, NULL);
    }

    healthbar_set_health_position(roundcontext->healthbar, 1.0f, 0.5f, false);

    modelholder_destroy(&default_icon_model_opponent);
    modelholder_destroy(&default_icon_model_player);

    return true;
}

static void week_init_girlfriend(RoundContext roundcontext, GameplayManifestGirlfriend* girlfriend_manifest) {
    InitParams* initparams = &roundcontext->initparams;

    if (roundcontext->girlfriend) {
        character_destroy(&roundcontext->girlfriend);
        roundcontext->girlfriend = NULL;
    }

    if (!girlfriend_manifest) return;

    const char* manifest;
    switch (girlfriend_manifest->refer) {
        case GameplayManifestRefer_BOYFRIEND:
            manifest = initparams->default_boyfriend;
            break;
        case GameplayManifestRefer_GIRLFRIEND:
            manifest = initparams->default_girlfriend;
            break;
        default:
            if (string_is_empty(girlfriend_manifest->manifest)) return;
            manifest = girlfriend_manifest->manifest;
            break;
    }

    CharacterManifest charactermanifest = charactermanifest_init(manifest, true);
    roundcontext->girlfriend = character_init(charactermanifest);
    charactermanifest_destroy(&charactermanifest);
}

static void week_init_stage(RoundContext roundcontext, const char* stage_src) {
    InitParams* initparams = &roundcontext->initparams;

    // keep the old layout if older assets are reused
    Layout old_layout = roundcontext->layout;
    LayoutPlaceholder* placeholder;

    if (old_layout && initparams->gameplaymanifest->songs[roundcontext->song_index].disable_resource_cache_between_songs) {
        // forget now the old layout for the current song
        layout_destroy(&old_layout);
        old_layout = NULL;
    }

    if (string_is_not_empty(stage_src)) {
        roundcontext->layout = main_helper_init_layout_suffixed(stage_src, false);
    } else {
        roundcontext->layout = NULL;
    }

    if (old_layout) layout_destroy(&old_layout);

    free_chk(initparams->layout_characters);
    initparams->layout_characters_size = 0;

    initparams->layout_characters = NULL;
    initparams->layout_girlfriend.placeholder_id = -1;

    if (!roundcontext->layout) return;

    // keep triggers synced
    layout_sync_triggers_with_global_beatwatcher(roundcontext->layout, true);

    // pick all player characters placement
    int32_t count = (int32_t)layout_get_attached_value_long(
        roundcontext->layout, "character_count", 0
    );

    initparams->layout_characters = malloc_for_array(LayoutCharacter, count);
    initparams->layout_characters_size = count;

    for (int32_t i = 0; i < count; i++) {
        initparams->layout_characters[i] = (LayoutCharacter){
            .align_vertical = ALIGN_START,
            .align_horizontal = ALIGN_START,
            .reference_width = -1.0f,
            .reference_height = -1.0f,
            .x = 0.0f,
            .y = 0.0f,
            .z = 0.0f,
            .scale = 1.0f,
            .placeholder_id = -1
        };

        placeholder = week_internal_read_placeholder(
            roundcontext->layout, WEEKROUND_CHARACTER_PREFIX, i
        );
        if (!placeholder) continue;

        initparams->layout_characters[i].placeholder_id = layout_get_placeholder_id(
            roundcontext->layout, placeholder->name
        );
        initparams->layout_characters[i].scale = week_internal_read_value_prefix_float(
            roundcontext->layout, "character_scale_", i, 1.0
        );

        initparams->layout_characters[i].x = placeholder->x;
        initparams->layout_characters[i].y = placeholder->y;
        initparams->layout_characters[i].z = placeholder->z;
        initparams->layout_characters[i].align_vertical = placeholder->align_vertical;
        initparams->layout_characters[i].align_horizontal = placeholder->align_horizontal;
        initparams->layout_characters[i].reference_width = placeholder->width;
        initparams->layout_characters[i].reference_height = placeholder->height;
    }

    // pick girlfriend character placement
    placeholder = layout_get_placeholder(roundcontext->layout, "character_girlfriend");
    if (placeholder) {
        initparams->layout_girlfriend = (LayoutCharacter){
            .align_vertical = ALIGN_START,
            .align_horizontal = ALIGN_START,
            .reference_width = -1.0f,
            .reference_height = -1.0f,
            .x = 0.0f,
            .y = 0.0f,
            .z = 0.0f,
            .scale = 1.0f,
            .placeholder_id = -1
        };

        initparams->layout_girlfriend.placeholder_id = layout_get_placeholder_id(
            roundcontext->layout, placeholder->name
        );
        initparams->layout_girlfriend.scale = (float)layout_get_attached_value_double(
            roundcontext->layout, "character_girlfriend_scale", 1.0
        );

        initparams->layout_girlfriend.x = placeholder->x;
        initparams->layout_girlfriend.y = placeholder->y;
        initparams->layout_girlfriend.z = placeholder->z;
        initparams->layout_girlfriend.align_vertical = placeholder->align_vertical;
        initparams->layout_girlfriend.align_horizontal = placeholder->align_horizontal;
        initparams->layout_girlfriend.reference_width = placeholder->width;
        initparams->layout_girlfriend.reference_height = placeholder->height;
    } else {
        initparams->layout_girlfriend.placeholder_id = -1;
    }

    week_internal_pick_counters_values_from_layout(roundcontext);
}

static void week_init_script(RoundContext roundcontext, const char* script_src) {
    if (roundcontext->script) {
        weekscript_notify_scriptchange(roundcontext->script);
        weekscript_destroy(&roundcontext->script);
    }

    roundcontext->script = NULL;

    if (string_is_not_empty(script_src)) {
        roundcontext->script = weekscript_init(script_src, roundcontext, true);
    }

    if (roundcontext->dialogue) dialogue_set_script(roundcontext->dialogue, roundcontext->script);

    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        if (roundcontext->players[i].strums) {
            strums_set_params(
                roundcontext->players[i].strums,
                roundcontext->players[i].ddrkeymon,
                roundcontext->players[i].playerstats,
                roundcontext->script
            );
        }
    }
}

static bool week_init_chart_and_players(RoundContext roundcontext, GameplayManifest gameplaymanifest, bool new_ui) {
    InitParams* initparams = &roundcontext->initparams;
    int32_t song_index = roundcontext->song_index;
    bool disable_resource_cache = initparams->gameplaymanifest->songs[song_index].disable_resource_cache_between_songs;

    Chart chart = chart_init(gameplaymanifest->songs[song_index].chart, initparams->difficult);

    // keep just in case the same textures are used
    PlayerStruct* old_players = roundcontext->players;
    int32_t old_players_size = roundcontext->players_size;

    // update events table
    free_chk(roundcontext->events);
    roundcontext->events = CLONE_STRUCT_ARRAY(ChartEventEntry, chart->events, chart->events_size);
    roundcontext->events_size = chart->events_size;
    roundcontext->events_peek_index = 0;
    roundcontext->settings.original_bpm = chart->bpm;
    roundcontext->settings.original_speed = chart->speed;

    // Pick players & strum distributions from default or current song
    bool distributions_from_default = !gameplaymanifest->songs[song_index].has_distributions;
    bool players_from_default = !gameplaymanifest->songs[song_index].has_players;
    GameplayManifestPlayer* players = gameplaymanifest->default_->players;
    int32_t players_size = gameplaymanifest->default_->players_size;
    Distribution* distributions = gameplaymanifest->default_->distributions;
    int32_t distributions_size = gameplaymanifest->default_->distributions_size;

    if (gameplaymanifest->songs[song_index].has_players) {
        players = gameplaymanifest->songs[song_index].players;
        players_size = gameplaymanifest->songs[song_index].players_size;
    }
    if (gameplaymanifest->songs[song_index].has_distributions) {
        distributions = gameplaymanifest->songs[song_index].distributions;
        distributions_size = gameplaymanifest->songs[song_index].distributions_size;
    }

    // check if is required update the current players & strum distributions
    bool same_players = roundcontext->players_from_default && players_from_default;
    bool same_distributions = roundcontext->distributions_from_default == distributions_from_default;
    if (same_players && same_distributions && old_players) {
        week_internal_reset_players_and_girlfriend(roundcontext);

        // update only the strums and the character animations
        for (int32_t i = 0; i < roundcontext->players_size; i++) {
            if (players[i].distribution_index >= 0 || roundcontext->players[i].strums) {
                if (players[i].distribution_index >= distributions_size) {
                    logger_error("invalid distribution_index");
                    assert(players[i].distribution_index < distributions_size);
                }

                Distribution* distribution = &distributions[players[i].distribution_index];
                strums_set_notes(
                    roundcontext->players[i].strums,
                    chart,
                    distribution->strums,
                    distribution->strums_size,
                    roundcontext->players[i].notepool
                );
            }
        }

        chart_destroy(&chart);

        if (new_ui) {
            for (int32_t i = 0; i < roundcontext->players_size; i++) {
                if (!roundcontext->players[i].strums) continue;
                int32_t id = players[i].layout_strums_id < 0 ? i : players[i].layout_strums_id;

                if (id >= initparams->layout_strums_size) {
                    logger_error("week_init_chart_and_players() invalid player[" FMT_I4 "].layoutStrumsId=" FMT_I4, i, id);
                    continue;
                } else if (id < 0) {
                    continue;
                }

                LayoutStrum* layout_strum = &initparams->layout_strums[id];
                strums_force_rebuild(
                    roundcontext->players[i].strums,
                    layout_strum->x,
                    layout_strum->y,
                    layout_strum->z,
                    layout_strum->marker_dimmen,
                    layout_strum->invdimmen,
                    layout_strum->length,
                    layout_strum->gap,
                    layout_strum->is_vertical,
                    layout_strum->keep_marker_scale
                );
            }

            week_change_scroll_direction(roundcontext);
        }

        return false;
    }

    if (disable_resource_cache && old_players_size > 0) {
        // dispose old players now
        for (int32_t i = 0; i < old_players_size; i++) {
            if (old_players[i].playerstats) playerstats_destroy(&old_players[i].playerstats);
            if (old_players[i].character) character_destroy(&old_players[i].character);
            if (old_players[i].conductor) conductor_destroy(&old_players[i].conductor);
            if (old_players[i].notepool) notepool_destroy(&old_players[i].notepool);
            if (old_players[i].strums) strums_destroy(&old_players[i].strums);
            if (old_players[i].ddrkeymon) ddrkeymon_destroy(&old_players[i].ddrkeymon);
            if (old_players[i].controller) gamepad_destroy(&old_players[i].controller);
        }
        free_chk(old_players);
        old_players = NULL;
        old_players_size = 0;
    }

    // remember where players and distributions was picked
    roundcontext->players_from_default = players_from_default;
    roundcontext->distributions_from_default = distributions_from_default;

    CharacterManifest* charactermanifests = malloc_for_array(CharacterManifest, players_size);
    CharacterManifest manifest_player = NULL;
    CharacterManifest manifest_opponent = NULL;

    roundcontext->players = malloc_for_array(PlayerStruct, players_size);
    roundcontext->players_size = players_size;

    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        const char* manifest_src;
        switch (players[i].refer) {
            case GameplayManifestRefer_BOYFRIEND:
                manifest_src = initparams->default_boyfriend;
                break;
            case GameplayManifestRefer_GIRLFRIEND:
                manifest_src = initparams->default_girlfriend;
                break;
            default:
                manifest_src = players[i].manifest;
                break;
        }

        charactermanifests[i] = charactermanifest_init(manifest_src, true);

        roundcontext->players[i] = (PlayerStruct){
            .character = character_init(charactermanifests[i]),
            .conductor = conductor_init(),
            .playerstats = playerstats_init(),
            .notepool = NULL,
            .strums = NULL,
            .type = CharacterType_ACTOR,
            .ddrkeymon = NULL,
            .ddrkeys_fifo = NULL,
            .controller = NULL,
            .is_vertical = true,
            .can_die = players[i].can_die,
            .can_recover = players[i].can_recover,
            .is_opponent = players[i].is_opponent
        };

        conductor_set_character(
            roundcontext->players[i].conductor, roundcontext->players[i].character
        );
        conductor_set_missnotefx(roundcontext->players[i].conductor, roundcontext->missnotefx);

        character_face_as_opponent(roundcontext->players[i].character, roundcontext->players[i].is_opponent);

        // obtain the position in the UI layout
        int32_t layout_strums_id = players[i].layout_strums_id < 0 ? i : players[i].layout_strums_id;

        if (layout_strums_id >= initparams->layout_strums_size) {
            logger_error("week_init_chart_and_players() invalid player[" FMT_I4 "].layoutStrumsId=" FMT_I4, i, layout_strums_id);
            layout_strums_id = -1;
        }

        if (layout_strums_id < 0 || players[i].distribution_index < 0) {
            roundcontext->players[i].type = CharacterType_ACTOR;
            continue;
        }
        if (players[i].distribution_index >= distributions_size) {
            logger_error("invalid distribution_index");
            assert(players[i].distribution_index < distributions_size);
        }

        Distribution* distribution = &distributions[players[i].distribution_index];
        CharacterType type = players[i].controller < 0 ? CharacterType_BOT : CharacterType_PLAYER;
        LayoutStrum* layout_strum = &initparams->layout_strums[layout_strums_id];

        if (type == CharacterType_PLAYER) {
            playerstats_enable_penality_on_empty_strum(
                roundcontext->players[i].playerstats, SETTINGS.penality_on_empty_strum
            );
            roundcontext->players[i].controller = gamepad_init2(players[i].controller, false);
            roundcontext->players[i].ddrkeymon = ddrkeymon_init(
                roundcontext->players[i].controller, distribution->strum_binds, distribution->strums_size
            );
            roundcontext->players[i].ddrkeys_fifo = ddrkeymon_get_fifo(roundcontext->players[i].ddrkeymon);
        }

        // initialize the player components: controller+strums+conductor+character
        roundcontext->players[i].is_vertical = layout_strum->is_vertical;
        roundcontext->players[i].type = type;
        roundcontext->players[i].notepool = notepool_init(
            distribution->notes,
            distribution->notes_size,
            layout_strum->marker_dimmen,
            layout_strum->invdimmen,
            ScrollDirection_UPSCROLL
        );
        roundcontext->players[i].strums = strums_init(
            layout_strum->x,
            layout_strum->y,
            layout_strum->z,
            layout_strum->marker_dimmen,
            layout_strum->invdimmen,
            layout_strum->length,
            layout_strum->gap,
            layout_strums_id,
            layout_strum->is_vertical,
            layout_strum->keep_marker_scale,
            distribution->strums,
            distribution->strums_size
        );
        strums_set_params(
            roundcontext->players[i].strums,
            roundcontext->players[i].ddrkeymon,
            roundcontext->players[i].playerstats,
            roundcontext->script
        );
        strums_set_notes(
            roundcontext->players[i].strums,
            chart,
            distribution->strums,
            distribution->strums_size,
            roundcontext->players[i].notepool
        );
        strums_set_scroll_speed(roundcontext->players[i].strums, chart->speed);
        strums_set_bpm(roundcontext->players[i].strums, chart->bpm);
        strums_use_funkin_maker_duration(roundcontext->players[i].strums, SETTINGS.use_funkin_marker_duration);

        // attach strums and notes states
        for (int32_t j = 0; j < distribution->states_size; j++) {
            DistributionStrumState* state = &distribution->states[j];
            ModelHolder marker = NULL, sick_effect = NULL, background = NULL, notes = NULL;

            if (string_is_not_empty(state->model_marker))
                marker = modelholder_init(state->model_marker);
            if (string_is_not_empty(state->model_sick_effect))
                sick_effect = modelholder_init(state->model_sick_effect);
            if (string_is_not_empty(state->model_background) & fs_file_exists(state->model_background))
                background = modelholder_init(state->model_background);
            if (string_is_not_empty(state->model_notes))
                notes = modelholder_init(state->model_notes);

            strums_state_add(
                roundcontext->players[i].strums, marker, sick_effect, background, state->name
            );
            if (notes)
                notepool_add_state(roundcontext->players[i].notepool, notes, state->name);

            if (marker) modelholder_destroy(&marker);
            if (sick_effect) modelholder_destroy(&sick_effect);
            if (background) modelholder_destroy(&background);
            if (notes) modelholder_destroy(&notes);
        }

        // attach all character states
        for (int32_t j = 0; j < players[i].states_size; j++) {
            ModelHolder modelholder = modelholder_init(players[i].states[j].model);
            if (modelholder) {
                character_state_add(
                    roundcontext->players[i].character, modelholder, players[i].states[j].name
                );
                modelholder_destroy(&modelholder);
            }
        }

        // set alpha of all sustain notes
        notepool_change_alpha_sustain(roundcontext->players[i].notepool, layout_strum->sustain_alpha);

        // toggle default state
        strums_state_toggle(roundcontext->players[i].strums, NULL);
        strums_enable_post_sick_effect_draw(roundcontext->players[i].strums, true);

        // pick the health icons model (if required)
        switch (layout_strums_id) {
            case 0:
                // the opponent should always be the first player
                manifest_opponent = charactermanifests[i];
                break;
            case 1:
                // the player should always be the second player
                manifest_player = charactermanifests[i];
                break;
        }
    }

    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        if (!roundcontext->players[i].strums) continue;
        // strums-->conductors-->character
        conductor_use_strums(roundcontext->players[i].conductor, roundcontext->players[i].strums);
        int32_t count = strums_get_lines_count(roundcontext->players[i].strums);
        int32_t mapped = conductor_map_automatically(roundcontext->players[i].conductor, false);

        if (mapped != count) {
            logger_warn("week_init_chart_and_players() only mapped " FMT_I4 " of " FMT_I4 " lines", mapped, count);
        }
    }

    if (roundcontext->players_size > 0) {
        if (!manifest_opponent) {
            manifest_opponent = charactermanifests[0];
        }
        if (!manifest_player && roundcontext->players_size > 1) {
            manifest_opponent = charactermanifests[1];
        }
    }

    week_change_scroll_direction(roundcontext);

    // set the health icon&color from the character manifest
    if (manifest_opponent) {
        roundcontext->healthbarparams.opponent_icon_model = string_duplicate(
            manifest_opponent->model_health_icons
        );
        roundcontext->healthbarparams.has_opponent_color = manifest_opponent->has_healthbar_color;
        roundcontext->healthbarparams.opponent_color_rgb8 = manifest_opponent->healthbar_color;
    } else {
        roundcontext->healthbarparams.opponent_icon_model = NULL;
        roundcontext->healthbarparams.has_opponent_color = false;
    }
    if (manifest_player) {
        roundcontext->healthbarparams.player_icon_model = string_duplicate(
            manifest_player->model_health_icons
        );
        roundcontext->healthbarparams.has_player_color = manifest_player->has_healthbar_color;
        roundcontext->healthbarparams.player_color_rgb8 = manifest_player->healthbar_color;
    } else {
        roundcontext->healthbarparams.player_icon_model = NULL;
        roundcontext->healthbarparams.has_player_color = false;
    }

    // dispose chart
    chart_destroy(&chart);

    // dispose all charactermanifest loaded
    for (int32_t i = 0; i < players_size; i++) charactermanifest_destroy(&charactermanifests[i]);
    free_chk(charactermanifests);

    // dispose old players array
    for (int32_t i = 0; i < old_players_size; i++) {
        if (old_players[i].playerstats) playerstats_destroy(&old_players[i].playerstats);
        if (old_players[i].character) character_destroy(&old_players[i].character);
        if (old_players[i].conductor) conductor_destroy(&old_players[i].conductor);
        if (old_players[i].notepool) notepool_destroy(&old_players[i].notepool);
        if (old_players[i].strums) strums_destroy(&old_players[i].strums);
        if (old_players[i].ddrkeymon) ddrkeymon_destroy(&old_players[i].ddrkeymon);
        if (old_players[i].controller) gamepad_destroy(&old_players[i].controller);
    }
    free_chk(old_players);

    return true;
}

static void week_init_ui_cosmetics(RoundContext roundcontext) {
    InitParams* initparams = &roundcontext->initparams;
    float viewport_width = 0.0f, viewport_height = 0.0f;
    Layout layout = roundcontext->layout ? roundcontext->layout : roundcontext->ui_layout;
    bool disable_resource_cache = initparams->gameplaymanifest->songs[roundcontext->song_index].disable_resource_cache_between_songs;

    layout_get_viewport_size(roundcontext->ui_layout, &viewport_width, &viewport_height);
    bool has_autoplace = autouicosmetics_prepare_placeholders(roundcontext->autouicosmetics, layout);

    LayoutPlaceholder* placeholder_streakcounter = week_internal_read_placeholder_counter(
        layout, "ui_streakcounter", !has_autoplace
    );
    LayoutPlaceholder* placeholder_rankingcounter_rank = week_internal_read_placeholder_counter(
        layout, "ui_rankingcounter_rank", !has_autoplace
    );
    LayoutPlaceholder* placeholder_rankingcounter_accuracy = week_internal_read_placeholder_counter(
        layout, "ui_rankingcounter_accuracy", !has_autoplace
    );

    // guess the position of streakcounter and rankingcounter if has missing placeholders
    if (has_autoplace) {
        if (placeholder_streakcounter) placeholder_streakcounter = &AUTOUICOSMETICS_PLACEHOLDER_STREAK;
        if (placeholder_rankingcounter_rank) placeholder_rankingcounter_rank = &AUTOUICOSMETICS_PLACEHOLDER_RANK;
        if (placeholder_rankingcounter_accuracy) placeholder_rankingcounter_accuracy = &AUTOUICOSMETICS_PLACEHOLDER_ACCURACY;
    }

    // keep a copy of the old values
    RankingCounter old_rankingcounter = roundcontext->rankingcounter;
    StreakCounter old_streakcounter = roundcontext->streakcounter;
    Countdown old_countdown = roundcontext->countdown;
    SongProgressbar old_songprogressbar = roundcontext->songprogressbar;
    if (roundcontext->roundstats) roundstats_destroy(&roundcontext->roundstats);
    if (roundcontext->songinfo) textsprite_destroy(&roundcontext->songinfo);

    if (disable_resource_cache) {
        // dispose old ui elements now
        if (old_rankingcounter) rankingcounter_destroy(&old_rankingcounter);
        if (old_streakcounter) streakcounter_destroy(&old_streakcounter);
        if (old_countdown) countdown_destroy(&old_countdown);
        if (old_songprogressbar) songprogressbar_destroy(&old_songprogressbar);
    }

    // step 1: initialize all "cosmetic" components
    ModelHolder modelholder_rankingstreak = modelholder_init(UI_RANKINGCOUNTER_MODEL);
    ModelHolder modelholder_streakcounter = modelholder_init(UI_STREAKCOUNTER_MODEL);
    ModelHolder modelholder_countdown = modelholder_init(UI_COUNTDOWN_MODEL);

    // step 1a: ranking counter
    roundcontext->rankingcounter = rankingcounter_init(
        placeholder_rankingcounter_rank,
        placeholder_rankingcounter_accuracy,
        initparams->font
    );
    rankingcounter_add_state(roundcontext->rankingcounter, modelholder_rankingstreak, NULL);
    rankingcounter_toggle_state(roundcontext->rankingcounter, NULL);
    rankingcounter_set_default_ranking_animation(
        roundcontext->rankingcounter, initparams->animlist
    );
    rankingcounter_set_default_ranking_text_animation(
        roundcontext->rankingcounter, initparams->animlist
    );
    rankingcounter_use_percent_instead(
        roundcontext->rankingcounter,
        initparams->ui.rankingcounter_percentonly
    );

    // step 1b: initializae streak counter
    roundcontext->streakcounter = streakcounter_init(
        placeholder_streakcounter,
        initparams->ui.streakcounter_comboheight,
        initparams->ui.streakcounter_numbergap,
        initparams->ui.streakcounter_delay
    );
    streakcounter_state_add(
        roundcontext->streakcounter, modelholder_rankingstreak, modelholder_streakcounter, NULL
    );
    streakcounter_state_toggle(roundcontext->streakcounter, NULL);
    streakcounter_set_number_animation(
        roundcontext->streakcounter, initparams->animlist
    );
    streakcounter_set_combo_animation(
        roundcontext->streakcounter, initparams->animlist
    );

    // step 1c: initialize roundstats
    roundcontext->roundstats = roundstats_init(
        initparams->ui.roundstats_x,
        initparams->ui.roundstats_y,
        initparams->ui.roundstats_z,
        initparams->font,
        initparams->ui.roundstats_size,
        viewport_width
    );
    roundstats_hide_nps(roundcontext->roundstats, initparams->ui.roundstats_hide);

    // step 1d: initialize songprogressbar
    if (SETTINGS.song_progressbar) {
        roundcontext->songprogressbar = songprogressbar_init(
            initparams->ui.songprogressbar_x, initparams->ui.songprogressbar_y,
            initparams->ui.songprogressbar_z,
            initparams->ui.songprogressbar_width, initparams->ui.songprogressbar_height,
            initparams->ui.songprogressbar_align,
            initparams->ui.songprogressbar_bordersize, initparams->ui.songprogressbar_isvertical,
            initparams->ui.songprogressbar_showtime,
            initparams->font,
            initparams->ui.songprogressbar_fontsize, initparams->ui.songprogressbar_fontbordersize,
            initparams->ui.songprogressbar_colorrgba8_text,
            initparams->ui.songprogressbar_colorrgba8_background,
            initparams->ui.songprogressbar_colorrgba8_barback,
            initparams->ui.songprogressbar_colorrgba8_barfront
        );
        songprogressbar_set_songplayer(roundcontext->songprogressbar, roundcontext->songplayer);
        songprogressbar_set_duration(roundcontext->songprogressbar, roundcontext->round_duration);
        if (SETTINGS.song_progressbar_remaining) songprogressbar_show_elapsed(roundcontext->songprogressbar, false);
    } else {
        roundcontext->songprogressbar = NULL;
    }

    // step 1e: initialize countdown
    roundcontext->countdown = countdown_init(
        modelholder_countdown,
        initparams->ui.countdown_height
    );
    countdown_set_default_animation(roundcontext->countdown, initparams->animlist);
    countdown_set_layout_viewport(
        roundcontext->countdown, initparams->ui_layout_width, initparams->ui_layout_height
    );

    // step 1f: initialize songinfo
    roundcontext->songinfo = textsprite_init2(
        initparams->font, initparams->ui.songinfo_fontsize, initparams->ui.songinfo_fontcolor
    );
    textsprite_set_align(
        roundcontext->songinfo,
        initparams->ui.songinfo_alignvertical,
        initparams->ui.songinfo_alignhorinzontal
    );
    textsprite_set_max_draw_size(
        roundcontext->songinfo,
        initparams->ui.songinfo_maxwidth,
        initparams->ui.songinfo_maxheight
    );
    textsprite_set_draw_location(
        roundcontext->songinfo, initparams->ui.songinfo_x, initparams->ui.songinfo_y
    );
    textsprite_set_z_index(roundcontext->songinfo, initparams->ui.songinfo_z);
    textsprite_border_enable(roundcontext->songinfo, true);
    textsprite_border_set_size(roundcontext->songinfo, ROUNDSTATS_FONT_BORDER_SIZE);
    textsprite_border_set_color_rgba8(roundcontext->songinfo, 0x000000FF); // black

    // step 2: dispose all modelholders used
    modelholder_destroy(&modelholder_rankingstreak);
    modelholder_destroy(&modelholder_streakcounter);
    modelholder_destroy(&modelholder_countdown);

    // step 3: dispose old ui elements
    if (old_rankingcounter) rankingcounter_destroy(&old_rankingcounter);
    if (old_streakcounter) streakcounter_destroy(&old_streakcounter);
    if (old_countdown) countdown_destroy(&old_countdown);
    if (old_songprogressbar) songprogressbar_destroy(&old_songprogressbar);

    // step 4: drawn away if ui cosmetics are disabled
    if (!SETTINGS.gameplay_enabled_uicosmetics) {
        // drawn away
        if (placeholder_streakcounter) placeholder_streakcounter->vertex = NULL;
        if (placeholder_rankingcounter_rank) placeholder_rankingcounter_rank->vertex = NULL;
        if (placeholder_rankingcounter_accuracy) placeholder_rankingcounter_accuracy->vertex = NULL;
    }

    // step 5: pick drawables if "ui_autoplace_cosmetics" placeholder is present
    autouicosmetics_pick_drawables(roundcontext->autouicosmetics);
}

static void week_init_ui_gameover(RoundContext roundcontext) {
    WeekGameOver old_weekgameover = roundcontext->weekgameover;
    uint32_t version = week_gameover_read_version();
    bool disable_resource_cache = roundcontext->initparams.gameplaymanifest->songs[roundcontext->song_index].disable_resource_cache_between_songs;

    if (old_weekgameover && version == roundcontext->weekgameover_from_version) return;

    if (disable_resource_cache && old_weekgameover) {
        week_gameover_destroy(&old_weekgameover);
        old_weekgameover = NULL;
    }

    // build the gameover screen and dispose the older one
    roundcontext->weekgameover = week_gameover_init();
    roundcontext->weekgameover_from_version = version;
    if (old_weekgameover) week_gameover_destroy(&old_weekgameover);
}

static void week_init_dialogue(RoundContext roundcontext, const char* dialogue_params, bool dialog_ignore_on_freeplay) {
    if (!dialogue_params) return;

    // dettach from the layout
    Layout layout = roundcontext->layout ? roundcontext->layout : roundcontext->ui_layout;
    int32_t group_id = layout_get_group_id(layout, WEEKROUND_UI_GROUP_NAME2);
    if (!layout) return; // this never should happen

    if (roundcontext->dialogue) dialogue_destroy(&roundcontext->dialogue);

    if (dialog_ignore_on_freeplay) {
        roundcontext->dialogue = NULL;
    } else {
        float width = 0.0f, height = 0.0f;
        layout_get_viewport_size(roundcontext->ui_layout, &width, &height);
        roundcontext->dialogue = dialogue_init(dialogue_params, width, height);
    }

    if (roundcontext->dialogue) {
        dialogue_set_script(roundcontext->dialogue, roundcontext->script);
        layout_external_vertex_set_entry(
            layout, 7, VERTEX_DRAWABLE, dialogue_get_drawable(roundcontext->dialogue), group_id
        );
    } else {
        layout_external_vertex_set_entry(layout, 7, VERTEX_DRAWABLE, NULL, group_id);
    }
}


static void week_place_in_layout(RoundContext roundcontext) {
    InitParams* initparams = &roundcontext->initparams;
    const int32_t UI_SIZE = 10; // all UI "cosmetics" elements + screen background + dialogue + messagebox

    Layout layout;
    bool is_ui;
    if (roundcontext->layout) {
        is_ui = false;
        layout = roundcontext->layout;
        if (roundcontext->ui_layout) layout_external_vertex_create_entries(roundcontext->ui_layout, 0);
    } else {
        is_ui = true;
        layout = roundcontext->ui_layout;
        if (roundcontext->layout) layout_external_vertex_create_entries(roundcontext->layout, 0);
    }

    // if there no stage layout, hide all characters
    if (is_ui) return;

    // step 1: declare the amout of items to add
    int32_t size = UI_SIZE;
    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        // count strums
        if (roundcontext->players[i].strums) size++;
    }

    int32_t ui1 = layout_external_create_group(layout, WEEKROUND_UI_GROUP_NAME, 0);
    if (ui1 < 0) ui1 = layout_get_group_id(layout, WEEKROUND_UI_GROUP_NAME);
    layout_set_group_static_to_screen_by_id(layout, ui1, WEEKROUND_UI_MATRIX_CAMERA);

    int32_t ui2 = layout_external_create_group(layout, WEEKROUND_UI_GROUP_NAME2, 0);
    if (ui2 < 0) ui2 = layout_get_group_id(layout, WEEKROUND_UI_GROUP_NAME2);
    layout_set_group_static_to_screen_by_id(layout, ui2, WEEKROUND_UI_MATRIX);

    // allocate space for all ui items
    layout_external_vertex_create_entries(roundcontext->layout, size);

    // step 2: place all UI elements
    layout_external_vertex_set_entry(
        layout, 0, VERTEX_DRAWABLE, healthbar_get_drawable(roundcontext->healthbar), ui1
    );
    layout_external_vertex_set_entry(
        layout, 1, VERTEX_DRAWABLE, roundstats_get_drawable(roundcontext->roundstats), ui1
    );
    layout_external_vertex_set_entry(
        layout, 2, VERTEX_DRAWABLE, roundcontext->songprogressbar ? songprogressbar_get_drawable(roundcontext->songprogressbar) : NULL, ui1
    );
    layout_external_vertex_set_entry(
        layout, 3, VERTEX_DRAWABLE, countdown_get_drawable(roundcontext->countdown), ui1
    );
    layout_external_vertex_set_entry(
        layout, 4, VERTEX_TEXTSPRITE, roundcontext->songinfo, ui1
    );
    layout_external_vertex_set_entry(
        layout, 5, VERTEX_DRAWABLE, week_gameover_get_drawable(roundcontext->weekgameover), ui2
    );
    layout_external_vertex_set_entry(
        layout, 6, VERTEX_SPRITE, roundcontext->screen_background, ui2
    );
    layout_external_vertex_set_entry(
        layout, 7, VERTEX_DRAWABLE, roundcontext->dialogue ? dialogue_get_drawable(roundcontext->dialogue) : NULL, ui2
    );
    layout_external_vertex_set_entry(
        layout, 8, VERTEX_DRAWABLE,
        roundcontext->autouicosmetics->drawable_self, roundcontext->autouicosmetics->layout_group_id
    );
    layout_external_vertex_set_entry(
        layout, 9, VERTEX_DRAWABLE, roundcontext->messagebox ? messagebox_get_drawable(roundcontext->messagebox) : NULL, ui2
    );

    // step 3: initialize the ui camera
    Modifier* modifier = camera_get_modifier(roundcontext->ui_camera);
    camera_stop(roundcontext->ui_camera);
    modifier->width = pvr_context.screen_width;
    modifier->height = pvr_context.screen_height;

    // step 4: place girlfriend
    if (roundcontext->girlfriend) {
        if (initparams->layout_girlfriend.placeholder_id > 0) {
            week_internal_place_character(
                layout,
                roundcontext->girlfriend,
                &roundcontext->initparams.layout_girlfriend
            );
        } else {
            logger_error("week_place_in_layout() missing layout space for declared girlfriend");
        }
    }

    // step 5: place all player characters & strums
    for (int32_t i = 0, j = UI_SIZE; i < roundcontext->players_size; i++) {
        if (roundcontext->players[i].character) {
            if (i < initparams->layout_characters_size) {
                week_internal_place_character(
                    layout,
                    roundcontext->players[i].character,
                    &initparams->layout_characters[i]
                );
            } else {
                logger_error("week_place_in_layout() missing layout space for declared player[" FMT_I4 "]", i);
            }
        }
        if (roundcontext->players[i].strums) {
            layout_external_vertex_set_entry(
                layout, j++, VERTEX_DRAWABLE, strums_get_drawable(roundcontext->players[i].strums), ui1
            );
        }
    }

    week_ui_set_visibility(roundcontext, true);
    return;
}

static void week_toggle_states(RoundContext roundcontext, GameplayManifest gameplaymanifest) {
    GameplayManifestSong* song = &gameplaymanifest->songs[roundcontext->song_index];

    if (song->has_selected_state_name) {
        for (int32_t i = 0; i < roundcontext->players_size; i++) {
            if (roundcontext->players[i].strums)
                strums_state_toggle(roundcontext->players[i].strums, song->selected_state_name);
            if (roundcontext->players[i].character)
                character_state_toggle(roundcontext->players[i].character, song->selected_state_name);
        }
        healthbar_state_toggle(roundcontext->healthbar, song->selected_state_name);
    }

    int32_t size = song->selected_state_name_per_player_size;
    if (roundcontext->players_size < size) size = roundcontext->players_size;
    for (int32_t i = 0; i < size; i++) {
        const char* state_name = song->selected_state_name_per_player[i];
        if (roundcontext->players[i].strums)
            strums_state_toggle(roundcontext->players[i].strums, state_name);
        if (roundcontext->players[i].character)
            character_state_toggle(roundcontext->players[i].character, state_name);
    }
}

void week_update_bpm(RoundContext roundcontext, float bpm) {
    if (roundcontext->healthbar) healthbar_set_bpm(roundcontext->healthbar, bpm);
    if (roundcontext->countdown) countdown_set_bpm(roundcontext->countdown, bpm);
    if (roundcontext->roundstats) roundstats_tweenkeyframe_set_bpm(roundcontext->roundstats, bpm);
    if (roundcontext->ui_camera) camera_set_bpm(roundcontext->ui_camera, bpm);
    if (roundcontext->layout) layout_set_bpm(roundcontext->layout, bpm);
    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        if (roundcontext->players[i].character) {
            character_set_bpm(roundcontext->players[i].character, bpm);
        }
        if (roundcontext->players[i].strums) {
            strums_set_bpm(roundcontext->players[i].strums, bpm);
        }
    }
    beatwatcher_change_bpm(&WEEK_BEAT_WATCHER, bpm);
    beatwatcher_change_bpm(&WEEK_QUARTER_WATCHER, bpm);

    // logger_log("week_update_bpm() original=" FMT_FLT " new=" FMT_FLT, roundcontext->settings.original_bpm, bpm);

    roundcontext->settings.bpm = bpm;
}

void week_update_speed(RoundContext roundcontext, float64 speed) {
    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        if (roundcontext->players[i].character) {
            character_set_idle_speed(roundcontext->players[i].character, (float)speed);
        }
    }

    if (roundcontext->girlfriend) character_set_idle_speed(roundcontext->girlfriend, (float)speed);

    roundcontext->settings.speed = speed;
}

void week_disable_layout_rollback(RoundContext roundcontext, bool disable) {
    roundcontext->settings.layout_rollback = !disable;
}

void week_override_common_folder(RoundContext roundcontext, const char* custom_common_path) {
    (void)roundcontext;
    fs_override_common_folder(custom_common_path);
}


static int32_t week_round(RoundContext roundcontext, bool from_retry, bool show_dialog) {
    GamepadButtons pressed_buttons = 0x00;
    SongPlayerInfo songinfo = {.timestamp = DOUBLE_NaN, .completed = true};
    InitParams* initparams = &roundcontext->initparams;

    float elapsed;
    bool check_ready = roundcontext->settings.ask_ready;
    bool do_countdown = roundcontext->settings.do_countdown;
    PlayerStats playerstats = NULL;
    Layout layout = roundcontext->layout ? roundcontext->layout : roundcontext->ui_layout;
    Camera camera = layout_get_camera_helper(layout);
    float64 round_duration = roundcontext->round_duration;
    float64 round_end_timestamp;
    bool voices_muted = false;

    if (round_duration < 0.0) round_duration = DOUBLE_Inf;
    layout_resume(layout);

    if (roundcontext->songprogressbar) {
        float64 duration = roundcontext->songplayer ? songplayer_get_duration(roundcontext->songplayer) : round_duration;
        songprogressbar_manual_update_enable(roundcontext->songprogressbar, true);
        songprogressbar_manual_set_position(roundcontext->songprogressbar, 0.0, duration, true);
    }

    if (roundcontext->script) {
        weekscript_notify_timersong(roundcontext->script, 0.0);
        weekscript_notify_beforeready(roundcontext->script, from_retry);
    }
    week_halt(roundcontext, true);

    if (roundcontext->playerstats_index >= 0) {
        playerstats = roundcontext->players[roundcontext->playerstats_index].playerstats;
        roundstats_peek_playerstats(roundcontext->roundstats, 0.0, playerstats);
    }

    while (show_dialog) {
        elapsed = pvrctx_wait_ready();
        pvr_context_reset(&pvr_context);
        week_internal_check_screen_resolution(roundcontext, false);
        beatwatcher_global_set_timestamp_from_kos_timer();

        if (pvr_context_is_offscreen(&pvr_context)) {
            layout_suspend(layout);
            dialogue_suspend(roundcontext->dialogue);

            int32_t decision = week_pause_helper_show(roundcontext->weekpause, roundcontext, -1);
            switch (decision) {
                case 1:
                    return 2; // restart song
                case 2:
                    return 1; // back to weekselector
                case 3:
                    return 3; // back to mainmenu
            }

            layout_resume(layout);
            dialogue_resume(roundcontext->dialogue);
            continue;
        }

        if (roundcontext->script) weekscript_notify_frame(roundcontext->script, elapsed);

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        dialogue_poll(roundcontext->dialogue, elapsed);

        if (dialogue_is_completed(roundcontext->dialogue)) {
            show_dialog = false;
            week_internal_do_antibounce(roundcontext);
        }

        week_halt(roundcontext, true);
    }

    if (check_ready)
        countdown_ready(roundcontext->countdown);
    else if (do_countdown)
        countdown_start(roundcontext->countdown);

    int32_t dettached_controller_index = -1;
    bool back_pressed = false;

    while (check_ready || do_countdown) {
        elapsed = pvrctx_wait_ready();
        pvr_context_reset(&pvr_context);

        week_internal_check_screen_resolution(roundcontext, false);
        beatwatcher_global_set_timestamp_from_kos_timer();

        if (pvr_context_is_offscreen(&pvr_context) || dettached_controller_index >= 0 || back_pressed) {
            int32_t decision = week_pause_helper_show(
                roundcontext->weekpause, roundcontext, dettached_controller_index
            );
            dettached_controller_index = -1;
            switch (decision) {
                case 1:
                    return 2; // restart song
                case 2:
                    return 1; // back to weekselector
                case 3:
                    return 3; // back to mainmenu
            }

            for (int32_t i = 0; i < roundcontext->players_size; i++) {
                Gamepad controller = roundcontext->players[i].controller;
                if (controller) gamepad_clear_buttons(controller);
            }
            continue;
        }

        if (check_ready) {
            bool is_ready = false;
            bool bot_only = true;

            for (int32_t i = 0; i < roundcontext->players_size; i++) {
                Gamepad controller = roundcontext->players[i].controller;
                if (!controller) continue;
                bot_only = false;
                if (gamepad_is_dettached(controller)) {
                    dettached_controller_index = gamepad_get_controller_index(controller);
                    break;
                }
                if (gamepad_has_pressed(controller, WEEKROUND_READY_BUTTONS)) {
                    gamepad_clear_buttons(controller); // antibouce
                    is_ready = true;
                }
                if (gamepad_has_pressed(controller, GAMEPAD_BACK) != 0x00) {
                    is_ready = false;
                    back_pressed = true;
                    break;
                }

                if (gamepad_get_managed_presses(controller, false, &pressed_buttons) && roundcontext->script) {
                    weekscript_notify_buttons(roundcontext->script, i, pressed_buttons);
                }
            }

            if (is_ready || bot_only) {
                check_ready = false;

                if (roundcontext->script) {
                    weekscript_notify_ready(roundcontext->script);
                    week_halt(roundcontext, true);
                }

                if (do_countdown) countdown_start(roundcontext->countdown);
                layout_trigger_camera(layout, WEEKROUND_CAMERA_ROUNDSTART);
            }
        } else if (countdown_has_ended(roundcontext->countdown)) {
            do_countdown = false;
        }

        if (roundcontext->scriptcontext.force_end_flag) {
            if (roundcontext->scriptcontext.force_end_loose_or_win)
                return 1; // give-up and returns to the week selector
            else
                return 0;
        }

        if (roundcontext->script) weekscript_notify_frame(roundcontext->script, elapsed);

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        if (roundcontext->dialogue) dialogue_poll(roundcontext->dialogue, elapsed);
    }

    if (!roundcontext->settings.ask_ready) layout_trigger_camera(layout, WEEKROUND_CAMERA_ROUNDSTART);

    if (roundcontext->songprogressbar) songprogressbar_manual_update_enable(roundcontext->songprogressbar, false);

    if (roundcontext->script) weekscript_notify_aftercountdown(roundcontext->script);
    week_halt(roundcontext, true);

    // start this round!!!!!
    dettached_controller_index = -1;

    if (roundcontext->songplayer) {
        songplayer_play(roundcontext->songplayer, &songinfo);
    } else {
        songinfo.timestamp = 0.0;
        songinfo.completed = true;
    }

    // prepare beatwatchers
    beatwatcher_global_set_timestamp((int64_t)songinfo.timestamp);
    beatwatcher_reset(&WEEK_BEAT_WATCHER, true, roundcontext->settings.bpm);
    beatwatcher_reset(&WEEK_QUARTER_WATCHER, false, roundcontext->settings.bpm);

    bool gameover = false;
    int32_t next_camera_bump = 0;
    DDRKeymon has_reference_ddrkeymon = NULL;
    float64 song_timestamp = 0.0;

    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        if (roundcontext->players[i].ddrkeymon) {
            gamepad_clear_buttons(roundcontext->players[i].controller);
            has_reference_ddrkeymon = roundcontext->players[i].ddrkeymon;
            ddrkeymon_start(roundcontext->players[i].ddrkeymon, songinfo.timestamp);
        }
    }

    round_end_timestamp = timer_ms_gettime64() + round_duration;

    // gameplay logic
    while (timer_ms_gettime64() < round_end_timestamp && !songinfo.completed) {
        // wait for frame
        elapsed = pvrctx_wait_ready();
        pvr_context_reset(&pvr_context);

        week_internal_check_screen_resolution(roundcontext, false);

        // check for pause
        bool paused = false;
        for (int32_t i = 0; i < roundcontext->players_size; i++) {
            if (!roundcontext->players[i].controller) continue;
            if (gamepad_is_dettached(roundcontext->players[i].controller)) {
                dettached_controller_index = gamepad_get_controller_index(
                    roundcontext->players[i].controller
                );
                paused = true;
                break;
            }

            // important: use gamepad_get_last_pressed() to avoid mess up the maple pad or keyboard inputs
            if (gamepad_get_last_pressed(roundcontext->players[i].controller) & (GAMEPAD_START | GAMEPAD_BACK)) {
                paused = true;
                break;
            }
        }

        if (paused || pvr_context_is_offscreen(&pvr_context)) {
            // pause all critical stuff
            round_duration = round_end_timestamp - timer_ms_gettime64();
            layout_suspend(layout);
            if (roundcontext->songplayer) songplayer_pause(roundcontext->songplayer);
            for (int32_t i = 0; i < roundcontext->players_size; i++) {
                if (roundcontext->players[i].ddrkeymon) {
                    ddrkeymon_stop(roundcontext->players[i].ddrkeymon);
                }
            }

            // display pause menu (visible when the screen focus returns)
            int32_t decision = week_pause_helper_show(
                roundcontext->weekpause, roundcontext, dettached_controller_index
            );
            dettached_controller_index = -1;
            switch (decision) {
                case 1:
                    return 2; // restart song
                case 2:
                    return 1; // back to weekselector
                case 3:
                    return 3; // back to mainmenu
            }

            // resume
            round_end_timestamp = timer_ms_gettime64() + round_duration;
            layout_resume(layout);

            if (roundcontext->songplayer) {
                songplayer_seek(roundcontext->songplayer, song_timestamp);
                songplayer_play(roundcontext->songplayer, &songinfo);
            }

            for (int32_t i = 0; i < roundcontext->players_size; i++) {
                if (roundcontext->players[i].ddrkeymon) {
                    gamepad_clear_buttons(roundcontext->players[i].controller);
                    ddrkeymon_start(roundcontext->players[i].ddrkeymon, -song_timestamp);
                    strums_force_key_release(roundcontext->players[i].strums);
                }
            }

            continue;
        }

        if (roundcontext->songplayer) songplayer_poll(roundcontext->songplayer, &songinfo);

        if (has_reference_ddrkeymon)
            song_timestamp = ddrkeymon_peek_timestamp(has_reference_ddrkeymon);
        else
            song_timestamp = songinfo.timestamp;

        // update beatwatchers
        beatwatcher_global_set_timestamp((int64_t)song_timestamp);

        // Important: use only in key checking and/or strum scroll
        float64 song_timestamp2 = song_timestamp;

        //
        // If strums_set_draw_offset() is not used, enable this line
        //
        song_timestamp2 -= SETTINGS.input_offset;

        // check camera/alt-anims events
        week_peek_chart_events(roundcontext, song_timestamp);

        bool has_misses = false, has_hits = false;

        for (int32_t i = 0; i < roundcontext->players_size; i++) {
            switch (roundcontext->players[i].type) {
                case CharacterType_BOT:
                    strums_scroll_auto(roundcontext->players[i].strums, song_timestamp2);
                    conductor_poll(roundcontext->players[i].conductor);
                    if (conductor_has_misses(roundcontext->players[i].conductor)) has_misses = true;
                    if (conductor_has_hits(roundcontext->players[i].conductor)) has_hits = true;
                    break;
                case CharacterType_PLAYER:
                    strums_scroll_full(roundcontext->players[i].strums, song_timestamp2);
                    conductor_poll(roundcontext->players[i].conductor);
                    if (gamepad_get_managed_presses(roundcontext->players[i].controller, false, &pressed_buttons)) {
                        if (roundcontext->script) weekscript_notify_buttons(roundcontext->script, i, pressed_buttons);
                    }
                    if (conductor_has_misses(roundcontext->players[i].conductor)) has_misses = true;
                    if (conductor_has_hits(roundcontext->players[i].conductor)) has_hits = true;
                    break;
                    CASE_UNUSED(CharacterType_ACTOR)
            }
        }

        if (roundcontext->script) {
            weekscript_notify_after_strum_scroll(roundcontext->script);

            if (roundcontext->scriptcontext.halt_flag) {
                week_halt(roundcontext, false);

                for (int32_t i = 0; i < roundcontext->players_size; i++) {
                    if (roundcontext->players[i].ddrkeymon) {
                        gamepad_clear_buttons(roundcontext->players[i].controller);
                        ddrkeymon_clear(roundcontext->players[i].ddrkeymon);
                    }
                }
            }
        }

        if (playerstats) {
            rankingcounter_peek_ranking(roundcontext->rankingcounter, playerstats);
            if (streakcounter_peek_streak(roundcontext->streakcounter, playerstats)) {
                if (roundcontext->girlfriend && roundcontext->settings.girlfriend_cry) {
                    character_play_extra(roundcontext->girlfriend, FUNKIN_GIRLFRIEND_COMBOBREAK, false);
                }
            }
            roundstats_peek_playerstats(roundcontext->roundstats, songinfo.timestamp, playerstats);
        }

        healthwatcher_balance(roundcontext->healthwatcher, roundcontext->healthbar);

        if (healthwatcher_has_deads(roundcontext->healthwatcher, true) > 0) {
            gameover = true;
            break;
        }

        if (roundcontext->songplayer) {
            if (has_misses && !has_hits && !voices_muted) {
                songplayer_mute_track(roundcontext->songplayer, true, true);
                voices_muted = true;
            } else if (has_hits && voices_muted) {
                songplayer_mute_track(roundcontext->songplayer, true, false);
                voices_muted = false;
            }
        }


        if (beatwatcher_poll(&WEEK_BEAT_WATCHER)) {
            // bump UI
            if (WEEK_BEAT_WATCHER.count > next_camera_bump) {
                next_camera_bump += 4;
                if (roundcontext->settings.camera_bumping) {
                    camera_animate(camera, WEEK_BEAT_WATCHER.since);
                    camera_slide(roundcontext->ui_camera, FLOAT_NaN, FLOAT_NaN, 1.05f, FLOAT_NaN, FLOAT_NaN, 1.0f);
                }
            }

            // notify script
            if (roundcontext->script) {
                weekscript_notify_beat(
                    roundcontext->script, WEEK_BEAT_WATCHER.count, WEEK_BEAT_WATCHER.since
                );
            }
        }

        if (beatwatcher_poll(&WEEK_QUARTER_WATCHER) && roundcontext->script) {
            weekscript_notify_quarter(
                roundcontext->script, WEEK_QUARTER_WATCHER.count, WEEK_QUARTER_WATCHER.since
            );
        }

        // animate camera
        camera_animate(roundcontext->ui_camera, elapsed);

        // flush modifier
        camera_apply(roundcontext->ui_camera, NULL);
        sh4matrix_copy_to(WEEKROUND_UI_MATRIX, WEEKROUND_UI_MATRIX_CAMERA);
        camera_apply_offset(roundcontext->ui_camera, WEEKROUND_UI_MATRIX_CAMERA);
        sh4matrix_apply_modifier(WEEKROUND_UI_MATRIX_CAMERA, camera_get_modifier(roundcontext->ui_camera));

        if (roundcontext->script) {
            weekscript_notify_timersong(roundcontext->script, song_timestamp);
            weekscript_notify_frame(roundcontext->script, elapsed);
        }
        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        if (roundcontext->dialogue) dialogue_poll(roundcontext->dialogue, elapsed);

        if (roundcontext->scriptcontext.halt_flag) {
            week_halt(roundcontext, false);

            for (int32_t i = 0; i < roundcontext->players_size; i++) {
                if (roundcontext->players[i].ddrkeymon) {
                    gamepad_clear_buttons(roundcontext->players[i].controller);
                    ddrkeymon_clear(roundcontext->players[i].ddrkeymon);
                }
            }
        }

        if (roundcontext->scriptcontext.force_end_flag) {
            if (roundcontext->songplayer) songplayer_pause(roundcontext->songplayer);
            if (roundcontext->scriptcontext.force_end_loose_or_win) {
                return 1; // give-up and returns to the week selector
            } else {
                return 0; // interrupt the gameplay
            }
        }
    }

    // stop all ddrkeymon instances
    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        if (roundcontext->players[i].ddrkeymon) {
            ddrkeymon_stop(roundcontext->players[i].ddrkeymon);
        }
    }

    // notify the script about the current round result and halt (if necessary)
    if (roundcontext->songplayer) songplayer_pause(roundcontext->songplayer);
    layout_trigger_camera(layout, WEEKROUND_CAMERA_ROUNDEND);
    if (roundcontext->script) {
        weekscript_notify_roundend(roundcontext->script, gameover);
        week_halt(roundcontext, true);
    }
    week_ui_set_visibility(roundcontext, false);


    if (gameover) {
        float64 duration = roundcontext->round_duration;
        if (roundcontext->songplayer) {
            if (duration < 0.0) duration = songplayer_get_duration(roundcontext->songplayer);
            songplayer_pause(roundcontext->songplayer);
        }

        week_gameover_display(
            roundcontext->weekgameover,
            song_timestamp,
            duration,
            playerstats,
            initparams->weekinfo,
            initparams->difficult
        );

        // ask for player decision
        roundcontext->scriptcontext.force_end_flag = false;
        int32_t decision = week_gameover_helper_ask_to_player(roundcontext->weekgameover, roundcontext);
        const char* song_difficult = week_gameover_get_difficult(roundcontext->weekgameover);
        week_gameover_hide(roundcontext->weekgameover);

        // notify script and wait (if necessary)
        if (roundcontext->script) {
            weekscript_notify_gameoverended(roundcontext->script);
            week_halt(roundcontext, true);
        }

        roundcontext->song_difficult = song_difficult;

        return decision;
    }

    return 0; // round win
}


void week_halt(RoundContext roundcontext, bool peek_global_beatwatcher) {
    if (!roundcontext->scriptcontext.halt_flag) return;

    GamepadButtons preesed = 0x00;
    Layout layout = roundcontext->layout ? roundcontext->layout : roundcontext->ui_layout;

    logger_log("week_halt() waiting for script signal...");

    if (peek_global_beatwatcher) beatwatcher_global_set_timestamp_from_kos_timer();

    while (roundcontext->scriptcontext.halt_flag) {
        float elapsed = pvrctx_wait_ready();
        pvr_context_reset(&pvr_context);

        week_internal_check_screen_resolution(roundcontext, false);

        for (int32_t i = 0; i < roundcontext->players_size; i++) {
            Gamepad controller = roundcontext->players[i].controller;
            if (controller && gamepad_get_managed_presses(controller, true, &preesed) && roundcontext->script) {
                weekscript_notify_buttons(roundcontext->script, i, preesed);
            }
        }

        if (roundcontext->script) weekscript_notify_frame(roundcontext->script, elapsed);

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        if (roundcontext->dialogue) dialogue_poll(roundcontext->dialogue, elapsed);

        if (peek_global_beatwatcher) beatwatcher_global_set_timestamp_from_kos_timer();

        if (roundcontext->scriptcontext.force_end_flag) {
            logger_log("week_halt() wait interrupted because week_end() was called");
            return;
        }
    }

    week_internal_do_antibounce(roundcontext);

    logger_log("week_halt() wait done");
}

static void week_peek_chart_events(RoundContext roundcontext, float64 timestamp) {
    int32_t i = roundcontext->events_peek_index;

    for (; i < roundcontext->events_size; i++) {
        if (timestamp < roundcontext->events[i].timestamp) break;
        switch (roundcontext->events[i].command) {
            case ChartEvent_CAMERA_OPPONENT:
                week_camera_focus_guess(roundcontext, roundcontext->settings.camera_name_opponent, -1);
                break;
            case ChartEvent_CAMERA_PLAYER:
                week_camera_focus_guess(roundcontext, roundcontext->settings.camera_name_player, -1);
                break;
            case ChartEvent_CHANGE_BPM:
                week_update_bpm(roundcontext, (float)roundcontext->events[i].parameter);
                break;
            case ChartEvent_ALT_ANIM_OPPONENT:
                for (int32_t j = 0; j < roundcontext->players_size; j++) {
                    if (roundcontext->players[j].is_opponent) {
                        character_use_alternate_sing_animations(
                            roundcontext->players[j].character, roundcontext->events[i].parameter
                        );
                    }
                }
                break;
            case ChartEvent_ALT_ANIM_PLAYER:
                for (int32_t j = 0; j < roundcontext->players_size; j++) {
                    if (roundcontext->players[j].type == CharacterType_PLAYER) {
                        character_use_alternate_sing_animations(
                            roundcontext->players[j].character, roundcontext->events[i].parameter
                        );
                    }
                }
                break;
            case ChartEvent_UNKNOWN_NOTE:
                if (roundcontext->script) {
                    weekscript_notify_unknownnote(
                        roundcontext->script,
                        roundcontext->events[i].is_player_or_opponent ? 1 : 0,
                        roundcontext->events[i].timestamp,
                        (int32_t)roundcontext->events[i].parameter,
                        roundcontext->events[i].parameter2,
                        roundcontext->events[i].parameter3
                    );
                }
                break;
                CASE_UNUSED(ChartEvent_NONE)
        }
    }

    if (i != roundcontext->events_peek_index) roundcontext->events_peek_index = i;
}


static void week_check_directives_round(RoundContext roundcontext, bool completed) {
    foreach (ModifiedDirective*, directive_info, LINKEDLIST_ITERATOR, roundcontext->scriptcontext.directives) {
        if (directive_info->completed_week) continue;

        if (completed || !directive_info->completed_round) {
            if (directive_info->create)
                funkinsave_create_unlock_directive(directive_info->name, directive_info->value);
            else
                funkinsave_delete_unlock_directive(directive_info->name);
        }

        // remove this item from the list
        linkedlist_remove_item(roundcontext->scriptcontext.directives, directive_info);
        free_chk(directive_info->name);
        free_chk(directive_info);
    }
}

static void week_check_directives_week(RoundContext roundcontext, bool completed) {
    foreach (ModifiedDirective*, directive_info, LINKEDLIST_ITERATOR, roundcontext->scriptcontext.directives) {
        if (completed || !directive_info->completed_week) {
            if (directive_info->create)
                funkinsave_create_unlock_directive(directive_info->name, directive_info->value);
            else
                funkinsave_delete_unlock_directive(directive_info->name);
        }

        // remove this item from the list
        linkedlist_remove_item(roundcontext->scriptcontext.directives, directive_info);
        free_chk(directive_info->name);
        free_chk(directive_info);
        roundcontext->has_directive_changes = true;
    }
}

static void week_change_scroll_direction(RoundContext roundcontext) {
    int32_t direction;
    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        if (!roundcontext->players[i].strums) continue;

        if (roundcontext->players[i].is_vertical)
            direction = SETTINGS.inverse_strum_scroll ? ScrollDirection_DOWNSCROLL : ScrollDirection_UPSCROLL;
        else
            direction = SETTINGS.inverse_strum_scroll ? ScrollDirection_RIGHTSCROLL : ScrollDirection_LEFTSCROLL;

        strums_set_scroll_direction(roundcontext->players[i].strums, direction);
        notepool_change_scroll_direction(roundcontext->players[i].notepool, direction);
    }
}

void week_unlockdirective_create(RoundContext roundcontext, const char* name, bool completed_round, bool completed_week, float64 value) {
    if (string_is_empty(name)) return;

    foreach (ModifiedDirective*, directive_info, LINKEDLIST_ITERATOR, roundcontext->scriptcontext.directives) {
        if (string_equals(directive_info->name, name)) {
            directive_info->completed_round = completed_round;
            directive_info->completed_week = completed_week;
            directive_info->value = value;
            directive_info->create = true;
            return;
        }
    }

    ModifiedDirective* new_directive = malloc_chk(sizeof(ModifiedDirective));
    malloc_assert(new_directive, ModifiedDirective);

    *new_directive = (ModifiedDirective){
        .name = string_duplicate(name),
        .completed_round = completed_round,
        .completed_week = completed_week,
        .value = value,
        .create = true
    };

    linkedlist_add_item(roundcontext->scriptcontext.directives, new_directive);
}

void week_unlockdirective_remove(RoundContext roundcontext, const char* name, bool completed_round, bool completed_week) {
    foreach (ModifiedDirective*, directive_info, LINKEDLIST_ITERATOR, roundcontext->scriptcontext.directives) {
        if (string_equals(directive_info->name, name)) {
            directive_info->completed_round = completed_round;
            directive_info->completed_week = completed_week;
            directive_info->create = false;
        }
    }

    ModifiedDirective* delete_directive = malloc_chk(sizeof(ModifiedDirective));
    malloc_assert(delete_directive, ModifiedDirective);

    *delete_directive = (ModifiedDirective){
        name : string_duplicate(name),
        completed_round : completed_round,
        completed_week : completed_week,
        value : 0x00,
        create : false
    };

    linkedlist_add_item(roundcontext->scriptcontext.directives, delete_directive);
}

float64 week_unlockdirective_get(RoundContext roundcontext, const char* name) {
    (void)roundcontext;

    float64 value = 0.0;
    funkinsave_read_unlock_directive(name, &value);
    return value;
}

bool week_unlockdirective_has(RoundContext roundcontext, const char* name) {
    (void)roundcontext;
    return funkinsave_contains_unlock_directive(name);
}

size_t week_storage_get(RoundContext roundcontext, const char* name, uint8_t** out_data) {
    return funkinsave_storage_get(roundcontext->initparams.weekinfo->name, name, out_data);
}

bool week_storage_set(RoundContext roundcontext, const char* name, const uint8_t* data, size_t data_size) {
    bool ret = funkinsave_storage_set(roundcontext->initparams.weekinfo->name, name, data, data_size);
    if (ret) roundcontext->has_directive_changes = true;
    return ret;
}

SongPlayer week_get_songplayer(RoundContext roundcontext) {
    return roundcontext->songplayer;
}

Layout week_get_stage_layout(RoundContext roundcontext) {
    return roundcontext->layout == roundcontext->ui_layout ? NULL : roundcontext->layout;
}

Layout week_ui_get_layout(RoundContext roundcontext) {
    return roundcontext->ui_layout;
}

Camera week_ui_get_camera(RoundContext roundcontext) {
    return roundcontext->ui_camera;
}

void week_set_halt(RoundContext roundcontext, bool halt) {
    roundcontext->scriptcontext.halt_flag = halt;
}

void week_disable_week_end_results(RoundContext roundcontext, bool disable) {
    roundcontext->scriptcontext.no_week_end_result_screen = disable;
}

void week_disable_girlfriend_cry(RoundContext roundcontext, bool disable) {
    roundcontext->settings.girlfriend_cry = !disable;
}

void week_disable_ask_ready(RoundContext roundcontext, bool disable) {
    roundcontext->settings.ask_ready = !disable;
}

void week_disable_countdown(RoundContext roundcontext, bool disable) {
    roundcontext->settings.do_countdown = !disable;
}

void week_disable_camera_bumping(RoundContext roundcontext, bool disable) {
    roundcontext->settings.camera_bumping = !disable;
}


Character week_get_girlfriend(RoundContext roundcontext) {
    return roundcontext->girlfriend;
}

int32_t week_get_character_count(RoundContext roundcontext) {
    return roundcontext->players_size;
}

Character week_get_character(RoundContext roundcontext, int32_t index) {
    if (index < 0 || index >= roundcontext->players_size) return NULL;
    return roundcontext->players[index].character;
}

MessageBox week_get_messagebox(RoundContext roundcontext) {
    return roundcontext->messagebox;
}

int32_t week_ui_get_strums_count(RoundContext roundcontext) {
    return roundcontext->initparams.layout_strums_size;
}

TextSprite week_ui_get_round_textsprite(RoundContext roundcontext) {
    return roundcontext->songinfo;
}

void week_ui_set_visibility(RoundContext roundcontext, bool visible) {
    layout_set_group_visibility(roundcontext->layout, WEEKROUND_UI_GROUP_NAME, visible);
}

void week_get_current_chart_info(RoundContext roundcontext, float* bpm, float64* speed) {
    *bpm = roundcontext->settings.original_bpm;
    *speed = roundcontext->settings.original_speed;
}

void week_get_current_song_info(RoundContext roundcontext, const char** name, const char** difficult, int32_t* index) {
    *name = roundcontext->initparams.gameplaymanifest->songs[roundcontext->song_index].name;
    *difficult = roundcontext->song_difficult;
    *index = roundcontext->song_index;
}

void week_change_character_camera_name(RoundContext roundcontext, bool opponent_or_player, const char* new_name) {
    Settings* settings = &roundcontext->settings;

    if (opponent_or_player) {
        if (/* pointer equals */ settings->camera_name_opponent != WEEKROUND_CAMERA_OPONNENT) free_chk(settings->camera_name_opponent);
        settings->camera_name_opponent = new_name == NULL ? (char*)WEEKROUND_CAMERA_OPONNENT : string_duplicate(new_name);
    } else {
        if (/* pointer equals */ settings->camera_name_player != WEEKROUND_CAMERA_PLAYER) free_chk(settings->camera_name_player);
        settings->camera_name_player = new_name == NULL ? (char*)WEEKROUND_CAMERA_PLAYER : string_duplicate(new_name);
    }
}

void week_enable_credits_on_completed(RoundContext roundcontext) {
    roundcontext->settings.show_credits = true;
}

void week_end(RoundContext roundcontext, bool round_or_week, bool loose_or_win) {
    roundcontext->scriptcontext.force_end_flag = true;
    roundcontext->scriptcontext.force_end_round_or_week = round_or_week;
    roundcontext->scriptcontext.force_end_loose_or_win = loose_or_win;
}

Dialogue week_get_dialogue(RoundContext roundcontext) {
    return roundcontext->dialogue;
}

/*void week_set_ui_shader(RoundContext roundcontext, PSShader psshader) {
    Layout layout = roundcontext->layout ? roundcontext->layout : roundcontext->ui_layout;
    layout_set_group_shader(layout, WEEKROUND_UI_GROUP_NAME, psshader);
}*/

Conductor week_get_conductor(RoundContext roundcontext, int32_t character_index) {
    if (character_index < 0 || character_index >= roundcontext->players_size) return NULL;
    return roundcontext->players[character_index].conductor;
}

HealthWatcher week_get_healthwatcher(RoundContext roundcontext) {
    return roundcontext->healthwatcher;
}

MissNoteFX week_get_missnotefx(RoundContext roundcontext) {
    return roundcontext->missnotefx;
}

PlayerStats week_get_playerstats(RoundContext roundcontext, int32_t character_index) {
    if (character_index < 0 || character_index >= roundcontext->players_size) return NULL;
    return roundcontext->players[character_index].playerstats;
}

void week_rebuild_ui(RoundContext roundcontext) {
    week_init_ui_cosmetics(roundcontext);
}

Countdown week_ui_get_countdown(RoundContext roundcontext) {
    return roundcontext->countdown;
}

Healthbar week_ui_get_healthbar(RoundContext roundcontext) {
    return roundcontext->healthbar;
}

RankingCounter week_ui_get_rankingcounter(RoundContext roundcontext) {
    return roundcontext->rankingcounter;
}

RoundStats week_ui_get_roundstats(RoundContext roundcontext) {
    return roundcontext->roundstats;
}

SongProgressbar week_ui_get_songprogressbar(RoundContext roundcontext) {
    return roundcontext->songprogressbar;
}

StreakCounter week_ui_get_streakcounter(RoundContext roundcontext) {
    return roundcontext->streakcounter;
}

Strums week_ui_get_strums(RoundContext roundcontext, int32_t strums_id) {
    InitParams* initparams = &roundcontext->initparams;
    GameplayManifest gameplaymanifest = initparams->gameplaymanifest;
    int32_t song_index = roundcontext->song_index;

    GameplayManifestPlayer* players = gameplaymanifest->default_->players;
    int32_t players_size = gameplaymanifest->default_->players_size;
    if (gameplaymanifest->songs[song_index].has_players) {
        players = gameplaymanifest->songs[song_index].players;
        players_size = gameplaymanifest->songs[song_index].players_size;
    }

    for (int32_t i = 0; i < players_size; i++) {
        // obtain the position in the UI layout
        int32_t layout_strums_id = players[i].layout_strums_id < 0 ? i : players[i].layout_strums_id;

        if (layout_strums_id >= initparams->layout_strums_size) layout_strums_id = -1;
        if (layout_strums_id < 0 || roundcontext->players[i].type == CharacterType_ACTOR) continue;

        if (layout_strums_id == strums_id) return roundcontext->players[i].strums;
    }

    // unable to guess the correct player's strums
    return NULL;
}

void week_set_gameover_option(RoundContext roundcontext, int32_t opt, float nro, const char* str) {
    if (!roundcontext->weekgameover) return;
    week_gameover_set_option(roundcontext->weekgameover, opt, nro, str);
}

void week_get_accumulated_stats(RoundContext roundcontext, WeekResult_Stats* stats) {
    if (!roundcontext->weekresult) return;
    week_result_get_accumulated_stats(roundcontext->weekresult, stats);
}

Layout week_get_layout_of(RoundContext roundcontext, char g_p_r) {
    switch (g_p_r) {
        case 'g':
            if (!roundcontext->weekgameover) break;
            return week_gameover_get_layout(roundcontext->weekgameover);
        case 'p':
            if (!roundcontext->weekpause) break;
            return week_pause_get_layout(roundcontext->weekpause);
        case 'r':
            if (!roundcontext->weekresult) break;
            return week_result_get_layout(roundcontext->weekresult);
    }

    return NULL;
}

void week_set_pause_background_music(RoundContext roundcontext, const char* filename) {
    if (roundcontext->weekpause) week_pause_change_background_music(roundcontext->weekpause, filename);
}


char* week_internal_concat_suffix(const char* name, int32_t number_suffix) {
    char digits[16];
    int written = snprintf(digits, sizeof(digits), FMT_I4, number_suffix);
    digits[written] = '\0';

    return string_concat(2, name, digits);
}

static LayoutAttachedValue week_internal_read_value_prefix(Layout layout, const char* name, int32_t number_suffix, AttachedValueType type, LayoutAttachedValue default_value) {
    char* temp = week_internal_concat_suffix(name, number_suffix);
    LayoutAttachedValue value = layout_get_attached_value(layout, temp, type, default_value);
    free_chk(temp);
    return value;
}

static LayoutPlaceholder* week_internal_read_placeholder(Layout layout, const char* prefix_name, int32_t number_suffix) {
    char* placeholder_name = week_internal_concat_suffix(prefix_name, number_suffix);
    LayoutPlaceholder* placeholder = layout_get_placeholder(layout, placeholder_name);
    if (!placeholder) logger_error("week_internal_read_placeholder() missing layout placeholder: %s", placeholder_name);
    free_chk(placeholder_name);
    return placeholder;
}

/*static void week_internal_parse_character_placeholder(InitParams* initparams, int32_t index, LayoutPlaceholder* placeholder) {
    initparams->layout_characters[index] = (LayoutCharacter){
        .align_vertical = ALIGN_START,
        .align_horizontal = ALIGN_START,
        .reference_width = -1.0f,
        .reference_height = -1.0f,
        .x = 0.0f,
        .y = 0.0f,
        .z = 0.0f
    };

    if (!placeholder) return;

    initparams->layout_characters[index].x = placeholder->x;
    initparams->layout_characters[index].y = placeholder->y;
    initparams->layout_characters[index].z = placeholder->z;
    initparams->layout_characters[index].align_vertical = placeholder->align_vertical;
    initparams->layout_characters[index].align_horizontal = placeholder->align_horizontal;
    initparams->layout_characters[index].reference_width = placeholder->width;
    initparams->layout_characters[index].reference_height = placeholder->height;
}*/

static void week_internal_place_character(Layout layout, Character character, LayoutCharacter* layout_character) {
    character_set_scale(character, layout_character->scale);

    character_set_draw_location(character, layout_character->x, layout_character->y);
    character_set_z_index(character, layout_character->z);

    character_update_reference_size(
        character, layout_character->reference_width, layout_character->reference_height
    );

    bool enable = layout_character->reference_width >= 0.0f || layout_character->reference_height >= 0.0f;
    character_enable_reference_size(character, enable);

    character_set_draw_align(
        character, layout_character->align_vertical, layout_character->align_horizontal
    );

    layout_set_placeholder_drawable_by_id(
        layout, layout_character->placeholder_id, character_get_drawable(character)
    );

    character_state_toggle(character, NULL);
}

static GameplayManifest week_internal_load_gameplay_manifest(const char* src) {
    GameplayManifest gameplaymanifest = gameplaymanifest_init(src);

    if (!gameplaymanifest) {
        logger_error("week_internal_load_gameplay_manifest() missing file %s", src);
        return NULL;
    }

    if (gameplaymanifest->songs_size < 1) {
        gameplaymanifest_destroy(&gameplaymanifest);
        logger_error("week_internal_load_gameplay_manifest() no songs defined, goodbye. File: %s", src);
        return NULL;
    }

    return gameplaymanifest;
}

void week_camera_focus_guess(RoundContext roundcontext, const char* target_name, int32_t character_index) {
    Layout layout = roundcontext->layout ? roundcontext->layout : roundcontext->ui_layout;
    Camera camera = layout_get_camera_helper(layout);

    if (character_index >= 0) {
        char* name = week_internal_concat_suffix(WEEKROUND_CAMERA_CHARACTER, character_index);
        bool found = camera_from_layout(camera, layout, name);
        free_chk(name);
        if (found) return;
    }

    camera_from_layout(camera, layout, target_name);
}

static void week_internal_reset_players_and_girlfriend(RoundContext roundcontext) {
    float64 speed = roundcontext->settings.original_speed;
    week_change_scroll_direction(roundcontext);
    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        if (roundcontext->players[i].strums) {
            strums_reset(roundcontext->players[i].strums, speed, NULL);
            if (roundcontext->players[i].character)
                character_reset(roundcontext->players[i].character);
            if (roundcontext->players[i].conductor)
                conductor_poll_reset(roundcontext->players[i].conductor);
            if (roundcontext->players[i].ddrkeymon)
                ddrkeymon_stop(roundcontext->players[i].ddrkeymon);
            if (roundcontext->players[i].playerstats) {
                playerstats_reset(roundcontext->players[i].playerstats);
                playerstats_enable_health_recover(
                    roundcontext->players[i].playerstats, roundcontext->players[i].can_recover
                );
            }
        }
    }

    missnotefx_disable(roundcontext->missnotefx, false);

    if (roundcontext->girlfriend) character_reset(roundcontext->girlfriend);
}

static void week_internal_pick_counters_values_from_layout(RoundContext roundcontext) {
    UIParams* ui = &roundcontext->initparams.ui;
    Layout layout = roundcontext->layout ? roundcontext->layout : roundcontext->ui_layout;

    // streakcounter values
    ui->streakcounter_comboheight = layout_get_attached_value_as_float(
        layout, "ui_streakcounter_comboHeight", ui->streakcounter_comboheight
    );
    ui->streakcounter_numbergap = layout_get_attached_value_as_float(
        layout, "ui_streakcounter_numberGap", ui->streakcounter_numbergap
    );
    ui->streakcounter_delay = layout_get_attached_value_as_float(
        layout, "ui_streakcounter_delay", ui->streakcounter_delay
    );

    // ranking counter values
    ui->rankingcounter_percentonly = layout_get_attached_value_boolean(
        layout, "ui_rankingcounter_accuracy_percent", ui->rankingcounter_percentonly
    );
}

static LayoutPlaceholder* week_internal_read_placeholder_counter(Layout layout, const char* name, bool warn) {
    LayoutPlaceholder* placeholder = layout_get_placeholder(layout, name);
    if (warn && !placeholder) logger_error("week_internal_read_placeholder_counter() missing layout '%s' placeholder", name);
    return placeholder;
}

static void week_internal_check_screen_resolution(RoundContext roundcontext, bool force) {
    float width = pvr_context.screen_width;
    float height = pvr_context.screen_height;
    InitParams* initparams = &roundcontext->initparams;

    if (!force && roundcontext->resolution_changes == pvr_context.resolution_changes) return;
    roundcontext->resolution_changes = pvr_context.resolution_changes;

    sprite_set_draw_size(roundcontext->screen_background, width, height);

    float scale_x = width / initparams->ui_layout_width;
    float scale_y = height / initparams->ui_layout_height;

    float scale = math2d_min_float(scale_x, scale_y);
    float translate_x = (width - initparams->ui_layout_width * scale) / 2.0f;
    float translate_y = (height - initparams->ui_layout_height * scale) / 2.0f;

    sh4matrix_clear(WEEKROUND_UI_MATRIX);
    sh4matrix_translate(WEEKROUND_UI_MATRIX, translate_x, translate_y);
    sh4matrix_scale(WEEKROUND_UI_MATRIX, scale, scale);

    sh4matrix_copy_to(WEEKROUND_UI_MATRIX, WEEKROUND_UI_MATRIX_CAMERA);
}

static void week_internal_do_antibounce(RoundContext roundcontext) {
    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        if (roundcontext->players[i].controller != NULL) {
            gamepad_clear_buttons(roundcontext->players[i].controller); // antibounce
        }
    }
}

static void* week_internal_savemanager_should_show(void* param) {
    return (void*)savemanager_should_show((bool)param);
}
