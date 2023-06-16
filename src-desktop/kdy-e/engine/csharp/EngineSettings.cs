using System;
using System.IO;
using Engine.Platform;
using Settings;

namespace CsharpWrapper;

internal static class EngineSettings {
    public const string INI_BINDING_SECTION = "KeyboardBindings";
    public const string INI_MISC_SECTION = "Misc";
    public const string INI_GAMEPLAY_SECTION = "Gameplay";

    public static INI ini;
    public static bool ini_exists;

    public static string expansion_directory = null;// folder name inside of "/expansions/"
    public static byte[] expansion_window_icon = null;// window icon bytes
    public static string expansion_window_title = null;// window title
    public static string style = null;// folder name inside of "/expansions/"
    public static bool widescreen = true;// uses 1280x720 (like Funkin) instead of 640x480
    public static bool pixelbufferobjects = false;// uses the OpenGL PBO (no async support)

    public static int input_offset = 0;
    public static bool penality_on_empty_strum = true;
    public static bool inverse_strum_scroll = false;
    public static bool song_progressbar = false;
    public static bool song_progressbar_remaining = false;

    public static bool show_fps = false;
    public static byte fps_limit = 0;
    public static bool autohide_cursor = true;
    public static bool mute_on_minimize = true;
    public static int master_volume = 100;
    public static bool show_loading_screen = false;
    public static bool use_funkin_marker_duration = true;
    public static bool gameplay_enabled_distractions = true;
    public static bool gameplay_enabled_flashinglights = true;
    public static bool gameplay_enabled_uicosmetics = true;

    internal static void LoadINI() {
        if (ini == null) {
            string ini_path = EngineSettings.EngineDir + "settings.ini";

            ini_exists = File.Exists(ini_path);
            ini = new INI(ini_path);
        }

        input_offset = GetInt(true, "input_offset", input_offset);
        penality_on_empty_strum = GetBool(true, "penality_on_empty_strum", penality_on_empty_strum);
        inverse_strum_scroll = GetBool(true, "inverse_strum_scroll", inverse_strum_scroll);
        song_progressbar = GetBool(true, "song_progressbar", song_progressbar);
        song_progressbar_remaining = GetBool(true, "song_progressbar_remaining", song_progressbar_remaining);
        use_funkin_marker_duration = GetBool(true, "use_funkin_marker_duration", use_funkin_marker_duration);
        gameplay_enabled_distractions = GetBool(true, "gameplay_enabled_distractions", gameplay_enabled_distractions);
        gameplay_enabled_flashinglights = GetBool(true, "gameplay_enabled_flashinglights", gameplay_enabled_flashinglights);
        gameplay_enabled_uicosmetics = GetBool(true, "gameplay_enabled_uicosmetics", true);
        pixelbufferobjects = GetBool(false, "pixelbufferobjects", pixelbufferobjects);

        show_fps = GetBool(false, "show_fps", show_fps);
        switch (GetString(false, "fps_limit", null)) {
            case "vsync":
                fps_limit = 0;
                break;
            case "deterministic":
                fps_limit = 1;
                break;
            case "off":
                fps_limit = 2;
                break;
        }
        autohide_cursor = GetBool(false, "autohide_cursor", autohide_cursor);
        mute_on_minimize = GetBool(false, "mute_on_minimize", mute_on_minimize);
        master_volume = GetInt(false, "master_volume", master_volume);
        show_loading_screen = GetBool(false, "show_loading_screen", show_loading_screen);
    }

    internal static void Reload() {
        int old_fps_limit = fps_limit;
        LoadINI();

        if (old_fps_limit != fps_limit) PVRContext.global_context.SetFPSLimit(fps_limit, true);
        PVRContext.UnHideCursor();
        PVRContext.MuteAudioOutputOnMinimized(mute_on_minimize);
    }

    public static void GetBind(string ini_key, ref int scancode) {
        long raw_value = ((long)scancode) << 32;// [scancode][glfw key enum]
        raw_value = ini.GetLong(INI_BINDING_SECTION, ini_key, raw_value);

        scancode = (int)(raw_value >> 32);
    }

    public static int GetInt(bool is_gameplay_setting, string ini_key, int def_value) {
        string section = is_gameplay_setting ? INI_GAMEPLAY_SECTION : INI_MISC_SECTION;
        return ini.GetInt(section, ini_key, def_value);
    }

    public static bool GetBool(bool is_gameplay_setting, string ini_key, bool def_value) {
        string section = is_gameplay_setting ? INI_GAMEPLAY_SECTION : INI_MISC_SECTION;
        return ini.GetBool(section, ini_key, def_value);
    }

    public static string GetString(bool is_gameplay_setting, string ini_key, string def_value) {
        string section = is_gameplay_setting ? INI_GAMEPLAY_SECTION : INI_MISC_SECTION;
        return ini.GetString(section, ini_key, def_value);
    }

    public static string EngineDir {
        get => AppDomain.CurrentDomain.BaseDirectory;
    }

}
