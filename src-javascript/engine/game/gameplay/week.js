"use strict";

const FUNKIN_GIRLFRIEND_COMBOBREAK = "sad";
const FUNKIN_PLAYER_DIES = "dies";// the player has lost
const FUNKIN_PLAYER_SCARED = "scared";// the other players reacts
const FUNKIN_OPPONENT_VICTORY = "victory";// the opponents "laughs"
const FUNKIN_PLAYER_RETRY = "retry";
const FUNKIN_PLAYER_GIVEUP = "giveup";

const WEEKROUND_READY_BUTTONS = GAMEPAD_A | GAMEPAD_START;
const WEEKROUND_CAMERA_GIRLFRIEND = "camera_girlfriend";
const WEEKROUND_CAMERA_CHARACTER = "camera_character";
const WEEKROUND_CAMERA_PLAYER = "camera_player";
const WEEKROUND_CAMERA_OPONNENT = "camera_opponent";
const WEEKROUND_CAMERA_ROUNDSTART = "camera_roundStart";
const WEEKROUND_CAMERA_ROUNDEND = "camera_roundEnd";
const WEEKROUND_UI_MATRIX = new Float32Array(SH4MATRIX_SIZE);
const WEEKROUND_UI_MATRIX_CAMERA = new Float32Array(SH4MATRIX_SIZE);
const WEEKROUND_UI_GROUP_NAME = "______UI______";// for internally use only
const WEEKROUND_UI_GROUP_NAME2 = "______UI2______";// for internally use only

const UI_RANKINGCOUNTER_MODEL = "/assets/common/image/week-round/ranking.xml";
const UI_STREAKCOUNTER_MODEL = "/assets/common/font/numbers.xml";
const UI_COUNTDOWN_MODEL = "/assets/common/image/week-round/countdown.xml";
const UI_ANIMLIST = "/assets/common/anims/week-round.xml";

const UI_LAYOUT_WIDESCREEN = "/assets/common/image/week-round/ui.xml";
const UI_LAYOUT_DREAMCAST = "/assets/common/image/week-round/ui~dreamcast.xml";
const UI_STRUMS_LAYOUT_PLACEHOLDER = { x: 0, y: 0, z: 100, width: 300, height: 54 };
const UI_STUB_LAYOUT_PLACEHOLDER = { x: 0, y: 0, z: -1, width: 100, height: 100 };
const UI_SONGINFO_FORMAT = "$s $s[$s] {kdy $s}";
const UI_SONGINFO_ALT_SUFFIX = "(alt) ";

const WEEKROUND_CHARACTER_PREFIX = "character_";

const WEEK_BEAT_WATCHER = { count: 0, since: 0 };
const WEEK_QUARTER_WATCHER = { count: 0, since: 0 };



/** @readonly @enum {number} */
const CHARACTERTYPE = {
    ACTOR: 1,
    BOT: 2,
    PLAYER: 3
};

/**
 * @typedef {object} ScriptContext
 * @property {bool} halt_flag
 * @property {object} directives
 * @property {bool} force_end_flag
 * @property {bool} force_end_round_or_week
 * @property {bool} force_end_loose_or_win
 * @property {bool} no_week_end_result_screen
 */

/**
 * @typedef {object} PlayerStruct
 * @property {object} conductor
 * @property {object} character
 * @property {object} playerstats
 * @property {CHARACTERTYPE} type
 * @property {object} strums
 * @property {DDRKeymon} ddrkeymon
 * @property {DDRKeysFIFO} ddrkeys_fifo
 * @property {GamepadKDY} controller
 * @property {object} notepool
 *
 * @property {bool} is_vertical
 * @property {bool} is_opponent
 * @property {bool} can_die
 * @property {bool} can_recover
 */


/**
 * @typedef {object} Settings
 *
 * @property {bool} girlfriend_cry
 * @property {bool} ask_ready
 * @property {bool} do_countdown
 * @property {bool} camera_bumping
 * 
 * @property {number} bpm
 * @property {number} speed
 * 
 * @property {number} original_bpm
 * @property {number} original_speed
 * 
 * @property {string} camera_name_opponent
 * @property {string} camera_name_player
 * 
 * @property {bool} layout_rollback
 * @property {bool} show_credits
 * @property {bool} no_healthbar
 * 
 */

/**
 * @typedef {object} UIParams
 * @property {number} healthbar_x
 * @property {number} healthbar_y
 * @property {number} healthbar_z
 * @property {number} healthbar_length
 * @property {number} healthbar_dimmen
 * @property {number} healthbar_border
 * @property {number} healthbar_iconoverlap
 * @property {number} healthbar_warnheight
 * @property {number} healthbar_lockheight
 * @property {bool} healthbar_is_vertical
 * @property {bool} healthbar_nowarns
 * @property {bool} roundstats_hide
 * @property {number} roundstats_x
 * @property {number} roundstats_y
 * @property {number} roundstats_z
 * @property {number} roundstats_size
 * @property {number} roundstats_fontcolor
 * @property {number} streakcounter_comboheight
 * @property {number} streakcounter_numbergap
 * @property {number} streakcounter_delay
 * @property {bool} rankingcounter_percentonly
 * @property {number} songinfo_x
 * @property {number} songinfo_y
 * @property {number} songinfo_z
 * @property {number} songinfo_maxwidth
 * @property {number} songinfo_maxheight
 * @property {number} songinfo_alignvertical
 * @property {number} songinfo_alignhorinzontal
 * @property {number} songinfo_fontsize
 * @property {number} songinfo_fontcolor
 * @property {number} countdown_height
 * @property {number} songprogressbar_x
 * @property {number} songprogressbar_y
 * @property {number} songprogressbar_z
 * @property {number} songprogressbar_width
 * @property {number} songprogressbar_height
 * @property {number} songprogressbar_align
 * @property {number} songprogressbar_bordersize
 * @property {number} songprogressbar_fontsize
 * @property {number} songprogressbar_fontbordersize
 * @property {number} songprogressbar_isvertical
 * @property {number} songprogressbar_showtime
 * @property {number} songprogressbar_colorrgba8_text
 * @property {number} songprogressbar_colorrgba8_background
 * @property {number} songprogressbar_colorrgba8_barback
 * @property {number} songprogressbar_colorrgba8_barfront
 */

/**
 * @typedef {object} InitParams
 * @property {LayoutStrum[]} layout_strums
 * @property {number} layout_strums_size
 * 
 * @property {LayoutCharacter} layout_girlfriend
 * @property {LayoutCharacter[]} layout_characters
 * @property {number} layout_characters_size
 *
 * @property {object} font
 * @property {number} ui_layout_width
 * @property {number} ui_layout_height
 * @property {object} animlist
 * @property {object} gameplaymanifest
 * 
 * @property {UIParams} ui
 * 
 * @property {bool} alt_tracks
 * @property {string} difficult
 * @property {string} default_boyfriend
 * @property {string} default_girlfriend
 * @property {bool} single_song
 * 
 * @property {object} weekinfo
 * 
 */

/** 
 * @typedef {object} LayoutStrum
 * @property {number} x
 * @property {number} y
 * @property {number} z
 * @property {number} length
 * @property {number} gap
 * @property {bool} keep_marker_scale
 * @property {number} marker_dimmen
 * @property {number} invdimmen
 * @property {number} sustain_alpha
 * @property {bool} is_vertical
 */

/**
 * @typedef {object} LayoutCharacter
 * @property {number} x
 * @property {number} y
 * @property {number} z
 * @property {number} scale
 * @property {number} align_vertical
 * @property {number} align_horizontal
 * @property {number} reference_width
 * @property {number} reference_height
 * @property {number} placeholder_id
 */


/**
 * @typedef {object} HealthBarParams
 * @property {string} opponent_icon_model
 * @property {string} player_icon_model
 *
 * @property {bool} has_player_color
 * @property {number} player_color_rgb8
 
 * @property {bool} has_opponent_color
 * @property {number} opponent_color_rgb8
 */

/**
 * @typedef {object} RoundContext
 * @property {object} rankingcounter
 * @property {object} streakcounter
 * @property {object} healthbar
 * @property {object} roundstats
 * @property {object} songinfo
 * @property {object} countdown
 * @property {object} weekgameover
 * @property {object} weekpause
 * @property {object} weekresult
 * @property {object} messagebox
 * @property {object} ui_camera
 * @property {object} missnotefx
 * @property {object} songprogressbar
 * @property {object} autouicosmetics
 * 
 * @property {object} layout
 *  
 * @property {object} songplayer
 * @property {object} script
 * @property {number} playerstats_index
 * @property {object} healthwatcher
 * @property {Settings} settings
 *
 * @property {object} girlfriend
 * @property {PlayerStruct[]} players
 * @property {number} players_size
 * 
 * @property {number} song_index
 * @property {string} song_difficult
 * @property {number} round_duration
 * 
 * @property {ChartEvent[]} events
 * @property {number} events_size
 * @property {number} events_peek_index
 * 
 * @property {HealthBarParams} healthbarparams
 * 
 * @property {ScriptContext} scriptcontext
 * @property {object} dialogue
 * 
 * @property {bool} girlfriend_from_default
 * @property {bool} healthbar_from_default
 * @property {bool} stage_from_default
 * @property {bool} script_from_default
 * @property {bool} dialogue_from_default
 * @property {number} weekgameover_from_version
 * @property {bool} ui_from_default
 * @property {bool} pause_menu_from_default
 *
 * @property {bool} players_from_default
 * @property {bool} distributions_from_default
 * 
 * @property {object} ui_layout
 * @property {object} screen_background
 * 
 * @property {bool} has_directive_changes
 * @property {number} resolution_changes
 * 
 * @property {InitParams} initparams
 *
 */


async function week_destroy(/** @type {RoundContext} */ roundcontext, gameplaymanifest) {
    const initparams = roundcontext.initparams;

    texture_disable_defering(0);

    gameplaymanifest_destroy(gameplaymanifest);
    healthwatcher_destroy(roundcontext.healthwatcher);

    if (roundcontext.rankingcounter) rankingcounter_destroy(roundcontext.rankingcounter);
    if (roundcontext.streakcounter) streakcounter_destroy(roundcontext.streakcounter);
    if (roundcontext.healthbar) healthbar_destroy(roundcontext.healthbar);
    if (roundcontext.layout) layout_destroy(roundcontext.layout);
    if (roundcontext.roundstats) roundstats_destroy(roundcontext.roundstats);
    if (roundcontext.songplayer) songplayer_destroy(roundcontext.songplayer);
    if (roundcontext.script) weekscript_destroy(roundcontext.script);
    if (roundcontext.countdown) countdown_destroy(roundcontext.countdown);
    if (roundcontext.girlfriend) character_destroy(roundcontext.girlfriend);
    if (roundcontext.songinfo) textsprite_destroy(roundcontext.songinfo);
    if (roundcontext.weekgameover) week_gameover_destroy(roundcontext.weekgameover);
    if (roundcontext.weekpause) await week_pause_destroy(roundcontext.weekpause);
    if (roundcontext.screen_background) sprite_destroy(roundcontext.screen_background);
    if (roundcontext.weekresult) week_result_destroy(roundcontext.weekresult);
    if (roundcontext.messagebox) messagebox_destroy(roundcontext.messagebox);
    if (roundcontext.ui_camera) camera_destroy(roundcontext.ui_camera);
    if (roundcontext.missnotefx) missnotefx_destroy(roundcontext.missnotefx);
    if (roundcontext.dialogue) dialogue_destroy(roundcontext.dialogue);
    if (roundcontext.songprogressbar) songprogressbar_destroy(roundcontext.songprogressbar);
    if (roundcontext.autouicosmetics) autouicosmetics_destroy(roundcontext.autouicosmetics);

    roundcontext.events = undefined;
    roundcontext.healthbarparams.player_icon_model = undefined;
    roundcontext.healthbarparams.opponent_icon_model = undefined;

    for (let i = 0; i < roundcontext.players_size; i++) {
        if (roundcontext.players[i].character) character_destroy(roundcontext.players[i].character);
        if (roundcontext.players[i].conductor) conductor_destroy(roundcontext.players[i].conductor);
        if (roundcontext.players[i].notepool) notepool_destroy(roundcontext.players[i].notepool);
        if (roundcontext.players[i].strums) strums_destroy(roundcontext.players[i].strums);
        if (roundcontext.players[i].ddrkeymon) ddrkeymon_destroy(roundcontext.players[i].ddrkeymon);
        if (roundcontext.players[i].controller) gamepad_destroy(roundcontext.players[i].controller);
    }
    roundcontext.players = undefined;

    for (let directive_info of linkedlist_iterate4(roundcontext.scriptcontext.directives)) {
        directive_info.name = undefined;
        directive_info = undefined;
    }
    linkedlist_destroy(roundcontext.scriptcontext.directives);

    if (roundcontext.ui_layout) layout_destroy(roundcontext.ui_layout);

    initparams.layout_strums = undefined;
    initparams.layout_characters = undefined;
    initparams.layout_girlfriend = undefined;

    if (roundcontext.settings.camera_name_opponent != WEEKROUND_CAMERA_OPONNENT)
        roundcontext.settings.camera_name_opponent = undefined;
    if (roundcontext.settings.camera_name_player != WEEKROUND_CAMERA_PLAYER)
        roundcontext.settings.camera_name_player = undefined;

    if (roundcontext.initparams.weekinfo.custom_folder_gameplay)
        fs_override_common_folder(null);
}


