using System;
using System.Diagnostics;
using CsharpWrapper;
using Engine.Animation;
using Engine.Font;
using Engine.Game.Common;
using Engine.Game.Gameplay.Helpers;
using Engine.Image;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;
using KallistiOS;

namespace Engine.Game.Gameplay {

    public enum CharacterType {
        ACTOR,
        BOT,
        PLAYER
    };

    public class ModifiedDirective {
        public string name;
        public double value;
        internal bool completed_week;
        internal bool completed_round;
        internal bool create;
    }

    public class ScriptContext {
        public bool halt_flag;
        public LinkedList<ModifiedDirective> directives;
        public bool force_end_flag;
        public bool force_end_round_or_week;
        public bool force_end_loose_or_win;
    }

    public class PlayerStruct {
        public Conductor conductor;
        public Character character;
        public PlayerStats playerstats;
        public CharacterType type;
        public Strums strums;
        public DDRKeymon ddrkeymon;
        public DDRKeysFIFO ddrkeys_fifo;
        public Gamepad controller;
        public NotePool notepool;

        public bool is_vertical;
        public bool is_opponent;
        public bool can_die;
        public bool can_recover;
    }

    public class Settings {

        public bool girlfriend_cry;
        public bool ask_ready;
        public bool do_countdown;
        public bool camera_bumping;

        public float bpm;
        public double speed;

        public float original_bpm;
        public double original_speed;

        public string camera_name_opponent;
        public string camera_name_player;

        public bool layout_rollback;
        public bool show_credits;
        public bool no_healthbar;
    }

    public class UIParams {
        public float healthbar_x;
        public float healthbar_y;
        public float healthbar_z;
        public float healthbar_length;
        public float healthbar_dimmen;
        public float healthbar_border;
        public float healthbar_iconoverlap;
        public float healthbar_warnheight;
        public float healthbar_lockheight;
        public bool healthbar_is_vertical;
        public bool healthbar_nowarns;
        public bool roundstats_hide;
        public float roundstats_x;
        public float roundstats_y;
        public float roundstats_z;
        public float roundstats_size;
        public uint roundstats_fontcolor;
        public float streakcounter_comboheight;
        public float streakcounter_numbergap;
        public float streakcounter_delay;
        public bool rankingcounter_percentonly;
        public float trackinfo_x;
        public float trackinfo_y;
        public float trackinfo_z;
        public float trackinfo_maxwidth;
        public float trackinfo_maxheight;
        public Align trackinfo_alignvertical;
        public Align trackinfo_alignhorinzontal;
        public float trackinfo_fontsize;
        public uint trackinfo_fontcolor;
        public float countdown_height;
        public float songprogressbar_x;
        public float songprogressbar_y;
        public float songprogressbar_z;
        public float songprogressbar_width;
        public float songprogressbar_height;
        public Align songprogressbar_align;
        public float songprogressbar_bordersize;
        public float songprogressbar_fontsize;
        public float songprogressbar_fontbordersize;
        public bool songprogressbar_isvertical;
        public bool songprogressbar_showtime;
        public uint songprogressbar_colorrgba8_text;
        public uint songprogressbar_colorrgba8_background;
        public uint songprogressbar_colorrgba8_barback;
        public uint songprogressbar_colorrgba8_barfront;
    }

    public class InitParams {
        public LayoutStrum[] layout_strums;
        public int layout_strums_size;

        public LayoutCharacter layout_girlfriend;
        public LayoutCharacter[] layout_characters;
        public int layout_characters_size;

        public FontHolder font;
        public float ui_layout_width;
        public float ui_layout_height;
        public AnimList animlist;
        public GameplayManifest gameplaymanifest;

        public UIParams ui;

        public bool alt_tracks;
        public string difficult;
        public string default_boyfriend;
        public string default_girlfriend;

        public WeekInfo weekinfo;
    }

    public struct LayoutStrum {
        public float x;
        public float y;
        public float z;
        public float length;
        public float gap;
        public bool keep_marker_scale;
        public float marker_dimmen;
        public float invdimmen;
        public float sustain_alpha;
        public bool is_vertical;
    }

    public class LayoutCharacter {
        public float x;
        public float y;
        public float z;
        public float scale;
        public Align align_vertical;
        public Align align_horizontal;
        public float reference_width;
        public float reference_height;
        public int placeholder_id;
    }

    public class HealthBarParams {
        public string opponent_icon_model;
        public string player_icon_model;

        public bool has_player_color;
        public uint player_color_rgb8;

        public bool has_opponent_color;
        public uint opponent_color_rgb8;
    }

    public class RoundContext {
        public RankingCounter rankingcounter;
        public StreakCounter streakcounter;
        public HealthBar healthbar;
        public RoundStats roundstats;
        public TextSprite trackinfo;
        public Countdown countdown;
        public WeekGameOver weekgameover;
        public WeekPause weekpause;
        public WeekResult weekresult;
        public MessageBox messagebox;
        public Camera ui_camera;
        public MissNoteFX missnotefx;
        public SongProgressbar songprogressbar;

        public Layout layout;

        public SongPlayer songplayer;
        public WeekScript script;
        public int playerstats_index;
        public HealthWatcher healthwatcher;
        public Settings settings;

        public Character girlfriend;
        public PlayerStruct[] players;
        public int players_size;

        public int track_index;
        public string track_difficult;
        public double round_duration;

        public ChartEventEntry[] events;
        public int events_size;
        public int events_peek_index;

        public HealthBarParams healthbarparams;

        public ScriptContext scriptcontext;
        public Dialogue dialogue;

        public bool girlfriend_from_default;
        public bool healthbar_from_default;
        public bool stage_from_default;
        public bool script_from_default;
        public bool dialogue_from_default;
        public int weekgameover_from_version;
        public bool ui_from_default;
        public bool pause_menu_from_default;

        public bool players_from_default;
        public bool distributions_from_default;

        public Layout ui_layout;
        public Sprite screen_background;

        public bool has_directive_changes;
        public int resolution_changes;

        public InitParams initparams;
    }


    public class Week {


        private const GamepadButtons ROUND_READY_BUTTONS = GamepadButtons.A | GamepadButtons.START;
        private const string ROUND_CAMERA_GIRLFRIEND = "camera_girlfriend";
        internal const string ROUND_CAMERA_PLAYER = "camera_player";// camera_player0, camera_player1, etc
        internal const string ROUND_CAMERA_OPONNENT = "camera_opponent";
        private const string ROUND_CAMERA_ROUNDSTART = "camera_roundStart";
        private const string ROUND_CAMERA_ROUNDEND = "camera_roundEnd";
        private static readonly SH4Matrix ROUND_UI_MATRIX = new SH4Matrix();
        private static readonly SH4Matrix ROUND_UI_MATRIX_CAMERA = new SH4Matrix();
        private const string ROUND_UI_GROUP_NAME = "______UI______";// for internally use only
        private const string ROUND_UI_GROUP_NAME2 = "______UI2______";// for internally use only

        private const string UI_RANKINGCOUNTER_MODEL = "/assets/common/image/week-round/ranking.xml";
        private const string UI_STREAKCOUNTER_MODEL = "/assets/common/font/numbers.xml";
        private const string UI_COUNTDOWN_MODEL = "/assets/common/image/week-round/countdown.xml";
        private const string UI_ANIMLIST = "/assets/common/anims/week-round.xml";

        private const string UI_LAYOUT_WIDESCREEN = "/assets/common/image/week-round/ui.xml";
        private const string UI_LAYOUT_DREAMCAST = "/assets/common/image/week-round/ui~dreamcast.xml";
        private static readonly LayoutPlaceholder UI_STRUMS_LAYOUT_PLACEHOLDER = new LayoutPlaceholder() { x = 0f, y = 0f, z = 100f, width = 300f, height = 54f };
        private static readonly LayoutPlaceholder UI_STUB_LAYOUT_PLACEHOLDER = new LayoutPlaceholder() { x = 0f, y = 0f, z = -1f, width = 100f, height = 100f };
        private const string UI_TRACKINFO_ALT_SUFFIX = "(alt) ";
#if DEBUG
        private const string UI_TRACKINFO_FORMAT = "$s $s[$s] {kdy $s}";
#else
        private const string UI_TRACKINFO_FORMAT = "$s $s[$s]";
#endif

        internal const string ROUND_CHARACTER_PREFIX = "character_";

        public static readonly BeatWatcher BEAT_WATCHER = new BeatWatcher() { count = 0, since = 0f };
        public static readonly BeatWatcher QUARTER_WATCHER = new BeatWatcher() { count = 0, since = 0f };




        private static void Destroy(RoundContext roundcontext, GameplayManifest gameplaymanifest) {
            InitParams initparams = roundcontext.initparams;

            Texture.DisableDefering(false);

            gameplaymanifest.Destroy();
            roundcontext.healthwatcher.Destroy();

            if (roundcontext.rankingcounter != null) roundcontext.rankingcounter.Destroy();
            if (roundcontext.streakcounter != null) roundcontext.streakcounter.Destroy();
            if (roundcontext.healthbar != null) roundcontext.healthbar.Destroy();
            if (roundcontext.layout != null) roundcontext.layout.Destroy();
            if (roundcontext.roundstats != null) roundcontext.roundstats.Destroy();
            if (roundcontext.songplayer != null) roundcontext.songplayer.Destroy();
            if (roundcontext.script != null) roundcontext.script.Destroy();
            if (roundcontext.countdown != null) roundcontext.countdown.Destroy();
            if (roundcontext.girlfriend != null) roundcontext.girlfriend.Destroy();
            if (roundcontext.trackinfo != null) roundcontext.trackinfo.Destroy();
            if (roundcontext.weekgameover != null) roundcontext.weekgameover.Destroy();
            if (roundcontext.weekpause != null) roundcontext.weekpause.Destroy();
            if (roundcontext.screen_background != null) roundcontext.screen_background.Destroy();
            if (roundcontext.weekresult != null) roundcontext.weekresult.Destroy();
            if (roundcontext.messagebox != null) roundcontext.messagebox.Destroy();
            if (roundcontext.ui_camera != null) roundcontext.ui_camera.Destroy();
            if (roundcontext.missnotefx != null) roundcontext.missnotefx.Destroy();
            if (roundcontext.dialogue != null) roundcontext.dialogue.Destroy();
            if (roundcontext.songprogressbar != null) roundcontext.songprogressbar.Destroy();

            //free(roundcontext.events);
            //free(roundcontext.healthbarparams.player_icon_model);
            //free(roundcontext.healthbarparams.opponent_icon_model);

            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                if (roundcontext.players[i].character != null) roundcontext.players[i].character.Destroy();
                if (roundcontext.players[i].conductor != null) roundcontext.players[i].conductor.Destroy();
                if (roundcontext.players[i].notepool != null) roundcontext.players[i].notepool.Destroy();
                if (roundcontext.players[i].strums != null) roundcontext.players[i].strums.Destroy();
                if (roundcontext.players[i].ddrkeymon != null) roundcontext.players[i].ddrkeymon.Destroy();
                if (roundcontext.players[i].controller != null) roundcontext.players[i].controller.Destroy();
            }
            //free(roundcontext.players);

            //foreach (ModifiedDirective directive_info in roundcontext.scriptcontext.directives) {
            //    free(directive_info.name);
            //    free(directive_info);
            //}
            roundcontext.scriptcontext.directives.Destroy();

            if (roundcontext.ui_layout != null) roundcontext.ui_layout.Destroy();

            //free(initparams.layout_strums);
            //free(initparams.layout_characters);
            //free(initparams.layout_girlfriend);

            //if (roundcontext.settings.camera_name_opponent != Week.ROUND_CAMERA_OPONNENT)
            //    free(roundcontext.settings.camera_name_opponent);
            //if (roundcontext.settings.camera_name_player != Week.ROUND_CAMERA_PLAYER)
            //    free(roundcontext.settings.camera_name_player);


            if (roundcontext.initparams.weekinfo.custom_folder_gameplay != null)
                FS.OverrideCommonFolder(null);
        }


