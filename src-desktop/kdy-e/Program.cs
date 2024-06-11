using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using Engine;
using Engine.Externals.GLFW;
using Engine.Game;
using Engine.Platform;
using Engine.Utils;

namespace CsharpWrapper;

class Program {

    private const int ATTACH_PARENT_PROCESS = -1;

    [DllImport("kernel32")]
    private static extern bool AttachConsole(int dwProcessId);

    [DllImport("kernel32", SetLastError = true)]
    private static extern int AllocConsole();

    [DllImport("kernel32")]
    private static extern nint GetConsoleWindow();


    [STAThread]
    static int Main() {
        Application.SetHighDpiMode(HighDpiMode.SystemAware);
        Application.SetCompatibleTextRenderingDefault(false);
        Application.EnableVisualStyles();

        // adquire the executable name and arguments
        string[] args = Environment.GetCommandLineArgs();

        // before continue, check if is necessary alloc the logging console
        if (GetConsoleWindow() == 0x00) {
            bool should_attach = true;

            for (int i = 1 ; i < args.Length ; i++) {
                if (args[0].Equals("-console", StringComparison.InvariantCultureIgnoreCase)) {
                    should_attach = AllocConsole() == 0;
                }
            }

            if (should_attach) AttachConsole(ATTACH_PARENT_PROCESS);
        }

        // catch any unhandled exception before crash
        AppDomain.CurrentDomain.UnhandledException += UnhandledExceptionHandler;

        // used in texture loading
        Thread.CurrentThread.Name = "MainThread";

        // load ini file containing all settings
        EngineSettings.LoadINI();

        // check if the ini file and saves folder exists
        if (!EngineSettings.IniExists && !Directory.Exists(EngineSettings.EngineDir + "saves")) {
            // skip save selector for fresh installations
            FunkinSave.WriteToVMU();
        }

        // try load GAMECONTROLLERDB or SDL_GAMECONTROLLERCONFIG
        string gamecontrollerdb_path = EngineSettings.EngineDir + Glfw.GAMECONTROLLERDB;
        if (File.Exists(gamecontrollerdb_path)) {
            string mappings = File.ReadAllText(gamecontrollerdb_path, Encoding.UTF8);
            if (!Glfw.UpdateGamepadMappings(mappings)) {
                Logger.Error($"Failed to import gamepad mapping from {gamecontrollerdb_path}");
            }
        } else {
            string mappings = Environment.GetEnvironmentVariable("SDL_GAMECONTROLLERCONFIG");
            if (mappings != null && !Glfw.UpdateGamepadMappings(mappings)) {
                Logger.Error($"Failed to import gamepad mapping from SDL_GAMECONTROLLERCONFIG");
            }
        }

        // pick the expansion directory name from the ini (if present)
        string expansion_directory = EngineSettings.expansion_directory;

        // parse command line arguments
        for (int i = 1 ; i < args.Length ; i++) {
            int next_argument = i + i;
            bool is_last_argument = next_argument >= args.Length;


            switch (args[i].ToLowerInvariant()) {
                case "-expansionsloader":
                    expansion_directory = ExpansionsLoader.Main();
                    if (expansion_directory == null) return 0;
                    if (expansion_directory != null) {
                        Logger.Info($"Selected expansion: {expansion_directory}\n");
                    }
                    break;
                case "-expansion":
                    if (is_last_argument) continue;
                    EngineSettings.expansion_directory = args[next_argument];
                    if (StringUtils.IsEmpty(EngineSettings.expansion_directory)) {
                        EngineSettings.expansion_directory = null;
                        break;
                    }
                    if (EngineSettings.expansion_directory.ToLower() == "funkin") {
                        Logger.Info("'/expansions/funkin' is always applied");
                        EngineSettings.expansion_directory = null;
                    }
                    break;
                case "-style":
                    if (is_last_argument || StringUtils.IsEmpty(args[next_argument])) continue;
                    EngineSettings.style_from_week_name = args[next_argument].ToString();
                    break;
                case "-fullscreen":
                    EngineSettings.fullscreen = true;
                    break;
                case "-nowidescreen":
                    EngineSettings.widescreen = false;
                    break;
                case "-layoutdebugtriggercalls":
                    Layout.DEBUG_PRINT_TRIGGER_CALLS = true;
                    break;
                case "-saveslots":
                    if (is_last_argument) continue;
                    if (!Int32.TryParse(args[next_argument], out EngineSettings.saveslots)) EngineSettings.saveslots = 1;
                    break;
                case "-h":
                case "-help":
                case "--help":
                case "/help":
                case "/h":
                    Console.WriteLine(
                        $"{GameMain.ENGINE_NAME} {GameMain.ENGINE_VERSION}\r\n" +
                        "\r\n" +
                        $"    {args[0]} [-help] [-saveslots #] [-expansion FOLDER_NAME] [-style WEEK_NAME] [-fullscreen] [-nowidescreen] [-console] [-expansionloader] [-layoutdebugtriggercalls]\r\n" +
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

        // load selected expansion or default ("funkin" folder)
        Expansions.Load(expansion_directory);

        // initialize GLFW and OpenGL
        PVRContext.Init();

        // load keyboard mappings (GLFW must be already initialized)
        KallistiOS.MAPLE.maple_mappings.LoadKeyboardMappings();

        // now run the engine
        return GameMain.Main(args.Length, args);
    }

    private static void UnhandledExceptionHandler(object sender, UnhandledExceptionEventArgs e) {
        if (!e.IsTerminating) {
            Logger.Error($"Unhandled exception: {e.ExceptionObject}");
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
            Logger.Error($"Unhandled exception: {e.ExceptionObject}");
        }
    }

}