async function week_main(weekinfo, alt_tracks, difficult, default_bf, default_gf, gameplaymanifest_src, single_song_index) {

    sh4matrix_clear(WEEKROUND_UI_MATRIX);
    sh4matrix_clear(WEEKROUND_UI_MATRIX_CAMERA);


    /** @type {InitParams} */
    const initparams = {
        alt_tracks: alt_tracks,
        difficult: difficult,
        default_boyfriend: default_bf,
        default_girlfriend: default_gf,
        single_song: single_song_index >= 0,

        layout_strums: null,
        layout_strums_size: 0,

        layout_girlfriend: null,
        layout_characters: null,
        layout_characters_size: 0,

        weekinfo: weekinfo,
        gameplaymanifest: null,

        animlist: null,
        font: null,
        ui_layout_height: 0,
        ui_layout_width: 0,
        ui: {
            healthbar_x: 0,
            healthbar_y: 0,
            healthbar_z: 0,
            healthbar_length: 0,
            healthbar_dimmen: 0,
            healthbar_border: 0,
            healthbar_iconoverlap: 0,
            healthbar_warnheight: 0,
            healthbar_lockheight: 0,
            healthbar_is_vertical: 0,
            healthbar_nowarns: 0,
            roundstats_x: 0,
            roundstats_y: 0,
            roundstats_z: 0,
            roundstats_size: 0,
            roundstats_fontcolor: 0,
            roundstats_hide: 0,
            streakcounter_comboheight: 0,
            streakcounter_numbergap: 0,
            streakcounter_delay: 0,
            rankingcounter_percentonly: 0,
            songinfo_x: 0,
            songinfo_y: 0,
            songinfo_z: 0,
            songinfo_maxwidth: -1,
            songinfo_maxheight: -1,
            songinfo_alignvertical: ALIGN_START,
            songinfo_alignhorinzontal: ALIGN_START,
            songinfo_fontcolor: 0x00,
            songinfo_fontsize: 0x00,
            countdown_height: 0,
            songprogressbar_x: 0,
            songprogressbar_y: 0,
            songprogressbar_z: 0,
            songprogressbar_width: 0,
            songprogressbar_height: 0,
            songprogressbar_align: ALIGN_NONE,
            songprogressbar_bordersize: 0,
            songprogressbar_fontsize: 0,
            songprogressbar_fontbordersize: 0,
            songprogressbar_isvertical: 0,
            songprogressbar_showtime: 0,
            songprogressbar_colorrgba8_text: 0x00,
            songprogressbar_colorrgba8_background: 0x00,
            songprogressbar_colorrgba8_barback: 0x00,
            songprogressbar_colorrgba8_barfront: 0x00
        }
    };

    /** @type {RoundContext} */
    const roundcontext = {
        rankingcounter: null,
        streakcounter: null,
        healthbar: null,
        layout: null,
        roundstats: null,
        songplayer: null,
        script: null,
        dialogue: null,
        playerstats_index: -1,
        healthwatcher: healthwatcher_init(),
        countdown: null,
        songinfo: null,
        weekgameover: null,
        weekpause: null,
        weekresult: null,
        messagebox: null,
        ui_camera: camera_init(null, 640, 480),
        missnotefx: null,
        songprogressbar: null,
        autouicosmetics: autouicosmetics_init(),
        screen_background: sprite_init_from_rgb8(0x0),

        has_directive_changes: 0,

        settings: {
            ask_ready: 1,
            do_countdown: 1,
            girlfriend_cry: 1,
            original_bpm: 100,
            original_speed: 1,
            camera_bumping: 1,
            show_credits: 0,
            no_healthbar: 0,

            bpm: 100,
            speed: 1,

            camera_name_opponent: WEEKROUND_CAMERA_PLAYER,
            camera_name_player: WEEKROUND_CAMERA_OPONNENT,

            layout_rollback: 1
        },
        girlfriend: null,

        song_index: 0,
        round_duration: -1,
        song_difficult: difficult,

        events: null,
        events_size: 0,
        events_peek_index: 0,

        players: null,
        players_size: 0,

        girlfriend_from_default: 1,
        healthbar_from_default: 1,
        stage_from_default: 1,
        script_from_default: 1,
        dialogue_from_default: 1,
        ui_from_default: 1,
        pause_menu_from_default: 0,
        weekgameover_from_version: 0,

        healthbarparams: {
            has_opponent_color: false,
            has_player_color: false,
            opponent_color_rgb8: 0x00,
            player_color_rgb8: 0x00,
            opponent_icon_model: null,
            player_icon_model: null
        },

        scriptcontext: {
            halt_flag: 0,
            directives: linkedlist_init(),
            force_end_flag: 0,
            force_end_round_or_week: 0,
            force_end_loose_or_win: 0,
            no_week_end_result_screen: 0
        },

        players_from_default: 0,
        distributions_from_default: 0,
        ui_layout: null,
        resolution_changes: 0,

        initparams: initparams
    };

    // (JS & C# only) disable texture deferring, avoid shuttering when drawing the first frame
    texture_disable_defering(1);

    // the UI "bump" has one octave of beat as duration (example: 75ms @ 100bpm)
    camera_set_transition_duration(roundcontext.ui_camera, 1, 0.125);

    // screen background
    sprite_set_z_index(roundcontext.screen_background, -Infinity);

    // pause menu
    roundcontext.weekpause = await week_pause_init();
    // song/week stats
    roundcontext.weekresult = week_result_init();
    // messagebox
    roundcontext.messagebox = await messagebox_init();

    // setup custom folder (if exists) and the week folder as current directory
    let week_folder = weekenumerator_get_week_folder(weekinfo);
    fs_set_working_folder(week_folder, 0);
    week_folder = undefined;
    custom_style_from_week = weekinfo;

    if (weekinfo.custom_folder_gameplay) {
        fs_override_common_folder(weekinfo.custom_folder_gameplay);
    } else if (weekinfo.custom_folder) {
        fs_override_common_folder(weekinfo.custom_folder);
    }

    // step 1: load the gameplay manifest this hosts all engine components behavior
    let gameplaymanifest;
    if (gameplaymanifest_src != null) {
        gameplaymanifest = await week_internal_load_gameplay_manifest(gameplaymanifest_src);
    } else {
        gameplaymanifest_src = weekenumerator_get_gameplay_manifest(weekinfo);
        gameplaymanifest = await week_internal_load_gameplay_manifest(gameplaymanifest_src);
        gameplaymanifest_src = undefined;
    }
    if (!gameplaymanifest) {
        texture_disable_defering(0);
        return 1;
    }

    // step 2: initialize the first song (round n° 1)
    roundcontext.song_index = 0;// this is very important
    initparams.gameplaymanifest = gameplaymanifest;

    let gameover = 0;
    let retry = 0;
    let mainmenu = 0;
    let weekselector = 0;
    let songs_attempts = new Array(gameplaymanifest.songs_size);
    let first_init = 1;
    let reject_completed = false;
    let last_song = gameplaymanifest.songs_size - 1;
    let single_song = single_song_index >= 0;

    for (let i = 0; i < gameplaymanifest.songs_size; i++) songs_attempts[i] = 0;

    if (single_song) {
        if (single_song_index > gameplaymanifest.songs_size) {
            console.error("week_main() single_song_index is out of bounds, check your gameplay manifest");
            gameplaymanifest_destroy(gameplaymanifest);
            songs_attempts = undefined;
            return 1;
        }
        roundcontext.song_index = single_song_index;
    }

    // step 3: start the round cycle
    while (roundcontext.song_index < gameplaymanifest.songs_size) {
        beatwatcher_global_set_timestamp(0);

        if (!retry) {
            await week_round_prepare(roundcontext, gameplaymanifest);
        }

        const layout = roundcontext.layout ?? roundcontext.ui_layout;

        // before continue set default values
        beatwatcher_reset(WEEK_BEAT_WATCHER, 1, roundcontext.settings.original_bpm);
        beatwatcher_reset(WEEK_QUARTER_WATCHER, 0, roundcontext.settings.original_bpm);
        week_change_character_camera_name(roundcontext, 1, WEEKROUND_CAMERA_OPONNENT);
        week_change_character_camera_name(roundcontext, 0, WEEKROUND_CAMERA_PLAYER);
        camera_to_origin(roundcontext.ui_camera, 0);
        sh4matrix_copy_to(WEEKROUND_UI_MATRIX, WEEKROUND_UI_MATRIX_CAMERA);

        week_toggle_states(roundcontext, gameplaymanifest);
        messagebox_set_image_sprite(roundcontext.messagebox, null);
        for (let i = 0; i < roundcontext.players_size; i++) {
            character_use_alternate_sing_animations(roundcontext.players[i].character, 0);
            character_freeze_animation(roundcontext.players[i].character, 0);
            character_set_visible(roundcontext.players[i].character, 1);
        }
        roundcontext.scriptcontext.halt_flag = 0;
        layout_set_single_item_to_draw(layout, null);
        if (roundcontext.songplayer) songplayer_mute(roundcontext.songplayer, 0);

        if (first_init) {
            if (roundcontext.script) {
                await weekscript_notify_weekinit(roundcontext.script, single_song ? single_song_index : -1);
                await week_halt(roundcontext, 1);
            }
            first_init = 0;
        }

        if (roundcontext.scriptcontext.force_end_flag) {
            if (!roundcontext.scriptcontext.force_end_round_or_week) {
                gameover = roundcontext.scriptcontext.force_end_loose_or_win ? 1 : 0;
                break;
            }
            roundcontext.scriptcontext.force_end_flag = 0;
        }

        // set the healthbar position
        if (roundcontext.healthbar) {
            healthbar_set_health_position2(roundcontext.healthbar, 0.5);
            healthbar_hide_warnings(roundcontext.healthbar);
        }

        // update songprogressbar
        if (roundcontext.songprogressbar) {
            songprogressbar_set_songplayer(roundcontext.songprogressbar, roundcontext.songplayer);
            songprogressbar_set_duration(roundcontext.songprogressbar, roundcontext.round_duration);
        }

        // check if necessary show dialogue if an dialog text is provided
        let show_dialog = 0;
        let dialog_on_freeplay = !gameplaymanifest.songs[roundcontext.song_index].dialog_ignore_on_freeplay;
        if (!retry && (!single_song || (single_song && dialog_on_freeplay))) {
            let dialog_text = gameplaymanifest.songs[roundcontext.song_index].dialog_text;
            if (!dialog_text) {
                // nothing to do
            } else if (roundcontext.dialogue == null) {
                console.error(`[ERROR] week_round() can not load '${dialog_text}' there no dialogue instance`);
            } else if (await dialogue_show_dialog(roundcontext.dialogue, dialog_text)) {
                if (roundcontext.script != null) await weekscript_notify_dialogue_builtin_open(roundcontext.script, dialog_text);
                show_dialog = 1;
            } else {
                console.error(`week_round() failed to read '${dialog_text}' file`);
            }
        }

        // actual gameplay is here
        let current_song_index = roundcontext.song_index;
        let round_result = await week_round(roundcontext, retry, show_dialog);

        retry = 0;
        week_check_directives_round(roundcontext, round_result == 0);

        if (round_result == 0) {
            week_result_add_stats(roundcontext.weekresult, roundcontext);
        }

        if (roundcontext.scriptcontext.force_end_flag) {
            if (!roundcontext.scriptcontext.force_end_round_or_week) {
                gameover = 1;
                break;
            }
            roundcontext.scriptcontext.force_end_flag = 0;
        }

        if ((round_result == 0 && roundcontext.song_index != last_song) || round_result == 2) {
            if (roundcontext.settings.layout_rollback) {
                layout_stop_all_triggers(layout);
                layout_trigger_any(layout, null);
            }
            week_ui_set_visibility(roundcontext, 1);
            week_internal_reset_players_and_girlfriend(roundcontext);
            if (roundcontext.healthwatcher) healthwatcher_reset_opponents(roundcontext.healthwatcher);
            if (roundcontext.roundstats) roundstats_reset(roundcontext.roundstats);
            if (roundcontext.streakcounter) streakcounter_reset(roundcontext.streakcounter);
        }

        if (round_result == 1) {
            gameover = 1;
            break;
        } else if (round_result == 3) {
            mainmenu = 1;
            break;
        } else if (round_result == 4) {
            weekselector = 1;
            break;
        } else if (round_result == 2) {
            // round loose, retry
            songs_attempts[roundcontext.song_index]++;
            if (roundcontext.songplayer) {
                songplayer_seek(roundcontext.songplayer, 0.0);
                songplayer_mute(roundcontext.songplayer, 0);
            }

            week_toggle_states(roundcontext, gameplaymanifest);

            if (current_song_index != roundcontext.song_index) continue;
            if (roundcontext.song_difficult != initparams.difficult) {
                reject_completed = current_song_index > 0;
                initparams.difficult = roundcontext.song_difficult;
                continue;
            }

            retry = 1;
            roundcontext.events_peek_index = 0;
            continue;
        }

        if (single_song) break;// week launched from freeplaymenu

        // round completed, next one
        roundcontext.song_index++;
        retry = 0;
    }

    if (mainmenu || weekselector) {
        // notify script
        if (roundcontext.script) await weekscript_notify_weekleave(roundcontext.script);

        // flush unlock directives that do not need completed week
        week_check_directives_week(roundcontext, !gameover);

        if (roundcontext.has_directive_changes) {
            let save_error = await savemanager_should_show(1);
            if (save_error != 0) {
                let savemanager = savemanager_init(true, save_error);

                savemanager_show(savemanager);
                savemanager_destroy(savemanager);
            }
        }

        // dispose all allocated resources
        songs_attempts = undefined;
        await week_destroy(roundcontext, gameplaymanifest);

        // if false, goto weekselector
        return mainmenu ? 0 : 1;
    }

    if (roundcontext.script) {
        await weekscript_notify_weekend(roundcontext.script, gameover);
        await week_halt(roundcontext, 1);
    }

    // TODO: check unlockeables
    week_check_directives_week(roundcontext, !gameover);
    if (!gameover && !reject_completed && weekinfo.emit_directive) {
        // if the week was completed successfully emit the directive
        funkinsave_create_unlock_directive(weekinfo.emit_directive, 0x00);
    }

    // show the whole week stats and wait for the player to press START to return
    if (!gameover) {
        let total_attempts = 0;
        let songs_count = single_song ? 1 : gameplaymanifest.songs_size;

        for (let i = 0; i < gameplaymanifest.songs_size; i++) total_attempts += songs_attempts[i];

        if (!roundcontext.scriptcontext.no_week_end_result_screen) {
            await week_result_helper_show_summary(
                roundcontext.weekresult, roundcontext, total_attempts, songs_count, reject_completed
            );
        }
        if (roundcontext.script) {
            await weekscript_notify_afterresults(roundcontext.script, total_attempts, songs_count, reject_completed);
            await week_halt(roundcontext, 1);
        }
    }

    // save progress
    if (roundcontext.has_directive_changes || !gameover && !reject_completed) {
        let total_score = 0;
        for (let i = 0; i < roundcontext.players_size; i++) {
            if (roundcontext.players[i].type != CHARACTERTYPE.PLAYER) continue;
            if (roundcontext.players[i].is_opponent) continue;
            total_score += playerstats_get_score(roundcontext.players[i].playerstats);
        }

        funkinsave_set_week_score(weekinfo.name, roundcontext.song_difficult, total_score);

        // keep displaying the stage layout until the save is done
        messagebox_use_small_size(roundcontext.messagebox, 1);
        messagebox_use_full_title(roundcontext.messagebox, 1);
        messagebox_set_title(roundcontext.messagebox, "Saving progress...");
        messagebox_hide_buttons(roundcontext.messagebox);
        messagebox_set_message(roundcontext.messagebox, null);
        messagebox_show(roundcontext.messagebox, 1);

        // do save
        const layout = roundcontext.layout ?? roundcontext.ui_layout;
        let save_error = await main_spawn_coroutine(layout, savemanager_should_show, 1);

        if (save_error) {
            layout_suspend(layout);
            let savemanager = await savemanager_init(1, save_error);
            await savemanager_show(savemanager);
            savemanager_destroy(savemanager);
        }
    }

    let show_credits = !gameover && roundcontext.settings.show_credits;

    // dispose all allocated resources
    songs_attempts = undefined;
    await week_destroy(roundcontext, gameplaymanifest);

    if (show_credits) {
        // game ending
        await credits_main();
    }

    return 1;
}

