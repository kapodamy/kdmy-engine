using System;
using System.Diagnostics;
using CsharpWrapper;
using Engine.Animation;
using Engine.Externals.GLFW;
using Engine.Font;
using Engine.Game.Common;
using Engine.Game.Gameplay.Helpers;
using Engine.Image;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;

namespace Engine.Game {
    public static class SettingsMenu {
        private const string KEY_NONE = "(none)";
        private const string KEY_BIND = "(waiting)";
        private const double DELAY_SECONDS = 0.25;
        private static readonly KeyCallback delegate_key_callback = InternalKeyCallback;


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
                animlist = "/assets/common/anims/settings-menu.xml",

                anim_selected = "menu_item_selected",
                anim_idle = "menu_item_idle",
                anim_choosen = null,// unused
                anim_discarded = null,// unused
                anim_rollback = null,// unused
                anim_in = null,// unused
                anim_out = null,// unused

                anim_transition_in_delay = 0,// unused
                anim_transition_out_delay = 0,// unused

                font = "/assets/common/font/Alphabet.xml",
                font_glyph_suffix = "bold",
                font_color_by_difference = false,// unused
                font_size = 46f,
                font_color = 0xFFFFFF,
                font_border_color = 0x00,// unused
                font_border_size = Single.NaN,// unused

                is_sparse = false,// unused
                is_vertical = true,
                is_per_page = false,
                static_index = 1,

