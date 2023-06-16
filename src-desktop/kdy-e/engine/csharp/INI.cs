using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Text;

namespace Settings;

internal class INI {
    private const string NEW_LINE = "\r\n";

    private readonly string path;
    private readonly List<Section> sections = new List<Section>();

    internal INI(string path) {
        this.path = path;
        Reload();
    }


    private void ParseFromFile(List<Section> list_sections) {
        using (FileStream fs = new FileStream(
                path,
                FileMode.OpenOrCreate,
                FileAccess.Read,
                FileShare.ReadWrite | FileShare.Inheritable
            )
        ) {
            using (StreamReader sr = new StreamReader(fs, Encoding.UTF8)) {
                StringBuilder comments_buffer = null;
                Section current_section = new Section();

                while (!sr.EndOfStream) {
                    string line = sr.ReadLine();

                    if (line.Length < 1 || IsEmptyOrComment(line)) {
                        if (comments_buffer == null) {
                            comments_buffer = new StringBuilder(128);
                            if (line.Length < 1) continue;
                        }

                        if (line.Length < 1) {
                            comments_buffer.Append(NEW_LINE);
                            continue;
                        }

                        if (comments_buffer.Length > 0) comments_buffer.Append(NEW_LINE);
                        comments_buffer.Append(line);
                        continue;
                    }

                    string section_name = Section.IsSectionLine(line);
                    if (section_name == null) {
                        if (comments_buffer != null) {
                            current_section.AddEntry(comments_buffer.ToString(), true);
                            comments_buffer = null;
                        }

                        current_section.AddEntry(line, false);
                    } else {
                        list_sections.Add(current_section);

                        current_section = new Section();
                        current_section.name = section_name;

                        if (comments_buffer == null)
                            current_section.comment = null;
                        else if (comments_buffer.Length > 0)
                            current_section.comment = comments_buffer.ToString();

                        comments_buffer = null;
                    }
                }

                list_sections.Add(current_section);
            }
        }
    }

    internal void Reload() {
        ParseFromFile(sections);
    }

    internal void ReloadUnModifiedSections() {
        List<Section> reloaded_sections = new List<Section>(sections.Capacity);
        ParseFromFile(reloaded_sections);

        for (int i = 0 ; i < reloaded_sections.Count ; i++) {
            string reloaded_name = reloaded_sections[i].name;

            foreach (Section section_modified in sections)
                if (section_modified.modified && section_modified.name == reloaded_name)
                    reloaded_sections[i] = section_modified;
        }

        sections.Clear();
        sections.AddRange(reloaded_sections);
    }

    internal void Flush() {
        using (FileStream fs = new FileStream(
                path,
                FileMode.Create,
                FileAccess.Write,
                FileShare.ReadWrite | FileShare.Inheritable
            )
        ) {
            using (StreamWriter sw = new StreamWriter(fs, Encoding.UTF8)) {
                sw.NewLine = NEW_LINE;

                foreach (Section section in sections) {
                    if (section.name == null && section.entries.Count < 1) continue;

                    // section comment header
                    if (section.comment != null)
                        sw.WriteLine(section.comment);

                    // section name
                    if (section.name != null)
                        sw.WriteLine(Section.TOKEN_OPEN + section.name + Section.TOKEN_CLOSE);

                    // entries
                    foreach (Entry entry in section.entries)
                        sw.WriteLine(entry.content);

                    sw.WriteLine();
                }
            }
        }
    }

    private bool IsEmptyOrComment(string line) {
        for (int i = 0 ; i < line.Length ; i++) {
            char c = line[i];
            if (Char.IsWhiteSpace(c)) continue;
            return c == ';';
        }

        return true;
    }

    #region Sections methods

    private Section GetSection(string name, bool createIfNotExists) {
        foreach (Section section in sections)
            if (section.name == name)
                return section;

        if (!createIfNotExists) return null;

        Section new_section = new Section();
        new_section.name = name;
        sections.Add(new_section);

        return new_section;
    }

    internal bool RenameSection(string oldName, string newName) {
        if (newName == null) throw new ArgumentNullException("newName");

        Section section = GetSection(oldName, false);
        if (section == null) return false;

        section.name = newName;
        return true;
    }

    internal bool HasSection(string name) {
        for (int i = 0 ; i < sections.Count ; i++) {
            if (sections[i].name == name) return true;
        }

        return false;
    }

    internal string[] GetSections() {
        string[] names = new string[sections.Count];
        for (int i = 0 ; i < names.Length ; i++)
            names[i] = sections[i].name;

        return names;
    }

    internal string[] GetSections(string prefixName) {
        List<string> names = new List<string>(sections.Count);
        for (int i = 0 ; i < sections.Count ; i++) {
            string name = sections[i].name;
            if (name != null && name.StartsWith(prefixName)) names.Add(name);
        }

        return names.ToArray();
    }