async function week_init_ui_layout(src_layout,/** @type {InitParams} */ initparams, roundcontext) {
    const layout_size = [0, 0];
    const ui = initparams.ui;
    let src;
    let placeholder;

    if (src_layout) src = src_layout;
    else src = pvrctx_is_widescreen() ? UI_LAYOUT_WIDESCREEN : UI_LAYOUT_DREAMCAST;

    let layout = await layout_init(src);
    if (roundcontext.ui_layout) layout_destroy(roundcontext.ui_layout);

    if (!layout) throw new Error("Can not load the ui layout: " + src);

    initparams.layout_strums = undefined;
    roundcontext.ui_layout = layout;

    layout_get_viewport_size(layout, layout_size);
    camera_change_viewport(roundcontext.ui_camera, layout_size[0], layout_size[1]);
    initparams.ui_layout_width = layout_size[0];
    initparams.ui_layout_height = layout_size[1];
    ui.countdown_height = initparams.ui_layout_height / 3;

    initparams.font = layout_get_attached_font(layout, "ui_font");

    let src_animlist = layout_get_attached_value(layout, "ui_animlist", LAYOUT_TYPE_STRING, UI_ANIMLIST);
    let old_animlist = initparams.animlist;
    initparams.animlist = src_animlist ? await animlist_init(src_animlist) : null;
    if (old_animlist) animlist_destroy(old_animlist);

    initparams.layout_strums_size = layout_get_attached_value(layout, "ui_strums_count", LAYOUT_TYPE_INTEGER, 0);
    initparams.layout_strums = initparams.layout_strums_size > 0 ? new Array(initparams.layout_strums_size) : null;
    for (let i = 0; i < initparams.layout_strums_size; i++) {
        placeholder = week_internal_read_placeholder(layout, "ui_strums", i);
        if (!placeholder) placeholder = UI_STRUMS_LAYOUT_PLACEHOLDER;

        initparams.layout_strums[i] = {
            marker_dimmen: week_internal_read_value_prefix(layout, "ui_strums_markerDimmen", i, LAYOUT_TYPE_FLOAT, 50),
            invdimmen: week_internal_read_value_prefix(layout, "ui_strums_markerInvdimmen", i, LAYOUT_TYPE_FLOAT, 50),
            sustain_alpha: week_internal_read_value_prefix(layout, "ui_strums_sustainAlpha", i, LAYOUT_TYPE_FLOAT, 1.0),
            gap: week_internal_read_value_prefix(layout, "ui_strums_gap", i, LAYOUT_TYPE_FLOAT, 0),
            keep_marker_scale: week_internal_read_value_prefix(layout, "ui_strums_keepMarkerScale", i, LAYOUT_TYPE_BOOLEAN, 1),
            is_vertical: week_internal_read_value_prefix(layout, "ui_strums_verticalScroll", i, LAYOUT_TYPE_BOOLEAN, 1),
            length: 0,
            x: placeholder.x,
            y: placeholder.y,
            z: placeholder.z
        };
        initparams.layout_strums[i].length = initparams.layout_strums[i].is_vertical ? placeholder.height : placeholder.width;
    }

    placeholder = layout_get_placeholder(layout, "ui_healthbar");
    if (!placeholder) {
        console.error("missing layout ui_healthbar placeholder");
        placeholder = UI_STUB_LAYOUT_PLACEHOLDER;
    }
    ui.healthbar_is_vertical = layout_get_attached_value(layout, "ui_healthbar_isVertical", LAYOUT_TYPE_BOOLEAN, 0);
    ui.healthbar_nowarns = !layout_get_attached_value(layout, "ui_healthbar_showWarnings", LAYOUT_TYPE_BOOLEAN, 1);
    ui.healthbar_border = layout_get_attached_value(layout, "ui_healthbar_borderSize", LAYOUT_TYPE_FLOAT, 0);
    ui.healthbar_iconoverlap = layout_get_attached_value(layout, "ui_healthbar_iconOverlap", LAYOUT_TYPE_FLOAT, 0);
    ui.healthbar_warnheight = layout_get_attached_value(layout, "ui_healthbar_warningHeight", LAYOUT_TYPE_FLOAT, 0);
    ui.healthbar_lockheight = layout_get_attached_value(layout, "ui_healthbar_lockedHeight", LAYOUT_TYPE_FLOAT, 0);
    ui.healthbar_x = placeholder.x;
    ui.healthbar_y = placeholder.y;
    ui.healthbar_z = placeholder.z;
    ui.healthbar_length = placeholder.width;
    ui.healthbar_dimmen = placeholder.height;
    if (ui.healthbar_is_vertical) {
        let temp = ui.healthbar_length;
        ui.healthbar_length = ui.healthbar_dimmen;
        ui.healthbar_dimmen = temp;
    }

    placeholder = layout_get_placeholder(layout, "ui_roundstats");
    if (!placeholder) {
        console.error("missing layout ui_roundstats placeholder");
        placeholder = UI_STUB_LAYOUT_PLACEHOLDER;
    }
    ui.roundstats_x = placeholder.x;
    ui.roundstats_y = placeholder.y;
    ui.roundstats_z = placeholder.z;
    ui.roundstats_hide = layout_get_attached_value(layout, "ui_roundstats_hidden", LAYOUT_TYPE_BOOLEAN, 0);
    ui.roundstats_size = layout_get_attached_value(layout, "ui_roundstats_fontSize", LAYOUT_TYPE_FLOAT, 12);
    ui.roundstats_fontcolor = layout_get_attached_value(layout, "ui_roundstats_fontColor", LAYOUT_TYPE_HEX, 0xFFFFFF);


    placeholder = layout_get_placeholder(layout, "ui_songprogressbar");
    if (!placeholder) {
        console.error("missing layout ui_songprogressbar placeholder");
        placeholder = UI_STUB_LAYOUT_PLACEHOLDER;
    }
    ui.songprogressbar_bordersize = layout_get_attached_value(layout, "ui_songprogressbar_borderSize", LAYOUT_TYPE_FLOAT, 2);
    ui.songprogressbar_fontsize = layout_get_attached_value(layout, "ui_songprogressbar_fontSize", LAYOUT_TYPE_FLOAT, 11);
    ui.songprogressbar_fontbordersize = layout_get_attached_value(layout, "ui_songprogressbar_fontBorderSize", LAYOUT_TYPE_FLOAT, 1.4);
    ui.songprogressbar_isvertical = layout_get_attached_value(layout, "ui_songprogressbar_isVertical", LAYOUT_TYPE_BOOLEAN, 0);
    ui.songprogressbar_showtime = layout_get_attached_value(layout, "ui_songprogressbar_showTime", LAYOUT_TYPE_BOOLEAN, 1);
    ui.songprogressbar_colorrgba8_text = layout_get_attached_value(layout, "ui_songprogressbar_colorRGBA8_text", LAYOUT_TYPE_HEX, 0xFFFFFFFF);
    ui.songprogressbar_colorrgba8_background = layout_get_attached_value(layout, "ui_songprogressbar_colorRGBA8_background", LAYOUT_TYPE_HEX, 0x000000FF);
    ui.songprogressbar_colorrgba8_barback = layout_get_attached_value(layout, "ui_songprogressbar_colorRGBA8_barBack", LAYOUT_TYPE_HEX, 0x808080FF);
    ui.songprogressbar_colorrgba8_barfront = layout_get_attached_value(layout, "ui_songprogressbar_colorRGBA8_barFront", LAYOUT_TYPE_HEX, 0xFFFFFFFF);
    ui.songprogressbar_x = placeholder.x;
    ui.songprogressbar_y = placeholder.y;
    ui.songprogressbar_z = placeholder.z;
    ui.songprogressbar_width = placeholder.width;
    ui.songprogressbar_height = placeholder.height;
    ui.songprogressbar_align = ui.songprogressbar_isvertical ? placeholder.align_vertical : placeholder.align_horizontal;


    // pick streakcounter and rankingcounter values
    week_internal_pick_counters_values_from_layout(roundcontext);

    placeholder = layout_get_placeholder(layout, "ui_song_info");
    if (!placeholder) {
        console.error("missing layout ui_song_info placeholder");
        placeholder = UI_STUB_LAYOUT_PLACEHOLDER;
    }
    ui.songinfo_x = placeholder.x;
    ui.songinfo_y = placeholder.y;
    ui.songinfo_z = placeholder.z;
    ui.songinfo_maxwidth = placeholder.width;
    ui.songinfo_maxheight = placeholder.height;
    ui.songinfo_alignvertical = placeholder.align_vertical;
    ui.songinfo_alignhorinzontal = placeholder.align_horizontal;
    ui.songinfo_fontsize = layout_get_attached_value(layout, "ui_song_info_fontSize", LAYOUT_TYPE_FLOAT, 10);
    ui.songinfo_fontcolor = layout_get_attached_value(layout, "ui_song_info_fontColor", LAYOUT_TYPE_HEX, 0xFFFFFF);

    // initialize adaptation of the UI elements in the stage layout
    week_internal_check_screen_resolution(roundcontext, true);

    // pick default counters values
    week_internal_pick_counters_values_from_layout(roundcontext);
}

function week_pick_inverted_ui_layout_values(/** @type {RoundContext} */ roundcontext) {
    const layout = roundcontext.ui_layout;
    const initparams = roundcontext.initparams;
    const ui = roundcontext.initparams.ui;
    let placeholder;

    for (let i = 0; i < initparams.layout_strums_size; i++) {
        placeholder = week_internal_read_placeholder(layout, "ui_strums_inverted", i);
        if (placeholder == null) continue;

        initparams.layout_strums[i].x = placeholder.x;
        initparams.layout_strums[i].y = placeholder.y;
        initparams.layout_strums[i].z = placeholder.z;
        initparams.layout_strums[i].length = initparams.layout_strums[i].is_vertical ? placeholder.height : placeholder.width;
    }

    placeholder = layout_get_placeholder(layout, "ui_healthbar_inverted");
    if (placeholder != null) {
        ui.healthbar_x = placeholder.x;
        ui.healthbar_y = placeholder.y;
        ui.healthbar_z = placeholder.z;
        ui.healthbar_length = placeholder.width;
        ui.healthbar_dimmen = placeholder.height;
        if (ui.healthbar_is_vertical) {
            let temp = ui.healthbar_length;
            ui.healthbar_length = ui.healthbar_dimmen;
            ui.healthbar_dimmen = temp;
        }
    }

    placeholder = layout_get_placeholder(layout, "ui_roundstats_inverted");
    if (placeholder != null) {
        ui.roundstats_x = placeholder.x;
        ui.roundstats_y = placeholder.y;
        ui.roundstats_z = placeholder.z;
    }

    placeholder = layout_get_placeholder(layout, "ui_songprogressbar_inverted");
    if (placeholder != null) {
        ui.songprogressbar_x = placeholder.x;
        ui.songprogressbar_y = placeholder.y;
        ui.songprogressbar_z = placeholder.z;
    }

    placeholder = layout_get_placeholder(layout, "ui_song_info_inverted");
    if (placeholder != null) {
        ui.songinfo_x = placeholder.x;
        ui.songinfo_y = placeholder.y;
        ui.songinfo_z = placeholder.z;
    }
}


async function week_round_prepare(/**@type {RoundContext}*/roundcontext, gameplaymanifest) {
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
    const songmanifest = gameplaymanifest.songs[roundcontext.song_index];
    const initparams = roundcontext.initparams;

    let updated_ui = 0;
    let updated_distributions_or_players = 0;
    let updated_stage = 0;

    // initialize layout
    if (songmanifest.has_stage) {
        updated_stage = 1;
        roundcontext.stage_from_default = 0;
        await week_init_stage(roundcontext, songmanifest.stage);
    } else if (!roundcontext.layout || !roundcontext.stage_from_default) {
        updated_stage = 1;
        roundcontext.stage_from_default = 1;
        await week_init_stage(roundcontext, gameplaymanifest.default.stage);
    }

    // initialize script/stagescript
    if (songmanifest.has_script) {
        roundcontext.script_from_default = 0;
        await week_init_script(roundcontext, songmanifest.script);
    } else if (!roundcontext.script || !roundcontext.script_from_default) {
        roundcontext.script_from_default = 1;
        await week_init_script(roundcontext, gameplaymanifest.default.script);
    }

    // initialize dialogue
    if (songmanifest.dialogue_params) {
        roundcontext.dialogue_from_default = 0;
        await week_init_dialogue(
            roundcontext,
            songmanifest.dialogue_params,
            songmanifest.dialog_ignore_on_freeplay && initparams.single_song
        );
    } else if (roundcontext.dialogue == null || !roundcontext.script_from_default) {
        roundcontext.dialogue_from_default = 1;
        await week_init_dialogue(
            roundcontext,
            gameplaymanifest.default.dialogue_params,
            songmanifest.dialog_ignore_on_freeplay && initparams.single_song
        );
    }

    // reload the music only
    await week_pause_prepare(roundcontext.weekpause);

    if (!roundcontext.missnotefx) {
        roundcontext.missnotefx = await missnotefx_init();
    }

    // initialize ui
    if (songmanifest.has_ui_layout) {
        let src = songmanifest.ui_layout;
        if (!src) src = gameplaymanifest.default.ui_layout;

        await week_init_ui_layout(src, initparams, roundcontext);
        roundcontext.ui_from_default = 0;
        updated_ui = 1;
    } else if (!roundcontext.ui_layout || !roundcontext.ui_from_default) {
        await week_init_ui_layout(gameplaymanifest.default.ui_layout, initparams, roundcontext);
        roundcontext.ui_from_default = 1;
        updated_ui = 1;
    }

    if (updated_ui && SETTINGS.inverse_strum_scroll) {
        // pick inverted placeholder values
        week_pick_inverted_ui_layout_values(roundcontext);
    }

    // initialize strums, character and controller misc.
    if (await week_init_chart_and_players(roundcontext, gameplaymanifest, updated_ui)) {
        roundcontext.playerstats_index = -1;

        // pick playerstats from the first playable character
        for (let i = 0; i < roundcontext.players_size; i++) {
            if (roundcontext.players[i].type != CHARACTERTYPE.PLAYER) continue;
            roundcontext.playerstats_index = i;
            break;
        }
        if (roundcontext.playerstats_index < 0 && roundcontext.players_size > 0) {
            roundcontext.playerstats_index = 0;
        }

        updated_distributions_or_players = 1;
    }

    //let multiplier = roundcontext.song_difficult == FUNKIN_DIFFICULT_EASY ? 1.25 : 1.0;
    //for (let i = 0; i < roundcontext.players_size; i++) {
    //    if (roundcontext.players[i].strums)
    //        strums_set_marker_duration_multiplier(roundcontext.players[i].strums, multiplier);
    //}

    // rebuild the healthwatcher
    healthwatcher_clear(roundcontext.healthwatcher);
    for (let i = 0; i < roundcontext.players_size; i++) {
        playerstats_reset(roundcontext.players[i].playerstats);

        // untested
        //if (roundcontext.players[i].type == CHARACTERTYPE.ACTOR) continue;

        if (roundcontext.players[i].is_opponent) {
            healthwatcher_add_opponent(
                roundcontext.healthwatcher,
                roundcontext.players[i].playerstats,
                roundcontext.players[i].can_recover,
                roundcontext.players[i].can_die
            );
        } else {
            healthwatcher_add_player(
                roundcontext.healthwatcher,
                roundcontext.players[i].playerstats,
                roundcontext.players[i].can_recover,
                roundcontext.players[i].can_die
            );
        }
    }

    // initialize healthbar
    if (updated_distributions_or_players && roundcontext.healthbar) {
        updated_stage = await week_init_healthbar(roundcontext, gameplaymanifest, updated_distributions_or_players);
    } else if (!roundcontext.healthbar) {
        updated_stage = await week_init_healthbar(roundcontext, gameplaymanifest, 1);
    }

    // initialize girlfriend
    if (songmanifest.has_girlfriend) {
        updated_stage = 1;
        roundcontext.girlfriend_from_default = 0;
        await week_init_girlfriend(roundcontext, songmanifest.girlfriend);
    } else if (!roundcontext.girlfriend || !roundcontext.girlfriend_from_default) {
        updated_stage = 1;
        roundcontext.girlfriend_from_default = 1;
        await week_init_girlfriend(roundcontext, gameplaymanifest.default.girlfriend);
    }

    // add additional pause menu
    if (songmanifest.has_pause_menu) {
        roundcontext.pause_menu_from_default = 0;
        await week_pause_external_set_menu(roundcontext.weekpause, songmanifest.pause_menu);
    } else if (!roundcontext.pause_menu_from_default) {
        roundcontext.pause_menu_from_default = 1;
        await week_pause_external_set_menu(roundcontext.weekpause, gameplaymanifest.default.pause_menu);
    }

    // initialize the song
    if (roundcontext.songplayer) songplayer_destroy(roundcontext.songplayer);
    roundcontext.songplayer = await songplayer_init(songmanifest.file, initparams.alt_tracks);

    // initialize the gameover screen
    await week_init_ui_gameover(roundcontext);

    // update location&size of UI cosmetics (if required)
    if (updated_ui || updated_stage) await week_init_ui_cosmetics(roundcontext);

    textsprite_set_text_formated(
        roundcontext.songinfo,
        UI_SONGINFO_FORMAT,
        songmanifest.name,
        initparams.alt_tracks ? UI_SONGINFO_ALT_SUFFIX : null,
        initparams.difficult,
        ENGINE_VERSION
    );

    // Incorporates all ui elements in the stage layout
    if (updated_stage || updated_ui || updated_distributions_or_players) {
        week_place_in_layout(roundcontext);
    }

    // toggle states
    week_toggle_states(roundcontext, gameplaymanifest);
    week_update_bpm(roundcontext, roundcontext.settings.original_bpm);
    week_update_speed(roundcontext, roundcontext.settings.original_speed);
    roundcontext.round_duration = songmanifest.duration;
}