        public static int Main(WeekInfo weekinfo, bool alt_tracks, string difficult, string default_bf, string default_gf, string gameplaymanifest_src, int single_track_index) {
            Week.ROUND_UI_MATRIX.Clear();
            Week.ROUND_UI_MATRIX_CAMERA.Clear();

            InitParams initparams = new InitParams() {
                alt_tracks = alt_tracks,
                difficult = difficult,
                default_boyfriend = default_bf,
                default_girlfriend = default_gf,

                layout_strums = null,
                layout_strums_size = 0,

                layout_girlfriend = null,
                layout_characters = null,
                layout_characters_size = 0,

                weekinfo = weekinfo,
                gameplaymanifest = null,

                animlist = null,
                font = null,
                ui_layout_height = 0f,
                ui_layout_width = 0f,
                ui = new UIParams() {
                    healthbar_x = 0f,
                    healthbar_y = 0f,
                    healthbar_z = 0f,
                    healthbar_length = 0f,
                    healthbar_dimmen = 0f,
                    healthbar_border = 0f,
                    healthbar_iconoverlap = 0f,
                    healthbar_warnheight = 0f,
                    healthbar_lockheight = 0f,
                    healthbar_is_vertical = false,
                    healthbar_nowarns = false,
                    roundstats_x = 0f,
                    roundstats_y = 0f,
                    roundstats_z = 0f,
                    roundstats_size = 0f,
                    roundstats_fontcolor = 0x00,
                    roundstats_hide = false,
                    streakcounter_comboheight = 0f,
                    streakcounter_numbergap = 0f,
                    streakcounter_delay = 0f,
                    rankingcounter_percentonly = false,
                    trackinfo_x = 0f,
                    trackinfo_y = 0f,
                    trackinfo_z = 0f,
                    trackinfo_maxwidth = -1f,
                    trackinfo_maxheight = -1f,
                    trackinfo_alignvertical = Align.START,
                    trackinfo_alignhorinzontal = Align.START,
                    trackinfo_fontcolor = 0x00,
                    trackinfo_fontsize = 0x00,
                    countdown_height = 0f,
                    songprogressbar_x = 0f,
                    songprogressbar_y = 0f,
                    songprogressbar_z = 0f,
                    songprogressbar_width = 0f,
                    songprogressbar_height = 0f,
                    songprogressbar_align = Align.NONE,
                    songprogressbar_bordersize = 0f,
                    songprogressbar_fontsize = 0f,
                    songprogressbar_fontbordersize = 0f,
                    songprogressbar_isvertical = false,
                    songprogressbar_showtime = false,
                    songprogressbar_colorrgba8_text = 0x00,
                    songprogressbar_colorrgba8_background = 0x00,
                    songprogressbar_colorrgba8_barback = 0x00,
                    songprogressbar_colorrgba8_barfront = 0x00
                }
            };

            RoundContext roundcontext = new RoundContext() {
                rankingcounter = null,
                streakcounter = null,
                healthbar = null,
                layout = null,
                roundstats = null,
                songplayer = null,
                script = null,
                playerstats_index = -1,
                healthwatcher = new HealthWatcher(),
                countdown = null,
                trackinfo = null,
                weekgameover = null,
                weekpause = null,
                weekresult = null,
                messagebox = null,
                ui_camera = new Camera(null, 640f, 480f),
                missnotefx = null,
                songprogressbar = null,
                screen_background = Sprite.InitFromRGB8(0x0),

                has_directive_changes = false,

                settings = new Settings() {
                    ask_ready = true,
                    do_countdown = true,
                    girlfriend_cry = true,
                    original_bpm = 100f,
                    original_speed = 1.0,
                    camera_bumping = true,
                    show_credits = false,
                    no_healthbar = false,

                    bpm = 100f,
                    speed = 1.0,

                    camera_name_opponent = Week.ROUND_CAMERA_PLAYER,
                    camera_name_player = Week.ROUND_CAMERA_OPONNENT,

                    layout_rollback = true
                },
                girlfriend = null,

                track_index = 0,
                round_duration = -1,
                track_difficult = difficult,

                events = null,
                events_size = 0,
                events_peek_index = 0,

                players = null,
                players_size = 0,

                girlfriend_from_default = true,
                healthbar_from_default = true,
                stage_from_default = true,
                script_from_default = true,
                ui_from_default = true,
                pause_menu_from_default = true,
                weekgameover_from_version = 0,

                healthbarparams = new HealthBarParams() {
                    has_opponent_color = false,
                    has_player_color = false,
                    opponent_color_rgb8 = 0x00,
                    player_color_rgb8 = 0x00,
                    opponent_icon_model = null,
                    player_icon_model = null
                },

                scriptcontext = new ScriptContext() {
                    halt_flag = false,
                    directives = new LinkedList<ModifiedDirective>(),
                    force_end_flag = false,
                    force_end_round_or_week = false,
                    force_end_loose_or_win = false,
                },

                players_from_default = false,
                distributions_from_default = false,
                ui_layout = null,
                resolution_changes = 0,

                initparams = initparams
            };

            // (JS & C# only) disable texture deferring, avoid shuttering when drawing the first frame
            Texture.DisableDefering(true);

            // the UI "bump" has one octave of beat as duration (example: 75ms @ 100bpm)
            roundcontext.ui_camera.SetTransitionDuration(true, 0.125f);

            // screen background
            roundcontext.screen_background.SetZIndex(Single.NegativeInfinity);

            // pause menu
            roundcontext.weekpause = new WeekPause();
            // track/week stats
            roundcontext.weekresult = new WeekResult();
            // messagebox
            roundcontext.messagebox = new MessageBox();

            // setup custom folder (if exists) and the week folder as current directory
            string week_folder = WeekEnumerator.GetWeekFolder(weekinfo);
            FS.SetWorkingFolder(week_folder, false);
            //free(week_folder);
            GameMain.custom_style_from_week = weekinfo;

            if (!String.IsNullOrEmpty(weekinfo.custom_folder_gameplay)) {
                FS.OverrideCommonFolder(weekinfo.custom_folder_gameplay);
            } else if (!String.IsNullOrEmpty(weekinfo.custom_folder)) {
                FS.OverrideCommonFolder(weekinfo.custom_folder);
            }

            // step 1: load the gameplay manifest this hosts all engine components behavior
            GameplayManifest gameplaymanifest;
            if (gameplaymanifest_src != null) {
                gameplaymanifest = Week.InternalLoadGameplayManifest(gameplaymanifest_src);
            } else {
                gameplaymanifest_src = WeekEnumerator.GetGameplayManifest(weekinfo);
                gameplaymanifest = Week.InternalLoadGameplayManifest(gameplaymanifest_src);
                //free(gameplaymanifest_src);
            }
            if (gameplaymanifest == null) {
                Texture.DisableDefering(false);
                return 0;
            }

            // step 2: initialize the first track (round n° 1)
            roundcontext.track_index = 0;// this is very important
            initparams.gameplaymanifest = gameplaymanifest;

            bool gameover = false;
            bool retry = false;
            bool mainmenu = false;
            bool weekselector = false;
            int[] tracks_attempts = new int[gameplaymanifest.tracks_size];
            bool first_init = true;
            bool reject_completed = false;
            int last_track = gameplaymanifest.tracks_size - 1;
            bool single_track = single_track_index >= 0;

            for (int i = 0 ; i < gameplaymanifest.tracks_size ; i++) tracks_attempts[i] = 0;
            if (single_track) roundcontext.track_index = single_track_index;

            // step 3: start the round cycle
            while (roundcontext.track_index < gameplaymanifest.tracks_size) {
                BeatWatcher.GlobalSetTimestamp(0.0);

                if (!retry) {
                    Week.RoundPrepare(roundcontext, gameplaymanifest);
                }

                // before continue set default values
                Week.BEAT_WATCHER.Reset(true, roundcontext.settings.original_bpm);
                Week.QUARTER_WATCHER.Reset(false, roundcontext.settings.original_bpm);
                Week.ChangeCharacterCameraName(roundcontext, true, Week.ROUND_CAMERA_OPONNENT);
                Week.ChangeCharacterCameraName(roundcontext, false, Week.ROUND_CAMERA_PLAYER);
                roundcontext.ui_camera.ToOrigin(false);
                Week.ROUND_UI_MATRIX.CopyTo(Week.ROUND_UI_MATRIX_CAMERA);

                Week.ToggleStates(roundcontext, gameplaymanifest);
                roundcontext.messagebox.SetImageSprite(null);
                for (int i = 0 ; i < roundcontext.players_size ; i++) {
                    roundcontext.players[i].character.UseAlternateSingAnimations(false);
                    roundcontext.players[i].character.FreezeAnimation(false);
                    roundcontext.players[i].character.SetVisible(true);
                }
                roundcontext.scriptcontext.halt_flag = false;
                roundcontext.layout.SetSingleItemToDraw(null);
                if (roundcontext.songplayer != null) roundcontext.songplayer.Mute(false);

                if (first_init) {
                    if (roundcontext.script != null) {
                        roundcontext.script.NotifyWeekinit(single_track ? single_track_index : -1);
                        Week.Halt(roundcontext, true);
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
                if (roundcontext.healthbar != null) {
                    roundcontext.healthbar.SetHealthPosition2(0.5f);
                    roundcontext.healthbar.HideWarnings();
                }

                // update songprogressbar
                if (roundcontext.songprogressbar != null) {
                    roundcontext.songprogressbar.SetSongplayer(roundcontext.songplayer);
                    roundcontext.songprogressbar.SetDuration(roundcontext.round_duration);
                }

                // check if necessary show dialogue if an dialog text is provided
                bool show_dialog = false;
                if (!retry && !String.IsNullOrEmpty(gameplaymanifest.tracks[roundcontext.track_index].dialog_text)) {
                    string dialog_text = gameplaymanifest.tracks[roundcontext.track_index].dialog_text;
                    if (roundcontext.dialogue == null) {
                        Console.Error.WriteLine($"[ERROR] week_round() can not load '{dialog_text}' there no dialogue instance");
                    } else if (roundcontext.dialogue.ShowDialog(dialog_text)) {
                        show_dialog = true;
                    } else {
                        Console.Error.WriteLine($"[ERROR] week_round() failed to read '{dialog_text}' file");
                    }
                }

                // actual gameplay is here
                int current_track_index = roundcontext.track_index;
                int round_result = Round(roundcontext, retry, show_dialog);

                retry = false;
                Week.CheckDirectivesRound(roundcontext, round_result == 0);

                if (round_result == 0) {
                    roundcontext.weekresult.AddStats(roundcontext);
                }

                if (roundcontext.scriptcontext.force_end_flag) {
                    if (!roundcontext.scriptcontext.force_end_round_or_week) {
                        gameover = true;
                        break;
                    }
                    roundcontext.scriptcontext.force_end_flag = false;
                }

                if ((round_result == 0 && roundcontext.track_index != last_track) || round_result == 2) {
                    if (roundcontext.settings.layout_rollback) {
                        roundcontext.layout.StopAllTriggers();
                        roundcontext.layout.TriggerAny(null);
                    }
                    Week.UISetVisibility(roundcontext, true);
                    Week.InternalResetPlayersAndGirlfriend(roundcontext);
                    if (roundcontext.healthwatcher != null) roundcontext.healthwatcher.ResetOpponents();
                    if (roundcontext.roundstats != null) roundcontext.roundstats.Reset();
                    if (roundcontext.streakcounter != null) roundcontext.streakcounter.Reset();
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
                    tracks_attempts[roundcontext.track_index]++;
                    if (roundcontext.songplayer != null) {
                        roundcontext.songplayer.Seek(0.0);
                        roundcontext.songplayer.Mute(false);
                    }

                    Week.ToggleStates(roundcontext, gameplaymanifest);

                    if (current_track_index != roundcontext.track_index) continue;
                    if (roundcontext.track_difficult != initparams.difficult) {
                        reject_completed = current_track_index > 0;
                        initparams.difficult = roundcontext.track_difficult;
                        continue;
                    }

                    retry = true;
                    roundcontext.events_peek_index = 0;
                    continue;
                }

                if (single_track) break;// week launched from freeplaymenu

                // round completed, next one
                roundcontext.track_index++;
                retry = false;
            }

            if (mainmenu || weekselector) {
                // notify script
                if (roundcontext.script != null) roundcontext.script.NotifyWeekleave();

                // flush unlock directives that do not need completed week
                Week.CheckDirectivesWeek(roundcontext, !gameover);

                if (roundcontext.has_directive_changes) {
                    int save_error = SaveManager.ShouldShow(true);
                    if (save_error != 0) {
                        SaveManager savemanager = new SaveManager(true, save_error);

                        savemanager.Show();
                        savemanager.Destroy();
                    }
                }

                // dispose all allocated resources
                Week.Destroy(roundcontext, gameplaymanifest);

                // if true, goto weekselector
                return mainmenu ? 0 : 1;
            }


            if (roundcontext.script != null) roundcontext.script.NotifyWeekend(gameover);

            Week.Halt(roundcontext, true);

            // TODO: check unlockeables
            Week.CheckDirectivesWeek(roundcontext, !gameover);
            if (!gameover && !reject_completed && !String.IsNullOrEmpty(weekinfo.emit_directive)) {
                // if the week was completed successfully emit the directive
                FunkinSave.CreateUnlockDirective(weekinfo.emit_directive, 0x00);
            }

            // show the whole week stats and wait for the player to press START to return
            if (!gameover) {
                int total_attempts = 0;
                for (int i = 0 ; i < gameplaymanifest.tracks_size ; i++) total_attempts += tracks_attempts[i];

                int tracks_count = single_track ? 1 : gameplaymanifest.tracks_size;
                roundcontext.weekresult.HelperShowSummary(
                    roundcontext, total_attempts, tracks_count, reject_completed
                );

                if (roundcontext.script != null) roundcontext.script.NotifyAfterresults();

                Week.Halt(roundcontext, true);
            }

            // save progress
            if (roundcontext.has_directive_changes || !gameover && !reject_completed) {
                long total_score = 0;
                for (int i = 0 ; i < roundcontext.players_size ; i++) {
                    if (roundcontext.players[i].type != CharacterType.PLAYER) continue;
                    if (roundcontext.players[i].is_opponent) continue;
                    total_score += roundcontext.players[i].playerstats.GetScore();
                }

                FunkinSave.SetWeekScore(weekinfo.name, roundcontext.track_difficult, total_score);

                // keep displaying the stage layout until the save is done
                roundcontext.messagebox.UseSmallSize(true);
                roundcontext.messagebox.UseFullTitle(true);
                roundcontext.messagebox.SetTitle("Saving progress...");
                roundcontext.messagebox.HideButtons();
                roundcontext.messagebox.SetMessage(null);
                roundcontext.messagebox.Show(true);

                // do save
                int save_error = (int)GameMain.SpawnCoroutine(roundcontext.layout, delegate (object arg) {
                    return (object)SaveManager.ShouldShow((bool)arg);
                }, true);

                if (save_error != 0) {
                    roundcontext.layout.Suspend();
                    SaveManager savemanager = new SaveManager(true, save_error);

                    savemanager.Show();
                    savemanager.Destroy();
                }
            }

            bool show_credits = !gameover && roundcontext.settings.show_credits;

            // dispose all allocated resources
            Week.Destroy(roundcontext, gameplaymanifest);

            if (show_credits) {
                // game ending
                Credits.Main();
            }

            return 1;
        }

        private static void InitUILayout(string src_layout, InitParams initparams, RoundContext roundcontext) {
            float layout_width, layout_height;
            UIParams ui = initparams.ui;
            string src;
            LayoutPlaceholder placeholder;

            if (!String.IsNullOrEmpty(src_layout)) src = src_layout;
            else src = PVRContext.global_context.IsWidescreen() ? UI_LAYOUT_WIDESCREEN : UI_LAYOUT_DREAMCAST;

            Layout layout = Layout.Init(src);
            if (roundcontext.ui_layout != null) roundcontext.ui_layout.Destroy();

            if (layout == null) throw new Exception("Can not load the ui layout: " + src);

            //free(initparams.layout_strums);
            roundcontext.ui_layout = layout;

            layout.GetViewportSize(out layout_width, out layout_height);
            initparams.ui_layout_width = layout_width;
            initparams.ui_layout_height = layout_height;
            ui.countdown_height = initparams.ui_layout_height / 3f;

            initparams.font = layout.GetAttachedFont("ui_font");

            string src_animlist = (string)layout.GetAttachedValue("ui_animlist", AttachedValueType.STRING, UI_ANIMLIST);
            AnimList old_animlist = initparams.animlist;
            initparams.animlist = !String.IsNullOrEmpty(src_animlist) ? AnimList.Init(src_animlist) : null;
            if (old_animlist != null) old_animlist.Destroy();

            initparams.layout_strums_size = (int)((long)layout.GetAttachedValue("ui_strums_count", AttachedValueType.INTEGER, 0));
            initparams.layout_strums = initparams.layout_strums_size > 0 ? new LayoutStrum[initparams.layout_strums_size] : null;
            for (int i = 0 ; i < initparams.layout_strums_size ; i++) {
                placeholder = Week.InternalReadPlaceholder(layout, "ui_strums", i);
                if (placeholder == null) placeholder = UI_STRUMS_LAYOUT_PLACEHOLDER;

                initparams.layout_strums[i] = new LayoutStrum() {
                    marker_dimmen = (float)((double)Week.InternalReadValuePrefix(layout, "ui_strums_markerDimmen", i, AttachedValueType.FLOAT, 50.0)),
                    invdimmen = (float)((double)Week.InternalReadValuePrefix(layout, "ui_strums_markerInvdimmen", i, AttachedValueType.FLOAT, 50.0)),
                    sustain_alpha = (float)((double)Week.InternalReadValuePrefix(layout, "ui_strums_sustainAlpha", i, AttachedValueType.FLOAT, 1.0)),
                    gap = (float)((double)Week.InternalReadValuePrefix(layout, "ui_strums_gap", i, AttachedValueType.FLOAT, 0.0)),
                    keep_marker_scale = (bool)Week.InternalReadValuePrefix(layout, "ui_strums_keepMarkerScale", i, AttachedValueType.BOOLEAN, true),
                    is_vertical = (bool)Week.InternalReadValuePrefix(layout, "ui_strums_verticalScroll", i, AttachedValueType.BOOLEAN, true),
                    length = 0f,
                    x = placeholder.x,
                    y = placeholder.y,
                    z = placeholder.z
                };
                initparams.layout_strums[i].length = initparams.layout_strums[i].is_vertical ? placeholder.height : placeholder.width;
            }

            placeholder = layout.GetPlaceholder("ui_healthbar");
            if (placeholder == null) {
                Console.Error.WriteLine("[ERROR] missing layout ui_healthbar placeholder");
                placeholder = UI_STUB_LAYOUT_PLACEHOLDER;
            }
            ui.healthbar_is_vertical = (bool)layout.GetAttachedValue("ui_healthbar_isVertical", AttachedValueType.BOOLEAN, false);
            ui.healthbar_nowarns = !(bool)layout.GetAttachedValue("ui_healthbar_showWarnings", AttachedValueType.BOOLEAN, true);
            ui.healthbar_border = (float)((double)layout.GetAttachedValue("ui_healthbar_borderSize", AttachedValueType.FLOAT, 0.0));
            ui.healthbar_iconoverlap = (float)((double)layout.GetAttachedValue("ui_healthbar_iconOverlap", AttachedValueType.FLOAT, 0.0));
            ui.healthbar_warnheight = (float)((double)layout.GetAttachedValue("ui_healthbar_warningHeight", AttachedValueType.FLOAT, 0.0));
            ui.healthbar_lockheight = (float)((double)layout.GetAttachedValue("ui_healthbar_lockedHeight", AttachedValueType.FLOAT, 0.0));
            ui.healthbar_x = placeholder.x;
            ui.healthbar_y = placeholder.y;
            ui.healthbar_z = placeholder.z;
            ui.healthbar_length = placeholder.width;
            ui.healthbar_dimmen = placeholder.height;
            if (ui.healthbar_is_vertical) {
                float temp = ui.healthbar_length;
                ui.healthbar_length = ui.healthbar_dimmen;
                ui.healthbar_dimmen = temp;
            }

            placeholder = layout.GetPlaceholder("ui_roundstats");
            if (placeholder == null) {
                Console.Error.WriteLine("[ERROR] missing layout ui_roundstats placeholder");
                placeholder = UI_STUB_LAYOUT_PLACEHOLDER;
            }
            ui.roundstats_x = placeholder.x;
            ui.roundstats_y = placeholder.y;
            ui.roundstats_z = placeholder.z;
            ui.roundstats_hide = (bool)layout.GetAttachedValue("ui_roundstats_hidden", AttachedValueType.BOOLEAN, false);
            ui.roundstats_size = (float)((double)layout.GetAttachedValue("ui_roundstats_fontSize", AttachedValueType.FLOAT, 12.0));
            ui.roundstats_fontcolor = (uint)layout.GetAttachedValue("ui_roundstats_fontColor", AttachedValueType.HEX, 0xFFFFFFU);

            placeholder = layout.GetPlaceholder("ui_songprogressbar");
            if (placeholder == null) {
                Console.Error.WriteLine("[ERROR] missing layout ui_songprogressbar placeholder");
                placeholder = UI_STUB_LAYOUT_PLACEHOLDER;
            }
            ui.songprogressbar_bordersize = (float)(double)layout.GetAttachedValue("ui_songprogressbar_borderSize", AttachedValueType.FLOAT, 2.0);
            ui.songprogressbar_fontsize = (float)(double)layout.GetAttachedValue("ui_songprogressbar_fontSize", AttachedValueType.FLOAT, 11.0);
            ui.songprogressbar_fontbordersize = (float)(double)layout.GetAttachedValue("ui_songprogressbar_fontBorderSize", AttachedValueType.FLOAT, 1.4);
            ui.songprogressbar_isvertical = (bool)layout.GetAttachedValue("ui_songprogressbar_isVertical", AttachedValueType.BOOLEAN, false);
            ui.songprogressbar_showtime = (bool)layout.GetAttachedValue("ui_songprogressbar_showTime", AttachedValueType.BOOLEAN, true);
            ui.songprogressbar_colorrgba8_text = (uint)layout.GetAttachedValue("ui_songprogressbar_colorRGBA8_text", AttachedValueType.HEX, 0xFFFFFFFFU);
            ui.songprogressbar_colorrgba8_background = (uint)layout.GetAttachedValue("ui_songprogressbar_colorRGBA8_background", AttachedValueType.HEX, 0x000000FFU);
            ui.songprogressbar_colorrgba8_barback = (uint)layout.GetAttachedValue("ui_songprogressbar_colorRGBA8_barBack", AttachedValueType.HEX, 0x808080FFU);
            ui.songprogressbar_colorrgba8_barfront = (uint)layout.GetAttachedValue("ui_songprogressbar_colorRGBA8_barFront", AttachedValueType.HEX, 0xFFFFFFFFU);
            ui.songprogressbar_x = placeholder.x;
            ui.songprogressbar_y = placeholder.y;
            ui.songprogressbar_z = placeholder.z;
            ui.songprogressbar_width = placeholder.width;
            ui.songprogressbar_height = placeholder.height;
            ui.songprogressbar_align = ui.songprogressbar_isvertical ? placeholder.align_vertical : placeholder.align_horizontal;

            // pick streakcounter and rankingcounter values
            Week.InternalPickCountersValuesFromLayout(roundcontext);

            placeholder = layout.GetPlaceholder("ui_track_info");
            if (placeholder == null) {
                Console.Error.WriteLine("[ERROR] missing layout ui_track_info placeholder");
                placeholder = UI_STUB_LAYOUT_PLACEHOLDER;
            }
            ui.trackinfo_x = placeholder.x;
            ui.trackinfo_y = placeholder.y;
            ui.trackinfo_z = placeholder.z;
            ui.trackinfo_maxwidth = placeholder.width;
            ui.trackinfo_maxheight = placeholder.height;
            ui.trackinfo_alignvertical = placeholder.align_vertical;
            ui.trackinfo_alignhorinzontal = placeholder.align_horizontal;
            ui.trackinfo_fontsize = (float)((double)layout.GetAttachedValue("ui_track_info_fontSize", AttachedValueType.FLOAT, 10.0));
            ui.trackinfo_fontcolor = (uint)layout.GetAttachedValue("ui_track_info_fontColor", AttachedValueType.HEX, 0xFFFFFFU);

            // initialize adaptation of the UI elements in the stage layout
            InternalCheckScreenResolution(roundcontext, true);

            // pick default counters values
            Week.InternalPickCountersValuesFromLayout(roundcontext);
        }

        public static void PickInvertedUILayoutValues(RoundContext roundcontext) {
            Layout layout = roundcontext.ui_layout;
            InitParams initparams = roundcontext.initparams;
            UIParams ui = roundcontext.initparams.ui;
            LayoutPlaceholder placeholder;

            for (int i = 0 ; i < initparams.layout_strums_size ; i++) {
                placeholder = Week.InternalReadPlaceholder(layout, "ui_strums_inverted", i);
                if (placeholder == null) continue;

                initparams.layout_strums[i].x = placeholder.x;
                initparams.layout_strums[i].y = placeholder.y;
                initparams.layout_strums[i].z = placeholder.z;
                initparams.layout_strums[i].length = initparams.layout_strums[i].is_vertical ? placeholder.height : placeholder.width;
            }

            placeholder = layout.GetPlaceholder("ui_healthbar_inverted");
            if (placeholder != null) {
                ui.healthbar_x = placeholder.x;
                ui.healthbar_y = placeholder.y;
                ui.healthbar_z = placeholder.z;
                ui.healthbar_length = placeholder.width;
                ui.healthbar_dimmen = placeholder.height;
                if (ui.healthbar_is_vertical) {
                    float temp = ui.healthbar_length;
                    ui.healthbar_length = ui.healthbar_dimmen;
                    ui.healthbar_dimmen = temp;
                }
            }

            placeholder = layout.GetPlaceholder("ui_roundstats_inverted");
            if (placeholder != null) {
                ui.roundstats_x = placeholder.x;
                ui.roundstats_y = placeholder.y;
                ui.roundstats_z = placeholder.z;
            }

            placeholder = layout.GetPlaceholder("ui_songprogressbar_inverted");
            if (placeholder != null) {
                ui.songprogressbar_x = placeholder.x;
                ui.songprogressbar_y = placeholder.y;
                ui.songprogressbar_z = placeholder.z;
            }

            placeholder = layout.GetPlaceholder("ui_track_info_inverted");
            if (placeholder != null) {
                ui.trackinfo_x = placeholder.x;
                ui.trackinfo_y = placeholder.y;
                ui.trackinfo_z = placeholder.z;
            }
        }


        public static void RoundPrepare(RoundContext roundcontext, GameplayManifest gameplaymanifest) {
            //
            // Note:
            //      Some UI components can be redefined in each week round (track). If the
            //      track does not specify this, the UI component/s are initialized to thier
            //      default settings.
            //
            //      Each track can not use the defined UI component/s of previous tracks, if
            //      those tracks does not have thier own definitions it will reinitialized to
            //      default.
            //
            GameplayManifestTrack trackmanifest = gameplaymanifest.tracks[roundcontext.track_index];
            InitParams initparams = roundcontext.initparams;

            bool updated_ui = false;
            bool updated_distributions_or_players = false;
            bool updated_stage = false;

            // initialize layout
            if (trackmanifest.has_stage) {
                updated_stage = true;
                roundcontext.stage_from_default = false;

                Week.InitStage(roundcontext, trackmanifest.stage);
            } else if (roundcontext.layout == null || !roundcontext.stage_from_default) {
                updated_stage = true;
                roundcontext.stage_from_default = true;

                Week.InitStage(roundcontext, gameplaymanifest.@default.stage);
            }

            // initialize script/stagescript
            if (trackmanifest.has_script) {
                roundcontext.script_from_default = false;

                Week.InitScript(roundcontext, trackmanifest.script);
            } else if (roundcontext.script == null || !roundcontext.script_from_default) {
                roundcontext.script_from_default = true;

                Week.InitScript(roundcontext, gameplaymanifest.@default.script);
            }

            // initialize dialogue
            if (!String.IsNullOrEmpty(trackmanifest.dialogue_params)) {
                roundcontext.dialogue_from_default = false;
                if (Week.InitDialogue(roundcontext, trackmanifest.dialogue_params)) updated_ui = true;
            } else if (roundcontext.dialogue == null || !roundcontext.script_from_default) {
                roundcontext.dialogue_from_default = true;
                if (Week.InitDialogue(roundcontext, gameplaymanifest.@default.dialogue_params)) updated_ui = true;
            }

            // reload the music only  
            roundcontext.weekpause.Prepare();

            if (roundcontext.missnotefx == null) {
                roundcontext.missnotefx = new MissNoteFX();
            }

            // initialize ui
            if (trackmanifest.has_ui_layout) {
                string src = trackmanifest.ui_layout;
                if (String.IsNullOrEmpty(src)) src = gameplaymanifest.@default.ui_layout;


                Week.InitUILayout(src, initparams, roundcontext);
                roundcontext.ui_from_default = true;
                updated_ui = true;
            } else if (roundcontext.ui_layout == null || !roundcontext.ui_from_default) {

                Week.InitUILayout(gameplaymanifest.@default.ui_layout, initparams, roundcontext);
                roundcontext.ui_from_default = true;
                updated_ui = true;
            }

            if (updated_ui && EngineSettings.inverse_strum_scroll) {
                // pick inverted placeholder values
                Week.PickInvertedUILayoutValues(roundcontext);
            }

            // initialize strums, character and controller misc.
            if (Week.InitChartAndPlayers(roundcontext, gameplaymanifest, updated_ui)) {
                roundcontext.playerstats_index = -1;

                // pick playerstats from the first playable character
                for (int i = 0 ; i < roundcontext.players_size ; i++) {
                    if (roundcontext.players[i].type != CharacterType.PLAYER) continue;
                    roundcontext.playerstats_index = i;
                    break;
                }
                if (roundcontext.playerstats_index < 0 && roundcontext.players_size > 0) {
                    roundcontext.playerstats_index = 0;
                }

                updated_distributions_or_players = true;
            }

            // update/set strums params
            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                if (roundcontext.players[i].strums != null) {
                    roundcontext.players[i].strums.SetParams(
                        roundcontext.players[i].ddrkeymon,
                        roundcontext.players[i].playerstats,
                        roundcontext.script
                    );
                }
            }

            //float multiplier = roundcontext.track_difficult == Funkin.DIFFICULT_EASY ? 1.25 : 1.0;
            //for (int i = 0 ; i < roundcontext.players_size ; i++) {
            //    if (roundcontext.players[i].strums)
            //        roundcontext.players[i].strums.SetMarkerDurationMultiplier(multiplier);
            //}

            // rebuild the healthwatcher
            roundcontext.healthwatcher.Clear();
            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                roundcontext.players[i].playerstats.Reset();

                // untested
                //if (roundcontext.players[i].type == CharacterType.ACTOR) continue;

                if (roundcontext.players[i].is_opponent) {
                    roundcontext.healthwatcher.AddOpponent(
                        roundcontext.players[i].playerstats,
                        roundcontext.players[i].can_recover,
                        roundcontext.players[i].can_die
                    );
                } else {
                    roundcontext.healthwatcher.AddPlayer(
                        roundcontext.players[i].playerstats,
                        roundcontext.players[i].can_recover,
                        roundcontext.players[i].can_die
                    );
                }
            }

            // initialize healthbar
            if (updated_distributions_or_players && roundcontext.healthbar != null) {
                updated_stage = Week.InitHealthbar(roundcontext, gameplaymanifest, updated_distributions_or_players);
            } else if (roundcontext.healthbar == null) {
                updated_stage = Week.InitHealthbar(roundcontext, gameplaymanifest, true);
            }

            // initialize girlfriend
            if (trackmanifest.has_girlfriend) {
                updated_stage = true;
                roundcontext.girlfriend_from_default = false;

                Week.InitGirlfriend(roundcontext, trackmanifest.girlfriend);
            } else if (roundcontext.girlfriend == null || !roundcontext.girlfriend_from_default) {
                updated_stage = true;
                roundcontext.girlfriend_from_default = true;

                Week.InitGirlfriend(roundcontext, gameplaymanifest.@default.girlfriend);
            }

            // add additional pause menu
            if (trackmanifest.has_pause_menu) {
                roundcontext.pause_menu_from_default = false;

                roundcontext.weekpause.ExternalSetMenu(trackmanifest.pause_menu);
            } else if (!roundcontext.pause_menu_from_default) {
                roundcontext.pause_menu_from_default = true;

                roundcontext.weekpause.ExternalSetMenu(gameplaymanifest.@default.pause_menu);
            }

            // initialize the song tracks
            if (roundcontext.songplayer != null) roundcontext.songplayer.Destroy();
            roundcontext.songplayer = SongPlayer.Init(trackmanifest.song, initparams.alt_tracks);

            // initialize the gameover screen
            Week.InitUIGameover(roundcontext);

            // update location&size of UI cosmetics (if required)
            if (updated_ui || updated_stage) Week.InitUICosmetics(roundcontext);

            roundcontext.trackinfo.SetTextFormated(
                UI_TRACKINFO_FORMAT,
                trackmanifest.name,
                initparams.alt_tracks ? UI_TRACKINFO_ALT_SUFFIX : null,
                initparams.difficult
#if DEBUG
                , GameMain.ENGINE_VERSION
#endif
            );

            // Incorporates all ui elements in the stage layout
            if (updated_stage || updated_ui || updated_distributions_or_players) {
                Week.PlaceInLayout(roundcontext);
            }

            // toggle states
            Week.ToggleStates(roundcontext, gameplaymanifest);
            Week.UpdateBpm(roundcontext, roundcontext.settings.original_bpm);
            Week.UpdateSpeed(roundcontext, roundcontext.settings.original_speed);
            roundcontext.round_duration = trackmanifest.duration;
        }

