using System;
using Engine.Externals.LuaScriptInterop;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game.Gameplay.Helpers;

public class MenuManifest {

    public class Parameters {
        public uint font_color;
        public string atlas;
        public string animlist;
        public string font;
        public float font_size;
        public string font_glyph_suffix;
        public bool font_color_by_addition;// false by default
        public uint font_border_color;
        public string anim_discarded;
        public string anim_idle;
        public string anim_rollback;
        public string anim_selected;
        public string anim_choosen;
        public string anim_in;
        public string anim_out;
        public float anim_transition_in_delay;
        public float anim_transition_out_delay;
        public bool is_vertical;
        public Align items_align;
        public float items_gap;
        public bool is_sparse;
        public int static_index;
        public bool is_per_page;
        public float items_dimmen;
        public float font_border_size;
        public float texture_scale;
        public bool enable_horizontal_text_correction;
        public string suffix_selected;
        public string suffix_choosen;
        public string suffix_discarded;
        public string suffix_idle;
        public string suffix_rollback;
        public string suffix_in;
        public string suffix_out;
    }

    public struct Placement {
        public float gap;
        public float x;
        public float y;
        public float dimmen;
    }

    public class Item {
        public Placement placement;
        public bool has_font_color;
        public uint font_color;
        public string text;
        public string model;
        public float texture_scale;
        public string name;
        public string anim_selected;
        public string anim_choosen;
        public string anim_discarded;
        public string anim_idle;
        public string anim_rollback;
        public string anim_in;
        public string anim_out;
        public bool hidden;
        public string description;
    }

    public Parameters parameters;
    public Item[] items;
    public int items_size;

    public MenuManifest() { }

    public MenuManifest(string src) {

        JSONToken json = JSONParser.LoadFrom(src);
        if (json == null) throw new Exception("menumanifest_init() misssing or invalid file: " + src);

        JSONToken json_parameters = JSONParser.ReadObject(json, "parameters");
        if (json_parameters == null) throw new Exception("menumanifest_init() misssing parameters in json: " + src);

        FS.FolderStackPush();
        FS.SetWorkingFolder(src, true);

        string align_value = JSONParser.ReadString(json, "itemsAlign", null);
        Align align = VertexProps.ParseAlign2(align_value);
        switch (align) {
            case Align.INVALID:
                align = Align.CENTER;
                break;
            case Align.START:
            case Align.CENTER:
            case Align.END:
                break;
            default:
                Logger.Warn($"menumanifest_init() unknown 'itemsAlign' value in: {src}");
                align = Align.CENTER;
                break;
        }

        JSONToken array_items = JSONParser.ReadArray(json, "items");
        int array_items_length = JSONParser.ReadArrayLength(array_items);

        if (array_items_length < 1) {
            throw new Exception("menumanifest_init() misssing or invalid 'items' property in: " + src);
        }

        this.parameters = new Parameters() {
            suffix_selected = JSONParser.ReadString(json_parameters, "suffixSelected", "selected"),
            suffix_choosen = JSONParser.ReadString(json_parameters, "suffixChoosen", "choosen"),
            suffix_discarded = JSONParser.ReadString(json_parameters, "suffixDiscarded", "discarded"),
            suffix_idle = JSONParser.ReadString(json_parameters, "suffixIdle", "idle"),
            suffix_rollback = JSONParser.ReadString(json_parameters, "suffixRollback", "rollback"),
            suffix_in = JSONParser.ReadString(json_parameters, "suffixIn", "in"),
            suffix_out = JSONParser.ReadString(json_parameters, "suffixOut", "out"),

            atlas = MenuManifest.InternalParsePath(json_parameters, "atlas"),
            animlist = MenuManifest.InternalParsePath(json_parameters, "animlist"),

            anim_selected = JSONParser.ReadString(json_parameters, "animSelected", null),
            anim_choosen = JSONParser.ReadString(json_parameters, "animChoosen", null),
            anim_discarded = JSONParser.ReadString(json_parameters, "animDiscarded", null),
            anim_idle = JSONParser.ReadString(json_parameters, "animIdle", null),
            anim_rollback = JSONParser.ReadString(json_parameters, "animRollback", null),
            anim_in = JSONParser.ReadString(json_parameters, "animIn", null),
            anim_out = JSONParser.ReadString(json_parameters, "animOut", null),

            anim_transition_in_delay = (float)JSONParser.ReadNumberDouble(json_parameters, "transitionInDelay", 0f),
            anim_transition_out_delay = (float)JSONParser.ReadNumberDouble(json_parameters, "transitionOutDelay", 0f),

            font = JSONParser.ReadString(json_parameters, "fontPath", null),
            font_glyph_suffix = JSONParser.ReadString(json_parameters, "fontSuffix", null),
            font_color_by_addition = JSONParser.ReadBoolean(json_parameters, "fontColorByAddition", false),
            font_size = (float)JSONParser.ReadNumberDouble(json_parameters, "fontSize", 0.0),
            font_color = JSONParser.ReadHex(json_parameters, "fontColor", 0xFFFFFF),
            font_border_color = JSONParser.ReadHex(json_parameters, "fontBorderColor", 0x0),
            font_border_size = (float)JSONParser.ReadNumberDouble(json_parameters, "fontBorderSize", 0.0),

            is_sparse = JSONParser.ReadBoolean(json_parameters, "isSparse", false),
            is_vertical = JSONParser.ReadBoolean(json_parameters, "isVertical", true),
            is_per_page = JSONParser.ReadBoolean(json_parameters, "isPerPage", false),

            items_align = align,
            items_gap = (float)JSONParser.ReadNumberDouble(json_parameters, "itemsGap", 0.0),
            items_dimmen = (float)JSONParser.ReadNumberDouble(json_parameters, "itemsDimmen", 0.0),
            static_index = (int)JSONParser.ReadNumberLong(json_parameters, "staticIndex", 0L),
            texture_scale = (float)JSONParser.ReadNumberDouble(json_parameters, "textureScale", Double.NaN),
            enable_horizontal_text_correction = JSONParser.ReadBoolean(json_parameters, "enableHorizontalTextCorrection", false),
        };

        this.items = new Item[array_items_length];
        this.items_size = array_items_length;

        for (int i = 0 ; i < array_items_length ; i++) {
            JSONToken json_item = JSONParser.ReadArrayItemObject(array_items, i);
            JSONToken json_placement = JSONParser.ReadObject(json_item, "placement");

            this.items[i] = new Item() {
                name = JSONParser.ReadString(json_item, "name", null),
                text = JSONParser.ReadString(json_item, "text", null),
                model = JSONParser.ReadString(json_item, "model", null),
                hidden = JSONParser.ReadBoolean(json_item, "hidden", false),
                description = JSONParser.ReadString(json_item, "description", null),
                texture_scale = (float)JSONParser.ReadNumberDouble(json_item, "textureScale", 0.0),

                placement = new Placement() {
                    x = (float)JSONParser.ReadNumberDouble(json_placement, "x", 0.0),
                    y = (float)JSONParser.ReadNumberDouble(json_placement, "y", 0.0),
                    dimmen = (float)JSONParser.ReadNumberDouble(json_placement, "dimmen", 0.0),
                    gap = (float)JSONParser.ReadNumberDouble(json_placement, "gap", 0.0)
                },

                anim_selected = JSONParser.ReadString(json_item, "animSelected", null),
                anim_choosen = JSONParser.ReadString(json_item, "animChoosen", null),
                anim_discarded = JSONParser.ReadString(json_item, "animDiscarded", null),
                anim_idle = JSONParser.ReadString(json_item, "animIdle", null),
                anim_rollback = JSONParser.ReadString(json_item, "animRollback", null),
                anim_in = JSONParser.ReadString(json_item, "animIn", null),
                anim_out = JSONParser.ReadString(json_item, "animOut", null),
            };
        }

        FS.FolderStackPop();
    }

