using System;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game.Gameplay.Helpers {

    public class CharacterManifest {

        public static readonly string[] DEFAULT_OPPOSITE_DIR_FROM = { "left", "right" };
        public static readonly string[] DEFAULT_OPPOSITE_DIR_TO = { "right", "left" };

        public class Opposite {
            public int sizes;
            public string[] from;
            public string[] to;
        }
        public class Sing {
            public string direction;
            public string model_src;
            public string anim;
            public string anim_hold;
            public string anim_rollback;
            public bool rollback;
            public bool follow_hold;
            public bool full_sustain;
            public float offset_x;
            public float offset_y;
        }
        public class Miss {
            public string direction;
            public string model_src;
            public int stop_after_beats;
            public string anim;
            public float offset_x;
            public float offset_y;
        }
        public class Extra {
            public string name;
            public string model_src;
            public int stop_after_beats;
            public string anim;
            public string anim_hold;
            public string anim_rollback;
            public bool static_until_beat;
            public float offset_x;
            public float offset_y;
        }
        public class Actions {
            public int sing_size;
            public int miss_size;
            public int extras_size;
            public Sing[] sing;
            public Sing[] sing_alt;
            public Miss[] miss;
            public Extra[] extras;
            public Extra idle;
            public Extra hey;
            internal bool has_idle;
            internal bool has_hey;
        }

        public string model_character;
        public Actions actions;
        public bool left_facing;
        public Align align_horizontal;
        public Align align_vertical;
        public bool actions_apply_chart_speed;
        public bool continuous_idle;
        public bool has_reference_size;
        public float reference_width;
        public float reference_height;
        public string sing_suffix;
        public string sing_alternate_suffix;
        public Opposite opposite_directions;
        public string model_health_icons;
        public string model_week_selector;
        public bool has_healthbar_color;
        public uint healthbar_color;
        public float offset_x;
        public float offset_y;
        public bool week_selector_enable_beat;

        public CharacterManifest(string src, bool gameplay_required_models_only) {
            double offset_x, offset_y;

            JSONParser json = JSONParser.LoadFrom(src);
            if (json == null) throw new Exception("missing or invalid file: " + src);

            FS.FolderStackPush();
            FS.SetWorkingFolder(src, true);

            string model_character = null;
            string model_health_icons = null;
            string model_week_selector = null;
            bool has_healthbar_color = JSONParser.HasPropertyHex(json, "healthbarColor");
            uint healthbar_color = JSONParser.ReadHex(json, "healthbarColor", 0x00FFFF);

            if (gameplay_required_models_only) {
                model_character = CharacterManifest.InternalPathOf(json, "model", src);
                if (JSONParser.HasPropertyString(json, "healthIconsModel") && !JSONParser.IsPropertyNull(json, "healthIconsModel")) {
                    model_health_icons = CharacterManifest.InternalPathOf(json, "healthIconsModel", src);
                }
            } else {
                model_week_selector = CharacterManifest.InternalPathOf(json, "modelWeekSelector", src);
            }


            this.model_character = model_character;
            this.model_health_icons = model_health_icons;
            this.model_week_selector = model_week_selector;
            this.has_healthbar_color = has_healthbar_color;
            this.healthbar_color = healthbar_color;

            this.sing_suffix = JSONParser.ReadString(json, "singSuffix", null);
            this.sing_alternate_suffix = JSONParser.ReadString(json, "singAlternateSuffix", null);
            this.continuous_idle = JSONParser.ReadBoolean(json, "continuousIdle", false);
            this.actions_apply_chart_speed = JSONParser.ReadBoolean(json, "actionsApplyChartSpeed", false);

            this.offset_x = (float)JSONParser.ReadNumberDouble(json, "offsetX", 0);
            this.offset_y = (float)JSONParser.ReadNumberDouble(json, "offsetY", 0);
            this.left_facing = JSONParser.ReadBoolean(json, "leftFacing", false);
            this.week_selector_enable_beat = JSONParser.ReadBoolean(json, "weekSelectorEnableBeat", true);

            this.actions = new Actions() {
                sing = null,
                sing_size = 0,
                miss = null,
                miss_size = 0,
                extras = null,
                extras_size = 0,
                idle = new Extra() { },
                hey = new Extra() { },
                has_idle = false,
                has_hey = false
            };
            this.align_vertical = Align.CENTER;
            this.align_horizontal = Align.CENTER;

            this.has_reference_size = false;
            this.reference_width = -1;
            this.reference_height = -1;

            this.opposite_directions = new Opposite() {
                sizes = 2,
                from = CharacterManifest.DEFAULT_OPPOSITE_DIR_FROM,
                to = CharacterManifest.DEFAULT_OPPOSITE_DIR_TO
            };


            JSONToken json_actions = JSONParser.ReadObject(json, "actions");

            JSONToken sing_array = JSONParser.ReadArray(json_actions, "sing");
            int sing_array_size = JSONParser.ReadArrayLength(sing_array);
            JSONToken miss_array = JSONParser.ReadArray(json_actions, "miss");
            int miss_array_size = JSONParser.ReadArrayLength(miss_array);
            JSONToken extras_array = JSONParser.ReadArray(json_actions, "extras");
            int extras_array_size = JSONParser.ReadArrayLength(extras_array);

            if (sing_array_size > 0) {
                this.actions.sing = new Sing[sing_array_size];
                this.actions.sing_size = sing_array_size;
            }
            if (miss_array_size > 0) {
                this.actions.miss = new Miss[miss_array_size];
                this.actions.miss_size = miss_array_size;
            }
            if (extras_array_size > 0) {
                this.actions.extras = new Extra[extras_array_size];
                this.actions.extras_size = extras_array_size;
            }

            for (int i = 0 ; i < this.actions.sing_size ; i++) {
                JSONToken item_json = JSONParser.ReadArrayItemObject(sing_array, i);
                InternalReadOffsets(item_json, out offset_x, out offset_y);

                this.actions.sing[i] = new Sing() {
                    direction = JSONParser.ReadString(item_json, "direction", null),
                    anim = JSONParser.ReadString(item_json, "anim", null),
                    anim_hold = JSONParser.ReadString(item_json, "animHold", null),
                    anim_rollback = JSONParser.ReadString(item_json, "animRollback", null),
                    rollback = JSONParser.ReadBoolean(item_json, "rollback", false),
                    follow_hold = JSONParser.ReadBoolean(item_json, "followHold", false),
                    full_sustain = JSONParser.ReadBoolean(item_json, "fullSustain", false),
                    model_src = InternalPathOf(item_json, "model", null),
                    offset_x = (float)offset_x,
                    offset_y = (float)offset_y
                };

                if (String.IsNullOrEmpty(this.actions.sing[i].direction)) {
                    throw new Exception($"missing actions.sing[{i}].direction in '{src}'");
                }
            }
            for (int i = 0 ; i < this.actions.miss_size ; i++) {
                JSONToken item_json = JSONParser.ReadArrayItemObject(miss_array, i);
                InternalReadOffsets(item_json, out offset_x, out offset_y);

                this.actions.miss[i] = new Miss() {
                    direction = JSONParser.ReadString(item_json, "direction", null),
                    anim = JSONParser.ReadString(item_json, "anim", null),
                    stop_after_beats = (int)JSONParser.ReadNumberLong(item_json, "stopAfterBeats", 1L),
                    model_src = InternalPathOf(item_json, "model", null),
                    offset_x = (float)offset_x,
                    offset_y = (float)offset_y
                };

                if (String.IsNullOrEmpty(this.actions.miss[i].direction)) {
                    throw new Exception("missing actions.miss[" + i + "].direction in '" + src + "'");
                }
            }
            for (int i = 0 ; i < this.actions.extras_size ; i++) {
                JSONToken item_json = JSONParser.ReadArrayItemObject(extras_array, i);

                this.actions.extras[i] = new Extra() { };
                InternalParseExtra(item_json, false, this.actions.extras[i]);
            }

            this.actions.has_idle = JSONParser.HasPropertyObject(json_actions, "idle");
            if (this.actions.has_idle) {
                JSONToken json_extra = JSONParser.ReadObject(json_actions, "idle");
                InternalParseExtra(json_extra, true, this.actions.idle);
            }

            this.actions.has_hey = JSONParser.HasPropertyObject(json_actions, "hey");
            if (this.actions.has_hey) {
                JSONToken json_extra = JSONParser.ReadObject(json_actions, "hey");
                InternalParseExtra(json_extra, true, this.actions.hey);
            }

            this.align_vertical = InternalAlign(json, "alignVertical", true);
            this.align_horizontal = InternalAlign(json, "alignHorizontal", false);

            this.has_reference_size = JSONParser.HasProperty(json, "referenceSize");
            if (this.has_reference_size) {
                if (!JSONParser.HasPropertyObject(json, "referenceSize")) {
                    throw new Exception("invalid 'referenceSize' in " + src);
                }
                this.reference_width = (float)JSONParser.ReadNumberDouble(json, "width", -1.0);
                this.reference_height = (float)JSONParser.ReadNumberDouble(json, "height", -1.0);
            }

            JSONToken opposite_directions = JSONParser.ReadObject(json, "oppositeDirections");
            if (opposite_directions != null) {

                JSONToken from_array = JSONParser.ReadArray(opposite_directions, "from");
                int from_array_size = JSONParser.ReadArrayLength(from_array);
                JSONToken to_array = JSONParser.ReadArray(opposite_directions, "to");
                int to_array_size = JSONParser.ReadArrayLength(to_array);

                if (from_array_size != to_array_size) {
                    throw new Exception(
                        "invalid 'oppositeDirections.from', and/or 'oppositeDirectionsto' in " + src
                    );
                }

                if (to_array_size == 0) {
                    this.opposite_directions.sizes = 0;
                    this.opposite_directions.from = null;
                    this.opposite_directions.to = null;
                } else if (to_array_size > 0) {
                    this.opposite_directions.sizes = from_array_size;
                    this.opposite_directions.from = new string[from_array_size];
                    this.opposite_directions.to = new string[to_array_size];

                    CharacterManifest.InternalParseOppositeDir(
                        from_array, from_array_size, this.opposite_directions.from
                    );
                    CharacterManifest.InternalParseOppositeDir(
                        to_array, to_array_size, this.opposite_directions.to
                    );
                }
            } else {

            }

            JSONParser.Destroy(json);
            FS.FolderStackPop();

        }

        public void Destroy() {
            //free(this.model_character);
            //free(this.model_week_selector);
            //free(this.model_health_icons);
            //free(this.sing_suffix);
            //free(this.sing_alternate_suffix);

            //for (int i = 0 ; i < this.actions.sing_size ; i++) {
            //    free(this.actions.sing[i].direction);
            //    free(this.actions.sing[i].anim);
            //    free(this.actions.sing[i].anim_hold);
            //    free(this.actions.sing[i].anim_rollback);
            //    free(this.actions.sing[i].model_src);
            //}

            //for (int i = 0 ; i < this.actions.miss_size ; i++) {
            //    free(this.actions.miss[i].direction);
            //    free(this.actions.miss[i].anim);
            //    free(this.actions.miss[i].model_src);
            //}

            //for (int i = 0 ; i < this.actions.extra_size ; i++) {
            //    free(this.actions.extras[i].name);
            //    free(this.actions.extras[i].anim);
            //    free(this.actions.extras[i].anim_hold);
            //    free(this.actions.extras[i].anim_rollback);
            //    free(this.actions.extras[i].model_src);
            //}

            //if (this.actions.has_hey) {
            //    free(this.actions.hey.name);
            //    free(this.actions.hey.anim);
            //    free(this.actions.hey.anim_hold);
            //    free(this.actions.hey.anim_rollback);
            //    free(this.actions.hey.model_src);
            //}

            //if (this.actions.has_idle) {
            //    free(this.actions.idle.name);
            //    free(this.actions.idle.anim);
            //    free(this.actions.idle.anim_hold);
            //    free(this.actions.idle.anim_rollback);
            //    free(this.actions.idle.model_src);
            //}

            //if (this.opposite_directions.from != CHARACTERMANIFEST_DEFAULT_OPPOSITE_DIR_FROM) {
            //    for (int i = 0 ; i < this.opposite_directions.sizes ; i++) {
            //        free(this.opposite_directions.from[i]);
            //    }
            //    free(this.opposite_directions.from);
            //}

            //if (this.opposite_directions.to != CharacterManifest.DEFAULT_OPPOSITE_DIR_TO) {
            //    for (int i = 0 ; i < this.opposite_directions.sizes ; i++) {
            //        free(this.opposite_directions.to[i]);
            //    }
            //    free(this.opposite_directions.to);
            //}


            //free(this);
        }


        private static void InternalParseExtra(JSONToken json_extra, bool ignore_name, Extra entry) {
            double offset_x, offset_y;
            if (ignore_name) {
                entry.name = null;
            } else {
                entry.name = JSONParser.ReadString(json_extra, "name", null);
                if (entry.name == null) throw new Exception("missing extra animation name");
            }
            entry.anim = JSONParser.ReadString(json_extra, "anim", null);
            entry.anim_hold = JSONParser.ReadString(json_extra, "animHold", null);
            entry.stop_after_beats = (int)JSONParser.ReadNumberLong(json_extra, "stopAfterBeats", 0L);
            entry.static_until_beat = JSONParser.ReadBoolean(json_extra, "staticUntilBeat", true);
            entry.anim_rollback = JSONParser.ReadString(json_extra, "animRollback", null);
            entry.model_src = CharacterManifest.InternalPathOf(json_extra, "model", null);

            CharacterManifest.InternalReadOffsets(json_extra, out offset_x, out offset_y);
            entry.offset_x = (float)offset_x;
            entry.offset_y = (float)offset_y;
        }

        private static void InternalParseOppositeDir(JSONToken json_array, int size, string[] array) {
            for (int i = 0 ; i < size ; i++)
                array[i] = JSONParser.ReadArrayItemString(json_array, i, null);
        }

        private static string InternalPathOf(JSONToken json, string property_name, string optional_src) {
            string str = JSONParser.ReadString(json, property_name, null);
            if (String.IsNullOrEmpty(str)) {
                if (optional_src == null) return null;
                throw new Exception("missing '" + property_name + "' in the '" + optional_src + "' file");
            }
            string path = FS.GetFullPath(str);
            //free(str);
            return path;
        }

        private static Align InternalAlign(JSONToken json, string property_name, bool is_vertical) {
            string str = JSONParser.ReadString(json, property_name, null);
            if (str == null) return is_vertical ? Align.END : Align.START;
            Align align = VertexProps.ParseAlign2(str);

            switch (align) {
                case Align.START:
                case Align.CENTER:
                case Align.END:
                    break;
                default:
                    Console.Error.WriteLine("[WARN] charactermanifest_internal_align() invalid align: " + str);
                    align = Align.CENTER;
                    break;
            }

            //free(str);
            return align;
        }

        private static void InternalReadOffsets(JSONToken json, out double offset_x, out double offset_y) {
            offset_x = JSONParser.ReadNumberDouble(json, "offsetX", 0);
            offset_y = JSONParser.ReadNumberDouble(json, "offsetY", 0);
        }


    }
}