async function week_init_healthbar(roundcontext, gameplaymanifest, force_update) {
    const initparams = roundcontext.initparams;

    let healthbarmanifest = gameplaymanifest.default.healthbar;
    const healthbarparams = roundcontext.healthbarparams;

    if (gameplaymanifest.songs[roundcontext.song_index].healthbar) {
        roundcontext.healthbar_from_default = 0;
        healthbarmanifest = gameplaymanifest.songs[roundcontext.song_index].healthbar;
    } else if (force_update || !roundcontext.healthbar || !roundcontext.healthbar_from_default) {
        roundcontext.healthbar_from_default = 1;
    } else {
        // no chages to make
        return 0;
    }

    if (roundcontext.healthbar) {
        healthbar_destroy(roundcontext.healthbar);
        //roundcontext.healthbar = null;
    }

    console.assert(healthbarmanifest, "the manifest of the healthbar was null");

    roundcontext.healthbar = healthbar_init(
        initparams.ui.healthbar_x,
        initparams.ui.healthbar_y,
        initparams.ui.healthbar_z,
        initparams.ui.healthbar_length,
        initparams.ui.healthbar_dimmen,
        initparams.ui.healthbar_border,
        initparams.ui.healthbar_iconoverlap,
        initparams.ui.healthbar_warnheight,
        initparams.ui.healthbar_lockheight
    );
    if (healthbarmanifest.has_disable) {
        roundcontext.settings.no_healthbar = healthbarmanifest.disable;
    }
    healthbar_enable_vertical(roundcontext.healthbar, initparams.ui.healthbar_is_vertical);
    healthbar_set_layout_size(
        roundcontext.healthbar, initparams.ui_layout_width, initparams.ui_layout_height
    );
    if (healthbarmanifest.warnings_model) {
        let modelholder_warn_icons = await modelholder_init(
            healthbarmanifest.warnings_model
        );
        if (modelholder_warn_icons) {
            healthbar_load_warnings(
                roundcontext.healthbar,
                modelholder_warn_icons,
                healthbarmanifest.use_alt_icons
            );
            modelholder_destroy(modelholder_warn_icons);
        }
    }
    healthbar_set_visible(roundcontext.healthbar, !roundcontext.settings.no_healthbar);
    healthbar_disable_warnings(roundcontext.healthbar, initparams.ui.healthbar_nowarns);
    healthbar_set_bump_animation(roundcontext.healthbar, initparams.animlist);


    let default_icon_model_opponent = null;
    let default_icon_model_player = null;

    if (healthbarparams.opponent_icon_model) {
        default_icon_model_opponent = await modelholder_init(healthbarparams.opponent_icon_model);
        healthbarparams.opponent_icon_model = undefined;
    }
    if (healthbarparams.player_icon_model) {
        default_icon_model_player = await modelholder_init(healthbarparams.player_icon_model);
        healthbarparams.player_icon_model = undefined;
    }

    // import healthbar states
    for (let i = 0; i < healthbarmanifest.states_size; i++) {
        let state = healthbarmanifest.states[i];

        // temporal variables
        let icon_modelholder = null;
        let bar_color = state.opponent.bar_color;
        let bar_model = state.opponent.bar_model;

        if (!state.opponent.icon_model && default_icon_model_opponent) {
            // pick from the player manifest, player 0 shold be always the opponent
            icon_modelholder = default_icon_model_opponent;
        } else if (state.opponent.icon_model) {
            icon_modelholder = await modelholder_init(state.opponent.icon_model);
        }

        if (bar_model) {
            let modelholder_bar = await modelholder_init(bar_model);
            healthbar_state_opponent_add(
                roundcontext.healthbar, icon_modelholder, modelholder_bar, state.name
            );
            modelholder_destroy(modelholder_bar);
        } else {
            healthbar_state_opponent_add2(
                roundcontext.healthbar, icon_modelholder, bar_color, state.name
            );
        }
        if (icon_modelholder != default_icon_model_opponent) modelholder_destroy(icon_modelholder);

        icon_modelholder = null;
        bar_color = state.player.bar_color;
        bar_model = state.player.bar_model;
        if (!state.player.icon_model && default_icon_model_player) {
            // pick from the player manifest, player 0 shold be always the player
            icon_modelholder = default_icon_model_player;
        } else if (state.player.icon_model) {
            icon_modelholder = await modelholder_init(state.player.icon_model);
        }

        if (bar_model) {
            let modelholder_bar = await modelholder_init(bar_model);
            healthbar_state_player_add(
                roundcontext.healthbar, icon_modelholder, modelholder_bar, state.name
            );
            modelholder_destroy(modelholder_bar);
        } else {
            healthbar_state_player_add2(
                roundcontext.healthbar, icon_modelholder, bar_color, state.name
            );
        }
        if (icon_modelholder != default_icon_model_player) modelholder_destroy(icon_modelholder);

        if (state.background.bar_model) {
            let modelholder_bar = await modelholder_init(state.background.bar_model);
            if (modelholder_bar != null) {
                healthbar_state_background_add(
                    roundcontext.healthbar, modelholder_bar, state.name
                );
                modelholder_destroy(modelholder_bar);
            }
        } else {
            healthbar_state_background_add2(
                roundcontext.healthbar, null, state.background.bar_color, state.name
            );
        }
    }

    // if the healthbar does not have states, assign the bar colors if was specified
    if (healthbarmanifest.states_size < 1) {
        let opponent_color_rgb8 = HEALTHBAR_DEFAULT_COLOR_DAD;
        let player_color_rgb8 = HEALTHBAR_DEFAULT_COLOR_BOYFRIEND;

        if (healthbarmanifest.has_opponent_color)
            opponent_color_rgb8 = healthbarmanifest.opponent_color_rgb8;
        else if (healthbarparams.has_opponent_color)
            opponent_color_rgb8 = healthbarparams.opponent_color_rgb8;

        if (healthbarmanifest.has_player_color)
            player_color_rgb8 = healthbarmanifest.player_color_rgb8;
        else if (healthbarparams.has_player_color)
            player_color_rgb8 = healthbarparams.player_color_rgb8;

        healthbar_state_background_add2(
            roundcontext.healthbar, HEALTHBAR_DEFAULT_COLOR_BACKGROUND, null, null
        );
        healthbar_state_opponent_add2(
            roundcontext.healthbar, default_icon_model_opponent, opponent_color_rgb8, null
        );
        healthbar_state_player_add2(
            roundcontext.healthbar, default_icon_model_player, player_color_rgb8, null
        );
        healthbar_state_toggle(roundcontext.healthbar, null);
    }

    healthbar_set_health_position(roundcontext.healthbar, 1.0, 0.5, 0);

    modelholder_destroy(default_icon_model_opponent);
    modelholder_destroy(default_icon_model_player);

    return 1;
}

async function week_init_girlfriend(roundcontext, girlfriend_manifest) {
    const initparams = roundcontext.initparams;

    if (roundcontext.girlfriend) {
        character_destroy(roundcontext.girlfriend);
        roundcontext.girlfriend = null;
    }

    if (!girlfriend_manifest) return;

    let manifest;
    switch (girlfriend_manifest.refer) {
        case GAMEPLAYMANIFEST_REFER_BOYFRIEND:
            manifest = initparams.default_boyfriend;
            break;
        case GAMEPLAYMANIFEST_REFER_GIRLFRIEND:
            manifest = initparams.default_girlfriend;
            break;
        default:
            if (!girlfriend_manifest.manifest) return;
            manifest = girlfriend_manifest.manifest;
            break;
    }

    let charactermanifest = await charactermanifest_init(manifest, 1);
    roundcontext.girlfriend = await character_init(charactermanifest);
    charactermanifest_destroy(charactermanifest);
}

async function week_init_stage(roundcontext, stage_src) {
    const initparams = roundcontext.initparams;

    // keep the old layout if older assets are reused
    let old_layout = roundcontext.layout;
    let placeholder;

    if (stage_src)
        roundcontext.layout = await layout_init(stage_src);
    else
        roundcontext.layout = null;

    if (old_layout) layout_destroy(old_layout);

    initparams.layout_girlfriend = undefined;
    initparams.layout_characters = undefined;
    initparams.layout_characters_size = 0;

    initparams.layout_girlfriend = null;
    initparams.layout_characters = null;

    if (!roundcontext.layout) return;

    // keep triggers synced
    layout_sync_triggers_with_global_beatwatcher(roundcontext.layout, 1);

    // pick all player characters placement
    let count = layout_get_attached_value(
        roundcontext.layout, "character_count", LAYOUT_TYPE_INTEGER, 0
    );

    initparams.layout_characters = count > 0 ? new Array(count) : null;
    initparams.layout_characters_size = count;

    for (let i = 0; i < count; i++) {
        initparams.layout_characters[i] = {
            align_vertical: ALIGN_START,
            align_horizontal: ALIGN_START,
            reference_width: -1,
            reference_height: -1,
            x: 0,
            y: 0,
            z: 0,
            scale: 1.0,
            placeholder_id: -1
        };

        placeholder = week_internal_read_placeholder(
            roundcontext.layout, WEEKROUND_CHARACTER_PREFIX, i
        );
        if (!placeholder) continue;

        initparams.layout_characters[i].placeholder_id = layout_get_placeholder_id(
            roundcontext.layout, placeholder.name
        );
        initparams.layout_characters[i].scale = week_internal_read_value_prefix(
            roundcontext.layout, "character_scale_", i, LAYOUT_TYPE_FLOAT, 1.0
        );

        initparams.layout_characters[i].x = placeholder.x;
        initparams.layout_characters[i].y = placeholder.y;
        initparams.layout_characters[i].z = placeholder.z;
        initparams.layout_characters[i].align_vertical = placeholder.align_vertical;
        initparams.layout_characters[i].align_horizontal = placeholder.align_horizontal;
        initparams.layout_characters[i].reference_width = placeholder.width;
        initparams.layout_characters[i].reference_height = placeholder.height;
    }

    // pick girlfriend character placement
    placeholder = layout_get_placeholder(roundcontext.layout, "character_girlfriend");
    if (placeholder) {
        initparams.layout_girlfriend = {
            align_vertical: ALIGN_START,
            align_horizontal: ALIGN_START,
            reference_width: -1,
            reference_height: -1,
            x: 0,
            y: 0,
            z: 0,
            scale: 1.0,
            placeholder_id: -1
        };

        initparams.layout_girlfriend.placeholder_id = layout_get_placeholder_id(
            roundcontext.layout, placeholder.name
        );
        initparams.layout_girlfriend.scale = layout_get_attached_value(
            roundcontext.layout, "character_girlfriend_scale", LAYOUT_TYPE_FLOAT, 1.0
        );

        initparams.layout_girlfriend.x = placeholder.x;
        initparams.layout_girlfriend.y = placeholder.y;
        initparams.layout_girlfriend.z = placeholder.z;
        initparams.layout_girlfriend.align_vertical = placeholder.align_vertical;
        initparams.layout_girlfriend.align_horizontal = placeholder.align_horizontal;
        initparams.layout_girlfriend.reference_width = placeholder.width;
        initparams.layout_girlfriend.reference_height = placeholder.height;
    }

    week_internal_pick_counters_values_from_layout(roundcontext);
}

async function week_init_script(/**@type {RoundContext}*/roundcontext, script_src) {
    if (roundcontext.script) {
        await weekscript_notify_scriptchange(roundcontext.script);
        weekscript_destroy(roundcontext.script);
    }

    roundcontext.script = null;

    if (script_src) {
        roundcontext.script = await weekscript_init(script_src, roundcontext, 1);
    }

    if (roundcontext.dialogue != null) dialogue_set_script(roundcontext.dialogue, roundcontext.script);

    for (let i = 0; i < roundcontext.players_size; i++) {
        if (roundcontext.players[i].strums) {
            strums_set_params(
                roundcontext.players[i].strums,
                roundcontext.players[i].ddrkeymon,
                roundcontext.players[i].playerstats,
                roundcontext.script
            );
        }
    }
}

