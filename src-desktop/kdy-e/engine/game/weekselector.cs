using System;
using System.Text;
using Engine.Animation;
using Engine.Font;
using Engine.Game.Common;
using Engine.Game.Gameplay;
using Engine.Game.Helpers;
using Engine.Image;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;

namespace Engine.Game {

    public class WeekSelector {

        private const string SCORE = "WEEK SCORE: ";
        private const string TRACKS = "TRACKS";
        private const string TRACKS_MORE = "...";
        private const string COMPLETED = "WEEKS COMPLETED";
        private const string CHOOSEN = "WEEK CHOOSEN";
        private const string DIFFICULTY = "DIFFICULTY";
        private const string ALT_WARNING = "TRACKS WARNING";
        private const string SENSIBLE_CONTENT = "SENSIBLE CONTENT";
        private const string HELP_N_ALT = "USE ALTERNATIVE TRACKS";
        private const string HELP_W_ALT = "USE DEFAULT TRACKS";
        private const string HELP_GFBF_SWITCH = "SWITCH BOYFRIEND/GIRLFRIEND";
        private const string HELP_SELECT_WEEK = "SELECT ANOTHER WEEK";
        private const string HELP_BACK = "BACK";
        private const string HELP_START = "PLAY THIS WEEK";
        private const string GIRLFRIEND = "GIRLFRIEND";
        private const string BOYFRIEND = "BOYFRIEND";

        private const string ANIM_IDLE = "idle";
        private const string ANIM_CHOOSEN = "choosen";
        private const float CHARACTER_Z_INDEX = 3;

        internal const int BUTTON_DELAY = 200;
        internal const string BUTTON_X = "x";
        internal const string BUTTON_B = "b";
        internal const string BUTTON_LT_RT = "lt_rt";
        internal const string BUTTON_START = "start";

        private const string ARROW_NAME = "arrow_L push";
        private const string ARROW_NAME2 = "arrow_L";
        internal const string LOCKED = "locked";

        internal const string BUTTONS_MODEL = "/assets/common/image/week-selector/dreamcast_buttons.xml";
        internal const string UI_ICONS_MODEL = "/assets/common/image/week-selector/week_selector_UI.xml";
        internal const string UI_ANIMS = "/assets/common/image/week-selector/ui_anims.xml";

        internal const float ARROW_DISABLED_ALPHA = 0.1f;
        internal const float PREVIEW_DISABLED_ALPHA = 0.7f;
        internal const string ARROW_SPRITE_NAME = "arrow_L push";

        private const GamepadButtons BUTTONS_OK = GamepadButtons.X | GamepadButtons.A;
        private const string LAYOUT = "/assets/common/image/week-selector/layout.xml";
        private const string LAYOUT_DREAMCAST = "/assets/common/image/week-selector/layout~dreamcast.xml";


        private enum SND {
            NONE,
            CANCEL,
            SCROLL,
            ASTERIK
        }

        private class UI {
            public Gamepad maple_pads;

            public WeekSelectorHelpText helptext_alternate;
            public WeekSelectorHelpText helptext_back;
            public WeekSelectorWeekMSG weekmsg_sensible;
            public WeekSelectorWeekMSG weekmsg_alternate;
            public WeekSelectorDifficult weekdifficult;
            public WeekSelectorWeekList weeklist;
            public WeekSelectorWeekTitle weektitle;
            public WeekSelectorHelpText helptext_bfgf;
            public WeekSelectorHelpText helptext_start;
            public TextSprite weeks_choosen_index;
            public Sprite week_background_color;

            public WeekSelectorMdlSelect mdl_boyfriend;
            public WeekSelectorMdlSelect mdl_girlfriend;
            public TextSprite tracks_list;
            public TextSprite score;
            public TextSprite description;

            public Layout layout;
            public StringBuilder stringbuilder;
        }

        private class STATE {
            public WeekInfo weekinfo;

            public bool update_ui;
            public bool update_extra_ui;
            public int page_selected_ui;
            public SND play_sound;

