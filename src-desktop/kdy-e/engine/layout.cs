using System;
using System.Collections.Generic;
using System.Text;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Font;
using Engine.Game.Common;
using Engine.Image;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;

namespace Engine {

    public class LayoutParallax {
        public float x;
        public float y;
        public float z;
    }

    public class LayoutPlaceholder {
        public PVRContextVertex type;
        public int group_id;
        public string name;
        public Align align_vertical;
        public Align align_horizontal;
        public float x;
        public float y;
        public float z;
        public float height;
        public float width;
        public LayoutParallax parallax;
        public bool static_camera;
        public IVertex vertex;
    }

    internal class CameraPlaceholder {

        public string name;
        public bool has_duration;
        public bool duration_in_beats;
        public float duration;
        public bool move_only;
        public bool has_from;
        public bool is_empty;
        public bool has_parallax_offset_only;
        public bool move_offset_only;
        public bool has_offset_from;
        public bool has_offset_to;
        public float to_offset_x;
        public float to_offset_y;
        public float to_offset_z;
        public float from_offset_x;
        public float from_offset_y;
        public float from_offset_z;
        public AnimSprite animation;
        public float from_x, from_y, from_z;
        public float to_x, to_y, to_z;
        public float offset_x, offset_y, offset_z;
    }

    [Flags]
    public enum AttachedValueType : int {
        NOTFOUND = 0x00,
        STRING = 0x01,
        FLOAT = 0x02,
        INTEGER = 0x04,
        HEX = 0x08,// unsigned integer
        BOOLEAN = 0x10
    }

    public class Layout : IDraw, IAnimate {
        //
        // Public constants
        //

        private const int ACTION_LOCATION = 0;
        private const int ACTION_SIZE = 1;
        private const int ACTION_ANIMATION = 2;
        private const int ACTION_ANIMATIONREMOVE = 3;
        private const int ACTION_OFFSETCOLOR = 4;
        private const int ACTION_COLOR = 5;
        private const int ACTION_PROPERTY = 6;
        private const int ACTION_ATLASAPPLY = 7;
        private const int ACTION_VISIBILITY = 8;
        private const int ACTION_MODIFIER = 9;
        private const int ACTION_PARALLAX = 10;
        private const int ACTION_TEXTBORDER = 11;
        private const int ACTION_RESIZE = 12;
        private const int ACTION_RESETMATRIX = 13;
        private const int ACTION_STATIC = 14;
        private const int ACTION_ANIMATIONEND = 15;
        private const int ACTION_EXECUTE = 16;
        private const int ACTION_PUPPETITEM = 17;
        private const int ACTION_PUPPETGROUP = 18;
        private const int ACTION_SETSHADER = 19;
        private const int ACTION_REMOVESHADER = 20;
        private const int ACTION_SETSHADERUNIFORM = 22;
        private const int LAYOUT_ACTION_SETBLENDING = 23;
        private const int LAYOUT_ACTION_VIEWPORT = 24;

        public const string GROUP_ROOT = "___root-group___";
        private const float BPM_STEPS = 32;// 1/32 beats

        private static readonly HelperZbufferSortImpl HelperZbufferSort = new HelperZbufferSortImpl();


        private Layout() { }


        private float animation_speed;
        private Camera camera_helper;
        private Camera camera_secondary_helper;
        private Font[] fonts;
        private int fonts_size;
        private ExternalValue[] values;
        private int values_size;
        private Macro[] macro_list;
        private int macro_list_size;
        private bool keep_aspect_ratio;
        private float viewport_width;
        private float viewport_height;
        private Modifier modifier_camera;
        private Modifier modifier_camera_secondary;
        private Modifier modifier_viewport;
        private Item[] vertex_list;
        private int vertex_list_size;
        private Group[] group_list;
        private int group_list_size;
        private CameraPlaceholder[] camera_list;
        private int camera_list_size;
        private Trigger[] trigger_list;
        private int trigger_list_size;
        private Item[] sound_list;
        private int sound_list_size;
        private Texture[] textures;
        private int textures_size;
        private ZBufferEntry[] z_buffer;
        private int z_buffer_size;
        private Item[] external_vertex_list;
        private int external_vertex_list_size;
        private Item single_item;
        private bool suspended;
        private bool beatwatcher_synced_triggers;
        private double beatwatcher_last_timestamp;
        private int beatwatcher_resyncs_count;
        private BeatWatcher beatwatcher;
        private bool antialiasing_disabled;
        private int resolution_changes;
        private PSShader psshader;



        public static Layout Init(string src) {
            XmlParser xml = XmlParser.Init(src);

            if (xml == null) {
                Console.Error.WriteLine("[ERROR] layout_init() error reading: " + src);
                return null;
            }

            // change the current working folder but remember the old one
            FS.FolderStackPush();
            FS.SetWorkingFolder(src, true);

            LayoutContext layout_context = new LayoutContext() {
                animlist = null,
                vertex_list = new ArrayList<Item>(),
                group_list = new ArrayList<Group>(),
                camera_list = new ArrayList<CameraPlaceholder>(),
                trigger_list = new ArrayList<Trigger>(),
                sound_list = new ArrayList<Item>(),
                resource_pool = new ResourcePool() { textures = new ArrayList<ResourcePoolEntry>(), atlas = new ArrayList<ResourcePoolEntry>() },
                fonts = null,
                fonts_size = -1,
                values = null,
                values_size = -1,
                macro_list = null,
                macro_list_size = 0
            };

            XmlParserNode root = xml.GetRoot();

            // step 1: read attributes of the root
            float viewport_width = Layout.HelperParseFloat(root, "width", -1f);
            float viewport_height = Layout.HelperParseFloat(root, "height", -1f);
            float animation_speed = Layout.HelperParseFloat(root, "animationSpeed", 1.0f);
            bool keep_aspect_ratio = VertexProps.ParseBoolean(root, "keepAspectRatio", true);

            if (viewport_width < 1) throw new Exception("Invalid/missing layout width");
            if (viewport_height < 1) throw new Exception("Invalid/missing layout height");

            string animlist_filename = root.GetAttribute("animationList");
            if (!String.IsNullOrEmpty(animlist_filename)) {
                layout_context.animlist = AnimList.Init(animlist_filename);
            }

            // step 2.1: read fonts
            ParseFonts(root, layout_context);

            // step 2.2: import all external values
            ParseExternalvalues(root, layout_context);

            // step 2.3: import macros
            ParseMacro(root, layout_context);

            // step 3: import all groups (including sprites, texts and placeholders)
            ParseGroup(root, layout_context, null);


            // step 4: build the layout object
            Layout layout = new Layout() {
                animation_speed = animation_speed,

                camera_helper = null,
                camera_secondary_helper = null,

                fonts = layout_context.fonts,
                fonts_size = layout_context.fonts_size,

                values = layout_context.values,
                values_size = layout_context.values_size,

                macro_list = layout_context.macro_list,
                macro_list_size = layout_context.macro_list_size,

                keep_aspect_ratio = keep_aspect_ratio,

                viewport_width = viewport_width,
                viewport_height = viewport_height,

                modifier_camera = new Modifier(),
                modifier_camera_secondary = new Modifier(),
                modifier_viewport = new Modifier(),

                vertex_list = null,
                vertex_list_size = -1,

                group_list = null,
                group_list_size = -1,

                camera_list = null,
                camera_list_size = -1,

                trigger_list = null,
                trigger_list_size = -1,

                sound_list = null,
                sound_list_size = -1,

                textures = null,
                textures_size = 0,

                z_buffer = null,
                z_buffer_size = 0,

                external_vertex_list = null,
                external_vertex_list_size = 0,

                single_item = null,
                suspended = false,

                beatwatcher_synced_triggers = false,
                beatwatcher_last_timestamp = 0,
                beatwatcher_resyncs_count = 0,
                beatwatcher = new BeatWatcher(),

                antialiasing_disabled = false,
                resolution_changes = 0,
                psshader = null
            };

            // step 5: build modifiers
            layout.modifier_camera.Clear();
            layout.modifier_camera_secondary.Clear();
            layout.modifier_viewport.Clear();

            layout.modifier_camera.width = viewport_width;
            layout.modifier_camera.height = viewport_height;
            layout.camera_helper = new Camera(layout.modifier_camera, -1f, -1f);
            layout.camera_secondary_helper = new Camera(layout.modifier_camera_secondary, -1f, -1f);

            layout.modifier_viewport.x = 0f;
            layout.modifier_viewport.y = 0f;
            layout.modifier_viewport.width = layout.viewport_width;
            layout.modifier_viewport.height = layout.viewport_height;

            // step 6: build vertex and group arrays
            layout_context.vertex_list.Destroy2(out layout.vertex_list_size, ref layout.vertex_list);
            layout_context.group_list.Destroy2(out layout.group_list_size, ref layout.group_list);
            layout_context.camera_list.Destroy2(out layout.camera_list_size, ref layout.camera_list);
            layout_context.trigger_list.Destroy2(out layout.trigger_list_size, ref layout.trigger_list);
            layout_context.sound_list.Destroy2(out layout.sound_list_size, ref layout.sound_list);

            // step 7: build textures array
            layout.textures_size = layout_context.resource_pool.textures.Size();
            layout.textures = new Texture[layout.textures_size];

            ResourcePoolEntry[] list = layout_context.resource_pool.textures.PeekArray();
            for (int i = 0 ; i < layout.textures_size ; i++) {
                layout.textures[i] = (Texture)list[i].data;
                //free(list[i].src);
            }

            // step 8: build z-buffer
            layout.z_buffer = new ZBufferEntry[layout.vertex_list_size];
            layout.z_buffer_size = layout.vertex_list_size;
            layout.HelperZbufferBuild();

            // step 9: cleanup
            foreach (ResourcePoolEntry definition in layout_context.resource_pool.atlas) {
                if (definition.data != null) ((Atlas)definition.data).Destroy();
                //free(definition.src);
            }

            layout_context.resource_pool.textures.Destroy(false);
            layout_context.resource_pool.atlas.Destroy(false);

            if (layout_context.animlist != null) layout_context.animlist.Destroy();

            // configure layout viewport with the PVR's render size (screen resolution)
            layout.resolution_changes = PVRContext.global_context.resolution_changes;
            layout.UpdateRenderSize(PVRContext.global_context.ScreenWidth, PVRContext.global_context.ScreenHeight);

            // step 10: execute all default actions (actions with no names) and initial actions
            for (int i = 0 ; i < layout.vertex_list_size ; i++) {
                string initial_action_name = layout.vertex_list[i].initial_action_name;
                bool intial_action_found = initial_action_name == null;

                for (int j = 0 ; j < layout.vertex_list[i].actions_size ; j++) {
                    string action_name = layout.vertex_list[i].actions[j].name;

                    if (action_name == null || action_name == initial_action_name) {
                        if (action_name == initial_action_name) intial_action_found = true;

                        layout.HelperExecuteAction(
                             layout.vertex_list[i], layout.vertex_list[i].actions[j]
                         );
                    }
                }

                if (!intial_action_found)
                    Console.Error.WriteLine("[WARN] layout_init() initial action not found" + initial_action_name);
            }

            for (int i = 0 ; i < layout.group_list_size ; i++) {
                string initial_action_name = layout.group_list[i].initial_action_name;
                bool intial_action_found = initial_action_name == null;

                for (int j = 0 ; j < layout.group_list[i].actions_size ; j++) {
                    string action_name = layout.group_list[i].actions[j].name;

                    if (action_name == null || action_name == initial_action_name) {
                        if (action_name == initial_action_name) intial_action_found = true;

                        Layout.HelperExecuteActionInGroup(
                              layout.group_list[i].actions[j], layout.group_list[i]
                          );
                    }

                }

                if (!intial_action_found)
                    Console.Error.WriteLine("[WARN] layout_init() initial action not found" + initial_action_name);
            }

            for (int i = 0 ; i < layout.sound_list_size ; i++) {
                string initial_action_name = layout.sound_list[i].initial_action_name;
                bool intial_action_found = initial_action_name == null;

                for (int j = 0 ; j < layout.sound_list[i].actions_size ; j++) {
                    String action_name = layout.sound_list[i].actions[j].name;

                    if (action_name == null || action_name == initial_action_name) {
                        if (action_name == initial_action_name) intial_action_found = true;

                        Layout.HelperExecuteActionInSound(
                              layout.sound_list[i].actions[j], layout.sound_list[i]
                          );
                    }

                }

                if (!intial_action_found)
                    Console.Error.WriteLine("[WARN] layout_init() initial action not found" + initial_action_name);
            }

            for (int i = 0 ; i < layout.trigger_list_size ; i++) {
                if (layout.trigger_list[i].name != null) continue;
                layout.HelperExecuteTrigger(layout.trigger_list[i]);
            }

            // only is possible to trigger the first null-named camera
            layout.camera_helper.FromLayout(layout, null);
            layout.camera_helper.SetParentLayout(layout);
            layout.camera_secondary_helper.SetParentLayout(layout);

            // set default beats per second
            float bpm = Layout.HelperParseFloat(root, "defaultBPM", 100f);
            layout.beatwatcher.Reset(true, bpm);
            layout.SetBpm(bpm);

            // restore previous working folder
            FS.FolderStackPop();

            return layout;
        }

        public void Destroy() {

            for (int i = 0 ; i < this.vertex_list_size ; i++) {
                HelperDestroyActions(this.vertex_list[i].actions, this.vertex_list[i].actions_size);
                //free(this.vertex_list[i].name);
                //free(this.vertex_list[i].initial_action_name);

                switch (this.vertex_list[i].type) {
                    case PVRContextVertex.SPRITE:
                        this.vertex_list[i].vertex.Destroy();
                        break;
                    case PVRContextVertex.DRAWABLE:
                        this.vertex_list[i].placeholder.vertex = null;// external drawable ¡DO NOT DISPOSE!
                        //free(this.vertex_list[i].placeholder);
                        break;
                    case PVRContextVertex.TEXTSPRITE:
                        this.vertex_list[i].vertex.Destroy();
                        break;
                }
            }
            //free(this.vertex_list);

            //for (int i = 0; i < this.values_size; i++)
            //{
            //    if (this.values[i].type == Layout.ExternalValueType.STRING) //free(this.values[i].misc);
            //    free(this.values[i].name);
            //}
            //free(this.values);

            for (int i = 0 ; i < this.fonts_size ; i++) {
                //free(this.fonts[i].name);
                this.fonts[i].fontholder.Destroy();
            }
            //free(this.fonts);

            for (int i = 0 ; i < this.textures_size ; i++) {
                this.textures[i].Destroy();
            }
            //free(this.textures);

            for (int i = 0 ; i < this.group_list_size ; i++) {
                //free(this.group_list[i].name);
                //free(this.group_list[i].initial_action_name);
                Layout.HelperDestroyActions(this.group_list[i].actions, this.group_list[i].actions_size);
                if (this.group_list[i].psframebuffer != null) this.group_list[i].psframebuffer.Destroy();
            }
            //free(this.group_list);

            for (int i = 0 ; i < this.camera_list_size ; i++) {
                //free(this.camera_list[i].name);
                if (this.camera_list[i].animation != null) this.camera_list[i].animation.Destroy();
            }
            //free(this.camera_list);

            for (int i = 0 ; i < this.trigger_list_size ; i++) {
                //free(this.trigger_list[i].name);
                //free(this.trigger_list[i].initial_action_name);
                //free(this.trigger_list[i].action_name);
                //free(this.trigger_list[i].camera_name);
                //free(this.trigger_list[i].trigger_name);

            }
            //free(this.trigger_list);

            for (int i = 0 ; i < this.sound_list_size ; i++) {
                HelperDestroyActions(this.sound_list[i].actions, this.sound_list[i].actions_size);
                //free(this.sound_list[i].name);
                //free(this.sound_list[i].initial_action_name);
                this.sound_list[i].soundplayer.Destroy();

            }
            //free(this.sound_list);

            for (int i = 0 ; i < this.macro_list_size ; i++) {
                for (int j = 0 ; j < this.macro_list[i].actions_size ; j++) {
                    //free(this.macro_list[i].actions[j].target_name);
                    //free(this.macro_list[i].actions[j].action_name);
                    //free(this.macro_list[i].actions[j].trigger_name);
                    //free(this.macro_list[i].actions[j].camera_name);
                }
                //free(this.macro_list[i].actions);
            }
            //free(this.macro_list);

            this.camera_helper.Destroy();
            this.camera_secondary_helper.Destroy();

            //free(this.external_vertex_list);
            //free(this.z_buffer);

            Luascript.DropShared(this.modifier_camera);
            Luascript.DropShared(this.modifier_camera_secondary);
            Luascript.DropShared(this);
            //free(layout);
        }


        public int TriggerAny(string action_triger_camera_interval_name) {
            int res = 0;
            res += TriggerAction(null, action_triger_camera_interval_name);
            res += TriggerCamera(action_triger_camera_interval_name) ? 1 : 0;
            res += TriggerTrigger(action_triger_camera_interval_name);
            return res;
        }

        public int TriggerAction(string target_name, string action_name) {
            int count = 0;
            string initial_action_name;

            for (int i = 0 ; i < this.vertex_list_size ; i++) {
                if (target_name != null && this.vertex_list[i].name != target_name) continue;

                if (action_name == null && this.vertex_list[i].initial_action_name != null)
                    initial_action_name = this.vertex_list[i].initial_action_name;
                else
                    initial_action_name = action_name;

                for (int j = 0 ; j < this.vertex_list[i].actions_size ; j++) {
                    Action action = this.vertex_list[i].actions[j];

                    if (action.name == initial_action_name || action.name == action_name) {
                        HelperExecuteAction(this.vertex_list[i], action);
                        count++;
                    }
                }
            }

            for (int i = 0 ; i < this.group_list_size ; i++) {
                if (target_name != null && this.group_list[i].name != target_name) continue;

                for (int j = 0 ; j < this.group_list[i].actions_size ; j++) {
                    Action action = this.group_list[i].actions[j];

                    if (action_name == null && this.group_list[i].initial_action_name != null)
                        initial_action_name = this.group_list[i].initial_action_name;
                    else
                        initial_action_name = action_name;

                    if (action.name == initial_action_name || action.name == action_name) {
                        HelperExecuteActionInGroup(action, this.group_list[i]);
                        count++;
                    }
                }
            }

            for (int i = 0 ; i < this.sound_list_size ; i++) {
                if (target_name != null && this.sound_list[i].name != target_name) continue;

                for (int j = 0 ; j < this.sound_list[i].actions_size ; j++) {
                    Action action = this.sound_list[i].actions[j];

                    if (action_name == null && this.sound_list[i].initial_action_name != null)
                        initial_action_name = this.sound_list[i].initial_action_name;
                    else
                        initial_action_name = action_name;

                    if (action.name == initial_action_name || action.name == action_name) {
                        HelperExecuteActionInSound(action, this.sound_list[i]);
                        count++;
                    }
                }
            }

            return count;
        }