async function week_init_chart_and_players(/**@type {RoundContext}*/roundcontext, gameplaymanifest, new_ui) {
    const initparams = roundcontext.initparams;
    const song_index = roundcontext.song_index;

    let chart = await chart_init(gameplaymanifest.songs[song_index].chart, initparams.difficult);

    // keep just in case the same textures are used
    let old_players = roundcontext.players;
    let old_players_size = roundcontext.players_size;

    // update events table
    roundcontext.events = undefined;
    roundcontext.events = clone_array(chart.events, chart.events_size);
    roundcontext.events_size = chart.events_size;
    roundcontext.events_peek_index = 0;
    roundcontext.settings.original_bpm = chart.bpm;
    roundcontext.settings.original_speed = chart.speed;

    // Pick players & strum distributions from default or current song
    let distributions_from_default = !gameplaymanifest.songs[song_index].has_distributions;
    let players_from_default = !gameplaymanifest.songs[song_index].has_players;
    let players = gameplaymanifest.default.players;
    let players_size = gameplaymanifest.default.players_size;
    let distributions = gameplaymanifest.default.distributions;
    let distributions_size = gameplaymanifest.default.distributions_size;

    if (gameplaymanifest.songs[song_index].has_players) {
        players = gameplaymanifest.songs[song_index].players;
        players_size = gameplaymanifest.songs[song_index].players_size;
    }
    if (gameplaymanifest.songs[song_index].has_distributions) {
        distributions = gameplaymanifest.songs[song_index].distributions;
        distributions_size = gameplaymanifest.songs[song_index].distributions_size;
    }

    //
    // JS & C# only, guess if the keyboard should count as another gamepad
    // or the keyboard and the first connected gamepad should be mixed
    //
    let required_controllers = 0;
    for (let i = 0; i < players_size; i++) {
        if (players[i].controller >= 0) required_controllers++;
    }
    let mix_keyboard = required_controllers < 2;

    // check if is required update the current players & strum distributions
    let same_players = roundcontext.players_from_default && players_from_default;
    let same_distributions = roundcontext.distributions_from_default == distributions_from_default;
    if (same_players && same_distributions && old_players) {
        week_internal_reset_players_and_girlfriend(roundcontext);

        // update only the strums and the character animations
        for (let i = 0; i < roundcontext.players_size; i++) {
            if (players[i].distribution_index >= 0 || roundcontext.players[i].strums) {
                if (players[i].distribution_index >= distributions_size) {
                    throw new RangeError("invalid distribution_index");
                }

                let distribution = distributions[players[i].distribution_index];
                strums_set_notes(
                    roundcontext.players[i].strums,
                    chart,
                    distribution.strums,
                    distribution.strums_size,
                    roundcontext.players[i].notepool
                );
            }
        }

        chart_destroy(chart);

        if (new_ui) {
            for (let i = 0; i < roundcontext.players_size; i++) {
                if (!roundcontext.players[i].strums) continue;
                let id = players[i].layout_strums_id < 0 ? i : players[i].layout_strums_id;

                if (id >= initparams.layout_strums_size) {
                    console.error(`week_main() invalid player[${i}].layoutStrumsId=${id}`);
                    continue;
                } else if (id < 0) {
                    continue;
                }

                let layout_strum = initparams.layout_strums[id];
                strums_force_rebuild(
                    roundcontext.players[i].strums,
                    layout_strum.x,
                    layout_strum.y,
                    layout_strum.z,
                    layout_strum.marker_dimmen,
                    layout_strum.invdimmen,
                    layout_strum.length,
                    layout_strum.gap,
                    layout_strum.is_vertical,
                    layout_strum.keep_marker_scale
                );
            }

            week_change_scroll_direction(roundcontext);
        }

        return 0;
    }

    // remember where players and distributions was picked
    roundcontext.players_from_default = players_from_default;
    roundcontext.distributions_from_default = distributions_from_default;

    let charactermanifests = new Array(players_size);
    let manifest_player = null;
    let manifest_opponent = null;

    roundcontext.players = new Array(players_size);
    roundcontext.players_size = players_size;

    for (let i = 0; i < roundcontext.players_size; i++) {
        let manifest_src;
        switch (players[i].refer) {
            case GAMEPLAYMANIFEST_REFER_BOYFRIEND:
                manifest_src = initparams.default_boyfriend;
                break;
            case GAMEPLAYMANIFEST_REFER_GIRLFRIEND:
                manifest_src = initparams.default_girlfriend;
                break;
            default:
                manifest_src = players[i].manifest;
                break;
        }

        charactermanifests[i] = await charactermanifest_init(manifest_src, 1);

        roundcontext.players[i] = {
            character: await character_init(charactermanifests[i]),
            conductor: conductor_init(),
            playerstats: playerstats_init(),
            notepool: null,
            strums: null,
            type: CHARACTERTYPE.ACTOR,
            ddrkeymon: null,
            ddrkeys_fifo: null,
            controller: null,
            is_vertical: 1,
            can_die: players[i].can_die,
            can_recover: players[i].can_recover,
            is_opponent: players[i].is_opponent
        };

        conductor_set_character(
            roundcontext.players[i].conductor, roundcontext.players[i].character
        );
        conductor_set_missnotefx(roundcontext.players[i].conductor, roundcontext.missnotefx);

        character_face_as_opponent(roundcontext.players[i].character, roundcontext.players[i].is_opponent);

        // obtain the position in the UI layout
        let layout_strums_id = players[i].layout_strums_id < 0 ? i : players[i].layout_strums_id;

        if (layout_strums_id >= initparams.layout_strums_size) {
            console.error(`week_main() invalid player[${i}].layoutStrumsId=${layout_strums_id}`);
            layout_strums_id = -1;
        }

        if (layout_strums_id < 0 || players[i].distribution_index < 0) {
            roundcontext.players[i].type = CHARACTERTYPE.ACTOR;
            continue;
        }
        if (players[i].distribution_index >= distributions_size) {
            throw new RangeError("invalid distribution_index");
        }

        let distribution = distributions[players[i].distribution_index];
        let type = players[i].controller < 0 ? CHARACTERTYPE.BOT : CHARACTERTYPE.PLAYER;
        let layout_strum = initparams.layout_strums[layout_strums_id];

        if (type == CHARACTERTYPE.PLAYER) {
            playerstats_enable_penality_on_empty_strum(
                roundcontext.players[i].playerstats, SETTINGS.penality_on_empty_strum
            );
            roundcontext.players[i].controller = gamepad_init2(players[i].controller, mix_keyboard);
            roundcontext.players[i].ddrkeymon = ddrkeymon_init(
                roundcontext.players[i].controller, distribution.strum_binds, distribution.strums_size
            );
            roundcontext.players[i].ddrkeys_fifo = ddrkeymon_get_fifo(roundcontext.players[i].ddrkeymon);
        }

        // initialize the player components: controller+strums+conductor+character
        roundcontext.players[i].is_vertical = layout_strum.is_vertical;
        roundcontext.players[i].type = type;
        roundcontext.players[i].notepool = await notepool_init(
            distribution.notes,
            distribution.notes_size,
            layout_strum.marker_dimmen,
            layout_strum.invdimmen,
            STRUM_UPSCROLL
        );
        roundcontext.players[i].strums = await strums_init(
            layout_strum.x,
            layout_strum.y,
            layout_strum.z,
            layout_strum.marker_dimmen,
            layout_strum.invdimmen,
            layout_strum.length,
            layout_strum.gap,
            layout_strums_id,
            layout_strum.is_vertical,
            layout_strum.keep_marker_scale,
            distribution.strums,
            distribution.strums_size
        );
        strums_set_params(
            roundcontext.players[i].strums,
            roundcontext.players[i].ddrkeymon,
            roundcontext.players[i].playerstats,
            roundcontext.script
        );
        strums_set_notes(
            roundcontext.players[i].strums,
            chart,
            distribution.strums,
            distribution.strums_size,
            roundcontext.players[i].notepool
        );
        strums_set_scroll_speed(roundcontext.players[i].strums, chart.speed);
        strums_set_bpm(roundcontext.players[i].strums, chart.bpm);
        strums_use_funkin_maker_duration(roundcontext.players[i].strums, SETTINGS.use_funkin_marker_duration);

        // attach strums and notes states
        for (let j = 0; j < distribution.states_size; j++) {
            const state = distribution.states[j];
            let marker = null, sick_effect = null, background = null, notes = null;

            if (state.model_marker) marker = await modelholder_init(state.model_marker);
            if (state.model_sick_effect) sick_effect = await modelholder_init(state.model_sick_effect);
            if (state.model_background && await fs_file_exists(state.model_background)) background = await modelholder_init(state.model_background);
            if (state.model_notes) notes = await modelholder_init(state.model_notes);

            strums_state_add(
                roundcontext.players[i].strums, marker, sick_effect, background, state.name
            );
            if (state.model_notes)
                notepool_add_state(roundcontext.players[i].notepool, notes, state.name);

            if (marker) modelholder_destroy(marker);
            if (sick_effect) modelholder_destroy(sick_effect);
            if (background) modelholder_destroy(background);
            if (notes) modelholder_destroy(notes);
        }

        // attach all character states
        for (let j = 0; j < players[i].states_size; j++) {
            let modelholder = await modelholder_init(players[i].states[j].model);
            if (modelholder) {
                character_state_add(
                    roundcontext.players[i].character, modelholder, players[i].states[j].name
                );
                modelholder_destroy(modelholder);
            }
        }

        // set alpha of all sustain notes
        notepool_change_alpha_sustain(roundcontext.players[i].notepool, layout_strum.sustain_alpha);

        // toggle default state
        strums_state_toggle(roundcontext.players[i].strums, null);
        strums_enable_post_sick_effect_draw(roundcontext.players[i].strums, 1);

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

    for (let i = 0; i < roundcontext.players_size; i++) {
        if (!roundcontext.players[i].strums) continue;
        // strums-->conductors-->character
        conductor_use_strums(roundcontext.players[i].conductor, roundcontext.players[i].strums);
        let count = strums_get_lines_count(roundcontext.players[i].strums);
        let mapped = conductor_map_automatically(roundcontext.players[i].conductor, 0);

        if (mapped != count) {
            console.warn(`conductor_map_automatically() only mapped ${mapped} of ${count} lines`);
        }
    }

    if (roundcontext.players_size > 0) {
        if (!manifest_opponent) {
            manifest_opponent = charactermanifests[0];
        }
        if (!manifest_player && roundcontext.players_size > 1) {
            manifest_opponent = charactermanifests[1];
        }
    }

    week_change_scroll_direction(roundcontext);

    // set the health icon&color from the character manifest
    if (manifest_opponent) {
        roundcontext.healthbarparams.opponent_icon_model = strdup(
            manifest_opponent.model_health_icons
        );
        roundcontext.healthbarparams.has_opponent_color = manifest_opponent.has_healthbar_color;
        roundcontext.healthbarparams.opponent_color_rgb8 = manifest_opponent.healthbar_color;
    } else {
        roundcontext.healthbarparams.opponent_icon_model = null;
        roundcontext.healthbarparams.has_opponent_color = 0;
    }
    if (manifest_player) {
        roundcontext.healthbarparams.player_icon_model = strdup(
            manifest_player.model_health_icons
        );
        roundcontext.healthbarparams.has_player_color = manifest_player.has_healthbar_color;
        roundcontext.healthbarparams.player_color_rgb8 = manifest_player.healthbar_color;
    } else {
        roundcontext.healthbarparams.player_icon_model = null;
        roundcontext.healthbarparams.has_player_color = 0;
    }

    // dispose chart
    chart_destroy(chart);

    // dispose all charactermanifest loaded
    for (let i = 0; i < players_size; i++) charactermanifest_destroy(charactermanifests[i]);
    charactermanifests = undefined;

    // dispose old players array
    for (let i = 0; i < old_players_size; i++) {
        if (old_players[i].character) character_destroy(old_players[i].character);
        if (old_players[i].conductor) conductor_destroy(old_players[i].conductor);
        if (old_players[i].notepool) notepool_destroy(old_players[i].notepool);
        if (old_players[i].strums) strums_destroy(old_players[i].strums);
        if (old_players[i].ddrkeymon) ddrkeymon_destroy(old_players[i].ddrkeymon);
        if (old_players[i].controller) gamepad_destroy(old_players[i].controller);
    }
    old_players = undefined;

    return 1;
}

async function week_init_ui_cosmetics(/**@type {RoundContext}*/roundcontext) {
    const initparams = roundcontext.initparams;
    const viewport_size = [0, 0];
    let layout = roundcontext.layout ? roundcontext.layout : roundcontext.ui_layout;

    layout_get_viewport_size(roundcontext.ui_layout, viewport_size);
    let has_autoplace = autouicosmetics_prepare_placeholders(roundcontext.autouicosmetics, layout);

    let placeholder_streakcounter = week_internal_read_placeholder_counter(
        layout, "ui_streakcounter", !has_autoplace
    );
    let placeholder_rankingcounter_rank = week_internal_read_placeholder_counter(
        layout, "ui_rankingcounter_rank", !has_autoplace
    );
    let placeholder_rankingcounter_accuracy = week_internal_read_placeholder_counter(
        layout, "ui_rankingcounter_accuracy", !has_autoplace
    );

    // guess the position of streakcounter and rankingcounter if has missing placeholders
    if (has_autoplace) {
        if (placeholder_streakcounter) placeholder_streakcounter = AUTOUICOSMETICS_PLACEHOLDER_STREAK;
        if (placeholder_rankingcounter_rank) placeholder_rankingcounter_rank = AUTOUICOSMETICS_PLACEHOLDER_RANK;
        if (placeholder_rankingcounter_accuracy) placeholder_rankingcounter_accuracy = AUTOUICOSMETICS_PLACEHOLDER_ACCURACY;
    }

    // keep a copy of the old values
    let old_rankingcounter = roundcontext.rankingcounter;
    let old_streakcounter = roundcontext.streakcounter;
    let old_countdown = roundcontext.countdown;
    let old_songprogressbar = roundcontext.songprogressbar;
    if (roundcontext.roundstats) roundstats_destroy(roundcontext.roundstats);
    if (roundcontext.songinfo) textsprite_destroy(roundcontext.songinfo);

    // step 1: initialize all "cosmetic" components
    let modelholder_rankingstreak = await modelholder_init(UI_RANKINGCOUNTER_MODEL);
    let modelholder_streakcounter = await modelholder_init(UI_STREAKCOUNTER_MODEL);
    let modelholder_countdown = await modelholder_init(UI_COUNTDOWN_MODEL);

    // step 1a: ranking counter
    roundcontext.rankingcounter = rankingcounter_init(
        placeholder_rankingcounter_rank,
        placeholder_rankingcounter_accuracy,
        initparams.font
    );
    rankingcounter_add_state(roundcontext.rankingcounter, modelholder_rankingstreak, null);
    rankingcounter_toggle_state(roundcontext.rankingcounter, null);
    rankingcounter_set_default_ranking_animation(
        roundcontext.rankingcounter, initparams.animlist
    );
    rankingcounter_set_default_ranking_text_animation(
        roundcontext.rankingcounter, initparams.animlist
    );
    rankingcounter_use_percent_instead(
        roundcontext.rankingcounter,
        initparams.ui.rankingcounter_percentonly
    );

    // step 1b: initializae streak counter
    roundcontext.streakcounter = streakcounter_init(
        placeholder_streakcounter,
        initparams.ui.streakcounter_comboheight,
        initparams.ui.streakcounter_numbergap,
        initparams.ui.streakcounter_delay
    );
    streakcounter_state_add(
        roundcontext.streakcounter, modelholder_rankingstreak, modelholder_streakcounter, null
    );
    streakcounter_state_toggle(roundcontext.streakcounter, null);
    streakcounter_set_number_animation(
        roundcontext.streakcounter, initparams.animlist
    );
    streakcounter_set_combo_animation(
        roundcontext.streakcounter, initparams.animlist
    );

    // step 1c: initialize roundstats
    roundcontext.roundstats = roundstats_init(
        initparams.ui.roundstats_x,
        initparams.ui.roundstats_y,
        initparams.ui.roundstats_z,
        initparams.font,
        initparams.ui.roundstats_size,
        viewport_size[0]
    );
    roundstats_hide_nps(roundcontext.roundstats, initparams.ui.roundstats_hide);

    // step 1d: initialize songprogressbar
    if (SETTINGS.song_progressbar) {
        roundcontext.songprogressbar = await songprogressbar_init(
            initparams.ui.songprogressbar_x, initparams.ui.songprogressbar_y,
            initparams.ui.songprogressbar_z,
            initparams.ui.songprogressbar_width, initparams.ui.songprogressbar_height,
            initparams.ui.songprogressbar_align,
            initparams.ui.songprogressbar_bordersize, initparams.ui.songprogressbar_isvertical,
            initparams.ui.songprogressbar_showtime,
            initparams.font,
            initparams.ui.songprogressbar_fontsize, initparams.ui.songprogressbar_fontbordersize,
            initparams.ui.songprogressbar_colorrgba8_text,
            initparams.ui.songprogressbar_colorrgba8_background,
            initparams.ui.songprogressbar_colorrgba8_barback,
            initparams.ui.songprogressbar_colorrgba8_barfront
        );
        songprogressbar_set_songplayer(roundcontext.songprogressbar, roundcontext.songplayer);
        songprogressbar_set_duration(roundcontext.songprogressbar, roundcontext.round_duration);
        if (SETTINGS.song_progressbar_remaining) songprogressbar_show_elapsed(roundcontext.songprogressbar, 0);
    } else {
        roundcontext.songprogressbar = null;
    }

    // step 1e: initialize countdown
    roundcontext.countdown = await countdown_init(
        modelholder_countdown,
        initparams.ui.countdown_height
    );
    countdown_set_default_animation(roundcontext.countdown, initparams.animlist);
    countdown_set_layout_viewport(
        roundcontext.countdown, initparams.ui_layout_width, initparams.ui_layout_height
    );

    // step 1f: initialize songinfo
    roundcontext.songinfo = textsprite_init2(
        initparams.font, initparams.ui.songinfo_fontsize, initparams.ui.songinfo_fontcolor
    );
    textsprite_set_align(
        roundcontext.songinfo,
        initparams.ui.songinfo_alignvertical,
        initparams.ui.songinfo_alignhorinzontal
    );
    textsprite_set_max_draw_size(
        roundcontext.songinfo,
        initparams.ui.songinfo_maxwidth,
        initparams.ui.songinfo_maxheight
    );
    textsprite_set_draw_location(
        roundcontext.songinfo, initparams.ui.songinfo_x, initparams.ui.songinfo_y
    );
    textsprite_set_z_index(roundcontext.songinfo, initparams.ui.songinfo_z);
    textsprite_border_enable(roundcontext.songinfo, 1);
    textsprite_border_set_size(roundcontext.songinfo, ROUNDSTATS_FONT_BORDER_SIZE);
    textsprite_border_set_color_rgba8(roundcontext.songinfo, 0x000000FF);// black

    // step 2: dispose all modelholders used
    modelholder_destroy(modelholder_rankingstreak);
    modelholder_destroy(modelholder_streakcounter);
    modelholder_destroy(modelholder_countdown);

    // step 3: dispose old ui elements
    if (old_rankingcounter) rankingcounter_destroy(old_rankingcounter);
    if (old_streakcounter) streakcounter_destroy(old_streakcounter);
    if (old_countdown) countdown_destroy(old_countdown);
    if (old_songprogressbar) songprogressbar_destroy(old_songprogressbar);

    // step 4: drawn away if ui cosmetics are disabled
    if (!SETTINGS.gameplay_enabled_uicosmetics) {
        // drawn away
        if (placeholder_streakcounter) placeholder_streakcounter.vertex = null;
        if (placeholder_rankingcounter_rank) placeholder_rankingcounter_rank.vertex = null;
        if (placeholder_rankingcounter_accuracy) placeholder_rankingcounter_accuracy.vertex = null;
    }

    // step 5: pick drawables if "ui_autoplace_cosmetics" placeholder is present
    autouicosmetics_pick_drawables(roundcontext.autouicosmetics);
}

async function week_init_ui_gameover(/**@type {RoundContext}*/roundcontext) {
    let old_weekgameover = roundcontext.weekgameover;
    let version = await week_gameover_read_version();

    if (old_weekgameover && version == roundcontext.weekgameover_from_version) return;

    // build the gameover screen and dispose the older one
    roundcontext.weekgameover = await week_gameover_init();
    roundcontext.weekgameover_from_version = version;
    if (old_weekgameover) week_gameover_destroy(old_weekgameover);
}

async function week_init_dialogue(/**@type {RoundContext}*/roundcontext, dialogue_params, dialog_ignore_on_freeplay) {
    if (!dialogue_params) return;

    // dettach from the layout
    let layout = roundcontext.layout ?? roundcontext.ui_layout;
    let group_id = layout_get_group_id(layout, WEEKROUND_UI_GROUP_NAME2);
    if (!layout) return;// this never should happen

    if (roundcontext.dialogue) dialogue_destroy(roundcontext.dialogue);

    if (dialog_ignore_on_freeplay) {
        roundcontext.dialogue = null;
    } else {
        const size = [0, 0];
        layout_get_viewport_size(roundcontext.ui_layout, size);
        roundcontext.dialogue = await dialogue_init(dialogue_params, size[0], size[1]);
    }

    if (roundcontext.dialogue) {
        dialogue_set_script(roundcontext.dialogue, roundcontext.script);
        layout_external_vertex_set_entry(
            layout, 7, VERTEX_DRAWABLE, dialogue_get_drawable(roundcontext.dialogue), group_id
        );
    } else {
        layout_external_vertex_set_entry(layout, 7, VERTEX_DRAWABLE, null, group_id);
    }

}


function week_place_in_layout(roundcontext) {
    const initparams = roundcontext.initparams;
    const UI_SIZE = 9;// all UI "cosmetics" elements + screen background + dialogue

    let layout, is_ui;
    if (roundcontext.layout) {
        is_ui = 0;
        layout = roundcontext.layout;
        if (roundcontext.ui_layout) layout_external_vertex_create_entries(roundcontext.ui_layout, 0);
    } else {
        is_ui = 1;
        layout = roundcontext.ui_layout;
        if (roundcontext.layout) layout_external_vertex_create_entries(roundcontext.layout, 0);
    }

    // if there no stage layout, hide all characters
    if (is_ui) return;

    // step 1: declare the amout of items to add
    let size = UI_SIZE;
    for (let i = 0; i < roundcontext.players_size; i++) {
        // count strums
        if (roundcontext.players[i].strums) size++;
    }

    let ui1 = layout_external_create_group(layout, WEEKROUND_UI_GROUP_NAME, 0);
    if (ui1 < 0) ui1 = layout_get_group_id(layout, WEEKROUND_UI_GROUP_NAME);
    layout_set_group_static_to_screen_by_id(layout, ui1, WEEKROUND_UI_MATRIX_CAMERA);

    let ui2 = layout_external_create_group(layout, WEEKROUND_UI_GROUP_NAME2, 0);
    if (ui2 < 0) ui2 = layout_get_group_id(layout, WEEKROUND_UI_GROUP_NAME2);
    layout_set_group_static_to_screen_by_id(layout, ui2, WEEKROUND_UI_MATRIX);

    // allocate space for all ui items
    layout_external_vertex_create_entries(roundcontext.layout, size);

    // step 2: place all UI elements
    layout_external_vertex_set_entry(
        layout, 0, VERTEX_DRAWABLE, healthbar_get_drawable(roundcontext.healthbar), ui1
    );
    layout_external_vertex_set_entry(
        layout, 1, VERTEX_DRAWABLE, roundstats_get_drawable(roundcontext.roundstats), ui1
    );
    layout_external_vertex_set_entry(
        layout, 2, VERTEX_DRAWABLE, roundcontext.songprogressbar ? songprogressbar_get_drawable(roundcontext.songprogressbar) : null, ui1
    );
    layout_external_vertex_set_entry(
        layout, 3, VERTEX_DRAWABLE, countdown_get_drawable(roundcontext.countdown), ui1
    );
    layout_external_vertex_set_entry(
        layout, 4, VERTEX_TEXTSPRITE, roundcontext.songinfo, ui1
    );
    layout_external_vertex_set_entry(
        layout, 5, VERTEX_DRAWABLE, week_gameover_get_drawable(roundcontext.weekgameover), ui2
    );
    layout_external_vertex_set_entry(
        layout, 6, VERTEX_SPRITE, roundcontext.screen_background, ui2
    );
    layout_external_vertex_set_entry(
        layout, 7, VERTEX_DRAWABLE, roundcontext.dialogue ? dialogue_get_drawable(roundcontext.dialogue) : null, ui2
    );
    layout_external_vertex_set_entry(
        layout, 8, VERTEX_DRAWABLE,
        roundcontext.autouicosmetics.drawable_self, roundcontext.autouicosmetics.layout_group_id
    );

    // step 3: initialize the ui camera
    let modifier = camera_get_modifier(roundcontext.ui_camera);
    camera_stop(roundcontext.ui_camera);
    modifier.width = pvr_context.screen_width;
    modifier.height = pvr_context.screen_height;

    // step 4: place girlfriend
    if (roundcontext.girlfriend) {
        if (initparams.layout_girlfriend) {
            week_internal_place_character(
                layout,
                roundcontext.girlfriend,
                roundcontext.initparams.layout_girlfriend
            );
        } else {
            console.error("week_place_in_layout() missing layout space for declared girlfriend");
        }
    }

    // step 5: place all player characters & strums
    for (let i = 0, j = UI_SIZE; i < roundcontext.players_size; i++) {
        if (roundcontext.players[i].character) {
            if (i < initparams.layout_characters_size) {
                week_internal_place_character(
                    layout,
                    roundcontext.players[i].character,
                    initparams.layout_characters[i]
                );
            } else {
                console.error(`week_place_in_layout() missing layout space for declared player[${i}]`);
            }
        }
        if (roundcontext.players[i].strums) {
            layout_external_vertex_set_entry(
                layout, j++, VERTEX_DRAWABLE, strums_get_drawable(roundcontext.players[i].strums), ui1
            );
        }
    }

    week_ui_set_visibility(roundcontext, 1);
    return;
}

function week_toggle_states(roundcontext, gameplaymanifest) {
    const song = gameplaymanifest.songs[roundcontext.song_index];

    if (song.has_selected_state_name) {
        for (let i = 0; i < roundcontext.players_size; i++) {
            if (roundcontext.players[i].strums)
                strums_state_toggle(roundcontext.players[i].strums, song.selected_state_name);
            if (roundcontext.players[i].character)
                character_state_toggle(roundcontext.players[i].character, song.selected_state_name);
        }
        healthbar_state_toggle(roundcontext.healthbar, song.selected_state_name);
    }

    let size = song.selected_state_name_per_player_size;
    if (roundcontext.players_size < size) size = roundcontext.players_size;
    for (let i = 0; i < size; i++) {
        const state_name = song.selected_state_name_per_player[i];
        if (roundcontext.players[i].strums)
            strums_state_toggle(roundcontext.players[i].strums, state_name);
        if (roundcontext.players[i].character)
            character_state_toggle(roundcontext.players[i].character, state_name);
    }
}

function week_update_bpm(roundcontext, bpm) {
    if (roundcontext.healthbar) healthbar_set_bpm(roundcontext.healthbar, bpm);
    if (roundcontext.countdown) countdown_set_bpm(roundcontext.countdown, bpm);
    if (roundcontext.roundstats) roundstats_tweenkeyframe_set_bpm(roundcontext.roundstats, bpm);
    if (roundcontext.ui_camera) camera_set_bpm(roundcontext.ui_camera, bpm);
    if (roundcontext.layout) layout_set_bpm(roundcontext.layout, bpm);
    for (let i = 0; i < roundcontext.players_size; i++) {
        if (roundcontext.players[i].character) {
            character_set_bpm(roundcontext.players[i].character, bpm);
        }
        if (roundcontext.players[i].strums) {
            strums_set_bpm(roundcontext.players[i].strums, bpm);
        }
    }
    beatwatcher_change_bpm(WEEK_BEAT_WATCHER, bpm);
    beatwatcher_change_bpm(WEEK_QUARTER_WATCHER, bpm);

    //console.log(`week_update_bpm() original=${roundcontext.settings.original_bpm} new=${bpm}`);

    roundcontext.settings.bpm = bpm;
}

function week_update_speed(/** @type {RoundContext} */roundcontext, speed) {
    for (let i = 0; i < roundcontext.players_size; i++) {
        if (roundcontext.players[i].character) {
            character_set_idle_speed(roundcontext.players[i].character, speed);
        }
    }

    if (roundcontext.girlfriend) character_set_idle_speed(roundcontext.girlfriend, speed);

    roundcontext.settings.speed = speed;
}

function week_disable_layout_rollback(/** @type {RoundContext} */roundcontext, disable) {
    roundcontext.settings.layout_rollback = !disable;
}

function week_override_common_folder(/** @type {RoundContext} */roundcontext, custom_common_path) {
    fs_override_common_folder(custom_common_path);
}


async function week_round(/** @type {RoundContext} */roundcontext, from_retry, show_dialog) {
    const pressed_buttons = [0x00];
    const songinfo = { timestamp: NaN, completed: true };
    const initparams = roundcontext.initparams;

    let elapsed;
    let check_ready = roundcontext.settings.ask_ready;
    let do_countdown = roundcontext.settings.do_countdown;
    let playerstats = null;
    let layout = roundcontext.layout ?? roundcontext.ui_layout;
    let camera = layout_get_camera_helper(layout);
    let round_duration = roundcontext.round_duration;
    let round_end_timestamp;
    let unmute_timestamp = Infinity;

    window["roundcontext"] = roundcontext;

    if (round_duration < 0) round_duration = Infinity;
    layout_resume(layout);

    if (roundcontext.songprogressbar) {
        let duration = roundcontext.songplayer ? songplayer_get_duration(roundcontext.songplayer) : round_duration;
        songprogressbar_manual_update_enable(roundcontext.songprogressbar, 1);
        songprogressbar_manual_set_position(roundcontext.songprogressbar, 0, duration, 1);
    }

    if (roundcontext.script) {
        await weekscript_notify_timersong(roundcontext.script, 0.0);
        await weekscript_notify_beforeready(roundcontext.script, from_retry);
    }
    await week_halt(roundcontext, 1);

    if (roundcontext.playerstats_index >= 0) {
        playerstats = roundcontext.players[roundcontext.playerstats_index].playerstats;
        roundstats_peek_playerstats(roundcontext.roundstats, 0, playerstats);
    }

    while (show_dialog) {
        elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);
        week_internal_check_screen_resolution(roundcontext, 0);
        beatwatcher_global_set_timestamp_from_kos_timer();

        if (pvr_is_offscreen(pvr_context)) {
            layout_suspend(layout);
            dialogue_suspend(roundcontext.dialogue);

            let decision = await week_pause_helper_show(roundcontext.weekpause, roundcontext, -1);
            switch (decision) {
                case 1:
                    return 2;// restart song
                case 2:
                    return 1;// back to weekselector
                case 3:
                    return 3;// back to mainmenu
            }

            layout_resume(layout);
            dialogue_resume(roundcontext.dialogue);
            continue;
        }

        if (roundcontext.script != null) await weekscript_notify_frame(roundcontext.script, elapsed);
        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        if (dialogue_is_completed(roundcontext.dialogue)) {
            show_dialog = 0;
            week_internal_do_antibounce(roundcontext);
        }

        await week_halt(roundcontext, 1);
    }

    if (check_ready) countdown_ready(roundcontext.countdown);
    else if (do_countdown) countdown_start(roundcontext.countdown);

    let dettached_controller_index = -1;
    let back_pressed = 0;

    while (check_ready || do_countdown) {
        elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);

        week_internal_check_screen_resolution(roundcontext, false);
        beatwatcher_global_set_timestamp_from_kos_timer();

        if (pvr_is_offscreen(pvr_context) || dettached_controller_index >= 0 || back_pressed) {
            let decision = await week_pause_helper_show(
                roundcontext.weekpause, roundcontext, dettached_controller_index
            );
            dettached_controller_index = -1;
            switch (decision) {
                case 1:
                    return 2;// restart song
                case 2:
                    return 1;// back to weekselector
                case 3:
                    return 3;// back to mainmenu
            }

            for (let i = 0; i < roundcontext.players_size; i++) {
                const controller = roundcontext.players[i].controller;
                if (controller) gamepad_clear_buttons(controller);
            }
            continue;
        }

        if (check_ready) {
            let is_ready = 0;
            let bot_only = 1;

            for (let i = 0; i < roundcontext.players_size; i++) {
                let controller = roundcontext.players[i].controller;
                if (!controller) continue;
                bot_only = 0;
                if (gamepad_is_dettached(controller)) {
                    dettached_controller_index = gamepad_get_controller_index(controller);
                    break;
                }
                if (gamepad_has_pressed(controller, WEEKROUND_READY_BUTTONS)) {
                    gamepad_clear_buttons(controller);// antibouce
                    is_ready++;
                }
                if (gamepad_has_pressed(controller, GAMEPAD_BACK) != 0x00) {
                    is_ready = 0;
                    back_pressed = 1;
                    break;
                }

                if (gamepad_get_managed_presses(controller, 0, pressed_buttons) && roundcontext.script) {
                    await weekscript_notify_buttons(roundcontext.script, i, pressed_buttons[0]);
                }
            }

            if (is_ready || bot_only) {
                check_ready = 0;

                if (roundcontext.script) {
                    await weekscript_notify_ready(roundcontext.script);
                    await week_halt(roundcontext, 1);
                }

                if (do_countdown) countdown_start(roundcontext.countdown);
                layout_trigger_camera(layout, WEEKROUND_CAMERA_ROUNDSTART);
            }
        } else if (countdown_has_ended(roundcontext.countdown)) {
            do_countdown = 0;
        }

        if (roundcontext.scriptcontext.force_end_flag) {
            if (roundcontext.scriptcontext.force_end_loose_or_win)
                return 1;// give-up and returns to the week selector
            else
                return 0;
        }

        if (roundcontext.script) await weekscript_notify_frame(roundcontext.script, elapsed);
        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);
    }

    if (!roundcontext.settings.ask_ready) layout_trigger_camera(layout, WEEKROUND_CAMERA_ROUNDSTART);

    if (roundcontext.songprogressbar) songprogressbar_manual_update_enable(roundcontext.songprogressbar, 0);

    if (roundcontext.script) await weekscript_notify_aftercountdown(roundcontext.script);
    await week_halt(roundcontext, 1);

    // start this round!!!!!
    dettached_controller_index = -1;
    elapsed = roundcontext.songplayer ? (await songplayer_play(roundcontext.songplayer, songinfo)) : 0.0;

    // prepare beatwatchers
    beatwatcher_global_set_timestamp(elapsed);
    beatwatcher_reset(WEEK_BEAT_WATCHER, 1, roundcontext.settings.bpm);
    beatwatcher_reset(WEEK_QUARTER_WATCHER, 0, roundcontext.settings.bpm);

    let gameover = 0;
    let next_camera_bump = 0;
    let has_reference_ddrkeymon = null;
    let song_timestamp = 0.0;

    round_end_timestamp = timer_ms_gettime64() + round_duration;

    for (let i = 0; i < roundcontext.players_size; i++) {
        if (roundcontext.players[i].ddrkeymon) {
            gamepad_clear_buttons(roundcontext.players[i].controller);
            has_reference_ddrkeymon = roundcontext.players[i].ddrkeymon;
            ddrkeymon_start(roundcontext.players[i].ddrkeymon, songinfo.timestamp);
        }
    }

    // gameplay logic
    while (timer_ms_gettime64() < round_end_timestamp && !songinfo.completed) {
        // wait for frame
        elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);

        week_internal_check_screen_resolution(roundcontext, false);

        // check for pause
        let paused = 0;
        for (let i = 0; i < roundcontext.players_size; i++) {
            if (!roundcontext.players[i].controller) continue;
            if (gamepad_is_dettached(roundcontext.players[i].controller)) {
                dettached_controller_index = gamepad_get_controller_index(
                    roundcontext.players[i].controller
                );
                paused = 1;
                break;
            }

            // important: use gamepad_get_last_pressed() to avoid mess up the maple pad or keyboard inputs
            if (gamepad_get_last_pressed(roundcontext.players[i].controller) & (GAMEPAD_START | GAMEPAD_BACK)) {
                paused = 1;
                break;
            }
        }

        if (paused || pvr_is_offscreen(pvr_context)) {
            // pause all critical stuff
            round_duration = round_end_timestamp - timer_ms_gettime64();
            layout_suspend(layout);
            if (roundcontext.songplayer) songplayer_pause(roundcontext.songplayer);
            for (let i = 0; i < roundcontext.players_size; i++) {
                if (roundcontext.players[i].ddrkeymon) {
                    ddrkeymon_stop(roundcontext.players[i].ddrkeymon);
                }
            }

            // display pause menu (visible when the screen focus returns)
            let decision = await week_pause_helper_show(
                roundcontext.weekpause, roundcontext, dettached_controller_index
            );
            dettached_controller_index = -1;
            switch (decision) {
                case 1:
                    return 2;// restart song
                case 2:
                    return 1;// back to weekselector
                case 3:
                    return 3;// back to mainmenu
            }

            // resume
            round_end_timestamp = timer_ms_gettime64() + round_duration;
            layout_resume(layout);

            if (roundcontext.songplayer) {
                songplayer_seek(roundcontext.songplayer, song_timestamp);
                await songplayer_play(roundcontext.songplayer, songinfo);
            }

            for (let i = 0; i < roundcontext.players_size; i++) {
                if (roundcontext.players[i].ddrkeymon) {
                    gamepad_clear_buttons(roundcontext.players[i].controller);
                    ddrkeymon_start(roundcontext.players[i].ddrkeymon, -song_timestamp);
                }
            }

            continue;
        }

        if (roundcontext.songplayer) songplayer_poll(roundcontext.songplayer, songinfo);

        if (has_reference_ddrkeymon)
            song_timestamp = ddrkeymon_peek_timestamp(has_reference_ddrkeymon);
        else
            song_timestamp = songinfo.timestamp;

        // update beatwatchers
        beatwatcher_global_set_timestamp(song_timestamp);

        // Important: use only in key checking and/or strum scroll
        let song_timestamp2 = song_timestamp;

        //
        // If strums_set_draw_offset() is not used, enable this line
        //
        song_timestamp2 -= SETTINGS.input_offset;

        let missed_milliseconds = 0;

        for (let i = 0; i < roundcontext.players_size; i++) {
            switch (roundcontext.players[i].type) {
                case CHARACTERTYPE.BOT:
                    strums_scroll_auto(roundcontext.players[i].strums, song_timestamp2);
                    conductor_poll(roundcontext.players[i].conductor);
                    missed_milliseconds += conductor_get_missed_milliseconds(roundcontext.players[i].conductor);
                    break;
                case CHARACTERTYPE.PLAYER:
                    ddrkeymon_poll_CSJS(roundcontext.players[i].ddrkeymon);
                    strums_scroll_full(roundcontext.players[i].strums, song_timestamp2);
                    conductor_poll(roundcontext.players[i].conductor);
                    if (gamepad_get_managed_presses(roundcontext.players[i].controller, 0, pressed_buttons)) {
                        if (roundcontext.script) await weekscript_notify_buttons(roundcontext.script, i, pressed_buttons[0]);
                    }
                    missed_milliseconds += conductor_get_missed_milliseconds(roundcontext.players[i].conductor);
                    break;
            }
        }

        if (roundcontext.script) {
            await weekscript_notify_after_strum_scroll(roundcontext.script);
            if (roundcontext.scriptcontext.halt_flag) await week_halt(roundcontext, 0);
        }

        if (playerstats) {
            rankingcounter_peek_ranking(roundcontext.rankingcounter, playerstats);
            if (streakcounter_peek_streak(roundcontext.streakcounter, playerstats)) {
                if (roundcontext.girlfriend && roundcontext.settings.girlfriend_cry) {
                    character_play_extra(roundcontext.girlfriend, FUNKIN_GIRLFRIEND_COMBOBREAK, 0);
                }
            }
            roundstats_peek_playerstats(roundcontext.roundstats, songinfo.timestamp, playerstats);
        }

        healthwatcher_balance(roundcontext.healthwatcher, roundcontext.healthbar);

        if (healthwatcher_has_deads(roundcontext.healthwatcher, 1)) {
            gameover = 1;
            break;
        }

        if (roundcontext.songplayer) {
            if (/*Number.isFinite(missed_milliseconds) && */missed_milliseconds > 0) {
                if (Number.isFinite(unmute_timestamp)) {
                    unmute_timestamp += missed_milliseconds / 2;
                    songplayer_mute_track(roundcontext.songplayer, 1, 1);
                } else {
                    unmute_timestamp = song_timestamp + missed_milliseconds;
                    songplayer_mute_track(roundcontext.songplayer, 1, 1);
                }
            } else if (song_timestamp > unmute_timestamp) {
                unmute_timestamp = Infinity;
                songplayer_mute_track(roundcontext.songplayer, 1, 0);
            }
        }

        await week_peek_chart_events(roundcontext, song_timestamp);

        if (beatwatcher_poll(WEEK_BEAT_WATCHER)) {
            // bump UI
            if (WEEK_BEAT_WATCHER.count > next_camera_bump) {
                next_camera_bump += 4;
                if (roundcontext.settings.camera_bumping) {
                    camera_animate(camera, WEEK_BEAT_WATCHER.since);
                    camera_slide(roundcontext.ui_camera, NaN, NaN, 1.05, NaN, NaN, 1);
                }
            }

            // notify script
            if (roundcontext.script) {
                await weekscript_notify_beat(
                    roundcontext.script, WEEK_BEAT_WATCHER.count, WEEK_BEAT_WATCHER.since
                );
            }
        }

        if (beatwatcher_poll(WEEK_QUARTER_WATCHER) && roundcontext.script) {
            await weekscript_notify_quarter(
                roundcontext.script, WEEK_QUARTER_WATCHER.count, WEEK_QUARTER_WATCHER.since
            );
        }

        // animate camera
        camera_animate(roundcontext.ui_camera, elapsed);

        // flush modifier
        camera_apply(roundcontext.ui_camera, null);
        sh4matrix_copy_to(WEEKROUND_UI_MATRIX, WEEKROUND_UI_MATRIX_CAMERA);
        camera_apply_offset(roundcontext.ui_camera, WEEKROUND_UI_MATRIX_CAMERA);
        sh4matrix_apply_modifier(WEEKROUND_UI_MATRIX_CAMERA, camera_get_modifier(roundcontext.ui_camera));

        if (roundcontext.script) {
            await weekscript_notify_timersong(roundcontext.script, song_timestamp);
            await weekscript_notify_frame(roundcontext.script, elapsed);
        }
        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        // JS only
        if (roundcontext.scriptcontext.halt_flag) await week_halt(roundcontext, 0);

        if (roundcontext.scriptcontext.force_end_flag) {
            if (roundcontext.songplayer) songplayer_pause(roundcontext.songplayer);
            if (roundcontext.scriptcontext.force_end_loose_or_win) {
                return 1;// give-up and returns to the week selector
            } else {
                return 0;// interrupt the gameplay
            }
        }
    }

    // stop all ddrkeymon instances
    for (let i = 0; i < roundcontext.players_size; i++) {
        if (roundcontext.players[i].ddrkeymon) {
            ddrkeymon_stop(roundcontext.players[i].ddrkeymon);
        }
    }

    // notify the script about the current round result and halt (if necessary)
    if (roundcontext.songplayer) songplayer_pause(roundcontext.songplayer);
    layout_trigger_camera(layout, WEEKROUND_CAMERA_ROUNDEND);
    if (roundcontext.script) {
        await weekscript_notify_roundend(roundcontext.script, gameover);
        await week_halt(roundcontext, 1);
    }
    week_ui_set_visibility(roundcontext, 0);

    if (gameover) {
        let duration = roundcontext.round_duration;
        if (roundcontext.songplayer) {
            if (duration < 0) duration = songplayer_get_duration(roundcontext.songplayer);
            songplayer_pause(roundcontext.songplayer);
        }

        week_gameover_display(
            roundcontext.weekgameover,
            song_timestamp,
            duration,
            playerstats,
            initparams.weekinfo,
            initparams.difficult
        );

        // ask for player decision
        let decision = await week_gameover_helper_ask_to_player(roundcontext.weekgameover, roundcontext);
        let song_difficult = week_gameover_get_difficult(roundcontext.weekgameover);
        week_gameover_hide(roundcontext.weekgameover);

        // notify script and wait (if necessary)
        if (roundcontext.script) {
            let change = roundcontext.song_difficult === song_difficult ? null : song_difficult;
            await weekscript_notify_diedecision(roundcontext.script, decision, change);
            await week_halt(roundcontext, 1);
        }

        roundcontext.song_difficult = song_difficult;

        return decision;
    }

    return 0;// round win
}


