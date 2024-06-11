using System;
using System.Diagnostics;
using Engine.Game.Common;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Image;

[DebuggerDisplay("name={name} x={x} y={y} width={width} height={height} frame=\\{ x={frame_x} y={frame_y} width={frame_width} height={frame_height} \\} pivot=\\{ x={pivot_x} y={pivot_y} \\}")]
public class AtlasEntry : ICloneable {
    /// <summary>
    /// Atlas entry name, this field is only valid during the Atlas lifetime
    /// </summary>
    public string name;
    public float x;
    public float y;
    public float width;
    public float height;
    public float frame_x;
    public float frame_y;
    public float frame_width;
    public float frame_height;
    public float pivot_x;
    public float pivot_y;

    public object Clone() => this.MemberwiseClone();
}

public class Atlas {

    public int resolution_width;
    public int resolution_height;
    public bool has_declared_resolution;
    public string texture_filename;
    //public string name;
    public float glyph_fps;
    public int size;
    public AtlasEntry[] entries;

    internal Atlas() { }


    public static Atlas Init(string src) {
        if (src.LowercaseEndsWithKDY(".txt")) {
            return Atlas.ParseFromPlainText(src);
        }

        Atlas atlas = new Atlas();
        XmlParser xml = XmlParser.Init(src);

        if (xml == null) {
            Logger.Error($"atlas_init() error loading {src}");
            return null;
        }

        XmlParserNode xml_atlas = xml.GetRoot();
        Debug.Assert(xml_atlas != null, "the file " + src + " was empty");
        if (xml_atlas.TagName != "TextureAtlas") return null;

        atlas.size = 0;
        atlas.entries = null;
        //atlas.name = src;

        atlas.glyph_fps = VertexProps.ParseFloat(xml_atlas, "glyphFps", Single.NaN);
        if (Single.IsNaN(atlas.glyph_fps)) atlas.glyph_fps = 0.0f;

        atlas.texture_filename = xml_atlas.GetAttribute("imagePath");
        atlas.resolution_width = Funkin.SCREEN_RESOLUTION_WIDTH;
        atlas.resolution_height = Funkin.SCREEN_RESOLUTION_HEIGHT;
        atlas.has_declared_resolution = Atlas.ParseResolution(atlas, xml_atlas.GetAttribute("resolution"));

        if (atlas.texture_filename != null) {
            if (atlas.texture_filename.Length > 0) {
                string path = FS.BuildPath2(src, atlas.texture_filename);
                atlas.texture_filename = path;
            } else {
                atlas.texture_filename = null;
            }
        }

        ArrayList<AtlasEntry> arraylist = new ArrayList<AtlasEntry>();

        foreach (XmlParserNode unparsed_entry in xml_atlas.Children) {
            switch (unparsed_entry.TagName) {
                case "TileSet":
                    Atlas.ParseTileSet(arraylist, unparsed_entry);
                    continue;
                case "SubTexture":
                    break;
                default:
                    Logger.Warn($"atlas_init() unknown TextureAtlas entry: {unparsed_entry.OuterXML}");
                    continue;
            }

            AtlasEntry atlas_entry = new AtlasEntry() {
                name = unparsed_entry.GetAttribute("name"),
                x = VertexProps.ParseInteger2(unparsed_entry.GetAttribute("x"), 0),
                y = VertexProps.ParseInteger2(unparsed_entry.GetAttribute("y"), 0),
                width = VertexProps.ParseInteger2(unparsed_entry.GetAttribute("width"), Math2D.MAX_INT32),
                height = VertexProps.ParseInteger2(unparsed_entry.GetAttribute("height"), Math2D.MAX_INT32),
                frame_x = VertexProps.ParseInteger2(unparsed_entry.GetAttribute("frameX"), 0),
                frame_y = VertexProps.ParseInteger2(unparsed_entry.GetAttribute("frameY"), 0),
                frame_width = VertexProps.ParseInteger2(unparsed_entry.GetAttribute("frameWidth"), 0),
                frame_height = VertexProps.ParseInteger2(unparsed_entry.GetAttribute("frameHeight"), 0),
                pivot_x = 0,// VertexProps.ParseFloat(unparsed_entry.getAttribute("pivotX"), 0.0f),
                pivot_y = 0,// VertexProps.ParseFloat(unparsed_entry.getAttribute("pivotY"), 0.0f),
            };

            Debug.Assert(atlas_entry.width != Math2D.MAX_INT32 || atlas_entry.height != Math2D.MAX_INT32, unparsed_entry.OuterXML);

            if (Single.IsNaN(atlas_entry.frame_x)) atlas_entry.frame_x = 0;
            if (Single.IsNaN(atlas_entry.frame_y)) atlas_entry.frame_y = 0;
            if (Single.IsNaN(atlas_entry.frame_width)) atlas_entry.frame_width = 0;
            if (Single.IsNaN(atlas_entry.frame_height)) atlas_entry.frame_height = 0;
            if (Single.IsNaN(atlas_entry.pivot_x)) atlas_entry.pivot_x = 0.0f;
            if (Single.IsNaN(atlas_entry.pivot_y)) atlas_entry.pivot_y = 0.0f;

            arraylist.Add(atlas_entry);
        }

        xml.Destroy();
        arraylist.Destroy2(out atlas.size, ref atlas.entries);

        return atlas;
    }