            public bool choosing_boyfriend;

            public bool week_unlocked;

            public bool has_choosen_alternate;
            public string difficult;
            public bool can_start;

            public bool quit;
            public bool back_to_main_menu;

            public uint default_background_color;
            public Layout custom_layout;
        }


        private static void ShowWeekInfo(UI ui, WeekInfo weekinfo, long score) {
            float max_draw_width, max_draw_height;
            float font_size = ui.tracks_list.GetFontSize();
            ui.tracks_list.GetMaxDrawSize(out max_draw_width, out max_draw_height);

            int songs_count = weekinfo.songs_count;
            int max_lines = (int)(max_draw_height / font_size) - 1;
            bool has_overflow;
            if (songs_count > max_lines) {
                has_overflow = true;
                songs_count = max_lines;
            } else {
                has_overflow = false;
            }

            StringBuilder list = ui.stringbuilder;
            list.Clear();

            for (int i = 0 ; i < weekinfo.songs_count ; i++) {
                if (weekinfo.songs[i].freeplay_only) continue;
                list.AddKDY("\n");
                list.AddKDY(weekinfo.songs[i].name);
            }
            if (has_overflow) {
                list.AddKDY("\n");
                list.AddKDY(WeekSelector.TRACKS_MORE);
            }

            // set the strings
            ui.score.SetTextFormated("$s$l", WeekSelector.SCORE, score);
            ui.description.SetTextIntern(true, weekinfo.description);
            ui.tracks_list.SetTextIntern(true, list.InternKDY());
        }

        private static void ChangePage(UI ui, int page) {
            // hide everything first
            ui.layout.SetGroupVisibility("ui_week_selector", page == 0);
            ui.layout.SetGroupVisibility("ui_difficult_selector", page == 1);
            ui.layout.SetGroupVisibility("ui_character_selector", page == 2);

            ui.helptext_alternate.SetVisible(false);
            ui.helptext_bfgf.SetVisible(false);
            ui.helptext_back.SetVisible(page > 0);
            ui.helptext_start.SetVisible(false);
        }

        private static void TriggerDifficultChange(UI ui, STATE state, bool empty) {
            if (state.custom_layout == null) return;

            state.custom_layout.TriggerAny("change-difficult");

            if (empty) return;

            string default_difficult = ui.weekdifficult.GetSelected();
            if (default_difficult != null) {
                string difficult_action = StringUtils.Concat("selected-difficult-", default_difficult);
                state.custom_layout.TriggerAny(difficult_action);
                //free(difficult_action);
            }
        }

        private static void TriggerAlternateChange(STATE state) {
            if (state.custom_layout == null) return;

            string alternate_action;
            if (state.has_choosen_alternate) alternate_action = "selected-no-alternate";
            else alternate_action = "selected-alternate";

            state.custom_layout.TriggerAny(alternate_action);
        }

        private static void LoadCustomWeekBackground(STATE state) {
            if (state.custom_layout != null) {
                state.custom_layout.Destroy();
                state.custom_layout = null;
            }

            if (String.IsNullOrEmpty(state.weekinfo.custom_selector_layout)) return;

            string path = WeekEnumerator.GetAsset(state.weekinfo, state.weekinfo.custom_selector_layout);
            state.custom_layout = Layout.Init(path);
            //free(path);
            if (state.custom_layout != null) {
                state.custom_layout.TriggerAny("show-principal");
                if (!state.week_unlocked) state.custom_layout.TriggerAny("week-locked");
            }
        }

        private static void SetText(Layout layout, string name, string format, object text) {
            TextSprite textsprite = layout.GetTextsprite(name);
            if (textsprite == null) return;
            if (format != null)
                textsprite.SetTextFormated(format, text);
            else
                textsprite.SetTextIntern(true, (string)text);
        }