async function week_halt(/** @type {RoundContext} */roundcontext, peek_global_beatwatcher) {
    if (!roundcontext.scriptcontext.halt_flag) return;

    const preesed = [0x00];
    const layout = roundcontext.layout ?? roundcontext.ui_layout;

    console.log("week_halt() waiting for script signal...");

    if (peek_global_beatwatcher) beatwatcher_global_set_timestamp_from_kos_timer();

    while (roundcontext.scriptcontext.halt_flag) {
        let elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);

        week_internal_check_screen_resolution(roundcontext, false);

        for (let i = 0; i < roundcontext.players_size; i++) {
            let controller = roundcontext.players[i].controller;
            if (controller && gamepad_get_managed_presses(controller, 1, preesed) && roundcontext.script) {
                await weekscript_notify_buttons(roundcontext.script, i, preesed[0]);
            }
        }

        if (roundcontext.script) await weekscript_notify_frame(roundcontext.script, elapsed);
        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        if (peek_global_beatwatcher) beatwatcher_global_set_timestamp_from_kos_timer();

        if (roundcontext.scriptcontext.force_end_flag) {
            console.log("week_halt() wait interrupted because week_end() was called");
            return;
        }
    }

    week_internal_do_antibounce(roundcontext);

    console.log("week_halt() wait done");
}

