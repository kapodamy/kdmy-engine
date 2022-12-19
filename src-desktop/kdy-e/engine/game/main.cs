using System;
using Engine.Font;
using Engine.Game.Common;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;
using KallistiOS.THD;
using KallistiOS.UTSNAME;

namespace Engine.Game {

    public static class GameMain {

        // this file contains all shared data across the game
        public const string ENGINE_NAME = "kdmy-engine";
        public const string ENGINE_VERSION = "0.47.7";

        /**
         * The background music used in all menus, inherited from introscreen
         */
        public static SoundPlayer background_menu_music = null;

        /**
         * Menus style (customized by a week) used across the game
         */
        public static WeekInfo custom_style_from_week = null;


        public static int Main(string[] argc, int argv) {
            // vital parts
            FS.Init();// intialize filesystem access for the main thread (this thread)

            utsname osinfo;
            UTSNAME.uname(out osinfo);
            Console.Error.WriteLine($"[LOG] {ENGINE_NAME} {ENGINE_VERSION}");
            Console.Error.WriteLine($"[LOG] Console: {osinfo.machine}");
            Console.Error.WriteLine($"[LOG] OS: {osinfo.version}\r\n");

            // (JS & CS only) preload fonts
            new FontHolder("/assets/common/font/vcr.ttf", -1, null);
            //new FontHolder("/assets/common/font/pixel.otf", -1, null);
            new FontHolder("/assets/common/font/alphabet.xml", -1, null);

            WeekEnumerator.Enumerate();

            if (Funkin.weeks_array.size < 1) Console.Error.WriteLine("[WARN] main() no weeks detected");

            // load savedata
            int load_result = SaveManager.ShouldShow(false);
            if (load_result != 0) {
                SaveManager savemanager = new SaveManager(false, load_result);
                savemanager.Show();
                savemanager.Destroy();
            }

            // choose a random menu style
            bool try_choose_last_played = Math2D.RandomFloat() <= 0.25f;
            bool[] visited = new bool[Funkin.weeks_array.size];
            int visited_count = 0;
            for (int i = 0 ; i < Funkin.weeks_array.size ; i++) visited[i] = false;

            while (visited_count < Funkin.weeks_array.size) {
                WeekInfo weekinfo;

                if (try_choose_last_played) {
                    try_choose_last_played = false;
                    weekinfo = null;
                    string last_played = FunkinSave.GetLastPlayedWeek();

                    if (CsharpWrapper.EngineSettings.style != null) last_played = CsharpWrapper.EngineSettings.style;

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
                    int random_index = Math2D.RandomInt(0, Funkin.weeks_array.size - 1);
                    if (visited[random_index]) continue;
                    visited[random_index] = true;
                    visited_count++;
                    weekinfo = Funkin.weeks_array.array[random_index];
                }

                if (String.IsNullOrEmpty(weekinfo.custom_folder) && !weekinfo.has_greetings) continue;

                if (FunkinSave.ContainsUnlockDirective(weekinfo.unlock_directive)) {
                    custom_style_from_week = weekinfo;

                    // override commons files (if possible)
                    if (!String.IsNullOrEmpty(weekinfo.custom_folder)) {
                        FS.OverrideCommonFolder(weekinfo.custom_folder);
                    }
                    break;
                }
            }

            GameMain.background_menu_music = SoundPlayer.Init(Funkin.BACKGROUND_MUSIC);
            if (GameMain.background_menu_music != null) background_menu_music.LoopEnable(true);

            // this code is for quick week debugging only
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
                -1
            );
            Console.Error.WriteLine("[LOG] week_main() execution done, engine is terminated");
            return 0;*/

            IntroScreen.Main();

            while (true) {
                if (StartScreen.Main()) break;
                while (MainMenu.Main()) ;
                if (background_menu_music != null) background_menu_music.Replay();
            }

            return 1;
        }

        public static object SpawnCoroutine(Layout background_layout, Func<object, object> function_routine, object argument_routine) {
            kthread_t thread = THDHelperSpawn(function_routine, argument_routine);
            object ret;

            if (background_layout != null) {
                while (thread.state != STATE.FINISHED) {
                    float elapsed = PVRContext.global_context.WaitReady();
                    PVRContext.global_context.Reset();

                    background_layout.Animate(elapsed);
                    background_layout.Draw(PVRContext.global_context);
                }

                // aquire the return value and destroy the thread struct
                ret = thread.rv;
                thd.destroy(thread);
            } else {
                // no layout specified, wait for thread end
                object tmp = null;
                thd.join(thread, ref tmp);
                ret = tmp;
            }

            return ret;
        }

        /**
         * Create a new thread.
         * This function creates a new kernel thread with default parameters to run the given routine.
         * The thread will terminate and clean up resources when the routine completes
         * This function warps {@link thd_create} to allow multi-thread filesystem support (fs_* engine functions)
         * @param {function} routine The function to call in the new thread.
         * @param {object} param A parameter to pass to the function called.
         * @returns The new thread on success, NULL on failure.
         */
        public static kthread_t THDHelperSpawn(Func<object, object> routine, object param) {
            // in C, "init_data" this an allocated struct
            InitData init_data = new InitData() { routine = routine, @params = param };
            return thd.create(true, GameMain.THDHelperSpawnWrapper, init_data);
        }

        private static object THDHelperSpawnWrapper(object arg) {
            InitData init_data = arg as InitData;
            Func<object, object> routine = init_data.routine;
            object @params = init_data.@params;

            // dispose "init_data" because was allocated in thd_helper_spawn
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


        private class InitData {
            public Func<object, object> routine;
            public object @params;
        }

    }

}