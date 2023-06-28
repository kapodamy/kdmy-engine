using System;
using Engine.Externals.LuaScriptInterop;
using Engine.Game.Common;
using Engine.Platform;
using KallistiOS;

namespace Engine.Game;

public class ScreenMenu {
    private const float BUTTONS_DELAY = 200;
    private const GamepadButtons BACK_BUTTONS = GamepadButtons.BACK | GamepadButtons.B;

    private Modding modding;
    private Layout layout;


    private ScreenMenu() { }


    public static ScreenMenu Init(string layout_src, string script_src) {
        Layout layout = Layout.Init(layout_src);
        if (layout == null) {
            Logger.Error($"screenmenu_init() failed to load layout: {layout_src}");
            return null;
        }

        Modding modding = new Modding(layout, script_src);
        modding.exit_delay_ms = layout.GetAttachedValueAsFloat("exit_delay", 0f);

        return new ScreenMenu() { modding = modding, layout = layout };
    }

    public void Destroy() {
        this.layout.Destroy();
        this.modding.Destroy();
        //free(this);
    }

    public object Display(PVRContext pvrctx, object script_arg) {
        Luascript script = this.modding.script?.GetLuaScript();
        Layout layout = this.layout;

        if (script != null) {
            script.notify_modding_init(script_arg);
            if (this.modding.has_exit) return script.notify_modding_exit();
        }

        float active_gamepad_delay = 0f;
        GamepadButtons last_pressed = GamepadButtons.NOTHING;
        Gamepad gamepad = new Gamepad(-1);
        gamepad.ClearButtons();

        while (!this.modding.has_exit) {
            float elapsed = pvrctx.WaitReady();
            pvrctx.Reset();

            GamepadButtons pressed = gamepad.GetPressed();
            bool back_pressed = (pressed & ScreenMenu.BACK_BUTTONS).Bool();

            // ignore back buttons if halt flag not is signalated and call "f_modding_back" instead
            if (back_pressed && !this.modding.has_halt) pressed &= ~ScreenMenu.BACK_BUTTONS;

            if (script != null) {
                if (last_pressed != pressed) {
                    last_pressed = pressed;
                    script.notify_buttons(-1, (uint)pressed);
                }
                script.notify_timer_run(timer.ms_gettime64());
                script.notify_frame(elapsed);
            }

            if (this.modding.has_halt) goto L_layout;

            if (active_gamepad_delay > 0f) {
                active_gamepad_delay -= elapsed;
                if (active_gamepad_delay > 0f) goto L_layout;
            }

            Menu menu = this.modding.active_menu;

            bool go_back = false;
            bool has_selected = false;
            bool has_choosen = false;

            if (back_pressed)
                go_back = true;
            else if (menu == null)
                goto L_layout;
            else if ((pressed & GamepadButtons.DALL_LEFT).Bool())
                has_selected = menu.SelectHorizontal(-1);
            else if ((pressed & GamepadButtons.DALL_RIGHT).Bool())
                has_selected = menu.SelectHorizontal(1);
            else if ((pressed & GamepadButtons.DALL_UP).Bool())
                has_selected = menu.SelectVertical(-1);
            else if ((pressed & GamepadButtons.DALL_DOWN).Bool())
                has_selected = menu.SelectVertical(1);
            else if ((pressed & MainMenu.GAMEPAD_OK).Bool())
                has_choosen = menu.GetSelectedIndex() >= 0;


            if (go_back) {
                if (script == null) break;
                if (!script.notify_modding_back()) break;
                active_gamepad_delay = ScreenMenu.BUTTONS_DELAY;
            }

            if (has_selected) {
                if (script == null) {
                    int index = menu.GetSelectedIndex();
                    string name = menu.GetSelectedItemName();
                    script.notify_modding_menu_option_selected(menu, index, name);
                }
                active_gamepad_delay = ScreenMenu.BUTTONS_DELAY;
            }

            if (has_choosen) {
                if (script == null) break;

                int index = menu.GetSelectedIndex();
                string name = menu.GetSelectedItemName();

                if (!script.notify_modding_menu_option_choosen(menu, index, name)) {
                    menu.ToggleChoosen(true);
                    break;
                }

                active_gamepad_delay = ScreenMenu.BUTTONS_DELAY;
            }

L_layout:
            layout.Animate(elapsed);
            layout.Draw(pvrctx);
        }

        // if there no script ¿exit_value should be null or pick the menu selectd option name?
        object exit_value = null;
        if (script != null) exit_value = script.notify_modding_exit();

        // delay exit (if applicable)
        double exit_delay = this.modding.exit_delay_ms;
        while (exit_delay > 0) {
            float elapsed = pvrctx.WaitReady();

            exit_delay -= elapsed;
            pvrctx.Reset();

            if (script != null) {
                script.notify_timer_run(timer.ms_gettime64());
                script.notify_frame(elapsed);
            }

            layout.Animate(elapsed);
            layout.Draw(pvrctx);
        }

        // flush framebuffer again with last drawn frame
        pvrctx.WaitReady();

        // do antibounce before return (just in case)
        Gamepad.ClearAllGamepads();

        gamepad.Destroy();
        return exit_value;
    }

}