        public static bool InitHealthbar(RoundContext roundcontext, GameplayManifest gameplaymanifest, bool force_update) {
            InitParams initparams = roundcontext.initparams;

            GameplayManifestHealthBar healthbarmanifest = gameplaymanifest.@default.healthbar;
            HealthBarParams healthbarparams = roundcontext.healthbarparams;

            if (gameplaymanifest.tracks[roundcontext.track_index].healthbar != null) {
                roundcontext.healthbar_from_default = false;
                healthbarmanifest = gameplaymanifest.tracks[roundcontext.track_index].healthbar;
            } else if (force_update || roundcontext.healthbar == null || !roundcontext.healthbar_from_default) {
                roundcontext.healthbar_from_default = true;
            } else {
                // no chages to make
                return false;
            }

            if (roundcontext.healthbar != null) {
                roundcontext.healthbar.Destroy();
                //roundcontext.healthbar = null;
            }

            Debug.Assert(healthbarmanifest != null, "the manifest of the healthbar was null");

            roundcontext.healthbar = new HealthBar(
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
            roundcontext.healthbar.EnableVertical(initparams.ui.healthbar_is_vertical);
            roundcontext.healthbar.SetLayoutSize(
                 initparams.ui_layout_width, initparams.ui_layout_height
            );
            if (healthbarmanifest.warnings_model != null) {
                ModelHolder modelholder_warn_icons = ModelHolder.Init(
                    healthbarmanifest.warnings_model
                );
                if (modelholder_warn_icons != null) {
                    roundcontext.healthbar.LoadWarnings(
                        modelholder_warn_icons,
                        healthbarmanifest.use_alt_icons
                    );
                    modelholder_warn_icons.Destroy();
                }
            }
            roundcontext.healthbar.SetVisible(!roundcontext.settings.no_healthbar);
            roundcontext.healthbar.DisableWarnings(initparams.ui.healthbar_nowarns);
            roundcontext.healthbar.SetBumpAnimation(initparams.animlist);


            ModelHolder default_icon_model_opponent = null;
            ModelHolder default_icon_model_player = null;

            if (!String.IsNullOrEmpty(healthbarparams.opponent_icon_model)) {
                default_icon_model_opponent = ModelHolder.Init(healthbarparams.opponent_icon_model);
                //free(healthbarparams.opponent_icon_model);
            }
            if (!String.IsNullOrEmpty(healthbarparams.player_icon_model)) {
                default_icon_model_player = ModelHolder.Init(healthbarparams.player_icon_model);
                //free(healthbarparams.player_icon_model);
            }

            // import healthbar states
            for (int i = 0 ; i < healthbarmanifest.states_size ; i++) {
                GameplayManifestHealthBarState state = healthbarmanifest.states[i];

                // temporal variables
                ModelHolder icon_modelholder = null;
                uint bar_color = state.opponent.bar_color;
                string bar_model = state.opponent.bar_model;

                if (String.IsNullOrEmpty(state.opponent.icon_model) && default_icon_model_opponent != null) {
                    // pick from the player manifest, player 0 shold be always the opponent
                    icon_modelholder = default_icon_model_opponent;
                } else if (!String.IsNullOrEmpty(state.opponent.icon_model)) {
                    icon_modelholder = ModelHolder.Init(state.opponent.icon_model);
                }

                if (!String.IsNullOrEmpty(bar_model)) {
                    ModelHolder modelholder_bar = ModelHolder.Init(bar_model);
                    if (modelholder_bar != null) {
                        roundcontext.healthbar.StateOpponentAdd(
                                 icon_modelholder, modelholder_bar, state.name
                        );
                        modelholder_bar.Destroy();
                    }
                } else {
                    roundcontext.healthbar.StateOpponentAdd2(
                        icon_modelholder, bar_color, state.name
                    );
                }
                if (icon_modelholder != default_icon_model_opponent) icon_modelholder.Destroy();

                icon_modelholder = null;
                bar_color = state.player.bar_color;
                bar_model = state.player.bar_model;
                if (String.IsNullOrEmpty(state.player.icon_model) && default_icon_model_player != null) {
                    // pick from the player manifest, player 0 shold be always the player
                    icon_modelholder = default_icon_model_player;
                } else if (!String.IsNullOrEmpty(state.player.icon_model)) {
                    icon_modelholder = ModelHolder.Init(state.player.icon_model);
                }

                if (!String.IsNullOrEmpty(bar_model)) {
                    ModelHolder modelholder_bar = ModelHolder.Init(bar_model);
                    roundcontext.healthbar.StatePlayerAdd(
                        icon_modelholder, modelholder_bar, state.name
                    );
                    modelholder_bar.Destroy();
                } else {
                    roundcontext.healthbar.StatePlayerAdd2(
                        icon_modelholder, bar_color, state.name
                    );
                }
                if (icon_modelholder != default_icon_model_player) icon_modelholder.Destroy();

                if (!String.IsNullOrEmpty(state.background.bar_model)) {
                    ModelHolder modelholder_bar = ModelHolder.Init(state.background.bar_model);
                    if (modelholder_bar != null) {
                        roundcontext.healthbar.StateBackgroundAdd(
                             modelholder_bar, state.name
                        );
                        modelholder_bar.Destroy();
                    }
                } else {
                    roundcontext.healthbar.StateBackgroundAdd2(
                        state.background.bar_color, null, state.name
                    );
                }
            }

            // if the healthbar does not have states, assign the bar colors if was specified
            if (healthbarmanifest.states_size < 1) {
                uint opponent_color_rgb8 = HealthBar.DEFAULT_COLOR_DAD;
                uint player_color_rgb8 = HealthBar.DEFAULT_COLOR_BOYFRIEND;

                if (healthbarmanifest.has_opponent_color)
                    opponent_color_rgb8 = healthbarmanifest.opponent_color_rgb8;
                else if (healthbarparams.has_opponent_color)
                    opponent_color_rgb8 = healthbarparams.opponent_color_rgb8;

                if (healthbarmanifest.has_player_color)
                    player_color_rgb8 = healthbarmanifest.player_color_rgb8;
                else if (healthbarparams.has_player_color)
                    player_color_rgb8 = healthbarparams.player_color_rgb8;

                roundcontext.healthbar.StateBackgroundAdd2(
                    HealthBar.DEFAULT_COLOR_BACKGROUND, null, null
                );
                roundcontext.healthbar.StateOpponentAdd2(
                     default_icon_model_opponent, opponent_color_rgb8, null
                );
                roundcontext.healthbar.StatePlayerAdd2(
                     default_icon_model_player, player_color_rgb8, null
                );
                roundcontext.healthbar.StateToggle(null);
            }

            roundcontext.healthbar.SetHealthPosition(1.0f, 0.5f, false);

            default_icon_model_opponent.Destroy();
            default_icon_model_player.Destroy();

            return true;
        }

        public static void InitGirlfriend(RoundContext roundcontext, GameplayManifestGirlfriend girlfriend_manifest) {
            InitParams initparams = roundcontext.initparams;

            if (roundcontext.girlfriend != null) {
                roundcontext.girlfriend.Destroy();
                roundcontext.girlfriend = null;
            }

            if (girlfriend_manifest == null) return;

            string manifest;
            switch (girlfriend_manifest.refer) {
                case GameplayManifestRefer.BOYFRIEND:
                    manifest = initparams.@default_boyfriend;
                    break;
                case GameplayManifestRefer.GIRLFRIEND:
                    manifest = initparams.@default_girlfriend;
                    break;
                default:
                    if (String.IsNullOrEmpty(girlfriend_manifest.manifest)) return;
                    manifest = girlfriend_manifest.manifest;
                    break;
            }

            CharacterManifest charactermanifest = new CharacterManifest(manifest, true);
            roundcontext.girlfriend = new Character(charactermanifest);
            charactermanifest.Destroy();
        }

        public static void InitStage(RoundContext roundcontext, string stage_src) {
            InitParams initparams = roundcontext.initparams;

            // keep the old layout if older assets are reused
            Layout old_layout = roundcontext.layout;
            LayoutPlaceholder placeholder;

            if (!String.IsNullOrEmpty(stage_src))
                roundcontext.layout = Layout.Init(stage_src);
            else
                roundcontext.layout = null;

            if (old_layout != null) old_layout.Destroy();

            //free(initparams.layout_girlfriend);
            //free(initparams.layout_characters);
            initparams.layout_characters_size = 0;

            initparams.layout_girlfriend = null;
            initparams.layout_characters = null;

            if (roundcontext.layout == null) return;

            // keep triggers synced
            roundcontext.layout.SyncTriggersWithGlobalBeatwatcher(true);

            // pick all player characters placement
            int count = (int)((long)roundcontext.layout.GetAttachedValue(
                "character_count", AttachedValueType.INTEGER, 0L
            ));

            initparams.layout_characters = count > 0 ? new LayoutCharacter[count] : null;
            initparams.layout_characters_size = count;

            for (int i = 0 ; i < count ; i++) {
                initparams.layout_characters[i] = new LayoutCharacter() {
                    align_vertical = Align.START,
                    align_horizontal = Align.START,
                    reference_width = -1f,
                    reference_height = -1f,
                    x = 0f,
                    y = 0f,
                    z = 0f,
                    scale = 1.0f,
                    placeholder_id = -1
                };

                placeholder = Week.InternalReadPlaceholder(
                    roundcontext.layout, Week.ROUND_CHARACTER_PREFIX, i
                );
                if (placeholder == null) continue;

                initparams.layout_characters[i].placeholder_id = roundcontext.layout.GetPlaceholderId(
                    placeholder.name
                );
                initparams.layout_characters[i].scale = (float)((double)Week.InternalReadValuePrefix(
                    roundcontext.layout, "character_scale_", i, AttachedValueType.FLOAT, 1.0
                ));

                initparams.layout_characters[i].x = placeholder.x;
                initparams.layout_characters[i].y = placeholder.y;
                initparams.layout_characters[i].z = placeholder.z;
                initparams.layout_characters[i].align_vertical = placeholder.align_vertical;
                initparams.layout_characters[i].align_horizontal = placeholder.align_horizontal;
                initparams.layout_characters[i].reference_width = placeholder.width;
                initparams.layout_characters[i].reference_height = placeholder.height;
            }

            // pick girlfriend character placement
            placeholder = roundcontext.layout.GetPlaceholder("character_girlfriend");
            if (placeholder != null) {
                initparams.layout_girlfriend = new LayoutCharacter() {
                    align_vertical = Align.START,
                    align_horizontal = Align.START,
                    reference_width = -1f,
                    reference_height = -1f,
                    x = 0f,
                    y = 0f,
                    z = 0f,
                    scale = 1.0f,
                    placeholder_id = -1
                };

                initparams.layout_girlfriend.placeholder_id = roundcontext.layout.GetPlaceholderId(
                    placeholder.name
                );
                initparams.layout_girlfriend.scale = (float)((double)roundcontext.layout.GetAttachedValue(
                    "character_girlfriend_scale", AttachedValueType.FLOAT, 1.0
                ));

                initparams.layout_girlfriend.x = placeholder.x;
                initparams.layout_girlfriend.y = placeholder.y;
                initparams.layout_girlfriend.z = placeholder.z;
                initparams.layout_girlfriend.align_vertical = placeholder.align_vertical;
                initparams.layout_girlfriend.align_horizontal = placeholder.align_horizontal;
                initparams.layout_girlfriend.reference_width = placeholder.width;
                initparams.layout_girlfriend.reference_height = placeholder.height;
            }

            Week.InternalPickCountersValuesFromLayout(roundcontext);
        }

        public static void InitScript(RoundContext roundcontext, string script_src) {
            if (roundcontext.script != null) {
                roundcontext.script.NotifyScriptchange();
                roundcontext.script.Destroy();
            }

            roundcontext.script = null;

            if (!String.IsNullOrEmpty(script_src)) {
                roundcontext.script = WeekScript.Init(script_src, roundcontext, true);
                if (roundcontext.dialogue != null) roundcontext.dialogue.SetScript(roundcontext.script);
            }

            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                if (roundcontext.players[i].strums != null) {
                    roundcontext.players[i].strums.SetParams(
                        roundcontext.players[i].ddrkeymon,
                        roundcontext.players[i].playerstats,
                        roundcontext.script
                    );
                }
            }
        }

