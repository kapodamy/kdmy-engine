using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using Engine;
using Engine.Externals;
using Engine.Externals.GLFW;
using Engine.Externals.LuaInterop;
using Engine.Game;
using Engine.Platform;

namespace CsharpWrapper {

    class Program {

        [DllImport("kernel32")]
        static extern bool AttachConsole(int dwProcessId);
        private const int ATTACH_PARENT_PROCESS = -1;

        [DllImport("kernel32", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool AllocConsole();

        [STAThread]
        static int Main() {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            bool should_attach_console = true;
            string[] argv = Environment.GetCommandLineArgs();

            // before any print, check if is necessary alloc the consoles
            if (HasCommandLineSwitch(argv, "-console")) {
                should_attach_console = !AllocConsole();
            }
            if (should_attach_console) AttachConsole(ATTACH_PARENT_PROCESS);

            // show expansions loader if is necessary
            if (HasCommandLineSwitch(argv, "-expansionsloader")) {
                ExpansionsLoader form = new ExpansionsLoader();

                Application.Run(form);
                Application.Exit();

                bool exit = form.NotLaunchAndExit;
                EngineSettings.expansion = form.SelectedExpansion;
                form.Destroy();

                if (exit) return 0;

                Console.WriteLine("Selected expansion: " + (EngineSettings.expansion ?? "<none>"));
                Console.WriteLine();
            }

            AppDomain.CurrentDomain.UnhandledException += UnhandledExceptionHandler;

            Console.Error.WriteLine("AICA:" + AICA.sndbridge_get_runtime_info());
            Console.Error.WriteLine("FontAtlas: FreeType V" + ModuleFontAtlas.fontatlas_get_version());
            Console.Error.WriteLine("LuaScript: " + ManagedLuaState.GetVersion());
            Console.Error.WriteLine("FFGraph: " + FFgraph.ffgraph_get_runtime_info() ?? "<not loaded>");

            Thread.CurrentThread.Name = "MainThread";// used in texture loading
            EngineSettings.LoadINI();

            bool force_fullscreen = false;

            for (int i = 1 ; i < argv.Length ; i++) {
                switch (argv[i].ToLowerInvariant()) {
                    case "-expansion":
                        NextArgAs(argv, i + 1, ref EngineSettings.expansion);
                        if (String.IsNullOrEmpty(EngineSettings.expansion)) {
                            EngineSettings.expansion = null;
                            break;
                        }
                        if (EngineSettings.expansion.ToLower() == "funkin") {
                            Console.Error.WriteLine("[INFO] '/expansions/funkin' is always applied");
                            EngineSettings.expansion = null;
                        }
                        break;
                    case "-style":
                        NextArgAs(argv, i + 1, ref EngineSettings.style);
                        break;
                    case "-fullscreen":
                        force_fullscreen = true;
                        break;
                    case "-nowidescreen":
                        EngineSettings.widescreen = false;
                        break;
                    case "-layoutdebugtriggercalls":
                        Layout.DEBUG_PRINT_TRIGGER_CALLS = true;
                        break;
                    case "-h":
                    case "-help":
                    case "--help":
                    case "/help":
                    case "/h":
                        Console.WriteLine(
                            $"{GameMain.ENGINE_NAME} {GameMain.ENGINE_VERSION}\r\n" +
                            "\r\n" +
                            $"    {argv[0]} [-help] [-saveslots #] [-expansion FOLDER_NAME] [-style WEEK_NAME] [-fullscreen] [-nowidescreen] [-console] [-expansionloader] [-layoutdebugtriggercalls]\r\n" +
                            "\r\n" +
                            "Options:\r\n" +
                            "    -help                      Show this help message\r\n" +
                            "    -saveslots                 Number of emulated VMU (Visual Memory Card) availabe. Defaults to 1\r\n" +
                            "    -expansion                 Folder name inside of '/expansions' folder, this overrides the '/assets' folder contents. Disabled by default\r\n" +
                            "    -style                     Week name (folder name inside of '/assets/weeks' folder) and picks the folder '/assets/weeks/WEEK_NAME/custom' or the defined in 'about.json' file. Defaults to the last played week\r\n" +
                            "    -fullscreen                Starts the engine in fullscreen, toggle to windowed pressing 'F11' key. Defaults to windowed\r\n" +
                            "    -nowidescreen              Forces the 4:3 aspect ratio like in the dreamcast. Defaults to 16:9, but changes if resized\r\n" +
                            "    -console                   Opens a console window for all engine and lua scripts messages\r\n" +
                            "    -expansionloader           Opens a window to choose the expansion to use\r\n" +
                            "    -layoutdebugtriggercalls   Prints in console all layout_triger_***() calls\r\n" +
                            "\r\n" +
                            "Notes:\r\n" +
                            "  -nowidescreen uses the 640x480 window size, otherwise defaults to 950x540. Anyways, the window still can be resized.\r\n" +
                            "  in -saveslots, the maximum is 8 VMUS, and the names are a1, b1, c1, d1, a3, b5, etc...\r\n" +
                            "  the folder '/expansions/funkin' is always applied, place modifications here if you do not want to messup '/assets' contents\r\n" +
                            "  the files override order is:   '<requested file in assets>' --> '/assets/weeks/WEEK_NAME/custom_commons' --> '/expansions/EXPANSION_NAME' --> '/expansions/funkin' --> '/assets/*'\r\n" +
                            "  shaders under '/assets/shaders' can not be overrided (for now).\n" +
                            "  -style only overrides '/assets/common' folder while expansions overrides everything under '/assets'\r\n" +
                            "\r\n"
                        );
                        return 0;
                    default:
                        continue;
                }
                i++;
            }

            Thread thread = new Thread(delegate () {
                FS.Init();
                Expansions.Load(EngineSettings.expansion);
            });
            thread.Start();
            thread.Join();

            PVRContext.InitializeGlobalContext();
            KallistiOS.MAPLE.maple_mappings.LoadKeyboardMappings();

            if (force_fullscreen && !PVRContext.IsRunningInFullscreen) {
                PVRContext.ToggleFullscreen();
            }

            if (!EngineSettings.ini_exists && !Directory.Exists(EngineSettings.EngineDir + "saves")) {
                // skip save selector for fresh installations
                FunkinSave.WriteToVMU();
            }

            string gamecontrollerdb_path = EngineSettings.EngineDir + Glfw.GAMECONTROLLERDB;
            if (File.Exists(gamecontrollerdb_path)) {
                Console.WriteLine("detected 'gamecontrollerdb.txt' file, updating mappings");
                string mappings = File.ReadAllText(gamecontrollerdb_path, Encoding.ASCII);
                if (!Glfw.glfwUpdateGamepadMappings(mappings)) {
                    Console.Error.WriteLine("[ERROR] failed to import " + gamecontrollerdb_path);
                }
            }

            return GameMain.Main(argv, argv.Length);
        }

        private static void NextArgAs(string[] argv, int index, ref string val) {
            if (index >= argv.Length) return;
            val = argv[index];
        }

        private static void NextArgAs(string[] argv, int index, ref int val) {
            if (index >= argv.Length) return;
            int value = (int)val;
            if (Int32.TryParse(argv[index], out value)) val = value;
        }

        private static bool HasCommandLineSwitch(string[] argv, string arg) {
            for (int i = 1 ; i < argv.Length ; i++) {
                if (argv[i].ToLowerInvariant() == arg) return true;
            }
            return false;
        }

        private static void UnhandledExceptionHandler(object sender, UnhandledExceptionEventArgs e) {
            if (!e.IsTerminating) {
                Console.Error.WriteLine("Unhandled exception: " + e.ExceptionObject.ToString());
                return;
            }

            try {
                string filename = "crash_" + DateTime.Now.ToString("yyyy-MM-ddTHHmmss.fff") + ".txt";
                string path = AppDomain.CurrentDomain.BaseDirectory + filename;
                string text = "Unhandled exception\r\n" +
                              $"Engine: {GameMain.ENGINE_NAME} {GameMain.ENGINE_VERSION}\r\n" +
                              "Error: " + e.ExceptionObject.ToString();

                File.WriteAllText(path, text);
                return;
            } catch {
                Console.Error.WriteLine("Unhandled exception: " + e.ExceptionObject.ToString());
            }
        }

    }
}
