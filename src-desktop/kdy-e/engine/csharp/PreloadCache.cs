using System;
using System.Diagnostics;
using System.IO;
using Engine.Utils;

namespace Engine.Platform;

internal static class PreloadCache {
    public const string PRELOAD_FILENAME = "preload.ini";
    private const char COMMENT_CHAR = ';';
    private const int MAX_MEMORY = 512 * 1024 * 1024;// 512MiB
    private const int CACHE_MAX_ENTRIES = 512;

    private static CacheEntry[] cache = new CacheEntry[CACHE_MAX_ENTRIES];
    private static int memory_used = 0;
    private static int entries_count = 0;
    private static int ids = 0;


    public static int AddFileList(string src_filelist) {
        if (memory_used >= MAX_MEMORY || entries_count >= cache.Length) {
            return -1;
        }

        src_filelist = FS.ResolvePath(src_filelist);

        string filelist_absolute_path = IO.GetAbsolutePath(src_filelist, true, false, false);
        if (!File.Exists(filelist_absolute_path)) return -1;

        string filelist = File.ReadAllText(filelist_absolute_path);
        if (StringUtils.IsEmpty(filelist)) return -1;

        Logger.Info($"PreloadCache::AddFileList() reading {src_filelist}");

        Stopwatch stopwatch = new Stopwatch();
        Tokenizer tokenizer = Tokenizer.Init("\r\n", true, false, filelist);
        int added = 0;
        int id = ids++;
        string line;

        stopwatch.Start();
        while ((line = tokenizer.ReadNext()) != null) {
            if (line[0] == COMMENT_CHAR) continue;

            string path = FS.BuildPath2(src_filelist, line);
            if (path.StartsWith(FS.ASSETS_FOLDER)) path = FS.GetFullPathAndOverride(path);

            string absolute_path = IO.GetAbsolutePath(path, false, true, true);
            if (Directory.Exists(absolute_path)) {
                foreach (string file in ListFilesOfFolder(absolute_path)) {
                    if (!AddFileToCache(line, path, file, id, ref added)) {
                        goto L_return;
                    }
                }
                continue;
            }

            absolute_path = IO.GetAbsolutePath(path, true, false, true);
            if (!File.Exists(absolute_path)) {
                Logger.Warn($"PreCache::AddFileList() file not found {line} (resolved as {path})");
                continue;
            }

            if (!AddFileToCache(line, path, absolute_path, id, ref added)) break;
        }

        double elapsed = stopwatch.ElapsedMilliseconds / 1000.0;

        if (added < 1)
            Logger.Info($"PreloadCache::AddFileList() no files cached (took {elapsed}sec)");
        else
            Logger.Info($"PreloadCache::AddFileList() {added} files cached in {elapsed}sec");
L_return:
        tokenizer.Destroy();
        return added < 1 ? -1 : id;
    }

    public static void ClearCache() {
        if (entries_count < 1) return;

        for (int i = 0 ; i < entries_count ; i++) {
            cache[i].Drop();
        }

        entries_count = 0;
        memory_used = 0;
    }

    public static void ClearById(int id) {
        if (id < 0) return;

        int old_count = entries_count;
        for (int i = 0 ; i < old_count ; i++) {
            if (cache[i].id == id && cache[i].data != null) {
                cache[i].data.Dispose();
                cache[i].data = null;
                entries_count--;
            }
        }

        CacheEntry[] new_cache = new CacheEntry[CACHE_MAX_ENTRIES];
        for (int i = 0, j = 0 ; i < old_count ; i++) {
            if (cache[i].id < 0 || cache[i].data == null) continue;
            new_cache[j++] = cache[i];
        }

        cache = new_cache;
    }

    public static Stream RetrieveStream(string absolute_path) {
        // Note: this can fail because the comparision is case sensitive
        for (int i = 0 ; i < entries_count ; i++) {
            if (cache[i].absolute_path == absolute_path) {
                cache[i].data.Seek(0, SeekOrigin.Begin);
                return cache[i].data;
            }
        }
        return null;
    }

    public static byte[] RetrieveBuffer(string absolute_path) {
        // Note: this can fail because the comparision is case sensitive
        for (int i = 0 ; i < entries_count ; i++) {
            if (cache[i].absolute_path == absolute_path) {
                return cache[i].buffer;
            }
        }
        return null;
    }


    private static string[] ListFilesOfFolder(string folder_absolute_path) {
        return Directory.GetFiles(folder_absolute_path, "*.*", SearchOption.AllDirectories);
    }

    private static bool AddFileToCache(string src, string path, string absolute_path, int id, ref int added) {
        for (int i = 0 ; i < entries_count ; i++) {
            if (cache[i].absolute_path == absolute_path) {
                // already added
                return true;
            }
        }

        try {
            byte[] buffer = File.ReadAllBytes(absolute_path);

            cache[entries_count].absolute_path = absolute_path;
            cache[entries_count].data = new MemoryStream(buffer, 0, buffer.Length, false, true);
            cache[entries_count].id = id;

            entries_count++;
            memory_used += buffer.Length;
            added++;
        } catch (OutOfMemoryException) {
            Logger.Warn($"PreCache::AddFileList() out of memory for {src} (resolved as {path})");
            return false;
        } catch (Exception e) {
            Logger.Warn($"PreCache::AddFileList() exception for {src} (resolved as {path}):\n{e.Message}");
            return true;
        }

        return memory_used < MAX_MEMORY && entries_count < cache.Length;
    }


    [DebuggerDisplay("id={id} absolute_path={absolute_path} data={data}")]
    private struct CacheEntry {
        public int id;
        public string absolute_path;
        public MemoryStream data;
        public byte[] buffer { get => data?.GetBuffer(); }

        public void Drop() {
            if (data != null) data.Dispose();
            data = null;
            id = -1;
            absolute_path = null;
        }
    }

}