        public static bool InitChartAndPlayers(RoundContext roundcontext, GameplayManifest gameplaymanifest, bool new_ui) {
            InitParams initparams = roundcontext.initparams;
            int track_index = roundcontext.track_index;

            Chart chart = new Chart(gameplaymanifest.tracks[track_index].chart, initparams.difficult);

            // keep just in case the same textures are used
            PlayerStruct[] old_players = roundcontext.players;
            int old_players_size = roundcontext.players_size;

            // update events table
            //free(roundcontext.events);
            roundcontext.events = CloneUtils.CloneArray(chart.events, chart.events_size);
            roundcontext.events_size = chart.events_size;
            roundcontext.events_peek_index = 0;
            roundcontext.settings.original_bpm = chart.bpm;
            roundcontext.settings.original_speed = chart.speed;

            // Pick players & strum distributions from default or current track
            bool distributions_from_default = !gameplaymanifest.tracks[track_index].has_distributions;
            bool players_from_default = !gameplaymanifest.tracks[track_index].has_players;
            GameplayManifestPlayer[] players = gameplaymanifest.@default.players;
            int players_size = gameplaymanifest.@default.players_size;
            Distribution[] distributions = gameplaymanifest.@default.distributions;

            if (gameplaymanifest.tracks[track_index].has_players) {
                players = gameplaymanifest.tracks[track_index].players;
                players_size = gameplaymanifest.tracks[track_index].players_size;
            }
            if (gameplaymanifest.tracks[track_index].has_distributions) {
                distributions = gameplaymanifest.tracks[track_index].distributions;
            }

            //
            // JS & C# only, guess if the keyboard should count as another gamepad
            // or the keyboard and the first connected gamepad should be mixed
            //
            int required_controllers = 0;
            for (int i = 0 ; i < players_size ; i++) {
                if (players[i].controller >= 0) required_controllers++;
            }
            bool mix_keyboard = required_controllers < 2;

            // check if is required update the current players & strum distributions
            bool same_players = roundcontext.players_from_default && players_from_default;
            bool same_distributions = roundcontext.distributions_from_default == distributions_from_default;
            if (same_players && same_distributions && old_players != null) {
                Week.InternalResetPlayersAndGirlfriend(roundcontext);

                // update only the strums and the character animations
                for (int i = 0 ; i < roundcontext.players_size ; i++) {
                    if (players[i].distribution_index >= 0 || roundcontext.players[i].strums != null) {
                        Distribution distribution = distributions[players[i].distribution_index];
                        roundcontext.players[i].strums.SetNotes(
                            chart,
                            distribution.strums,
                            distribution.strums_size,
                            roundcontext.players[i].notepool
                        );
                    }
                }

                chart.Destroy();

                if (new_ui) {
                    for (int i = 0 ; i < roundcontext.players_size ; i++) {
                        if (roundcontext.players[i].strums == null) continue;
                        int id = players[i].layout_strums_id < 0 ? i : players[i].layout_strums_id;

                        if (id >= initparams.layout_strums_size) {
                            Console.Error.WriteLine($"[ERROR] week_main() invalid player[{i}].layoutStrumsId={id}");
                            continue;
                        } else if (id < 0) {
                            continue;
                        }

                        LayoutStrum layout_strum = initparams.layout_strums[id];
                        roundcontext.players[i].strums.ForceRebuild(
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

                    Week.ChangeScrollDirection(roundcontext);
                }

                return false;
            }

            // remember where players and distributions was picked
            roundcontext.players_from_default = players_from_default;
            roundcontext.distributions_from_default = distributions_from_default;

            CharacterManifest[] charactermanifests = new CharacterManifest[players_size];
            CharacterManifest manifest_player = null;
            CharacterManifest manifest_opponent = null;

            roundcontext.players = new PlayerStruct[players_size];
            roundcontext.players_size = players_size;

            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                string manifest_src;
                switch (players[i].refer) {
                    case GameplayManifestRefer.BOYFRIEND:
                        manifest_src = initparams.@default_boyfriend;
                        break;
                    case GameplayManifestRefer.GIRLFRIEND:
                        manifest_src = initparams.@default_girlfriend;
                        break;
                    default:
                        manifest_src = players[i].manifest;
                        break;
                }

                charactermanifests[i] = new CharacterManifest(manifest_src, true);

                roundcontext.players[i] = new PlayerStruct() {
                    character = new Character(charactermanifests[i]),
                    conductor = new Conductor(),
                    playerstats = new PlayerStats(),
                    notepool = null,
                    strums = null,
                    type = CharacterType.ACTOR,
                    ddrkeymon = null,
                    ddrkeys_fifo = null,
                    controller = null,
                    is_vertical = true,
                    can_die = players[i].can_die,
                    can_recover = players[i].can_recover,
                    is_opponent = players[i].is_opponent
                };

                roundcontext.players[i].conductor.SetCharacter(
                    roundcontext.players[i].character
                );
                roundcontext.players[i].conductor.SetMissnotefx(roundcontext.missnotefx);

                roundcontext.players[i].character.FaceAsOpponent(roundcontext.players[i].is_opponent);

                // obtain the position in the UI layout
                int layout_strums_id = players[i].layout_strums_id < 0 ? i : players[i].layout_strums_id;

                if (layout_strums_id >= initparams.layout_strums_size) {
                    Console.Error.WriteLine($"[ERROR] week_main() invalid player[{i}].layoutStrumsId={layout_strums_id}");
                    layout_strums_id = -1;
                }

                if (layout_strums_id < 0 || players[i].distribution_index < 0) {
                    roundcontext.players[i].type = CharacterType.ACTOR;
                    continue;
                }

                Distribution distribution = distributions[players[i].distribution_index];
                CharacterType type = players[i].controller < 0 ? CharacterType.BOT : CharacterType.PLAYER;
                LayoutStrum layout_strum = initparams.layout_strums[layout_strums_id];

                if (type == CharacterType.PLAYER) {
                    roundcontext.players[i].playerstats.EnablePenalityOnEmptyStrum(
                        EngineSettings.penality_on_empty_strum
                    );
                    roundcontext.players[i].controller = new Gamepad(players[i].controller, mix_keyboard);
                    roundcontext.players[i].ddrkeymon = new DDRKeymon(
                        roundcontext.players[i].controller, distribution.strum_binds, distribution.strums_size
                    );
                    roundcontext.players[i].ddrkeys_fifo = roundcontext.players[i].ddrkeymon.GetFifo();
                }

                // initialize the player components: controller+strums+conductor+character
                roundcontext.players[i].is_vertical = layout_strum.is_vertical;
                roundcontext.players[i].type = type;
                roundcontext.players[i].notepool = new NotePool(
                    distribution.notes,
                    distribution.notes_size,
                    layout_strum.marker_dimmen,
                    layout_strum.invdimmen,
                    ScrollDirection.UPSCROLL
                );
                roundcontext.players[i].strums = new Strums(
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
                roundcontext.players[i].strums.SetParams(
                    roundcontext.players[i].ddrkeymon,
                    roundcontext.players[i].playerstats,
                    roundcontext.script
                );
                roundcontext.players[i].strums.SetNotes(
                    chart,
                    distribution.strums,
                    distribution.strums_size,
                    roundcontext.players[i].notepool
                );
                roundcontext.players[i].strums.SetScrollSpeed(chart.speed);
                roundcontext.players[i].strums.SetBpm(chart.bpm);
                roundcontext.players[i].strums.UseFunkinMakerDuration(EngineSettings.use_funkin_marker_duration);

                // attach strums and notes states
                for (int j = 0 ; j < distribution.states_size ; j++) {
                    DistributionStrumState state = distribution.states[j];
                    ModelHolder marker = null, sick_effect = null, background = null, notes = null;

                    if (!String.IsNullOrEmpty(state.model_marker)) marker = ModelHolder.Init(state.model_marker);
                    if (!String.IsNullOrEmpty(state.model_sick_effect)) sick_effect = ModelHolder.Init(state.model_sick_effect);
                    if (!String.IsNullOrEmpty(state.model_background) && FS.FileExists(state.model_background)) background = ModelHolder.Init(state.model_background);
                    if (!String.IsNullOrEmpty(state.model_notes)) notes = ModelHolder.Init(state.model_notes);

                    roundcontext.players[i].strums.StateAdd(
                        marker, sick_effect, background, state.name
                    );
                    if (!String.IsNullOrEmpty(state.model_notes))
                        roundcontext.players[i].notepool.AddState(notes, state.name);

                    if (marker != null) marker.Destroy();
                    if (sick_effect != null) sick_effect.Destroy();
                    if (background != null) background.Destroy();
                    if (notes != null) notes.Destroy();
                }

                // attach all character states
                for (int j = 0 ; j < players[i].states_size ; j++) {
                    ModelHolder modelholder = ModelHolder.Init(players[i].states[j].model);
                    if (modelholder != null) {
                        roundcontext.players[i].character.StateAdd(
                                modelholder, players[i].states[j].name
                        );
                        modelholder.Destroy();
                    }
                }

                // set alpha of all sustain notes
                roundcontext.players[i].notepool.ChangeAlphaSustain(layout_strum.sustain_alpha);

                // toggle default state
                roundcontext.players[i].strums.StateToggle(null);
                roundcontext.players[i].strums.EnablePostSickEffectDraw(true);

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

            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                if (roundcontext.players[i].strums == null) continue;
                // strums-->conductors-->character
                roundcontext.players[i].conductor.UseStrums(roundcontext.players[i].strums);
                int count = roundcontext.players[i].strums.GetLinesCount();
                int mapped = roundcontext.players[i].conductor.MapAutomatically(false);

                if (mapped != count) {
                    Console.Error.WriteLine($"[WARN] conductor_map_automatically() only mapped {mapped} of {count} lines");
                }
            }

            if (roundcontext.players_size > 0) {
                if (manifest_opponent == null) {
                    manifest_opponent = charactermanifests[0];
                }
                if (manifest_player == null && roundcontext.players_size > 1) {
                    manifest_opponent = charactermanifests[1];
                }
            }

            Week.ChangeScrollDirection(roundcontext);

            // set the health icon&color from the character manifest
            if (manifest_opponent != null) {
                roundcontext.healthbarparams.opponent_icon_model = manifest_opponent.model_health_icons;
                roundcontext.healthbarparams.has_opponent_color = manifest_opponent.has_healthbar_color;
                roundcontext.healthbarparams.opponent_color_rgb8 = manifest_opponent.healthbar_color;
            } else {
                roundcontext.healthbarparams.opponent_icon_model = null;
                roundcontext.healthbarparams.has_opponent_color = false;
            }
            if (manifest_player != null) {
                roundcontext.healthbarparams.player_icon_model = manifest_player.model_health_icons;
                roundcontext.healthbarparams.has_player_color = manifest_player.has_healthbar_color;
                roundcontext.healthbarparams.player_color_rgb8 = manifest_player.healthbar_color;
            } else {
                roundcontext.healthbarparams.player_icon_model = null;
                roundcontext.healthbarparams.has_player_color = false;
            }

            // dispose chart
            chart.Destroy();

            // dispose all charactermanifest loaded
            for (int i = 0 ; i < players_size ; i++) charactermanifests[i].Destroy();
            //free(charactermanifests);

            // dispose old players array
            for (int i = 0 ; i < old_players_size ; i++) {
                if (old_players[i].character != null) old_players[i].character.Destroy();
                if (old_players[i].conductor != null) old_players[i].conductor.Destroy();
                if (old_players[i].notepool != null) old_players[i].notepool.Destroy();
                if (old_players[i].strums != null) old_players[i].strums.Destroy();
                if (old_players[i].ddrkeymon != null) old_players[i].ddrkeymon.Destroy();
                if (old_players[i].controller != null) old_players[i].controller.Destroy();
            }
            //free(old_players);

            return true;
        }

        public static void InitUICosmetics(RoundContext roundcontext) {
            InitParams initparams = roundcontext.initparams;
            float viewport_width = 0f, viewport_height = 0f;
            Layout layout = roundcontext.layout != null ? roundcontext.layout : roundcontext.ui_layout;

            roundcontext.ui_layout.GetViewportSize(out viewport_width, out viewport_height);

            LayoutPlaceholder placeholder_streakcounter = Week.InternalReadPlaceholderCounter(
                layout, "ui_streakcounter"
            );
            LayoutPlaceholder placeholder_rankingcounter_rank = Week.InternalReadPlaceholderCounter(
                layout, "ui_rankingcounter_rank"
            );
            LayoutPlaceholder placeholder_rankingcounter_accuracy = Week.InternalReadPlaceholderCounter(
                layout, "ui_rankingcounter_accuracy"
            );

            // keep a copy of the old values
            RankingCounter old_rankingcounter = roundcontext.rankingcounter;
            StreakCounter old_streakcounter = roundcontext.streakcounter;
            Countdown old_countdown = roundcontext.countdown;
            SongProgressbar old_songprogressbar = roundcontext.songprogressbar;
            if (roundcontext.roundstats != null) roundcontext.roundstats.Destroy();
            if (roundcontext.trackinfo != null) roundcontext.trackinfo.Destroy();

            // step 1: initialize all "cosmetic" components
            ModelHolder modelholder_rankingstreak = ModelHolder.Init(UI_RANKINGCOUNTER_MODEL);
            ModelHolder modelholder_streakcounter = ModelHolder.Init(UI_STREAKCOUNTER_MODEL);
            ModelHolder modelholder_countdown = ModelHolder.Init(UI_COUNTDOWN_MODEL);

            // step 1a: ranking counter
            roundcontext.rankingcounter = new RankingCounter(
                placeholder_rankingcounter_rank,
                placeholder_rankingcounter_accuracy,
                initparams.font
            );
            roundcontext.rankingcounter.AddState(modelholder_rankingstreak, null);
            roundcontext.rankingcounter.ToggleState(null);
            roundcontext.rankingcounter.SetDefaultRankingAnimation(
                initparams.animlist
            );
            roundcontext.rankingcounter.SetDefaultRankingTextAnimation(
                initparams.animlist
            );
            roundcontext.rankingcounter.UsePercentInstead(
                initparams.ui.rankingcounter_percentonly
            );

            // step 1b: initializae streak counter
            roundcontext.streakcounter = new StreakCounter(
                placeholder_streakcounter,
                initparams.ui.streakcounter_comboheight,
                initparams.ui.streakcounter_numbergap,
                initparams.ui.streakcounter_delay
            );
            roundcontext.streakcounter.StateAdd(
                modelholder_rankingstreak, modelholder_streakcounter, null
            );
            roundcontext.streakcounter.StateToggle(null);
            roundcontext.streakcounter.SetNumberAnimation(
                initparams.animlist
            );
            roundcontext.streakcounter.SetComboAnimation(
                initparams.animlist
            );

            // step 1c: initialize roundstats
            roundcontext.roundstats = new RoundStats(
                initparams.ui.roundstats_x,
                initparams.ui.roundstats_y,
                initparams.ui.roundstats_z,
                initparams.font,
                initparams.ui.roundstats_size,
                viewport_width
            );
            roundcontext.roundstats.HideNps(initparams.ui.roundstats_hide);


            // step 1d: initialize songprogressbar
            if (EngineSettings.song_progressbar) {
                roundcontext.songprogressbar = new SongProgressbar(
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
                roundcontext.songprogressbar.SetSongplayer(roundcontext.songplayer);
                roundcontext.songprogressbar.SetDuration(roundcontext.round_duration);
                if (EngineSettings.song_progressbar_remaining) roundcontext.songprogressbar.ShowElapsed(false);
            } else {
                roundcontext.songprogressbar = null;
            }

            // step 1e: initialize countdown
            roundcontext.countdown = new Countdown(
                modelholder_countdown,
                initparams.ui.countdown_height
            );
            roundcontext.countdown.SetDefaultAnimation(initparams.animlist);
            roundcontext.countdown.SetLayoutViewport(
                initparams.ui_layout_width, initparams.ui_layout_height
            );

            // step 1f: initialize trackinfo
            roundcontext.trackinfo = TextSprite.Init2(
                initparams.font, initparams.ui.trackinfo_fontsize, initparams.ui.trackinfo_fontcolor
            );
            roundcontext.trackinfo.SetAlign(
                initparams.ui.trackinfo_alignvertical,
                initparams.ui.trackinfo_alignhorinzontal
            );
            roundcontext.trackinfo.SetMaxDrawSize(
                initparams.ui.trackinfo_maxwidth,
                initparams.ui.trackinfo_maxheight
            );
            roundcontext.trackinfo.SetDrawLocation(
                initparams.ui.trackinfo_x, initparams.ui.trackinfo_y
            );
            roundcontext.trackinfo.SetZIndex(initparams.ui.trackinfo_z);
            roundcontext.trackinfo.BorderEnable(true);
            roundcontext.trackinfo.BorderSetSize(1f);
            roundcontext.trackinfo.BorderSetColorRGBA8(0x000000FF);// black

            // step 2: dispose all modelholders used
            modelholder_rankingstreak.Destroy();
            modelholder_streakcounter.Destroy();
            modelholder_countdown.Destroy();

            // step 3: dispose old ui elements
            if (old_rankingcounter != null) old_rankingcounter.Destroy();
            if (old_streakcounter != null) old_streakcounter.Destroy();
            if (old_countdown != null) old_countdown.Destroy();
            if (old_songprogressbar != null) old_songprogressbar.Destroy();
        }

        public static void InitUIGameover(RoundContext roundcontext) {
            WeekGameOver old_weekgameover = roundcontext.weekgameover;
            int version = WeekGameOver.ReadVersion();

            if (old_weekgameover != null && version == roundcontext.weekgameover_from_version) return;

            // build the gameover screen and dispose the older one
            roundcontext.weekgameover = new WeekGameOver();
            roundcontext.weekgameover_from_version = version;
            if (old_weekgameover != null) old_weekgameover.Destroy();
        }

        private static bool InitDialogue(RoundContext roundcontext, string dialogue_params) {
            if (String.IsNullOrEmpty(dialogue_params)) return false;
            if (roundcontext.dialogue != null) roundcontext.dialogue.Destroy();
            roundcontext.dialogue = Dialogue.Init(dialogue_params);
            if (roundcontext.dialogue != null) roundcontext.dialogue.SetScript(roundcontext.script);
            return roundcontext.dialogue != null;
        }


        public static void PlaceInLayout(RoundContext roundcontext) {
            InitParams initparams = roundcontext.initparams;
            const byte UI_SIZE = 8;// all UI "cosmetics" elements + screen background + dialogue

            Layout layout; bool is_ui;
            if (roundcontext.layout != null) {
                is_ui = false;
                layout = roundcontext.layout;
                if (roundcontext.ui_layout != null) roundcontext.ui_layout.ExternalVertexCreateEntries(0);
            } else {
                is_ui = true;
                layout = roundcontext.ui_layout;
                if (roundcontext.layout != null) roundcontext.layout.ExternalVertexCreateEntries(0);
            }

            // if there no stage layout, hide all characters
            if (is_ui) return;

            // step 1: declare the amout of items to add
            int size = UI_SIZE;
            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                // count strums
                if (roundcontext.players[i].strums != null) size++;
            }

            int ui1 = layout.ExternalCreateGroup(Week.ROUND_UI_GROUP_NAME, 0);
            if (ui1 < 0) ui1 = layout.GetGroupId(Week.ROUND_UI_GROUP_NAME);
            layout.SetGroupStaticToScreenById(ui1, Week.ROUND_UI_MATRIX_CAMERA);

            int ui2 = layout.ExternalCreateGroup(Week.ROUND_UI_GROUP_NAME2, 0);
            if (ui2 < 0) ui2 = layout.GetGroupId(Week.ROUND_UI_GROUP_NAME2);
            layout.SetGroupStaticToScreenById(ui2, Week.ROUND_UI_MATRIX);

            // allocate space for all ui items
            roundcontext.layout.ExternalVertexCreateEntries(size);

            // step 2: place all UI elements
            layout.ExternalVertexSetEntry(
                0, PVRContextVertex.DRAWABLE, roundcontext.healthbar.GetDrawable(), ui1
            );
            layout.ExternalVertexSetEntry(
                1, PVRContextVertex.DRAWABLE, roundcontext.roundstats.GetDrawable(), ui1
            );
            layout.ExternalVertexSetEntry(
                2, PVRContextVertex.DRAWABLE, roundcontext.songprogressbar != null ? roundcontext.songprogressbar.GetDrawable() : null, ui1
            );
            layout.ExternalVertexSetEntry(
                3, PVRContextVertex.DRAWABLE, roundcontext.countdown.GetDrawable(), ui1
            );
            layout.ExternalVertexSetEntry(
                4, PVRContextVertex.TEXTSPRITE, roundcontext.trackinfo, ui1
            );
            layout.ExternalVertexSetEntry(
                5, PVRContextVertex.DRAWABLE, roundcontext.weekgameover.GetDrawable(), ui2
            );
            layout.ExternalVertexSetEntry(
                6, PVRContextVertex.SPRITE, roundcontext.screen_background, ui2
            );
            layout.ExternalVertexSetEntry(
                7, PVRContextVertex.DRAWABLE, roundcontext.dialogue != null ? roundcontext.dialogue.GetDrawable() : null, ui2
            );

            // step 3: initialize the ui camera
            Modifier modifier = roundcontext.ui_camera.GetModifier();
            roundcontext.ui_camera.Stop();
            modifier.width = PVRContext.global_context.ScreenWidth;
            modifier.height = PVRContext.global_context.ScreenHeight;

            // step 4: place girlfriend
            if (roundcontext.girlfriend != null) {
                if (initparams.layout_girlfriend != null) {
                    Week.InternalPlaceCharacter(
                        layout,
                        roundcontext.girlfriend,
                        roundcontext.initparams.layout_girlfriend
                    );
                } else {
                    Console.Error.WriteLine("[ERROR] week_place_in_layout() missing layout space for declared girlfriend");
                }
            }

            // step 5: place all player characters & strums
            for (int i = 0, j = UI_SIZE ; i < roundcontext.players_size ; i++) {
                if (roundcontext.players[i].character != null) {
                    if (i < initparams.layout_characters_size) {
                        Week.InternalPlaceCharacter(
                            layout,
                            roundcontext.players[i].character,
                            initparams.layout_characters[i]
                        );
                    } else {
                        Console.Error.WriteLine($"[ERROR] week_place_in_layout() missing layout space for declared player[{i}]");
                    }
                }
                if (roundcontext.players[i].strums != null) {
                    layout.ExternalVertexSetEntry(
                        j++, PVRContextVertex.DRAWABLE, roundcontext.players[i].strums.GetDrawable(), ui1
                    );
                }
            }

            Week.UISetVisibility(roundcontext, true);
            return;
        }

        public static void ToggleStates(RoundContext roundcontext, GameplayManifest gameplaymanifest) {
            GameplayManifestTrack track = gameplaymanifest.tracks[roundcontext.track_index];

            if (track.has_selected_state_name) {
                for (int i = 0 ; i < roundcontext.players_size ; i++) {
                    if (roundcontext.players[i].strums != null)
                        roundcontext.players[i].strums.StateToggle(track.selected_state_name);
                    if (roundcontext.players[i].character != null)
                        roundcontext.players[i].character.StateToggle(track.selected_state_name);
                }
                roundcontext.healthbar.StateToggle(track.selected_state_name);
            }

            int size = track.selected_state_name_per_player_size;
            if (roundcontext.players_size < size) size = roundcontext.players_size;
            for (int i = 0 ; i < size ; i++) {
                string state_name = track.selected_state_name_per_player[i];
                if (roundcontext.players[i].strums != null)
                    roundcontext.players[i].strums.StateToggle(state_name);
                if (roundcontext.players[i].character != null)
                    roundcontext.players[i].character.StateToggle(state_name);
            }
        }

        public static void UpdateBpm(RoundContext roundcontext, float bpm) {
            if (roundcontext.healthbar != null) roundcontext.healthbar.SetBpm(bpm);
            if (roundcontext.countdown != null) roundcontext.countdown.SetBpm(bpm);
            if (roundcontext.roundstats != null) roundcontext.roundstats.TweenkeyframeSetBpm(bpm);
            if (roundcontext.ui_camera != null) roundcontext.ui_camera.SetBpm(bpm);
            if (roundcontext.layout != null) roundcontext.layout.SetBpm(bpm);
            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                if (roundcontext.players[i].character != null) {
                    roundcontext.players[i].character.SetBpm(bpm);
                }
                if (roundcontext.players[i].strums != null) {
                    roundcontext.players[i].strums.SetBpm(bpm);
                }
            }
            Week.BEAT_WATCHER.ChangeBpm(bpm);
            Week.QUARTER_WATCHER.ChangeBpm(bpm);

            //Console.Error.WriteLine($"[INFO] week_update_bpm() original={roundcontext.settings.original_bpm} new={bpm}");

            roundcontext.settings.bpm = bpm;
        }

        public static void UpdateSpeed(RoundContext roundcontext, double speed) {
            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                if (roundcontext.players[i].character != null) {
                    roundcontext.players[i].character.SetIdleSpeed((float)speed);
                }
            }

            if (roundcontext.girlfriend != null) roundcontext.girlfriend.SetIdleSpeed((float)speed);

            roundcontext.settings.speed = speed;
        }

