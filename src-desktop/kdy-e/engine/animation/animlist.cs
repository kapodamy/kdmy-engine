using System;
using System.Diagnostics;
using Engine.Game.Common;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Animation {

    public class AnimListItem {
        public string name;
        public bool is_tweenlerp;
        public AnimList.TweenLerpEntry[] tweenlerp_entries;
        public int tweenlerp_entries_count;
        public int loop;
        public float frame_rate;
        public bool alternate_per_loop;
        public bool alternate_no_random;
        public int instructions_count;
        public MacroExecutorInstruction[] instructions;
        public int frame_count;
        public AtlasEntry[] frames;
        internal AnimList.AlternateEntry[] alternate_set;
        internal int alternate_set_size;
        internal int frames_count;
        internal int frame_restart_index;
        internal bool frame_allow_size_change;
    }

    public class AnimList {


        /*
        const int MODIFIER_SINE = 0;
        const int MODIFIER_COSINE = 1;
        const int MODIFIER_LOG = 2;
        const int MODIFIER_EXP = 3;
        */


        private static Map<AnimList> POOL = new Map<AnimList>();
        private static int IDS = 0;


        private string src;
        private int id;
        private int references;
        public int entries_count;
        public AnimListItem[] entries;


        public static AnimList Init(string src) {
            string full_path = FS.GetFullPathAndOverride(src);

            // find a previous loaded animlist
            foreach (AnimList obj in AnimList.POOL) {
                if (obj.src == full_path) {
                    obj.references++;
                    //free(full_path);
                    return obj;
                }
            }

            XmlParser xml = XmlParser.Init(src);

            FS.FolderStackPush();
            FS.SetWorkingFolder(full_path, true);

            XmlParserNode anims_list = xml.GetRoot();
            Debug.Assert(anims_list != null && anims_list.TagName == "AnimationList", "missing AnimationList in: " + src);

            XmlParserNodeList anims = anims_list.Children;
            LinkedList<CachedAtlas> atlas_cache = new LinkedList<CachedAtlas>();
            LinkedList<AnimListItem> parsed_animations = new LinkedList<AnimListItem>();
            float default_fps = VertexProps.ParseFloat(anims_list, "frameRate", Funkin.DEFAULT_ANIMATIONS_FRAMERATE);
            Atlas default_atlas;
            bool is_macro;

            if (anims_list.HasAttribute("atlasPath")) {
                string default_atlas_path = FS.BuildPath2(full_path, anims_list.GetAttribute("atlasPath"));
                default_atlas = AnimList.LoadAtlas(default_atlas_path);
                //free(default_atlas_path);
            } else {
                default_atlas = null;
            }

            for (int i = 0 ; i < anims.Length ; i++) {

                switch (anims[i].TagName) {
                    case "Animation":
                        is_macro = false;
                        break;
                    case "AnimationMacro":
                        is_macro = true;
                        break;
                    case "TweenLerp":
                        parsed_animations.AddItem(AnimList.ReadTweenlerpAnimation(anims[i]));
                        continue;
                    default:
                        Console.Error.WriteLine("Unknown animation: " + anims[i].TagName);
                        continue;
                }

                Atlas atlas = AnimList.LoadRequiredAtlas(
                    anims[i], atlas_cache, default_atlas, is_macro, full_path
                );
                AnimListItem animlist_item;

                if (is_macro)
                    animlist_item = AnimList.ReadMacroAnimation(anims[i], atlas);
                else
                    animlist_item = AnimList.ReadFrameAnimation(anims[i], atlas, default_fps);

                if (animlist_item != null) parsed_animations.AddItem(animlist_item);
            }

            AnimList animlist = new AnimList();
            animlist.entries_count = parsed_animations.Count();
            animlist.entries = parsed_animations.ToArray();
            parsed_animations.Destroy();

            if (default_atlas != null) default_atlas.Destroy();

            // dispose atlas cache
            foreach (CachedAtlas entry in atlas_cache) {
                if (entry.atlas != null) entry.atlas.Destroy();
                //free(entry.path);
                //free(entry);
            }

            atlas_cache.Destroy();

            animlist.src = full_path;
            animlist.id = AnimList.IDS++;
            animlist.references = 1;
            AnimList.POOL.Set(animlist.id, animlist);

            FS.FolderStackPop();

            return animlist;
        }

        public void Destroy() {
            this.references--;
            if (this.references > 0) return;

            for (int i = 0 ; i < this.entries_count ; i++) {
                //if (this.entries[i].frames != null) free(this.entries[i].frames);
                //if (this.entries[i].alternate_set != null) free(this.entries[i].alternate_set);

                //for (int j = 0 ; j < this.entries[i].instructions_count ; j++) {
                //    if (this.entries[i].instructions[j].values != null) free(this.entries[i].instructions[j].values);
                //    free(this.entries[i].instructions[j]);
                //}


                //if (this.entries[i].is_tweenlerp) free(this.entries[i].tweenlerp_entries);

                //free(this.entries[i].instructions);
                //free(this.entries[i]);
            }

            //free(this.entries);
            //free(animlist);
        }

        public AnimListItem GetAnimation(string animation_name) {
            for (int i = 0 ; i < this.entries_count ; i++)
                if (this.entries[i].name == animation_name)
                    return this.entries[i];

            return null;
        }

        public static bool IsItemMacroAnimation(AnimListItem animlist_item) {
            return animlist_item.instructions_count > 0;
        }

        public static bool IsItemFrameAnimation(AnimListItem animlist_item) {
            return !animlist_item.is_tweenlerp && animlist_item.instructions_count < 1;
        }



        private static Atlas LoadRequiredAtlas(XmlParserNode animlist_item, LinkedList<CachedAtlas> atlas_list, Atlas def_atlas, bool is_macro, string ref_path) {
            string filename = animlist_item.GetAttribute("atlasPath");

            if (filename == null) {
                if (is_macro) return null;

                if (def_atlas == null) {
                    Console.Error.WriteLine(
                        "animlist_load_required_atlas() animation without atlas", animlist_item.OuterHTML
                    );
                }
                return def_atlas;
            }

            CachedAtlas obj = null;

            foreach (CachedAtlas entry in atlas_list) {
                if (entry.path == filename) {
                    obj = entry;
                    break;
                }
            }

            if (obj == null) {
                // JS only (build path)
                string atlas_path = FS.BuildPath2(ref_path, filename);
                obj = new CachedAtlas() { path = filename, atlas = LoadAtlas(atlas_path) };
                //free(atlas_path);
                atlas_list.AddItem(obj);
            }

            if (obj.atlas == null) {
                Console.Error.WriteLine(
                    "animlist_load_required_atlas() missing atlas " + filename, animlist_item.OuterHTML
                );
            }

            return obj.atlas;
        }

        private static Atlas LoadAtlas(string src) {
            return !String.IsNullOrEmpty(src) ? Atlas.Init(src) : null;
        }

        private static AnimListItem ReadFrameAnimation(XmlParserNode entry, Atlas atlas, float default_fps) {
            string name = entry.GetAttribute("name");
            if (name == null) {
                Console.Error.WriteLine("animlist_read_frame_animation() missing animation name", entry.OuterHTML);
                return null;
            }
            if (atlas == null) {
                Console.Error.WriteLine("animlist_read_frame_animation() missing atlas", entry.OuterHTML);
                return null;
            }

            AnimListItem anim = new AnimListItem() {
                is_tweenlerp = false,
                name = name,
                loop = VertexProps.ParseInteger(entry, "loop", 1),
                frame_rate = VertexProps.ParseFloat(entry, "frameRate", default_fps),
                alternate_per_loop = VertexProps.ParseBoolean(entry, "alternateInLoops", false),
                alternate_no_random = !VertexProps.ParseBoolean(entry, "alternateRandomize", false)
            };

            XmlParserNodeList frames = entry.Children;
            LinkedList<AtlasEntry> parsed_frames = new LinkedList<AtlasEntry>();
            LinkedList<AlternateEntry> parsed_alternates = new LinkedList<AlternateEntry>();
            int last_alternate_index = 0;

            for (int i = 0 ; i < frames.Length ; i++) {
                switch (frames[i].TagName) {
                    case "FrameArray":
                        string name_prefix = frames[i].GetAttribute("entryPrefixName");
                        string name_suffix = frames[i].GetAttribute("entrySuffixName");
                        bool has_number_suffix = VertexProps.ParseBoolean(frames[i], "hasNumberSuffix", true);
                        int index_start = VertexProps.ParseInteger(frames[i], "indexStart", 0);
                        int index_end = VertexProps.ParseInteger(frames[i], "indexEnd", -1);

                        string frame_name = !String.IsNullOrEmpty(name_prefix) ? name_prefix : name;
                        if (!String.IsNullOrEmpty(name_suffix)) frame_name += $" {name_suffix}";// add space before suffix         

                        AnimList.ReadEntriesToFramesArray(
                            parsed_frames, frame_name, has_number_suffix, atlas, index_start, index_end
                        );

                        //if (name_prefix) free(name_prefix);
                        //if (name_suffix) free(name_suffix);
                        //free(frame_name);
                        break;
                    case "Frame":
                        string name_frame = frames[i].GetAttribute("entryName");
                        if (!String.IsNullOrEmpty(name_frame)) name = name_frame;

                        AnimList.AddEntryFromAtlas(parsed_frames, name, atlas);
                        //if (name_frame != name) free(name_frame);
                        break;
                    case "Pause":
                        float duration = VertexProps.ParseFloat(frames[i], "duration", 1);
                        AtlasEntry last_frame = parsed_frames.GetLastItem();
                        if (last_frame != null) {
                            while (duration-- > 0) parsed_frames.AddItem(last_frame);
                        }
                        break;
                    case "AlternateSet":
                        int frame_count = parsed_frames.Count();
                        if (AnimList.AddAlternateEntry(parsed_alternates, frame_count, last_alternate_index))
                            Console.Error.WriteLine($"consecutive AlternateSet found (no enough frames in '{name}')");
                        else
                            last_alternate_index = frame_count;
                        break;
                    default:
                        Console.Error.WriteLine("Unknown frame type: " + frames[i].TagName, frames[i]);
                        break;
                }
            }

            anim.frame_count = parsed_frames.Count();
            anim.frames = parsed_frames.ToSolidArray();

            anim.instructions = null;
            anim.instructions_count = 0;

            if (parsed_alternates.Count() > 0) {
                // add the last frames set
                int frames_count = parsed_frames.Count();
                AnimList.AddAlternateEntry(parsed_alternates, frames_count, last_alternate_index);
            }
            anim.alternate_set = parsed_alternates.ToSolidArray();
            anim.alternate_set_size = parsed_alternates.Count();

            parsed_frames.Destroy();

            return anim;
        }

        private static void CopyEntriesToFramesArray(LinkedList<AtlasEntry> frame_list, string name, bool has_number_suffix, Atlas atlas) {
            int start = name.Length;

            for (int i = 0 ; i < atlas.size ; i++) {
                string atlas_entry_name = atlas.entries[i].name;
                if (!atlas_entry_name.StartsWith(name)) continue;

                if (has_number_suffix && !Atlas.NameHasNumberSuffix(atlas_entry_name, start))
                    continue;

                frame_list.AddItem(atlas.entries[i]);
            }
        }

        internal static void ReadEntriesToFramesArray(LinkedList<AtlasEntry> frame_list, string name, bool has_number_suffix, Atlas atlas, int start, int end) {
            int index_name_start = name.Length;

            if (end < 0) end = atlas.size;

            int found_index = 0;

            for (int i = 0 ; i < atlas.size ; i++) {
                string atlas_entry_name = atlas.entries[i].name;
                if (!atlas_entry_name.StartsWith(name)) continue;

                if (has_number_suffix && !Atlas.NameHasNumberSuffix(atlas_entry_name, index_name_start))
                    continue;

                if (found_index >= start) {
                    frame_list.AddItem(atlas.entries[i]);
                    if (found_index >= end) break;
                }

                found_index++;
            }
        }

        private static void AddEntryFromAtlas(LinkedList<AtlasEntry> frame_list, string name, Atlas atlas) {
            for (int i = 0 ; i < atlas.size ; i++) {
                AtlasEntry entry = atlas.entries[i];

                if (entry.name == name) {
                    frame_list.AddItem(entry);
                    return;
                }
            }

            Console.Error.WriteLine("animlist: Missing atlas entry: " + name, atlas);
        }


        private static AnimInterpolator ParseInterpolator(XmlParserNode node) {
            string type = node.GetAttribute("type");

            if (type == null)
                return AnimInterpolator.LINEAR;

            type = type.ToLowerInvariant();

            switch (type) {
                case "ease":
                    return AnimInterpolator.EASE;
                case "ease-in":
                    return AnimInterpolator.EASE_IN;
                case "ease-out":
                    return AnimInterpolator.EASE_OUT;
                case "ease-in-out":
                    return AnimInterpolator.EASE_IN_OUT;
                case "linear":
                    return AnimInterpolator.LINEAR;
                case "steps":
                    return AnimInterpolator.STEPS;
            }

            Console.Error.WriteLine("animlist: unknown interpolator type " + type);
            return AnimInterpolator.LINEAR;
        }

        private static int ParseRegister(XmlParserNode node) {
            string value = node.GetAttribute("register");
            if (value != null) {
                value = value.ToLowerInvariant();
                for (int i = 0 ; i < MacroExecutor.MACROEXECUTOR_REGISTER_COUNT ; i++) {
                    if (value == ("reg" + i)) return i;
                }
                Console.Error.WriteLine("animlist_parse_register() invalid register", value);
            }
            return -1;
        }

        private static void ParseComplexValue2(XmlParserNode node, string name, float def_value, ref MacroExecutorValue value) {
            ParseComplexValue(node.GetAttribute(name), def_value, ref value);
        }

        private static void ParseComplexValue(string unparsed_value, float def_value, ref MacroExecutorValue value) {
            if (String.IsNullOrEmpty(unparsed_value)) {
                // returnd default value (as literal)
                value.kind = MacroExecutorValueKind.LITERAL;
                value.literal = def_value;
                return;
            }

            // special values used in macroexecutor
            string as_special = unparsed_value.ToLowerInvariant();
            value.kind = MacroExecutorValueKind.SPECIAL;
            switch (as_special) {
                case "rnd":
                    value.reference = MacroExecutor.ANIM_MACRO_SPECIAL_RANDOM;
                    return;
                case "reg0":
                    value.reference = MacroExecutor.ANIM_MACRO_SPECIAL_REGISTER0;
                    return;
                case "reg1":
                    value.reference = MacroExecutor.ANIM_MACRO_SPECIAL_REGISTER1;
                    return;
                case "reg2":
                    value.reference = MacroExecutor.ANIM_MACRO_SPECIAL_REGISTER2;
                    return;
                case "reg3":
                    value.reference = MacroExecutor.ANIM_MACRO_SPECIAL_REGISTER3;
                    return;
            }

            // boolean and numbers are literals
            value.kind = MacroExecutorValueKind.LITERAL;

            // check if is a boolean value
            if (VertexProps.IsValueBoolean(unparsed_value)) {
                value.literal = VertexProps.ParseBoolean2(unparsed_value, false) ? 1f : 0f;
                return;
            }

            uint as_unsigned;
            if (VertexProps.ParseHex(unparsed_value, out as_unsigned, true)) {
                value.literal = as_unsigned;
                return;
            }

            // check if the value is just a number
            float as_number = VertexProps.ParseFloat2(unparsed_value, Single.NaN);
            if (!Single.IsNaN(as_number)) {
                value.literal = as_number;
                return;
            }

            // check if the value is a property name
            int as_property = VertexProps.ParseTextSpriteProperty2(unparsed_value);
            if (as_property < 0) as_property = VertexProps.ParseSpriteProperty2(unparsed_value);
            if (as_property < 0) as_property = VertexProps.ParseMediaProperty2(unparsed_value);
            if (as_property >= 0) {
                value.kind = MacroExecutorValueKind.PROPERTY;
                value.reference = as_property;
                return;
            }

            // check if the value is a property value enum
            int as_enum = (int)VertexProps.ParseFlag2(unparsed_value, PVRContextFlag.INVALID_VALUE);
            if (as_enum < 0) as_enum = VertexProps.ParseTextSpriteForceCase2(unparsed_value);
            if (as_enum < 0) as_enum = VertexProps.ParseWordbreak2(unparsed_value);
            if (as_enum < 0) as_enum = VertexProps.ParsePlayback2(unparsed_value);
            if (as_enum < 0) as_enum = (int)VertexProps.ParseAlign2(unparsed_value);
            if (as_enum >= 0) {
                value.kind = MacroExecutorValueKind.LITERAL;
                value.literal = as_enum;
                return;
            }

            // no matches, return the default value
            value.kind = MacroExecutorValueKind.LITERAL;
            value.literal = def_value;
        }


        private static bool AddAlternateEntry(LinkedList<AlternateEntry> list, int frame_count, int index) {
            int length = frame_count - index;
            if (length < 1) return true;
            list.AddItem(new AlternateEntry() { index = index, length = length });
            return false;
        }


        private static AnimListItem ReadMacroAnimation(XmlParserNode entry, Atlas atlas) {

            AnimListItem anim = new AnimListItem();
            anim.is_tweenlerp = false;
            anim.name = entry.GetAttribute("name");
            anim.loop = VertexProps.ParseInteger(entry, "loop", 1);
            anim.frames = null;
            anim.frames_count = 0;
            anim.frame_restart_index = VertexProps.ParseInteger(entry, "frameRestartIndex", -1);
            anim.frame_allow_size_change = VertexProps.ParseBoolean(entry, "frameAllowChangeSize", false);

            anim.alternate_set = null;
            anim.alternate_set_size = 0;

            string atlasPrefixEntryName = entry.GetAttribute("atlasPrefixEntryName");
            bool atlasHasNumberSuffix = VertexProps.ParseBoolean(entry, "atlasHasNumberSuffix", true);

            XmlParserNodeList unparsed_list = entry.Children;
            LinkedList<MacroExecutorInstruction> parsed_instructions = new LinkedList<MacroExecutorInstruction>();

            MacroExecutorInstruction instruction;
            int property_id;

            for (int i = 0 ; i < unparsed_list.Length ; i++) {
                switch (unparsed_list[i].TagName) {
                    case "Interpolator":
                        property_id = AnimList.ParseProperty(unparsed_list[i], "property", true);
                        if (property_id < 0) continue;

                        instruction = new MacroExecutorInstruction() {
                            type = AnimMacroType.INTERPOLATOR,
                            interpolator = AnimList.ParseInterpolator(unparsed_list[i]),
                            property = property_id,
                            start = new MacroExecutorValue(),
                            end = new MacroExecutorValue(),
                            duration = new MacroExecutorValue(),
                            steps_count = new MacroExecutorValue(),
                            steps_method = new MacroExecutorValue()
                        };

                        AnimList.ParseComplexValue2(unparsed_list[i], "start", Single.NaN, ref instruction.start);
                        AnimList.ParseComplexValue2(unparsed_list[i], "end", Single.NaN, ref instruction.end);
                        AnimList.ParseComplexValue2(unparsed_list[i], "duration", 0f, ref instruction.duration);
                        AnimList.ParseComplexValue2(unparsed_list[i], "steps_count", 1, ref instruction.steps_count);
                        AnimList.ParseComplexValue2(unparsed_list[i], "steps_method", 0, ref instruction.steps_method);

                        parsed_instructions.AddItem(instruction);
                        break;
                    case "Set":
                        property_id = AnimList.ParseProperty(unparsed_list[i], "property", true);
                        if (property_id < 0) continue;

                        instruction = new MacroExecutorInstruction() {
                            type = AnimMacroType.SET,
                            property = property_id,
                            value = new MacroExecutorValue()
                        };

                        AnimList.ParseComplexValue2(unparsed_list[i], "value", 0f, ref instruction.value);

                        parsed_instructions.AddItem(instruction);
                        break;
                    case "Yield":
                        instruction = new MacroExecutorInstruction() {
                            type = AnimMacroType.YIELD,
                            value = new MacroExecutorValue()
                        };

                        AnimList.ParseComplexValue2(unparsed_list[i], "duration", 0f, ref instruction.value);

                        parsed_instructions.AddItem(instruction);
                        break;
                    case "Pause":
                        instruction = new MacroExecutorInstruction() {
                            type = AnimMacroType.PAUSE,
                            value = new MacroExecutorValue()
                        };

                        AnimList.ParseComplexValue2(unparsed_list[i], "duration", 1f, ref instruction.value);

                        parsed_instructions.AddItem(instruction);
                        break;
                    case "Reset":
                        instruction = new MacroExecutorInstruction() {
                            type = AnimMacroType.RESET
                        };
                        parsed_instructions.AddItem(instruction);
                        break;
                    case "RandomSetup":
                        instruction = new MacroExecutorInstruction() {
                            type = AnimMacroType.RANDOM_SETUP,
                            start = new MacroExecutorValue(),
                            end = new MacroExecutorValue()
                        };

                        AnimList.ParseComplexValue2(unparsed_list[i], "start", 0f, ref instruction.start);
                        AnimList.ParseComplexValue2(unparsed_list[i], "end", 1f, ref instruction.end);

                        parsed_instructions.AddItem(instruction);
                        break;
                    case "RandomChoose":
                        instruction = new MacroExecutorInstruction() {
                            type = AnimMacroType.RANDOM_CHOOSE
                        };
                        parsed_instructions.AddItem(instruction);
                        break;
                    case "RandomExact":
                        int values_size;
                        MacroExecutorValue[] values_array = AnimList.ParseRandomExact(
                            unparsed_list[i], out values_size
                        );
                        if (values_array == null) continue;

                        instruction = new MacroExecutorInstruction() {
                            type = AnimMacroType.RANDOM_EXACT,
                            values = values_array,
                            values_size = values_size
                        };
                        parsed_instructions.AddItem(instruction);
                        break;
                    case "RegisterProp":
                        property_id = AnimList.ParseProperty(unparsed_list[i], "property", true);
                        if (property_id < 0) continue;

                        instruction = new MacroExecutorInstruction() {
                            type = AnimMacroType.REGISTER_PROP,
                            register_index = AnimList.ParseRegister(unparsed_list[i]),
                            property = property_id
                        };
                        parsed_instructions.AddItem(instruction);
                        break;
                    case "RegisterSet":
                        instruction = new MacroExecutorInstruction() {
                            type = AnimMacroType.REGISTER_SET,
                            register_index = AnimList.ParseRegister(unparsed_list[i]),
                            value = new MacroExecutorValue()
                        };

                        AnimList.ParseComplexValue2(unparsed_list[i], "value", 0.0f, ref instruction.value);

                        parsed_instructions.AddItem(instruction);
                        break;
                    default:
                        Console.Error.WriteLine(
                            "animlist: unknown instruction: " + unparsed_list[i].TagName,
                            unparsed_list[i].OuterHTML
                        );
                        break;
                }
            }

            anim.instructions_count = parsed_instructions.Count();
            anim.instructions = parsed_instructions.ToSolidArray();
            parsed_instructions.Destroy2(/*free*/);// note: keep "instruction[].values" allocated

            if (!String.IsNullOrEmpty(atlasPrefixEntryName)) {
                LinkedList<AtlasEntry> parsed_frames = new LinkedList<AtlasEntry>();

                AnimList.CopyEntriesToFramesArray(
                    parsed_frames, atlasPrefixEntryName, atlasHasNumberSuffix, atlas
                );
                anim.frame_count = parsed_frames.Count();
                anim.frames = parsed_frames.ToSolidArray();

                parsed_frames.Destroy();
            } else {
                anim.frame_count = 0;
                anim.frames = null;
            }

            return anim;
        }

        private static MacroExecutorValue[] ParseRandomExact(XmlParserNode unparsed_randomexact, out int size) {
            string unparsed_values = unparsed_randomexact.GetAttribute("values");

            Tokenizer tokenizer = Tokenizer.Init("\x20", true, false, unparsed_values);
            if (tokenizer == null) {
                Console.Error.WriteLine("missing attribute values in RandomExact", unparsed_randomexact);
                size = -1;
                return null;
            }

            ArrayList<MacroExecutorValue> parsed_values = new ArrayList<MacroExecutorValue>(tokenizer.CountOccurrences());
            string str;

            while ((str = tokenizer.ReadNext()) != null) {
                MacroExecutorValue parsed_value = new MacroExecutorValue();
                AnimList.ParseComplexValue(str, Single.NaN, ref parsed_value);

                if (parsed_value.reference == VertexProps.TEXTSPRITE_PROP_STRING && parsed_value.kind == MacroExecutorValueKind.PROPERTY) {
                    Console.Error.WriteLine(
                        "animlist_read_macro_animation() illegal property used: string",
                        unparsed_randomexact.OuterHTML
                    );

                    //free(string);
                    continue;
                }

                if (Single.IsNaN(parsed_value.literal) && parsed_value.kind == MacroExecutorValueKind.LITERAL) {
                    Console.Error.WriteLine(
                        "animlist_read_macro_animation() invalid or unreconized value found",
                        unparsed_randomexact.OuterHTML
                    );

                    //free(string);
                    continue;
                }

                parsed_values.Add(parsed_value);
                //free(string);

            }
            tokenizer.Destroy();

            MacroExecutorValue[] values_array = null;
            size = parsed_values.Size();

            parsed_values.Destroy2(out _, ref values_array);
            return values_array;
        }

        private static int ParseProperty(XmlParserNode node, string name, bool warn) {
            int value = VertexProps.ParseTextSpriteProperty(node, name, false);
            if (value < 0) value = VertexProps.ParseSpriteProperty(node, name, warn);
            if (value < 0) value = VertexProps.ParseMediaProperty(node, name, warn);

            if (value == VertexProps.TEXTSPRITE_PROP_STRING) {
                Console.Error.WriteLine("animlist_parse_property() illegal property: string", node.OuterHTML);
                return -1;
            }

            return value;
        }

        private static AnimListItem ReadTweenlerpAnimation(XmlParserNode entry) {
            LinkedList<TweenLerpEntry> linkedlist = new LinkedList<TweenLerpEntry>();

            foreach (XmlParserNode node in entry.Children) {
                // <Interpolator id="" type="" start="" end="" duration="" stepsCount="" stepsMethod="" />

                int id = AnimList.ParseProperty(node, "id", false);

                AnimInterpolator interp = AnimList.ParseInterpolator(node);

                float start = VertexProps.ParseFloat2(node.GetAttribute("start"), 0f);

                float end = VertexProps.ParseFloat2(node.GetAttribute("end"), 0f);

                float duration = VertexProps.ParseFloat2(node.GetAttribute("duration"), -1f);

                uint steps_count = VertexProps.ParseUnsignedInteger(node.GetAttribute("stepsCount"), 1);

                Align steps_dir = VertexProps.ParseAlign(node, "stepsMethod", false, false);

                linkedlist.AddItem(new TweenLerpEntry() {
                    id = id,
                    start = start,
                    end = end,
                    duration = duration,
                    interp = interp,
                    steps_dir = steps_dir,
                    steps_count = steps_count
                });
            }

            int tweenlerp_entries_count = linkedlist.Count();
            TweenLerpEntry[] tweenlerp_entries = linkedlist.ToSolidArray();

            linkedlist.Destroy2(/*free*/);

            return new AnimListItem() {
                name = entry.GetAttribute("name"),
                is_tweenlerp = true,
                tweenlerp_entries = tweenlerp_entries,
                tweenlerp_entries_count = tweenlerp_entries_count
            };
        }


        public class TweenLerpEntry {
            public int id;
            public float start;
            public float end;
            public float duration;
            public AnimInterpolator interp;
            public Align steps_dir;
            public uint steps_count;
        }



        public struct AlternateEntry {
            public int index;
            public int length;
        }

        private class CachedAtlas {
            public string path;
            public Atlas atlas;
        }

    }

}