    internal bool DeleteSection(string name) {
        for (int i = 0 ; i < sections.Count ; i++) {
            if (sections[i].name != name) continue;
            sections.RemoveAt(i);
            return true;
        }

        return false;
    }

    internal void CreateSection(string name) {
        Section section = GetSection(name, false);
        if (section == null)
            sections.Add(
                new Section() { name = name }
            );
    }

    internal bool SetSectionComment(string section, string comment) {
        Section sec = GetSection(section, false);
        if (sec == null) return false;

        sec.SetComment(comment);
        return true;
    }

    internal string GetSectionComment(string section) {
        Section sec = GetSection(section, false);
        if (sec == null) return null;

        return sec.comment;
    }

    internal void DeleteAllSections(string prefixName) {
        if (sections == null || sections.Count < 1) return;

        if (String.IsNullOrEmpty(prefixName))
            throw new ArgumentException("invalid prefix", "prefixName");

        for (int i = sections.Count - 1 ; i >= 0 ; i--)
            if (sections[i].name.StartsWith(prefixName))
                sections.RemoveAt(i);
    }

    #endregion

    #region Getters methods

    internal string GetString(string section, string key, string @default = null) {
        Section sec = GetSection(section, false);
        if (sec == null) return @default;
        string value = sec.GetValue(key);

        return value == null ? @default : value;
    }

    internal int GetInt(string section, string key, int @default = 0) {
        string value = GetString(section, key);
        if (value == null)
            return @default;
        else
            return Int32.Parse(value);
    }

    internal long GetLong(string section, string key, long @default = 0) {
        string value = GetString(section, key);
        if (value == null)
            return @default;
        else
            return Int64.Parse(value);

    }

    internal bool GetBool(string section, string key, bool @default = false) {
        string value = GetString(section, key);
        if (value == "1") return true;
        else if (value == "0") return false;
        else return @default;
    }

    internal byte GetByte(string section, string key, byte @default = 0) {
        string value = GetString(section, key);
        if (value == null)
            return @default;
        else
            return Byte.Parse(value);
    }

    internal float GetFloat(string section, string key, float @default = 0) {
        string value = GetString(section, key);
        if (value == null)
            return @default;
        else
            return Single.Parse(value, CultureInfo.InvariantCulture);
    }

    internal byte[] GetData(string section, string key, byte[] @default = null) {
        string value = GetString(section, key);
        if (value == null) return @default;
        if (value.Length < 1) return new byte[0];

        if ((value.Length % 2) != 0) value = "0" + value;

        byte[] data = new byte[value.Length / 2];

        for (int i = 0 ; i < value.Length ; i += 2) {
            data[i / 2] = Byte.Parse(
                value.Substring(i, 2),
                NumberStyles.HexNumber | NumberStyles.AllowHexSpecifier,
                CultureInfo.InvariantCulture
            );
        }

        return data;
    }

    #endregion

    #region Setters methods

    internal void SetString(string section, string key, string value) {
        Section sec = GetSection(section, true);
        if (sec == null) return;
        sec.SetValue(key, value);
    }

    internal void SetInt(string section, string key, int value) {
        SetString(section, key, value.ToString(CultureInfo.InvariantCulture));
    }

    internal void SetLong(string section, string key, long value) {
        SetString(section, key, value.ToString(CultureInfo.InvariantCulture));
    }

    internal void SetBool(string section, string key, bool value) {
        if (value)
            SetString(section, key, "1");
        else
            SetString(section, key, "0");
    }

    internal void SetByte(string section, string key, byte value) {
        SetString(section, key, value.ToString(CultureInfo.InvariantCulture));
    }

    internal void SetFloat(string section, string key, float value) {
        SetString(section, key, value.ToString(CultureInfo.InvariantCulture));
    }

    internal void SetData(string section, string key, byte[] value) {
        if (value == null) {
            SetString(section, key, null);
            return;
        }

        StringBuilder str = new StringBuilder(value.Length * 2);
        foreach (byte val in value)
            str.Append(val.ToString("X2", CultureInfo.InvariantCulture));

        SetString(section, key, str.ToString());
    }

    #endregion

    #region Entry methods

    internal bool DeleteKey(string sectionName, string keyName) {
        Section section = GetSection(sectionName, false);
        if (section == null) return false;

        for (int i = 0 ; i < section.entries.Count ; i++) {
            Entry entry = section.entries[i];
            if (entry.IsComment) continue;
            if (entry.Key != keyName) continue;

            section.entries.RemoveAt(i);
            return true;
        }

        return false;
    }

    internal string ExchangeKey(string section, string key, string @default) {
        Section sec = GetSection(section, true);

        string value = sec.GetValue(key);
        if (value != null) return value;

        sec.entries.Add(new Entry(key, @default));
        return @default;
    }

