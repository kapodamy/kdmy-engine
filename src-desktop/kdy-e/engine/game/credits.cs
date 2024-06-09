using Engine.Platform;
using Engine.Sound;
using Engine.Utils;
using KallistiOS;

namespace Engine.Game;

public class Credits {
    private const string LAYOUT = "/assets/common/credits/layout.xml";
    private const string LAYOUT_DREAMCAST = "/assets/common/credits/layout~dreamcast.xml";

    internal const GamepadButtons BUTTONS = GamepadButtons.START | GamepadButtons.A | GamepadButtons.B | GamepadButtons.X | GamepadButtons.Y | GamepadButtons.BACK;

    public static void Main() {
        GamepadButtons buttons = GamepadButtons.NOTHING;
        Layout layout = Layout.Init(PVRContext.global_context.IsWidescreen() ? LAYOUT : LAYOUT_DREAMCAST);
        if (layout == null) return;

        Modding moddingcontext = new Modding(layout, "/assets/common/credits/credits.lua");
        Gamepad gamepad = new Gamepad(-1);
        double end_timestamp_base = -1.0;
        double end_timestamp_before_shoot = -1.0;
        double end_timestamp_shoot = -1.0;
        double end_timestamp_ending = -1.0;
        double fade_duration = 0.0;
        bool is_scripted = true;
        int state = 0;

        if (moddingcontext.script == null) {
            end_timestamp_base = layout.GetAttachedValueAsFloat("base_duration", 25000.0f);
            end_timestamp_before_shoot = layout.GetAttachedValueAsFloat("before_shoot_duration", 5000.0f);
            end_timestamp_shoot = layout.GetAttachedValueAsFloat("shoot_duration", 3000.0f);
            end_timestamp_ending = layout.GetAttachedValueAsFloat("after_shoot_duration", 5000.0f);

            double timestamp = timer.ms_gettime64();
            is_scripted = false;
            fade_duration = end_timestamp_ending;

            end_timestamp_base += timestamp;
            end_timestamp_before_shoot += end_timestamp_base;
            end_timestamp_shoot += end_timestamp_before_shoot;
            end_timestamp_ending += end_timestamp_shoot;
        }

        gamepad.ClearButtons();

        while (!moddingcontext.has_exit) {
            float elapsed = PVRContext.global_context.WaitReady();
            PVRContext.global_context.Reset();

            if (gamepad.GetManagedPresses(true, ref buttons)) {
                if (moddingcontext.script != null) moddingcontext.script.NotifyButtons(-1, buttons);
            }
            if (moddingcontext.script != null) moddingcontext.script.NotifyFrame(elapsed);

            layout.Animate(elapsed);
            layout.Draw(PVRContext.global_context);

            if (is_scripted) continue;

            double timestamp = timer.ms_gettime64();

            if (state != 3 && gamepad.HasPressed(Credits.BUTTONS).Bool()) {
                if (state < 3) {
                    timestamp = Math2D.MAX_INT32;
                    state = 3;
                    gamepad.ClearButtons();
                } else {
                    break;
                }
            }

            switch (state) {
                case 0:
                    if (timestamp >= end_timestamp_base) {
                        layout.TriggerAny("next");
                        state = 1;
                    }
                    break;
                case 1:
                    if (timestamp > end_timestamp_before_shoot) {
                        layout.TriggerAny("before_shoot");
                        state = 2;
                    }
                    break;
                case 2:
                    if (timestamp > end_timestamp_shoot) {
                        layout.TriggerAny("shoot");
                        state = 3;
                    }
                    break;
                case 3:
                    if (timestamp > end_timestamp_ending) {
                        layout.TriggerAny("outro");
                        SoundPlayer soundplayer = layout.GetSoundplayer("bg-music");
                        if (soundplayer != null) soundplayer.Fade(false, (float)fade_duration);
                        state = 4;
                    }
                    goto case 4;
                case 4:
                    if (layout.AnimationIsCompleted("transition_effect") > 0) {
                        break;
                    }
                    break;
            }
        }

        moddingcontext.Destroy();
        gamepad.Destroy();
        layout.Destroy();
    }

}
