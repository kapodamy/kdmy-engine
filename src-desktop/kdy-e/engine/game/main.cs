using System;
using CsharpWrapper;
using Engine.Externals.FFGraphInterop;
using Engine.Externals.FontAtlasInterop;
using Engine.Externals.LuaInterop;
using Engine.Externals.SoundBridge;
using Engine.Font;
using Engine.Game.Common;
using Engine.Game.Gameplay.Helpers;
using Engine.Image;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;
using KallistiOS.THD;
using KallistiOS.UTSNAME;

namespace Engine.Game;

public static class GameMain {

    // this file contains all shared data across the game
    public const string ENGINE_NAME = "kdmy-engine";
    public const string ENGINE_VERSION = "0.60";

    /**
     * The background music used in all menus, inherited from introscreen
     */
    public static SoundPlayer background_menu_music = null;

    /**
     * Menus style (customized by a week) used across the game
     */
    public static WeekInfo custom_style_from_week = null;


    public static int Main(int argc, string[] argv) {
        // vital parts
        FS.Init();
        PVRContext.Init();
        MasterVolume.Init();

        utsname osinfo;
        UTSNAME.uname(out osinfo);

        Logger.Info("SoundBridge:" + SoundBridge.GetRuntimeInfo());
        Logger.Info("FontAtlas: FreeType V" + FontAtlas.GetVersion());
        Logger.Info("LuaScript: " + ManagedLuaState.GetVersion());
        Logger.Info("FFGraph: " + FFGraph.GetRuntimeInfo() ?? "<not loaded>");
        Console.WriteLine();
        Logger.Log($"{ENGINE_NAME} {ENGINE_VERSION}");
        Logger.Log($"Console: {osinfo.machine}");
        Logger.Log($"OS: {osinfo.version}\r\n");

        // (JS & CS only) preload fonts
        new FontHolder("/assets/common/font/vcr.ttf", -1, null, false);
        //new FontHolder("/assets/common/font/pixel.otf", -1, null, false);
        new FontHolder("/assets/common/font/alphabet.xml", -1, null, false);

        WeekEnumerator.Enumerate();

        if (Funkin.weeks_array.size < 1) Logger.Warn("main() no weeks detected");

        // load savedata
        FunkinSave.PickFirstAvailableVMU();
        int load_result = SaveManager.ShouldShow(false);
        if (load_result != 0) {
            SaveManager savemanager = new SaveManager(false, load_result);
            savemanager.Show();
            savemanager.Destroy();
        }

        // choose a random menu style
        bool[] visited = EngineUtils.CreateArray<bool>(Funkin.weeks_array.size);
        bool try_choose_last_played = Math2D.RandomFloat() <= 0.15f;
        int visited_count = 0;
        for (int i = 0 ; i < Funkin.weeks_array.size ; i++) visited[i] = false;

        while (visited_count < Funkin.weeks_array.size) {
            WeekInfo weekinfo;

            if (try_choose_last_played) {
                try_choose_last_played = false;
                weekinfo = null;
                string last_played = EngineSettings.style_from_week_name ?? FunkinSave.GetLastPlayedWeek();

                if (last_played != null) {
                    for (int i = 0 ; i < Funkin.weeks_array.size ; i++) {
                        if (Funkin.weeks_array.array[i].name == last_played) {
                            visited[i] = true;
                            visited_count++;
                            weekinfo = Funkin.weeks_array.array[i];
                            break;
                        }
                    }
                }
                if (weekinfo == null) continue;
            } else {
                int random_index = Math2D.RandomInt(0, Funkin.weeks_array.size);
                if (visited[random_index]) continue;
                visited[random_index] = true;
                visited_count++;
                weekinfo = Funkin.weeks_array.array[random_index];
            }

            if (StringUtils.IsEmpty(weekinfo.custom_folder) && !weekinfo.has_greetings) continue;

            if (FunkinSave.ContainsUnlockDirective(weekinfo.unlock_directive)) {
                custom_style_from_week = weekinfo;

                // override commons files (if possible)
                if (StringUtils.IsNotEmpty(weekinfo.custom_folder)) {
                    FS.OverrideCommonFolder(weekinfo.custom_folder);
                }
                break;
            }
        }

        GameMain.background_menu_music = SoundPlayer.Init(Funkin.BACKGROUND_MUSIC);
        if (GameMain.background_menu_music != null) background_menu_music.LoopEnable(true);

        // the following code is for quick debugging only
        /*
        FreeplayMenu.Main();
		Credits.Main();
        SettingsMenu.Main();
        WeekSelector.Main();
        Engine.Game.Gameplay.Week.Main(
            Funkin.weeks_array.array[0],
            false,
            "NORMAL",
            "/assets/common/data/BOYFRIEND.json",
            "/assets/common/data/GIRLFRIEND.json",
            null,
            -1,
            null
        );
        Logger.Log("week_main() execution done, engine is terminated");
        return 0;*/

        IntroScreen.Main();

        while (true) {
            if (StartScreen.Main()) break;
            while (MainMenu.Main()) ;
            if (background_menu_music != null) background_menu_music.Replay();
        }

        return 1;
    }