        public int ContainsAction(string action_name, string item_name) {
            if (item_name == null) throw new ArgumentNullException("entry_name", "entry_name is required");

            int count = 0;

            for (int i = 0 ; i < this.vertex_list_size ; i++) {
                if (this.vertex_list[i].name != item_name) continue;
                for (int j = 0 ; j < this.vertex_list[i].actions_size ; j++) {
                    if (this.vertex_list[i].actions[j].name == action_name) count++;
                }
            }

            for (int i = 0 ; i < this.group_list_size ; i++) {
                if (this.group_list[i].name != item_name) continue;
                for (int j = 0 ; j < this.group_list[i].actions_size ; j++) {
                    if (this.group_list[i].actions[j].name == action_name) count++;
                }
            }

            for (int i = 0 ; i < this.sound_list_size ; i++) {
                if (this.sound_list[i].name != item_name) continue;
                for (int j = 0 ; j < this.sound_list[i].actions_size ; j++) {
                    if (this.sound_list[i].actions[j].name == action_name) count++;
                }
            }

            return count;
        }

        public int AnimationIsCompleted(string item_name) {
            if (item_name == null) throw new ArgumentNullException("entry_name", "entry_name is required");

            AnimSprite obj = null;
            bool is_drawable = false;
            bool found = false;

            for (int i = 0 ; i < this.vertex_list_size ; i++) {
                if (this.vertex_list[i].name == item_name) {
                    obj = this.vertex_list[i].animation;
                    is_drawable = this.vertex_list[i].type == PVRContextVertex.DRAWABLE;
                    found = true;
                    break;
                }
            }

            if (!found) {
                // check if a group with this name exists
                for (int i = 0 ; i < this.group_list_size ; i++) {
                    if (this.group_list[i].name == item_name) {
                        obj = this.vertex_list[i].animation;
                        found = true;
                        break;
                    }
                }
            }

            if (!found) return 2;

            if (obj == null) return 1;

            if (is_drawable) {
                //
                // Note: is not posible peek the drawable animation, call .Animate()
                // with a zero elapsed duration to check if was completed.
                //
                if (obj.Animate(0) > 0) return 1;
            } else {
                if (obj.IsCompleted()) return 1;
            }

            return 0;
        }

        public void UpdateRenderSize(float screen_width, float screen_height) {
            if (this.keep_aspect_ratio) {
                float scale_x = screen_width / this.viewport_width;
                float scale_y = screen_height / this.viewport_height;
                float scale = Math.Min(scale_x, scale_y);

                this.modifier_viewport.scale_x = this.modifier_viewport.scale_y = scale;
                this.modifier_viewport.translate_x = (screen_width - this.viewport_width * scale) / 2f;
                this.modifier_viewport.translate_y = (screen_height - this.viewport_height * scale) / 2f;
            } else {
                this.modifier_viewport.scale_x = screen_width / this.viewport_width;
                this.modifier_viewport.scale_y = screen_height / this.viewport_height;
                this.modifier_viewport.translate_x = this.modifier_viewport.translate_y = 0f;
            }
        }

        public void CameraSetView(float x, float y, float depth) {
            this.camera_helper.SetAbsolute(x, y, depth);
        }

        public bool CameraIsCompleted() {
            return this.camera_helper.IsCompleted();
        }

        public bool TriggerCamera(string camera_name) {
            return this.camera_helper.FromLayout(this, camera_name);
        }

        public int TriggerTrigger(string trigger_name) {
            int count = 0;
            for (int i = 0 ; i < this.trigger_list_size ; i++) {
                if (this.trigger_list[i].name == trigger_name) {
                    HelperExecuteTrigger(this.trigger_list[i]);
                }
            }
            return count;
        }

        public void StopTrigger(string trigger_name) {
            for (int i = 0 ; i < this.trigger_list_size ; i++) {
                if (this.trigger_list[i].name == trigger_name)
                    this.trigger_list[i].context.running = false;
            }
        }

        public void StopAllTriggers() {
            for (int i = 0 ; i < this.trigger_list_size ; i++) {
                this.trigger_list[i].context.running = false;
            }
        }

        public bool SetPlaceholderDrawableById(int id, Drawable drawable) {
            if (id < 0 || id >= this.vertex_list_size) return false;
            if (this.vertex_list[id].type != PVRContextVertex.DRAWABLE) return false;

            this.vertex_list[id].placeholder.vertex = drawable;
            return true;
        }

        public void SyncTriggersWithGlobalBeatwatcher(bool enable) {
            this.beatwatcher_synced_triggers = enable;
            this.beatwatcher.last_global_timestamp = -1;
            this.beatwatcher.resyncs = 1;
        }

        public void SetBpm(float beats_per_minute) {
            float duration = Math2D.BeatsPerMinuteToBeatPerMilliseconds(beats_per_minute);

            this.camera_helper.SetBpm(beats_per_minute);
            this.camera_secondary_helper.SetBpm(beats_per_minute);

            this.beatwatcher.ChangeBpm(beats_per_minute);

            for (int i = 0 ; i < this.trigger_list_size ; i++) {
                Trigger trigger = this.trigger_list[i];

                if (trigger.loop_delay_beats_in_beats) {
                    trigger.loop_delay = trigger.loop_delay_beats * duration;
                }
                if (trigger.start_delay_beats_in_beats) {
                    trigger.start_delay = trigger.start_delay_beats * duration;
                }
            }
        }


        public LayoutPlaceholder GetPlaceholder(string name) {
            for (int i = 0 ; i < this.vertex_list_size ; i++) {
                if (this.vertex_list[i].placeholder != null && this.vertex_list[i].placeholder.name == name) {
                    return this.vertex_list[i].placeholder;
                }
            }
            return null;
        }

        public int GetPlaceholderId(string name) {
            for (int i = 0 ; i < this.vertex_list_size ; i++) {
                if (this.vertex_list[i].placeholder != null && this.vertex_list[i].placeholder.name == name) {
                    return i;
                }
            }
            return -1;
        }


        public TextSprite GetTextsprite(string name) {
            return (TextSprite)HelperGetVertex(PVRContextVertex.TEXTSPRITE, name);
        }

        public Sprite GetSprite(string name) {
            return (Sprite)HelperGetVertex(PVRContextVertex.SPRITE, name);
        }

        public SoundPlayer GetSoundplayer(string name) {
            for (int i = 0 ; i < this.sound_list_size ; i++) {
                if (this.sound_list[i].name == name) {
                    return this.sound_list[i].soundplayer;
                }
            }
            return null;
        }

        public Camera GetCameraHelper() {
            return this.camera_helper;
        }

        public Camera GetSecondaryCameraHelper() {
            return this.camera_secondary_helper;
        }


        public FontHolder GetAttachedFont(string font_name) {
            if (font_name == null)
                return this.fonts_size < 1 ? null : this.fonts[0].fontholder;

            for (int i = 0 ; i < this.fonts_size ; i++) {
                if (this.fonts[i].name == font_name)
                    return this.fonts[i].fontholder;
            }

            return null;
        }

        public object GetAttachedValue(string name, AttachedValueType expected_type, object default_value) {
            for (int i = 0 ; i < this.values_size ; i++) {
                if (this.values[i].name != name) continue;

                if ((this.values[i].type & expected_type) == 0x00) {
                    Console.Error.WriteLine("[WARN] layout_get_attached_value() type missmatch of: " + name);

                    if (expected_type == AttachedValueType.FLOAT && this.values[i].type == AttachedValueType.INTEGER)
                        return this.values[i].value;

                    break;// type missmatch
                }
                return this.values[i].value;
            }
            //Console.Error.WriteLine("layout_get_attached_value() value not found: " + name);
            return default_value;
        }

        public AttachedValueType GetAttachedValue2(string name, out object result) {
            for (int i = 0 ; i < this.values_size ; i++) {
                if (this.values[i].name != name) continue;

                result = this.values[i].value;
                return this.values[i].type;
            }

            result = null;
            return AttachedValueType.NOTFOUND;
        }

        public float GetAttachedValueAsFloat(string name, float default_value) {
            const AttachedValueType LIKE_NUMBER = AttachedValueType.INTEGER | AttachedValueType.FLOAT | AttachedValueType.HEX;

            for (int i = 0 ; i < this.values_size ; i++) {
                if (this.values[i].name != name) continue;

                if ((this.values[i].type & LIKE_NUMBER) == 0x00) break;// type missmatch

                // convert the value to float (IEEE 754)
                switch (this.values[i].type) {
                    case AttachedValueType.INTEGER:
                        return (float)((long)this.values[i].value);
                    case AttachedValueType.FLOAT:
                        return (float)((double)this.values[i].value);
                    case AttachedValueType.HEX:
                        return (float)((uint)this.values[i].value);
                }
            }
            return default_value;
        }

        public AttachedValueType GetAttachedValueType(string name) {
            for (int i = 0 ; i < this.values_size ; i++) {
                if (this.values[i].name == name) return this.values[i].type;
            }
            return AttachedValueType.NOTFOUND;
        }

        public Modifier GetModifierViewport() {
            return this.modifier_viewport;
        }

        public Modifier GetModifierCamera() {
            return this.modifier_camera;
        }

        public void GetViewportSize(out float viewport_width, out float viewport_height) {
            viewport_width = this.viewport_width;
            viewport_height = this.viewport_height;
        }

        public void ExternalVertexCreateEntries(int amount) {
            if (amount < 0) amount = 0;

            if (this.external_vertex_list != null) {
                Array.Resize(ref this.external_vertex_list, amount);
                //if (this.external_vertex_list == null) throw new Exception("layout_external_vertex_create_entries() not enough system memory");
            } else {
                this.external_vertex_list = new Item[amount];
            }

            for (int i = this.external_vertex_list_size ; i < amount ; i++) {
                this.external_vertex_list[i] = new Item() {
                    vertex = null,
                    type = PVRContextVertex.DRAWABLE,
                    group_id = 0,// layout root
                    parallax = new LayoutParallax() { x = 10f, y = 1.0f, z = 1.0f },
                    static_camera = false,
                    placeholder = null
                };
            }
            this.external_vertex_list_size = amount;

            this.z_buffer_size = this.vertex_list_size + this.external_vertex_list_size;
            Array.Resize(ref this.z_buffer, this.z_buffer_size);
            //if (!this.z_buffer) throw new Exception("not enough system memory");

            // re-build z-buffer
            HelperZbufferBuild();
        }

        public bool ExternalVertexSetEntry(int index, PVRContextVertex vertex_type, IVertex vertex, int group_id) {
            if (index < 0 || index >= this.external_vertex_list_size) return false;
            if (group_id < 0 || group_id >= this.group_list_size) group_id = 0;
            this.external_vertex_list[index].vertex = vertex;
            this.external_vertex_list[index].type = vertex_type;
            this.external_vertex_list[index].group_id = group_id;
            return true;
        }

        public int ExternalCreateGroup(string group_name, int parent_group_id) {
            if (group_name != null) {
                int index = HelperGetGroupIndex(group_name);
                if (index >= 0) {
                    Console.Error.WriteLine("layout_external_create_group() the group '" + group_name + "' already exists");
                    return -1;
                }
            }

            if (parent_group_id < 0 || parent_group_id >= this.group_list_size) {
                // layout root
                parent_group_id = 0;
            }

            // increase group_list size
            int group_id = this.group_list_size++;
            Array.Resize(ref this.group_list, this.group_list_size);
            //if (!this.group_list) throw new Exception("layout_external_create_group() out-of-memory");

            this.group_list[group_id] = new Group() {
                name = group_name,
                group_id = parent_group_id,
                actions = null,
                actions_size = 0,
                initial_action_name = null,
                antialiasing = PVRContextFlag.DEFAULT,

                visible = true,
                alpha = 1.0f,
                offsetcolor = new float[4],
                modifier = new Modifier(),
                parallax = new LayoutParallax() { x = 1.0f, y = 1.0f, z = 1.0f },
                static_camera = false,
                static_screen = null,

                animation = null,
                psshader = null,
                psframebuffer = null,

                blend_enabled = true,
                blend_src_rgb = Blend.DEFAULT,
                blend_dst_rgb = Blend.DEFAULT,
                blend_src_alpha = Blend.DEFAULT,
                blend_dst_alpha = Blend.DEFAULT,

                viewport_x = -1f,
                viewport_y = -1f,
                viewport_width = -1f,
                viewport_height = -1f,

                context = new GroupContext() {
                    visible = true,
                    alpha = 1.0f,
                    antialiasing = PVRContextFlag.DEFAULT,
                    matrix = new SH4Matrix(),
                    offsetcolor = new float[4],
                    parallax = new LayoutParallax() { x = 1.0f, y = 1.0f, z = 1.0f },

                    next_child = null,
                    next_sibling = null,
                    parent_group = null,
                    last_z_index = -1
                }
            };

            // append to parent group
            HelperAddGroupToParent(
                this.group_list[parent_group_id].context, this.group_list[group_id]
            );

            //this.group_list[group_id].matrix.Reset();
            this.group_list[group_id].modifier.Clear();
            PVRContext.HelperClearOffsetColor(this.group_list[group_id].offsetcolor);

            return group_id;
        }

        public bool SetGroupStaticToCamera(string group_name, bool enable) {
            int index = HelperGetGroupIndex(group_name);
            if (index < 0) return false;
            this.group_list[index].static_camera = enable;
            return true;
        }

        public bool SetGroupStaticToCameraById(int group_id, bool enable) {
            if (group_id < 0 || group_id >= this.group_list_size) return false;
            this.group_list[group_id].static_camera = !!enable;
            return true;
        }

        public bool SetGroupStaticToScreenById(int group_id, SH4Matrix sh4matrix) {
            if (group_id < 0 || group_id >= this.group_list_size) return false;
            this.group_list[group_id].static_screen = sh4matrix;
            return true;
        }

        public bool ExternalVertexSetEntryStatic(int vertex_index, bool enable) {
            if (vertex_index < 0 || vertex_index >= this.external_vertex_list_size) return false;
            this.external_vertex_list[vertex_index].static_camera = enable;
            return true;
        }

        public void SetGroupVisibility(string group_name, bool visible) {
            int index = HelperGetGroupIndex(group_name);
            if (index >= 0) this.group_list[index].visible = visible;
        }

        public void SetGroupVisibilityById(int group_id, bool visible) {
            if (group_id < 0 || group_id >= this.group_list_size) return;
            this.group_list[group_id].visible = visible;
        }

        public void SetGroupAlpha(string group_name, float alpha) {
            if (Single.IsNaN(alpha)) alpha = 0.0f;
            int index = HelperGetGroupIndex(group_name);
            if (index >= 0) this.group_list[index].alpha = Math2D.Clamp(alpha, 0.0f, 1.0f);
        }

        public void SetGroupAlphaById(int group_id, float alpha) {
            if (Single.IsNaN(alpha)) alpha = 0.0f;
            if (group_id < 0 || group_id >= this.group_list_size) return;
            this.group_list[group_id].alpha = Math2D.Clamp(alpha, 0.0f, 1.0f);
        }

        public void SetGroupAntialiasing(string group_name, PVRContextFlag antialiasing) {
            int index = HelperGetGroupIndex(group_name);
            if (index >= 0) this.group_list[index].antialiasing = antialiasing;
        }

        public void SetGroupAntialiasingById(int group_id, PVRContextFlag antialiasing) {
            if (group_id < 0 || group_id >= this.group_list_size) return;
            this.group_list[group_id].antialiasing = antialiasing;
        }

        public void SetGroupOffsetcolor(string group_name, float r, float g, float b, float a) {
            int index = HelperGetGroupIndex(group_name);
            if (index >= 0) {
                if (!Single.IsNaN(r)) this.group_list[index].offsetcolor[0] = Math2D.Clamp(r, 0.0f, 1.0f);
                if (!Single.IsNaN(g)) this.group_list[index].offsetcolor[1] = Math2D.Clamp(g, 0.0f, 1.0f);
                if (!Single.IsNaN(b)) this.group_list[index].offsetcolor[2] = Math2D.Clamp(b, 0.0f, 1.0f);
                if (!Single.IsNaN(a)) this.group_list[index].offsetcolor[3] = Math2D.Clamp(a, 0.0f, 1.0f);
            }
        }

        public Modifier GetGroupModifier(string group_name) {
            int index = HelperGetGroupIndex(group_name);
            return index < 0 ? null : this.group_list[index].modifier;
        }

        public Modifier GetGroupModifierById(int group_id) {
            if (group_id < 0 || group_id >= this.group_list_size) return null;
            return this.group_list[group_id].modifier;
        }

        public int GetGroupId(string group_name) {
            return HelperGetGroupIndex(group_name);
        }

        internal CameraPlaceholder GetCameraPlaceholder(string camera_name) {
            for (int i = 0 ; i < this.camera_list_size ; i++) {
                if (this.camera_list[i].name == camera_name) {
                    return this.camera_list[i];
                }
            }
            return null;
        }

        public bool SetSingleItemToDraw(string item_name) {
            this.single_item = null;
            if (item_name == null) return true;
            for (int i = 0 ; i < this.vertex_list_size ; i++) {
                if (this.vertex_list[i].name == item_name) {
                    this.single_item = this.vertex_list[i];
                    return true;
                }
            }
            return false;
        }

        public void Suspend() {
            for (int i = 0 ; i < this.sound_list_size ; i++) {
                SoundPlayer soundplayer = this.sound_list[i].soundplayer;
                this.sound_list[i].was_playing = soundplayer.IsPlaying();
                if (this.sound_list[i].was_playing) soundplayer.Pause();
            }
            this.suspended = true;
        }