    public void Destroy() {
        // if (!this) return;

        for (int i = 0 ; i < this.size ; i++) {
            //free(this.entries[i].name);
            this.entries[i].name = null;
        }

        //free(this.entries);
        this.size = 0;
        this.entries = null;
        //free(this.texture_filename);
        //free(this);
    }

    public int GetIndexOf(string name) {
        for (int i = 0 ; i < this.size ; i++) {
            if (this.entries[i].name == name)
                return i;
        }

        return -1;
    }

    public AtlasEntry GetEntry(string name) {
        int index = GetIndexOf(name);

        return index >= 0 ? this.entries[index] : null;
    }

    public AtlasEntry GetEntryWithNumberSuffix(string name_prefix) {
        int start = name_prefix.Length;

        for (int i = 0 ; i < this.size ; i++) {
            string atlas_entry_name = this.entries[i].name;

            if (
                atlas_entry_name.StartsWith(name_prefix) &&
                Atlas.NameHasNumberSuffix(atlas_entry_name, start)
            )
                return this.entries[i];
        }

        return null;
    }

    public float GetGlyphFPS() {
        return this.glyph_fps;
    }

    public string GetTexturePath() {
        return this.texture_filename;
    }

    public bool Apply(Sprite sprite, string name, bool override_draw_size) {
        int i = GetIndexOf(name);
        if (i < 0) return true;

        Atlas.ApplyFromEntry(sprite, this.entries[i], override_draw_size);
        return false;
    }

    public static void ApplyFromEntry(Sprite sprite, AtlasEntry atlas_entry, bool override_draw_size) {
        sprite.SetOffsetSource(
            atlas_entry.x, atlas_entry.y,
            atlas_entry.width, atlas_entry.height
        );
        sprite.SetOffsetFrame(
            atlas_entry.frame_x, atlas_entry.frame_y,
            atlas_entry.frame_width, atlas_entry.frame_height
        );
        sprite.SetOffsetPivot(
            atlas_entry.pivot_x,
            atlas_entry.pivot_y
        );

        if (override_draw_size) {
            float width = atlas_entry.frame_width > 0f ? atlas_entry.frame_width : atlas_entry.width;
            float height = atlas_entry.frame_height > 0f ? atlas_entry.frame_height : atlas_entry.height;
            sprite.SetDrawSize(width, height);
        }
    }

    public static bool NameHasNumberSuffix(string atlas_entry_name, int start_index) {
        int string_length = atlas_entry_name.Length;
        bool ignore_space = true;

        if (start_index >= string_length) return false;

        for (int j = start_index ; j < string_length ; j++) {
            int code = (int)atlas_entry_name[j];

            if (code < 0x30 || code > 0x39) {
                if (ignore_space) {
                    switch (code) {
                        case 0x09:// tabulation
                        case 0x20:// white space
                                  //case 0xff:// hard space
                                  // case 0x5F:// underscore (used in plain-text atlas)
                            ignore_space = false;
                            continue;
                    }
                }

                // the name does not end with numbers
                return false;
            }
        }

        return true;
    }

