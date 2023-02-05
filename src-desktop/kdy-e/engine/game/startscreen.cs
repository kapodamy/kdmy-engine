using System;
using Engine.Externals.GLFW;
using Engine.Platform;
using Engine.Sound;
using KallistiOS;

namespace Engine.Game {

    public class StartScreen {
        private const string LAYOUT = "/assets/common/image/start-screen/layout.xml";
        private const string LAYOUT_DREAMCAST = "/assets/common/image/start-screen/layout~dreamcast.xml";
        private const string MODDING_SCRIPT = "/assets/data/scripts/startscreen.lua";

        private static bool has_press_start;

        public static bool Main() {
            Layout layout = Layout.Init(PVRContext.global_context.IsWidescreen() ? LAYOUT : LAYOUT_DREAMCAST);
            if (layout == null) {
                Console.Error.WriteLine("[WARN] startscreen_main() can not load mainmenu layout");
                return false;
            }

            SoundPlayer soundplayer_confirm = SoundPlayer.Init("/assets/common/sound/confirmMenu.ogg");
            Gamepad maple_pad = new Gamepad(-1);
            Modding modding = new Modding(layout, StartScreen.MODDING_SCRIPT);

            float total_elapsed = 0.0f;
            bool enter_pressed = false;
            bool trigger_fade_away = true;
            bool exit_to_bios = false;

            StartScreen.has_press_start = false;
            modding.callback_option = StartScreen.InternalHandleOption;

            maple_pad.ClearButtons();
            maple_pad.SetButtonsDelay(200);
            modding.HelperNotifyInit(Modding.NATIVE_MENU_SCREEN);
            modding.HelperNotifyModdingEvent("start-screen");

            while (true) {
                float elapsed = PVRContext.global_context.WaitReady();

                layout.Animate(elapsed);

                ModdingHelperResult res = modding.HelperHandleCustomMenu(maple_pad, elapsed);
                if (modding.has_exit || res != ModdingHelperResult.CONTINUE) break;

                GamepadButtons pressed = maple_pad.HasPressedDelayed(MainMenu.GAMEPAD_BUTTONS);
                if (modding.has_halt) pressed = GamepadButtons.NOTHING;

                if (!enter_pressed && ((pressed & MainMenu.GAMEPAD_OK).Bool() || StartScreen.has_press_start)) {
                    enter_pressed = true;
                    if (soundplayer_confirm != null) soundplayer_confirm.Play();
                    layout.TriggerAny("start_pressed");
                    modding.HelperNotifyModdingEvent("start_pressed");
                } else if (enter_pressed) {
                    if (total_elapsed >= 2000.0f) {
                        if (trigger_fade_away) {
                            trigger_fade_away = false;
                            layout.TriggerAny("outro");
                            modding.HelperNotifyModdingEvent("outro");
                        } else {
                            int state = layout.AnimationIsCompleted("transition-intro-outro-effect");

                            if (state == -1) break;// infinite looped animation

                            // check if:
                            //          the animation is completed (1)
                            //          there no animation on the sprite (2)
                            //          there no sprite in the layout with that name (3)
                            else if (state != 0) break;
                        }
                    }

                    if (!modding.has_halt) total_elapsed += elapsed;
                } else if ((pressed & MainMenu.GAMEPAD_CANCEL).Bool() && !modding.HelperNotifyBack()) {
                    modding.HelperNotifyModdingEvent("back_to_dcbios");
                    layout.TriggerAny("outro");
                    enter_pressed = true;
                    exit_to_bios = true;
                }

                PVRContext.global_context.Reset();
                layout.Draw(PVRContext.global_context);
            };

            modding.HelperNotifyExit2();

            modding.Destroy();
            layout.Destroy();
            maple_pad.Destroy();
            if (soundplayer_confirm != null) soundplayer_confirm.Destroy();

            if (exit_to_bios) {
                // boot dreamcast BIOS menu
                if (GameMain.background_menu_music != null) GameMain.background_menu_music.Stop();
                Glfw.SwapBuffers(PVRContext.InternalNativeWindow);
                arch.menu();
                return true;
            }

            return false;
        }

        private static bool InternalHandleOption(string option_name) {
            bool has_press_start = option_name == "start_pressed";
            StartScreen.has_press_start = has_press_start;
            return has_press_start;
        }

    }

}