        public void Resume() {
            for (int i = 0 ; i < this.sound_list_size ; i++) {
                SoundPlayer soundplayer = this.sound_list[i].soundplayer;
                if (this.sound_list[i].was_playing) soundplayer.Play();
            }
            this.suspended = false;
        }

        public void DisableAntialiasing(bool disable) {
            this.antialiasing_disabled = disable;
        }

        public bool IsAntialiasingDisabled() {
            return this.antialiasing_disabled;
        }

        public PVRContextFlag GetLayoutAntialiasing() {
            return this.group_list[0].antialiasing;
        }

        public void SetLayoutAntialiasing(PVRContextFlag flag) {
            this.group_list[0].antialiasing = flag;
        }

        public void SetShader(PSShader psshader) {
            this.psshader = psshader;
        }

        public PSShader GetGroupShader(string group_name) {
            int index = this.HelperGetGroupIndex(group_name);
            if (index < 0) return null;

            return this.group_list[index].psshader;
        }

        public bool SetGroupShader(string group_name, PSShader psshader) {
            int index = HelperGetGroupIndex(group_name);
            if (index < 0) return false;

            this.group_list[index].psshader = psshader;
            return true;
        }


        public int Animate(float elapsed) {
            if (this.suspended) return 0;
            if (this.animation_speed != 1.0f) elapsed *= this.animation_speed;

            int completed = 0;

            completed += HelperCheckTriggerQueue(elapsed);

            for (int i = 0 ; i < this.vertex_list_size ; i++) {
                switch (this.vertex_list[i].type) {
                    case PVRContextVertex.SPRITE:
                        completed += this.vertex_list[i].vertex.Animate(elapsed);
                        break;
                    case PVRContextVertex.TEXTSPRITE:
                        completed += this.vertex_list[i].vertex.Animate(elapsed);
                        break;
                    case PVRContextVertex.DRAWABLE:
                        LayoutPlaceholder placeholder = this.vertex_list[i].placeholder;
                        if (placeholder.vertex != null)
                            completed += placeholder.vertex.Animate(elapsed);
                        break;
                }
            }

            for (int i = 0 ; i < this.external_vertex_list_size ; i++) {
                IVertex vertex = this.external_vertex_list[i].vertex;
                if (vertex == null) continue;

                switch (this.external_vertex_list[i].type) {
                    case PVRContextVertex.SPRITE:
                        completed += vertex.Animate(elapsed);
                        break;
                    case PVRContextVertex.TEXTSPRITE:
                        completed += vertex.Animate(elapsed);
                        break;
                    case PVRContextVertex.DRAWABLE:
                        if (vertex != null)
                            completed += vertex.Animate(elapsed);
                        break;
                }
            }

            for (int i = 0 ; i < this.group_list_size ; i++) {
                completed += Layout.HelperGroupAnimate(this.group_list[i], elapsed);
            }

            for (int i = 0 ; i < this.fonts_size ; i++) {
                FontHolder fontholder = this.fonts[i].fontholder;
                if (fontholder.font_from_atlas) completed += fontholder.font.Animate(elapsed);
            }

            if (this.camera_helper.Animate(elapsed) < 1) {
                this.camera_helper.Apply(null);
            }

            if (this.camera_secondary_helper.Animate(elapsed) < 1) {
                this.camera_secondary_helper.Apply(null);
            }

            return completed;
        }

        public void Draw(PVRContext pvrctx) {
            pvrctx.Save();
            if (this.psshader != null) pvrctx.AddShader(this.psshader);

            if (this.antialiasing_disabled) pvrctx.SetGlobalAntialiasing(PVRContextFlag.DISABLE);

            if (this.resolution_changes != pvrctx.resolution_changes) {
                UpdateRenderSize(pvrctx.ScreenWidth, pvrctx.ScreenHeight);
                this.resolution_changes = pvrctx.resolution_changes;

                for (int i = 0 ; i < this.group_list_size ; i++) {
                    if (this.group_list[i].psframebuffer != null)
                        this.group_list[i].psframebuffer.Resize();
                }
            }

            SH4Matrix pvr_matrix = new SH4Matrix();
            pvrctx.CurrentMatrix.CopyTo(pvr_matrix);

            pvrctx.ApplyModifier(this.modifier_viewport);

            this.camera_secondary_helper.ApplyOffset(pvrctx.CurrentMatrix);
            pvrctx.ApplyModifier(this.modifier_camera_secondary);

            this.camera_helper.ApplyOffset(pvrctx.CurrentMatrix);

            // step 1: sort z_buffer
            for (int i = 0 ; i < this.z_buffer_size ; i++) {
                Item item = this.z_buffer[i].item;
                IVertex vertex = item.vertex;
                PVRContextVertex vertex_type = item.type;

                switch (vertex_type) {
                    case PVRContextVertex.SPRITE:
                        this.z_buffer[i].z_index = vertex.GetZIndex();
                        this.z_buffer[i].visible = vertex.IsVisible();
                        break;
                    case PVRContextVertex.TEXTSPRITE:
                        this.z_buffer[i].z_index = vertex.GetZIndex();
                        this.z_buffer[i].visible = vertex.IsVisible();
                        break;
                    case PVRContextVertex.DRAWABLE:
                        if (item.placeholder != null) vertex = item.placeholder.vertex;
                        if (vertex != null) {
                            this.z_buffer[i].z_index = vertex.GetZIndex();
                            this.z_buffer[i].visible = vertex.IsVisible();
                        } else {
                            this.z_buffer[i].z_index = Math2D.MAX_INT32;
                            this.z_buffer[i].visible = false;
                        }
                        break;
                }
            }
            Array.Sort(this.z_buffer, 0, this.z_buffer_size, Layout.HelperZbufferSort);

            // step 2: find top-most item of each group
            for (int i = 0 ; i < this.z_buffer_size ; i++) {
                Group group = this.group_list[this.z_buffer[i].item.group_id];
                group.context.last_z_index = i;
            }

            // step 3: build root group context
            Group layout_root = this.group_list[0];

            //sh4matrix_copy_to(layout_root.matrix, layout_root.context.matrix);
            layout_root.context.matrix.Clear();
            layout_root.context.matrix.ApplyModifier(layout_root.modifier);

            layout_root.context.alpha = layout_root.alpha;
            layout_root.context.antialiasing = layout_root.antialiasing;
            layout_root.context.visible = layout_root.visible;
            layout_root.context.parallax.x = layout_root.parallax.x;
            layout_root.context.parallax.y = layout_root.parallax.y;
            layout_root.context.parallax.z = layout_root.parallax.z;
            for (int i = 0 ; i < 4 ; i++) layout_root.context.offsetcolor[i] = layout_root.offsetcolor[i];

            // step 4: stack all groups
            Layout.HelperStackGroups(layout_root);

            // step 5: draw all layout items
            bool has_single_item = this.single_item != null;
            for (int i = 0 ; i < this.z_buffer_size ; i++) {
                if (!this.z_buffer[i].visible) continue;
                if (has_single_item && this.z_buffer[i].item != this.single_item) continue;

                IVertex vertex = this.z_buffer[i].item.vertex;
                PVRContextVertex vertex_type = this.z_buffer[i].item.type;
                Group group = this.group_list[this.z_buffer[i].item.group_id];
                LayoutParallax item_parallax = this.z_buffer[i].item.parallax;
                bool item_is_static_to_camera = this.z_buffer[i].item.static_camera;

                if (this.z_buffer[i].item.placeholder != null) {
                    item_is_static_to_camera = this.z_buffer[i].item.placeholder.static_camera;
                    vertex = this.z_buffer[i].item.placeholder.vertex;
                    item_parallax = this.z_buffer[i].item.placeholder.parallax;
                }

                if (!group.context.visible) continue;

                pvrctx.Save();

                // check whatever the current and/or parent group has framebuffer
                if (group.psframebuffer != null) {
                    pvrctx.SetFramebuffer(group.psframebuffer);
                } else {
                    // use group and parent group shaders
                    Layout.HelperStackGroupsShaders(group, pvrctx);
                    pvrctx.SetFramebuffer(null);
                }

                SH4Matrix matrix = pvrctx.CurrentMatrix;

                // apply group context (¿should be applied after the camera?)
                matrix.MultiplyWithMatrix(group.context.matrix);
                pvrctx.SetGlobalAlpha(group.psframebuffer != null ? 1.0f : group.context.alpha);
                pvrctx.SetGlobalAntialiasing(group.context.antialiasing);
                pvrctx.SetGlobalOffsetColor(group.context.offsetcolor);

                if (item_is_static_to_camera || group.static_camera || group.static_screen != null) {
                    if (group.static_screen != null) {
                        pvr_matrix.CopyTo(matrix);
                        matrix.MultiplyWithMatrix(group.static_screen);
                    } else {
                        this.camera_helper.UnapplyOffset(matrix);
                    }

                    //pvrctx.Flush();

                    switch (vertex_type) {
                        case PVRContextVertex.SPRITE:
                            vertex.Draw(pvrctx);
                            break;
                        case PVRContextVertex.TEXTSPRITE:
                            vertex.Draw(pvrctx);
                            break;
                        case PVRContextVertex.DRAWABLE:
                            vertex.Draw(pvrctx);
                            break;
                    }

                    pvrctx.Restore();
                    continue;
                }


                float translate_x = this.modifier_camera.translate_x;
                float translate_y = this.modifier_camera.translate_y;
                float px = group.context.parallax.x * item_parallax.x;
                float py = group.context.parallax.y * item_parallax.y;
                float pz = group.context.parallax.z * item_parallax.z;

                // parallax z as percent of camera scale
                float scale_x = 1.0f - ((1.0f - this.modifier_camera.scale_x) * pz);
                float scale_y = 1.0f - ((1.0f - this.modifier_camera.scale_y) * pz);

                // parallax translation
                float tx = translate_x * px;
                float ty = translate_y * py;

                // camera translation+scale position correction
                tx += px * ((translate_x * scale_x) - translate_x);
                ty += py * ((translate_y * scale_y) - translate_y);

                // camera with parallax scale correction
                tx += (this.viewport_width * (Math.Abs(scale_x) - 1.0f) * Math.Sign(scale_x)) / -2.0f;
                ty += (this.viewport_height * (Math.Abs(scale_y) - 1.0f) * Math.Sign(scale_y)) / -2.0f;

                // apply translation (with all parallax corrections)
                matrix.Translate(tx, ty);

                // parallax scale
                matrix.Scale(scale_x, scale_y);

                //pvrctx.Flush();

                vertex.Draw(pvrctx);

                pvrctx.Restore();

                // if the last item of the current group was drawn, flush the group framebuffer
                if (group.psframebuffer != null && group.context.last_z_index == i) {

                    pvrctx.Save();

                    // draw group framebuffer
                    pvrctx.SetFramebuffer(null);

                    // use group and parent group shaders
                    Layout.HelperStackGroupsShaders(group, pvrctx);

                    pvrctx.SetVertexBlend(
                        group.blend_enabled, group.blend_src_rgb, group.blend_dst_rgb, group.blend_src_alpha, group.blend_dst_alpha
                    );

                    // draw group framebuffer in the screen
                    float x = group.viewport_x > 0 ? group.viewport_x : 0;
                    float y = group.viewport_y > 0 ? group.viewport_y : 0;
                    float width = group.viewport_width > 0 ? group.viewport_width : this.viewport_width;
                    float height = group.viewport_height > 0 ? group.viewport_height : this.viewport_height;

                    //pvr_context_apply_modifier(pvrctx, layout.modifier_viewport);
                    pvrctx.SetVertexAlpha(group.context.alpha);

                    float sx = x * this.modifier_viewport.scale_x;
                    float sy = y * this.modifier_viewport.scale_y;
                    float sw = width * this.modifier_viewport.scale_x;
                    float sh = height * this.modifier_viewport.scale_y;

                    pvrctx.DrawFramebuffer(group.psframebuffer, sx, sy, sw, sh, x, y, width, height);

                    pvrctx.Restore();
                    group.psframebuffer.Invalidate();
                }
            }

            pvrctx.Restore();
        }



        //////////////////////////////////
        ///          HELPERS           ///
        //////////////////////////////////

        private static void HelperDestroyActions(Action[] actions, int actions_size) {
            for (int i = 0 ; i < actions_size ; i++) {
                Action action = actions[i];

                for (int j = 0 ; j < action.entries_size ; j++) {
                    switch (action.entries[j].type) {
                        case Layout.ACTION_ANIMATION:
                            ((AnimSprite)action.entries[j].misc).Destroy();
                            break;
                        case Layout.ACTION_ATLASAPPLY:
                            //free(action.entries[j].misc);
                            break;
                        case Layout.ACTION_PROPERTY:
                            //if (action.entries[j].property == VertexProps.TEXTSPRITE_PROP_STRING)
                            //    free(action.entries[j].misc);
                            break;
                        case Layout.ACTION_SETSHADER:
                            ((PSShader)action.entries[j].misc).Destroy();
                            break;
                        case Layout.ACTION_SETSHADERUNIFORM:
                            //free(action.entries[j].uniform_name);
                            //free(action.entries[j].misc);
                            break;
                    }
                }
                //free(action.entries);
                //free(action.name);

            }
            //free(actions);
        }

        private static object HelperGetResource(ResourcePool resource_pool, string src, bool is_texture) {
            ArrayList<ResourcePoolEntry> pool = is_texture ? resource_pool.textures : resource_pool.atlas;

            foreach (ResourcePoolEntry definition in pool) {
                if (definition.src == src) return definition.data;
            }

            // resource not found in the pool load it
            object data;

            if (is_texture)
                data = Texture.InitDeferred(src, true/* do not upload to the PVR VRAM */);
            else
                data = Atlas.Init(src);

            if (data == null) {
                string path = FS.GetFullPathAndOverride(src);
                Console.Error.WriteLine("[WARN] layout_helper_get_resource() missing resource '" + src + "' (" + path + ")");
                //free(path);
            }

            ResourcePoolEntry new_definition = new ResourcePoolEntry() { data = data, src = src, is_texture = is_texture };
            pool.Add(new_definition);

            return data;
        }

        private static void HelperLocation(ActionEntry action_entry, float width, float height, float v_width, float v_height, out float location_x, out float location_y) {
            Align align_vertical = action_entry.align_vertical;
            Align align_horizontal = action_entry.align_horizontal;
            location_x = action_entry.x;
            location_y = action_entry.y;

            float offset_x = 0, offset_y = 0;
            // Note: align center means relative to the viewport

            // vertical align
            switch (align_vertical) {
                case Align.START:
                    offset_y = 0;
                    break;
                case Align.CENTER:
                    offset_y = (v_height - height) / 2.0f;
                    break;
                case Align.END:
                    offset_y = v_height - height;
                    break;
            }

            // horizontal align
            switch (align_horizontal) {
                case Align.START:
                    offset_x = 0;
                    break;
                case Align.CENTER:
                    offset_x = (v_width - width) / 2.0f;
                    break;
                case Align.END:
                    offset_x = v_width - width;
                    break;
            }

            offset_x += location_x;
            offset_y += location_y;

            location_x = offset_x;
            location_y = offset_y;
        }

        private static uint HelperParseHex(XmlParserNode node, string attr_name, uint def_value) {
            uint value;
            if (!VertexProps.ParseHex(node.GetAttribute(attr_name), out value, false)) {
                Console.Error.WriteLine("[ERROR] layout_helper_parse_hex() invalid value of '" + attr_name + "': " + node.OuterHTML);
                return def_value;
            }

            return value;
        }

        private static float HelperParseFloat(XmlParserNode node, string attr_name, float def_value) {
            string str = node.GetAttribute(attr_name);
            if (String.IsNullOrEmpty(str)) return def_value;

            float value = VertexProps.ParseFloat2(str, Single.NaN);

            if (Single.IsNaN(value)) {
                Console.Error.WriteLine("[ERROR] layout_parse_float(): invalid value: " + str);
                return def_value;
            }

            return value;
        }

        private static Align HelperParseAlign(XmlParserNode node, bool is_vertical) {
            string attribute = is_vertical ? "alignVertical" : "alignHorizontal";
            return VertexProps.ParseAlign(node, attribute, false, true);
        }

        private static Align HelperParseAlign2(XmlParserNode node, bool is_vertical, Align def_value) {
            string attribute = is_vertical ? "alignVertical" : "alignHorizontal";
            if (!node.HasAttribute(attribute)) return def_value;

            Align align = VertexProps.ParseAlign2(node.GetAttribute(attribute));
            if (align == Align.BOTH) {
                align = Align.START;
                Console.Error.WriteLine("layout_helper_parse_align2() invalid align found at: " + node.OuterHTML);
            }

            return align;
        }

        private static void HelperParseColor(XmlParserNode node, float[] rgba) {
            if (node.HasAttribute("color")) {
                //
                // Check if the color is rgba (0xRRGGBBAA format) or rgb (0xRRGGBB format)
                //
                string value = node.GetAttribute("color");
                int length = value.Length;
                if (value.StartsWithKDY("0x", 0) || value.StartsWithKDY("0X", 0)) length -= 2;

                uint raw_value = Layout.HelperParseHex(node, "color", 0xFFFFFF);
                bool has_alpha = length == 8;
                Math2D.ColorBytesToFloats(raw_value, has_alpha, rgba);
            } else if (node.HasAttribute("rgb")) {
                uint rgb8_color = Layout.HelperParseHex(node, "rgb", 0xFFFFFF);
                Math2D.ColorBytesToFloats(rgb8_color, false, rgba);
            } else if (node.HasAttribute("rgba")) {
                uint rgba8_color = Layout.HelperParseHex(node, "rgba", 0xFFFFFFFF);
                Math2D.ColorBytesToFloats(rgba8_color, true, rgba);
            } else if (node.HasAttribute("argb")) {
                uint rgba8_color = Layout.HelperParseHex(node, "argb", 0xFFFFFFFF);
                Math2D.ColorBytesToFloats(rgba8_color, true, rgba);
                // move alpha location
                float a = rgba[0];
                float r = rgba[1];
                float g = rgba[2];
                float b = rgba[3];
                rgba[0] = r;
                rgba[1] = g;
                rgba[2] = b;
                rgba[3] = a;
            } else {
                rgba[0] = Layout.HelperParseFloat(node, "r", Single.NaN);
                rgba[1] = Layout.HelperParseFloat(node, "g", Single.NaN);
                rgba[2] = Layout.HelperParseFloat(node, "b", Single.NaN);
                rgba[3] = Layout.HelperParseFloat(node, "a", Single.NaN);
            }
        }