        public static void DisableLayoutRollback(RoundContext roundcontext, bool disable) {
            roundcontext.settings.layout_rollback = !disable;
        }

        public static void OverrideCommonFolder(RoundContext roundcontext, string custom_common_path) {
            FS.OverrideCommonFolder(custom_common_path);
        }


        public static int Round(RoundContext roundcontext, bool from_retry, bool show_dialog) {
            GamepadButtons pressed_buttons = GamepadButtons.NOTHING;
            SongPlayerInfo songinfo = new SongPlayerInfo { timestamp = Double.NaN, completed = true };
            InitParams initparams = roundcontext.initparams;
            PVRContext pvr_context = PVRContext.global_context;

            float elapsed;
            bool check_ready = roundcontext.settings.ask_ready;
            bool do_countdown = roundcontext.settings.do_countdown;
            PlayerStats playerstats = null;
            Camera camera = roundcontext.layout != null ? roundcontext.layout.GetCameraHelper() : null;
            double round_duration = roundcontext.round_duration;
            double round_end_timestamp;
            double unmute_timestamp = Double.PositiveInfinity;

            if (round_duration < 0) round_duration = Double.PositiveInfinity;
            if (roundcontext.layout != null) roundcontext.layout.Resume();

            if (roundcontext.songprogressbar != null) {
                double duration = roundcontext.songplayer != null ? roundcontext.songplayer.GetDuration() : round_duration;
                roundcontext.songprogressbar.ManualSetPosition(0.0, duration, true);
                roundcontext.songprogressbar.ManualUpdateEnable(true);
            }

            if (roundcontext.script != null) {
                roundcontext.script.NotifyTimerSong(0.0);
                roundcontext.script.NotifyBeforeready(from_retry);
            }
            Week.Halt(roundcontext, true);

            if (roundcontext.playerstats_index >= 0) {
                playerstats = roundcontext.players[roundcontext.playerstats_index].playerstats;
                roundcontext.roundstats.PeekPlayerstats(0, playerstats);
            }

            while (show_dialog) {
                elapsed = pvr_context.WaitReady();
                pvr_context.Reset();
                InternalCheckScreenResolution(roundcontext, false);
                BeatWatcher.GlobalSetTimestampFromKosTimer();

                if (pvr_context.IsOffscreen()) {
                    roundcontext.layout.Suspend();
                    roundcontext.dialogue.Suspend();

                    int decision = roundcontext.weekpause.HelperShow(roundcontext, -1);
                    switch (decision) {
                        case 1:
                            return 2;// restart song
                        case 2:
                            return 1;// back to weekselector
                        case 3:
                            return 3;// back to mainmenu
                    }

                    roundcontext.layout.Resume();
                    roundcontext.dialogue.Resume();
                    continue;
                }

                if (roundcontext.script != null) roundcontext.script.NotifyFrame(elapsed);
                roundcontext.layout.Animate(elapsed);
                roundcontext.layout.Draw(pvr_context);

                if (roundcontext.dialogue.IsCompleted()) {
                    show_dialog = false;
                    for (int i = 0 ; i < roundcontext.players_size ; i++) {
                        if (roundcontext.players[i].controller != null) {
                            roundcontext.players[i].controller.ClearButtons();// antibounce
                        }
                    }
                }
            }

            if (check_ready) roundcontext.countdown.Ready();
            else if (do_countdown) roundcontext.countdown.Start();

            int dettached_controller_index = -1;
            bool back_pressed = false;

            while (check_ready || do_countdown) {
                elapsed = pvr_context.WaitReady();
                pvr_context.Reset();

                InternalCheckScreenResolution(roundcontext, false);
                BeatWatcher.GlobalSetTimestampFromKosTimer();

                if (pvr_context.IsOffscreen() || dettached_controller_index >= 0 || back_pressed) {
                    int decision = roundcontext.weekpause.HelperShow(
                         roundcontext, dettached_controller_index
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
                    back_pressed = false;
                    continue;
                }

                if (check_ready) {
                    int is_ready = 0;
                    bool bot_only = true;

                    for (int i = 0 ; i < roundcontext.players_size ; i++) {
                        Gamepad controller = roundcontext.players[i].controller;
                        if (controller == null) continue;
                        bot_only = false;
                        if (controller.IsDettached()) {
                            dettached_controller_index = controller.GetControllerIndex();
                            break;
                        }
                        if (controller.HasPressed(Week.ROUND_READY_BUTTONS) != GamepadButtons.NOTHING) {
                            controller.ClearButtons();// antibouce
                            is_ready++;
                        }
                        if (controller.HasPressed(GamepadButtons.BACK) != GamepadButtons.NOTHING) {
                            is_ready = 0;
                            back_pressed = true;
                            break;
                        }

                        if (controller.GetManagedPresses(false, ref pressed_buttons)) {

                            if (roundcontext.script != null) roundcontext.script.NotifyButtons(i, pressed_buttons);
                        }
                    }

                    if (is_ready > 0 || bot_only) {
                        check_ready = false;


                        if (roundcontext.script != null) roundcontext.script.NotifyReady();

                        Week.Halt(roundcontext, true);

                        if (do_countdown) roundcontext.countdown.Start();
                        if (roundcontext.layout != null)
                            roundcontext.layout.TriggerCamera(Week.ROUND_CAMERA_ROUNDSTART);
                    }
                } else if (roundcontext.countdown.HasEnded()) {
                    do_countdown = false;
                }

                if (roundcontext.scriptcontext.force_end_flag) {
                    if (roundcontext.scriptcontext.force_end_loose_or_win)
                        return 1;// give-up and returns to the week selector
                    else
                        return 0;
                }


                if (roundcontext.script != null) roundcontext.script.NotifyFrame(elapsed);
                roundcontext.layout.Animate(elapsed);
                roundcontext.layout.Draw(pvr_context);
            }

            if (!roundcontext.settings.ask_ready && roundcontext.layout != null) {
                roundcontext.layout.TriggerCamera(Week.ROUND_CAMERA_ROUNDSTART);
            }

            if (roundcontext.songprogressbar != null) roundcontext.songprogressbar.ManualUpdateEnable(false);

            if (roundcontext.script != null) roundcontext.script.NotifyAftercountdown();
            Week.Halt(roundcontext, true);

            // start this round!!!!!
            dettached_controller_index = -1;
            double elapsed_play = roundcontext.songplayer != null ? roundcontext.songplayer.Play(songinfo) : 0.0;

            // prepare beatwatchers
            BeatWatcher.GlobalSetTimestamp(elapsed_play);
            Week.BEAT_WATCHER.Reset(true, roundcontext.settings.bpm);
            Week.QUARTER_WATCHER.Reset(false, roundcontext.settings.bpm);

            bool gameover = false;
            int next_camera_bump = 0;
            DDRKeymon has_reference_ddrkeymon = null;
            double song_timestamp = 0.0;

            round_end_timestamp = timer.ms_gettime64() + round_duration;

            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                if (roundcontext.players[i].ddrkeymon != null) {
                    roundcontext.players[i].controller.ClearButtons();
                    has_reference_ddrkeymon = roundcontext.players[i].ddrkeymon;
                    roundcontext.players[i].ddrkeymon.Start(songinfo.timestamp);
                }
            }

            // gameplay logic
            while (timer.ms_gettime64() < round_end_timestamp && !songinfo.completed) {
                // wait for frame
                elapsed = pvr_context.WaitReady();
                pvr_context.Reset();

                InternalCheckScreenResolution(roundcontext, false);

                // check for pause
                bool paused = false;
                for (int i = 0 ; i < roundcontext.players_size ; i++) {
                    if (roundcontext.players[i].controller == null) continue;
                    if (roundcontext.players[i].controller.IsDettached()) {
                        dettached_controller_index = roundcontext.players[i].controller.GetControllerIndex();
                        paused = true;
                        break;
                    }

                    // important: use .GetLastPressed() to avoid mess up the maple pad or keyboard inputs
                    if ((roundcontext.players[i].controller.GetLastPressed() & (GamepadButtons.START | GamepadButtons.BACK)) != GamepadButtons.NOTHING) {
                        paused = true;
                        break;
                    }
                }

                if (paused || pvr_context.IsOffscreen()) {
                    // pause all critical stuff
                    round_duration = round_end_timestamp - timer.ms_gettime64();
                    roundcontext.layout.Suspend();
                    if (roundcontext.songplayer != null) roundcontext.songplayer.Pause();
                    for (int i = 0 ; i < roundcontext.players_size ; i++) {
                        if (roundcontext.players[i].ddrkeymon != null) {
                            roundcontext.players[i].ddrkeymon.Stop();
                        }
                    }

                    // display pause menu (visible when the screen focus returns)
                    int decision = roundcontext.weekpause.HelperShow(
                        roundcontext, dettached_controller_index
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
                    round_end_timestamp = timer.ms_gettime64() + round_duration;
                    roundcontext.layout.Resume();

                    if (roundcontext.songplayer != null) {
                        roundcontext.songplayer.Seek(song_timestamp);
                        roundcontext.songplayer.Play(songinfo);
                    }

                    for (int i = 0 ; i < roundcontext.players_size ; i++) {
                        if (roundcontext.players[i].ddrkeymon != null) {
                            roundcontext.players[i].controller.ClearButtons();
                            roundcontext.players[i].ddrkeymon.Start(-song_timestamp);
                        }
                    }

                    continue;
                }

                if (roundcontext.songplayer != null) roundcontext.songplayer.Poll(songinfo);

                if (has_reference_ddrkeymon != null)
                    song_timestamp = has_reference_ddrkeymon.PeekTimestamp();
                else
                    song_timestamp = songinfo.timestamp;

                // update beatwatchers
                BeatWatcher.GlobalSetTimestamp(song_timestamp);

                // Important: use only in key checking and/or strum scroll
                double song_timestamp2 = song_timestamp;

                //
                // If Strums.SetDrawOffset() is not used, enable this line
                //
                song_timestamp2 -= EngineSettings.input_offset;

                double missed_milliseconds = 0.0;

                for (int i = 0 ; i < roundcontext.players_size ; i++) {
                    switch (roundcontext.players[i].type) {
                        case CharacterType.BOT:
                            roundcontext.players[i].strums.ScrollAuto(song_timestamp2);
                            roundcontext.players[i].conductor.Poll();
                            missed_milliseconds += roundcontext.players[i].conductor.GetMissedMilliseconds();
                            break;
                        case CharacterType.PLAYER:
                            roundcontext.players[i].ddrkeymon.PollCSJS();
                            roundcontext.players[i].strums.ScrollFull(song_timestamp2);
                            roundcontext.players[i].conductor.Poll();
                            if (roundcontext.players[i].controller.GetManagedPresses(false, ref pressed_buttons)) {
                                if (roundcontext.script != null) roundcontext.script.NotifyButtons(i, pressed_buttons);
                            }
                            missed_milliseconds += roundcontext.players[i].conductor.GetMissedMilliseconds();
                            break;
                    }
                }

                if (roundcontext.script != null) {
                    roundcontext.script.NotifyAfterStrumScroll();
                    if (roundcontext.scriptcontext.halt_flag) Week.Halt(roundcontext, false);
                }

                if (playerstats != null) {
                    roundcontext.rankingcounter.PeekRanking(playerstats);
                    if (roundcontext.streakcounter.PeekStreak(playerstats)) {
                        if (roundcontext.girlfriend != null && roundcontext.settings.girlfriend_cry) {
                            roundcontext.girlfriend.PlayExtra(Funkin.GIRLFRIEND_COMBOBREAK, false);
                        }
                    }
                    roundcontext.roundstats.PeekPlayerstats(songinfo.timestamp, playerstats);
                }

                roundcontext.healthwatcher.Balance(roundcontext.healthbar);

                if (roundcontext.healthwatcher.HasDeads(true) > 0) {
                    gameover = true;
                    break;
                }

                if (roundcontext.songplayer != null) {
                    if (/*!Double.IsNaN(missed_milliseconds) && */missed_milliseconds > 0.0) {
                        if (!Double.IsInfinity(unmute_timestamp)) {
                            unmute_timestamp += missed_milliseconds / 2.0;
                            roundcontext.songplayer.MuteTrack(true, true);
                        } else {
                            unmute_timestamp = song_timestamp + missed_milliseconds;
                            roundcontext.songplayer.MuteTrack(true, true);
                        }
                    } else if (song_timestamp > unmute_timestamp) {
                        unmute_timestamp = Double.PositiveInfinity;
                        roundcontext.songplayer.MuteTrack(true, false);
                    }
                }


                Week.PeekChartEvents(roundcontext, song_timestamp);

                if (Week.BEAT_WATCHER.Poll()) {
                    // bump UI
                    if (Week.BEAT_WATCHER.count > next_camera_bump) {
                        next_camera_bump += 4;
                        if (roundcontext.settings.camera_bumping) {
                            camera.Animate(Week.BEAT_WATCHER.since);
                            roundcontext.ui_camera.Slide(Single.NaN, Single.NaN, 1.05f, Single.NaN, Single.NaN, 1.0f);
                        }
                    }

                    // notify script
                    if (roundcontext.script != null) roundcontext.script.NotifyBeat(
                         Week.BEAT_WATCHER.count, Week.BEAT_WATCHER.since
                      );
                }

                if (Week.QUARTER_WATCHER.Poll()) {

                    if (roundcontext.script != null) roundcontext.script.NotifyQuarter(
                        Week.QUARTER_WATCHER.count, Week.QUARTER_WATCHER.since
                      );
                }

                // animate camera
                roundcontext.ui_camera.Animate(elapsed);

                // flush modifier
                roundcontext.ui_camera.Apply(null);
                Week.ROUND_UI_MATRIX.CopyTo(Week.ROUND_UI_MATRIX_CAMERA);
                roundcontext.ui_camera.ApplyOffset(Week.ROUND_UI_MATRIX_CAMERA);
                Week.ROUND_UI_MATRIX_CAMERA.ApplyModifier(roundcontext.ui_camera.GetModifier());


                if (roundcontext.script != null) {
                    roundcontext.script.NotifyTimerSong(song_timestamp);
                    roundcontext.script.NotifyFrame(elapsed);
                }
                roundcontext.layout.Animate(elapsed);
                roundcontext.layout.Draw(pvr_context);

                if (roundcontext.scriptcontext.halt_flag) Week.Halt(roundcontext, false);

                if (roundcontext.scriptcontext.force_end_flag) {
                    if (roundcontext.songplayer != null) roundcontext.songplayer.Pause();
                    if (roundcontext.scriptcontext.force_end_loose_or_win) {
                        return 1;// give-up and returns to the week selector
                    } else {
                        return 0;// interrupt the gameplay
                    }
                }
            }

            // stop all ddrkeymon instances
            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                if (roundcontext.players[i].ddrkeymon != null) {
                    roundcontext.players[i].ddrkeymon.Stop();
                }
            }

            // notify the script about the current round result and halt (if necessary)
            if (roundcontext.songplayer != null) roundcontext.songplayer.Pause();
            roundcontext.layout.TriggerCamera(Week.ROUND_CAMERA_ROUNDEND);

            if (roundcontext.script != null) roundcontext.script.NotifyRoundend(gameover);

            Week.Halt(roundcontext, true);
            Week.UISetVisibility(roundcontext, false);

            if (gameover) {
                double duration = roundcontext.round_duration;
                if (roundcontext.songplayer != null) {
                    if (duration < 0) duration = roundcontext.songplayer.GetDuration();
                    roundcontext.songplayer.Pause();
                }

                roundcontext.weekgameover.Display(
                    song_timestamp,
                    duration,
                    playerstats,
                    initparams.weekinfo,
                    initparams.difficult
                );

                // ask for player decision
                int decision = roundcontext.weekgameover.HelperAskToPlayer(roundcontext);
                roundcontext.track_difficult = roundcontext.weekgameover.GetDifficult();
                roundcontext.weekgameover.Hide();

                // notify script and wait (if necessary)

                if (roundcontext.script != null) roundcontext.script.NotifyDiedecision(decision > 0);

                Week.Halt(roundcontext, true);

                return decision;
            }

            return 0;// round win
        }