                items_align = Align.START,
                items_gap = 38f,
                items_dimmen = 0f,// unused
                texture_scale = Single.NaN,// unused
                enable_horizontal_text_correction = true// unused
            },
            items = new MenuManifest.Item[] {
               new MenuManifest.Item() {
                    name= null,
                    text= "KEYBOARD BINDINGS IN GAMEPLAY",// unused
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
                    text= "KEYBOARD BINDINGS IN MENUS",// unused
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
                    text= "GAMEPLAY SETTINGS",// unused
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
                    text= "MISCELLANEOUS",// unused
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
                    text= "RETURN TO THE MAIN MENU",// unused
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
        private static readonly MenuManifest MENU_COMMON = new MenuManifest() {
            parameters = new MenuManifest.Parameters() {
                suffix_selected = null,// unused
                suffix_idle = null,// unused
                suffix_choosen = null,// unused
                suffix_discarded = null,// unused
                suffix_rollback = null,// unused
                suffix_in = null,// unused
                suffix_out = null,// unused

                atlas = null,
                animlist = "/assets/common/anims/settings-menu.xml",

                anim_selected = null,
                anim_idle = null,
                anim_choosen = null,// unused
                anim_discarded = null,// unused
                anim_rollback = null,// unused
                anim_in = null,// unused
                anim_out = null,// unused

                anim_transition_in_delay = 0,// unused
                anim_transition_out_delay = 0,// unused

                font = "/assets/common/font/pixel.otf",
                font_glyph_suffix = null,// unused
                font_color_by_difference = false,// unused
                font_size = 28f,
                font_color = 0xFFFFFF,
                font_border_color = 0x000000FF,// unused
                font_border_size = 4,// unused

                is_sparse = false,// unused
                is_vertical = true,
                is_per_page = true,
                static_index = 0,

                items_align = Align.START,
                items_gap = 58f,
                items_dimmen = 0f,// unused
                texture_scale = Single.NaN,// unused
                enable_horizontal_text_correction = true// unused
            },
            items = null,
            items_size = 0
        };


        private static double next_pressed_timestamp = 0.0;
        private static Keycode last_detected_keycode = Keycode.None;
        private static KeyCallback previous_key_callback = null;


        public static void Main() {
            SettingOption[] options_help = {
                new SettingOption() { description = "Change the assigned key for each strum.\nDirectonial keys are always assigned" },
                new SettingOption() { description = "Change the keys assigned to navigate between menus.\nDirectonial keys are always assigned" },
                new SettingOption() { description = "Gameplay settings like ghost-tapping and input offset" },
                new SettingOption() { description = "Specific engine settings" },
                new SettingOption() { description = "Returns back to the main menu.\n¡Settings are automatically saved!" }
            };

            AnimList animlist = AnimList.Init("/assets/common/anims/settings-menu.xml");
            AnimSprite anim_binding, anim_binding_rollback;

            if (animlist != null) {
                anim_binding = AnimSprite.InitFromAnimlist(animlist, "binding");
                anim_binding_rollback = AnimSprite.InitFromAnimlist(animlist, "binding_rollback");
                animlist.Destroy();
            } else {
                anim_binding = anim_binding_rollback = null;
            }

            Layout layout = Layout.Init("/assets/common/image/settings-menu/main.xml");
            if (layout == null) {
                Console.Error.WriteLine("[ERROR] settingsmenu_main() can not load the layout");
                return;
            }

            SettingsMenu.MENU.parameters.is_vertical = (bool)layout.GetAttachedValue(
                "menu_isVertical", AttachedValueType.BOOLEAN, SettingsMenu.MENU.parameters.is_vertical
            );
            SettingsMenu.MENU.parameters.font_size = layout.GetAttachedValueAsFloat(
                "menu_fontSize", SettingsMenu.MENU.parameters.font_size
            );
            SettingsMenu.MENU.parameters.items_gap = layout.GetAttachedValueAsFloat(
                "menu_itemGap", SettingsMenu.MENU.parameters.items_gap
            );
            LayoutPlaceholder menu_placeholder = layout.GetPlaceholder("menu");
            if (menu_placeholder == null) throw new System.Exception("Missing menu placeholder");

            Menu menu = new Menu(
                SettingsMenu.MENU,
                menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
                menu_placeholder.width, menu_placeholder.height
            );
            menu_placeholder.vertex = menu.GetDrawable();

            string backgroud_music_filename = (string)layout.GetAttachedValue(
                "backgroud_music_filename", AttachedValueType.STRING, null
            );
            float backgroud_music_volume = (float)(double)layout.GetAttachedValue(
                "backgroud_music_volume", AttachedValueType.STRING, 0.5
            );

            SoundPlayer bg_music = null;
            if (backgroud_music_filename != null) bg_music = SoundPlayer.Init(backgroud_music_filename);
            if (bg_music != null) bg_music.SetVolume(backgroud_music_volume);

            previous_key_callback = Glfw.SetKeyCallback(PVRContext.InternalNativeWindow, delegate_key_callback);

            Gamepad gamepad = new Gamepad(-1);
            gamepad.SetButtonsDelay(75);
            gamepad.ClearButtons();

            if (bg_music != null) {
                if (GameMain.background_menu_music != null) GameMain.background_menu_music.Pause();
                bg_music.LoopEnable(true);
                bg_music.Play();
            } else if (GameMain.background_menu_music != null) {
                GameMain.background_menu_music.SetVolume(0.5f);
            }

            while (true) {
                int selected_option = InCommonMenu(layout, gamepad, menu, options_help);
                switch (selected_option) {
                    case 0:
                        InGameplayBinding(anim_binding, anim_binding_rollback);
                        continue;
                    case 1:
                        InMenusBinding(anim_binding, anim_binding_rollback);
                        continue;
                    case 2:
                        InGameplaySettings(gamepad);
                        continue;
                    case 3:
                        InMiscSettings(gamepad);
                        continue;
                }
                break;
            }

            anim_binding.Destroy();
            anim_binding_rollback.Destroy();
            menu.Destroy();
            layout.Destroy();

            // flush settings to the INI file
            EngineSettings.ini.Flush();
            EngineSettings.Reload();

            Glfw.SetKeyCallback(PVRContext.InternalNativeWindow, previous_key_callback);

            if (bg_music != null) {
                bg_music.Destroy();
                if (GameMain.background_menu_music != null) GameMain.background_menu_music.Play();
            } else if (GameMain.background_menu_music != null) {
                GameMain.background_menu_music.SetVolume(1.0f);
            }
        }


        private static void InGameplayBinding(AnimSprite anim_binding, AnimSprite anim_binding_rollback) {
            Layout layout = Layout.Init("/assets/common/image/settings-menu/binds_gameplay.xml");

            if (layout == null) {
                Console.Error.WriteLine("[ERROR] settingsmenu_in_gameplay_binding() can not load the layout");
                return;
            }

            // binding colors
            uint color_bg_key = (uint)layout.GetAttachedValue("color_bg_key", AttachedValueType.HEX, 0x000000FFU);
            uint color_bg_key_selected = (uint)layout.GetAttachedValue("color_bg_key_selected", AttachedValueType.HEX, 0xFF0000FFU);
            uint color_bg_key_active = (uint)layout.GetAttachedValue("color_bg_key_active", AttachedValueType.HEX, 0x00FF00FFU);

            // aquire labels
            TextSprite[] labels = {
                layout.GetTextsprite("label-left0"),
                layout.GetTextsprite("label-down0"),
                layout.GetTextsprite("label-up0"),
                layout.GetTextsprite("label-right0"),

                layout.GetTextsprite("label-left1"),
                layout.GetTextsprite("label-down1"),
                layout.GetTextsprite("label-up1"),
                layout.GetTextsprite("label-right1"),

                layout.GetTextsprite("label-left2"),
                layout.GetTextsprite("label-down2"),
                layout.GetTextsprite("label-up2"),
                layout.GetTextsprite("label-right2"),

                layout.GetTextsprite("label-diamond")
            };

            // adquire backgrounds
            Sprite[] backs = {
                layout.GetSprite("back-left0"),
                layout.GetSprite("back-down0"),
                layout.GetSprite("back-up0"),
                layout.GetSprite("back-right0"),

                layout.GetSprite("back-left1"),
                layout.GetSprite("back-down1"),
                layout.GetSprite("back-up1"),
                layout.GetSprite("back-right1"),

                layout.GetSprite("back-left2"),
                layout.GetSprite("back-down2"),
                layout.GetSprite("back-up2"),
                layout.GetSprite("back-right2"),

                layout.GetSprite("back-diamond")
            };

            // key + scancodes
            Keycode[] keycodes = {
                new Keycode(Keys.F),
                new Keycode(Keys.G),
                new Keycode(Keys.J),
                new Keycode(Keys.H),
                Keycode.None,
                Keycode.None,
                Keycode.None,
                Keycode.None,
                Keycode.None,
                Keycode.None,
                Keycode.None,
                Keycode.None,

                new Keycode(Keys.SPACE)
            };

            int binds_count = 13;
            string[] ini_entries = {
                "left1", "down1", "up1", "right1",
                "left2", "down2", "up2", "right2",
                "left3", "down3", "up3", "right3",
                "diamond"
            };

            // load gameplay bindings
            for (int i = 0 ; i < binds_count ; i++) {
                keycodes[i] = InternalLoadKey(labels[i], ini_entries[i], keycodes[i]);
            }

            int row = 0, column = 0, selected_index = 0;
            bool is_binding = false;

            // select first holder
            backs[selected_index].SetVertexColorRGBA8(color_bg_key_selected);

            // anti-bounce from parent menu
            last_detected_keycode = Keycode.None;
            next_pressed_timestamp = Glfw.GetTime() + SettingsMenu.DELAY_SECONDS;

            while (true) {
                float elapsed = PVRContext.global_context.WaitReady();
                layout.Animate(elapsed);
                layout.Draw(PVRContext.global_context);

                if (is_binding && anim_binding != null) {
                    anim_binding.Animate(elapsed);
                    anim_binding.UpdateSprite(backs[selected_index], true);
                }

                GamepadButtons pressed = InternalReadKeyboard();

                if (is_binding) {
                    Keycode keycode = SettingsMenu.last_detected_keycode;

                    if (pressed != GamepadButtons.BACK) {
                        if (pressed == GamepadButtons.START) {
                            keycodes[selected_index] = Keycode.None;// clear binding
                        } else if (keycode.IsNone) {
                            continue;// keep waiting
                        } else {
                            keycodes[selected_index] = keycode;
                        }
                    }

                    // update holder
                    InternalUpdateHolder(
                        backs, labels, keycodes, selected_index,
                        anim_binding_rollback, true, color_bg_key_selected
                    );
                    is_binding = false;
                    continue;
                }

                if (pressed == GamepadButtons.NOTHING) {
                    continue;
                } else if (pressed == GamepadButtons.BACK) {
                    break;
                } else if (pressed == GamepadButtons.START) {
                    is_binding = true;
                    SettingsMenu.last_detected_keycode = Keycode.None;
                    InternalUpdateHolder(
                        backs, labels, keycodes, selected_index,
                        anim_binding, false, color_bg_key_active
                    );
                    labels[selected_index].SetTextIntern(true, KEY_BIND);
                    if (anim_binding != null) anim_binding.Restart();
                    continue;
                }

                // calculate movement
                int new_row = row, new_column = column, new_index = selected_index;

                if (pressed == GamepadButtons.DPAD_LEFT) new_column--;
                if (pressed == GamepadButtons.DPAD_RIGHT) new_column++;
                if (pressed == GamepadButtons.DPAD_UP) new_row--;
                if (pressed == GamepadButtons.DPAD_DOWN) new_row++;

                // unselect current holder
                InternalUpdateHolder(
                    backs, labels, keycodes, selected_index,
                    null, false, color_bg_key
                );

                if (new_row >= 0 && new_row < 4) row = new_row;
                if (new_column >= 0 && new_column < 4) column = new_column;

                if (new_row >= 4)
                    selected_index = binds_count - 1;
                else
                    selected_index = column + (row * 4);

                if (selected_index >= binds_count)
                    selected_index = binds_count - 1;

                // select the new holder
                InternalUpdateHolder(
                    backs, labels, keycodes, selected_index,
                    null, false, color_bg_key_selected
                );
            }

            // save bindings
            for (int i = 0 ; i < binds_count ; i++) {
                InternalSaveKey(ini_entries[i], keycodes[i]);
            }

            layout.Destroy();
        }

        private static void InMenusBinding(AnimSprite anim_binding, AnimSprite anim_binding_rollback) {
            Layout layout = Layout.Init("/assets/common/image/settings-menu/binds_menus.xml");

            if (layout == null) {
                Console.Error.WriteLine("[ERROR] settingsmenu_in_menus_binding() can not load the layout");
                return;
            }

            // binding colors
            uint color_bg_key = (uint)layout.GetAttachedValue("color_bg_key", AttachedValueType.HEX, 0x000000FFU);
            uint color_bg_key_selected = (uint)layout.GetAttachedValue("color_bg_key_selected", AttachedValueType.HEX, 0xFF0000FFU);
            uint color_bg_key_active = (uint)layout.GetAttachedValue("color_bg_key_active", AttachedValueType.HEX, 0x00FF00FFU);

            // aquire labels
            TextSprite[] labels = {
                layout.GetTextsprite("label-accept"),
                layout.GetTextsprite("label-selectorLeft"),
                layout.GetTextsprite("label-selectorRight"),
                layout.GetTextsprite("label-alternativeTracks")
            };

            // adquire backgrounds
            Sprite[] backs = {
                layout.GetSprite("back-accept"),
                layout.GetSprite("back-selectorLeft"),
                layout.GetSprite("back-selectorRight"),
                layout.GetSprite("back-alternativeTracks")
            };

            int binds_count = 4;
            string[] ini_entries = {
                "menuAccept", "menuSelectorLeft", "menuSelectorRight", "menuAlternativeTracks"
            };

            // key + scancodes
            Keycode[] keycodes = {
                new Keycode(Keys.A),
                new Keycode(Keys.Q),
                new Keycode(Keys.W),
                new Keycode(Keys.S)
            };

            // load gameplay bindings
            for (int i = 0 ; i < binds_count ; i++) {
                keycodes[i] = InternalLoadKey(labels[i], ini_entries[i], keycodes[i]);
            }

            int selected_index = 0;
            bool is_binding = false;

            // select first holder
            backs[selected_index].SetVertexColorRGBA8(color_bg_key_selected);

            // anti-bounce from parent menu
            last_detected_keycode = Keycode.None;
            next_pressed_timestamp = Glfw.GetTime() + SettingsMenu.DELAY_SECONDS;

            while (true) {
                float elapsed = PVRContext.global_context.WaitReady();
                layout.Animate(elapsed);
                layout.Draw(PVRContext.global_context);

                if (is_binding && anim_binding != null) {
                    anim_binding.Animate(elapsed);
                    anim_binding.UpdateSprite(backs[selected_index], true);
                }

                GamepadButtons pressed = InternalReadKeyboard();

                if (is_binding) {
                    Keycode keycode = SettingsMenu.last_detected_keycode;

                    if (pressed != GamepadButtons.BACK) {
                        if (pressed == GamepadButtons.START) {
                            keycodes[selected_index] = Keycode.None;// clear binding
                        } else if (keycode.IsNone) {
                            continue;// keep waiting
                        } else {
                            keycodes[selected_index] = keycode;
                        }
                    }

                    // update holder
                    InternalUpdateHolder(
                        backs, labels, keycodes, selected_index,
                        anim_binding_rollback, true, color_bg_key_selected
                    );
                    is_binding = false;
                    continue;
                }

                if (pressed == GamepadButtons.NOTHING) {
                    continue;
                } else if (pressed == GamepadButtons.BACK) {
                    break;
                } else if (pressed == GamepadButtons.START) {
                    is_binding = true;
                    SettingsMenu.last_detected_keycode = Keycode.None;
                    InternalUpdateHolder(
                        backs, labels, keycodes, selected_index,
                        anim_binding, false, color_bg_key_active
                    );
                    labels[selected_index].SetTextIntern(true, KEY_BIND);
                    if (anim_binding != null) anim_binding.Restart();
                    continue;
                }

                // calculate movement
                int new_index = selected_index;

                if (pressed == GamepadButtons.DPAD_UP) new_index--;
                if (pressed == GamepadButtons.DPAD_DOWN) new_index++;

                if (new_index < 0 || new_index >= binds_count) {
                    continue;
                }

                // unselect current holder
                InternalUpdateHolder(
                    backs, labels, keycodes, selected_index,
                    null, false, color_bg_key
                );

                // select the new holder
                InternalUpdateHolder(
                    backs, labels, keycodes, new_index,
                    null, false, color_bg_key_selected
                );
                selected_index = new_index;
            }

            // save bindings
            for (int i = 0 ; i < binds_count ; i++) {
                InternalSaveKey(ini_entries[i], keycodes[i]);
            }
            layout.Destroy();

            KallistiOS.MAPLE.maple_mappings.LoadKeyboardMappings();
        }

        private static int InCommonMenu(Layout layout, Gamepad gamepad, Menu menu, SettingOption[] options) {
            TextSprite hint = layout.GetTextsprite("hint");
            gamepad.SetButtonsDelay((int)(SettingsMenu.DELAY_SECONDS * 1000));

            layout.TriggerAny(null);

            int selected_index = menu.GetSelectedIndex();
            if (selected_index >= 0 && selected_index < menu.GetItemsCount() && hint != null) {
                hint.SetTextIntern(true, options[selected_index].description);
            }

            int option = -1;
            gamepad.ClearButtons();

            while (true) {
                float elapsed = PVRContext.global_context.WaitReady();

                layout.Animate(elapsed);
                layout.Draw(PVRContext.global_context);

                GamepadButtons buttons = gamepad.HasPressedDelayed(
                    GamepadButtons.AD |
                    GamepadButtons.START | GamepadButtons.BACK |
                    GamepadButtons.B | GamepadButtons.A
                );

                if ((buttons & (GamepadButtons.BACK | GamepadButtons.B)) != GamepadButtons.NOTHING) {
                    break;
                } else if ((buttons & (GamepadButtons.START | GamepadButtons.A)) != GamepadButtons.NOTHING) {
                    option = menu.GetSelectedIndex();
                    break;
                } else if ((buttons & GamepadButtons.AD_UP) != GamepadButtons.NOTHING) {
                    menu.SelectVertical(-1);
                } else if ((buttons & GamepadButtons.AD_DOWN) != GamepadButtons.NOTHING) {
                    menu.SelectVertical(1);
                } else if ((buttons & GamepadButtons.AD_LEFT) != GamepadButtons.NOTHING) {
                    menu.SelectHorizontal(-1);
                } else if ((buttons & GamepadButtons.AD_RIGHT) != GamepadButtons.NOTHING) {
                    menu.SelectHorizontal(1);
                } else {
                    continue;
                }

                selected_index = menu.GetSelectedIndex();
                if (selected_index >= 0 && selected_index < menu.GetItemsCount() && hint != null) {
                    hint.SetTextIntern(true, options[selected_index].description);
                }

            }

            return option;
        }

        private static void InGameplaySettings(Gamepad gamepad) {
            SettingOption[] options = {
                new SettingOption() {
                    name = "USE FUNKIN MARKER DURATION",
                    description = "Uses a 166.67ms marker duration instead of a pixel accurate duration.\n" +
                                  "ENABLED: the hit and rank calculation is tight like Funkin,\n" +
                                  "DISABLED: the engine calculate the duration according to the screen",
                    is_bool = true,
                    value_bool = true,
                    ini_key="use_funkin_marker_duration"
                },
                new SettingOption() {
                    name = "PENALITY ON EMPTY STRUM",
                    description = "Penalizes the player if press a button on empty strum.\n" +
                                  "ENABLED: the penality is enforced. DISABLED: allows ghost-tapping",
                    is_bool = true,
                    value_bool = true,
                    ini_key="penality_on_empty_strum"
                },
                new SettingOption() {
                    name = "INPUT OFFSET",
                    description = "Offets all button press timestamps. Defaults to 0 milliseconds\n"+
                    "useful for bluetooth headset or television screens",
                    is_int = true,
                    value_int = 0,
                    number_min = -1000,
                    number_max = 1000,
                    ini_key = "input_offset"
                },
                new SettingOption() {
                    name = "INVERSE STRUM SCROLLING",
                    description = "In the default UI layout this switch from UPSCROLL to DOWNSCROLL,\n" +
                                  "In custom UI layouts, reverses the declared direction",
                    is_bool = true,
                    value_bool = false,
                    ini_key = "inverse_strum_scroll"
                },
                new SettingOption() {
                    name = "SHOW SONG PROGRESS BAR",
                    description = "Shows a progress bar near the strums makers.\n" +
                                  "Indicates playback progress of the playing track",
                    is_bool = true,
                    value_bool = true,
                    ini_key = "song_progressbar"
                },
                new SettingOption() {
                    name = "SONG PROGRESS BAR REMAINING",
                    description = "Shows the remaining song duration instead of time elapsed",
                    is_bool = true,
                    value_bool = false,
                    ini_key = "song_progressbar_remaining"
                },
                new SettingOption() {
                    name = "DISTRACTIONS",
                    description = "Enables/disables stage distractions during the gameplay.\n" +
                                  "(Depends on the week if honours this setting)",
                    is_bool = true,
                    value_bool = true,
                    ini_key = "gameplay_enabled_distractions"
                },
                new SettingOption() {
                    name = "FLASHING LIGHTS",
                    description = "Rhythm games often use flashing lights, sometimes they can be annoying or cause seizures.\n" +
                                  "(Depends on the week if honours this setting)",
                    is_bool = true,
                    value_bool = true,
                    ini_key = "gameplay_enabled_flashinglights"
                },
                new SettingOption() {
                    name = "UI COSMETICS",
                    description = "Hides away the accuracy, rank, and streak counters.\n" +
                                  "These elements normally are shown near or surrounding girlfriend's speakers",
                    is_bool = true,
                    value_bool = true,
                    ini_key = "gameplay_enabled_ui_cosmetics"
                },

            };
            int options_size = options.Length;

            // load current settings
            for (int i = 0 ; i < options_size ; i++) {
                InternalLoadOption(ref options[i], EngineSettings.INI_GAMEPLAY_SECTION);
            }

            ShowCommon("GAMEPLAY SETTINGS", gamepad, options, options_size);

            // save settings
            for (int i = 0 ; i < options_size ; i++) {
                InternalSaveOption(ref options[i], EngineSettings.INI_GAMEPLAY_SECTION);
            }
        }

        private static void InMiscSettings(Gamepad gamepad) {
            SettingOption[] options = {
                new SettingOption() {
                    name = "DISPLAY FPS",
                    description = "Displays the current FPS in the top-left corner of the screen.\n",
                    is_bool = true,
                    value_bool = false,
                    ini_key = "show_fps"
                },
                new SettingOption() {
                    name = "FPS LIMIT",
                    description = "Sets the frame limiter. V-SYNC: uses the screen frame-rate (default).\n" +
                                  "DETERMINISTIC: for constant input checking with low CPU overhead.\n" +
                                  "OFF: no fps limit, high CPU usage.",
                    is_list = true,
                    value_list_index = 0,
                    list = new SettingOptionListItem[] {
                        new SettingOptionListItem() { name="V-SYNC", ini_value="vsync"},
                        new SettingOptionListItem() { name="Deterministic", ini_value="deterministic"},
                        new SettingOptionListItem() { name="Off", ini_value="off"}
                    },
                    list_size = 3,
                    ini_key = "fps_limit"
                },
                new SettingOption() {
                    name = "START IN FULLSCREEN",
                    description = "Runs the game in fullscreen.\n" +
                                  "Press F11 to toggle between windowed/fullscreen",
                    is_bool = true,
                    value_bool = false,
                    ini_key = "fullscreen"
                },
                new SettingOption() {
                    name = "AVAILABLE SAVESLOTS",
                    description = "(Desktop version only) Emulates the specified number of VMUs.\n" +
                                  "Defaults to 1, this applies before the engine runs",
                    is_int = true,
                    value_int = 1,
                    number_min = 1,
                    number_max = 8,
                    ini_key="saveslots"
                }
            };
            int options_size = 4;

            // load current settings
            for (int i = 0 ; i < options_size ; i++) {
                InternalLoadOption(ref options[i], EngineSettings.INI_MISC_SECTION);
            }

            ShowCommon("MISCELLANEOUS", gamepad, options, options_size);

            // save settings
            for (int i = 0 ; i < options_size ; i++) {
                InternalSaveOption(ref options[i], EngineSettings.INI_MISC_SECTION);
            }
        }

        private static void ShowCommon(string title, Gamepad gamepad, SettingOption[] options, int options_count) {
            Layout layout = Layout.Init("/assets/common/image/settings-menu/common.xml");

            if (layout == null) {
                Console.Error.WriteLine("[ERROR] settingsmenu_show_common() can not load the layout");
                return;
            }

            layout.GetTextsprite("title").SetTextIntern(true, title);

            TextSprite setting_name = layout.GetTextsprite("setting-name");

            // settings menu
            bool is_vertical = (bool)layout.GetAttachedValue(
                "menu_isVertical", AttachedValueType.BOOLEAN, false
            );
            float font_size = layout.GetAttachedValueAsFloat(
                "menu_fontSize", 28f
            );
            float items_gap = layout.GetAttachedValueAsFloat(
                "menu_itemGap", 56f
            );
            float font_border_size = layout.GetAttachedValueAsFloat(
                "menu_fontBorderSize", 4f
            );
            LayoutPlaceholder menu_placeholder = layout.GetPlaceholder("menu");
            if (menu_placeholder == null) throw new System.Exception("Missing menu placeholder");

            // create menu options
            SettingsMenu.MENU_COMMON.parameters.is_vertical = is_vertical;
            SettingsMenu.MENU_COMMON.parameters.font_size = font_size;
            SettingsMenu.MENU_COMMON.parameters.items_gap = items_gap;
            SettingsMenu.MENU_COMMON.parameters.items_align = is_vertical ? menu_placeholder.align_vertical : menu_placeholder.align_horizontal;
            SettingsMenu.MENU_COMMON.parameters.anim_selected = "menu_common_item_selected";
            SettingsMenu.MENU_COMMON.parameters.anim_idle = "menu_common_item_idle";
            SettingsMenu.MENU_COMMON.parameters.font_border_size = font_border_size;
            SettingsMenu.MENU_COMMON.items_size = options_count;
            SettingsMenu.MENU_COMMON.items = new MenuManifest.Item[options_count];

            for (int i = 0 ; i < options_count ; i++) {
                SettingsMenu.MENU_COMMON.items[i] = new MenuManifest.Item() {
                    name = null,// unused
                    text = options[i].name,
                    placement = { x = 0f, y = 0f, dimmen = 0f, gap = 0f },// unused
                    anim_selected = null,// unused
                    anim_choosen = null,// unused
                    anim_discarded = null,// unused
                    anim_idle = null,// unused
                    anim_rollback = null,// unused
                    anim_in = null,// unused
                    anim_out = null,// unused
                    hidden = false
                };
            }

            // initialize menu
            Menu menu = new Menu(
                SettingsMenu.MENU_COMMON,
                menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
                menu_placeholder.width, menu_placeholder.height
            );
            menu_placeholder.vertex = menu.GetDrawable();

            // items are not longer required, dispose
            //free(SettingsMenu.MENU_COMMON.items);
            SettingsMenu.MENU_COMMON.items = null;
            SettingsMenu.MENU_COMMON.items_size = 0;

            if (setting_name != null) setting_name.SetTextIntern(true, options[0].name);

            while (true) {
                int selected_option = InCommonMenu(layout, gamepad, menu, options);
                if (selected_option < 0) break;

                if (setting_name != null) setting_name.SetTextIntern(true, options[selected_option].name);

                layout.SetGroupVisibility("holder-boolean", false);
                layout.SetGroupVisibility("holder-numeric", false);
                layout.SetGroupVisibility("holder-list", false);
                layout.SetGroupVisibility("change-value", true);

                if (options[selected_option].is_bool) {
                    options[selected_option].value_bool = InCommonChangeBool(
                        layout, gamepad, options[selected_option].value_bool
                    );
                } else if (options[selected_option].is_int) {
                    options[selected_option].value_int = InCommonChangeNumber(
                        layout, gamepad, options[selected_option].number_min, options[selected_option].number_max, options[selected_option].value_int
                    );
                } else if (options[selected_option].is_list) {
                    options[selected_option].value_list_index = InCommonChangeList(
                        layout, gamepad, options[selected_option].list, options[selected_option].list_size, options[selected_option].value_list_index
                    );
                }

                layout.SetGroupVisibility("change-value", false);
            }

            layout.Destroy();
        }

        private static int InCommonChangeNumber(Layout layout, Gamepad gamepad, int min, int max, int value) {
            value = Math2D.Clamp(value, min, max);
            int orig_value = value;

            layout.SetGroupVisibility("holder-numeric", true);
            TextSprite number = layout.GetTextsprite("number-value");

            number.SetTextFormated("$i", value);
            gamepad.ClearButtons();

            while (true) {
                float elapsed = PVRContext.global_context.WaitReady();

                layout.Animate(elapsed);
                layout.Draw(PVRContext.global_context);

                GamepadButtons buttons = gamepad.HasPressedDelayed(
                    GamepadButtons.A | GamepadButtons.B |
                    GamepadButtons.START | GamepadButtons.BACK |
                    GamepadButtons.AD
                );

                if ((buttons & (GamepadButtons.A | GamepadButtons.START)) != GamepadButtons.NOTHING) {
                    break;
                }
                if ((buttons & (GamepadButtons.B | GamepadButtons.BACK)) != GamepadButtons.NOTHING) {
                    value = orig_value;
                    break;
                }

                if ((buttons & GamepadButtons.AD_UP) != GamepadButtons.NOTHING)
                    value++;
                else if ((buttons & GamepadButtons.AD_DOWN) != GamepadButtons.NOTHING)
                    value--;
                else if ((buttons & GamepadButtons.AD_LEFT) != GamepadButtons.NOTHING)
                    value -= 10;
                else if ((buttons & GamepadButtons.AD_RIGHT) != GamepadButtons.NOTHING)
                    value += 10;
                else
                    continue;

                value = Math2D.Clamp(value, min, max);
                number.SetTextFormated("$i", value);
            }

            return value;
        }

        private static bool InCommonChangeBool(Layout layout, Gamepad gamepad, bool value) {
            bool orig_value = value;

            layout.SetGroupVisibility("holder-boolean", true);
            layout.TriggerAny(value ? "bool-true" : "bool-false");
            gamepad.ClearButtons();

            while (true) {
                float elapsed = PVRContext.global_context.WaitReady();

                layout.Animate(elapsed);
                layout.Draw(PVRContext.global_context);

                GamepadButtons buttons = gamepad.HasPressedDelayed(
                    GamepadButtons.A | GamepadButtons.B |
                    GamepadButtons.START | GamepadButtons.BACK |
                    GamepadButtons.AD
                );

                if (buttons == GamepadButtons.NOTHING) continue;

                if ((buttons & (GamepadButtons.A | GamepadButtons.START)) != GamepadButtons.NOTHING) {
                    break;
                }
                if ((buttons & (GamepadButtons.B | GamepadButtons.BACK)) != GamepadButtons.NOTHING) {
                    value = orig_value;
                    break;
                }

                value = !value;
                layout.TriggerAny(value ? "bool-true" : "bool-false");
            }

            return value;
        }

        private static int InCommonChangeList(Layout layout, Gamepad gamepad, SettingOptionListItem[] list, int list_size, int index) {
            if (list_size < 1) return index;

            layout.SetGroupVisibility("holder-list", true);
            Sprite list_selected_back = layout.GetSprite("list-selected-back");

            bool is_vertical = (bool)layout.GetAttachedValue(
                "menu-selector_isVertical", AttachedValueType.BOOLEAN, false
            );
            float font_size = layout.GetAttachedValueAsFloat(
                "menu-selector_fontSize", 28f
            );
            float items_gap = layout.GetAttachedValueAsFloat(
                "menu-selector_itemGap", 56f
            );

            LayoutPlaceholder menu_placeholder = layout.GetPlaceholder("menu-selector");
            if (menu_placeholder == null) throw new System.Exception("Missing menu-selector placeholder");

            // create selector menu
            SettingsMenu.MENU_COMMON.parameters.font_border_size = 0f;
            SettingsMenu.MENU_COMMON.parameters.is_vertical = is_vertical;
            SettingsMenu.MENU_COMMON.parameters.font_size = font_size;
            SettingsMenu.MENU_COMMON.parameters.items_gap = items_gap;
            SettingsMenu.MENU_COMMON.parameters.items_align = is_vertical ? menu_placeholder.align_vertical : menu_placeholder.align_horizontal;
            SettingsMenu.MENU_COMMON.parameters.anim_selected = "menu_selector_item_selected";
            SettingsMenu.MENU_COMMON.parameters.anim_idle = "menu_selector_item_idle";
            SettingsMenu.MENU_COMMON.items_size = list_size;
            SettingsMenu.MENU_COMMON.items = new MenuManifest.Item[list_size];

            for (int i = 0 ; i < list_size ; i++) {
                SettingsMenu.MENU_COMMON.items[i] = new MenuManifest.Item() {
                    name = null,// unused
                    text = list[i].name,
                    placement = { x = 0f, y = 0f, dimmen = 0f, gap = 0f },// unused
                    anim_selected = null,// unused
                    anim_choosen = null,// unused
                    anim_discarded = null,// unused
                    anim_idle = null,// unused
                    anim_rollback = null,// unused
                    anim_in = null,// unused
                    anim_out = null,// unused
                    hidden = false
                };
            }

            // initialize menu
            Menu menu = new Menu(
                SettingsMenu.MENU_COMMON,
                menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
                menu_placeholder.width, menu_placeholder.height
            );
            menu_placeholder.vertex = menu.GetDrawable();

            // items are not longer required, dispose
            //free(SettingsMenu.MENU_COMMON.items);
            SettingsMenu.MENU_COMMON.items = null;
            SettingsMenu.MENU_COMMON.items_size = 0;

            menu.SelectIndex(index);
            gamepad.ClearButtons();

            bool first_run = true;
            float border_size = font_size * 1.25f;

            while (true) {
                float elapsed = PVRContext.global_context.WaitReady();

                layout.Animate(elapsed);
                layout.Draw(PVRContext.global_context);

                GamepadButtons buttons = gamepad.HasPressedDelayed(
                    GamepadButtons.A | GamepadButtons.B |
                    GamepadButtons.START | GamepadButtons.BACK |
                    GamepadButtons.AD
                );

                if ((buttons & (GamepadButtons.A | GamepadButtons.START)) != GamepadButtons.NOTHING) {
                    index = menu.GetSelectedIndex();
                    Debug.Assert(index >= 0 && index < menu.GetItemsCount());
                    break;
                }
                if ((buttons & (GamepadButtons.B | GamepadButtons.BACK)) != GamepadButtons.NOTHING) {
                    break;
                }

                if ((buttons & GamepadButtons.AD_UP) != GamepadButtons.NOTHING)
                    menu.SelectVertical(-1);
                else if ((buttons & GamepadButtons.AD_DOWN) != GamepadButtons.NOTHING)
                    menu.SelectVertical(1);
                else if ((buttons & GamepadButtons.AD_LEFT) != GamepadButtons.NOTHING)
                    menu.SelectHorizontal(-1);
                else if ((buttons & GamepadButtons.AD_RIGHT) != GamepadButtons.NOTHING)
                    menu.SelectHorizontal(1);
                else if (!first_run)
                    continue;

                if (list_selected_back == null) continue;

                float x, y, width, height;
                if (menu.GetSelectedItemRect(out x, out y, out width, out height)) {
                    x -= border_size;
                    y -= border_size;
                    width += border_size * 2f;
                    height += border_size * 2f;

                    list_selected_back.SetDrawLocation(x, y);
                    list_selected_back.SetDrawSize(width, height);
                    list_selected_back.SetVisible(true);
                } else {
                    list_selected_back.SetVisible(false);
                }

                first_run = false;
            }

            menu.Destroy();
            return index;
        }


        private static void InternalSaveKey(string settings_entry, Keycode keycode) {
            EngineSettings.ini.SetLong(EngineSettings.INI_BINDING_SECTION, settings_entry, keycode.RawValue);
        }

        private static Keycode InternalLoadKey(TextSprite label, string settings_entry, Keycode default_keycode) {
            long raw = EngineSettings.ini.GetLong(EngineSettings.INI_BINDING_SECTION, settings_entry, default_keycode.RawValue);
            Keycode keycode = new Keycode(raw);

            InternalSetKeyInLabel(label, keycode);
            return keycode;
        }

        private static void InternalSetKeyInLabel(TextSprite label, Keycode keycode) {
            string key_name = null;

            if (keycode.IsNone) {
                key_name = KEY_NONE;
            } else if (keycode.scancode != -1 && keycode.key != Keys.SPACE && keycode.key != Keys.TAB) {
                key_name = Glfw.GetKeyName(Keys.UNKNOWN, keycode.scancode);
                if (String.IsNullOrEmpty(key_name) && keycode.key != Keys.UNKNOWN)
                    key_name = keycode.key.ToString();
            } else {
                key_name = keycode.key.ToString();
            }

            if (String.IsNullOrEmpty(key_name)) key_name = KEY_NONE;

            label.SetTextIntern(true, key_name);
        }

        private static GamepadButtons InternalReadKeyboard() {
            double now = Glfw.GetTime();
            if (now < SettingsMenu.next_pressed_timestamp) return GamepadButtons.NOTHING;

            GamepadButtons buttons = GamepadButtons.NOTHING;

            if (Glfw.GetKey(PVRContext.InternalNativeWindow, Keys.LEFT) == InputState.Press)
                buttons |= GamepadButtons.DPAD_LEFT;
            if (Glfw.GetKey(PVRContext.InternalNativeWindow, Keys.DOWN) == InputState.Press)
                buttons |= GamepadButtons.DPAD_DOWN;
            if (Glfw.GetKey(PVRContext.InternalNativeWindow, Keys.UP) == InputState.Press)
                buttons |= GamepadButtons.DPAD_UP;
            if (Glfw.GetKey(PVRContext.InternalNativeWindow, Keys.RIGHT) == InputState.Press)
                buttons |= GamepadButtons.DPAD_RIGHT;
            if (Glfw.GetKey(PVRContext.InternalNativeWindow, Keys.ENTER) == InputState.Press)
                buttons |= GamepadButtons.START;
            if (Glfw.GetKey(PVRContext.InternalNativeWindow, Keys.ESCAPE) == InputState.Press)
                buttons |= GamepadButtons.BACK;

            if (buttons != GamepadButtons.NOTHING)
                SettingsMenu.next_pressed_timestamp = now + SettingsMenu.DELAY_SECONDS;

            return buttons;
        }

        private static void InternalKeyCallback(Window w, Keys key, int scancode, InputState state, ModKeys m) {
            if (previous_key_callback != null) previous_key_callback(w, key, scancode, state, m);

            if (state != InputState.Press) return;
            SettingsMenu.last_detected_keycode = new Keycode(key, scancode);
        }

        private static void InternalRollbackAnim(Sprite back, AnimSprite rollback) {
            if (rollback == null) return;
            rollback.Restart();
            rollback.ForceEnd();
            rollback.UpdateSprite(back, true);
        }

        private static void InternalUpdateHolder(Sprite[] backs, TextSprite[] labels, Keycode[] keycodes, int index, AnimSprite anim, bool rollback, uint color) {
            if (rollback) InternalRollbackAnim(backs[index], anim);
            backs[index].SetVertexColorRGBA8(color);
            InternalSetKeyInLabel(labels[index], keycodes[index]);
        }

        private static void InternalSaveOption(ref SettingOption option, string ini_section) {
            if (option.is_bool) {
                EngineSettings.ini.SetBool(ini_section, option.ini_key, option.value_bool);
            } else if (option.is_int) {
                EngineSettings.ini.SetInt(ini_section, option.ini_key, option.value_int);
            } else if (option.is_list) {
                string value_string = option.list[option.value_list_index].ini_value;
                EngineSettings.ini.SetString(ini_section, option.ini_key, value_string);
            }
        }

        private static void InternalLoadOption(ref SettingOption option, string ini_section) {
            if (option.is_bool) {
                option.value_bool = EngineSettings.ini.GetBool(
                    ini_section, option.ini_key, option.value_bool
                );
            } else if (option.is_int) {
                option.value_int = EngineSettings.ini.GetInt(
                    ini_section, option.ini_key, option.value_int
                );
            } else if (option.is_list) {
                string value_string = option.list[option.value_list_index].ini_value;
                value_string = EngineSettings.ini.GetString(
                    ini_section, option.ini_key, value_string
                );

                for (int i = 0 ; i < option.list_size ; i++) {
                    if (value_string == option.list[i].ini_value) {
                        option.value_list_index = i;
                        break;
                    }
                }
            }
        }


        private struct Keycode {
            public readonly int scancode;
            public readonly Keys key;

            public Keycode(Keys key) {
                this.scancode = -1;
                this.key = key;
                if (key != Keys.UNKNOWN) this.scancode = Glfw.GetKeyScanCode(key);
            }

            public Keycode(Keys key, int scancode) {
                this.scancode = scancode;
                this.key = key;
            }

            public Keycode(long raw) {
                this.scancode = (int)(raw >> 32);
                this.key = (Keys)(uint)(raw & 0xFFFFFFFF);
            }

            public long RawValue {
                get
                {
                    long value = scancode;
                    value <<= 32;
                    value |= (uint)key;
                    return value;
                }
            }

            public bool IsNone {
                get => scancode == -1 && key == Keys.UNKNOWN;
            }

            public static readonly Keycode None = new Keycode(Keys.UNKNOWN);
        }

        private struct SettingOption {
            public string ini_key;
            public string name;
            public string description;

            public bool is_bool;
            public bool is_int;
            public bool is_list;

            public bool value_bool;
            public int value_int;
            public int value_list_index;

            public int number_max;
            public int number_min;

            public SettingOptionListItem[] list;
            public int list_size;
        }

        private struct SettingOptionListItem {
            public string ini_value;
            public string name;
        }

    }
}