        private IVertex HelperGetVertex(PVRContextVertex type, string name) {
            for (int i = 0 ; i < this.vertex_list_size ; i++) {
                if (this.vertex_list[i].type == type && this.vertex_list[i].name == name) {
                    return this.vertex_list[i].vertex;
                }
            }
            return null;
        }

        private int HelperGetGroupIndex(string name) {
            if (name == Layout.GROUP_ROOT) {
                return 0;
            } else {
                int size = this.group_list_size;
                for (int i = 1 ; i < size ; i++) {
                    if (this.group_list[i].name == name) {
                        return i;
                    }
                }
            }
            return -1;
        }

        private void HelperExecuteAction(Item vertex, Action action) {
            switch (vertex.type) {
                case PVRContextVertex.SPRITE:
                    Layout.HelperExecuteActionInSprite(
                          action, vertex, this.viewport_width, this.viewport_height
                      );
                    break;
                case PVRContextVertex.TEXTSPRITE:
                    Layout.HelperExecuteActionInTextsprite(
                        action, vertex, this.viewport_width, this.viewport_height
                    );
                    break;
            }
        }

        private void HelperExecuteTrigger(Trigger trigger) {
            trigger.context.running = true;
            trigger.context.progress_delay = 0;
            trigger.context.loop_waiting = false;
            trigger.context.loop_count = 0;

            if (trigger.start_delay > 0) return;

            // there no start delay, commit now
            HelperCommitTrigger(trigger);
        }

        private static void HelperSetParallaxInfo(LayoutParallax parallax_info, ActionEntry parallax_action) {
            if (!Single.IsNaN(parallax_action.x)) parallax_info.x = parallax_action.x;
            if (!Single.IsNaN(parallax_action.y)) parallax_info.y = parallax_action.y;
            if (!Single.IsNaN(parallax_action.z)) parallax_info.z = parallax_action.z;
        }

        private static void HelperStackGroups(Group parent_group) {
            bool parent_visible = parent_group.context.visible && parent_group.context.alpha > 0;
            Group group = parent_group.context.next_child;

            while (group != null) {
                group.context.visible = parent_visible && group.visible && group.alpha > 0;
                group.context.parent_group = parent_group;

                if (group.context.visible) {
                    // interpolate the parent context in the current context
                    Math2D.ColorBlendNormal(
                        group.offsetcolor, parent_group.context.offsetcolor, group.context.offsetcolor
                    );

                    parent_group.context.matrix.CopyTo(group.context.matrix);
                    group.context.matrix.ApplyModifier(group.modifier);

                    group.context.alpha = group.alpha * parent_group.context.alpha;

                    if (group.antialiasing == PVRContextFlag.DEFAULT)
                        group.context.antialiasing = parent_group.context.antialiasing;
                    else
                        group.context.antialiasing = group.antialiasing;

                    group.context.parallax.x = group.parallax.x * parent_group.context.parallax.x;
                    group.context.parallax.y = group.parallax.y * parent_group.context.parallax.y;
                    group.context.parallax.z = group.parallax.z * parent_group.context.parallax.z;
                }

                if (group.context.next_child != null) Layout.HelperStackGroups(group);

                group = group.context.next_sibling;
            }
        }

        private static void HelperStackGroupsShaders(Group group, PVRContext pvrctx) {
            // if the parent has framebuffer, stop going up
            while (group != null) {
                if (group.psshader != null) {
                    if (!pvrctx.AddShader(group.psshader)) {
                        // limit reached
                        break;
                    }
                }
                group = group.context.parent_group;
            }
        }

        private static int HelperGroupAnimate(Group group, float elapsed) {
            if (group.animation == null) return 1;

            int completed = group.animation.Animate(elapsed);
            group.animation.UpdateUsingCallback(group, true);

            return completed;
        }

        private static void HelperGroupSetProperty(Group group, int property_id, float value) {
            switch (property_id) {
                case VertexProps.SPRITE_PROP_X:
                    group.modifier.x = value;
                    break;
                case VertexProps.SPRITE_PROP_Y:
                    group.modifier.y = value;
                    break;
                case VertexProps.SPRITE_PROP_WIDTH:
                    group.modifier.width = value;
                    break;
                case VertexProps.SPRITE_PROP_HEIGHT:
                    group.modifier.height = value;
                    break;
                case VertexProps.SPRITE_PROP_ALPHA:
                    group.alpha = Math2D.Clamp(value, 0.0f, 1.0f);
                    break;
                case VertexProps.SPRITE_PROP_VERTEX_COLOR_OFFSET_R:
                    group.offsetcolor[0] = value;
                    break;
                case VertexProps.SPRITE_PROP_VERTEX_COLOR_OFFSET_G:
                    group.offsetcolor[1] = value;
                    break;
                case VertexProps.SPRITE_PROP_VERTEX_COLOR_OFFSET_B:
                    group.offsetcolor[2] = value;
                    break;
                case VertexProps.SPRITE_PROP_VERTEX_COLOR_OFFSET_A:
                    group.offsetcolor[3] = value;
                    break;
                case VertexProps.SPRITE_PROP_ANTIALIASING:
                    group.antialiasing = (PVRContextFlag)(int)value;
                    break;
                default:
                    group.modifier.SetProperty(property_id, value);
                    break;
            }
        }

        private static void HelperParseProperty(XmlParserNode unparsed_entry, int property_id, string value_holder, ArrayList<ActionEntry> action_entries) {

            ActionEntry action_entry = new ActionEntry() {
                type = Layout.ACTION_PROPERTY,
                property = property_id,
                value = Single.NaN,
                misc = null,
            };

            action_entries.Add(action_entry);

            if (VertexProps.IsPropertyBoolean(property_id)) {
                string str = unparsed_entry.GetAttribute(value_holder);
                if (VertexProps.IsValueBoolean(str)) {
                    action_entry.value = VertexProps.ParseBoolean2(str, false) ? 1f : 0f;
                    return;
                }
            }
            switch (property_id) {
                case VertexProps.TEXTSPRITE_PROP_ALIGN_V:
                case VertexProps.TEXTSPRITE_PROP_ALIGN_H:
                case VertexProps.TEXTSPRITE_PROP_ALIGN_PARAGRAPH:
                    action_entry.value = (int)VertexProps.ParseAlign(unparsed_entry, value_holder, true, true);
                    break;
                case VertexProps.TEXTSPRITE_PROP_FORCE_CASE:
                    action_entry.value = VertexProps.ParseTextSpriteForceCase(unparsed_entry, value_holder, true);
                    break;
                case VertexProps.TEXTSPRITE_PROP_FONT_COLOR:
                    action_entry.value = Layout.HelperParseHex(unparsed_entry, value_holder, 0xFFFFFF);
                    break;
                case VertexProps.TEXTSPRITE_PROP_STRING:
                    action_entry.misc = unparsed_entry.GetAttribute(value_holder);
                    break;
                case VertexProps.MEDIA_PROP_PLAYBACK:
                    action_entry.value = VertexProps.ParsePlayback(unparsed_entry, value_holder, true);
                    break;
                case VertexProps.SPRITE_PROP_ANTIALIASING:
                    action_entry.value = (int)VertexProps.ParseFlag(unparsed_entry, value_holder, PVRContextFlag.ENABLE);
                    break;
                case VertexProps.FONT_PROP_WORDBREAK:
                    action_entry.value = VertexProps.ParseWordbreak(unparsed_entry, value_holder, true);
                    break;
                default:
                    action_entry.value = Layout.HelperParseFloat(unparsed_entry, value_holder, 0f);
                    break;
            }
        }

        private static void HerperParseOffsetmovefromto(XmlParserNode unparsed_offsetmovefromto, out float x, out float y, out float z) {
            x = Layout.HelperParseFloat(unparsed_offsetmovefromto, "x", Single.NaN);
            y = Layout.HelperParseFloat(unparsed_offsetmovefromto, "y", Single.NaN);
            z = Layout.HelperParseFloat(unparsed_offsetmovefromto, "z", Single.NaN);
        }

        private void HelperZbufferBuild() {
            int j = 0;
            for (int i = 0 ; i < this.z_buffer_size ; i++) {
                this.z_buffer[i] = new ZBufferEntry() { item = null, visible = false, z_index = 0 };
            }
            for (int i = 0 ; i < this.vertex_list_size ; i++) {
                this.z_buffer[j++].item = this.vertex_list[i];
            }
            for (int i = 0 ; i < this.external_vertex_list_size ; i++) {
                this.z_buffer[j++].item = this.external_vertex_list[i];
            }
        }






        private static void HelperAddGroupToParent(GroupContext parent_context, Group group) {
            if (parent_context.next_child != null) {
                Group sibling_group = parent_context.next_child;
                while (true) {
                    if (sibling_group.context.next_sibling == null) break;
                    sibling_group = sibling_group.context.next_sibling;
                }
                sibling_group.context.next_sibling = group;
            } else {
                parent_context.next_child = group;
            }
        }

        private int HelperCheckTriggerQueue(float elapsed) {
            int completed_count = 0;

            // complex timestamp based checker
            if (this.beatwatcher_synced_triggers) {
                bool do_sync = false;
                bool has_beat = this.beatwatcher.Poll();

                // check if the beatwatcher was resynchronized
                if (this.beatwatcher.resyncs > 0) {
                    // snapshot current beatwatcher timestamp
                    if (this.beatwatcher_resyncs_count != this.beatwatcher.resyncs) {
                        this.beatwatcher_resyncs_count = this.beatwatcher.resyncs;
                        this.beatwatcher_last_timestamp = this.beatwatcher.last_global_timestamp;
                        return 0;
                    }

                    // wait for a beat, this keep harmonic all beat-dependent animations (in theory)
                    if (!has_beat) return completed_count;

                    do_sync = true;
                    this.beatwatcher.resyncs = this.beatwatcher_resyncs_count = 0;
                }

                // calc time elasped since last HelperCheckTriggerQueue() call
                elapsed = (float)(this.beatwatcher.last_global_timestamp - this.beatwatcher_last_timestamp);
                this.beatwatcher_last_timestamp = this.beatwatcher.last_global_timestamp;

                if (do_sync) {
                    // resync all trigger progress and check later the queue
                    for (int i = 0 ; i < this.trigger_list_size ; i++) {
                        this.trigger_list[i].context.progress_delay = 0;
                    }
                }

                for (int i = 0 ; i < this.trigger_list_size ; i++) {
                    bool trigger_is_completed = false;
                    Trigger trigger = this.trigger_list[i];

                    if (!trigger.context.running) {
                        completed_count++;
                        continue;
                    }

                    bool commit = false;
                    trigger.context.progress_delay += elapsed;

                    while (true) {
                        float target_delay;
                        if (trigger.context.loop_waiting) {
                            target_delay = trigger.loop_delay;
                        } else {
                            target_delay = trigger.start_delay;
                        }

                        if (trigger.context.progress_delay >= target_delay) {
                            trigger.context.progress_delay -= target_delay;// keep in sync
                            commit = true;
                            trigger.context.loop_waiting = true;
                            trigger.context.loop_count++;
                        } else {
                            break;
                        }

                        if (trigger.loop < 1) {
                            trigger_is_completed = true;
                            break;
                        }
                        if (trigger.context.loop_count >= trigger.loop) {
                            trigger.context.running = false;
                            trigger_is_completed = true;
                            break;
                        }
                    }

                    if (trigger_is_completed) trigger.context.progress_delay = 0;

                    if (commit) {
                        HelperCommitTrigger(trigger);
                    }
                }

                return completed_count;
            }

            // classic way
            for (int i = 0 ; i < this.trigger_list_size ; i++) {
                Trigger trigger = this.trigger_list[i];

                if (!trigger.context.running) {
                    completed_count++;
                    continue;
                }

                float target_delay;

                if (trigger.context.loop_waiting) {
                    target_delay = trigger.loop_delay;
                } else {
                    target_delay = trigger.start_delay;
                }

                if (trigger.context.progress_delay >= target_delay) {
                    trigger.context.progress_delay -= target_delay;// keep in sync
                    HelperCommitTrigger(trigger);
                }

                trigger.context.progress_delay += elapsed;
            }

            return completed_count;
        }

        private void HelperCommitTrigger(Trigger trigger) {
            // increase the number of loops (or queue for the next loop) and check if still should run
            trigger.context.loop_count++;
            trigger.context.loop_waiting = true;
            trigger.context.running = trigger.loop < 1 || trigger.context.loop_count < trigger.loop;

            if (trigger.action_name != null && TriggerAction(null, trigger.action_name) < 1) {
                Console.Error.WriteLine("[WARN] layout_helper_commit_trigger() no actions with name: " + trigger.action_name);
            }
            if (trigger.camera_name != null && !TriggerCamera(trigger.camera_name)) {
                Console.Error.WriteLine("[WARN] layout_helper_commit_trigger() no camera with name: " + trigger.camera_name);
            }
            if (trigger.trigger_name == null) {
                return;
            }

            trigger.context.reject_recursive = true;// avoid self-trigger

            for (int i = 0 ; i < this.trigger_list_size ; i++) {
                Trigger trigger2 = this.trigger_list[i];
                if (trigger2.name != trigger.trigger_name) continue;

                if (!trigger2.context.reject_recursive) {
                    HelperExecuteTrigger(trigger2);
                    continue;
                }

                Console.Error.WriteLine("layout_helper_commit_trigger() self - trigger avoided: " + trigger.action_name);
            }

            trigger.context.reject_recursive = false;
            return;
        }

        private static void HelperSetShaderUniform(PSShader psshader, ActionEntry action_entry) {
            if (psshader == null) {
                Console.Error.WriteLine($"[WARN] layout_helper_set_shader_uniform() can not set {action_entry.uniform_name}, there no shader");
                return;
            }

            int ret = psshader.SetUniformAny(action_entry.uniform_name, (double[])action_entry.misc);

            switch (ret) {
                case 0:
                    Console.Error.WriteLine($"[WARN] layout_helper_set_shader_uniform() the shader does not have {action_entry.uniform_name}");
                    break;
                case -1:
                    Console.Error.WriteLine($"[WARN] layout_helper_set_shader_uniform() type of {action_entry.uniform_name} is not supported");
                    break;
                case -2:
                    Console.Error.WriteLine($"[ERROR] layout_helper_set_shader_uniform() bad setter for {action_entry.uniform_name}");
                    break;
            }
        }


        //////////////////////////////////
        ///        VERTEX PARSER       ///
        //////////////////////////////////

        private static void ParsePlaceholder(XmlParserNode unparsed_plchdlr, LayoutContext layout_context, int group_id) {
            string name = unparsed_plchdlr.GetAttribute("name");
            if (String.IsNullOrEmpty(name)) {
                Console.Error.WriteLine("[WARN] Missing placeholder name: " + unparsed_plchdlr.OuterHTML);
                return;
            }

            LayoutPlaceholder placeholder = new LayoutPlaceholder() {
                type = PVRContextVertex.DRAWABLE,
                group_id = group_id,

                name = name,

                align_vertical = Layout.HelperParseAlign2(unparsed_plchdlr, true, Align.NONE),
                align_horizontal = Layout.HelperParseAlign2(unparsed_plchdlr, false, Align.NONE),

                x = Layout.HelperParseFloat(unparsed_plchdlr, "x", 0f),
                y = Layout.HelperParseFloat(unparsed_plchdlr, "y", 0f),
                z = Layout.HelperParseFloat(unparsed_plchdlr, "z", 0f),
                height = Layout.HelperParseFloat(unparsed_plchdlr, "height", -1f),
                width = Layout.HelperParseFloat(unparsed_plchdlr, "width", -1f),

                parallax = new LayoutParallax() { x = 1.0f, y = 1.0f, z = 1.0f },
                static_camera = VertexProps.ParseBoolean(unparsed_plchdlr, "static", false),

                vertex = null
            };

            float x, y, z;

            XmlParserNode unparsed_parallax = unparsed_plchdlr.GetChildren("Parallax");
            if (unparsed_parallax != null) {
                Layout.HerperParseOffsetmovefromto(unparsed_parallax, out x, out y, out z);
                if (!Single.IsNaN(x)) placeholder.parallax.x = x;
                if (!Single.IsNaN(y)) placeholder.parallax.y = y;
                if (!Single.IsNaN(z)) placeholder.parallax.z = z;
            }

            XmlParserNode unparsed_location = unparsed_plchdlr.GetChildren("Location");
            if (unparsed_location != null) {
                Layout.HerperParseOffsetmovefromto(unparsed_location, out x, out y, out z);
                if (!Single.IsNaN(x)) placeholder.x = x;
                if (!Single.IsNaN(y)) placeholder.y = y;
                if (!Single.IsNaN(z)) placeholder.z = z;
                placeholder.align_vertical = Layout.HelperParseAlign2(
                    unparsed_location, true, placeholder.align_vertical
                );
                placeholder.align_horizontal = Layout.HelperParseAlign2(
                    unparsed_location, false, placeholder.align_horizontal
                );
            }

            XmlParserNode unparsed_size = unparsed_plchdlr.GetChildren("Size");
            if (unparsed_size != null) {
                placeholder.width = Layout.HelperParseFloat(unparsed_size, "width", placeholder.width);
                placeholder.height = Layout.HelperParseFloat(unparsed_size, "height", placeholder.height);
            }

            XmlParserNode unparsed_static = unparsed_plchdlr.GetChildren("Static");
            if (unparsed_static != null) {
                placeholder.static_camera = VertexProps.ParseBoolean(
                    unparsed_static, "enable", true
                );
            }

            Item item = new Item() {
                name = null,// STUB, the placeholder field contains the name
                actions_size = 0,
                type = PVRContextVertex.DRAWABLE,
                group_id = group_id,
                placeholder = placeholder,
                parallax = null,// STUB, the placeholder field contains the parallax
                initial_action_name = null
            };
            layout_context.vertex_list.Add(item);
        }