        public static int Main() {
            Layout layout = Layout.Init(PVRContext.global_context.IsWidescreen() ? LAYOUT : LAYOUT_DREAMCAST);
            AnimList animlist_ui = AnimList.Init(WeekSelector.UI_ANIMS);
            ModelHolder modelholder_ui = ModelHolder.Init(WeekSelector.UI_ICONS_MODEL);
            ModelHolder modelholder_buttons_ui = ModelHolder.Init(WeekSelector.BUTTONS_MODEL);

            TexturePool texpool = new TexturePool(128 * 1024 * 1024);// 128MiB

            SoundPlayer sound_confirm = SoundPlayer.Init("/assets/common/sound/confirmMenu.ogg");
            SoundPlayer sound_scroll = SoundPlayer.Init("/assets/common/sound/scrollMenu.ogg");
            SoundPlayer sound_cancel = SoundPlayer.Init("/assets/common/sound/cancelMenu.ogg");
            SoundPlayer sound_asterik = SoundPlayer.Init("/assets/common/sound/asterikMenu.ogg");

            BeatWatcher.GlobalSetTimestampFromKosTimer();

            // store all ui elements here (makes the code more readable)
            UI ui = new UI() {
                week_background_color = layout.GetSprite("week_background_color"),

                // week score&description
                score = layout.GetTextsprite("week_score"),
                description = layout.GetTextsprite("week_description"),

                // week selector ui elements
                tracks_list = layout.GetTextsprite("ui_tracks_list"),
                weeks_choosen_index = layout.GetTextsprite("ui_weeks_choosen_index"),
                layout = layout,

                // selected week warning messages
                weekmsg_sensible = new WeekSelectorWeekMSG(
                    layout, WeekSelector.SENSIBLE_CONTENT,
                    "warning_sensible_title", "warning_sensible_message"
                ),
                weekmsg_alternate = new WeekSelectorWeekMSG(
                    layout, WeekSelector.ALT_WARNING,
                    "warning_alternate_title", "warning_alternate_message"
                ),

                weeklist = new WeekSelectorWeekList(animlist_ui, modelholder_ui, layout, texpool),
                weektitle = new WeekSelectorWeekTitle(layout),

                mdl_boyfriend = new WeekSelectorMdlSelect(
                    animlist_ui, modelholder_ui, layout, texpool, true
                ),
                mdl_girlfriend = new WeekSelectorMdlSelect(
                    animlist_ui, modelholder_ui, layout, texpool, false
                ),

                weekdifficult = new WeekSelectorDifficult(animlist_ui, modelholder_ui, layout),

                helptext_alternate = new WeekSelectorHelpText(
                    modelholder_buttons_ui, layout, 3, false, WeekSelector.BUTTON_X,
                    WeekSelector.HELP_N_ALT, WeekSelector.HELP_W_ALT
                ),
                helptext_bfgf = new WeekSelectorHelpText(
                    modelholder_buttons_ui, layout, 2, false, WeekSelector.BUTTON_LT_RT,
                    WeekSelector.HELP_GFBF_SWITCH, null
                ),
                helptext_back = new WeekSelectorHelpText(
                    modelholder_buttons_ui, layout, 1, false, WeekSelector.BUTTON_B,
                    WeekSelector.HELP_BACK, WeekSelector.HELP_SELECT_WEEK
                ),
                helptext_start = new WeekSelectorHelpText(
                    modelholder_buttons_ui, layout, 1, true, WeekSelector.BUTTON_START,
                    WeekSelector.HELP_START, null
                ),

                maple_pads = new Gamepad(-1),
                stringbuilder = new StringBuilder(64)
            };

            // dispose unused resources
            modelholder_ui.Destroy();
            modelholder_buttons_ui.Destroy();
            animlist_ui.Destroy();

            // funkin stuff
            int weeks_completed = FunkinSave.GetCompletedCount();
            string last_difficult_played = FunkinSave.GetLastPlayedDifficult();

            // setup text boxes
            WeekSelector.SetText(layout, "ui_tracks_title", null, WeekSelector.TRACKS);
            WeekSelector.SetText(layout, "ui_weeks_completed_title", null, WeekSelector.COMPLETED);
            WeekSelector.SetText(layout, "ui_weeks_completed_count", "$i", weeks_completed);
            WeekSelector.SetText(layout, "ui_weeks_choosen_title", null, WeekSelector.CHOOSEN);

            // attach help texts to the layout
            int ui_group = layout.ExternalCreateGroup(null, 0);
            layout.ExternalVertexCreateEntries(4);
            layout.ExternalVertexSetEntry(
                0, PVRContextVertex.DRAWABLE, ui.helptext_alternate.GetDrawable(), ui_group
            );
            layout.ExternalVertexSetEntry(
                1, PVRContextVertex.DRAWABLE, ui.helptext_bfgf.GetDrawable(), ui_group
            );
            layout.ExternalVertexSetEntry(
                2, PVRContextVertex.DRAWABLE, ui.helptext_back.GetDrawable(), ui_group
            );
            layout.ExternalVertexSetEntry(
                3, PVRContextVertex.DRAWABLE, ui.helptext_start.GetDrawable(), ui_group
            );

            ui.maple_pads.SetButtonsDelay(WeekSelector.BUTTON_DELAY);

            STATE state = new STATE() {
                weekinfo = ui.weeklist.GetSelected(),
                difficult = last_difficult_played,

                week_unlocked = false,
                play_sound = SND.NONE,

                default_background_color = 0,

                can_start = false,
                choosing_boyfriend = true,
                has_choosen_alternate = false,

                update_extra_ui = true,
                page_selected_ui = 0,
                update_ui = true,
                quit = false,
                back_to_main_menu = false,

                custom_layout = null
            };

            if (ui.week_background_color != null)
                state.default_background_color = ui.week_background_color.GetVertexColorRGB8();

            WeekSelector.ChangePage(ui, state.page_selected_ui);
            state.week_unlocked = FunkinSave.ContainsUnlockDirective(state.weekinfo.unlock_directive);
            WeekSelector.LoadCustomWeekBackground(state);

            while (!state.quit) {
                float elapsed = PVRContext.global_context.WaitReady();
                int page_selected_ui = state.page_selected_ui;

                BeatWatcher.GlobalSetTimestampFromKosTimer();

                switch (page_selected_ui) {
                    case 0:
                        WeekSelector.Page0(ui, state);
                        break;
                    case 1:
                        WeekSelector.Page1(ui, state);
                        break;
                    case 2:
                        WeekSelector.Page2(ui, state);
                        break;
                }

                if (state.play_sound != SND.NONE) {
                    if (sound_cancel != null) sound_cancel.Stop();
                    if (sound_scroll != null) sound_scroll.Stop();
                    if (sound_asterik != null) sound_asterik.Stop();

                    SoundPlayer sound = null;
                    switch (state.play_sound) {
                        case SND.CANCEL:
                            sound = sound_cancel;
                            break;
                        case SND.SCROLL:
                            sound = sound_scroll;
                            break;
                        case SND.ASTERIK:
                            sound = sound_asterik;
                            break;
                    }

                    state.play_sound = SND.NONE;
                    if (sound != null) sound.Replay();
                }

                PVRContext.global_context.Reset();
                layout.Animate(elapsed);
                layout.Draw(PVRContext.global_context);

                if (state.custom_layout != null) {
                    state.custom_layout.Animate(elapsed);
                    state.custom_layout.Draw(PVRContext.global_context);
                }

                state.update_ui = page_selected_ui != state.page_selected_ui;
                if (state.update_ui) WeekSelector.ChangePage(ui, state.page_selected_ui);
            }

            bool no_anim_out = false;

            if (state.back_to_main_menu) {
                if (sound_cancel != null) sound_cancel.Replay();
                layout.TriggerAny("back-to-main-menu");
            } else {
                if (GameMain.background_menu_music != null) GameMain.background_menu_music.Stop();
                WeekSelector.ChangePage(ui, 0);
                if (sound_confirm != null) sound_confirm.Replay();

                if (state.custom_layout != null)
                    no_anim_out = state.custom_layout.TriggerAny("week-choosen") > 0;

                // if the custom layout does not contains the "week-choosen" action
                // trigger in the main layout
                if (!no_anim_out) layout.TriggerAny("week-choosen");

                layout.SetGroupVisibility("ui_game_progress", false);
                ui.weektitle.MoveDifficult(ui.weekdifficult);
                ui.mdl_boyfriend.ToggleChoosen();
                ui.mdl_girlfriend.ToggleChoosen();
                ui.weeklist.ToggleChoosen();
            }

            double total_elapsed = 0;
            while (total_elapsed < 2400.0) {
                float elapsed = PVRContext.global_context.WaitReady();
                total_elapsed += elapsed;

                BeatWatcher.GlobalSetTimestampFromKosTimer();

                PVRContext.global_context.Reset();
                layout.Animate(elapsed);
                layout.Draw(PVRContext.global_context);

                if (state.custom_layout != null) {
                    state.custom_layout.Animate(elapsed);
                    state.custom_layout.Draw(PVRContext.global_context);
                }

                if (state.back_to_main_menu && layout.AnimationIsCompleted("transition_effect") > 0)
                    break;
            }

            // gameplay parameters
            WeekInfo gameplay_weekinfo = state.weekinfo;
            string gameplay_difficult = ui.weekdifficult.GetSelected();
            bool gameplay_alternative_tracks = state.has_choosen_alternate;
            string gameplay_model_boyfriend = ui.mdl_boyfriend.GetManifest();
            string gameplay_model_girlfriend = ui.mdl_girlfriend.GetManifest();

            // dispose everything used
            ui.weeklist.Destroy();
            ui.weekdifficult.Destroy();
            ui.weektitle.Destroy();

            ui.mdl_boyfriend.Destroy();
            ui.mdl_girlfriend.Destroy();

            ui.maple_pads.Destroy();

            ui.helptext_alternate.Destroy();
            ui.helptext_bfgf.Destroy();
            ui.helptext_back.Destroy();
            ui.helptext_start.Destroy();

            ui.weekmsg_sensible.Destroy();
            ui.weekmsg_alternate.Destroy();

            if (sound_confirm != null) sound_confirm.Destroy();
            if (sound_asterik != null) sound_asterik.Destroy();
            if (sound_scroll != null) sound_scroll.Destroy();
            if (sound_cancel != null) sound_cancel.Destroy();

            //ui.stringbuilder.Destroy();

            if (state.custom_layout != null) state.custom_layout.Destroy();

            layout.Destroy();
            texpool.Destroy();

            if (state.back_to_main_menu) return 0;

            if (GameMain.background_menu_music != null) {
                GameMain.background_menu_music.Destroy();
                GameMain.background_menu_music = null;
            }

            FunkinSave.SetLastPlayed(gameplay_weekinfo.name, gameplay_difficult);

            // (C# only) before run check if necessary preload files
            int preload_id = PreloadCache.AddFileList(
                WeekEnumerator.GetAsset(gameplay_weekinfo, PreloadCache.PRELOAD_FILENAME)
            );

            int week_result = Week.Main(
                gameplay_weekinfo,
                gameplay_alternative_tracks,
                gameplay_difficult,
                gameplay_model_boyfriend,
                gameplay_model_girlfriend,
                null,
                -1
            );

            // forget all preloaded files
            PreloadCache.ClearById(preload_id);

            GameMain.background_menu_music = SoundPlayer.Init(Funkin.BACKGROUND_MUSIC);
            if (GameMain.background_menu_music != null) {
                GameMain.background_menu_music.LoopEnable(true);
                GameMain.background_menu_music.Replay();
            }

            return week_result;
        }


