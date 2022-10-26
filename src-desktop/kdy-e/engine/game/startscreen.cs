using Engine.Platform;
using Engine.Sound;
using KallistiOS;

namespace Engine.Game {

    public class StartScreen {
        private const string LAYOUT = "/assets/common/image/start-screen/layout.xml";
        private const string LAYOUT_DREAMCAST = "/assets/common/image/start-screen/layout~dreamcast.xml";

        public static bool Main() {
            SoundPlayer soundplayer_confirm = SoundPlayer.Init("/assets/common/sound/confirmMenu.ogg");
            Gamepad maple_pad = new Gamepad(-1);
            Layout layout = Layout.Init(PVRContext.global_context.OutputIsWidescreen() ? LAYOUT : LAYOUT_DREAMCAST);

            float total_elapsed = 0.0f;
            bool enter_pressed = false;
            bool trigger_fade_away = true;
            bool exit_to_bios = false;

            maple_pad.ClearButtons();

            while (true) {
                float elapsed = PVRContext.global_context.WaitReady();

                layout.Animate(elapsed);

                if (!enter_pressed && (maple_pad.HasPressed(GamepadButtons.START | GamepadButtons.A) != GamepadButtons.NOTHING)) {
                    enter_pressed = true;
                    if (soundplayer_confirm != null) soundplayer_confirm.Play();
                    layout.TriggerAny("start_pressed");
                } else if (enter_pressed) {
                    if (total_elapsed >= 2000.0f) {
                        if (trigger_fade_away) {
                            trigger_fade_away = false;
                            layout.TriggerAny("outro");
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

                    total_elapsed += elapsed;
                } else if (maple_pad.HasPressed(GamepadButtons.B | GamepadButtons.BACK) != GamepadButtons.NOTHING) {
                    layout.TriggerAny("outro");
                    enter_pressed = true;
                    exit_to_bios = true;
                }

                PVRContext.global_context.Reset();
                layout.Draw(PVRContext.global_context);
            };

            layout.Destroy();
            maple_pad.Destroy();
            if (soundplayer_confirm != null) soundplayer_confirm.Destroy();

            if (exit_to_bios) {
                // boot dreamcast BIOS menu
                if (GameMain.background_menu_music != null) GameMain.background_menu_music.Stop();
                arch.menu();
                return true;
            }

            return false;
        }

    }

}
