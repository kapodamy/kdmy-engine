namespace Engine.Font;

using System;
using System.IO;
using Engine.Externals.FontAtlasInterop;


public static class FontCache {
    private const string FOLDER = ".fontcache";

    public static FontCharMap Load(FontAtlas fontatlas, byte font_height, ushort revision, string suffix) {
        ulong xxhash = fontatlas.Xxhash();
        string cache_path = InternalCreatePath(xxhash, suffix);

        if (!File.Exists(cache_path)) {
            return null;
        }

        byte[] cache = File.ReadAllBytes(cache_path);
        return fontatlas.DeserializeAtlas(cache, font_height, revision);
    }

    public static void Store(FontAtlas fontatlas, FontCharMap charmap, byte font_height, ushort revision, string suffix) {
        ulong xxhash = fontatlas.Xxhash();
        string cache_path = InternalCreatePath(xxhash, suffix);

        string cache_folder_path = Path.GetDirectoryName(cache_path);
        if (!Directory.Exists(cache_folder_path)) {
            try {
                DirectoryInfo cache_folder = Directory.CreateDirectory(cache_folder_path);
                cache_folder.Attributes |= FileAttributes.Hidden;
            } catch (Exception e) {
                Logger.Error($"fontcache_store() {e.Message}");
            }
        }

        byte[] cache = fontatlas.SerializeAtlas(charmap, font_height, revision);
        File.WriteAllBytes(cache_path, cache);
    }


    private static string InternalCreatePath(ulong xxhash, string suffix) {
        string base_path = CsharpWrapper.EngineSettings.EngineDir;
        string xxhash_str = xxhash.ToString("X").PadLeft(sizeof(ulong) * 2, '0');

        return $"{base_path}{FontCache.FOLDER}{Path.DirectorySeparatorChar}{xxhash_str}{suffix}.bin";
    }

}
