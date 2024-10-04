using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Engine.Utils;


public class IndexListingCreator {

    private enum IndexListingEntryType {
        directory,
        file
    }

    private abstract class IndexListingEntry {
        public string name { get; set; }
        public IndexListingEntryType type { get; set; }
    }

    private class IndexListingEntryFile : IndexListingEntry {
        public long fileLength { get; set; }
    }

    private class IndexListingEntryDirectory : IndexListingEntry {
        public int directoryIndex { get; set; }
    }

    private class IndexListing {
        public object[][] directories { get; set; }
        public int rootIndex { get; set; }
    }

    private struct StackedDirectory {
        public DirectoryInfo info;
        public int index;
    }


    public static void SerializeDirectory(string path) {
        List<object[]> table = new List<object[]>(256);
        Stack<StackedDirectory> stack = new Stack<StackedDirectory>(256);

        table.Add(new IndexListingEntry[]{
            new IndexListingEntryDirectory() { name = "assets", directoryIndex = 1},
            new IndexListingEntryDirectory() { name = "expansions", directoryIndex = 2},
        });

        // stack assets directory
        stack.Push(new StackedDirectory() {
            info = new DirectoryInfo(Path.Combine(path, "assets")),
            index = 1
        });
        table.Add(null);

        // stack expansions directory
        stack.Push(new StackedDirectory() {
            info = new DirectoryInfo(Path.Combine(path, "expansions")),
            index = 2
        });
        table.Add(null);

        int table_index = 3;
        while (stack.Count > 0) {
            StackedDirectory current = stack.Pop();
            if (!current.info.Exists) {
                table[current.index] = new IndexListingEntry[0];
                continue;
            }

            List<IndexListingEntry> entries = new List<IndexListingEntry>(128);

            foreach (FileInfo file in current.info.EnumerateFiles()) {
                entries.Add(new IndexListingEntryFile() {
                    name = file.Name,
                    type = IndexListingEntryType.file,
                    fileLength = file.Length
                });
            }
            foreach (DirectoryInfo directory in current.info.EnumerateDirectories()) {
                entries.Add(new IndexListingEntryDirectory() {
                    name = directory.Name,
                    type = IndexListingEntryType.directory,
                    directoryIndex = table_index
                });
                stack.Push(new StackedDirectory() {
                    info = directory,
                    index = table_index
                });
                table.Add(null);
                table_index++;
            }

            entries.Sort(delegate (IndexListingEntry a, IndexListingEntry b) {
                return StringUtils.naturalStringComparer.Compare(a.name, b.name);
            });

            table[current.index] = entries.ToArray();
        }

        // create IndexListing
        IndexListing indexListing = new IndexListing() {
            rootIndex = 0,
            directories = table.ToArray()
        };

        JsonSerializerOptions options = new JsonSerializerOptions() {
            WriteIndented = true,
            Converters = { new JsonStringEnumConverter() }
        };

        byte[] json = JsonSerializer.SerializeToUtf8Bytes(indexListing, options);
        string output = Path.Combine(path, "indexlisting.json");

        Logger.Info($"{table_index} entries indexed to {output}");
        File.WriteAllBytes(output, json);
    }

}