        private static void Page0(UI ui, STATE state) {
            int seek_offset = 0;

            if (state.update_ui) ui.helptext_start.SetVisible(state.can_start);

            if (state.update_extra_ui) {
                long score = FunkinSave.GetWeekScore(state.weekinfo.name, state.difficult);
                int index = ui.weeklist.GetSelectedIndex() + 1;
                WeekSelector.ShowWeekInfo(ui, state.weekinfo, score);
                ui.weeks_choosen_index.SetTextFormated("$i/$i", index, Funkin.weeks_array.size);

                if (ui.week_background_color != null) {
                    uint background_color;
                    if (state.weekinfo.selector_background_color_override)
                        background_color = state.weekinfo.selector_background_color;
                    else
                        background_color = state.default_background_color;
                    ui.week_background_color.SetVertexColorRGB8(background_color);
                }

                state.update_extra_ui = false;
            }

            GamepadButtons buttons = ui.maple_pads.HasPressedDelayed(
                GamepadButtons.AD_UP | GamepadButtons.AD_DOWN | GamepadButtons.X | GamepadButtons.A | GamepadButtons.START | GamepadButtons.B | GamepadButtons.BACK
            );

            if ((buttons & GamepadButtons.AD_UP) != GamepadButtons.NOTHING) {
                seek_offset = -1;
            } else if ((buttons & GamepadButtons.AD_DOWN) != GamepadButtons.NOTHING) {
                seek_offset = 1;
            } else if ((buttons & WeekSelector.BUTTONS_OK) != GamepadButtons.NOTHING) {
                if (state.week_unlocked) {
                    state.play_sound = SND.SCROLL;
                    state.page_selected_ui = 1;
                } else {
                    state.play_sound = SND.ASTERIK;
                }
            } else if ((buttons & GamepadButtons.START) != GamepadButtons.NOTHING) {
                state.quit = state.can_start;
                if (!state.quit && state.week_unlocked) {
                    state.play_sound = SND.SCROLL;
                    state.page_selected_ui = 1;
                }
            } else if ((buttons & (GamepadButtons.B | GamepadButtons.BACK)) != GamepadButtons.NOTHING) {
                state.quit = true;
                state.back_to_main_menu = true;
                return;
            }

            if (state.page_selected_ui == 1 && state.custom_layout != null) {
                state.custom_layout.TriggerAny("principal-hide");
                state.custom_layout.TriggerAny("difficult-selector-show");
            }

            if (seek_offset == 0) return;

            if (!ui.weeklist.Scroll(seek_offset)) {
                state.play_sound = SND.ASTERIK;
                return;
            }

            state.weekinfo = ui.weeklist.GetSelected();
            state.week_unlocked = FunkinSave.ContainsUnlockDirective(state.weekinfo.unlock_directive);
            state.play_sound = SND.SCROLL;
            state.can_start = false;
            state.update_extra_ui = true;
            state.has_choosen_alternate = false;

            ui.helptext_start.SetVisible(false);
            ui.mdl_boyfriend.SelectDefault();
            ui.mdl_girlfriend.SelectDefault();

            WeekSelector.LoadCustomWeekBackground(state);
        }