        public static void Halt(RoundContext roundcontext, bool poke_global_beatwatcher) {
            if (!roundcontext.scriptcontext.halt_flag) return;

            GamepadButtons preesed = GamepadButtons.NOTHING;

            Console.Error.WriteLine("[LOG] week_halt() waiting for script signal...");

            if (poke_global_beatwatcher) BeatWatcher.GlobalSetTimestampFromKosTimer();

            while (roundcontext.scriptcontext.halt_flag) {
                float elapsed = PVRContext.global_context.WaitReady();
                PVRContext.global_context.Reset();

                InternalCheckScreenResolution(roundcontext, false);

                for (int i = 0 ; i < roundcontext.players_size ; i++) {
                    Gamepad controller = roundcontext.players[i].controller;
                    if (controller != null && controller.GetManagedPresses(true, ref preesed)) {

                        if (roundcontext.script != null) roundcontext.script.NotifyButtons(i, preesed);
                    }
                }


                if (roundcontext.script != null) roundcontext.script.NotifyFrame(elapsed);
                roundcontext.layout.Animate(elapsed);
                roundcontext.layout.Draw(PVRContext.global_context);

                if (poke_global_beatwatcher) BeatWatcher.GlobalSetTimestampFromKosTimer();

                if (roundcontext.scriptcontext.force_end_flag) {
                    Console.Error.WriteLine("[LOG] week_halt() wait interrupted because week_end() was called");
                    return;
                }
            }

            Console.Error.WriteLine("[LOG] week_halt() wait done");
        }