async function week_peek_chart_events(/** @type {RoundContext} */roundcontext, timestamp) {
    let i = roundcontext.events_peek_index;

    for (; i < roundcontext.events_size; i++) {
        if (timestamp < roundcontext.events[i].timestamp) break;
        switch (roundcontext.events[i].command) {
            case CHART_EVENT_CAMERA_OPPONENT:
                week_camera_focus_guess(roundcontext, WEEKROUND_CAMERA_OPONNENT, -1);
                break;
            case CHART_EVENT_CAMERA_PLAYER:
                week_camera_focus_guess(roundcontext, WEEKROUND_CAMERA_PLAYER, -1);
                break;
            case CHART_EVENT_CHANGE_BPM:
                week_update_bpm(roundcontext, roundcontext.events[i].parameter);
                break;
            case CHART_EVENT_ALT_ANIM_OPPONENT:
                for (let j = 0; j < roundcontext.players_size; j++) {
                    if (roundcontext.players[j].is_opponent) {
                        character_use_alternate_sing_animations(
                            roundcontext.players[j].character, roundcontext.events[i].parameter
                        );
                    }
                }
                break;
            case CHART_EVENT_ALT_ANIM_PLAYER:
                for (let j = 0; j < roundcontext.players_size; j++) {
                    if (roundcontext.players[j].type == CHARACTERTYPE.PLAYER) {
                        character_use_alternate_sing_animations(
                            roundcontext.players[j].character, roundcontext.events[i].parameter
                        );
                    }
                }
                break;
            case CHART_EVENT_UNKNOWN_NOTE:
                if (roundcontext.script) {
                    await weekscript_notify_unknownnote(
                        roundcontext.script,
                        roundcontext.events[i].is_player_or_opponent ? 1 : 0,
                        roundcontext.events[i].timestamp,
                        roundcontext.events[i].parameter,
                        roundcontext.events[i].parameter2,
                        roundcontext.events[i].parameter3
                    );
                }
                break;
        }
    }

    if (i != roundcontext.events_peek_index) roundcontext.events_peek_index = i;
}


function week_check_directives_round(roundcontext, completed) {
    for (let directive_info of linkedlist_iterate4(roundcontext.scriptcontext.directives)) {
        if (directive_info.completed_week) continue;

        if (completed || !directive_info.completed_round) {
            if (directive_info.create)
                funkinsave_create_unlock_directive(directive_info.name, directive_info.value);
            else
                funkinsave_delete_unlock_directive(directive_info.name);
        }

        // remove this item from the list
        linkedlist_remove_item(roundcontext.scriptcontext.directives, directive_info);
        directive_info.name = undefined;
        directive_info = undefined;
    }
}

function week_check_directives_week(roundcontext, completed) {
    for (let directive_info of linkedlist_iterate4(roundcontext.scriptcontext.directives)) {
        if (completed || !directive_info.completed_week) {
            if (directive_info.create)
                funkinsave_create_unlock_directive(directive_info.name, directive_info.value);
            else
                funkinsave_delete_unlock_directive(directive_info.name);
        }

        // remove this item from the list
        linkedlist_remove_item(roundcontext.scriptcontext.directives, directive_info);
        directive_info.name = undefined;
        directive_info = undefined;
        roundcontext.has_directive_changes = 1;
    }
}

function week_change_scroll_direction(roundcontext) {
    let direction;
    for (let i = 0; i < roundcontext.players_size; i++) {
        if (!roundcontext.players[i].strums) continue;

        if (roundcontext.players[i].is_vertical)
            direction = SETTINGS.inverse_strum_scroll ? STRUM_DOWNSCROLL : STRUM_UPSCROLL;
        else
            direction = SETTINGS.inverse_strum_scroll ? STRUM_RIGHTSCROLL : STRUM_LEFTSCROLL;

        strums_set_scroll_direction(roundcontext.players[i].strums, direction);
        notepool_change_scroll_direction(roundcontext.players[i].notepool, direction);
    }
}

function week_unlockdirective_create(roundcontext, name, completed_round, completed_week, value) {
    if (!name) return;
    if (typeof (value) != "number") value = 0;

    for (let directive_info of linkedlist_iterate4(roundcontext.scriptcontext.directives)) {
        if (directive_info.name == name) {
            directive_info.completed_round = !!completed_round;
            directive_info.completed_week = !!completed_week;
            directive_info.value = value;
            directive_info.create = 1;
            return;
        }
    }

    linkedlist_add_item(roundcontext.scriptcontext.directives, {
        name: strdup(name),
        completed_round: !!completed_round,
        completed_week: !!completed_week,
        value: value,
        create: 1
    });
}

