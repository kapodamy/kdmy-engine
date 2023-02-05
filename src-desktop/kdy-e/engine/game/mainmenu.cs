using System;
using System.Reflection;
using Engine.Game.Common;
using Engine.Game.Gameplay.Helpers;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;

namespace Engine.Game {

    public class MainMenu {

        private const int OPTION_SELECTION_DELAY = 200;// milliseconds
        private const string LAYOUT = "/assets/common/image/main-menu/layout.xml";
        private const string LAYOUT_DREAMCAST = "/assets/common/image/main-menu/layout~dreamcast.xml";
        private const string MODDING_SCRIPT = "/assets/data/scripts/mainmenu.lua";
        private const string MODDING_MENU = "/assets/data/menus/mainmenu.json";

        public static readonly MenuManifest MENU_MANIFEST = new MenuManifest() {
            parameters = new MenuManifest.Parameters() {
                suffix_selected = "selected",
                suffix_choosen = "choosen",
                suffix_discarded = "discarded",
                suffix_idle = null,// unused
                suffix_rollback = null,// unused
                suffix_in = null,// unused
                suffix_out = null,// unused

                atlas = "/assets/common/image/main-menu/FNF_main_menu_assets.xml",
                animlist = "/assets/common/image/main-menu/animations.xml",

                anim_selected = null,// unused
                anim_choosen = "choosen",// unused
                anim_discarded = null,// unused
                anim_idle = null,// unused
                anim_rollback = null,// unused
                anim_in = null,// unused
                anim_out = null,// unused

                anim_transition_in_delay = 0.0f,
                anim_transition_out_delay = -100.0f,// negative means start delays from the bottom

                font = null,// unused
                font_glyph_suffix = null,// unused
                font_color_by_difference = false,// unused
                font_size = 0f,// unused
                font_color = 0x00,// unused
                font_border_color = 0x00,// unused
                font_border_size = Single.NaN,// unused

                is_sparse = false,// unused
                is_vertical = false,// readed from the layout
                is_per_page = true,// unused

                items_align = Align.CENTER,// readed from the layout
                items_gap = 0f,// readed from the layout
                items_dimmen = 0f,// readed from the layout
                static_index = 0,// unused
                texture_scale = Single.NaN,// unused
                enable_horizontal_text_correction = true
            },
            items = new MenuManifest.Item[] {
                new MenuManifest.Item() {
                    name= "storymode",
                    text= null,// unused
                    placement= { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
                    anim_selected = null,// unused
                    anim_choosen = null,// unused
                    anim_discarded = null,
                    anim_idle = null,
                    anim_rollback = null,// unused
                    anim_in = "draw_even_index",
                    anim_out = "no_choosen_even_index",
                    hidden = false,
                    description= null// unused
                },
                new MenuManifest.Item() {
                    name = "freeplay",
                    text = null,// unused
                    placement = { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
                    anim_selected = null,// unused
                    anim_choosen = null,// unused
                    anim_discarded = null,
                    anim_idle = null,
                    anim_rollback = null,// unused
                    anim_in = "draw_odd_index",
                    anim_out = "no_choosen_odd_index",
                    hidden = false,
                    description= null// unused
                },
                new MenuManifest.Item() {
                    name = "donate",
                    text = null,// unused
                    placement = { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
                    anim_selected = null,// unused
                    anim_choosen = null,// unused
                    anim_discarded = null,
                    anim_idle = null,
                    anim_rollback = null,// unused
                    anim_in = "draw_even_index",
                    anim_out = "no_choosen_even_index",
                    hidden = false,
                    description= null// unused
                },
                new MenuManifest.Item() {
                    name = "options",
                    text = null,// unused
                    placement = { x= 0f, y= 0f, dimmen= 0f, gap= 0f },// unused
                    anim_selected = null,// unused
                    anim_choosen = null,// unused
                    anim_discarded = null,
                    anim_idle = null,
                    anim_rollback = null,// unused
                    anim_in = "draw_odd_index",
                    anim_out = "no_choosen_odd_index",
                    hidden = false,
                    description= null// unused
                }
            },
            items_size = 4
        };

        public const GamepadButtons GAMEPAD_OK = GamepadButtons.A | GamepadButtons.X | GamepadButtons.START;
        public const GamepadButtons GAMEPAD_CANCEL = GamepadButtons.B | GamepadButtons.Y | GamepadButtons.BACK;
        public const GamepadButtons GAMEPAD_BUTTONS = MainMenu.GAMEPAD_OK | MainMenu.GAMEPAD_CANCEL | GamepadButtons.AD;

        private static int option_index_choosen;

        public static bool Main() {
            Layout layout = Layout.Init(PVRContext.global_context.IsWidescreen() ? LAYOUT : LAYOUT_DREAMCAST);
            if (layout == null) {
                Console.Error.WriteLine("[WARN] mainmenu_main() can not load mainmenu layout");
                return MainMenu.HandleSelectedOption(0);
            }

            LayoutPlaceholder menu_placeholder = layout.GetPlaceholder("menu");

            // default values
            MainMenu.MENU_MANIFEST.parameters.items_align = Align.CENTER;
            MainMenu.MENU_MANIFEST.parameters.is_vertical = true;
            MainMenu.MENU_MANIFEST.parameters.items_dimmen = 60.0f;
            MainMenu.MENU_MANIFEST.parameters.items_gap = 24.0f;
            MainMenu.MENU_MANIFEST.parameters.texture_scale = 0.0f;

            float x = 0f;
            float y = 0f;
            float z = 2f;
            float size_width = PVRContext.global_context.ScreenWidth, size_height = PVRContext.global_context.ScreenHeight;

            layout.GetViewportSize(out size_width, out size_height);

            if (menu_placeholder != null) {
                x = menu_placeholder.x;
                y = menu_placeholder.y;
                z = menu_placeholder.z;
                if (menu_placeholder.width > 0) size_width = menu_placeholder.width;
                if (menu_placeholder.height > 0) size_height = menu_placeholder.height;

                MainMenu.MENU_MANIFEST.parameters.is_vertical = (bool)layout.GetAttachedValue(
                    "menu_isVertical", AttachedValueType.BOOLEAN,
                    MainMenu.MENU_MANIFEST.parameters.is_vertical
                );

                if (MainMenu.MENU_MANIFEST.parameters.is_vertical) {
                    if (menu_placeholder.align_horizontal != Align.NONE)
                        MainMenu.MENU_MANIFEST.parameters.items_align = menu_placeholder.align_horizontal;
                } else {
                    if (menu_placeholder.align_vertical != Align.NONE)
                        MainMenu.MENU_MANIFEST.parameters.items_align = menu_placeholder.align_vertical;
                }

                MainMenu.MENU_MANIFEST.parameters.items_dimmen = (float)(double)layout.GetAttachedValue(
                    "menu_itemDimmen", AttachedValueType.FLOAT,
                    (double)MainMenu.MENU_MANIFEST.parameters.items_dimmen
                );

                MainMenu.MENU_MANIFEST.parameters.texture_scale = (float)(double)layout.GetAttachedValue(
                    "menu_itemScale", AttachedValueType.FLOAT,
                    (double)MainMenu.MENU_MANIFEST.parameters.texture_scale
                );

                MainMenu.MENU_MANIFEST.parameters.items_gap = (float)(double)layout.GetAttachedValue(
                    "menu_itemGap", AttachedValueType.FLOAT,
                    (double)MainMenu.MENU_MANIFEST.parameters.items_gap
                );
            }

            MenuManifest menumanifest = MainMenu.MENU_MANIFEST;
            if (FS.FileExists(MainMenu.MODDING_MENU)) {
                menumanifest = new MenuManifest(MainMenu.MODDING_MENU);
                if (menumanifest == null) throw new Exception("failed to load " + MainMenu.MODDING_MENU);
            }

            Menu menu = new Menu(menumanifest, x, y, z, size_width, size_height);
            menu.TrasitionIn();
            menu.SelectIndex(0);
            GameMain.HelperTriggerActionMenu2(layout, menumanifest, 0, null, true, false);

            if (menu_placeholder != null) {
                menu_placeholder.vertex = menu.GetDrawable();
            } else {
                int index = layout.ExternalCreateGroup(null, 0);
                layout.ExternalVertexCreateEntries(1);
                layout.ExternalVertexSetEntry(0, PVRContextVertex.DRAWABLE, menu.GetDrawable(), index);
            }

            // attach camera animtion (if was defined)
            layout.TriggerCamera("camera_animation");

            SoundPlayer sound_confirm = SoundPlayer.Init("/assets/common/sound/confirmMenu.ogg");
            SoundPlayer sound_scroll = SoundPlayer.Init("/assets/common/sound/scrollMenu.ogg");
            SoundPlayer sound_asterik = SoundPlayer.Init("/assets/common/sound/asterikMenu.ogg");
            SoundPlayer sound_cancel = SoundPlayer.Init("/assets/common/sound/cancelMenu.ogg");

            Gamepad maple_pad = new Gamepad(-1);
            maple_pad.SetButtonsDelay(MainMenu.OPTION_SELECTION_DELAY);
            maple_pad.ClearButtons();

            Modding modding = new Modding(layout, MainMenu.MODDING_SCRIPT);
            modding.native_menu = menu;
            modding.callback_option = MainMenu.HandleOption;
            MainMenu.option_index_choosen = -1;
            modding.HelperNotifyInit(Modding.NATIVE_MENU_SCREEN);

            bool option_was_selected = false;
            int selected_index = -1;
            int last_selected_index = 0;

            while (!modding.has_exit) {
                if (MainMenu.option_index_choosen >= 0) {
                    selected_index = MainMenu.option_index_choosen;
                    menu.SelectIndex(selected_index);
                    break;
                }

                int selection_offset_x = 0;
                int selection_offset_y = 0;
                float elapsed = PVRContext.global_context.WaitReady();
                GamepadButtons buttons = maple_pad.HasPressedDelayed(MainMenu.GAMEPAD_BUTTONS);

                PVRContext.global_context.Reset();

                ModdingHelperResult res = modding.HelperHandleCustomMenu(maple_pad, elapsed);
                if (res != ModdingHelperResult.CONTINUE) break;

                layout.Animate(elapsed);
                layout.Draw(PVRContext.global_context);

                if (modding.has_halt || modding.active_menu != menu) continue;

                if ((buttons & MainMenu.GAMEPAD_OK).Bool()) {
                    selected_index = menu.GetSelectedIndex();
                    if (selected_index >= 0 && selected_index < menu.GetItemsCount()) {
                        if (modding.HelperNotifyOption(false)) continue;
                        option_was_selected = true;
                        break;
                    }
                } else if ((buttons & MainMenu.GAMEPAD_CANCEL).Bool() && !modding.HelperNotifyBack())
                    break;
                else if ((buttons & GamepadButtons.AD_DOWN).Bool())
                    selection_offset_y++;
                else if ((buttons & GamepadButtons.AD_UP).Bool())
                    selection_offset_y--;
                else if ((buttons & GamepadButtons.AD_LEFT).Bool())
                    selection_offset_x--;
                else if ((buttons & GamepadButtons.AD_RIGHT).Bool())
                    selection_offset_x++;

                if (selection_offset_x == 0 && selection_offset_y == 0) continue;

                bool success = false;

                if (selection_offset_x != 0 && menu.SelectHorizontal(selection_offset_x)) success = true;
                if (selection_offset_y != 0 && menu.SelectVertical(selection_offset_y)) success = true;

                if (success) {
                    if (sound_asterik != null) sound_asterik.Stop();
                    if (sound_scroll != null) sound_scroll.Replay();

                    GameMain.HelperTriggerActionMenu2(layout, menumanifest, last_selected_index, null, false, false);
                    last_selected_index = menu.GetSelectedIndex();
                    GameMain.HelperTriggerActionMenu2(layout, menumanifest, last_selected_index, null, true, false);
                    modding.HelperNotifyOption(true);
                } else {
                    if (sound_scroll != null) sound_scroll.Stop();
                    if (sound_asterik != null) sound_asterik.Replay();
                }
            }

            // apply choosen transition
            if (sound_scroll != null) sound_scroll.Stop();
            SoundPlayer target_sound = option_was_selected ? sound_confirm : sound_cancel;
            if (target_sound != null) target_sound.Replay();
            layout.TriggerAny(option_was_selected ? "option_selected" : "return");
            if (option_was_selected) menu.ToggleChoosen(true);
            else menu.TrasitionOut();

            if (option_was_selected) {
                GameMain.HelperTriggerActionMenu2(layout, menumanifest, selected_index, null, false, true);
            }

            float total_elapsed = 0f;

            // animate selected option
            if (option_was_selected) {
                while (total_elapsed < 1200f) {
                    float elapsed = PVRContext.global_context.WaitReady();
                    total_elapsed += elapsed;

                    layout.Animate(elapsed);
                    layout.Draw(PVRContext.global_context);
                }

                total_elapsed = 0f;
            }

            // trigger outro transition
            layout.TriggerAction(null, "outro");
            menu.TrasitionOut();

            while (total_elapsed < 1200f) {
                float elapsed = PVRContext.global_context.WaitReady();

                layout.Animate(elapsed);
                layout.Draw(PVRContext.global_context);

                if (layout.AnimationIsCompleted("transition_effect") > 0)
                    break;

                total_elapsed += elapsed;
            }

            selected_index = -1;
            string selected_option_name = menu.GetSelectedItemName();
            for (int i = 0 ; i < MainMenu.MENU_MANIFEST.items_size ; i++) {
                if (MainMenu.MENU_MANIFEST.items[i].name == selected_option_name) {
                    selected_index = i;
                    break;
                }
            }

            modding.HelperNotifyExit2();

            menu.Destroy();
            maple_pad.Destroy();
            if (sound_confirm != null) sound_confirm.Destroy();
            if (sound_scroll != null) sound_scroll.Destroy();
            if (sound_cancel != null) sound_cancel.Destroy();
            if (sound_asterik != null) sound_asterik.Destroy();
            layout.Destroy();
            modding.Destroy();

            if (menumanifest != MainMenu.MENU_MANIFEST) menumanifest.Destroy();

            // if no option was selected, jump to the start screen
            if (!option_was_selected) return false;

            // handle user action
            return MainMenu.HandleSelectedOption(selected_index);
        }

        public static void ShowDonate() {
            Layout layout = Layout.Init("/assets/common/credits/donation.xml");
            if (layout == null) return;

            bool pause_background_menu_music = (bool)layout.GetAttachedValue(
                 "pause_background_menu_music", AttachedValueType.BOOLEAN, false
            );
            double timeout = (double)layout.GetAttachedValue(
                 "timeout", AttachedValueType.FLOAT, -1.0
            );

            if (pause_background_menu_music) GameMain.background_menu_music.Pause();

            Gamepad gamepad = new Gamepad(-1);
            gamepad.SetButtonsDelay(250);
            gamepad.ClearButtons();

            double progress = 0.0;
            if (timeout < 0.0) timeout = Double.PositiveInfinity;

            while (progress < timeout) {
                float elapsed = PVRContext.global_context.WaitReady();
                PVRContext.global_context.Reset();

                progress += elapsed;

                layout.Animate(elapsed);
                layout.Draw(PVRContext.global_context);

                if (gamepad.HasPressedDelayed(Credits.BUTTONS).Bool()) break;
            }

            layout.Destroy();
            gamepad.ClearButtons();
            gamepad.Destroy();

            if (pause_background_menu_music) GameMain.background_menu_music.Play();
        }

        public static bool HandleSelectedOption(int selected_index) {
            //
            // we handle all menu options here
            // all functions called here are async in JS
            //
            // return false to go back to the start-screen, otherwise, reload the main-menu
            //
            switch (selected_index) {
                case -1:
                    // from custom menu
                    return true;
                case 0:// storymode
                    while (WeekSelector.Main() > 0) { }
                    return true;// main-menu
                case 1:
                    FreeplayMenu.Main();
                    return true;// main-menu
                case 2:
                    MainMenu.ShowDonate();
                    return true;// main-menu
                case 3:
                    SettingsMenu.Main();
                    return true;// main-menu
                default:
                    Console.Error.WriteLine("[LOG] unimplemented selected option:" + selected_index);
                    return false;
            }
        }

        public static bool HandleOption(string option_name) {
            if (option_name == null) return false;
            for (int i = 0 ; i < MainMenu.MENU_MANIFEST.items_size ; i++) {
                if (option_name == MainMenu.MENU_MANIFEST.items[i].name) {
                    MainMenu.option_index_choosen = i;
                    return true;
                }
            }
            return false;
        }
    }

}