        private static void Page1(UI ui, STATE state) {
            int seek_offset = 0;

            if (state.update_ui) {
                WeekInfo weekinfo = state.weekinfo;
                bool has_alternate = !String.IsNullOrEmpty(weekinfo.warning_message);
                bool has_warnings = has_alternate && !String.IsNullOrEmpty(weekinfo.sensible_content_message);

                ui.helptext_bfgf.SetVisible(
                    !(weekinfo.disallow_custom_boyfriend && weekinfo.disallow_custom_girlfriend)
                );

                ui.weekdifficult.Load(weekinfo, state.difficult);
                ui.weektitle.Move(ui.weeklist, has_warnings);

                ui.helptext_alternate.SetVisible(has_alternate);
                ui.weekmsg_sensible.SetMessage(weekinfo.sensible_content_message);
                ui.weekmsg_alternate.SetMessage(weekinfo.warning_message);
                ui.weekmsg_alternate.Disabled(state.has_choosen_alternate);
                ui.weekdifficult.Relayout(has_warnings);

                ui.helptext_start.SetVisible(!has_warnings);
                WeekSelector.TriggerDifficultChange(ui, state, false);
                WeekSelector.TriggerAlternateChange(state);
            }

            GamepadButtons buttons = ui.maple_pads.HasPressedDelayed(
                GamepadButtons.AD_LEFT | GamepadButtons.AD_RIGHT | GamepadButtons.X | GamepadButtons.B |
                GamepadButtons.T_LR | GamepadButtons.A | GamepadButtons.START | GamepadButtons.BACK
            );

            if ((buttons & GamepadButtons.AD_LEFT) != GamepadButtons.NOTHING) {
                seek_offset = -1;
            } else if ((buttons & GamepadButtons.AD_RIGHT) != GamepadButtons.NOTHING) {
                seek_offset = 1;
            } else if ((buttons & GamepadButtons.X) != GamepadButtons.NOTHING) {
                if (!String.IsNullOrEmpty(state.weekinfo.warning_message)) {
                    state.has_choosen_alternate = !state.has_choosen_alternate;
                    ui.helptext_alternate.UseAlt(state.has_choosen_alternate);
                    ui.weekmsg_alternate.Disabled(state.has_choosen_alternate);
                    state.play_sound = SND.SCROLL;
                    WeekSelector.TriggerAlternateChange(state);
                }
            } else if ((buttons & (GamepadButtons.B | GamepadButtons.BACK)) != GamepadButtons.NOTHING) {
                state.page_selected_ui = 0;
                state.has_choosen_alternate = false;
                state.play_sound = SND.CANCEL;
            } else if ((buttons & GamepadButtons.T_LR) != GamepadButtons.NOTHING) {
                if (state.weekinfo.disallow_custom_boyfriend && state.weekinfo.disallow_custom_girlfriend) {
                    state.play_sound = SND.ASTERIK;
                } else {
                    state.choosing_boyfriend = (buttons & GamepadButtons.TRIGGER_LEFT) == GamepadButtons.TRIGGER_LEFT;
                    state.page_selected_ui = 2;
                    state.play_sound = SND.SCROLL;
                }
            } else if ((buttons & GamepadButtons.A) != GamepadButtons.NOTHING) {
                if (ui.weekdifficult.SelectedIsLocked()) {
                    state.play_sound = SND.ASTERIK;
                } else {
                    state.page_selected_ui = 0;
                    state.can_start = true;
                }
            } else if ((buttons & GamepadButtons.START) != GamepadButtons.NOTHING) {
                if (ui.weekdifficult.SelectedIsLocked())
                    state.play_sound = SND.ASTERIK;
                else
                    state.quit = true;
            }

            if (state.page_selected_ui == 0 && state.custom_layout != null) {
                state.custom_layout.TriggerAny("difficult-selector-hide");
                state.custom_layout.TriggerAny("principal-show");
            }

            if (seek_offset == 0) return;

            if (ui.weekdifficult.Scroll(seek_offset)) {
                state.play_sound = SND.SCROLL;
                state.can_start = true;
                state.difficult = ui.weekdifficult.GetSelected();

                WeekSelector.TriggerDifficultChange(ui, state, false);
                WeekSelector.ShowWeekInfo(
                    ui, state.weekinfo, FunkinSave.GetWeekScore(state.weekinfo.name, state.difficult)
                );
            } else {
                state.play_sound = SND.ASTERIK;
            }
        }