        public static void PeekChartEvents(RoundContext roundcontext, double timestamp) {
            int i = roundcontext.events_peek_index;

            for (; i < roundcontext.events_size ; i++) {
                if (timestamp < roundcontext.events[i].timestamp) break;
                switch (roundcontext.events[i].command) {
                    case ChartEvent.CAMERA_OPPONENT:
                        Week.InternalCameraFocusGuess(roundcontext, true);
                        break;
                    case ChartEvent.CAMERA_PLAYER:
                        Week.InternalCameraFocusGuess(roundcontext, false);
                        break;
                    case ChartEvent.CHANGE_BPM:
                        Week.UpdateBpm(roundcontext, (float)roundcontext.events[i].parameter);
                        break;
                    case ChartEvent.ALT_ANIM_OPPONENT:
                        for (int j = 0 ; j < roundcontext.players_size ; j++) {
                            if (roundcontext.players[j].is_opponent) {
                                roundcontext.players[j].character.UseAlternateSingAnimations(
                                    roundcontext.events[i].parameter == 1.0
                                );
                            }
                        }
                        break;
                    case ChartEvent.ALT_ANIM_PLAYER:
                        for (int j = 0 ; j < roundcontext.players_size ; j++) {
                            if (roundcontext.players[j].type == CharacterType.PLAYER) {
                                roundcontext.players[j].character.UseAlternateSingAnimations(
                                    roundcontext.events[i].parameter == 1.0
                                );
                            }
                        }
                        break;
                    case ChartEvent.UNKNOWN_NOTE:
                        if (roundcontext.script != null) {
                            roundcontext.script.NotifyUnknownNote(
                              roundcontext.events[i].is_player_or_opponent ? 1 : 0,
                              roundcontext.events[i].timestamp,
                              (int)roundcontext.events[i].parameter,
                              roundcontext.events[i].parameter2,
                              roundcontext.events[i].parameter3
                          );
                        }
                        break;
                }
            }

            if (i != roundcontext.events_peek_index) roundcontext.events_peek_index = i;
        }


        public static void CheckDirectivesRound(RoundContext roundcontext, bool completed) {
            foreach (ModifiedDirective directive_info in roundcontext.scriptcontext.directives) {
                if (directive_info.completed_week) continue;

                if (completed || !directive_info.completed_round) {
                    if (directive_info.create)
                        FunkinSave.CreateUnlockDirective(directive_info.name, directive_info.value);
                    else
                        FunkinSave.DeleteUnlockDirective(directive_info.name);
                }

                // remove this item from the list
                roundcontext.scriptcontext.directives.RemoveItem(directive_info);
                //free(directive_info.name);
                //free(directive_info);
            }
        }

        public static void CheckDirectivesWeek(RoundContext roundcontext, bool completed) {
            foreach (ModifiedDirective directive_info in roundcontext.scriptcontext.directives) {
                if (completed || !directive_info.completed_week) {
                    if (directive_info.create)
                        FunkinSave.CreateUnlockDirective(directive_info.name, directive_info.value);
                    else
                        FunkinSave.DeleteUnlockDirective(directive_info.name);
                }

                // remove this item from the list
                roundcontext.scriptcontext.directives.RemoveItem(directive_info);
                //free(directive_info.name);
                //free(directive_info);
                roundcontext.has_directive_changes = true;
            }
        }