        private static void ParseSprite(XmlParserNode unparsed_sprite, LayoutContext layout_context, int group_id) {
            XmlParserNodeList actions = unparsed_sprite.GetChildrenList("Action");
            string texture_filename = unparsed_sprite.GetAttribute("texture");
            string atlas_filename = unparsed_sprite.GetAttribute("atlas");
            Sprite sprite;

            Atlas atlas;
            string atlas_texture_path = null;
            if (!String.IsNullOrEmpty(atlas_filename)) {
                atlas = (Atlas)Layout.HelperGetResource(layout_context.resource_pool, atlas_filename, false);
                if (atlas != null) atlas_texture_path = atlas.GetTexturePath();
            } else {
                atlas = null;
            }

            if (!String.IsNullOrEmpty(texture_filename) || !String.IsNullOrEmpty(atlas_texture_path)) {
                string src = texture_filename ?? atlas_texture_path;
                Texture texture = (Texture)Layout.HelperGetResource(layout_context.resource_pool, src, true);
                sprite = Sprite.Init(texture);
                if (atlas == null) sprite.SetDrawSizeFromSourceSize();
            } else {
                float[] rgba = new float[] { 0.0f, 0.0f, 0.0f, Single.NaN };
                Layout.HelperParseColor(unparsed_sprite, rgba);

                sprite = Sprite.InitFromRGB8(0xFFFFFF);

                sprite.SetVertexColor(rgba[0], rgba[1], rgba[2]);
                if (!Single.IsNaN(rgba[3])) sprite.SetAlpha(rgba[3]);
            }

            Item vertex = new Item() {
                type = PVRContextVertex.SPRITE,
                name = unparsed_sprite.GetAttribute("name"),
                vertex = sprite,
                group_id = group_id,
                actions_size = -1,
                actions = null,
                initial_action_name = unparsed_sprite.GetAttribute("initialAction"),
                parallax = new LayoutParallax() { x = 1.0f, y = 1.0f, z = 1.0f },
                static_camera = false,
                animation = null,
                placeholder = null
            };

            ArrayList<Action> actions_arraylist = new ArrayList<Action>(actions.Length);
            foreach (XmlParserNode action in actions) {
                ParseSpriteAction(action, layout_context.animlist, atlas, actions_arraylist);
            }
            actions_arraylist.Destroy2(out vertex.actions_size, ref vertex.actions);

            layout_context.vertex_list.Add(vertex);
        }

        private static void ParseText(XmlParserNode unparsed_text, LayoutContext layout_context, int group_id) {
            FontHolder fontholder = null;
            string font_name = unparsed_text.GetAttribute("fontName");
            float font_size = Layout.HelperParseFloat(unparsed_text, "fontSize", 12);
            float[] font_color = new float[] { 0.0f, 0.0f, 0.0f, Single.NaN };
            XmlParserNodeList actions = unparsed_text.GetChildrenList("Action");

            Layout.HelperParseColor(unparsed_text, font_color);

            for (int i = 0 ; i < layout_context.fonts_size ; i++) {
                if (layout_context.fonts[i].name == font_name) {
                    fontholder = layout_context.fonts[i].fontholder;
                    break;
                }
            }

            if (fontholder == null) {
                Console.Error.WriteLine("[ERROR] layout_parse_text() the font '" + font_name + "' is not attached");
                return;
            }

            Item vertex = new Item() {
                name = unparsed_text.GetAttribute("name"),
                type = PVRContextVertex.TEXTSPRITE,
                vertex = TextSprite.Init2(fontholder, font_size, 0x000000),
                actions_size = -1,
                actions = null,
                group_id = group_id,
                initial_action_name = unparsed_text.GetAttribute("initialAction"),
                parallax = new LayoutParallax() { x = 1.0f, y = 1.0f, z = 1.0f },
                static_camera = false,
                animation = null,
                placeholder = null
            };

            ((TextSprite)(vertex.vertex)).SetColor(font_color[0], font_color[1], font_color[2]);
            if (!Single.IsNaN(font_color[3])) ((TextSprite)(vertex.vertex)).SetAlpha(font_color[3]);

            ArrayList<Action> actions_arraylist = new ArrayList<Action>(actions.Length);
            foreach (XmlParserNode action in actions) {
                ParseTextAction(action, layout_context.animlist, actions_arraylist);
            }
            actions_arraylist.Destroy2(out vertex.actions_size, ref vertex.actions);

            layout_context.vertex_list.Add(vertex);
        }

        private static void ParseGroup(XmlParserNode unparsed_group, LayoutContext layout_context, GroupContext parent_context) {
            ArrayList<Action> actions_arraylist = new ArrayList<Action>(unparsed_group.Children.Length);

            Group group = new Group() {
                name = parent_context != null ? unparsed_group.GetAttribute("name") : Layout.GROUP_ROOT,
                group_id = layout_context.group_list.Size(),
                actions = null,
                actions_size = -1,
                initial_action_name = unparsed_group.GetAttribute("initialAction"),

                visible = VertexProps.ParseBoolean(unparsed_group, "visible", true),
                alpha = Layout.HelperParseFloat(unparsed_group, "alpha", 1.0f),
                antialiasing = VertexProps.ParseFlag(unparsed_group, "antialiasing", PVRContextFlag.DEFAULT),
                offsetcolor = new float[4],
                modifier = new Modifier(),
                parallax = new LayoutParallax() { x = 1.0f, y = 1.0f, z = 1.0f },
                static_camera = false,
                static_screen = null,

                animation = null,
                psshader = null,
                psframebuffer = null,

                blend_enabled = true,
                blend_src_rgb = Blend.DEFAULT,
                blend_dst_rgb = Blend.DEFAULT,
                blend_src_alpha = Blend.DEFAULT,
                blend_dst_alpha = Blend.DEFAULT,

                viewport_x = -1f,
                viewport_y = -1f,
                viewport_width = -1f,
                viewport_height = -1f,

                context = new GroupContext() {
                    visible = true,
                    alpha = 1.0f,
                    antialiasing = PVRContextFlag.DEFAULT,
                    matrix = new SH4Matrix(),
                    offsetcolor = new float[4],
                    parallax = new LayoutParallax() { x = 1.0f, y = 1.0f, z = 1.0f },

                    next_child = null,
                    next_sibling = null,
                    parent_group = null,
                    last_z_index = -1
                }
            };

            //sh4matrix_reset(group.matrix);
            group.modifier.Clear();
            PVRContext.HelperClearOffsetColor(group.offsetcolor);

            if (VertexProps.ParseBoolean(unparsed_group, "framebuffer", false)) {
                // assume layout as part of the main PVRContext renderer
                group.psframebuffer = new PSFramebuffer(PVRContext.global_context);
            }

            layout_context.group_list.Add(group);

            foreach (XmlParserNode item in unparsed_group.Children) {
                switch (item.TagName) {
                    case "Action":
                        /*if (parent_context == null) {
                            Console.Error.WriteLine(
                                "[WARN] layout_init(): layout_parse_group() action found in the layout root, " +
                                "will be imported as root group action."
                            );
                        }*/
                        Layout.ParseGroupAction(item, layout_context.animlist, actions_arraylist);
                        break;
                    case "Sprite":
                        Layout.ParseSprite(item, layout_context, group.group_id);
                        break;
                    case "Text":
                        Layout.ParseText(item, layout_context, group.group_id);
                        break;
                    case "Camera":
                        /*if (parent_context) {
                            Console.Error.WriteLine(
                                "[ERROR] layout_init(): layout_parse_group() groups can not contains cameras"
                            );
                        }*/
                        Layout.ParseCamera(item, layout_context);
                        break;
                    case "Group":
                        Layout.ParseGroup(item, layout_context, group.context);
                        break;
                    case "Placeholder":
                        Layout.ParsePlaceholder(item, layout_context, group.group_id);
                        break;
                    case "Sound":
                        Layout.ParseSound(item, layout_context);
                        break;
                    case "AttachValue":
                    case "Font":
                        break;
                    case "Trigger":
                        Layout.ParseTriggers(item, layout_context);
                        break;
                    default:
                        Console.Error.WriteLine("[WARN] layout_parse_group() unknown element: " + item.TagName);
                        break;
                }
            }

            actions_arraylist.Destroy2(out group.actions_size, ref group.actions);

            // add to group tree
            if (parent_context != null) {
                Layout.HelperAddGroupToParent(parent_context, group);
            }

        }

        private static void ParseFonts(XmlParserNode unparsed_root, LayoutContext layout_context) {
            XmlParserNodeList unparsed_fonts = unparsed_root.GetChildrenList("Font");
            ArrayList<Font> fonts_arraylist = new ArrayList<Font>(unparsed_fonts.Length);

            foreach (XmlParserNode item in unparsed_fonts) {
                string name = item.GetAttribute("name");
                string path = item.GetAttribute("path");
                bool glyph_animate = VertexProps.ParseBoolean(item, "glyphAnimate", true);
                string glyph_suffix = item.GetAttribute("glyphSuffix");
                bool glyph_color_by_difference = VertexProps.ParseBoolean(item, "colorByDifference", false);

                if (String.IsNullOrEmpty(name)) {
                    Console.Error.WriteLine("[ERROR] missing font name: " + item.OuterHTML);
                    continue;
                }
                if (String.IsNullOrEmpty(path)) {
                    Console.Error.WriteLine("[ERROR] missing font path: " + item.OuterHTML);
                    continue;
                }

                try {
                    IFontRender font;
                    bool is_atlas = Atlas.UtilsIsKnownExtension(path);

                    if (is_atlas) {
                        font = FontGlyph.Init(path, glyph_suffix, glyph_animate);
                        if (glyph_color_by_difference) font.EnableColorByDifference(true);
                    } else {
                        font = FontType.Init(path);
                    }

                    if (font == null) throw new Exception("missing or invalid font: " + path);
                    FontHolder fontholder = new FontHolder(font, is_atlas, -1);

                    fonts_arraylist.Add(new Font() { name = name, fontholder = fontholder });
                } catch (Exception e) {
                    Console.Error.WriteLine("[ERROR] Unable to read the font: {0}\n{1}", path, e.Message);
                    continue;
                } finally {
                    //free(path);
                }
            }

            fonts_arraylist.Destroy2(out layout_context.fonts_size, ref layout_context.fonts);
        }

        private static void ParseCamera(XmlParserNode unparsed_camera, LayoutContext layout_context) {
            float x, y, z;

            float duration_beats = Layout.HelperParseFloat(unparsed_camera, "durationInBeats", Single.NaN);
            float duration_milliseconds = Layout.HelperParseFloat(unparsed_camera, "duration", Single.NaN);

            bool duration_in_beats; float duration; bool has_duration;

            if (Single.IsNaN(duration_beats) && Single.IsNaN(duration_milliseconds)) {
                duration_in_beats = true;
                duration = 1;
                has_duration = false;
            } else {
                duration_in_beats = Single.IsNaN(duration_milliseconds);
                duration = duration_in_beats ? duration_beats : duration_milliseconds;
                has_duration = true;
            }

            CameraPlaceholder camera_placeholder = new CameraPlaceholder() {
                name = unparsed_camera.GetAttribute("name"),

                has_duration = has_duration,
                duration_in_beats = duration_in_beats,
                duration = duration,

                move_only = false,
                has_from = false,

                is_empty = false,

                has_parallax_offset_only = false,

                move_offset_only = false,
                has_offset_from = false,
                has_offset_to = false,

                to_offset_x = Single.NaN,
                to_offset_y = Single.NaN,
                to_offset_z = Single.NaN,

                from_offset_x = Single.NaN,
                from_offset_y = Single.NaN,
                from_offset_z = Single.NaN,

                //
                // Not implemented= requires a rework in the camera helper (camera.cs)
                //
                animation = null,

                from_x = Single.NaN,
                from_y = Single.NaN,
                from_z = Single.NaN,
                to_x = Single.NaN,
                to_y = Single.NaN,
                to_z = Single.NaN,
                offset_x = 0,
                offset_y = 0,
                offset_z = 1
            };

            //let anim_name = unparsed_camera.getAttribute("animationName");
            XmlParserNode unparsed_move = unparsed_camera.GetChildren("Move");
            XmlParserNode unparsed_from = unparsed_camera.GetChildren("From");
            XmlParserNode unparsed_to = unparsed_camera.GetChildren("To");

            XmlParserNode unparsed_offset = unparsed_camera.GetChildren("Offset");
            XmlParserNode unparsed_move_offset = unparsed_camera.GetChildren("OffsetMove");
            XmlParserNode unparsed_from_offset = unparsed_camera.GetChildren("FromOffset");
            XmlParserNode unparsed_to_offset = unparsed_camera.GetChildren("ToOffset");

            if (
                /*anim_name == null && */
                unparsed_move == null && unparsed_from == null && unparsed_to == null &&
                unparsed_offset == null &&
                unparsed_move_offset == null && unparsed_from_offset == null && unparsed_to_offset == null
            ) {
                // no animation or tween is defined
                camera_placeholder.is_empty = true;

                layout_context.camera_list.Add(camera_placeholder);
                return;
            }


            if (unparsed_move != null) {
                camera_placeholder.move_only = true;
                Layout.HerperParseOffsetmovefromto(unparsed_move, out x, out y, out z);
                camera_placeholder.from_x = camera_placeholder.to_x = x;
                camera_placeholder.from_y = camera_placeholder.to_y = y;
                camera_placeholder.from_z = camera_placeholder.to_z = z;
            } else {
                if (unparsed_from != null) {
                    Layout.HerperParseOffsetmovefromto(unparsed_from, out x, out y, out z);
                    camera_placeholder.from_x = x;
                    camera_placeholder.from_y = y;
                    camera_placeholder.from_z = z;
                    camera_placeholder.has_from = true;
                }
                if (unparsed_to != null) {
                    Layout.HerperParseOffsetmovefromto(unparsed_to, out x, out y, out z);
                    camera_placeholder.to_x = x;
                    camera_placeholder.to_y = y;
                    camera_placeholder.to_z = z;
                }
            }

            if (unparsed_offset != null) {
                Layout.HerperParseOffsetmovefromto(unparsed_offset, out x, out y, out z);
                camera_placeholder.offset_x = x;
                camera_placeholder.offset_y = y;
                camera_placeholder.offset_z = z;
                camera_placeholder.has_parallax_offset_only = true;
            } else if (unparsed_move_offset != null) {
                Layout.HerperParseOffsetmovefromto(unparsed_move_offset, out x, out y, out z);
                camera_placeholder.to_offset_x = x;
                camera_placeholder.to_offset_y = y;
                camera_placeholder.to_offset_z = z;
                camera_placeholder.move_offset_only = true;
            } else {
                if (unparsed_from_offset != null) {
                    Layout.HerperParseOffsetmovefromto(unparsed_from_offset, out x, out y, out z);
                    camera_placeholder.from_offset_x = x;
                    camera_placeholder.from_offset_y = y;
                    camera_placeholder.from_offset_z = z;
                    camera_placeholder.has_offset_from = true;
                }
                if (unparsed_to_offset != null) {
                    Layout.HerperParseOffsetmovefromto(unparsed_to_offset, out x, out y, out z);
                    camera_placeholder.to_offset_x = x;
                    camera_placeholder.to_offset_y = y;
                    camera_placeholder.to_offset_z = z;
                    camera_placeholder.has_offset_to = true;
                }
            }

            /*if (!String.IsNullOrEmpty(anim_name)) {
                camera_placeholder.animation = AnimSprite.InitFromAnimlist(
                    layout_context.animlist, anim_name
                );
            }*/

            layout_context.camera_list.Add(camera_placeholder);
        }

        private static void ParseExternalvalues(XmlParserNode unparsed_root, LayoutContext layout_context) {
            XmlParserNodeList list = unparsed_root.GetChildrenList("AttachValue");

            ArrayList<ExternalValue> values_arraylist = new ArrayList<ExternalValue>(list.Length);

            foreach (XmlParserNode item in list) {
                string name = item.GetAttribute("name");
                string unparsed_type = item.GetAttribute("type");
                string unparsed_value = item.GetAttribute("value");

                if (String.IsNullOrEmpty(name)) {
                    Console.Error.WriteLine("[ERROR] missing AttachValue name: " + item.OuterHTML);
                    continue;
                }
                if (String.IsNullOrEmpty(unparsed_type)) {
                    Console.Error.WriteLine("[ERROR] missing AttachValue type: " + item.OuterHTML);
                    continue;
                }

                dynamic value; AttachedValueType type; bool invalid;

                switch (unparsed_type.ToLowerInvariant()) {
                    case "string":
                        value = unparsed_value;
                        type = AttachedValueType.STRING;
                        invalid = false;
                        break;
                    case "float":
                        double val = VertexProps.ParseDouble2(unparsed_value, Double.NaN);
                        value = val;
                        type = AttachedValueType.FLOAT;
                        invalid = Double.IsNaN(val);
                        break;
                    case "integer":
                        value = VertexProps.ParseLongInteger2(unparsed_value, 0);
                        type = AttachedValueType.INTEGER;
                        invalid = !VertexProps.IsValueInteger(unparsed_value);
                        break;
                    case "hex":
                        uint hex;
                        invalid = !VertexProps.ParseHex(unparsed_value, out hex, false);
                        value = hex;
                        type = AttachedValueType.HEX;
                        break;
                    case "boolean":
                        value = VertexProps.ParseBoolean2(unparsed_value, false);
                        type = AttachedValueType.BOOLEAN;
                        invalid = !VertexProps.IsValueBoolean(unparsed_value);
                        break;
                    default:
                        Console.Error.WriteLine("[ERROR] unknown AttachValue type: " + item.OuterHTML);
                        continue;
                }

                if (invalid) {
                    Console.Error.WriteLine("[ERROR] layout_parse_externalvalues() value in : " + item.OuterHTML);
                    continue;
                }

                ExternalValue entry = new ExternalValue() { name = name, value = value, type = type };
                values_arraylist.Add(entry);
            }

            values_arraylist.Destroy2(out layout_context.values_size, ref layout_context.values);
        }