        private static void Page2(UI ui, STATE state) {
            int seek_offset = 0;
            int character_model_seek = 0;
            bool no_custom_boyfriend = state.weekinfo.disallow_custom_boyfriend;
            bool no_custom_girlfriend = state.weekinfo.disallow_custom_girlfriend;

            if (state.update_ui) {
                if (state.choosing_boyfriend && no_custom_boyfriend) state.choosing_boyfriend = false;
                if (!state.choosing_boyfriend && no_custom_girlfriend) state.choosing_boyfriend = true;
                ui.mdl_boyfriend.SelectDefault();
                ui.mdl_girlfriend.SelectDefault();
                ui.mdl_boyfriend.EnableArrows(state.choosing_boyfriend);
                ui.mdl_girlfriend.EnableArrows(!state.choosing_boyfriend);

                if (state.custom_layout != null)
                    state.custom_layout.TriggerAny("model-change-show");
            }

            GamepadButtons buttons = ui.maple_pads.HasPressedDelayed(
                GamepadButtons.T_LR | GamepadButtons.AD | GamepadButtons.B | GamepadButtons.A | GamepadButtons.START | GamepadButtons.BACK
            );

            if ((buttons & (GamepadButtons.TRIGGER_LEFT | GamepadButtons.AD_LEFT)) != GamepadButtons.NOTHING) {
                character_model_seek = -1;
            } else if ((buttons & (GamepadButtons.TRIGGER_RIGHT | GamepadButtons.AD_RIGHT)) != GamepadButtons.NOTHING) {
                character_model_seek = 1;
            } else if ((buttons & GamepadButtons.AD_UP) != GamepadButtons.NOTHING) {
                seek_offset = -1;
            } else if ((buttons & GamepadButtons.AD_DOWN) != GamepadButtons.NOTHING) {
                seek_offset = 1;
            } else if ((buttons & (GamepadButtons.B | GamepadButtons.BACK)) != GamepadButtons.NOTHING) {
                bool has_locked = false;
                if (ui.mdl_boyfriend.IsSelectedLocked()) {
                    has_locked = true;
                    ui.mdl_boyfriend.SelectDefault();
                }
                if (ui.mdl_girlfriend.IsSelectedLocked()) {
                    has_locked = true;
                    ui.mdl_girlfriend.SelectDefault();
                }
                state.page_selected_ui = 1;
                state.play_sound = has_locked ? SND.ASTERIK : SND.CANCEL;
            } else if ((buttons & GamepadButtons.A) != GamepadButtons.NOTHING) {
                bool has_locked = ui.mdl_boyfriend.IsSelectedLocked();
                if (ui.mdl_girlfriend.IsSelectedLocked()) has_locked = true;

                if (has_locked) {
                    state.play_sound = SND.ASTERIK;
                } else {
                    state.page_selected_ui = 0;
                    state.can_start = true;
                }
            } else if ((buttons & GamepadButtons.START) != GamepadButtons.NOTHING) {
                bool has_locked = ui.mdl_boyfriend.IsSelectedLocked();
                if (ui.mdl_girlfriend.IsSelectedLocked()) has_locked = true;

                if (has_locked)
                    state.play_sound = SND.ASTERIK;
                else
                    state.quit = true;
            }

            if (state.page_selected_ui != 2 && state.custom_layout != null)
                state.custom_layout.TriggerAny("model-change-hide");

            if (character_model_seek != 0) {
                bool old_choose_boyfriend = state.choosing_boyfriend;
                state.choosing_boyfriend = character_model_seek < 1;

                if (state.choosing_boyfriend != old_choose_boyfriend) {
                    if (no_custom_boyfriend || no_custom_girlfriend) {
                        state.play_sound = SND.ASTERIK;
                        state.choosing_boyfriend = old_choose_boyfriend;
                        return;
                    }

                    state.play_sound = SND.SCROLL;
                    ui.mdl_boyfriend.EnableArrows(state.choosing_boyfriend);
                    ui.mdl_girlfriend.EnableArrows(!state.choosing_boyfriend);
                }
            }

            if (seek_offset == 0) return;

            WeekSelectorMdlSelect mdlselect = state.choosing_boyfriend ? ui.mdl_boyfriend : ui.mdl_girlfriend;

            if (mdlselect.Scroll(seek_offset)) {
                state.play_sound = SND.SCROLL;
                state.can_start = true;
            } else {
                state.play_sound = SND.ASTERIK;
            }

        }

    }

}
