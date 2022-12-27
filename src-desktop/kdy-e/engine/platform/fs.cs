using System;
using System.IO;
using System.Text;
using Engine.Externals;
using Engine.Utils;
using KallistiOS.TLS;

namespace Engine.Platform {

    public class FSFolderEnumerator {
        internal struct Entry {
            public string name;
            public long length;
        }

        internal int ___index = 0;
        internal Entry[] ___entries = null;

        public string name = null;
        public bool is_file = false;
        public bool is_folder = false;
        public long length = -1;

        internal static void EnumerateTo(ArrayList<Entry> entries, string target_folder, bool resolve_expansion) {
            string path = IO.GetAbsolutePath(target_folder, false, resolve_expansion);
            DirectoryInfo directoryInfo = new DirectoryInfo(path);
            if (!directoryInfo.Exists) return;

            DirectoryInfo[] di_array = directoryInfo.GetDirectories();
            FileInfo[] fi_array = directoryInfo.GetFiles();

            for (int i = 0 ; i < di_array.Length ; i++) {
                AddUnique(entries, di_array[i].Name, -1);
            }
            for (int i = 0 ; i < fi_array.Length ; i++) {
                AddUnique(entries, fi_array[i].Name, fi_array[i].Length);
            }
        }

        private static void AddUnique(ArrayList<Entry> entries, string name, long length) {
            foreach (Entry entry in entries) {
                if (String.Equals(entry.name, name, StringComparison.OrdinalIgnoreCase)) {
                    return;
                }
            }
            entries.Add(new Entry() { name = name, length = length });
        }

        internal static int Compare(Entry entry1, Entry entry2) {
            return StringUtils.naturalStringComparer.Compare(entry1.name, entry2.name);
        }
    }


    public static class FS {

        public const char CHAR_SEPARATOR = '/';
        public const char CHAR_SEPARATOR_REJECT = '\\';
        private const string FILENAME_INVALID_CHARS = "<>:\"/\\|?*";
        public const string ASSETS_FOLDER = "/assets";
        public const string ASSETS_COMMON_FOLDER = "/assets/common/";
        public const string NO_OVERRIDE_COMMON = "/~assets/common/";

        private static bool fs_tls_init = true;
        private static kthread_key_t<FSTLS> fs_tls_key = new kthread_key_t<FSTLS>();
        private static string fs_cod = null;

        private class FSTLS {
            public string fs_cwd;
            public LinkedList<string> fs_stk;
        }

        public static void Init() {
            if (fs_tls_init) {
                // first run, initialize the thread local storage key
                kthread.key_create<FSTLS>(fs_tls_key, Destroy);
                fs_tls_init = false;
            }

            if (kthread.getspecific<FSTLS>(fs_tls_key) != null) {
                throw new InvalidOperationException("Duplicate call to fs_init()");
            }

            FSTLS fs_tls = new FSTLS() {
                fs_cwd = FS.ASSETS_FOLDER,
                fs_stk = new LinkedList<string>()
            };

            kthread.setspecific(fs_tls_key, fs_tls);
        }

        private static void Destroy(FSTLS fs_tls) {
            //free(fs_tls.fs_cwd);
            fs_tls.fs_stk.Destroy2(/*free*/);
            //free(fs_tls);
        }


        public static string ReadText(string src) {
            src = FS.GetFullPathAndOverride(src);
            return IO.ReadText(src);
        }

        public static byte[] ReadArrayBuffer(string src) {
            src = FS.GetFullPathAndOverride(src);
            return IO.ReadArrayBuffer(src);
        }

        public static ImageData ReadTexture(string src) {
            src = FS.GetFullPathAndOverride(src);
            return IO.ReadTexture(src);
        }


        public static bool FileExists(string src) {
            src = FS.GetFullPathAndOverride(src);
            return IO.ResourceExists(src, true, false);
        }

        public static bool FolderExists(string src) {
            src = FS.GetFullPathAndOverride(src);
            return IO.ResourceExists(src, false, true);
        }

        public static long FileLength(string src) {
            src = FS.GetFullPathAndOverride(src);
            return IO.FileSize(src);
        }