        private static void ParseTriggers(XmlParserNode unparsed_trigger, LayoutContext layout_context) {
            Trigger trigger = new Trigger() {
                name = unparsed_trigger.GetAttribute("name"),

                action_name = unparsed_trigger.GetAttribute("action"),
                camera_name = unparsed_trigger.GetAttribute("camera"),
                trigger_name = unparsed_trigger.GetAttribute("trigger"),

                loop = VertexProps.ParseInteger(unparsed_trigger, "loop", 1),// 1 means execute once

                loop_delay = 0f,
                loop_delay_beats = 0,
                loop_delay_beats_in_beats = false,

                start_delay = 0f,
                start_delay_beats = 0,
                start_delay_beats_in_beats = false,

                context = new TriggerContext() {
                    running = false,
                    reject_recursive = false,
                    progress_delay = 0.0,
                    loop_waiting = false,
                    loop_count = 0,
                }
            };

            if (unparsed_trigger.HasAttribute("loopDelayBeats")) {
                trigger.loop_delay_beats_in_beats = true;
                trigger.loop_delay_beats = Layout.HelperParseFloat(unparsed_trigger, "loopDelayBeats", 0f);
            } else if (unparsed_trigger.HasAttribute("loopDelay")) {
                trigger.loop_delay_beats_in_beats = false;
                trigger.loop_delay = Layout.HelperParseFloat(unparsed_trigger, "loopDelay", 0f);
            }

            if (unparsed_trigger.HasAttribute("startDelayInBeats")) {
                trigger.start_delay_beats_in_beats = true;
                trigger.start_delay_beats = Layout.HelperParseFloat(unparsed_trigger, "startDelayInBeats", 0f);
            } else if (unparsed_trigger.HasAttribute("startDelay")) {
                trigger.start_delay_beats_in_beats = false;
                trigger.start_delay = Layout.HelperParseFloat(unparsed_trigger, "startDelay", 0f);
            }

            layout_context.trigger_list.Add(trigger);
        }

        private static void ParseSound(XmlParserNode unparsed_sound, LayoutContext layout_context) {
            string src = unparsed_sound.GetAttribute("src");
            if (String.IsNullOrEmpty(src)) {
                Console.Error.WriteLine("[ERROR] layout_parse_sound() missing sound 'src'");
                return;
            }

            SoundPlayer soundplayer = SoundPlayer.Init(src);
            if (soundplayer == null) {
                Console.Error.WriteLine("[WARN] layout_parse_sound() can not load:" + src);
                return;
            }

            float volume = Layout.HelperParseFloat(unparsed_sound, "volume", 1.0f);
            bool looped = VertexProps.ParseBoolean(unparsed_sound, "looped", false);
            //float pan = Layout.HelperParseFloat(unparsed_sound, "pan", 0.0f);
            //bool muted = vertexprops.ParseBoolean(unparsed_sound, "muted", false);
            XmlParserNodeList actions = unparsed_sound.GetChildrenList("Action");
            ArrayList<Action> actions_arraylist = new ArrayList<Action>(actions.Length);

            Item sound = new Item() {
                name = unparsed_sound.GetAttribute("name"),
                initial_action_name = unparsed_sound.GetAttribute("initialAction"),
                soundplayer = SoundPlayer.Init(src),

                actions_size = 0,
                actions = null,

                was_playing = false
            };

            if (sound == null) return;

            sound.soundplayer.SetVolume(volume);
            sound.soundplayer.LoopEnable(looped);

            foreach (XmlParserNode action in actions) {
                ParseSoundAction(action, layout_context.animlist, actions_arraylist);
            }
            actions_arraylist.Destroy2(out sound.actions_size, ref sound.actions);

            layout_context.sound_list.Add(sound);
        }

        private static void ParseMacro(XmlParserNode unparsed_root, LayoutContext layout_context) {
            XmlParserNodeList list = unparsed_root.GetChildrenList("Macro");
            ArrayList<Macro> macro_arraylist = new ArrayList<Macro>(list.Length);

            foreach (XmlParserNode unparsed_macro in list) {
                if (!unparsed_macro.HasAttribute("name")) {
                    Console.Error.WriteLine("[WARN] layout_parse_macro() missing name in:" + unparsed_macro.OuterHTML);
                    continue;
                }

                string name = unparsed_macro.GetAttribute("name");
                string animation = unparsed_macro.GetAttribute("animation") ?? name;
                AnimSprite animsprite = AnimSprite.InitFromAnimlist(layout_context.animlist, animation);

                if (animsprite == null) {
                    Console.Error.WriteLine("[WARN] layout_parse_macro() missing animation: " + animation);
                    continue;
                }

                Macro macro = new Macro() {
                    name = name,
                    loop = VertexProps.ParseInteger(unparsed_macro, "loop", 0),
                    loop_by_beats = VertexProps.ParseBoolean(unparsed_macro, "loopByBeats", false),
                    actions = null,
                    actions_size = 0
                };

                ParseMacroActions(unparsed_macro.Children, macro);
                macro_arraylist.Add(macro);
            }

            macro_arraylist.Destroy2(out layout_context.macro_list_size, ref layout_context.macro_list);
        }


        //////////////////////////////////
        ///        ACTION PARSER       ///
        //////////////////////////////////

        private static void ParseSpriteAction(XmlParserNode unparsed_action, AnimList animlist, Atlas atlas, ArrayList<Action> action_entries) {
            ArrayList<ActionEntry> entries = new ArrayList<ActionEntry>(unparsed_action.Children.Length);

            foreach (XmlParserNode unparsed_entry in unparsed_action.Children) {
                switch (unparsed_entry.TagName) {
                    case "Location":
                        Layout.HelperAddActionLocation(unparsed_entry, entries);
                        break;
                    case "Size":
                        Layout.HelperAddActionSize(unparsed_entry, entries);
                        break;
                    case "OffsetColor":
                        Layout.HelperAddActionOffsetcolor(unparsed_entry, entries);
                        break;
                    case "Color":
                        Layout.HelperAddActionColor(unparsed_entry, entries);
                        break;
                    case "Property":
                        Layout.HelperAddActionProperty(unparsed_entry, false, entries);
                        break;
                    case "Properties":
                        Layout.HelperAddActionProperties(unparsed_entry, false, entries);
                        break;
                    case "AtlasApply":
                        Layout.HelperAddActionAtlasapply(unparsed_entry, atlas, entries);
                        break;
                    case "Resize":
                        Layout.HelperAddActionResize(unparsed_entry, entries);
                        break;
                    case "Animation":
                        Layout.HelperAddActionAnimation(unparsed_entry, animlist, entries);
                        break;
                    case "AnimationFromAtlas":
                        Layout.HelperAddActionAnimationfromatlas(unparsed_entry, atlas, entries);
                        break;
                    case "AnimationRemove":
                        Layout.HelperAddActionAnimationremove(unparsed_entry, entries);
                        break;
                    case "Parallax":
                        Layout.HelperAddActionParallax(unparsed_entry, entries);
                        break;
                    case "Modifier":
                        Layout.HelperAddActionModifier(unparsed_entry, entries);
                        break;
                    case "Static":
                        Layout.HelperAddActionStatic(unparsed_entry, entries);
                        break;
                    case "Hide":
                    case "Show":
                        Layout.HelperAddActionVisibility(unparsed_entry, entries);
                        break;
                    case "SetShader":
                        Layout.HelperAddActionSetshader(unparsed_entry, entries);
                        break;
                    case "RemoveShader":
                        Layout.HelperAddActionRemoveshader(unparsed_entry, entries);
                        break;
                    case "SetShaderUniform":
                        Layout.HelperAddActionSetshaderuniform(unparsed_entry, entries);
                        break;
                    case "SetBlending":
                        Layout.HelperAddActionSetblending(unparsed_entry, entries);
                        break;
                    default:
                        Console.Error.WriteLine("[WARN] Unknown action entry: " + unparsed_entry.TagName);
                        break;
                }
            }

            Action action = new Action() {
                name = unparsed_action.GetAttribute("name"),
                entries = null,
                entries_size = -1
            };

            entries.Destroy2(out action.entries_size, ref action.entries);
            action_entries.Add(action);
        }

        private static void ParseTextAction(XmlParserNode unparsed_action, AnimList animlist, ArrayList<Action> action_entries) {
            ArrayList<ActionEntry> entries = new ArrayList<ActionEntry>(unparsed_action.Children.Length);

            foreach (XmlParserNode unparsed_entry in unparsed_action.Children) {
                switch (unparsed_entry.TagName) {
                    case "Property":
                        Layout.HelperAddActionProperty(unparsed_entry, true, entries);
                        break;
                    case "Properties":
                        Layout.HelperAddActionProperties(unparsed_entry, true, entries);
                        break;
                    case "Location":
                        Layout.HelperAddActionLocation(unparsed_entry, entries);
                        break;
                    case "Color":
                        Layout.HelperAddActionColor(unparsed_entry, entries);
                        break;
                    case "MaxSize":
                        Layout.HelperAddActionSize(unparsed_entry, entries);
                        break;
                    case "String":
                        ActionEntry entry = new ActionEntry() {
                            type = Layout.ACTION_PROPERTY,
                            property = VertexProps.TEXTSPRITE_PROP_STRING,
                            misc = unparsed_entry.TextContent,
                        };
                        entries.Add(entry);
                        break;
                    case "Border":
                        Layout.HelperAddActionTextborder(unparsed_entry, entries);
                        break;
                    case "Animation":
                        Layout.HelperAddActionAnimation(unparsed_entry, animlist, entries);
                        break;
                    case "AnimationRemove":
                        Layout.HelperAddActionAnimationremove(unparsed_entry, entries);
                        break;
                    case "Parallax":
                        Layout.HelperAddActionParallax(unparsed_entry, entries);
                        break;
                    case "Modifier":
                        Layout.HelperAddActionModifier(unparsed_entry, entries);
                        break;
                    case "Static":
                        Layout.HelperAddActionStatic(unparsed_entry, entries);
                        break;
                    case "Hide":
                    case "Show":
                        Layout.HelperAddActionVisibility(unparsed_entry, entries);
                        break;
                    case "SetShader":
                        Layout.HelperAddActionSetshader(unparsed_entry, entries);
                        break;
                    case "RemoveShader":
                        Layout.HelperAddActionRemoveshader(unparsed_entry, entries);
                        break;
                    case "SetShaderUniform":
                        Layout.HelperAddActionSetshaderuniform(unparsed_entry, entries);
                        break;
                    case "SetBlending":
                        Layout.HelperAddActionSetblending(unparsed_entry, entries);
                        break;
                    default:
                        Console.Error.WriteLine("[WARN] Unknown Text action entry:" + unparsed_entry.TagName);
                        break;
                }
            }

            Action action = new Action() {
                name = unparsed_action.GetAttribute("name"),
                entries = null,
                entries_size = -1
            };

            entries.Destroy2(out action.entries_size, ref action.entries);
            action_entries.Add(action);
        }

        private static void ParseGroupAction(XmlParserNode unparsed_action, AnimList animlist, ArrayList<Action> action_entries) {
            ArrayList<ActionEntry> entries = new ArrayList<ActionEntry>(unparsed_action.Children.Length);

            foreach (XmlParserNode unparsed_entry in unparsed_action.Children) {
                switch (unparsed_entry.TagName) {
                    case "ResetMatrix":
                        Layout.HelperAddActionResetmatrix(unparsed_entry, entries);
                        break;
                    case "Modifier":
                        Layout.HelperAddActionModifier(unparsed_entry, entries);
                        break;
                    case "Property":
                        Layout.HelperAddActionProperty(unparsed_entry, false, entries);
                        break;
                    case "Properties":
                        Layout.HelperAddActionProperties(unparsed_entry, false, entries);
                        break;
                    case "OffsetColor":
                        Layout.HelperAddActionOffsetcolor(unparsed_entry, entries);
                        break;
                    case "Parallax":
                        HelperAddActionParallax(unparsed_entry, entries);
                        break;
                    case "Static":
                        Layout.HelperAddActionStatic(unparsed_entry, entries);
                        break;
                    case "Hide":
                    case "Show":
                        Layout.HelperAddActionVisibility(unparsed_entry, entries);
                        break;
                    case "Animation":
                        Layout.HelperAddActionAnimation(unparsed_entry, animlist, entries);
                        break;
                    case "AnimationRemove":
                        Layout.HelperAddActionAnimationremove(unparsed_entry, entries);
                        break;
                    case "SetShader":
                        Layout.HelperAddActionSetshader(unparsed_entry, entries);
                        break;
                    case "RemoveShader":
                        Layout.HelperAddActionRemoveshader(unparsed_entry, entries);
                        break;
                    case "SetShaderUniform":
                        Layout.HelperAddActionSetshaderuniform(unparsed_entry, entries);
                        break;
                    case "SetBlending":
                        Layout.HelperAddActionSetblending(unparsed_entry, entries);
                        break;
                    case "Viewport":
                        Layout.HelperAddActionViewport(unparsed_entry, entries);
                        break;
                }
            }

            Action action = new Action() {
                name = unparsed_action.GetAttribute("name"),
                entries = null,
                entries_size = -1
            };

            entries.Destroy2(out action.entries_size, ref action.entries);
            action_entries.Add(action);
        }

        private static void ParseSoundAction(XmlParserNode unparsed_action, AnimList animlist, ArrayList<Action> action_entries) {
            ArrayList<ActionEntry> entries = new ArrayList<ActionEntry>(unparsed_action.Children.Length);

            foreach (XmlParserNode unparsed_entry in unparsed_action.Children) {
                switch (unparsed_entry.TagName) {
                    case "Property":
                        Layout.HelperAddActionMediaproperty(unparsed_entry, entries);
                        break;
                    case "Properties":
                        Layout.HelperAddActionMediaproperties(unparsed_entry, entries);
                        break;
                    /*case "Animation":
                        Layout.HelperAddActionAnimation(unparsed_entry, animlist, entries);
                        break;
                    case "AnimationRemove":
                        Layout.HelperAddActionAnimationremove(unparsed_entry, entries);*/
                    default:
                        if (Layout.HelperAddActionMedia(unparsed_entry, entries))
                            Console.Error.WriteLine("[WARN] Unknown Sound action entry:" + unparsed_entry.TagName);
                        break;
                }
            }

            Action action = new Action() {
                name = unparsed_action.GetAttribute("name"),
                entries = null,
                entries_size = -1
            };

            entries.Destroy2(out action.entries_size, ref action.entries);
            action_entries.Add(action);
        }

        private static void ParseMacroActions(XmlParserNodeList unparsed_actions, Macro macro) {
            ArrayList<MacroAction> actions_arraylist = new ArrayList<MacroAction>(unparsed_actions.Length);

            foreach (XmlParserNode unparsed_action in unparsed_actions) {
                int type;
                string target_name = null;
                string action_name = null;
                string trigger_name = null;
                string camera_name = null;

                switch (unparsed_action.TagName) {
                    case "PuppetGroup":
                        type = Layout.ACTION_PUPPETGROUP;
                        target_name = unparsed_action.GetAttribute("target");
                        break;
                    case "Puppet":
                        type = Layout.ACTION_PUPPETITEM;
                        target_name = unparsed_action.GetAttribute("target");
                        break;
                    case "Execute":
                        type = Layout.ACTION_EXECUTE;
                        target_name = unparsed_action.GetAttribute("target");
                        action_name = unparsed_action.GetAttribute("action");
                        trigger_name = unparsed_action.GetAttribute("trigger");
                        camera_name = unparsed_action.GetAttribute("camera");
                        break;
                    default:
                        Console.Error.WriteLine("[WARN] Unknown Macro action:" + unparsed_action.TagName);
                        continue;
                }

                if (!unparsed_action.HasAttribute("id")) {
                    Console.Error.WriteLine("[WARN] Missing event id in Macro action:" + unparsed_action.OuterHTML);
                    //free(target_name);
                    //free(action_name);
                    //free(trigger_name);
                    //free(camera_name);
                    continue;
                }

                int id = VertexProps.ParseInteger(unparsed_action, "id", 0);
                MacroAction action = new MacroAction() {
                    type = type,
                    id = id,
                    target_name = target_name,
                    action_name = action_name,
                    trigger_name = trigger_name,
                    camera_name = camera_name
                };

                actions_arraylist.Add(action);
            }

            actions_arraylist.Destroy2(out macro.actions_size, ref macro.actions);
        }


        //////////////////////////////////
        ///      ACTION EXECUTERS      ///
        //////////////////////////////////