    internal bool HasKey(string sectionName, string keyName) {
        Section section = GetSection(sectionName, false);
        if (section == null) return false;

        return section.GetValue(keyName) != null;
    }

    #endregion


    [DebuggerDisplay("\\{name={name} entries={entries.Count}\\}", Target = typeof(string))]
    private class Section {
        internal const char TOKEN_OPEN = '[';
        internal const char TOKEN_CLOSE = ']';

        internal string name;
        internal string comment = null;
        internal bool modified;
        internal readonly List<Entry> entries = new List<Entry>();


        private Entry IndexOf(string key) {
            if (entries.Count < 1) return null;

            foreach (Entry entry in entries) {
                if (entry.IsComment) continue;
                if (entry.Key == key) return entry;
            }

            return null;
        }

        internal string GetValue(string key) {
            Entry entry = IndexOf(key);
            if (entry == null) return null;
            return entry.Value;
        }

        internal void SetValue(string key, string value) {
            Entry entry = IndexOf(key);

            if (entry == null) {
                if (value == null) return;
                entries.Add(new Entry(key, value));
            } else {
                if (value == null)
                    entries.Remove(entry);
                else
                    entry.Value = value;
            }

            modified = true;
        }

        internal void SetComment(string value) {
            if (value == null || value.Length < 1) {
                comment = value;
                return;
            }

            string line_break;

            if (comment.IndexOf("\r\n") >= 0)
                line_break = "\r\n";
            else if (comment.IndexOf('\n') >= 0)
                line_break = "\n";
            else if (comment.IndexOf('\r') >= 0)
                line_break = "\r";
            else
                line_break = null;

            if (line_break == null)
                comment = Entry.COMMENT_TOKEN + value;
            else
                comment = Entry.COMMENT_TOKEN + value.Replace(line_break, line_break + Entry.COMMENT_TOKEN);
        }

        internal void AddEntry(string value, bool isComment) {
            entries.Add(new Entry(value, isComment));
        }

        internal static string IsSectionLine(string line) {
            int open = -1;
            int close = -1;

            for (int i = 0 ; i < line.Length ; i++) {
                char c = line[i];
                if (Char.IsWhiteSpace(c)) continue;

                if (c == TOKEN_OPEN) {
                    open = i + 1;
                    break;
                }
            }
            if (open < 0) return null;

            for (int i = line.Length - 1 ; i >= open ; i--) {
                char c = line[i];
                if (Char.IsWhiteSpace(c)) continue;

                if (c == TOKEN_CLOSE) {
                    close = i;
                    break;
                }
            }
            if (close < 0) return null;

            if ((open + 1) == close) return null;

            return line.Substring(open, close - open);
        }

        /*
        internal string GetComment()
        {
            StringBuilder comment = new StringBuilder(128);

            foreach (Entry entry in entries)
                if (entry.IsComment)
                    comment.AppendLine(entry.content);

            return comment.ToString();
        }

        internal void SetComment(string comment)
        {
            int remove = 0;
            Entry target = null;

            foreach (Entry entry in entries)
            {
                if (!entry.IsComment) break;

                if (target == null)
                    target = entry;
                else
                    remove++;
            }

            if (target == null)
            {
                string line_break;

                if (comment.IndexOf("\r\n") >= 0) line_break = "\r\n";
                else if (comment.IndexOf('\n') >= 0) line_break = "\n";
                else if (comment.IndexOf('\r') >= 0) line_break = "\r";
                else
                {
                    entries.Insert(0, new Entry(comment, true));
                    return;
                }

                comment = comment.Replace(line_break, line_break + Entry.COMMENT_TOKEN);
                entries.Insert(0, new Entry(comment, true));
            }
        }
        */
    }

    [DebuggerDisplay("\\{{content}\\}")]
    private class Entry {
        private const char KEY_TOKEN = '=';
        internal const char COMMENT_TOKEN = ';';

        private readonly int at;
        internal string content;

        internal Entry(string str, bool isComment) {
            content = str;

            if (isComment)
                at = -1;
            else
                at = content.IndexOf(KEY_TOKEN);
        }

        internal Entry(string key, string value) {
            Create(key, value);
            at = key.Length;
        }

        internal void Create(string key, string value) {
            content = key + KEY_TOKEN + value;
        }

        internal bool IsComment => at < 1;


        internal string Key => content.Substring(0, at);

        internal string Value {
            get => content.Substring(at + 1);
            set => Create(Key, value);
        }

        /*
        internal static bool IsCommentLine(string line)
        {
            for (int i = 0; i < line.Length; i++)
            {
                char c = line[i];
                if (Char.IsWhiteSpace(c)) continue;
                return c == COMMENT_TOKEN;
            }

            return false;
        }
        */

    }

}