    public static void HelperDrawLoadingScreen() {
        PVRContext pvr_context = PVRContext.global_context;

        if (!EngineSettings.show_loading_screen) return;

        if (!FS.FileExists(Funkin.LOADING_SCREEN_TEXTURE)) {
            return;
        }

        Texture texture = Texture.Init(Funkin.LOADING_SCREEN_TEXTURE);
        Sprite sprite = Sprite.Init(texture);
        sprite.SetDrawLocation(0f, 0f);
        sprite.SetDrawSizeFromSourceSize();

        // funkay texture aspect ratio is not 16:9 or 4:3
        ImgUtils.CalcResizeSprite(sprite, pvr_context.ScreenWidth, pvr_context.ScreenHeight, true, true);

        pvr_context.Reset();
        sprite.Draw(pvr_context);
        pvr_context.WaitReady();

        sprite.DestroyFull();
    }

    public static void HelperTriggerActionMenu(Layout layout, string prefix, string name, bool selected, bool choosen) {
        if (layout == null || StringUtils.IsEmpty(name)) return;

        string suffix;

        if (choosen)
            suffix = "choosen";
        else if (selected)
            suffix = "selected";
        else
            suffix = "unselected";

        string target;

        if (StringUtils.IsNotEmpty(prefix))
            target = String.Concat(prefix, "-", name, "-", suffix);
        else
            target = String.Concat(name, "-", suffix);

        layout.TriggerAny(target);
        //free(target);
    }

    public static void HelperTriggerActionMenu2(Layout layout, MenuManifest menu_manifest, int index, string prefix, bool selected, bool choosen) {
        if (index < 0 || index >= menu_manifest.items_size) return;
        string name = menu_manifest.items[index].name ?? menu_manifest.items[index].text;
        GameMain.HelperTriggerActionMenu(layout, prefix, name, selected, choosen);
    }

    public static R SpawnCoroutine<R, T>(Layout background_layout, AsyncFunctionWithReturn<R, T> function_routine, T argument_routine) {
        kthread_t thread = THDHelperSpawn(false, (object arg) => function_routine((T)arg), argument_routine);
        R ret;

        if (background_layout != null) {
            do {
                float elapsed = PVRContext.global_context.WaitReady();
                PVRContext.global_context.Reset();

                background_layout.Animate(elapsed);
                background_layout.Draw(PVRContext.global_context);
            } while (thread.state != STATE.FINISHED);

            // aquire the return value and destroy the thread struct
            ret = (R)thread.rv;
            thd.destroy(thread);
        } else {
            // no layout specified, wait for thread end
            object tmp = null;
            thd.join(thread, ref tmp);
            ret = (R)tmp;
        }

        return ret;
    }

    public delegate object AsyncFunction<T>(T param) where T : class;
    public delegate R AsyncFunctionWithReturn<R, T>(T param);

    /**
     * Create a new thread.
     * This function creates a new kernel thread with default parameters to run the given routine.
     * The thread will terminate and clean up resources when the routine completes
     * This function warps {@link thd_create} to allow multi-thread filesystem support (fs_* engine functions)
     * @param {boolean} detached true to make the thread joinable, otherwise, false
     * @param {function} routine The function to call in the new thread.
     * @param {object} param A parameter to pass to the function called.
     * @returns {kthread_t} the created thread
     */
    public static kthread_t THDHelperSpawn<T>(bool detached, AsyncFunction<T> routine, T param) where T : class {
        // in C, "init_data" this an allocated struct
        InitData<T> init_data = new InitData<T>() { routine = routine, @params = param };
        kthread_t thread = thd.create(detached, arg => GameMain.THDHelperSpawnWrapper<T>(init_data), init_data);

        if (thread == null) {
            throw new Exception("THDHelperSpawn() call failed to thd_create()");
        }

        thd.pass();
        return thread;
    }

    private static object THDHelperSpawnWrapper<T>(object arg) where T : class {
        InitData<T> init_data = arg as InitData<T>;
        AsyncFunction<T> routine = init_data.routine;
        T @params = init_data.@params;

        // dispose "init_data" because was allocated in main_thd_helper_spawn
        //free(init_data);

        //
        // Initialize the filesystem for this thread and later execute the routine
        // fs_destroy() is called when the thread ends (KallistiOS does this job).
        //
        // Note: This can not be implemented in javascript, because there no TLS support
        //
        FS.Init();

        // now execute the routine
        return routine(@params);
    }


    private class InitData<T> where T : class {
        public AsyncFunction<T> routine;
        public T @params;
    }

}
