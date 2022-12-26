using System;
using Engine.Utils;

namespace Engine.Platform {

    public static class Expansions {
        public const string PATH = "/expansions/";
        private const string SYMBOLIC_PATH = "/~expansions/";
        private const string FUNKIN_EXPANSION_NAME = "funkin";
        private const string FUNKIN_EXPANSION_PATH = SYMBOLIC_PATH + FUNKIN_EXPANSION_NAME;
        private const string CHAIN_FILENAME = "chain.ini";
        public const string ABOUT_FILENAME = "expansion.json";
        private const char COMMENT_CHAR = ';';
        private const string SELF_NAME = "/self/";

        public static string[] chain_array;
        public static int chain_array_size;
        public static string overrided_weeks_folder;

        static Expansions() {
            chain_array = null;
            chain_array_size = 0;
        }


        public static void Load(string expansion_name) {
            if (expansion_name != null && FS.IsInvalidFilename(expansion_name)) {
                Console.Error.WriteLine($"[ERROR] Expansions::Load() invalid expansion name, ilegal filename: ${expansion_name}");
                return;
            }

            //for (int i = 0 ; i < chain_array_size ; i++) {
            //    if (chain_array[i] != FUNKIN_EXPANSION_PATH) {
            //        //free(chain_array[i]);
            //    }
            //}
            //free(chain_array);

            // force clear preload cache 
            PreloadCache.ClearCache();

            ArrayList<string> chain = new ArrayList<string>();

            if (IO.ResourceExists(Expansions.FUNKIN_EXPANSION_PATH, false, true)) {
                InternalLoadDependency(chain, Expansions.FUNKIN_EXPANSION_NAME);
            }

            InternalLoadDependency(chain, expansion_name);
            Expansions.overrided_weeks_folder = null;

            string expansion_base_path = Expansions.SYMBOLIC_PATH + expansion_name + FS.CHAR_SEPARATOR;
            string about_path = expansion_base_path + Expansions.ABOUT_FILENAME;
            bool has_about = IO.ResourceExists(about_path, true, false);

            if (has_about) {
                about_path = IO.GetAbsolutePath(expansion_base_path + Expansions.ABOUT_FILENAME, true, false);
                JSONParser json = JSONParser.LoadDirectFrom(about_path);
                bool override_weeks_folder = JSONParser.ReadBoolean(json, "overrideWeeksFolder", false);
                JSONParser.Destroy(json);

                if (override_weeks_folder) overrided_weeks_folder = expansion_base_path + "weeks";
            }

            chain_array_size = chain.Trim();
            chain_array = chain.PeekArray();
            chain.Destroy(true);
        }

        public static string ResolvePath(string path, bool is_file) {
            if (path == null) return null;
            if (!path.StartsWithKDY(FS.ASSETS_FOLDER, 0)) return path;


            int path_length = path.Length;
            int index = FS.ASSETS_FOLDER.Length;
            if (index < path_length && path[index] == FS.CHAR_SEPARATOR) index++;

            string relative_path = path.SubstringKDY(index, path.Length);
            string last_overrided_path = path.ToString();

            for (int i = 0 ; i < chain_array_size ; i++) {
                string overrided_path = StringUtils.Concat(
                    chain_array[i], FS.CHAR_SEPARATOR.ToString(), relative_path
                );

                if (IO.ResourceExists(overrided_path, is_file, !is_file)) {
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

            return new_path;

            /*if (IO.ResourceExists(new_path, true, true)) {
                return new_path;
            }

            //free(new_path);
            return null;*/
        }


        private static void InternalLoadDependency(ArrayList<string> chain, string expansion_name) {
            if (String.IsNullOrEmpty(expansion_name)) return;

            string expansion_path = Expansions.SYMBOLIC_PATH + expansion_name;
            if (!IO.ResourceExists(expansion_path, false, true)) {
                Console.Error.WriteLine($"[ERROR] expansions_load() '{expansion_name}' not found in {Expansions.PATH}");
                //free(expansion_path);
                return;
            }

            string expansion_preload_ini_path = expansion_path + FS.CHAR_SEPARATOR + PreloadCache.PRELOAD_FILENAME;
            if (IO.ResourceExists(expansion_preload_ini_path, true, false)) {
                PreloadCache.AddFileList(expansion_preload_ini_path);
            }

            string chain_ini_path = expansion_path + FS.CHAR_SEPARATOR + Expansions.CHAIN_FILENAME;
            bool has_chain = IO.ResourceExists(chain_ini_path, true, false);

            if (!has_chain) {
                InternalAddToChain(chain, expansion_path);
                //free(expansion_path);
                //free(chain_ini_path);
                return;
            }

            string uparsed_chain = IO.ReadText(chain_ini_path);
            //free(chain_ini_path);

            Tokenizer tokenizer = Tokenizer.Init("\r\n", false, false, uparsed_chain);
            if (tokenizer == null) {
                InternalAddToChain(chain, expansion_path);
                //free(uparsed_chain);
                //free(tokenizer);
                return;
            }

            string name;
            while ((name = tokenizer.ReadNext()) != null) {
                if (String.IsNullOrEmpty(name) || name[0] == Expansions.COMMENT_CHAR) {
                    //free(entry);
                    continue;
                }

                if (name == Expansions.SELF_NAME) {
                    InternalAddToChain(chain, expansion_path);
                } else if (FS.IsInvalidFilename(name)) {
                    Console.Error.WriteLine($"[ERROR] expansions_internal_load_dependency() '{name}' is not a valid folder name");
                } else {
                    InternalLoadDependency(chain, name);
                }

                //free(entry);
            }

            InternalAddToChain(chain, expansion_path);

            tokenizer.Destroy();
            //free(uparsed_chain);
            //free(tokenizer);
        }

        private static void InternalAddToChain(ArrayList<string> chain, string expansion_path) {
            foreach (string expansion in chain) {
                if (expansion == expansion_path) return;

                string lowercase_expansion = StringUtils.ToLowerCase(expansion);
                string lowercase_expansion_name = StringUtils.ToLowerCase(expansion_path);
                bool equals = lowercase_expansion == lowercase_expansion_name;

                if (equals) return;

                //free(lowercase_expansion);
                //free(lowercase_expansion_name);
            }
            chain.Add(expansion_path);
        }

    }

}
