using System;
using System.IO;
using Engine.Externals.GLFW;
using Engine.Utils;

namespace Engine.Platform;

public static class Expansions {
    public const string FOLDER_NAME = "expansions";
    public const string PATH = "/expansions";
    private const string SYMBOLIC_PATH = "/~expansions/";
    public const string FUNKIN_EXPANSION_NAME = "funkin";
    private const string FUNKIN_EXPANSION_PATH = SYMBOLIC_PATH + FUNKIN_EXPANSION_NAME;
    private const string CHAIN_FILENAME = "chain.ini";
    public const string ABOUT_FILENAME = "expansion.json";
    private const char COMMENT_CHAR = ';';
    private const string SELF_NAME = "/self/";

    internal static string[] chain_array;
    internal static int chain_array_size;

    public static string overrided_weeks_folder;
    public static byte[] startup_icon;
    public static string startup_title;

    static Expansions() {
        chain_array = null;
        chain_array_size = 0;
    }


    public static void Load(string expansion_name) {
        if (expansion_name != null && FS.IsInvalidFilename(expansion_name)) {
            Logger.Error($"expansions_load() invalid expansion name, ilegal filename: {expansion_name}");
            return;
        }

        //for (int i = 0 ; i < chain_array_size ; i++) {
        //    if (chain_array[i] != FUNKIN_EXPANSION_PATH) {
        //        //free(chain_array[i]);
        //    }
        //}
        //free(chain_array);
        chain_array_size = 0;

        // clear preload cache
        PreloadCache.ClearCache();

        ArrayList<string> chain = new ArrayList<string>();

        if (IO.ResourceExists(Expansions.FUNKIN_EXPANSION_PATH, false, true)) {
            InternalLoadDependency(chain, Expansions.FUNKIN_EXPANSION_NAME);
        }

        InternalLoadDependency(chain, expansion_name);

        //if (Expansions.overrided_weeks_folder) free(Expansions.overrided_weeks_folder);
        Expansions.overrided_weeks_folder = null;

        string expansion_base_path = StringUtils.Concat(Expansions.SYMBOLIC_PATH, expansion_name, FS.CHAR_SEPARATOR.ToString());
        string about_path = StringUtils.Concat(expansion_base_path, Expansions.ABOUT_FILENAME);
        bool has_about = IO.ResourceExists(about_path, true, false);

        if (has_about) {
            string about_json_path = StringUtils.Concat(expansion_base_path, Expansions.ABOUT_FILENAME);

            //free(about_path);
            about_path = IO.GetNativePath(about_json_path, true, false, false);

            JSONToken json = JSONParser.LoadDirectFrom(about_path);
            bool override_weeks_folder = JSONParser.ReadBoolean(json, "overrideWeeksFolder", false);

            // JS & C# only
            string window_title = JSONParser.ReadString(json, "windowTitle", null);
            string window_icon = JSONParser.ReadString(json, "windowIcon", null);
            Expansions.InternalUpdateWindow(expansion_base_path, window_title, window_icon);

            JSONParser.Destroy(json);
            //free(about_json_path);

            if (override_weeks_folder) {
                overrided_weeks_folder = StringUtils.Concat(expansion_base_path, "weeks");
            }
        }

        //free(expansion_base_path);
        //free(about_path);

        chain_array_size = chain.Trim();
        chain_array = chain.PeekArray();
        chain.Destroy(true);
    }

    public static string ResolvePath(string path, bool is_file, bool is_folder) {
        if (path == null) return null;
        if (!path.StartsWithKDY(FS.ASSETS_FOLDER, 0)) return path;

        int path_length = path.Length;
        int index = FS.ASSETS_FOLDER.Length;
        if (index < path_length && path[index] == FS.CHAR_SEPARATOR) index++;

        string relative_path = path.SubstringKDY(index, path_length);
        string last_overrided_path = path.ToString();

        for (int i = 0 ; i < chain_array_size ; i++) {
            string overrided_path = StringUtils.Concat(
                chain_array[i], FS.CHAR_SEPARATOR.ToString(), relative_path
            );

            if (IO.ResourceExists(overrided_path, is_file, is_folder)) {
                //if (last_overrided_path != null) free(last_overrided_path);
                last_overrided_path = overrided_path;
                continue;
            }

            //free(overrided_path);
        }

        //free(relative_path);

        return last_overrided_path;
    }