        private static void HelperExecuteActionInSprite(Action action, Item item, float viewport_width, float viewport_height) {
            float location_x, location_y;
            float draw_width, draw_height;

            Sprite sprite = (Sprite)item.vertex;

            for (int i = 0 ; i < action.entries_size ; i++) {
                ActionEntry entry = action.entries[i];

                switch (entry.type) {
                    case Layout.ACTION_LOCATION:

                        // Note: The sprite must have a draw size
                        sprite.GetDrawSize(out draw_width, out draw_height);

                        if (sprite.IsTextured()) {
                            Layout.HelperLocation(
                                  entry, draw_width, draw_height, viewport_width, viewport_height, out location_x, out location_y
                              );
                        } else {
                            sprite.GetDrawLocation(out location_x, out location_y);
                            if (!Single.IsNaN(entry.x)) location_x = entry.x;
                            if (!Single.IsNaN(entry.y)) location_y = entry.y;
                        }
                        sprite.SetDrawLocation(location_x, location_y);
                        if (!Single.IsNaN(entry.z)) sprite.SetZIndex(entry.z);
                        break;
                    case Layout.ACTION_SIZE:
                        if (entry.has_resize)
                            sprite.ResizeDrawSize(entry.width, entry.height, out _, out _);
                        else
                            sprite.SetDrawSize(entry.width, entry.height);
                        break;
                    case Layout.ACTION_OFFSETCOLOR:
                        sprite.SetOffsetColor(
                            entry.rgba[0], entry.rgba[1], entry.rgba[2], entry.rgba[3]
                        );
                        break;
                    case Layout.ACTION_COLOR:
                        sprite.SetVertexColor(entry.rgba[0], entry.rgba[1], entry.rgba[2]);
                        if (!Single.IsNaN(entry.rgba[3])) sprite.SetAlpha(entry.rgba[3]);
                        break;
                    case Layout.ACTION_PROPERTY:
                        sprite.SetProperty(entry.property, entry.value);
                        break;
                    case Layout.ACTION_ATLASAPPLY:
                        Atlas.ApplyFromEntry(sprite, (AtlasEntry)entry.misc, entry.override_size);
                        break;
                    case Layout.ACTION_RESIZE:
                        ImgUtils.CalcResizeSprite(
                            sprite, entry.max_width, entry.max_height, entry.cover, entry.center
                        );
                        break;
                    case Layout.ACTION_ANIMATION:
                        if (entry.misc == null && item.animation == null) break;
                        if (entry.misc != null) item.animation = (AnimSprite)entry.misc;
                        if (entry.restart) item.animation.Restart();
                        if (entry.stop_in_loop) item.animation.DisableLoop();

                        sprite.AnimationPlayByAnimsprite(item.animation, false);
                        sprite.Animate(0);
                        break;
                    case Layout.ACTION_ANIMATIONREMOVE:
                        sprite.AnimationPlayByAnimsprite(null, false);
                        break;
                    case Layout.ACTION_ANIMATIONEND:
                        sprite.AnimationEnd();
                        break;
                    case Layout.ACTION_VISIBILITY:
                        sprite.SetVisible(entry.visible);
                        break;
                    case Layout.ACTION_PARALLAX:
                        Layout.HelperSetParallaxInfo(item.parallax, entry);
                        break;
                    case Layout.ACTION_MODIFIER:
                        ((Modifier)entry.misc).CopyTo(sprite.MatrixGetModifier());
                        break;
                    case Layout.ACTION_STATIC:
                        item.static_camera = entry.enable;
                        break;
                    case Layout.ACTION_SETSHADER:
                        sprite.SetShader((PSShader)entry.misc);
                        break;
                    case Layout.ACTION_REMOVESHADER:
                        sprite.SetShader(null);
                        break;
                    case Layout.ACTION_SETSHADERUNIFORM:
                        PSShader psshader = sprite.GetShader();
                        Layout.HelperSetShaderUniform(psshader, entry);
                        break;
                    case LAYOUT_ACTION_SETBLENDING:
                        if (entry.has_enable) sprite.BlendEnable(entry.enable);
                        sprite.BlendSet(entry.blend_src_rgb, entry.blend_dst_rgb, entry.blend_src_alpha, entry.blend_dst_alpha);
                        break;
                }
            }
        }

        private static void HelperExecuteActionInTextsprite(Action action, Item item, float viewport_width, float viewport_height) {
            TextSprite textsprite = (TextSprite)item.vertex;
            for (int i = 0 ; i < action.entries_size ; i++) {
                ActionEntry entry = action.entries[i];
                switch (entry.type) {
                    case Layout.ACTION_TEXTBORDER:
                        if (entry.has_enable) textsprite.BorderEnable(entry.enable);
                        if (!Single.IsNaN(entry.size)) textsprite.BorderSetSize(entry.size);
                        textsprite.BorderSetColor(
                            entry.rgba[0], entry.rgba[1], entry.rgba[2], entry.rgba[3]
                        );
                        break;
                    case Layout.ACTION_PROPERTY:
                        if (entry.property == VertexProps.TEXTSPRITE_PROP_STRING)
                            textsprite.SetTextIntern(true, (string)entry.misc);
                        else
                            textsprite.SetProperty(entry.property, entry.value);
                        break;
                    case Layout.ACTION_LOCATION:
                        float location_x, location_y;
                        float draw_width, draw_height;

                        // NOTE: there must be text and size already set, otherwise
                        // this will not work !!!
                        textsprite.GetDrawSize(out draw_width, out draw_height);
                        Layout.HelperLocation(
                              entry, draw_width, draw_height, viewport_width, viewport_height, out location_x, out location_y
                          );
                        textsprite.SetDrawLocation(location_x, location_y);
                        if (!Single.IsNaN(entry.z)) textsprite.SetZIndex(entry.z);
                        break;
                    case Layout.ACTION_COLOR:
                        textsprite.SetColor(entry.rgba[0], entry.rgba[1], entry.rgba[2]);
                        if (!Single.IsNaN(entry.rgba[3])) textsprite.SetAlpha(entry.rgba[3]);
                        break;
                    case Layout.ACTION_SIZE:
                        textsprite.SetMaxDrawSize(entry.width, entry.height);
                        break;
                    case Layout.ACTION_ANIMATION:
                        if (entry.misc == null && item.animation == null) break;
                        if (entry.misc != null) item.animation = (AnimSprite)entry.misc;
                        if (entry.restart) item.animation.Restart();
                        if (entry.stop_in_loop) item.animation.DisableLoop();

                        textsprite.AnimationSet((AnimSprite)entry.misc);
                        textsprite.Animate(0);
                        break;
                    case Layout.ACTION_ANIMATIONREMOVE:
                        textsprite.AnimationSet(null);
                        break;
                    case Layout.ACTION_ANIMATIONEND:
                        textsprite.AnimationEnd();
                        break;
                    case Layout.ACTION_VISIBILITY:
                        textsprite.SetVisible(entry.visible);
                        break;
                    case Layout.ACTION_PARALLAX:
                        Layout.HelperSetParallaxInfo(item.parallax, entry);
                        break;
                    case Layout.ACTION_MODIFIER:
                        ((Modifier)entry.misc).CopyTo(textsprite.MatrixGetModifier());
                        break;
                    case Layout.ACTION_STATIC:
                        item.static_camera = entry.enable;
                        break;
                    case Layout.ACTION_SETSHADER:
                        textsprite.SetShader((PSShader)entry.misc);
                        break;
                    case Layout.ACTION_REMOVESHADER:
                        textsprite.SetShader(null);
                        break;
                    case Layout.ACTION_SETSHADERUNIFORM:
                        PSShader psshader = textsprite.GetShader();
                        Layout.HelperSetShaderUniform(psshader, entry);
                        break;
                    case LAYOUT_ACTION_SETBLENDING:
                        if (entry.has_enable) textsprite.BlendEnable(entry.enable);
                        textsprite.BlendSet(entry.blend_src_rgb, entry.blend_dst_rgb, entry.blend_src_alpha, entry.blend_dst_alpha);
                        break;
                }
            }
        }

        private static void HelperExecuteActionInGroup(Action action, Group group) {
            for (int i = 0 ; i < action.entries_size ; i++) {
                ActionEntry entry = action.entries[i];
                switch (entry.type) {
                    case Layout.ACTION_RESETMATRIX:
                        group.modifier.Clear();
                        //group.matrix.Reset();
                        break;
                    case Layout.ACTION_MODIFIER:
                        ((Modifier)entry.misc).CopyTo(group.modifier);
                        //group.matrix.ApplyModifier(entry.misc);
                        break;
                    case Layout.ACTION_PROPERTY:
                        Layout.HelperGroupSetProperty(group, entry.property, entry.value);
                        break;
                    case Layout.ACTION_OFFSETCOLOR:
                        group.offsetcolor[0] = entry.rgba[0];
                        group.offsetcolor[1] = entry.rgba[1];
                        group.offsetcolor[2] = entry.rgba[2];
                        group.offsetcolor[3] = entry.rgba[3];
                        break;
                    case Layout.ACTION_PARALLAX:
                        Layout.HelperSetParallaxInfo(group.parallax, entry);
                        break;
                    case Layout.ACTION_VISIBILITY:
                        group.visible = entry.visible;
                        break;
                    case Layout.ACTION_ANIMATION:
                        if (entry.misc == null && group.animation == null) break;
                        if (entry.misc != null) group.animation = (AnimSprite)entry.misc;
                        if (entry.restart) group.animation.Restart();
                        if (entry.stop_in_loop) group.animation.DisableLoop();

                        Layout.HelperGroupAnimate(group, 0);
                        break;
                    case Layout.ACTION_ANIMATIONREMOVE:
                        group.animation = null;
                        break;
                    case Layout.ACTION_ANIMATIONEND:
                        if (group.animation != null) {
                            group.animation.ForceEnd();
                            group.animation.UpdateUsingCallback(
                                group, true
                            );
                        }
                        break;
                    case Layout.ACTION_STATIC:
                        group.static_camera = entry.enable;
                        break;
                    case Layout.ACTION_SETSHADER:
                        group.psshader = (PSShader)entry.misc;
                        break;
                    case Layout.ACTION_REMOVESHADER:
                        group.psshader = null;
                        break;
                    case Layout.ACTION_SETSHADERUNIFORM:
                        Layout.HelperSetShaderUniform(group.psshader, entry);
                        break;
                    case LAYOUT_ACTION_SETBLENDING:
                        if (entry.has_enable) group.blend_enabled = entry.enable;
                        group.blend_src_rgb = entry.blend_src_rgb;
                        group.blend_dst_rgb = entry.blend_dst_rgb;
                        group.blend_src_alpha = entry.blend_src_alpha;
                        group.blend_dst_alpha = entry.blend_dst_alpha;
                        break;
                    case LAYOUT_ACTION_VIEWPORT:
                        if (!Single.IsNaN(entry.x)) group.viewport_x = entry.x;
                        if (!Single.IsNaN(entry.y)) group.viewport_y = entry.y;
                        if (!Single.IsNaN(entry.width)) group.viewport_width = entry.width;
                        if (!Single.IsNaN(entry.height)) group.viewport_height = entry.height;
                        break;
                }
            }
        }

        private static void HelperExecuteActionInSound(Action action, Item item) {
            SoundPlayer soundplayer = item.soundplayer;
            for (int i = 0 ; i < action.entries_size ; i++) {
                ActionEntry entry = action.entries[i];
                switch (entry.type) {
                    case Layout.ACTION_PROPERTY:
                        soundplayer.SetProperty(entry.property, entry.value);
                        break;
                        /*case Layout.ACTION_ANIMATION:
                            if (entry.misc == null && item.animation == null) break;
                            if (entry.misc != null) item.animation = (AnimSprite)entry.misc;
                            if (entry.restart) item.animation.Restart();
                            if (entry.stop_in_loop) item.animation.DisableLoop();

                            soundplayer.AnimationSet( entry.misc);
                            soundplayer.Animate( 0);
                            break;
                        case Layout.ACTION_ANIMATIONREMOVE:
                            soundplayer.AnimationSet( null);
                            break;
                        case Layout.ACTION_ANIMATIONEND:
                            soundplayer.AnimationEnd();
                            break;*/
                }
            }
        }




        //////////////////////////////////
        ///    ACTION ENTRY PARSERS    ///
        //////////////////////////////////

        private static void HelperAddActionProperty(XmlParserNode unparsed_entry, bool is_textsprite, ArrayList<ActionEntry> action_entries) {
            int property_id = VertexProps.ParseSpriteProperty(unparsed_entry, "name", !is_textsprite);
            if (property_id == -1 && is_textsprite) {
                property_id = VertexProps.ParseTextSpriteProperty(unparsed_entry, "name", true);
            } else if (property_id == -1) {
                property_id = VertexProps.ParseLayoutProperty(unparsed_entry, "name", true);
            }

            if (property_id < 0) return;

            Layout.HelperParseProperty(unparsed_entry, property_id, "value", action_entries);
        }

        private static void HelperAddActionProperties(XmlParserNode unparsed_entry, bool is_textsprite, ArrayList<ActionEntry> action_entries) {
            if (unparsed_entry.Attributes.Length < 1) {
                Console.Error.WriteLine(
                    "[WARN] layout_helper_add_action_properties() 'Properties' was empty" + unparsed_entry.OuterHTML
                );
                return;
            }

            foreach (XmlParserAttribute attribute in unparsed_entry.Attributes) {
                string name = attribute.name;
                int property_id;

                property_id = VertexProps.ParseSpriteProperty2(name);
                if (property_id == -1 && is_textsprite) {
                    property_id = VertexProps.ParseTextSpriteProperty2(name);
                } else if (property_id == -1) {
                    property_id = VertexProps.ParseLayoutProperty2(name);
                }

                if (property_id < 0) {
                    Console.Error.WriteLine(
                        "layout_helper_add_action_properties() unknown property '" +
                        name + "' in: " + unparsed_entry.OuterHTML
                    );
                    continue;
                }

                Layout.HelperParseProperty(unparsed_entry, property_id, name, action_entries);
            }

        }

        private static void HelperAddActionOffsetcolor(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            ActionEntry entry = new ActionEntry() {
                type = Layout.ACTION_OFFSETCOLOR,
                rgba = new float[] { Single.NaN, Single.NaN, Single.NaN, Single.NaN }
            };
            Layout.HelperParseColor(unparsed_entry, entry.rgba);
            action_entries.Add(entry);
        }

        private static ActionEntry HelperAddActionColor(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            ActionEntry entry = new ActionEntry() {
                type = Layout.ACTION_COLOR,
                rgba = new float[] { Single.NaN, Single.NaN, Single.NaN, Single.NaN }
            };
            Layout.HelperParseColor(unparsed_entry, entry.rgba);
            action_entries.Add(entry);
            return entry;
        }

        private static void HelperAddActionModifier(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            Modifier modifier_mask = new Modifier();
            ActionEntry action_entry = new ActionEntry() { type = Layout.ACTION_MODIFIER, misc = modifier_mask };

            modifier_mask.Clear();
            modifier_mask.Invalidate();

            for (int i = 0 ; i < unparsed_entry.Attributes.Length ; i++) {
                string attribute = unparsed_entry.Attributes[i].name;
                string name = attribute.ToLowerInvariant();
                bool value_bool = false;
                float value_float = Single.NaN;

                switch (name) {
                    case "rotatepivotenable":
                    case "scalesize":
                    case "scaletranslation":
                    case "translaterotation":
                        value_bool = VertexProps.ParseBoolean(unparsed_entry, attribute, false);
                        break;
                    default:
                        float val = Layout.HelperParseFloat(unparsed_entry, attribute, Single.NaN);
                        if (Single.IsNaN(val)) continue;
                        value_float = val;
                        break;
                }

                switch (name) {
                    case "x":
                        modifier_mask.x = value_float;
                        break;
                    case "y":
                        modifier_mask.y = value_float;
                        break;
                    case "translate":
                        modifier_mask.translate_x = modifier_mask.translate_y = value_float;
                        break;
                    case "translatex":
                        modifier_mask.translate_x = value_float;
                        break;
                    case "translatey":
                        modifier_mask.translate_y = value_float;
                        break;
                    case "scale":
                        modifier_mask.scale_x = modifier_mask.scale_y = value_float;
                        break;
                    case "scalex":
                        modifier_mask.scale_x = value_float;
                        break;
                    case "scaley":
                        modifier_mask.scale_y = value_float;
                        break;
                    case "skew":
                        modifier_mask.skew_x = modifier_mask.skew_y = value_float;
                        break;
                    case "skewx":
                        modifier_mask.skew_x = value_float;
                        break;
                    case "skewy":
                        modifier_mask.skew_y = value_float;
                        break;
                    case "rotate":
                        modifier_mask.rotate = value_float * Math2D.DEG_TO_RAD;
                        break;
                    case "scaledirection":
                        modifier_mask.scale_direction_x = modifier_mask.scale_direction_y = value_float;
                        break;
                    case "scaledirectionx":
                        modifier_mask.scale_direction_x = value_float;
                        break;
                    case "scaledirectiony":
                        modifier_mask.scale_direction_y = value_float;
                        break;
                    case "rotatepivot":
                        modifier_mask.rotate_pivot_u = modifier_mask.rotate_pivot_v = value_float;
                        break;
                    case "rotatepivotu":
                        modifier_mask.rotate_pivot_u = value_float;
                        break;
                    case "rotatepivotv":
                        modifier_mask.rotate_pivot_v = value_float;
                        break;
                }
            }

            action_entries.Add(action_entry);
        }

        private static void HelperAddActionParallax(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            ActionEntry entry = new ActionEntry() {
                type = Layout.ACTION_PARALLAX,
                x = Single.NaN,
                y = Single.NaN,
                z = Single.NaN
            };

            for (int i = 0 ; i < unparsed_entry.Attributes.Length ; i++) {
                string name = unparsed_entry.Attributes[i].name;
                float value = Layout.HelperParseFloat(unparsed_entry, name, Single.NaN);

                if (Single.IsNaN(value)) continue;

                switch (name) {
                    case "x":
                        entry.x = value;
                        break;
                    case "y":
                        entry.y = value;
                        break;
                    case "z":
                        entry.z = value;
                        break;
                }
            }

            action_entries.Add(entry);
        }

        private static void HelperAddActionLocation(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            ActionEntry entry = new ActionEntry() {
                type = Layout.ACTION_LOCATION,
                align_vertical = Layout.HelperParseAlign(unparsed_entry, true),
                align_horizontal = Layout.HelperParseAlign(unparsed_entry, false),
                x = Layout.HelperParseFloat(unparsed_entry, "x", 0f),
                y = Layout.HelperParseFloat(unparsed_entry, "y", 0f),
                z = Layout.HelperParseFloat(unparsed_entry, "z", Single.NaN),
            };
            action_entries.Add(entry);
        }

        private static void HelperAddActionSize(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            float width = Layout.HelperParseFloat(unparsed_entry, "width", Single.NaN);
            float height = Layout.HelperParseFloat(unparsed_entry, "height", Single.NaN);
            if (Single.IsNaN(width) && Single.IsNaN(height)) {
                Console.Error.WriteLine("[ERROR] layout_helper_add_action_size() invalid size: " + unparsed_entry.OuterHTML);
                return;
            }

            bool has_resize = width < 0f || height < 0f;
            if (has_resize && (Single.IsNaN(width) || Single.IsNaN(height))) {
                Console.Error.WriteLine("[WARN] layout_helper_add_action_size() invalid resize: " + unparsed_entry.OuterHTML);
                return;
            }

            ActionEntry entry = new ActionEntry() { type = Layout.ACTION_SIZE, width = width, height = height, has_resize = has_resize };
            action_entries.Add(entry);
        }

        private static void HelperAddActionVisibility(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            ActionEntry entry = new ActionEntry() { type = Layout.ACTION_VISIBILITY, visible = unparsed_entry.TagName != "Hide" };
            action_entries.Add(entry);
        }

