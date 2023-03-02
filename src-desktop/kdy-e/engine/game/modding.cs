using System;
using Engine.Externals.LuaScriptInterop;
using Engine.Game.Common;
using Engine.Game.Gameplay;
using Engine.Game.Gameplay.Helpers;
using Engine.Platform;
using Engine.Sound;
using KallistiOS;

namespace Engine.Game {

    public delegate bool ModdingCallbackOption(object private_data, string option_name);

    public enum ModdingHelperResult {
        CONTINUE,
        BACK,
        CHOOSEN
    }

    public class Modding {

        public const string NATIVE_MENU_SCREEN = "NATIVE_SCREEN";

        public readonly WeekScript script;
        public readonly Layout layout;
        public bool has_exit;
        public bool has_halt;
        public bool has_funkinsave_changes;
        public Menu native_menu;
        public Menu active_menu;
        public ModdingCallbackOption callback_option;
        public object callback_private_data;
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
            this.has_funkinsave_changes = false;

            this.native_menu = null;
            this.active_menu = null;
            this.callback_option = null;
            this.callback_private_data = null;
            this.exit_delay_ms = 0.0;
            this.custom_menu_gamepad_delay = 200.0;
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

        public bool StorageSet(string week_name, string name, byte[] data, uint data_size) {
            bool ret = FunkinSave.StorageSet(week_name, name, data, data_size);
            if (ret) this.has_funkinsave_changes = true;
            return ret;
        }

        public uint StorageGet(string week_name, string name, out byte[] data) {
            return FunkinSave.StorageGet(week_name, name, out data);
        }



        public bool ChooseNativeMenuOption(string name) {
            if (this.callback_option == null) return false;
            return this.callback_option(this.callback_private_data, name);
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
            Luascript script = this.script?.GetLuaScript();
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
            if (this.has_halt || menu == null || menu == this.native_menu) return ModdingHelperResult.CONTINUE;

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

        public bool HelperNotifyOption2(bool selected_or_choosen, Menu menu, int index, string name) {
            if (this.script == null) return false;

            Luascript script = this.script.GetLuaScript();

            if (selected_or_choosen) {
                script.notify_modding_menu_option_selected(menu, index, name);
                return false;
            }

            return script.notify_modding_menu_option_choosen(menu, index, name);
        }

        public void HelperNotifyFrame(float elapsed, double song_timestamp) {
            if (this.script == null) return;
            if (song_timestamp >= 0.0) this.script.NotifyTimerSong(song_timestamp);
            this.script.NotifyFrame(elapsed);
        }

        public bool HelperNotifyBack() {
            if (this.script == null) return false;
            return this.script.GetLuaScript().notify_modding_back();
        }

        public object HelperNotifyExit() {
            if (this.script == null) return null;
            return this.script.GetLuaScript().notify_modding_exit();
        }

        public void HelperNotifyExit2() {
            object ret = HelperNotifyExit();
            //if (ret is string) free(ret);
        }

        public void HelperNotifyInit(object arg) {
            if (this.script == null) return;
            this.script.GetLuaScript().notify_modding_init(arg);
        }

        public void HelperNotifyEvent(string evt) {
            if (this.script == null) return;
            this.script.GetLuaScript().notify_modding_event(evt);
        }

        public void HelperNotifyHandleCustomOption(string custom_option_name) {
            if (this.script == null) return;
            this.script.GetLuaScript().notify_modding_handle_custom_option(custom_option_name);
        }

        public void SetMenuInLayoutPlaceholder(string placeholder_name, Menu menu) {
            if (this.layout == null) return;
            LayoutPlaceholder placeholder = this.layout.GetPlaceholder(placeholder_name);
            if (placeholder == null) return;
            placeholder.vertex = menu == null ? null : menu.GetDrawable();
        }


        public WeekInfo[] GetLoadedWeeks(out int size) {
            size = Funkin.weeks_array.size;
            return Funkin.weeks_array.array;
        }

        public int LaunchWeek(string week_name, string difficult, bool alt_tracks, string bf, string gf, string gameplay_manifest, int song_idx) {
            WeekInfo gameplay_weekinfo = null;

            for (int i = 0 ; i < Funkin.weeks_array.size ; i++) {
                if (Funkin.weeks_array.array[i].name == week_name) {
                    gameplay_weekinfo = Funkin.weeks_array.array[i];
                }
            }

            if (gameplay_weekinfo == null) return -1;

            FunkinSave.SetLastPlayed(gameplay_weekinfo.name, difficult);

            if (String.IsNullOrEmpty(bf)) bf = FreeplayMenu.HelperGetDefaultCharacterManifest(true);
            if (String.IsNullOrEmpty(gf)) gf = FreeplayMenu.HelperGetDefaultCharacterManifest(false);

            // (C# only) before run check if necessary preload files
            int preload_id = PreloadCache.AddFileList(
                WeekEnumerator.GetAsset(gameplay_weekinfo, PreloadCache.PRELOAD_FILENAME)
            );

            int week_result = Week.Main(
                gameplay_weekinfo,
                alt_tracks,
                difficult,
                bf,
                gf,
                gameplay_manifest,
                song_idx
            );

            // forget all preloaded files
            PreloadCache.ClearById(preload_id);
            return week_result;
        }

        public void LaunchCredits() {
            Credits.Main();
        }

        public bool LaunchStartScreen() {
            return StartScreen.Main();
        }

        public bool LaunchMainMenu() {
            return MainMenu.Main();
        }

        public void LaunchSettings() {
            SettingsMenu.Main();
        }

        public void LaunchFreeplay() {
            FreeplayMenu.Main();
        }

        public int LaunchWeekSelector() {
            return WeekSelector.Main();
        }


    }

}