    public bool GetTextureResolution(out int width, out int height) {
        width = this.resolution_width;
        height = this.resolution_height;
        return this.has_declared_resolution;
    }

    private static bool ParseResolution(Atlas atlas, string resolution_string) {
        if (String.IsNullOrEmpty(resolution_string)) return false;

        int index = -1;
        int length = resolution_string.Length;

        for (int i = 0 ; i < length ; i++) {
            int chr = resolution_string[i];
            if (chr == 'x' || chr == 'X') {
                index = i;
                break;
            }
        }

        if (index < 0) {
            Logger.Warn($"atlas_parse_resolution() invalid resolution: {resolution_string}");
            return false;
        }

        uint width = VertexProps.ParseUnsignedInteger(
            resolution_string.SubstringKDY(0, index), UInt32.MaxValue
        );
        uint height = VertexProps.ParseUnsignedInteger(
            resolution_string.SubstringKDY(index + 1, resolution_string.Length), UInt32.MaxValue
        );

        if (width == UInt32.MaxValue || height == UInt32.MaxValue) {
            Logger.Warn($"atlas_parse_resolution() invalid resolution: {resolution_string}");
            return false;
        }

        atlas.resolution_width = (int)width;
        atlas.resolution_height = (int)height;
        return true;
    }

    public static bool UtilsIsKnownExtension(string src) {
        return src.LowercaseEndsWithKDY(".xml") || src.LowercaseEndsWithKDY(".txt");
    }

    private static Atlas ParseFromPlainText(string src_txt) {
        // create fake path to texture, assume the format is PNG
        string path = FS.GetFullPath(src_txt);
        int index = path.LastIndexOf('.');
        string subpath = path.SubstringKDY(0, index);
        string fake_texture_filename = StringUtils.Concat(subpath, ".png");
        //free(path);
        //free(subpath);

        string text = FS.ReadText(src_txt);
        if (text == null) {
            //free(fake_texture_filename);
            return null;
        }

        Atlas atlas = new Atlas() {
            //name = src,
            glyph_fps = 0.0f,
            texture_filename = fake_texture_filename,
            resolution_width = Funkin.SCREEN_RESOLUTION_WIDTH,
            resolution_height = Funkin.SCREEN_RESOLUTION_HEIGHT,
            has_declared_resolution = false,
            size = 0,
            entries = null
        };

        string unparsed_entry;
        string temp;
        uint x, y, width, height;
        Tokenizer tokenizer_entries = Tokenizer.Init("\r\n", true, false, text);
        ArrayList<AtlasEntry> arraylist = new ArrayList<AtlasEntry>(tokenizer_entries.CountOccurrences());

        while ((unparsed_entry = tokenizer_entries.ReadNext()) != null) {
            Tokenizer tokenizer_field = Tokenizer.Init("\x3D", true, false, unparsed_entry);

            temp = tokenizer_field.ReadNext();
            string name = StringUtils.Trim(temp, false, true);
            //free(temp);

            temp = tokenizer_field.ReadNext();
            string unparsed_coords = StringUtils.Trim(temp, true, false);
            //free(temp);

            tokenizer_field.Destroy();
            //free(unparsed_entry);

            if (name == null) continue;

            if (unparsed_coords == null) {
                //free(name);
                continue;
            }

            Tokenizer tokenizer_coords = Tokenizer.Init("\u00A0\x20\x09", true, false, unparsed_coords);

            temp = tokenizer_coords.ReadNext();
            x = VertexProps.ParseUnsignedInteger(temp, Math2D.MAX_INT32);
            //free(temp);

            temp = tokenizer_coords.ReadNext();
            y = VertexProps.ParseUnsignedInteger(temp, Math2D.MAX_INT32);
            //free(temp);

            temp = tokenizer_coords.ReadNext();
            width = VertexProps.ParseUnsignedInteger(temp, Math2D.MAX_INT32);
            //free(temp);

            temp = tokenizer_coords.ReadNext();
            height = VertexProps.ParseUnsignedInteger(temp, Math2D.MAX_INT32);
            //free(temp);

            tokenizer_coords.Destroy();
            //free(unparsed_coords);


            if (x == Math2D.MAX_INT32) goto L_parse_field_failed;
            if (y == Math2D.MAX_INT32) goto L_parse_field_failed;
            if (width == Math2D.MAX_INT32) goto L_parse_field_failed;
            if (height == Math2D.MAX_INT32) goto L_parse_field_failed;

            arraylist.Add(new AtlasEntry() {
                name = name,
                x = (int)x,
                y = (int)y,
                width = (int)width,
                height = (int)height,
                frame_x = 0.0f,
                frame_y = 0.0f,
                frame_width = 0.0f,
                frame_height = 0.0f,
                pivot_x = 0.0f,
                pivot_y = 0.0f,
            });

            continue;

L_parse_field_failed:
//free(name);
            continue;
        }

        //free(text);
        tokenizer_entries.Destroy();

        arraylist.Destroy2(out atlas.size, ref atlas.entries);

        temp = FS.GetFilenameWithoutExtension(src_txt);
        if (temp != null) {
            atlas.texture_filename = StringUtils.Concat(temp, ".png");
            //free(temp);
        }

        return atlas;
    }

