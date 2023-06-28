using System;
using Engine.Externals.GLFW;
using Engine.Platform;
using Engine.Sound;
using KallistiOS;

namespace Engine.Game;

public class StartScreen {
    private const string LAYOUT = "/assets/common/image/start-screen/layout.xml";
    private const string LAYOUT_DREAMCAST = "/assets/common/image/start-screen/layout~dreamcast.xml";
    private const string MODDING_SCRIPT = "/assets/common/data/scripts/startscreen.lua";


    public static bool Main() {
        Layout layout = Layout.Init(PVRContext.global_context.IsWidescreen() ? LAYOUT : LAYOUT_DREAMCAST);
        if (layout == null) {
            Logger.Warn("startscreen_main() can not load mainmenu layout");
            return false;
        }

        float delay_after_start = layout.GetAttachedValueAsFloat("delay_after_start", 2000f);
        SoundPlayer soundplayer_confirm = SoundPlayer.Init("/assets/common/sound/confirmMenu.ogg");
        Gamepad maple_pad = new Gamepad(-1);
        Modding modding = new Modding(layout, StartScreen.MODDING_SCRIPT);

        float total_elapsed = 0.0f;
        bool enter_pressed = false;
        bool trigger_fade_away = true;
        bool exit_to_bios = false;

        ModdingHelper moddinghelper = new ModdingHelper { start_pressed = false, exit_to_bios = false };
        modding.callback_private_data = moddinghelper;
        modding.callback_option = StartScreen.InternalHandleModdingOption;

        maple_pad.ClearButtons();
        maple_pad.SetButtonsDelay(200);
        modding.HelperNotifyInit(Modding.NATIVE_MENU_SCREEN);
        modding.HelperNotifyEvent("start_screen");

        while (true) {
            float elapsed = PVRContext.global_context.WaitReady();
            layout.Animate(elapsed);

            ModdingHelperResult res = modding.HelperHandleCustomMenu(maple_pad, elapsed);
            if (modding.has_exit || res != ModdingHelperResult.CONTINUE) break;

            GamepadButtons pressed = maple_pad.HasPressedDelayed(MainMenu.GAMEPAD_BUTTONS);
            if (modding.has_halt) pressed = GamepadButtons.NOTHING;

            if ((pressed & MainMenu.GAMEPAD_OK).Bool() || moddinghelper.start_pressed) {
                if (enter_pressed) {
                    break;
                } else {
                    enter_pressed = true;
                    moddinghelper.start_pressed = false;
                    if (soundplayer_confirm != null) soundplayer_confirm.Play();
                    layout.TriggerAny("start_pressed");
                    modding.HelperNotifyEvent("start_pressed");
                }
            } else if (enter_pressed) {
                if (total_elapsed >= delay_after_start) {
                    if (trigger_fade_away && !exit_to_bios) {
                        trigger_fade_away = false;
                        layout.TriggerAny("outro");
                        modding.HelperNotifyEvent("outro");
                    } else {
                        int state = layout.AnimationIsCompleted("transition-effect");

                        if (state == -1) break;// infinite looped animation

                        // check if:
                        //          the animation is completed (1)
                        //          there no animation on the sprite (2)
                        //          there no sprite in the layout with that name (3)
                        else if (state != 0) break;
                    }
                }

                if (!modding.has_halt) total_elapsed += elapsed;
            } else if (((pressed & MainMenu.GAMEPAD_CANCEL).Bool() && !modding.HelperNotifyBack()) || moddinghelper.exit_to_bios) {
                layout.TriggerAny("outro");
                modding.HelperNotifyEvent("exit_to_dcbios");
                if (GameMain.background_menu_music != null) GameMain.background_menu_music.Fade(false, 1000f);
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
            Glfw.HideWindow(PVRContext.InternalNativeWindow);
            arch.menu();
            return true;
        }

        return false;
    }

    private static bool InternalHandleModdingOption(object obj, string option_name) {
        ModdingHelper moddinghelper = (ModdingHelper)obj;
        switch (option_name) {
            case null:
            case "exit-to-bios":
                moddinghelper.start_pressed = true;
                break;
            case "start-pressed":
                moddinghelper.exit_to_bios = true;
                break;
            default:
                return false;
        }
        return true;
    }


    private class ModdingHelper {
        public bool start_pressed;
        public bool exit_to_bios;
    }
}