    public void Destroy() {
        Luascript.DropShared(this);

        for (int i = 0 ; i < this.items_size ; i++) {
            //free(this.items[i].name);
            //free(this.items[i].text);
            //free(this.items[i].description);
            //free(this.items[i].anim_selected);
            //free(this.items[i].anim_choosen);
            //free(this.items[i].anim_discarded);
            //free(this.items[i].anim_idle);
            //free(this.items[i].anim_rollback);
            //free(this.items[i].anim_in);
            //free(this.items[i].anim_out);
        }

        //free(this.parameters.suffix_selected);
        //free(this.parameters.suffix_choosen);
        //free(this.parameters.suffix_discarded);
        //free(this.parameters.suffix_idle);
        //free(this.parameters.suffix_rollback);
        //free(this.parameters.suffix_in);
        //free(this.parameters.suffix_out);

        //free(this.parameters.atlas);
        //free(this.parameters.animlist);

        //free(this.parameters.anim_selected);
        //free(this.parameters.anim_choosen);
        //free(this.parameters.anim_discarded);
        //free(this.parameters.anim_idle);
        //free(this.parameters.anim_rollback);
        //free(this.parameters.anim_in);
        //free(this.parameters.anim_out);

        //free(this.parameters.font);
        //free(this.parameters.font_glyph_suffix);

        //free(this.items);
        //free(this);
    }

    public int GetOptionIndex(string option_name) {
        for (int i = 0 ; i < this.items_size ; i++) {
            if (this.items[i].name == option_name) {
                return i;
            }
        }
        return -1;
    }

    private static string InternalParsePath(JSONToken json, string property_name) {
        string str = JSONParser.ReadString(json, property_name, null);
        if (String.IsNullOrEmpty(str)) return null;

        string path = FS.GetFullPath(str);

        return path;
    }

}