        private static void HelperAddActionAtlasapply(XmlParserNode unparsed_entry, Atlas atlas, ArrayList<ActionEntry> action_entries) {
            if (atlas == null) {
                Console.Error.WriteLine(
                    "[ERROR] layout_helper_add_action_atlasapply() missing atlas, can not import: " +
                    unparsed_entry.OuterHTML
                );
                return;
            }

            string atlas_entry_name = unparsed_entry.GetAttribute("entry");
            AtlasEntry atlas_entry = atlas.GetEntryCopy(atlas_entry_name);

            if (atlas_entry == null) {
                Console.Error.WriteLine("missing atlas entry name '" + atlas_entry_name + "': " + unparsed_entry.OuterHTML);
                return;
            }

            ActionEntry entry = new ActionEntry() {
                type = Layout.ACTION_ATLASAPPLY,
                misc = atlas_entry,
                override_size = VertexProps.ParseBoolean(unparsed_entry, "overrideSize", false)
            };

            action_entries.Add(entry);
        }

        private static void HelperAddActionAnimation(XmlParserNode unparsed_entry, AnimList animlist, ArrayList<ActionEntry> action_entries) {
            if (animlist == null) {
                Console.Error.WriteLine("[ERROR] layout_helper_add_action_animation() failed, missing animlist");
                return;
            }

            string anim_name = unparsed_entry.GetAttribute("name");
            AnimSprite animsprite = AnimSprite.InitFromAnimlist(animlist, anim_name);

            if (animsprite == null) {
                Console.Error.WriteLine("Missing animation '" + anim_name + "': " + unparsed_entry.OuterHTML);
                return;
            }

            ActionEntry entry = new ActionEntry() {
                type = Layout.ACTION_ANIMATION,
                misc = animsprite,
                restart = VertexProps.ParseBoolean(unparsed_entry, "restart", true),
                stop_in_loop = VertexProps.ParseBoolean(unparsed_entry, "stopOnLoop", false)
            };

            action_entries.Add(entry);
        }

        private static void HelperAddActionAnimationfromatlas(XmlParserNode unparsed_entry, Atlas atlas, ArrayList<ActionEntry> action_entries) {
            if (atlas == null) {
                Console.Error.WriteLine(
                    "[ERROR] layout_helper_add_action_animationfromatlas() failed, sprite has no atlas. " + unparsed_entry.OuterHTML
                );
                return;
            }

            string anim_name = unparsed_entry.GetAttribute("name");
            if (anim_name == null) {
                Console.Error.WriteLine("[ERROR] Missing animation name on: " + unparsed_entry.OuterHTML);
                return;
            }

            int loop = VertexProps.ParseInteger(unparsed_entry, "loop", 1);
            bool has_number_suffix = VertexProps.ParseBoolean(unparsed_entry, "hasNumberSuffix", true);

            AnimSprite animsprite = AnimSprite.InitFromAtlas(Funkin.DEFAULT_ANIMATIONS_FRAMERATE, loop, atlas, anim_name, has_number_suffix);

            if (animsprite == null) {
                Console.Error.WriteLine("[WARN] Missing animation '" + anim_name + "': " + unparsed_entry.OuterHTML);
                return;
            }

            ActionEntry entry = new ActionEntry() {
                type = Layout.ACTION_ANIMATION,
                misc = animsprite,
                restart = VertexProps.ParseBoolean(unparsed_entry, "restart", true),
                stop_in_loop = VertexProps.ParseBoolean(unparsed_entry, "stopOnLoop", false)
            };

            action_entries.Add(entry);
        }

        private static void HelperAddActionAnimationremove(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            ActionEntry entry = new ActionEntry() { type = Layout.ACTION_ANIMATIONREMOVE };
            action_entries.Add(entry);
        }

        private static void HelperAddActionAnimationend(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            ActionEntry entry = new ActionEntry() { type = Layout.ACTION_ANIMATIONEND };
            action_entries.Add(entry);
        }

        private static void HelperAddActionTextborder(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            ActionEntry entry = new ActionEntry() {
                type = Layout.ACTION_TEXTBORDER,
                size = Layout.HelperParseFloat(unparsed_entry, "size", Single.NaN),
                has_enable = unparsed_entry.HasAttribute("enable"),
                enable = VertexProps.ParseBoolean(unparsed_entry, "enable", false),
                rgba = new float[] { Single.NaN, Single.NaN, Single.NaN, Single.NaN }
            };

            float alpha = Layout.HelperParseFloat(unparsed_entry, "alpha", Single.NaN);
            Layout.HelperParseColor(unparsed_entry, entry.rgba);

            if (!Single.IsNaN(alpha)) entry.rgba[3] = alpha;

            action_entries.Add(entry);
        }

        private static void HelperAddActionResize(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            float max_width = Layout.HelperParseFloat(unparsed_entry, "maxWidth", Single.NaN);
            float max_height = Layout.HelperParseFloat(unparsed_entry, "maxHeight", Single.NaN);

            if (Single.IsNaN(max_width) && Single.IsNaN(max_height)) {
                Console.Error.WriteLine("[ERROR] layout_helper_add_action_resize() invalid resize: " + unparsed_entry.OuterHTML);
                return;
            }

            if (Single.IsNaN(max_width)) max_width = -1f;
            if (Single.IsNaN(max_height)) max_height = -1f;

            ActionEntry entry = new ActionEntry() {
                type = Layout.ACTION_RESIZE,
                max_width = max_width,
                max_height = max_height,
                cover = VertexProps.ParseBoolean(unparsed_entry, "cover", false),
                center = VertexProps.ParseBoolean(unparsed_entry, "center", false)
            };

            action_entries.Add(entry);
        }

        private static void HelperAddActionResetmatrix(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            ActionEntry entry = new ActionEntry() { type = Layout.ACTION_RESETMATRIX };
            action_entries.Add(entry);
        }

        private static void HelperAddActionStatic(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            ActionEntry entry = new ActionEntry() {
                type = Layout.ACTION_STATIC,
                enable = VertexProps.ParseBoolean(unparsed_entry, "enable", true)
            };
            action_entries.Add(entry);
        }

        private static bool HelperAddActionMedia(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            int value;

            switch (unparsed_entry.TagName) {
                case "Play":
                    value = VertexProps.PLAYBACK_PLAY;
                    break;
                case "Pause":
                    value = VertexProps.PLAYBACK_PAUSE;
                    break;
                case "Stop":
                    value = VertexProps.PLAYBACK_STOP;
                    break;
                case "Mute":
                    value = VertexProps.PLAYBACK_MUTE;
                    break;
                case "Unmute":
                    value = VertexProps.PLAYBACK_UNMUTE;
                    break;
                default:
                    return true;
            }

            ActionEntry entry = new ActionEntry() {
                type = Layout.ACTION_PROPERTY,
                property = VertexProps.MEDIA_PROP_PLAYBACK,
                value = value,
                misc = null,
            };

            action_entries.Add(entry);
            return false;
        }

        private static void HelperAddActionMediaproperty(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            int property_id = VertexProps.ParseMediaProperty(unparsed_entry, "name", false);
            if (property_id < 0) return;
            Layout.HelperParseProperty(unparsed_entry, property_id, "value", action_entries);
        }

        private static void HelperAddActionMediaproperties(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            if (unparsed_entry.Attributes.Length < 1) {
                Console.Error.WriteLine(
                    "[WARN] layout_helper_add_action_properties() 'Properties' was empty" + unparsed_entry.OuterHTML
                );
                return;
            }

            foreach (XmlParserAttribute attribute in unparsed_entry.Attributes) {
                string name = attribute.name;
                int property_id;

                property_id = VertexProps.ParseMediaProperty2(name);

                if (property_id < 0) {
                    Console.Error.WriteLine(
                        "[WARN] layout_helper_add_action_mediaproperties() unknown property '" +
                        name + "' in: " + unparsed_entry.OuterHTML
                    );
                    continue;
                }

                Layout.HelperParseProperty(unparsed_entry, property_id, name, action_entries);
            }

        }


        private static void HelperAddActionSetshader(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            string shader_vertex_src = unparsed_entry.GetAttribute("vertexSrc");
            string shader_fragment_src = unparsed_entry.GetAttribute("fragmentSrc");
            XmlParserNodeList shader_sources = unparsed_entry.Children;

            if (String.IsNullOrEmpty(shader_fragment_src) && String.IsNullOrEmpty(shader_vertex_src) && shader_sources.Length < 1) {
                Layout.HelperAddActionRemoveshader(unparsed_entry, action_entries);
                return;
            }


            StringBuilder sourcecode_vertex = new StringBuilder();
            StringBuilder sourcecode_fragment = new StringBuilder();

            if (!String.IsNullOrEmpty(shader_vertex_src)) {
                string tmp = FS.ReadText(shader_vertex_src);
                if (!String.IsNullOrEmpty(tmp)) sourcecode_vertex.AddKDY(tmp);
                //free(tmp);
            }

            if (!String.IsNullOrEmpty(shader_fragment_src)) {
                string tmp = FS.ReadText(shader_fragment_src);
                if (!String.IsNullOrEmpty(tmp)) sourcecode_fragment.AddKDY(tmp);
                //free(tmp);
            }

            // parse source elements
            foreach (XmlParserNode source in shader_sources) {
                StringBuilder target;
                switch (source.TagName) {
                    case "VertexSource":
                        target = sourcecode_vertex;
                        break;
                    case "FragmentSource":
                        target = sourcecode_fragment;
                        break;
                    default:
                        Console.Error.WriteLine($"[WARN] layout_helper_add_action_setshader() unknown element: {source.OuterHTML}");
                        continue;
                }

                target.AddCharCodepointKDY(0x0A);// newline char
                target.AddKDY(source.TextContent);
            }

            string str_vertex = sourcecode_vertex.ToString();
            string str_fragment = sourcecode_fragment.ToString();

            if (String.IsNullOrEmpty(str_vertex) && String.IsNullOrEmpty(str_fragment)) {
                //free(str_vertex);
                //free(str_fragment);
                Console.Error.WriteLine($"[WARN] layout_helper_add_action_setshader() empty shader: {unparsed_entry.OuterHTML}");
                return;
            }

            // assume layout as part of the main PVRContext renderer
            PSShader psshader = PSShader.BuildFromSource(PVRContext.global_context, str_vertex, str_fragment);
            //free(str_vertex);
            //free(str_fragment);

            if (psshader == null) {
                Console.Error.WriteLine($"[WARN] layout_helper_add_action_setshader() compilation failed: {unparsed_entry.OuterHTML}");
                return;
            }

            ActionEntry entry = new ActionEntry() { type = Layout.ACTION_SETSHADER, misc = psshader };
            action_entries.Add(entry);
        }

        private static void HelperAddActionRemoveshader(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            ActionEntry entry = new ActionEntry() { type = Layout.ACTION_REMOVESHADER };
            action_entries.Add(entry);
        }

        private static void HelperAddActionSetshaderuniform(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {

            double[] values = new double[16];
            for (int i = 0 ; i < 16 ; i++) values[i] = 0.0;

            string name = unparsed_entry.GetAttribute("name");
            double value = VertexProps.ParseDouble(unparsed_entry, "value", Double.NaN);
            string unparsed_values = unparsed_entry.GetAttribute("values");

            if (String.IsNullOrEmpty(name)) {
                Console.Error.WriteLine("[ERROR] layout_helper_add_action_setshaderuniform() missing name: " + unparsed_entry.OuterHTML);
                return;
            }

            if (unparsed_values != null) {
                // separator: white-space, hard-space, tabulation, carrier-return, new-line
                Tokenizer tokenizer = Tokenizer.Init("\x20\xA0\t\r\n", true, false, unparsed_values);
                int index = 0;
                String str;

                while ((str = tokenizer.ReadNext()) != null) {
                    double temp_value = VertexProps.ParseDouble2(str, Double.NaN);
                    if (Double.IsNaN(temp_value)) {
                        Console.Error.WriteLine("[WARN] layout_helper_add_action_setshaderuniform() invalid value: " + str);
                        temp_value = 0.0;
                    }

                    //free(str);
                    values[index++] = temp_value;
                    if (index >= 16) break;
                }
                tokenizer.Destroy();
            } else if (!Double.IsNaN(value)) {
                values[0] = value;
            }


            ActionEntry entry = new ActionEntry() { type = Layout.ACTION_SETSHADERUNIFORM, uniform_name = name, misc = values };
            action_entries.Add(entry);
        }

        private static void HelperAddActionSetblending(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            bool has_enabled = unparsed_entry.HasAttribute("enabled");
            bool enabled = VertexProps.ParseBoolean(unparsed_entry, "enabled", true);

            Blend blend_src_rgb = VertexProps.ParseBlending(unparsed_entry.GetAttribute("srcRGB"));
            Blend blend_dst_rgb = VertexProps.ParseBlending(unparsed_entry.GetAttribute("dstRGB"));
            Blend blend_src_alpha = VertexProps.ParseBlending(unparsed_entry.GetAttribute("srcAlpha"));
            Blend blend_dst_alpha = VertexProps.ParseBlending(unparsed_entry.GetAttribute("dstAlpha"));

            ActionEntry entry = new ActionEntry() {
                type = LAYOUT_ACTION_SETBLENDING,
                enabled = has_enabled,
                blend_src_rgb = blend_src_rgb,
                blend_dst_rgb = blend_dst_rgb,
                blend_src_alpha = blend_src_alpha,
                blend_dst_alpha = blend_dst_alpha
            };

            action_entries.Add(entry);
        }

        private static void HelperAddActionViewport(XmlParserNode unparsed_entry, ArrayList<ActionEntry> action_entries) {
            float x = VertexProps.ParseFloat(unparsed_entry, "x", Single.NaN);
            float y = VertexProps.ParseFloat(unparsed_entry, "y", Single.NaN);
            float width = VertexProps.ParseFloat(unparsed_entry, "width", Single.NaN);
            float height = VertexProps.ParseFloat(unparsed_entry, "height", Single.NaN);

            ActionEntry entry = new ActionEntry() {
                type = LAYOUT_ACTION_VIEWPORT,
                x = x,
                y = y,
                width = width,
                height = height
            };

            action_entries.Add(entry);
        }


        private class ZBufferEntry {
            public Item item; public float z_index; public bool visible;
        }
        private class ResourcePoolEntry {
            public object data; public string src; public bool is_texture;
        }
        private class ResourcePool {
            public ArrayList<ResourcePoolEntry> textures; public ArrayList<ResourcePoolEntry> atlas;
        }
        private class Font {
            public FontHolder fontholder; public string name;
        }
        private class ExternalValue {
            public string name; public AttachedValueType type; public object value;
        }
        private class TriggerContext {
            public bool running;
            public bool reject_recursive;
            public double progress_delay;
            public bool loop_waiting;
            public int loop_count;
        }
        private class Trigger {
            public string name;

            public string action_name;
            public string camera_name;
            public string trigger_name;

            public int loop;

            public float loop_delay;
            public float loop_delay_beats;
            public bool loop_delay_beats_in_beats;

            public float start_delay;
            public float start_delay_beats;
            public bool start_delay_beats_in_beats;

            public TriggerContext context;
        }
        private class LayoutContext {
            public int macro_list_size;
            public Macro[] macro_list;
            public AnimList animlist;
            public ArrayList<Item> sound_list;
            public ArrayList<Item> vertex_list;
            public ArrayList<Group> group_list;
            public ArrayList<Trigger> trigger_list;
            public ExternalValue[] values;
            public int values_size;
            public ArrayList<CameraPlaceholder> camera_list;
            public Font[] fonts;
            public int fonts_size;
            public ResourcePool resource_pool;
        }
        private class Item {
            public Action[] actions;
            public int actions_size;
            public PVRContextVertex type;
            public int group_id;
            public SoundPlayer soundplayer;
            public LayoutPlaceholder placeholder;
            public IVertex vertex;
            public LayoutParallax parallax;
            public bool static_camera;
            public string name;
            public AnimSprite animation;
            public bool was_playing;
            public string initial_action_name;
        }
        private class Macro {
            public string name;
            public MacroAction[] actions;
            public int actions_size;
            public int loop;
            public bool loop_by_beats;
        }
        private class MacroAction {
            public int type;
            public int id;
            public string target_name;
            public string action_name;
            public string trigger_name;
            public string camera_name;
        }
        private class Action {
            public int entries_size;
            public ActionEntry[] entries;
            public string name;
        }
        private class GroupContext {
            public bool visible;
            public float alpha;
            public PVRContextFlag antialiasing;
            public SH4Matrix matrix;
            public float[] offsetcolor;
            public LayoutParallax parallax;
            public Group next_child;
            public Group next_sibling;
            public Group parent_group;
            public int last_z_index;
        }
        private class ActionEntry {
            public int type;
            public bool enable;
            public object misc;
            public string uniform_name;
            public float max_width, max_height;
            public bool cover, center;
            public float size;
            public float[] rgba;
            public bool has_enable;
            public bool restart, stop_in_loop;
            public bool override_size;
            public bool visible;
            public float width, height;
            public bool has_resize;
            public float x, y, z;
            public Align align_vertical, align_horizontal;
            public int property;
            public float value;
            public bool enabled;
            public Blend blend_src_rgb;
            public Blend blend_dst_rgb;
            public Blend blend_src_alpha;
            public Blend blend_dst_alpha;
        }
        private class Group : ISetProperty {
            public Action[] actions;
            public int actions_size;
            public Modifier modifier;
            public string name;
            public float alpha;
            public float[] offsetcolor;
            public bool visible;
            public LayoutParallax parallax;
            public AnimSprite animation;
            public bool static_camera;
            public int group_id;
            public string initial_action_name;
            public SH4Matrix static_screen;
            public PVRContextFlag antialiasing;
            public PSShader psshader;
            public PSFramebuffer psframebuffer;
            public bool blend_enabled;
            public Blend blend_src_rgb;
            public Blend blend_src_alpha;
            public Blend blend_dst_rgb;
            public Blend blend_dst_alpha;
            public float viewport_x;
            public float viewport_y;
            public float viewport_width;
            public float viewport_height;
            public GroupContext context;

            public void SetProperty(int id, float value) => Layout.HelperGroupSetProperty(this, id, value);
        }
        private sealed class HelperZbufferSortImpl : IComparer<ZBufferEntry> {
            public int Compare(ZBufferEntry entry1, ZBufferEntry entry2) {
                return entry1.z_index.CompareTo(entry2.z_index);
            }
        }

    }

}