        public static bool FolderEnumerate(string src, FSFolderEnumerator folder_enumerator) {
            try {
                ArrayList<FSFolderEnumerator.Entry> entries = new ArrayList<FSFolderEnumerator.Entry>();

                // this is a disaster, enumerate src under "/expansions" too
                if (!src.StartsWith("/~assets/")) {
                    for (int i = Expansions.chain_array_size - 1 ; i >= 0 ; i--) {
                        string path = Expansions.GetPathFromExpansion(src, i);
                        if (path == null) continue;

                        FSFolderEnumerator.EnumerateTo(entries, path, true);
                    }
                }

                // now enumerate the resqueted folder
                FSFolderEnumerator.EnumerateTo(entries, src, false);

                if (entries.Size() < 1) {
                    entries.Destroy(false);
                    return false;
                }

                folder_enumerator.___index = 0;
                entries.Destroy2(out _, ref folder_enumerator.___entries);

                // sort filenames if one or more expansions was enumerated
                Array.Sort(folder_enumerator.___entries, FSFolderEnumerator.Compare);

                return true;
            } catch (Exception e) {
                Console.Error.WriteLine("fs_folder_enumerate() failed on: {0}.\n{1}", src, e.Message);

                folder_enumerator.___entries = null;
                folder_enumerator.name = null;
                return false;
            }
        }

        public static bool FolderEnumerateNext(FSFolderEnumerator folder_enumerator) {
            if (folder_enumerator.___entries == null) return false;
            if (folder_enumerator.___index >= folder_enumerator.___entries.Length) return false;

            var entry = folder_enumerator.___entries[folder_enumerator.___index++];
            folder_enumerator.name = entry.name;
            folder_enumerator.is_file = entry.length >= 0;
            folder_enumerator.is_folder = entry.length < 0;
            return true;
        }

        public static void FolderEnumerateClose(FSFolderEnumerator folder_enumerator) {
            folder_enumerator.___entries = null;
            folder_enumerator.___index = -1;
            folder_enumerator.name = null;
            folder_enumerator.is_file = false;
            folder_enumerator.is_folder = false;
        }

        public static bool IsInvalidFilename(string filename) {
            for (int i = 0 ; i < FS.FILENAME_INVALID_CHARS.Length ; i++) {
                for (int j = 0 ; j < filename.Length ; j++) {
                    if (filename[j] == FS.FILENAME_INVALID_CHARS[i])
                        return true;
                }
            }
            return false;
        }

        public static string GetParentFolder(string filename) {
            if (filename == null) throw new Exception("filename is null");

            int index = filename.LastIndexOf(FS.CHAR_SEPARATOR);
            if (index < 1) return "";
            return filename.SubstringKDY(0, index);
        }

        public static string BuildPath(string base_path, string filename) {
            if (filename == null) throw new Exception("filename is null");

            if (String.IsNullOrWhiteSpace(base_path) || filename[0] == FS.CHAR_SEPARATOR) return filename;

            int base_index = base_path.Length;
            if (base_path[base_index - 1] == FS.CHAR_SEPARATOR) base_index--;

            return base_path.SubstringKDY(0, base_index) + FS.CHAR_SEPARATOR + filename;
        }


        public static string BuildPath2(string reference_filename, string filename) {
            if (filename == null) throw new Exception("filename is null");

            FSTLS fs_tls = kthread.getspecific<FSTLS>(fs_tls_key);

            if (String.IsNullOrWhiteSpace(reference_filename) || filename[0] == FS.CHAR_SEPARATOR) return filename;

            int reference_parent_index = reference_filename.LastIndexOf(FS.CHAR_SEPARATOR);
            if (reference_parent_index < 0) {
                // the "reference_filename" does not have any parent folder, use the working directory
                return BuildPath(fs_tls.fs_cwd, filename);
            }

            // build the path using the reference parent folder
            // example: reference=/foobar/atlas.xml filename=texture.png result=/foobar/texture.png
            return reference_filename.SubstringKDY(0, reference_parent_index + 1) + filename;
        }

        public static string GetFilenameWithoutExtension(string filename) {
            if (filename == null) return null;
            if (filename.Length < 1) return "";

            int reference_parent_index = filename.LastIndexOf(FS.CHAR_SEPARATOR);
            int extension_index = filename.LastIndexOf('.');

            if (reference_parent_index < 0) reference_parent_index = 0;
            else reference_parent_index++;

            if (extension_index < reference_parent_index) extension_index = filename.Length;

            return filename.SubstringKDY(reference_parent_index, extension_index);
        }

