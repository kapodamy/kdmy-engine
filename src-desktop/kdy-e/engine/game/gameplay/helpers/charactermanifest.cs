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
            public float stop_after_beats;
            public float offset_x;
            public float offset_y;
        }
        public class Miss {
            public string direction;
            public string model_src;
            public float stop_after_beats;
            public string anim;
            public float offset_x;
            public float offset_y;
        }
        public class Extra {
            public string name;
            public string model_src;
            public float stop_after_beats;
            public string anim;
            public string anim_hold;
            public string anim_rollback;
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

        public class AdditionalState {
            public string name;
            public string model;
            public Actions actions;
        }

        public string model_character;
        public Actions actions;
        public bool left_facing;
        public Align align_horizontal;
        public Align align_vertical;
        public bool actions_apply_chart_speed;
        public bool continuous_idle;
        public bool has_reference_size;
        public bool allow_alternate_idle;
        public float reference_width;
        public float reference_height;
        public string sing_suffix;
        public string sing_alternate_suffix;
        public string sing_prefix;
        public string sing_alternate_prefix;
        public Opposite opposite_directions;
        public string model_health_icons;
        public bool has_healthbar_color;
        public uint healthbar_color;
        public float offset_x;
        public float offset_y;
        public string week_selector_model;
        public string week_selector_idle_anim_name;
        public string week_selector_choosen_anim_name;
        public bool week_selector_left_facing;
        public bool week_selector_enable_beat;
        public AdditionalState[] additional_states;
        public int additional_states_size;

        public CharacterManifest(string src, bool gameplay_required_models_only) {
            JSONToken json = JSONParser.LoadFrom(src);
            if (json == null) throw new Exception("missing or invalid file: " + src);

            FS.FolderStackPush();
            FS.SetWorkingFolder(src, true);

            string model_character = null;
            string model_health_icons = null;
            string week_selector_model = null;
            bool has_healthbar_color = JSONParser.HasPropertyHex(json, "healthbarColor");
            uint healthbar_color = JSONParser.ReadHex(json, "healthbarColor", 0x00FFFF);

            if (gameplay_required_models_only) {
                model_character = CharacterManifest.InternalPathOf(json, "model", src);
                if (JSONParser.HasPropertyString(json, "healthIconsModel") && !JSONParser.IsPropertyNull(json, "healthIconsModel")) {
                    model_health_icons = CharacterManifest.InternalPathOf(json, "healthIconsModel", src);
                }
            } else {
                week_selector_model = CharacterManifest.InternalPathOf(json, "weekSelectorModel", null);
            }


            this.model_character = model_character;
            this.model_health_icons = model_health_icons;
            this.has_healthbar_color = has_healthbar_color;
            this.healthbar_color = healthbar_color;

            this.sing_suffix = JSONParser.ReadString(json, "singSuffix", null);
            this.sing_alternate_suffix = JSONParser.ReadString(json, "singAlternateSuffix", null);
            this.sing_prefix = JSONParser.ReadString(json, "singPrefix", null);
            this.sing_alternate_prefix = JSONParser.ReadString(json, "singAlternatePrefix", null);
            this.allow_alternate_idle = JSONParser.ReadBoolean(json, "allowAlternateIdle", false);
            this.continuous_idle = JSONParser.ReadBoolean(json, "continuousIdle", false);
            this.actions_apply_chart_speed = JSONParser.ReadBoolean(json, "actionsApplyChartSpeed", false);

            this.offset_x = (float)JSONParser.ReadNumberDouble(json, "offsetX", 0);
            this.offset_y = (float)JSONParser.ReadNumberDouble(json, "offsetY", 0);
            this.left_facing = JSONParser.ReadBoolean(json, "leftFacing", false);

            this.week_selector_model = week_selector_model;
            this.week_selector_idle_anim_name = JSONParser.ReadString(json, "weekSelectorIdleAnimName", null);
            this.week_selector_choosen_anim_name = JSONParser.ReadString(json, "weekSelectorChoosenAnimName", null);
            this.week_selector_enable_beat = JSONParser.ReadBoolean(json, "weekSelectorEnableBeat", true);
            this.week_selector_left_facing = JSONParser.ReadBoolean(json, "weekSelectorLeftFacing", this.left_facing);

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
            CharacterManifest.InternalParseActions(json_actions, this.actions, src);

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

            JSONToken json_additional_states = JSONParser.ReadArray(json, "additionalStates");
            ArrayList<AdditionalState> additional_states = CharacterManifest.InternalReadAdditionalStates(
                json_additional_states, this.model_character, src
            );
            additional_states.Destroy2(out this.additional_states_size, ref this.additional_states);

            JSONParser.Destroy(json);
            FS.FolderStackPop();

        }

        public void Destroy() {
            //free(this.model_character);
            //free(this.model_health_icons);
            //free(this.sing_suffix);
            //free(this.sing_alternate_suffix);
            //free(this.sing_alternate_prefix);
            //free(this.allow_alternate_idle);
            //free(this.week_selector_model);
            //free(this.week_selector_idle_anim_name);
            //free(this.week_selector_choosen_anim_name);

            CharacterManifest.InternalDestroyActions(this.actions);

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

            for (int i = 0 ; i < this.additional_states_size ; i++) {
                //free(this.additional_states[i].name);
                CharacterManifest.InternalDestroyActions(this.additional_states[i].actions);
            }
            //free(this.additional_states);

            //free(this);
        }


        private static void InternalParseExtra(JSONToken json_extra, bool ignore_name, double default_beat_stop, Extra entry) {
            double offset_x, offset_y;
            if (ignore_name) {
                entry.name = null;
            } else {
                entry.name = JSONParser.ReadString(json_extra, "name", null);
                if (entry.name == null) throw new Exception("missing extra animation name");
            }
            entry.anim = JSONParser.ReadString(json_extra, "anim", null);
            entry.anim_hold = JSONParser.ReadString(json_extra, "animHold", null);
            entry.stop_after_beats = (float)JSONParser.ReadNumberDouble(json_extra, "stopAfterBeats", default_beat_stop);
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

        private static void InternalParseActions(JSONToken json_actions, Actions actions, string src) {
            double offset_x, offset_y;

            JSONToken sing_array = JSONParser.ReadArray(json_actions, "sing");
            int sing_array_size = JSONParser.ReadArrayLength(sing_array);
            JSONToken miss_array = JSONParser.ReadArray(json_actions, "miss");
            int miss_array_size = JSONParser.ReadArrayLength(miss_array);
            JSONToken extras_array = JSONParser.ReadArray(json_actions, "extras");
            int extras_array_size = JSONParser.ReadArrayLength(extras_array);

            if (sing_array_size > 0) {
                actions.sing = new Sing[sing_array_size];
                actions.sing_size = sing_array_size;
            }
            if (miss_array_size > 0) {
                actions.miss = new Miss[miss_array_size];
                actions.miss_size = miss_array_size;
            }
            if (extras_array_size > 0) {
                actions.extras = new Extra[extras_array_size];
                actions.extras_size = extras_array_size;
            }

            for (int i = 0 ; i < actions.sing_size ; i++) {
                JSONToken item_json = JSONParser.ReadArrayItemObject(sing_array, i);
                InternalReadOffsets(item_json, out offset_x, out offset_y);

                actions.sing[i] = new Sing() {
                    direction = JSONParser.ReadString(item_json, "direction", null),
                    anim = JSONParser.ReadString(item_json, "anim", null),
                    anim_hold = JSONParser.ReadString(item_json, "animHold", null),
                    anim_rollback = JSONParser.ReadString(item_json, "animRollback", null),
                    rollback = JSONParser.ReadBoolean(item_json, "rollback", false),
                    follow_hold = JSONParser.ReadBoolean(item_json, "followHold", false),
                    full_sustain = JSONParser.ReadBoolean(item_json, "fullSustain", false),
                    stop_after_beats = (float)JSONParser.ReadNumberDouble(item_json, "stopAfterBeats", 1.0),
                    model_src = InternalPathOf(item_json, "model", null),
                    offset_x = (float)offset_x,
                    offset_y = (float)offset_y
                };

                if (String.IsNullOrEmpty(actions.sing[i].direction)) {
                    throw new Exception($"missing actions.sing[{i}].direction in '{src}'");
                }
            }
            for (int i = 0 ; i < actions.miss_size ; i++) {
                JSONToken item_json = JSONParser.ReadArrayItemObject(miss_array, i);
                InternalReadOffsets(item_json, out offset_x, out offset_y);

                actions.miss[i] = new Miss() {
                    direction = JSONParser.ReadString(item_json, "direction", null),
                    anim = JSONParser.ReadString(item_json, "anim", null),
                    stop_after_beats = (float)JSONParser.ReadNumberDouble(item_json, "stopAfterBeats", 1.0),
                    model_src = InternalPathOf(item_json, "model", null),
                    offset_x = (float)offset_x,
                    offset_y = (float)offset_y
                };

                if (String.IsNullOrEmpty(actions.miss[i].direction)) {
                    throw new Exception("missing actions.miss[" + i + "].direction in '" + src + "'");
                }
            }
            for (int i = 0 ; i < actions.extras_size ; i++) {
                JSONToken item_json = JSONParser.ReadArrayItemObject(extras_array, i);

                actions.extras[i] = new Extra() { };
                InternalParseExtra(item_json, false, -1.0, actions.extras[i]);
            }

            actions.has_idle = JSONParser.HasPropertyObject(json_actions, "idle");
            if (actions.has_idle) {
                JSONToken json_extra = JSONParser.ReadObject(json_actions, "idle");
                InternalParseExtra(json_extra, true, -1.0, actions.idle);
            }

            actions.has_hey = JSONParser.HasPropertyObject(json_actions, "hey");
            if (actions.has_hey) {
                JSONToken json_extra = JSONParser.ReadObject(json_actions, "hey");
                InternalParseExtra(json_extra, true, -1.0, actions.hey);
            }
        }

        private static void InternalDestroyActions(Actions actions) {
            //for (int i = 0 ; i < actions.sing_size ; i++) {
            //    free(actions.sing[i].direction);
            //    free(actions.sing[i].anim);
            //    free(actions.sing[i].anim_hold);
            //    free(actions.sing[i].anim_rollback);
            //    free(actions.sing[i].model_src);
            //}

            //for (int i = 0 ; i < actions.miss_size ; i++) {
            //    free(actions.miss[i].direction);
            //    free(actions.miss[i].anim);
            //    free(actions.miss[i].model_src);
            //}

            //for (int i = 0 ; i < actions.extra_size ; i++) {
            //    free(actions.extras[i].name);
            //    free(actions.extras[i].anim);
            //    free(actions.extras[i].anim_hold);
            //    free(actions.extras[i].anim_rollback);
            //    free(actions.extras[i].model_src);
            //}

            //if (actions.has_hey) {
            //    free(actions.hey.name);
            //    free(actions.hey.anim);
            //    free(actions.hey.anim_hold);
            //    free(actions.hey.anim_rollback);
            //    free(actions.hey.model_src);
            //}

            //if (actions.has_idle) {
            //    free(actions.idle.name);
            //    free(actions.idle.anim);
            //    free(actions.idle.anim_hold);
            //    free(actions.idle.anim_rollback);
            //    free(actions.idle.model_src);
            //}
        }

        private static ArrayList<AdditionalState> InternalReadAdditionalStates(JSONToken json_array, string default_model, string src) {
            ArrayList<AdditionalState> additional_states = new ArrayList<AdditionalState>();

            int size = JSONParser.ReadArrayLength(json_array);
            for (int i = 0 ; i < size ; i++) {
                JSONToken item = JSONParser.ReadArrayItemObject(json_array, i);

                AdditionalState state = new AdditionalState() {
                    name = JSONParser.ReadString(item, "name", null),
                    model = CharacterManifest.InternalPathOf(item, "model", null),
                    actions = new Actions() {
                        extras = null,
                        extras_size = 0,
                        has_hey = false,
                        has_idle = false,
                        hey = new Extra() { },
                        idle = new Extra() { },
                        miss = null,
                        miss_size = 0,
                        sing = null,
                        sing_alt = null,
                        sing_size = 0
                    }
                };

                if (String.IsNullOrEmpty(state.model)) state.model = default_model;

                JSONToken json_actions = JSONParser.ReadObject(item, "actions");
                if (!JSONParser.IsPropertyNull(item, "actions")) {
                    CharacterManifest.InternalParseActions(json_actions, state.actions, src);
                }

                additional_states.Add(state);
            }

            return additional_states;
        }

    }
}
