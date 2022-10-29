using System;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game.Gameplay.Helpers {

    public class MenuManifest {

        public class Parameters {
            public uint font_color;
            public string atlas;
            public string animlist;
            public string font;
            public float font_size;
            public string font_glyph_suffix;
            public bool font_color_by_difference;// false by default
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
            public string modelholder;
            public float texture_scale;
            public string name;
            public string anim_selected;
            public string anim_choosen;
            public string anim_discarded;
            public string anim_idle;
            public string anim_rollback;
            public string anim_in;
            public string anim_out;
            public float gap;
            public bool hidden;
        }

        public Parameters parameters;
        public Item[] items;
        public int items_size;

        public MenuManifest() { }

        public MenuManifest(string src) {

            JSONParser json = JSONParser.LoadFrom(src);
            if (json == null) throw new Exception("menumanifest_init() misssing or invalid file: " + src);

            FS.FolderStackPush();
            FS.SetWorkingFolder(src, true);

            string align_value = JSONParser.ReadString(json, "itemsAlign", null);
            Align align = VertexProps.ParseAlign2(align_value);
            //free(align_value);
            switch (align) {
                case Align.INVALID:
                    align = Align.CENTER;
                    break;
                case Align.START:
                case Align.CENTER:
                case Align.END:
                    break;
                default:
                    Console.Error.WriteLine("[WARN] menumanifest_init() unknown 'itemsAlign' value in: " + src);
                    align = Align.CENTER;
                    break;
            }

            JSONToken array_items = JSONParser.ReadArray(json, "items");
            int array_items_length = JSONParser.ReadArrayLength(array_items);

            if (array_items_length < 1) {
                throw new Exception("menumanifest_init() misssing or invalid 'items' property in: " + src);
            }

            MenuManifest menumanifest = new MenuManifest() {
                parameters = new Parameters() {
                    suffix_selected = JSONParser.ReadString(json, "suffixSelected", "selected"),
                    suffix_choosen = JSONParser.ReadString(json, "suffixChoosen", "choosen"),
                    suffix_discarded = JSONParser.ReadString(json, "suffixDiscarded", "discarded"),
                    suffix_idle = JSONParser.ReadString(json, "suffixIdle", "idle"),
                    suffix_rollback = JSONParser.ReadString(json, "suffixRollback", "rollback"),
                    suffix_in = JSONParser.ReadString(json, "suffixIn", "in"),
                    suffix_out = JSONParser.ReadString(json, "suffixOut", "out"),

                    atlas = MenuManifest.InternalParsePath(json, "atlas"),
                    animlist = MenuManifest.InternalParsePath(json, "animlist"),

                    anim_selected = JSONParser.ReadString(json, "animSelected", null),
                    anim_choosen = JSONParser.ReadString(json, "animChoosen", null),
                    anim_discarded = JSONParser.ReadString(json, "animDiscarded", null),
                    anim_idle = JSONParser.ReadString(json, "animIdle", null),
                    anim_rollback = JSONParser.ReadString(json, "animRollback", null),
                    anim_in = JSONParser.ReadString(json, "animIn", null),
                    anim_out = JSONParser.ReadString(json, "animOut", null),

                    anim_transition_in_delay = (float)JSONParser.ReadNumberDouble(json, "transitionInDelay", 0f),
                    anim_transition_out_delay = (float)JSONParser.ReadNumberDouble(json, "transitionOutDelay", 0f),

                    font = JSONParser.ReadString(json, "fontPath", null),
                    font_glyph_suffix = JSONParser.ReadString(json, "fontSuffix", null),
                    font_color_by_difference = JSONParser.ReadBoolean(json, "fontColorByDifference", false),
                    font_size = (float)JSONParser.ReadNumberDouble(json, "fontSize", 0.0),
                    font_color = JSONParser.ReadHex(json, "fontColor", 0xFFFFFF),
                    font_border_color = JSONParser.ReadHex(json, "fontBorderColor", 0x0),
                    font_border_size = (float)JSONParser.ReadNumberDouble(json, "fontBorderSize", 0.0),

                    is_sparse = JSONParser.ReadBoolean(json, "isSparse", false),
                    is_vertical = JSONParser.ReadBoolean(json, "isVertical", true),
                    is_per_page = JSONParser.ReadBoolean(json, "isPerPage", false),

                    items_align = align,
                    items_gap = (float)JSONParser.ReadNumberDouble(json, "itemsGap", 0.0),
                    items_dimmen = (float)JSONParser.ReadNumberDouble(json, "itemsDimmen", 0.0),
                    static_index = (int)JSONParser.ReadNumberLong(json, "staticIndex", 0L),
                    texture_scale = (float)JSONParser.ReadNumberDouble(json, "textureScale", Double.NaN),
                    enable_horizontal_text_correction = JSONParser.ReadBoolean(json, "enableHorizontalTextCorrection", false),
                },

                items = new Item[array_items_length],
                items_size = array_items_length
            };

            for (int i = 0 ; i < array_items_length ; i++) {
                JSONToken json_item = JSONParser.ReadArrayItemObject(array_items, i);
                JSONToken json_placement = JSONParser.ReadObject(json_item, "placement");

                this.items[i] = new Item() {
                    name = JSONParser.ReadString(json_item, "name", null),
                    text = JSONParser.ReadString(json_item, "text", null),
                    hidden = JSONParser.ReadBoolean(json_item, "hidden", false),

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
            for (int i = 0 ; i < this.items_size ; i++) {
                //free(this.items[i].name);
                //free(this.items[i].text);
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


        private static string InternalParsePath(JSONToken json, string property_name) {
            string str = JSONParser.ReadString(json, property_name, null);
            if (str == null) return null;
            if (str.Length < 1) {
                //free(str);
                return null;
            }

            string path = FS.GetFullPath(str);
            //free(str);

            return path;
        }

    }

}
