using System;
using Engine.Externals.LuaScriptInterop;
using Engine.Game.Common;
using Engine.Game.Gameplay;
using Engine.Platform;
using Engine.Sound;
using KallistiOS;

namespace Engine.Game {

    public delegate bool ModdingCallbackOption(string option_name);

    public enum ModdingHelperResult {
        CONTINUE,
        BACK,
        CHOOSEN
    }

    public class Modding {
        public readonly WeekScript script;
        public readonly Layout layout;
        public bool has_exit;
        public bool has_halt;
        public bool ui_visible;
        public bool has_funkinsave_changes;
        public Menu native_menu;
        public Menu active_menu;
        public ModdingCallbackOption callback_option;
        public double exit_delay_ms;
        public double custom_menu_gamepad_delay;
        private double custom_menu_active_gamepad_delay;
        private GamepadButtons last_pressed;
        private MessageBox messagebox;

        public Modding(Layout layout, string src_script) {

            this.script = null;
            this.layout = layout;
            this.has_exit = false;
            this.has_halt = false;
            this.ui_visible = true;
            this.has_funkinsave_changes = false;

            this.native_menu = null;
            this.active_menu = null;
            this.callback_option = null;
            this.exit_delay_ms = 0.0;
            this.custom_menu_gamepad_delay = 0.0;
            this.custom_menu_active_gamepad_delay = 0.0;
            this.last_pressed = GamepadButtons.NOTHING;
            this.messagebox = null;

            if (!String.IsNullOrEmpty(src_script) && FS.FileExists(src_script))
                this.script = WeekScript.Init(src_script, this, false);

        }

        public void Destroy() {
            if (this.has_funkinsave_changes) FunkinSave.WriteToVMU();
            if (this.script != null) this.script.Destroy();
            if (this.messagebox != null) this.messagebox.Destroy();
            //free(this);
        }

        public Layout GetLayout() {
            return this.layout;
        }

        public void Exit() {
            this.has_exit = true;
        }

        public void SetHalt(bool halt) {
            this.has_halt = halt;
        }


        public void SetUiVisibility(bool visible) {
            this.ui_visible = !!visible;
        }

        public void UnlockdirectiveCreate(string name, double value) {
            this.has_funkinsave_changes = true;
            FunkinSave.CreateUnlockDirective(name, value);
        }

        public bool UnlockdirectiveHas(string name) {
            return FunkinSave.ContainsUnlockDirective(name);
        }

        public double UnlockdirectiveGet(string name) {
            double value;
            FunkinSave.ReadUnlockDirective(name, out value);
            return value;
        }

        public void UnlockdirectiveRemove(string name) {
            this.has_funkinsave_changes = true;
            FunkinSave.DeleteUnlockDirective(name);
        }


        public bool ChooseNativeMenuOption(string name) {
            if (this.callback_option == null) return false;
            return this.callback_option(name);
        }

        public void SetActiveMenu(Menu menu) {
            this.active_menu = menu;
        }

        public Menu GetActiveMenu() {
            return this.active_menu;
        }

        public Menu GetNativeMenu() {
            return this.native_menu;
        }

        public object SpawnScreen(string layout_src, string script_src, object arg) {
            ScreenMenu screenmenu = ScreenMenu.Init(layout_src, script_src);
            if (screenmenu == null) return null;

            object ret = screenmenu.Display(PVRContext.global_context, arg);
            screenmenu.Destroy();

            return ret;
        }

        public SoundPlayer GetNativeBackgroundMusic() {
            return GameMain.background_menu_music;
        }

        public SoundPlayer ReplaceNativeBackgroundMusic(string music_src) {
            if (GameMain.background_menu_music != null) {
                GameMain.background_menu_music.Destroy();
                GameMain.background_menu_music = null;
            }

            if (!String.IsNullOrEmpty(music_src)) {
                GameMain.background_menu_music = SoundPlayer.Init(music_src);
            }

            return GameMain.background_menu_music;
        }

        public void SetExitDelay(double delay_ms) {
            this.exit_delay_ms = delay_ms;
        }

        public MessageBox GetMessageBox() {
            if (this.messagebox == null) this.messagebox = new MessageBox();
            return this.messagebox;
        }


