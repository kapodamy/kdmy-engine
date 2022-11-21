using System;
using Engine.Platform;
using Engine.Game;
using Engine.Image;
using Engine.Font;
using Engine.Sound;
using Engine.Game.Gameplay.Helpers;
using Engine.Utils;
using Engine.Game.Common;

namespace Engine.Game.Gameplay {

    public class WeekPause {

        private static readonly MenuManifest MENU = new MenuManifest() {
            parameters = new MenuManifest.Parameters() {
                suffix_selected = null,// unused
                suffix_idle = null,// unused
                suffix_choosen = null,// unused
                suffix_discarded = null,// unused
                suffix_rollback = null,// unused
                suffix_in = null,// unused
                suffix_out = null,// unused

                atlas = null,
                animlist = "/assets/common/anims/week-round.xml",

                anim_selected = "pause_item_selected",
                anim_idle = "pause_item_idle",
                anim_choosen = null,// unused
                anim_discarded = null,// unused
                anim_rollback = null,// unused
                anim_in = null,// unused
                anim_out = null,// unused

                anim_transition_in_delay = 0,// unused
                anim_transition_out_delay = 0,// unused

                font = null,
                font_glyph_suffix = "bold",
                font_color_by_difference = false,// unused
                font_size = 48f,
                font_color = 0xFFFFFF,
                font_border_color = 0x00,// unused
                font_border_size = Single.NaN,// unused

                is_sparse = false,// unused
                is_vertical = true,
                is_per_page = false,
                static_index = 1,

                items_align = Align.START,
                items_gap = 48f,
                items_dimmen = 0f,// unused
                texture_scale = Single.NaN,// unused
                enable_horizontal_text_correction = false// unused
            },
            items = new MenuManifest.Item[] {
       new MenuManifest.Item() {
            name= null,
            text= "RESUME",// unused
            placement= { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
            anim_selected= null,// unused
            anim_choosen= null,// unused
            anim_discarded= null,// unused
            anim_idle= null,// unused
            anim_rollback= null,// unused
            anim_in= null,// unused
            anim_out= null,// unused
            hidden= false
        },
       new MenuManifest.Item() {
            name= null,
            text= "WEEK MENU",// unused
            placement= { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
            anim_selected= null,// unused
            anim_choosen= null,// unused
            anim_discarded= null,// unused
            anim_idle= null,// unused
            anim_rollback= null,// unused
            anim_in= null,// unused
            anim_out= null,// unused
            hidden= true
        },
        new MenuManifest.Item() {
            name= null,
            text= "RESTART SONG",// unused
            placement= { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
            anim_selected= null,// unused
            anim_choosen= null,// unused
            anim_discarded= null,// unused
            anim_idle= null,// unused
            anim_rollback= null,// unused
            anim_in= null,// unused
            anim_out= null,// unused
            hidden= false
        },
       new MenuManifest.Item()  {
            name= null,
            text= "EXIT TO WEEK SELECTOR",// unused
            placement= { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
            anim_selected= null,// unused
            anim_choosen= null,// unused
            anim_discarded= null,// unused
            anim_idle= null,// unused
            anim_rollback= null,// unused
            anim_in= null,// unused
            anim_out= null,// unused
            hidden= false
        },
       new MenuManifest.Item() {
            name= null,
            text= "EXIT TO MAIN MENU",// unused
            placement= { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
            anim_selected= null,// unused
            anim_choosen= null,// unused
            anim_discarded= null,// unused
            anim_idle= null,// unused
            anim_rollback= null,// unused
            anim_in= null,// unused
            anim_out= null,// unused
            hidden= false
        }
    },
            items_size = 5
        };
        public const string LAYOUT_WIDESCREEN = "/assets/common/image/week-round/pause.xml";
        public const string LAYOUT_DREAMCAST = "/assets/common/image/week-round/pause~dreamcast.xml";
        public const string NOCONTROLLER = "/assets/common/image/week-round/no_controller.png";
        public const int ANTIBOUNCE = 400;

        // messagebox strings
        public const string MSGCONTROLLER = "The controller $i was discconected, \n" +
            "reconnect it or press START on an unused \n" +
            "controller to replace it.";
        public const string MSGMENU = "The week progress will be lost, ¿return\n to the main menu?";
        public const string MSGWEEKSELECTOR = "The week progress will be lost, ¿return\n to the week selector?";


        private Menu menu;
        private MessageBox messagebox;
        private Sprite sprite_nocontroller;
        private Layout layout;
        private LayoutPlaceholder menu_placeholder;
        private Menu menu_external;
        private SoundPlayer background_menu_music;


        public WeekPause() {
            string src = PVRContext.global_context.IsWidescreen() ? WeekPause.LAYOUT_WIDESCREEN : WeekPause.LAYOUT_DREAMCAST;
            Layout layout = Layout.Init(src);
            if (layout == null) throw new Exception("can not load: " + src);

            LayoutPlaceholder menu_placeholder = layout.GetPlaceholder("menu");
            if (menu_placeholder == null) throw new Exception("missing 'menu' placeholder in: " + src);

            WeekPause.MENU.parameters.font = (string)layout.GetAttachedValue(
                "menu_font_path", AttachedValueType.STRING, null
            );
            WeekPause.MENU.parameters.font_size = layout.GetAttachedValueAsFloat(
                "menu_font_size", 24f
            );
            WeekPause.MENU.parameters.items_gap = WeekPause.MENU.parameters.font_size;

            Menu menu = new Menu(
                WeekPause.MENU,
                menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
                menu_placeholder.width, menu_placeholder.height
            );

            MessageBox messagebox = new MessageBox();
            Sprite sprite_nocontroller = Sprite.Init(Texture.Init(WeekPause.NOCONTROLLER));

            this.menu = menu;
            this.messagebox = messagebox;
            this.sprite_nocontroller = sprite_nocontroller;
            this.layout = layout;
            this.menu_placeholder = menu_placeholder;
            this.menu_external = null;
            this.background_menu_music = null;
        }

        public void Destroy() {
            this.layout.Destroy();
            this.menu.Destroy();
            this.messagebox.Destroy();
            this.sprite_nocontroller.DestroyFull();
            if (this.menu_external != null) this.menu_external.Destroy();
            if (this.background_menu_music != null) this.background_menu_music.Destroy();
            //free(this);
        }



        public void ExternalSetText(int index, string text) {
            if (this.menu_external != null) this.menu_external.SetItemText(index, text);
        }

        public void ExternalSetVisible(int index, bool visible) {
            if (this.menu_external != null) this.menu_external.SetItemVisibility(index, visible);
        }

        public void ExternalSetMenu(string menumanifest_src) {
            if (this.menu_external != null) {
                this.menu_external.Destroy();
                this.menu_external = null;
            }

            if (String.IsNullOrEmpty(menumanifest_src) || !FS.FileExists(menumanifest_src)) {
                this.menu.SetItemVisibility(1, false);
                return;
            }

            this.menu.SetItemVisibility(1, true);

            LayoutPlaceholder menu_placeholder = this.menu_placeholder;
            MenuManifest menumanifest = new MenuManifest(menumanifest_src);
            this.menu_external = new Menu(
                menumanifest,
                menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
                menu_placeholder.width, menu_placeholder.height
            );
            menumanifest.Destroy();
        }

        public void Prepare() {
            if (this.background_menu_music != null) this.background_menu_music.Destroy();
            this.background_menu_music = SoundPlayer.Init("/assets/common/music/breakfast.ogg");
            if (this.background_menu_music != null) this.background_menu_music.LoopEnable(true);
        }

        public int HelperShow(RoundContext roundcontext, int dettached_index) {
            bool dettached_controller = dettached_index >= 0;
            Gamepad controller = new Gamepad(-1);
            float antibounce = dettached_controller ? 1000.0f : WeekPause.ANTIBOUNCE;
            controller.SetButtonsDelay(WeekPause.ANTIBOUNCE / 2);

            this.messagebox.Hide(false);
            if (this.background_menu_music != null) {
                this.background_menu_music.SetVolume(0.5f);
                this.background_menu_music.Replay();
            }
            controller.ClearButtons();

            bool current_menu_external = false;
            Menu current_menu = this.menu;
            int selected_option = 0;

            this.menu.SelectIndex(0);
            if (this.menu_external != null) this.menu_external.SelectIndex(0);
            this.menu_placeholder.vertex = this.menu.GetDrawable();

            TextSprite textsprite = this.layout.GetTextsprite("stats");
            if (textsprite != null) {
                int track_index = roundcontext.track_index;
                GameplayManifestTrack trackmanifest = roundcontext.initparams.gameplaymanifest.tracks[track_index];
                string track_name = trackmanifest.name;
                string track_difficult = roundcontext.track_difficult;
                textsprite.SetTextFormated("$s\n$s", track_name, track_difficult);
            }

            if (roundcontext.script != null) roundcontext.script.NotifyPause(true);
            while (roundcontext.scriptcontext.halt_flag) InternalRender(roundcontext);

            this.layout.TriggerAny(null);

            if (dettached_controller) {
                dettached_index++;// do not use base-zero index
                this.messagebox.SetButtonSingle("(Waiting controller)");
                this.messagebox.SetTitle("Controller disconnected");
                this.messagebox.SetMessageFormated(WeekPause.MSGCONTROLLER, dettached_index);
                this.messagebox.ShowButtonsIcons(false);
                this.messagebox.UseSmallSize(false);
                this.messagebox.HideImage(false);
                this.messagebox.SetImageSprite(this.sprite_nocontroller);
                this.messagebox.Show(true);
            }

            while (true) {
                bool to_external_menu = false;
                float elapsed = InternalRender(roundcontext);

                GamepadButtons buttons = controller.HasPressedDelayed(
                    GamepadButtons.START | GamepadButtons.AD_UP | GamepadButtons.AD_DOWN | GamepadButtons.A | GamepadButtons.X | GamepadButtons.B | GamepadButtons.BACK
                );

                if (antibounce > 0) {
                    if (buttons != GamepadButtons.NOTHING)
                        antibounce -= elapsed;
                    else
                        antibounce = 0.0f;
                    continue;
                }

                if (selected_option != 0) {
                    if ((buttons & (GamepadButtons.A | GamepadButtons.X)) != GamepadButtons.NOTHING) {
                        break;
                    } else if ((buttons & (GamepadButtons.B | GamepadButtons.START | GamepadButtons.BACK)) != GamepadButtons.NOTHING) {
                        selected_option = 0;
                        if ((buttons & (GamepadButtons.B | GamepadButtons.BACK)) != GamepadButtons.NOTHING) {
                            antibounce = 200.0f;
                            this.messagebox.Hide(false);
                        } else {
                            break;
                        }
                    }
                    continue;
                }

                if (dettached_controller) {
                    int dettached_count = 0;
                    int available = 0;
                    for (int i = 0 ; i < roundcontext.players_size ; i++) {
                        if (roundcontext.players[i].controller == null) continue;
                        if (roundcontext.players[i].controller.IsDettached()) {
                            if (roundcontext.players[i].controller.Pick(true)) {
                                available++;
                            } else {
                                dettached_count++;
                            }
                        } else {
                            available++;
                        }
                    }
                    if (available < 1) continue;
                    if (dettached_count < 1) {
                        dettached_controller = false;
                        this.messagebox.Hide(false);
                    }
                }

                if ((buttons & GamepadButtons.START) != GamepadButtons.NOTHING) {
                    if (!current_menu_external) break;
                } else if ((buttons & GamepadButtons.AD_UP) != GamepadButtons.NOTHING) {
                    if (!current_menu.SelectVertical(-1))
                        current_menu.SelectIndex(current_menu.GetItemsCount() - 1);
                } else if ((buttons & GamepadButtons.AD_DOWN) != GamepadButtons.NOTHING) {
                    if (!current_menu.SelectVertical(1))
                        current_menu.SelectIndex(0);
                } else if ((buttons & (GamepadButtons.A | GamepadButtons.X)) != GamepadButtons.NOTHING) {
                    if (current_menu_external) {
                        to_external_menu = true;
                        buttons = controller.GetPressed();
                    } else {
                        int option_index = current_menu.GetSelectedIndex();
                        if (option_index == 0) {
                            selected_option = 0;// resume
                            break;
                        } else if (option_index == 1 && this.menu_external != null) {
                            // display week menu
                            this.menu_external.TrasitionIn();
                            this.menu_placeholder.vertex = this.menu_external.GetDrawable();
                            current_menu_external = true;
                            if (roundcontext.script != null) roundcontext.script.NotifyPauseMenuvisible(true);
                        } else if (option_index == 2) {
                            selected_option = 1;// restart song
                            break;
                        } else if (option_index == 3) {
                            selected_option = 2;// back to weekselector
                        } else if (option_index == 4) {
                            selected_option = 3;// back to mainmenu
                        }

                        if (selected_option != 0) {
                            string msg = selected_option == 2 ? WeekPause.MSGWEEKSELECTOR : WeekPause.MSGMENU;
                            this.messagebox.HideImage(true);
                            this.messagebox.SetButtonsIcons("a", "b");
                            this.messagebox.SetButtonsText("Ok", "Cancel");
                            this.messagebox.SetTitle("Confirm");
                            this.messagebox.UseSmallSize(true);
                            this.messagebox.SetMessage(msg);
                            this.messagebox.Show(false);
                            controller.ClearButtons();
                        }
                    }
                } else if ((buttons & (GamepadButtons.B | GamepadButtons.BACK)) != GamepadButtons.NOTHING) {
                    if (current_menu_external) {
                        current_menu_external = false;
                        this.menu_placeholder.vertex = this.menu.GetDrawable();
                        if (roundcontext.script != null) roundcontext.script.NotifyPauseMenuvisible(false);
                        this.menu.TrasitionIn();
                    } else {
                        selected_option = 0;
                        break;
                    }
                } else if (current_menu_external) {
                    buttons = controller.GetPressed();
                    to_external_menu = buttons != GamepadButtons.NOTHING;// notify script if has buttons pressed
                }

                if (to_external_menu) {
                    int option_index = this.menu_external.GetSelectedIndex();
                    buttons = controller.GetPressed();
                    if (roundcontext.script != null) roundcontext.script.NotifyPauseOptionselected(option_index, buttons);
                    while (roundcontext.scriptcontext.halt_flag) InternalRender(roundcontext);
                }
            }

            if (this.background_menu_music != null) this.background_menu_music.Fade(false, 100.0f);

            if (roundcontext.script != null) roundcontext.script.NotifyPause(false);
            while (roundcontext.scriptcontext.halt_flag) InternalRender(roundcontext);

            if (this.background_menu_music != null) this.background_menu_music.Stop();
            this.messagebox.Hide(true);

            if (selected_option == 0) {
                for (int i = 0 ; i < roundcontext.players_size ; i++) {
                    if (roundcontext.players[i].controller != null) roundcontext.players[i].controller.ClearButtons();
                }
            } else {
                string target = selected_option == 1 ? "transition_fast" : "transition";
                this.layout.TriggerAny(target);

                while (true) {
                    float elapsed = InternalRender(roundcontext);

                    if (this.layout.AnimationIsCompleted("transition_effect") > 0) break;
                }
            }

            // selected options:
            //      0 -> resume
            //      1 -> restart song
            //      2 -> back to weekselector
            //      3 -> back to mainmenu
            return selected_option;
        }

        private float InternalRender(RoundContext roundcontext) {
            GamepadButtons buttons = GamepadButtons.NOTHING;

            float elapsed = PVRContext.global_context.WaitReady();
            PVRContext.global_context.Reset();

            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                Gamepad controller = roundcontext.players[i].controller;
                if (controller != null && controller.GetManagedPresses(true, ref buttons)) {
                    if (roundcontext.script != null) roundcontext.script.NotifyButtons(i, buttons);
                }
            }

            if (roundcontext.script != null) roundcontext.script.NotifyFrame(elapsed);

            // draw the stage+ui layout but do not animate
            roundcontext.layout.Draw(PVRContext.global_context);

            this.layout.Animate(elapsed);
            this.layout.Draw(PVRContext.global_context);

            this.messagebox.Animate(elapsed);
            this.messagebox.Draw(PVRContext.global_context);

            return elapsed;
        }

    }

}