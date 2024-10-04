using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using CsharpWrapperExpansionsLoader;
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


    static int Main() {
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
                    expansion_directory = ExpansionsLoader.Main(
                        Engine.Properties.Resources.icon,
                        Program.QueryExpansions,
                        Path.Combine(EngineSettings.EngineDir, Expansions.PATH)
                    );
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
                case "-indexlisting":
                    Logger.Log("Creating index listing indexlisting.json file... ");
                    IndexListingCreator.SerializeDirectory(EngineSettings.EngineDir);
                    Logger.Log("Done");
                    return 1;
                case "-h":
                case "-help":
                case "--help":
                case "/help":
                case "/h":
                    Console.WriteLine(
                        $"{GameMain.ENGINE_NAME} {GameMain.ENGINE_VERSION}\r\n" +
                        "\r\n" +
                        $"    {args[0]} [-help] [-saveslots #] [-expansion FOLDER_NAME] [-style WEEK_NAME] [-fullscreen] [-nowidescreen] [-console] [-expansionloader] [-layoutdebugtriggercalls] [-indexlisting]\r\n" +
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
                        "    -indexlisting              Creates a JSON file listing all files and directories in '/assets' and '/expansions', which is necessary for javascript version\r\n" +
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

        // run in a separate thread because the FileSystem is not available yet
        Thread loader = new Thread(delegate () {
            FS.Init();
            // load selected expansion or default ("funkin" folder)
            Expansions.Load(expansion_directory);
        });
        loader.Start();
        loader.Join();

        // initialize GLFW and OpenGL
        PVRContext.Init();

        // load keyboard mappings (GLFW must be already initialized)
        KallistiOS.MAPLE.maple.LoadKeyboardMappings();

        // now run the engine
        return GameMain.Main(args.Length, args);
    }

    private static ExpansionInfo[] QueryExpansions(out int loaded_count) {
        string expansions_dir = Path.Combine(EngineSettings.EngineDir, Expansions.FOLDER_NAME);
        if (!Directory.Exists(expansions_dir)) {
            loaded_count = 0;
            return Array.Empty<ExpansionInfo>();
        }

        ArrayList<ExpansionInfo> expansions = new ArrayList<ExpansionInfo>();
        DirectoryInfo directory = new DirectoryInfo(expansions_dir);

        expansions.Add(new ExpansionInfo(null) {
            name = "(Launch without any expansion)",
            version = ""
        });

        foreach (DirectoryInfo dir in directory.EnumerateDirectories()) {
            string dir_relative_path = $"{Expansions.PATH}{FS.CHAR_SEPARATOR}{dir.Name}{FS.CHAR_SEPARATOR}";

            if (dir.Name.ToLowerInvariant() == Expansions.FUNKIN_EXPANSION_NAME) continue;

            ExpansionInfo expansion = new ExpansionInfo(dir.Name);

            string about_src = Path.Combine(dir.FullName, Expansions.ABOUT_FILENAME);
            if (!File.Exists(about_src)) {
                Logger.Error($"ExpansionsLoader::LoadExpansions() missing file {about_src}");
                expansions.Add(expansion);
                continue;
            }

            JSONToken json = JSONParser.LoadDirectFrom(about_src);
            if (json == null) {
                Logger.Warn($"ExpansionsLoader::LoadExpansions() can not open: {about_src}");
                continue;
            }

            expansion.name = JSONParser.ReadString(json, "name", dir.Name);
            expansion.version = JSONParser.ReadString(json, "version", null);
            expansion.submiter = JSONParser.ReadString(json, "submiter", null);
            expansion.description = JSONParser.ReadString(json, "description", null);
            string screenshoot_path = JSONParser.ReadString(json, "screenshoot", null);
            string icon_path = JSONParser.ReadString(json, "icon", null);
            JSONParser.Destroy(json);

            if (screenshoot_path != null) {
                screenshoot_path = IO.GetNativePath(
                    FS.ResolvePath($"{dir_relative_path}{screenshoot_path}"), true, false, false
                );

                if (File.Exists(screenshoot_path)) {
                    expansion.screenshoot_native_path = screenshoot_path;
                }
            }
            if (icon_path != null) {
                icon_path = IO.GetNativePath(
                    FS.ResolvePath($"{dir_relative_path}{icon_path}"), true, false, false
                );

                if (File.Exists(icon_path)) {
                    expansion.icon_native_path = icon_path;
                }
            }

            // TODO: markdown to RTF (https://es.wikipedia.org/wiki/Rich_Text_Format)
            if (expansion.submiter != null) {
                expansion.submiter = expansion.submiter.Replace("\r", "").Replace("\n", "\r\n");
            }
            if (expansion.description != null) {
                expansion.description = expansion.description.Replace("\r", "").Replace("\n", "\r\n");
            }
            if (expansion.version != null) {
                expansion.version = expansion.version.Replace("\r", "").Replace("\n", "\r\n");
            }
            if (expansion.name != null) {
                expansion.name = expansion.name.Replace("\r", "").Replace("\n", "\r\n");
            }

            expansions.Add(expansion);
        }

        ExpansionInfo[] array;
        expansions.Destroy2(out loaded_count, out array);

        return array;
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