        public static string ResolvePath(string src) {
            if (src == null) throw new IOException("fs_resolve_path() failed, the src was null");

            int src_length = src.Length;
            if (src.Length < 1) return "";

            if (src.IndexOf(FS.CHAR_SEPARATOR_REJECT, 0) >= 0) {
                throw new IOException("fs_resolve_path() path has invalid separator char: " + src);
            }

            if (src_length == 1) {
                if (src[0] == '.') {
                    // path is "." (current directory) or root directory "/"
                    return "";
                } else {
                    // match "/" and single-character files and/or folders 
                    return src;
                }
            }

            int stack_length = src.OccurrencesOfKDY(FS.CHAR_SEPARATOR) + 2;
            int[] stack = new int[stack_length];
            int stack_size = 0;
            int index = 0;
            int last_index = 0;
            int size;

            while (index >= 0 && index < src_length) {
                index = src.IndexOf(FS.CHAR_SEPARATOR, last_index);

                if (index < 0) {
                    size = src_length - last_index;
                } else {
                    size = index - last_index;
                    index++;
                }

                switch (size) {
                    case 0:
                        last_index = index;
                        continue;
                    case 1:
                        if (src[last_index] == '.') {
                            // current directory token
                            last_index = index;
                            continue;
                        }
                        break;
                    case 2:
                        if (src[last_index] == '.' && src[last_index + 1] == '.') {
                            // goto parent directory
                            stack_size--;
                            if (stack_size < 0) goto L_exit;
                            last_index = index;
                            continue;
                        }
                        break;
                }

                // store current part
                stack[stack_size] = last_index;
                if (stack_size < stack_length) stack_size++;

                last_index = index;
            }

L_exit:
            if (stack_size < 0) {
                // the path probably points ouside of assets folders
                //free(stack);
                return FS.CHAR_SEPARATOR.ToString();
            }

            // compute the final path
            StringBuilder builder = new StringBuilder(src_length + 2);

            if (src[0] == FS.CHAR_SEPARATOR) builder.Append(FS.CHAR_SEPARATOR);

            for (int i = 0 ; i < stack_size ; i++) {
                int j = src.IndexOf(FS.CHAR_SEPARATOR, stack[i]);
                if (j < 0) j = src_length;

                if (i > 0) builder.Append(FS.CHAR_SEPARATOR);
                builder.AddSubstringKDY(src, stack[i], j);
            }

            string path = builder.ToString();
            //stringbuilder_destroy(&builder);

            //free(stack);
            return path;
        }



        public static void SetWorkingFolder(string base_path, bool get_parent_of_base_path) {
            FSTLS fs_tls = kthread.getspecific<FSTLS>(fs_tls_key);

            if (base_path == null) {
                //free(fs_tls.fs_cwd);
                fs_tls.fs_cwd = FS.ASSETS_COMMON_FOLDER;
                return;
            }

            string temp_path;
            if (get_parent_of_base_path)
                temp_path = FS.GetParentFolder(base_path);
            else
                temp_path = base_path;

            // check if the parent directory is the current working directory
            if (temp_path.Length < 1) {
                //free(temp_path);
                return;
            }

            string resolved_path;
            if (temp_path[0] != FS.CHAR_SEPARATOR)
                resolved_path = FS.BuildPath(fs_tls.fs_cwd, temp_path);
            else
                resolved_path = FS.BuildPath(FS.ASSETS_FOLDER, temp_path);

            //free(temp_path);
            temp_path = FS.ResolvePath(resolved_path);
            //free(resolved_path);

            if (!temp_path.StartsWith(FS.ASSETS_FOLDER)) {
                throw new IOException("fs_set_working_folder() failed for: " + base_path);
            }

            //free(fs_tls.fs_cwd);
            fs_tls.fs_cwd = temp_path;
        }

        public static void SetWorkingSubfolder(string sub_path) {
            FSTLS fs_tls = kthread.getspecific<FSTLS>(fs_tls_key);

            string temp_path = FS.BuildPath(fs_tls.fs_cwd, sub_path);
            string resolved_path = FS.ResolvePath(temp_path);
            //free(temp_path);

            if (!resolved_path.StartsWith(FS.ASSETS_FOLDER)) {
                throw new IOException("fs_set_working_subfolder() failed, cwd=" + fs_tls.fs_cwd + " sub_path=" + sub_path);
            }

            //free(fs_tls.fs_cwd);
            fs_tls.fs_cwd = resolved_path;
        }

        public static string GetFullPath(string path) {
            if (String.IsNullOrWhiteSpace(path)) return FS.ASSETS_FOLDER;

            FSTLS fs_tls = kthread.getspecific<FSTLS>(fs_tls_key);

            string temp_path = FS.BuildPath(fs_tls.fs_cwd, path);
            string resolved_path = FS.ResolvePath(temp_path);
            //free(temp_path);

            if (resolved_path.Length < 1) {
                //free(resolved_path);
                return fs_tls.fs_cwd;
            }

            if (resolved_path[0] != FS.CHAR_SEPARATOR) {
                // expected "/***"
                throw new IOException("fs_get_full_path() failed cwd=" + fs_tls.fs_cwd + " path=" + path);
            }

            return resolved_path;
        }