    public static string GetPathFromExpansion(string path, int expansion_index) {
        if (path == null) return null;
        if (!path.StartsWithKDY(FS.ASSETS_FOLDER, 0)) return null;
        if (expansion_index < 0 || expansion_index >= chain_array_size) return null;

        int path_length = path.Length;
        int index = FS.ASSETS_FOLDER.Length;
        if (index < path_length && path[index] == FS.CHAR_SEPARATOR) index++;

        string relative_path = path.SubstringKDY(index, path_length);

        string new_path = StringUtils.Concat(
            chain_array[expansion_index], FS.CHAR_SEPARATOR.ToString(), relative_path
        );

        //free(relative_path);

        return new_path;

        /*if (IO.ResourceExists(new_path, true, true)) {
            return new_path;
        }

        //free(new_path);
        return null;*/
    }


    private static void InternalLoadDependency(ArrayList<string> chain, string expansion_name) {
        if (StringUtils.IsEmpty(expansion_name)) return;

        string expansion_path = StringUtils.Concat(Expansions.SYMBOLIC_PATH, expansion_name);
        if (!IO.ResourceExists(expansion_path, false, true)) {
            Logger.Error($"expansions_load() '{expansion_name}' not found in {Expansions.PATH}");
            //free(expansion_path);
            return;
        }

        string expansion_preload_ini_path = StringUtils.Concat(expansion_path, FS.CHAR_SEPARATOR.ToString(), PreloadCache.PRELOAD_FILENAME);
        if (IO.ResourceExists(expansion_preload_ini_path, true, false)) {
            PreloadCache.AddFileList(expansion_preload_ini_path);
        }
        //free(expansion_preload_ini_path);

        string chain_ini_path = StringUtils.Concat(expansion_path, FS.CHAR_SEPARATOR.ToString(), Expansions.CHAIN_FILENAME);
        bool has_chain = IO.ResourceExists(chain_ini_path, true, false);

        if (!has_chain) {
            InternalAddToChain(chain, expansion_path);
            //free(expansion_path);
            //free(chain_ini_path);
            return;
        }

        string unparsed_chain = IO.ReadText(chain_ini_path);
        //free(chain_ini_path);

        if (unparsed_chain == null) return;

        Tokenizer tokenizer = Tokenizer.Init("\r\n", true, false, unparsed_chain);
        if (tokenizer == null) {
            InternalAddToChain(chain, expansion_path);
            //free(unparsed_chain);
            return;
        }

        string name;
        while ((name = tokenizer.ReadNext()) != null) {
            if (StringUtils.IsEmpty(name) || name[0] == Expansions.COMMENT_CHAR) {
                //free(name);
                continue;
            }

            if (name == Expansions.SELF_NAME) {
                InternalAddToChain(chain, expansion_path);
            } else if (FS.IsInvalidFilename(name)) {
                Logger.Error($"expansions_internal_load_dependency() '{name}' is not a valid folder name");
            } else {
                // warning: recursive call
                InternalLoadDependency(chain, name);
            }

            //free(name);
        }

        InternalAddToChain(chain, expansion_path);

        tokenizer.Destroy();
        //free(unparsed_chain);
        //free(tokenizer);
    }

    private static void InternalAddToChain(ArrayList<string> chain, string expansion_path) {
        string lowercase_expansion_name = StringUtils.ToLowerCase(expansion_path);

        foreach (string expansion in chain) {
            if (expansion == expansion_path) return;

            string lowercase_expansion = StringUtils.ToLowerCase(expansion);
            bool equals = lowercase_expansion == lowercase_expansion_name;

            //free(lowercase_expansion);

            if (equals) goto L_return;
        }
        chain.Add(expansion_path);

L_return:
//free(lowercase_expansion_name);
        return;
    }

    private static void InternalUpdateWindow(string expansion_base_path, string window_title, string window_icon) {
        window_icon = FS.ResolvePath(expansion_base_path + window_icon);
        window_icon = IO.GetNativePath(window_icon, true, false, false);

        PVRContext pvr_context = PVRContext.global_context;
        byte[] icon = null;

        if (File.Exists(window_icon)) icon = File.ReadAllBytes(window_icon);

        if (pvr_context == null) {
            Expansions.startup_icon = icon;
            Expansions.startup_title = window_title;
            return;
        }

        using (IconLoader loader = new IconLoader(icon ?? Engine.Properties.Resources.icon)) {
            Icon[] icons = new Icon[loader.icons.Length];
            for (int i = 0 ; i < icons.Length ; i++) {
                icons[i] = new Icon(
                    loader.icons[i].width,
                    loader.icons[i].height,
                    loader.icons[i].pixels
                );
            }

            Window hnd = PVRContext.InternalNativeWindow;
            Glfw.SetWindowTitle(hnd, window_title ?? pvr_context.native_window_title);
            Glfw.SetWindowIcon(hnd, icons.Length, icons);
        }
    }
}