function week_unlockdirective_remove(roundcontext, name, completed_round, completed_week) {
    for (let directive_info of linkedlist_iterate4(roundcontext.scriptcontext.directives)) {
        if (directive_info.name == name) {
            directive_info.completed_round = !!completed_round;
            directive_info.completed_week = !!completed_week;
            directive_info.create = 0;
        }
    }
    linkedlist_add_item(roundcontext.scriptcontext.directives, {
        name: strdup(name),
        completed_round: !!completed_round,
        completed_week: !!completed_week,
        value: 0x00,
        create: 0
    });
}

function week_unlockdirective_get(roundcontext, name) {
    const value = [0];
    funkinsave_read_unlock_directive(name, value);
    return value[0];
}

function week_unlockdirective_has(roundcontext, name) {
    return funkinsave_contains_unlock_directive(name);
}

function week_storage_get(roundcontext, name, out_data) {
    return funkinsave_storge_get(roundcontext.initparams.weekinfo.name, name, out_data);
}

function week_storage_set(roundcontext, name, data, data_size) {
    let ret = funkinsave_storge_set(roundcontext.initparams.weekinfo.name, name, data, data_size);
    if (ret) roundcontext.has_directive_changes = 1;
    return ret;
}

function week_get_songplayer(/**@type {RoundContext}*/roundcontext) {
    return roundcontext.songplayer;
}

function week_get_stage_layout(/**@type {RoundContext}*/roundcontext) {
    return roundcontext.layout == roundcontext.ui_layout ? null : roundcontext.layout;
}

function week_ui_get_layout(/**@type {RoundContext}*/roundcontext) {
    return roundcontext.ui_layout;
}

function week_ui_get_camera(/**@type {RoundContext}*/roundcontext) {
    return roundcontext.ui_camera;
}

function week_set_halt(/**@type {RoundContext}*/roundcontext, halt) {
    roundcontext.scriptcontext.halt_flag = halt;
}

function week_disable_week_end_results(/**@type {RoundContext} */ roundcontext, disable) {
    roundcontext.scriptcontext.no_week_end_result_screen = disable;
}

function week_disable_girlfriend_cry(/**@type {RoundContext} */ roundcontext, disable) {
    roundcontext.settings.girlfriend_cry = !disable;
}

function week_disable_ask_ready(/**@type {RoundContext} */ roundcontext, disable) {
    roundcontext.settings.ask_ready = !disable;
}

function week_disable_countdown(/**@type {RoundContext} */ roundcontext, disable) {
    roundcontext.settings.do_countdown = !disable;
}

function week_disable_camera_bumping(/**@type {RoundContext} */ roundcontext, disable) {
    roundcontext.settings.camera_bumping = !disable;
}


function week_get_girlfriend(/**@type {RoundContext}*/roundcontext) {
    return roundcontext.girlfriend;
}

function week_get_character_count(/**@type {RoundContext}*/roundcontext) {
    return roundcontext.players_size;
}

function week_get_character(/**@type {RoundContext}*/roundcontext, index) {
    if (index < 0 || index >= roundcontext.players_size) return null;
    return roundcontext.players[index].character;
}

function week_get_messagebox(/**@type {RoundContext}*/roundcontext) {
    return roundcontext.messagebox;
}

function week_ui_get_strums_count(/**@type {RoundContext}*/roundcontext) {
    return roundcontext.initparams.layout_strums_size;
}

function week_ui_get_round_textsprite(/**@type {RoundContext}*/roundcontext) {
    return roundcontext.songinfo;
}

function week_ui_set_visibility(/**@type {RoundContext} */ roundcontext, visible) {
    layout_set_group_visibility(roundcontext.layout, WEEKROUND_UI_GROUP_NAME, visible);
}

function week_get_current_chart_info(/**@type {RoundContext} */ roundcontext, chartinfo) {
    chartinfo.bpm = roundcontext.settings.original_bpm;
    chartinfo.speed = roundcontext.settings.camera_name_player;
}

function week_get_current_song_info(/**@type {RoundContext} */ roundcontext, songinfo) {
    if (!songinfo) return;
    songinfo.name = roundcontext.initparams.gameplaymanifest.songs[roundcontext.song_index].name;
    songinfo.difficult = roundcontext.song_difficult;
    songinfo.index = roundcontext.song_index;
}

function week_change_character_camera_name(/**@type {RoundContext} */ roundcontext, opponent_or_player, new_name) {
    const settings = roundcontext.settings;

    if (opponent_or_player) {
        if (settings.camera_name_opponent != WEEKROUND_CAMERA_OPONNENT) settings.camera_name_opponent = undefined;
        settings.camera_name_opponent = new_name == null ? WEEKROUND_CAMERA_OPONNENT : strdup(new_name);
    } else {
        if (settings.camera_name_player != WEEKROUND_CAMERA_PLAYER) settings.camera_name_player = undefined;
        settings.camera_name_player = new_name == null ? WEEKROUND_CAMERA_PLAYER : strdup(new_name);
    }
}

function week_enable_credits_on_completed(/**@type {RoundContext} */ roundcontext) {
    roundcontext.settings.show_credits = 1;
}

function week_end(/**@type {RoundContext} */ roundcontext, round_or_week, loose_or_win) {
    roundcontext.scriptcontext.force_end_flag = 1;
    roundcontext.scriptcontext.force_end_round_or_week = round_or_week;
    roundcontext.scriptcontext.force_end_loose_or_win = loose_or_win;
}

function week_get_dialogue(/**@type {RoundContext} */ roundcontext) {
    return roundcontext.dialogue;
}

function week_set_ui_shader(/**@type {RoundContext} */ roundcontext, psshader) {
    let layout = roundcontext.layout ?? roundcontext.ui_layout;
    layout_set_group_shader(layout, WEEKROUND_UI_GROUP_NAME, psshader);
}

function week_get_conductor(/**@type {RoundContext} */ roundcontext, character_index) {
    if (character_index < 0 || character_index >= roundcontext.players_size) return null;
    return roundcontext.players[character_index].conductor;
}

function week_get_healthwatcher(/**@type {RoundContext} */ roundcontext) {
    return roundcontext.healthwatcher;
}

function week_get_missnotefx(/**@type {RoundContext} */ roundcontext) {
    return roundcontext.missnotefx;
}

function week_get_playerstats(/**@type {RoundContext} */ roundcontext, character_index) {
    if (character_index < 0 || character_index >= roundcontext.players_size) return null;
    return roundcontext.players[character_index].playerstats;
}

async function week_rebuild_ui(/**@type {RoundContext} */ roundcontext) {
    await week_init_ui_cosmetics(roundcontext);
}

function week_ui_get_countdown(/**@type {RoundContext} */ roundcontext) {
    return roundcontext.countdown;
}

function week_ui_get_healthbar(/**@type {RoundContext} */ roundcontext) {
    return roundcontext.healthbar;
}

function week_ui_get_rankingcounter(/**@type {RoundContext} */ roundcontext) {
    return roundcontext.rankingcounter;
}

function week_ui_get_roundstats(/**@type {RoundContext} */ roundcontext) {
    return roundcontext.roundstats;
}

function week_ui_get_songprogressbar(/**@type {RoundContext} */ roundcontext) {
    return roundcontext.songprogressbar;
}

function week_ui_get_streakcounter(/**@type {RoundContext} */ roundcontext) {
    return roundcontext.streakcounter;
}

function week_ui_get_strums(/**@type {RoundContext} */ roundcontext, strums_id) {
    const initparams = roundcontext.initparams;
    const gameplaymanifest = initparams.gameplaymanifest;
    const song_index = roundcontext.song_index;

    let players = gameplaymanifest.default.players;
    let players_size = gameplaymanifest.default.players_size;
    if (gameplaymanifest.songs[song_index].has_players) {
        players = gameplaymanifest.songs[song_index].players;
        players_size = gameplaymanifest.songs[song_index].players_size;
    }

    for (let i = 0; i < players_size; i++) {
        // obtain the position in the UI layout
        let layout_strums_id = players[i].layout_strums_id < 0 ? i : players[i].layout_strums_id;

        if (layout_strums_id >= initparams.layout_strums_size) layout_strums_id = -1;
        if (layout_strums_id < 0 || roundcontext.players[i].type == CHARACTERTYPE.ACTOR) continue;

        if (layout_strums_id == strums_id) return roundcontext.players[i].strums;
    }

    // unable to guess the correct player's strums
    return null;
}


function week_internal_concat_suffix(name, number_suffix) {
    // in C use a static char buffer to converter the integer into string
    let digits = number_suffix.toString();
    return string_concat(2, name, digits);
}

function week_internal_read_value_prefix(layout, name, number_suffix, type, default_value) {
    let temp = week_internal_concat_suffix(name, number_suffix);
    let value = layout_get_attached_value(layout, temp, type, default_value);
    temp = undefined;
    return value;
}

function week_internal_read_placeholder(layout, prefix_name, number_suffix) {
    let placeholder_name = week_internal_concat_suffix(prefix_name, number_suffix);
    let placeholder = layout_get_placeholder(layout, placeholder_name);
    if (!placeholder) console.error("Missing layout placeholder: " + placeholder_name);
    placeholder_name = undefined;
    return placeholder;
}

/*function week_internal_parse_character_placeholder(initparams, index, placeholder) {
    initparams.layout_characters[index] = {
        align_vertical: ALIGN_START,
        align_horizontal: ALIGN_START,
        reference_width: -1,
        reference_height: -1,
        x: 0,
        y: 0,
        z: 0
    };

    if (!placeholder) return;

    initparams.layout_characters[index].x = placeholder.x;
    initparams.layout_characters[index].y = placeholder.y;
    initparams.layout_characters[index].z = placeholder.z;
    initparams.layout_characters[index].align_vertical = placeholder.align_vertical;
    initparams.layout_characters[index].align_horizontal = placeholder.align_horizontal;
    initparams.layout_characters[index].reference_width = placeholder.width;
    initparams.layout_characters[index].reference_height = placeholder.height;
}*/

function week_internal_place_character(layout, character, layout_character) {

    /*const size = [0, 0];
    layout_get_viewport_size(layout, size);
    character_set_layout_resolution(character, 1280, 720);
    character_set_layout_resolution(character, initparams.ui_layout_width, initparams.ui_layout_height);
    character_set_layout_resolution(character, size[0], size[1]);*/

    character_set_scale(character, layout_character.scale);

    character_set_draw_location(character, layout_character.x, layout_character.y);
    character_set_z_index(character, layout_character.z);

    character_update_reference_size(
        character, layout_character.reference_width, layout_character.reference_height
    );

    let enable = layout_character.reference_width >= 0 || layout_character.reference_height >= 0;
    character_enable_reference_size(character, enable);

    character_set_draw_align(
        character, layout_character.align_vertical, layout_character.align_horizontal
    );

    layout_set_placeholder_drawable_by_id(
        layout, layout_character.placeholder_id, character_get_drawable(character)
    );

    character_state_toggle(character, null);
}

async function week_internal_load_gameplay_manifest(src) {
    let gameplaymanifest = await gameplaymanifest_init(src);

    if (!gameplaymanifest) {
        console.error("week_main() missing file " + src);
        return null;
    }

    if (gameplaymanifest.songs_size < 1) {
        gameplaymanifest_destroy(gameplaymanifest);
        console.error("week_main() no songs defined, goodbye. File: " + src);
        return null;
    }

    return gameplaymanifest;
}

function week_camera_focus_guess(/**@type {RoundContext} */ roundcontext, target_name, character_index) {
    const layout = roundcontext.layout ?? roundcontext.ui_layout;
    let camera = layout_get_camera_helper(layout);

    if (character_index >= 0) {
        let name = week_internal_concat_suffix(WEEKROUND_CAMERA_CHARACTER, character_index);
        let found = camera_from_layout(camera, layout, name);
        name = undefined;
        if (found) return;
    }

    camera_from_layout(camera, layout, target_name);
}

function week_internal_reset_players_and_girlfriend(/**@type {RoundContext} */ roundcontext) {
    const speed = roundcontext.settings.original_speed;
    week_change_scroll_direction(roundcontext);
    for (let i = 0; i < roundcontext.players_size; i++) {
        if (roundcontext.players[i].strums) {
            strums_reset(roundcontext.players[i].strums, speed, null);
            if (roundcontext.players[i].character)
                character_reset(roundcontext.players[i].character);
            if (roundcontext.players[i].conductor)
                conductor_poll_reset(roundcontext.players[i].conductor);
            if (roundcontext.players[i].ddrkeymon)
                ddrkeymon_stop(roundcontext.players[i].ddrkeymon);
            if (roundcontext.players[i].playerstats) {
                playerstats_reset(roundcontext.players[i].playerstats);
                playerstats_enable_health_recover(
                    roundcontext.players[i].playerstats, roundcontext.players[i].can_recover
                );
            }
        }
    }

    if (roundcontext.girlfriend) character_reset(roundcontext.girlfriend);

}

function week_internal_pick_counters_values_from_layout(/**@type {RoundContext}*/roundcontext) {
    const ui = roundcontext.initparams.ui;
    const layout = roundcontext.layout ?? roundcontext.ui_layout;

    // streakcounter values
    ui.streakcounter_comboheight = layout_get_attached_value_as_float(
        layout, "ui_streakcounter_comboHeight", ui.streakcounter_comboheight
    );
    ui.streakcounter_numbergap = layout_get_attached_value_as_float(
        layout, "ui_streakcounter_numberGap", ui.streakcounter_numbergap
    );
    ui.streakcounter_delay = layout_get_attached_value_as_float(
        layout, "ui_streakcounter_delay", ui.streakcounter_delay
    );

    // ranking counter values
    ui.rankingcounter_percentonly = layout_get_attached_value(
        layout, "ui_rankingcounter_accuracy_percent", LAYOUT_TYPE_BOOLEAN, ui.rankingcounter_percentonly
    );
}

function week_internal_read_placeholder_counter(layout, name, warn) {
    let placeholder = layout_get_placeholder(layout, name);
    if (warn && !placeholder) console.error(`week: missing layout '${name}' placeholder`);
    return placeholder;
}

function week_internal_check_screen_resolution(/**@type {RoundContext} */roundcontext, force) {
    const width = pvr_context.screen_width;
    const height = pvr_context.screen_height;
    const initparams = roundcontext.initparams;

    if (!force && roundcontext.resolution_changes == pvr_context.resolution_changes) return;
    roundcontext.resolution_changes = pvr_context.resolution_changes;

    sprite_set_draw_size(roundcontext.screen_background, width, height);

    let scale_x = width / initparams.ui_layout_width;
    let scale_y = height / initparams.ui_layout_height;

    let scale = Math.min(scale_x, scale_y);
    let translate_x = (width - initparams.ui_layout_width * scale) / 2;
    let translate_y = (height - initparams.ui_layout_height * scale) / 2;

    sh4matrix_clear(WEEKROUND_UI_MATRIX);
    sh4matrix_translate(WEEKROUND_UI_MATRIX, translate_x, translate_y);
    sh4matrix_scale(WEEKROUND_UI_MATRIX, scale, scale);

    sh4matrix_copy_to(WEEKROUND_UI_MATRIX, WEEKROUND_UI_MATRIX_CAMERA);
}

function week_internal_do_antibounce(/**@type {RoundContext} */roundcontext) {
    for (let i = 0; i < roundcontext.players_size; i++) {
        if (roundcontext.players[i].controller != null) {
            gamepad_clear_buttons(roundcontext.players[i].controller);// antibounce
        }
    }
}