        public static string GetFullPathAndOverride(string path) {
            if (String.IsNullOrWhiteSpace(path)) return FS.ASSETS_FOLDER;

            FSTLS fs_tls = kthread.getspecific<FSTLS>(fs_tls_key);

            string temp_path = FS.BuildPath(fs_tls.fs_cwd, path);
            string resolved_path = FS.ResolvePath(temp_path);
            //free(temp_path);

            if (resolved_path.Length < 1) {
                //free(resolved_path);
                return fs_tls.fs_cwd;
            }

            if (resolved_path[0] != FS.CHAR_SEPARATOR) {
                // expected "/***"
                throw new IOException("fs_get_full_path_and_override() failed cwd=" + fs_tls.fs_cwd + " path=" + path);
            }

            bool no_override = resolved_path.StartsWith(FS.NO_OVERRIDE_COMMON);

            if (fs_cod != null && !no_override && resolved_path.StartsWith(FS.ASSETS_COMMON_FOLDER)) {
                // override "/assets/common/***" --> "/assets/weeks/abc123/custom_common/***"
                string custom_path = StringUtils.Concat(
                    fs_cod,
                    FS.CHAR_SEPARATOR.ToString(),
                    resolved_path.SubstringKDY(FS.ASSETS_COMMON_FOLDER.Length, resolved_path.Length)
                );

                //
                // Note: the resource in origin must match the type in custom
                //       if the file does not exists in origin the behavior is undefined
                //
                bool is_file = true;
                bool is_folder = true;
                if (IO.ResourceExists(resolved_path, true, false)) is_folder = false;
                else if (IO.ResourceExists(resolved_path, false, true)) is_file = false;

                if (IO.ResourceExists(custom_path, is_file, is_folder)) {
                    /*if (DEBUG && is_file && is_folder) {
                        console.warn(
                            "fs_get_full_path_and_override() '" +
                            resolved_path +
                            "' does not exist and will be overrided by '" +
                            custom_path +
                            "'"
                        );
                    }*/
                    //free(resolved_path);
                    return custom_path;
                }

                // the file to override does not exist in the custom common folder
                //free(custom_path);
            } else if (no_override) {
                // replace "/~assets/common/file.txt" --> "/assets/common/file.txt"
                temp_path = FS.CHAR_SEPARATOR + resolved_path.SubstringKDY(2, resolved_path.Length);
                //free(resolved_path);
                return temp_path;
            }

            return resolved_path;
        }

        public static void OverrideCommonFolder(string base_path) {
            //free(fs_cod);

            FSTLS fs_tls = kthread.getspecific<FSTLS>(fs_tls_key);

            if (String.IsNullOrEmpty(base_path)) {
                fs_cod = null;
                return;
            } else if (base_path.StartsWith(FS.ASSETS_FOLDER)) {
                int last_index = base_path.Length;
                if (base_path[last_index - 1] == FS.CHAR_SEPARATOR) last_index = last_index - 1;
                fs_cod = base_path.SubstringKDY(0, last_index);
            } else {
                string temp_path = FS.BuildPath(fs_tls.fs_cwd, base_path);
                fs_cod = FS.ResolvePath(temp_path);
                //free(temp_path);

                if (fs_cod.StartsWith(FS.ASSETS_FOLDER)) return;

                throw new IOException("fs_override_common_folder() failed base_path=" + base_path + " cwd=" + fs_tls.fs_cwd);
            }
        }

        public static void FolderStackPush() {
            FSTLS fs_tls = kthread.getspecific<FSTLS>(fs_tls_key);
            fs_tls.fs_stk.AddItem(fs_tls.fs_cwd);
        }

        public static void FolderStackPop() {
            FSTLS fs_tls = kthread.getspecific<FSTLS>(fs_tls_key);

            int count = fs_tls.fs_stk.Count();
            if (count < 1) {
                Console.Error.WriteLine("fs: fs_folder_stack_pop() failed, folder stack was empty");
                return;
            }

            //free(fs_tls.fs_cwd);
            fs_tls.fs_cwd = fs_tls.fs_stk.RemoveItemAt(count - 1);
        }

    }
}