        public static void ChangeScrollDirection(RoundContext roundcontext) {
            ScrollDirection direction;
            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                if (roundcontext.players[i].strums == null) continue;

                if (roundcontext.players[i].is_vertical)
                    direction = EngineSettings.inverse_strum_scroll ? ScrollDirection.DOWNSCROLL : ScrollDirection.UPSCROLL;
                else
                    direction = EngineSettings.inverse_strum_scroll ? ScrollDirection.RIGHTSCROLL : ScrollDirection.LEFTSCROLL;

                roundcontext.players[i].strums.SetScrollDirection(direction);
                roundcontext.players[i].notepool.ChangeScrollDirection(direction);
            }
        }

        public static void UnlockDirectiveCreate(RoundContext roundcontext, string name, bool completed_round, bool completed_week, double value) {
            if (name == null) return;

            foreach (ModifiedDirective directive_info in roundcontext.scriptcontext.directives) {
                if (directive_info.name == name) {
                    directive_info.completed_round = !!completed_round;
                    directive_info.completed_week = !!completed_week;
                    directive_info.value = value;
                    directive_info.create = true;
                    return;
                }
            }

            roundcontext.scriptcontext.directives.AddItem(new ModifiedDirective {
                name = name,
                completed_round = !!completed_round,
                completed_week = !!completed_week,
                value = value,
                create = true
            });
        }

        public static void UnlockdirectiveRemove(RoundContext roundcontext, string name, bool completed_round, bool completed_week) {
            foreach (ModifiedDirective directive_info in roundcontext.scriptcontext.directives) {
                if (directive_info.name == name) {
                    directive_info.completed_round = !!completed_round;
                    directive_info.completed_week = !!completed_week;
                    directive_info.create = false;
                }
            }
            roundcontext.scriptcontext.directives.AddItem(new ModifiedDirective() {
                name = name,
                completed_round = !!completed_round,
                completed_week = !!completed_week,
                value = 0x00,
                create = false
            });
        }

        public static double UnlockdirectiveGet(RoundContext roundcontext, string name) {
            double value;
            FunkinSave.ReadUnlockDirective(name, out value);
            return value;
        }

        public static bool UnlockdirectiveHas(RoundContext roundcontext, string name) {
            return FunkinSave.ContainsUnlockDirective(name);
        }

        public static SongPlayer GetSongplayer(RoundContext roundcontext) {
            return roundcontext.songplayer;
        }

        public static Layout GetStageLayout(RoundContext roundcontext) {
            return roundcontext.layout == roundcontext.ui_layout ? null : roundcontext.layout;
        }

        public static Layout UIGetLayout(RoundContext roundcontext) {
            return roundcontext.ui_layout;
        }

        public static Camera UIGetCamera(RoundContext roundcontext) {
            return roundcontext.ui_camera;
        }

        public static void SetHalt(RoundContext roundcontext, bool halt) {
            roundcontext.scriptcontext.halt_flag = halt;
        }

        public static Character GetGirlfriend(RoundContext roundcontext) {
            return roundcontext.girlfriend;
        }

        public static int GetCharacterCount(RoundContext roundcontext) {
            return roundcontext.players_size;
        }

        public static Character GetCharacter(RoundContext roundcontext, int index) {
            if (index < 0 || index >= roundcontext.players_size) return null;
            return roundcontext.players[index].character;
        }

        public static MessageBox GetMessagebox(RoundContext roundcontext) {
            return roundcontext.messagebox;
        }

        public static int UiGetStrumsCount(RoundContext roundcontext) {
            return roundcontext.initparams.layout_strums_size;
        }

        public static TextSprite UIGetTrackinfo(RoundContext roundcontext) {
            return roundcontext.trackinfo;
        }

        public static void UISetVisibility(RoundContext roundcontext, bool visible) {
            roundcontext.layout.SetGroupVisibility(Week.ROUND_UI_GROUP_NAME, visible);
        }

        public static void GetCurrentChartInfo(RoundContext roundcontext, out float bpm, out double speed) {
            bpm = roundcontext.settings.bpm;
            speed = roundcontext.settings.speed;
        }

        public static void GetCurrentTrackInfo(RoundContext roundcontext, out string name, out string difficult, out int index) {
            name = roundcontext.initparams.gameplaymanifest.tracks[roundcontext.track_index].name;
            difficult = roundcontext.track_difficult;
            index = roundcontext.track_index;
        }

        public static void ChangeCharacterCameraName(RoundContext roundcontext, bool opponent_or_player, string new_name) {
            Settings settings = roundcontext.settings;

            if (opponent_or_player) {
                //if (settings.camera_name_opponent != Week.ROUND_CAMERA_OPONNENT) free(settings.camera_name_opponent);
                settings.camera_name_opponent = new_name == null ? Week.ROUND_CAMERA_OPONNENT : new_name;
            } else {
                //if (settings.camera_name_player != Week.ROUND_CAMERA_PLAYER) free(settings.camera_name_player);
                settings.camera_name_player = new_name == null ? Week.ROUND_CAMERA_PLAYER : new_name;
            }
        }

        public static void EnableCreditsOnCompleted(RoundContext roundcontext) {
            roundcontext.settings.show_credits = true;
        }

        public static void End(RoundContext roundcontext, bool round_or_week, bool loose_or_win) {
            roundcontext.scriptcontext.force_end_flag = true;
            roundcontext.scriptcontext.force_end_round_or_week = round_or_week;
            roundcontext.scriptcontext.force_end_loose_or_win = loose_or_win;
        }

        public static Dialogue GetDialogue(RoundContext roudcontext) {
            return roudcontext.dialogue;
        }

        public static void SeUIShader(RoundContext roundcontext, PSShader psshader) {
            Layout layout = roundcontext.layout ?? roundcontext.ui_layout;
            layout.SetGroupShader(Week.ROUND_UI_GROUP_NAME, psshader);
        }

        public static Conductor GetConductor(RoundContext roundcontext, int character_index) {
            if (character_index < 0 || character_index >= roundcontext.players_size) return null;
            return roundcontext.players[character_index].conductor;
        }

        public static HealthWatcher GetHealthwatcher(RoundContext roundcontext) {
            return roundcontext.healthwatcher;
        }

        public static MissNoteFX GetMissnotefx(RoundContext roundcontext) {
            return roundcontext.missnotefx;
        }

        public static PlayerStats GetPlayerstats(RoundContext roundcontext, int character_index) {
            if (character_index < 0 || character_index >= roundcontext.players_size) return null;
            return roundcontext.players[character_index].playerstats;
        }

        public static void RebuildUI(RoundContext roundcontext) {
            Week.InitUICosmetics(roundcontext);
        }

        public static Countdown UIGetcountdown(RoundContext roundcontext) {
            return roundcontext.countdown;
        }

        public static HealthBar UIGetHealthbar(RoundContext roundcontext) {
            return roundcontext.healthbar;
        }

        public static RankingCounter GetRankingcounter(RoundContext roundcontext) {
            return roundcontext.rankingcounter;
        }

        public static RoundStats UIGetroundstats(RoundContext roundcontext) {
            return roundcontext.roundstats;
        }

        public static SongProgressbar UIGetSongprogressbar(RoundContext roundcontext) {
            return roundcontext.songprogressbar;
        }

        public static RankingCounter UIGetRankingCounter(RoundContext roundcontext) {
            return roundcontext.rankingcounter;
        }

        public static StreakCounter UIGetStreakcounter(RoundContext roundcontext) {
            return roundcontext.streakcounter;
        }

        public static Strums UIGetStrums(RoundContext roundcontext, int strums_id) {
            InitParams initparams = roundcontext.initparams;
            GameplayManifest gameplaymanifest = initparams.gameplaymanifest;
            int track_index = roundcontext.track_index;

            GameplayManifestPlayer[] players = gameplaymanifest.@default.players;
            int players_size = gameplaymanifest.@default.players_size;
            if (gameplaymanifest.tracks[track_index].has_players) {
                players = gameplaymanifest.tracks[track_index].players;
                players_size = gameplaymanifest.tracks[track_index].players_size;
            }

            for (int i = 0 ; i < players_size ; i++) {
                // obtain the position in the UI layout
                int layout_strums_id = players[i].layout_strums_id < 0 ? i : players[i].layout_strums_id;

                if (layout_strums_id >= initparams.layout_strums_size) layout_strums_id = -1;
                if (layout_strums_id < 0 || roundcontext.players[i].type == CharacterType.ACTOR) continue;

                if (layout_strums_id == strums_id) return roundcontext.players[i].strums;
            }

            // unable to guess the correct player's strums
            return null;
        }


        internal static string InternalConcatSuffix(string name, int number_suffix) {
            string digits = number_suffix.ToString();
            return StringUtils.Concat(name, digits);
        }

        private static object InternalReadValuePrefix(Layout layout, string name, int number_suffix, AttachedValueType type, object default_value) {
            string temp = Week.InternalConcatSuffix(name, number_suffix);
            object value = layout.GetAttachedValue(temp, type, default_value);
            //free(temp);
            return value;
        }

        private static LayoutPlaceholder InternalReadPlaceholder(Layout layout, string prefix_name, int number_suffix) {
            string placeholder_name = Week.InternalConcatSuffix(prefix_name, number_suffix);
            LayoutPlaceholder placeholder = layout.GetPlaceholder(placeholder_name);
            if (placeholder == null) Console.Error.WriteLine("[ERROR] Missing layout placeholder: " + placeholder_name);
            //free(placeholder_name);
            return placeholder;
        }

        private static void InternalPlaceCharacter(Layout layout, Character character, LayoutCharacter layout_character) {

            /*float viewport_width, viewport_height;
            layout.GetViewportSize(out viewport_width, out viewport_height);
            character.SetLayoutResolution(1280, 720);
            character.SetLayoutResolution(initparams.ui_layout_width, initparams.ui_layout_height);
            character.SetLayoutResolution(viewport_width, viewport_height);*/

            character.SetScale(layout_character.scale);

            character.SetDrawLocation(layout_character.x, layout_character.y);
            character.SetZIndex(layout_character.z);

            character.UpdateReferenceSize(
                layout_character.reference_width, layout_character.reference_height
            );

            bool enable = layout_character.reference_width >= 0 || layout_character.reference_height >= 0;
            character.EnableReferenceSize(enable);

            character.SetDrawAlign(
                layout_character.align_vertical, layout_character.align_horizontal
            );

            layout.SetPlaceholderDrawableById(
                layout_character.placeholder_id, character.GetDrawable()
            );

            character.StateToggle(null);
        }

        private static GameplayManifest InternalLoadGameplayManifest(string src) {
            GameplayManifest gameplaymanifest = GameplayManifest.Init(src);

            if (gameplaymanifest == null) {
                Console.Error.WriteLine("[ERROR] week_main() missing file " + src);
                return null;
            }

            if (gameplaymanifest.tracks_size < 1) {
                gameplaymanifest.Destroy();
                Console.Error.WriteLine("[ERROR] week_main() no tracks defined, goodbye.File: " + src);
                return null;
            }

            return gameplaymanifest;
        }

        private static bool InternalCameraFocus(Layout layout, string character_prefix, int index) {
            if (layout == null) return true;
            Camera camera = layout.GetCameraHelper();

            if (index < 0) {
                return camera.FromLayout(layout, character_prefix);
            }

            string name = Week.InternalConcatSuffix(character_prefix, index);
            bool res = camera.FromLayout(layout, name);
            //free(name);
            return res;
        }

        private static void InternalCameraFocusGuess(RoundContext roundcontext, bool opponent_or_player) {
            if (roundcontext.layout == null) return;

            Settings settings = roundcontext.settings;

            // check if there absolute camera present in the layout
            string target = opponent_or_player ? settings.camera_name_opponent : settings.camera_name_player;
            Camera camera = roundcontext.layout.GetCameraHelper();
            if (camera.FromLayout(roundcontext.layout, target)) return;

            // guess from the character type and later invoke the camera named like "camera_player012345"
            int index = -1;
            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                if (opponent_or_player && roundcontext.players[i].type == CharacterType.BOT) {
                    index = i;
                    break;
                }
                if (!opponent_or_player && roundcontext.players[i].type == CharacterType.PLAYER) {
                    index = i;
                    break;
                }
            }

            if (index < 0) return;

            Week.InternalCameraFocus(roundcontext.layout, Week.ROUND_CAMERA_PLAYER, index);
        }

        private static void InternalResetPlayersAndGirlfriend(RoundContext roundcontext) {
            double speed = roundcontext.settings.original_speed;
            ChangeScrollDirection(roundcontext);
            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                if (roundcontext.players[i].strums != null) {
                    roundcontext.players[i].strums.Reset(speed, null);
                    if (roundcontext.players[i].character != null)
                        roundcontext.players[i].character.Reset();
                    if (roundcontext.players[i].conductor != null)
                        roundcontext.players[i].conductor.PollReset();
                    if (roundcontext.players[i].ddrkeymon != null)
                        roundcontext.players[i].ddrkeymon.Stop();
                    if (roundcontext.players[i].playerstats != null) {
                        roundcontext.players[i].playerstats.Reset();
                        roundcontext.players[i].playerstats.EnableHealthRecover(
                            roundcontext.players[i].can_recover
                        );
                    }
                }
            }

            if (roundcontext.girlfriend != null) roundcontext.girlfriend.Reset();
        }

        private static void InternalPickCountersValuesFromLayout(RoundContext roundcontext) {
            UIParams ui = roundcontext.initparams.ui;
            Layout layout = roundcontext.layout ?? roundcontext.ui_layout;

            // streakcounter values
            ui.streakcounter_comboheight = layout.GetAttachedValueAsFloat(
                "ui_streakcounter_comboHeight", ui.streakcounter_comboheight
            );
            ui.streakcounter_numbergap = layout.GetAttachedValueAsFloat(
                "ui_streakcounter_numberGap", ui.streakcounter_numbergap
            );
            ui.streakcounter_delay = layout.GetAttachedValueAsFloat(
                "ui_streakcounter_delay", ui.streakcounter_delay
            );

            // ranking counter values
            ui.rankingcounter_percentonly = (bool)layout.GetAttachedValue(
                "ui_rankingcounter_accuracy_percent", AttachedValueType.BOOLEAN, ui.rankingcounter_percentonly
            );
        }

        private static LayoutPlaceholder InternalReadPlaceholderCounter(Layout layout, string name) {
            LayoutPlaceholder placeholder = layout.GetPlaceholder(name);
            if (placeholder == null) Console.Error.WriteLine($"[ERROR] week: missing layout '{name}' placeholder");
            return placeholder;
        }

        private static void InternalCheckScreenResolution(RoundContext roundcontext, bool force) {
            if (!force && roundcontext.resolution_changes == PVRContext.global_context.resolution_changes) return;
            roundcontext.resolution_changes = PVRContext.global_context.resolution_changes;

            int width = PVRContext.global_context.ScreenWidth;
            int height = PVRContext.global_context.ScreenHeight;
            InitParams initparams = roundcontext.initparams;

            roundcontext.screen_background.SetDrawSize(width, height);

            float scale_x = width / initparams.ui_layout_width;
            float scale_y = height / initparams.ui_layout_height;

            float scale = Math.Min(scale_x, scale_y);
            float translate_x = (width - initparams.ui_layout_width * scale) / 2f;
            float translate_y = (height - initparams.ui_layout_height * scale) / 2f;

            Week.ROUND_UI_MATRIX.Clear();
            Week.ROUND_UI_MATRIX.Translate(translate_x, translate_y);
            Week.ROUND_UI_MATRIX.Scale(scale, scale);

            Week.ROUND_UI_MATRIX.CopyTo(Week.ROUND_UI_MATRIX_CAMERA);
        }

    }
}