    private static void ParseTileSet(ArrayList<AtlasEntry> arraylist, XmlParserNode unparsed_tileset) {
        uint sub_x = VertexProps.ParseUnsignedInteger(unparsed_tileset.GetAttribute("x"), Math2D.MAX_INT32);
        uint sub_y = VertexProps.ParseUnsignedInteger(unparsed_tileset.GetAttribute("y"), Math2D.MAX_INT32);
        uint sub_width = VertexProps.ParseUnsignedInteger(unparsed_tileset.GetAttribute("subTextureWidth"), Math2D.MAX_INT32);
        uint sub_height = VertexProps.ParseUnsignedInteger(unparsed_tileset.GetAttribute("subTextureHeight"), Math2D.MAX_INT32);
        uint tile_width = VertexProps.ParseUnsignedInteger(unparsed_tileset.GetAttribute("tileWidth"), Math2D.MAX_INT32);
        uint tile_height = VertexProps.ParseUnsignedInteger(unparsed_tileset.GetAttribute("tileHeight"), Math2D.MAX_INT32);

        if (
            sub_width == Math2D.MAX_INT32 || sub_height == Math2D.MAX_INT32 ||
            tile_width == Math2D.MAX_INT32 || tile_height == Math2D.MAX_INT32
        ) {
            Logger.Warn($"atlas_parse_tileset() missing fields in TileSet: {unparsed_tileset.OuterXML}");
            return;
        }

        if (sub_x == Math2D.MAX_INT32) sub_x = 0;
        if (sub_y == Math2D.MAX_INT32) sub_y = 0;

        // truncate should not be necessary
        double rows = Math.Truncate((double)sub_width / tile_width);
        double columns = Math.Truncate((double)sub_height / tile_height);

        int index = 0;

        foreach (XmlParserNode unparsed_tile in unparsed_tileset.Children) {
            if (unparsed_tile.TagName != "Tile") {
                Logger.Warn($"atlas_parse_tileset() unknown TileSet entry: {unparsed_tile.OuterXML}");
                continue;
            } else if (!unparsed_tile.HasAttribute("name")) {
                Logger.Warn($"atlas_parse_tileset() missing tile name: {unparsed_tile.OuterXML}");
                index++;
                continue;
            }

            double tile_index = VertexProps.ParseInteger(unparsed_tile, "index", -1);
            if (tile_index < 0) tile_index = index;

            string tile_name = unparsed_tile.GetAttribute("name");
            uint tile_x = (uint)(tile_index % rows);
            uint tile_y = (uint)((tile_index / rows) % columns);

            tile_x *= tile_width;
            tile_y *= tile_height;

            tile_x += sub_x;
            tile_y += sub_y;

            arraylist.Add(new AtlasEntry() {
                name = tile_name,
                x = (int)tile_x,
                y = (int)tile_y,
                width = (int)tile_width,
                height = (int)tile_height,
                frame_x = 0,
                frame_y = 0,
                frame_width = 0,
                frame_height = 0,
                pivot_x = 0.0f,
                pivot_y = 00.0f,
            });

            index++;
        }
    }

}