        public ModdingHelperResult HelperHandleCustomMenu(Gamepad gamepad, float elapsed) {
            Luascript script = this.script.GetLuaScript();
            GamepadButtons pressed = gamepad.GetPressed();

            if (script != null) {
                if (this.last_pressed != pressed) {
                    this.last_pressed = pressed;
                    script.notify_buttons(-1, (uint)pressed);
                }
                script.notify_timer_run(timer.ms_gettime64());
                script.notify_frame(elapsed);
            }

            Menu menu = this.active_menu;
            if (this.has_halt || menu == null) return ModdingHelperResult.CONTINUE;

            if (this.custom_menu_active_gamepad_delay > 0f) {
                this.custom_menu_active_gamepad_delay -= elapsed;
                if (this.custom_menu_active_gamepad_delay > 0f) return ModdingHelperResult.CONTINUE;
            }

            bool go_back = false;
            bool has_selected = false;
            bool has_choosen = false;

            if ((pressed & MainMenu.GAMEPAD_CANCEL) != GamepadButtons.NOTHING)
                go_back = true;
            else if ((pressed & GamepadButtons.DALL_LEFT) != GamepadButtons.NOTHING)
                has_selected = menu.SelectHorizontal(-1);
            else if ((pressed & GamepadButtons.DALL_RIGHT) != GamepadButtons.NOTHING)
                has_selected = menu.SelectHorizontal(1);
            else if ((pressed & GamepadButtons.DALL_UP) != GamepadButtons.NOTHING)
                has_selected = menu.SelectVertical(-1);
            else if ((pressed & GamepadButtons.DALL_DOWN) != GamepadButtons.NOTHING)
                has_selected = menu.SelectVertical(1);
            else if ((pressed & MainMenu.GAMEPAD_OK) != GamepadButtons.NOTHING)
                has_choosen = menu.GetSelectedIndex() >= 0;


            if (go_back) {
                if (script == null) return ModdingHelperResult.BACK;
                if (!script.notify_modding_back()) return ModdingHelperResult.BACK;
                this.custom_menu_active_gamepad_delay = this.custom_menu_gamepad_delay;
            }

            if (has_selected) {
                if (script == null) {
                    int index = menu.GetSelectedIndex();
                    string name = menu.GetSelectedItemName();
                    script.notify_modding_menu_option_selected(menu, index, name);
                }
                this.custom_menu_active_gamepad_delay = this.custom_menu_gamepad_delay;
            }

            if (has_choosen) {
                if (script == null) return ModdingHelperResult.CHOOSEN;

                int index = menu.GetSelectedIndex();
                string name = menu.GetSelectedItemName();

                if (!script.notify_modding_menu_option_choosen(menu, index, name)) {
                    menu.ToggleChoosen(true);
                    return ModdingHelperResult.CHOOSEN;
                }

                this.custom_menu_active_gamepad_delay = this.custom_menu_gamepad_delay;
            }

            return ModdingHelperResult.CONTINUE;
        }


        public bool HelperNotifyOption(bool selected_or_choosen) {
            if (this.active_menu == null || this.script == null) return false;


            int index = this.active_menu.GetSelectedIndex();
            string name = this.active_menu.GetSelectedItemName();
            Luascript script = this.script.GetLuaScript();

            if (selected_or_choosen) {
                script.notify_modding_menu_option_selected(this.active_menu, index, name);
                return false;
            }

            return script.notify_modding_menu_option_choosen(this.active_menu, index, name);
        }

        public bool HelperNotifyBack() {
            if (this.script == null) return false;
            return this.script.GetLuaScript().notify_modding_back();
        }

        public object HelperNotifyExit() {
            if (this.script == null) return null;
            return this.script.GetLuaScript().notify_modding_exit();
        }

        public void HelperNotifyInit(object arg) {
            if (this.script == null) return;
            this.script.GetLuaScript().notify_modding_init(arg);
        }

        public void SetMenuInLayoutPlaceholder(string placeholder_name, Menu menu) {
            if (this.layout == null) return;
            LayoutPlaceholder placeholder = this.layout.GetPlaceholder(placeholder_name);
            if (placeholder == null) return;
            placeholder.vertex = menu == null ? null : menu.GetDrawable();
        }

    }

}


