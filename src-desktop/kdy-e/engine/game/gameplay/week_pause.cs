using System;
using Engine.Font;
using Engine.Game.Common;
using Engine.Game.Gameplay.Helpers;
using Engine.Image;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;

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
                    name= "resume",
                    text= "RESUME",// unused
                    placement= { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
                    anim_selected= null,// unused
                    anim_choosen= null,// unused
                    anim_discarded= null,// unused
                    anim_idle= null,// unused
                    anim_rollback= null,// unused
                    anim_in= null,// unused
                    anim_out= null,// unused
                    hidden= false,
                    description= null// unused
                },
                new MenuManifest.Item() {
                    name= "week-menu",
                    text= "WEEK MENU",// unused
                    placement= { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
                    anim_selected= null,// unused
                    anim_choosen= null,// unused
                    anim_discarded= null,// unused
                    anim_idle= null,// unused
                    anim_rollback= null,// unused
                    anim_in= null,// unused
                    anim_out= null,// unused
                    hidden= true,
                    description= null// unused
                },
                new MenuManifest.Item() {
                    name= "restart-song",
                    text= "RESTART SONG",// unused
                    placement= { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
                    anim_selected= null,// unused
                    anim_choosen= null,// unused
                    anim_discarded= null,// unused
                    anim_idle= null,// unused
                    anim_rollback= null,// unused
                    anim_in= null,// unused
                    anim_out= null,// unused
                    hidden= false,
                    description= null// unused
                },
                new MenuManifest.Item()  {
                    name= "exit-week-selector",
                    text= "EXIT TO WEEK SELECTOR",// unused
                    placement= { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
                    anim_selected= null,// unused
                    anim_choosen= null,// unused
                    anim_discarded= null,// unused
                    anim_idle= null,// unused
                    anim_rollback= null,// unused
                    anim_in= null,// unused
                    anim_out= null,// unused
                    hidden= false,
                    description= null// unused
                },
                new MenuManifest.Item() {
                    name= "exit-main-menu",
                    text= "EXIT TO MAIN MENU",// unused
                    placement= { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
                    anim_selected= null,// unused
                    anim_choosen= null,// unused
                    anim_discarded= null,// unused
                    anim_idle= null,// unused
                    anim_rollback= null,// unused
                    anim_in= null,// unused
                    anim_out= null,// unused
                    hidden= false,
                    description= null// unused
                }
            },
            items_size = 5
        };
        private const string LAYOUT_WIDESCREEN = "/assets/common/image/week-round/pause.xml";
        private const string LAYOUT_DREAMCAST = "/assets/common/image/week-round/pause~dreamcast.xml";
        private const string NOCONTROLLER = "/assets/common/image/week-round/no_controller.png";
        private const string MODDING_SCRIPT = "/assets/common/data/scripts/weekpause.lua";
        private const string MODDING_MENU = "/assets/common/data/menus/weekpause.json";
        private const GamepadButtons BUTTONS = GamepadButtons.START | GamepadButtons.AD_UP | GamepadButtons.AD_DOWN | GamepadButtons.A | GamepadButtons.X | GamepadButtons.B | GamepadButtons.BACK;
        private const int DELAY = 170;
        private const int ANTIBOUNCE = (int)(WeekPause.DELAY * 1.25f);

        // messagebox strings
        public const string MSGCONTROLLER = "The controller $i was disconnected, \n" +
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
        private Modding modding;
        private string modding_choosen_option_name;


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

            MenuManifest menumanifest = WeekPause.MENU;
            if (FS.FileExists(WeekPause.MODDING_MENU)) {
                menumanifest = new MenuManifest(WeekPause.MODDING_MENU);
                if (menumanifest == null) throw new Exception("failed to load " + WeekPause.MODDING_MENU);
            }

            Menu menu = new Menu(
                menumanifest,
                menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
                menu_placeholder.width, menu_placeholder.height
            );

            if (menumanifest != WeekPause.MENU) menumanifest.Destroy();

            MessageBox messagebox = new MessageBox();
            Sprite sprite_nocontroller = Sprite.Init(Texture.Init(WeekPause.NOCONTROLLER));

            Modding modding = new Modding(layout, WeekPause.MODDING_SCRIPT);
            modding.native_menu = modding.active_menu = menu;
            modding.callback_private_data = this;
            modding.callback_option = this.InternalHandleModdingOption;

            this.menu = menu;
            this.messagebox = messagebox;
            this.sprite_nocontroller = sprite_nocontroller;
            this.layout = layout;
            this.menu_placeholder = menu_placeholder;
            this.menu_external = null;
            this.background_menu_music = null;
            this.modding = modding;
            this.modding_choosen_option_name = null;
        }

        public void Destroy() {
            this.layout.Destroy();
            this.menu.Destroy();
            this.messagebox.Destroy();
            this.sprite_nocontroller.DestroyFull();
            this.modding.Destroy();
            if (this.menu_external != null) this.menu_external.Destroy();
            if (this.background_menu_music != null) this.background_menu_music.Destroy();
            this.modding_choosen_option_name = null;// do not dispose
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
            controller.SetButtonsDelay(WeekPause.DELAY);

            this.messagebox.Hide(false);
            if (this.background_menu_music != null) {
                this.background_menu_music.SetVolume(0.5f);
                this.background_menu_music.Replay();
            }
            controller.ClearButtons();

            bool current_menu_is_external = false;
            Menu current_menu = this.menu;
            int return_value = 0;

            this.menu.SelectIndex(0);
            if (this.menu_external != null) this.menu_external.SelectIndex(0);
            this.menu_placeholder.vertex = this.menu.GetDrawable();

            TextSprite textsprite = this.layout.GetTextsprite("stats");
            if (textsprite != null) {
                int song_index = roundcontext.song_index;
                GameplayManifestSong songmanifest = roundcontext.initparams.gameplaymanifest.songs[song_index];
                string song_name = songmanifest.name;
                string song_difficult = roundcontext.song_difficult;
                textsprite.SetTextFormated("$s\n$s", song_name, song_difficult);
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

            this.modding_choosen_option_name = null;
            this.modding.has_exit = false;
            this.modding.has_halt = false;
            this.modding.native_menu = this.modding.active_menu;
            this.modding.HelperNotifyInit(Modding.NATIVE_MENU_SCREEN);
            this.modding.HelperNotifyOption(true);

            while (!this.modding.has_exit) {
                bool has_option_choosen = false;
                bool go_back = false;
                float elapsed = InternalRender(roundcontext);
                GamepadButtons buttons = controller.HasPressedDelayed(WeekPause.BUTTONS);

                // if the messagebox is visible, wait decision
                if (return_value != 0) {
                    if ((buttons & (GamepadButtons.A | GamepadButtons.X)).Bool()) {
                        break;
                    } else if ((buttons & (GamepadButtons.B | GamepadButtons.START | GamepadButtons.BACK)).Bool()) {
                        return_value = 0;
                        if ((buttons & (GamepadButtons.B | GamepadButtons.BACK)).Bool()) {
                            controller.SetButtonsDelay(WeekPause.DELAY);
                            controller.EnforceButtonsDelay();
                            this.messagebox.Hide(false);
                        } else {
                            break;
                        }
                    }
                    continue;
                }

                ModdingHelperResult res = this.modding.HelperHandleCustomMenu(controller, elapsed);
                if (res != ModdingHelperResult.CONTINUE) break;
                if (this.modding.has_halt || this.modding.active_menu != this.menu) continue;
                if (this.modding_choosen_option_name != null) {
                    buttons = GamepadButtons.NOTHING;
                    has_option_choosen = true;
                    break;
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

                if ((buttons & GamepadButtons.START).Bool()) {
                    if (!current_menu_is_external && !this.modding.HelperNotifyBack()) break;
                } else if ((buttons & GamepadButtons.AD_UP).Bool()) {
                    if (!current_menu.SelectVertical(-1))
                        current_menu.SelectIndex(current_menu.GetItemsCount() - 1);
                    this.modding.HelperNotifyOption(true);
                } else if ((buttons & GamepadButtons.AD_DOWN).Bool()) {
                    if (!current_menu.SelectVertical(1))
                        current_menu.SelectIndex(0);
                    this.modding.HelperNotifyOption(true);
                } else if ((buttons & (GamepadButtons.A | GamepadButtons.X)).Bool()) {
                    has_option_choosen = true;
                } else if ((buttons & (GamepadButtons.B | GamepadButtons.BACK)).Bool()) {
                    go_back = true;
                } else if (!has_option_choosen) {
                    // nothing to do
                    continue;
                }


                if (has_option_choosen && current_menu_is_external) {
                    int option_index = current_menu.GetSelectedIndex();
                    roundcontext.script.NotifyPauseOptionChoosen(option_index);
                    has_option_choosen = false;
                    go_back = true;
                } else if (has_option_choosen) {
                    if (this.modding_choosen_option_name == null) {
                        this.modding_choosen_option_name = current_menu.GetSelectedItemName();
                        if (this.modding.HelperNotifyOption(false)) {
                            this.modding_choosen_option_name = null;
                            continue;
                        }
                    }

                    return_value = this.InternalReturnValue();
                    this.modding_choosen_option_name = null;

                    if (return_value == 0) {
                        // resume
                        break;
                    } else if (return_value == 4) {
                        return_value = 0;
                        if (this.menu_external == null || roundcontext.script == null) continue;

                        // display week menu
                        current_menu = this.menu_external;
                        this.modding.callback_option = null;
                        this.modding.HelperNotifyEvent("week-custom-menu");
                        current_menu.TrasitionIn();
                        this.menu_placeholder.vertex = current_menu.GetDrawable();
                        current_menu_is_external = true;
                        if (roundcontext.script != null) roundcontext.script.NotifyPauseMenuvisible(true);
                    } else if (return_value == 1) {
                        // restart song
                        break;
                    } else if (return_value == 2 || return_value == 3) {
                        string msg = return_value == 2 ? WeekPause.MSGWEEKSELECTOR : WeekPause.MSGMENU;
                        this.messagebox.HideImage(true);
                        this.messagebox.SetButtonsIcons("a", "b");
                        this.messagebox.SetButtonsText("Ok", "Cancel");
                        this.messagebox.SetTitle("Confirm");
                        this.messagebox.UseSmallSize(true);
                        this.messagebox.SetMessage(msg);
                        this.messagebox.Show(false);
                        controller.SetButtonsDelay(WeekPause.ANTIBOUNCE);
                        controller.EnforceButtonsDelay();
                    } else if (return_value == -1) {
                        // custom option menu
                        this.modding.callback_option = null;
                        this.modding.HelperNotifyHandleCustomOption(this.modding_choosen_option_name);
                        this.modding.callback_option = this.InternalHandleModdingOption;
                        this.modding_choosen_option_name = null;
                        return_value = 0;
                    }
                }

                if (go_back && current_menu_is_external) {
                    current_menu_is_external = false;
                    current_menu = this.menu;
                    this.menu_placeholder.vertex = this.menu.GetDrawable();
                    roundcontext.script.NotifyPauseMenuvisible(false);
                    this.modding.callback_option = InternalHandleModdingOption;
                    this.modding.HelperNotifyEvent("week-pause-menu");
                    this.menu.TrasitionIn();
                } else if (go_back && !this.modding.HelperNotifyBack()) {
                    return_value = 0;
                    break;
                }

            }

            if (this.background_menu_music != null) this.background_menu_music.Fade(false, 100.0f);

            if (roundcontext.script != null) roundcontext.script.NotifyPause(false);
            while (roundcontext.scriptcontext.halt_flag) InternalRender(roundcontext);

            if (this.background_menu_music != null) this.background_menu_music.Stop();
            this.messagebox.Hide(true);

            if (return_value != 0) {
                string target = return_value == 1 ? "transition_fast" : "transition";
                this.layout.TriggerAny(target);

                while (true) {
                    InternalRender(roundcontext);
                    if (this.layout.AnimationIsCompleted("transition_effect") > 0) break;
                }
            }

            this.modding.HelperNotifyExit2();

            // selected options:
            //      0 -> resume
            //      1 -> restart song
            //      2 -> back to weekselector
            //      3 -> back to mainmenu
            return return_value;
        }

        private float InternalRender(RoundContext roundcontext) {
            float elapsed;

            do {
                GamepadButtons buttons = GamepadButtons.NOTHING;

                elapsed = PVRContext.global_context.WaitReady();
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

            } while (roundcontext.scriptcontext.halt_flag);

            this.layout.Animate(elapsed);
            this.layout.Draw(PVRContext.global_context);

            this.messagebox.Animate(elapsed);
            this.messagebox.Draw(PVRContext.global_context);

            return elapsed;
        }

        private bool InternalHandleModdingOption(object obj, string option_name) {
            if (option_name == null) {
                // resume
                this.modding_choosen_option_name = WeekPause.MENU.items[0].name;
                this.menu.SelectItem(this.modding_choosen_option_name);
                return true;
            }

            // select native option
            int index = WeekPause.MENU.GetOptionIndex(option_name);
            if (index >= 0) {
                this.modding_choosen_option_name = WeekPause.MENU.items[index].name;
                this.menu.SelectItem(this.modding_choosen_option_name);
                return true;
            }

            // select custom option
            if (this.menu.HasItem(option_name)) {
                this.menu.SelectItem(option_name);
                this.modding_choosen_option_name = this.menu.GetSelectedItemName();
                return true;
            }

            // unknown option
            this.modding_choosen_option_name = option_name;
            return false;
        }

        private int InternalReturnValue() {
            switch (this.modding_choosen_option_name) {
                case null:
                case "resume":
                    return 0;
                case "week-menu":
                    return 4;
                case "restart-song":
                    return 1;
                case "exit-week-selector":
                    return 2;
                case "exit-main-menu":
                    return 3;
                default:
                    // custom option
                    return -1;
            }
        }

    }

}