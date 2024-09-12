#include "layout.h"
#include "animlist.h"
#include "animsprite.h"
#include "arraylist.h"
#include "arraypointerlist.h"
#include "assert.h"
#include "beatwatcher.h"
#include "camera.h"
#include "cloneutils.h"
#include "drawable.h"
#include "externals/luascript.h"
#include "float64.h"
#include "fontglyph.h"
#include "fontholder.h"
#include "fonttype.h"
#include "fs.h"
#include "game/common/funkin.h"
#include "imgutils.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "number_format_specifiers.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "soundplayer.h"
#include "sprite.h"
#include "statesprite.h"
#include "stringbuilder.h"
#include "stringutils.h"
#include "textsprite.h"
#include "tokenizer.h"
#include "unused_switch_case.h"
#include "vertexprops.h"
#include "videoplayer.h"

#define ATTR_STRDUP(XML_NODE, ATTRIBUTE_NAME) string_duplicate(xmlparser_get_attribute_value2(XML_NODE, ATTRIBUTE_NAME))

#define LAYOUT_ACTION_LOCATION 0
#define LAYOUT_ACTION_SIZE 1
#define LAYOUT_ACTION_ANIMATION 2
#define LAYOUT_ACTION_ANIMATIONREMOVE 3
#define LAYOUT_ACTION_OFFSETCOLOR 4
#define LAYOUT_ACTION_COLOR 5
#define LAYOUT_ACTION_PROPERTY 6
#define LAYOUT_ACTION_ATLASAPPLY 7
#define LAYOUT_ACTION_VISIBILITY 8
#define LAYOUT_ACTION_MODIFIER 9
#define LAYOUT_ACTION_PARALLAX 10
#define LAYOUT_ACTION_TEXTBORDER 11
#define LAYOUT_ACTION_RESIZE 12
#define LAYOUT_ACTION_RESETMATRIX 13
#define LAYOUT_ACTION_STATIC 14
#define LAYOUT_ACTION_ANIMATIONEND 15
#define LAYOUT_ACTION_EXECUTE 16
#define LAYOUT_ACTION_PUPPETITEM 17
#define LAYOUT_ACTION_PUPPETGROUP 18
#define LAYOUT_ACTION_SETSHADER 19
#define LAYOUT_ACTION_REMOVESHADER 20
#define LAYOUT_ACTION_SETSHADERUNIFORM 22
#define LAYOUT_ACTION_SETBLENDING 23
#define LAYOUT_ACTION_VIEWPORT 24
#define LAYOUT_ACTION_TEXTBORDEROFFSET 25
#define LAYOUT_ACTION_SPRITE_TRAILING 26
#define LAYOUT_ACTION_SPRITE_TRAILINGOFFSETCOLOR 27
#define LAYOUT_ACTION_TEXTBACKGROUND 28
#define LAYOUT_ACTION_TEXTBACKGROUNDCOLOR 29
#define LAYOUT_ACTION_SOUNDFADE 30
#define LAYOUT_ACTION_SEEK 31


typedef union {
    Sprite as_sprite;
    TextSprite as_textsprite;
    void* ptr;
} VertexPointer;
typedef union {
    AnimSprite animsprite;
    AtlasEntry* atlas_entry;
    char* string;
    Modifier* modifier;
    // PSShader shader;
    // float64* uniform_values;
} MiscValue;
typedef struct Group_s Group;
typedef struct {
    union {
        Atlas as_atlas;
        Texture as_texture;
    };
    char* path;
    bool is_texture;
} ResourcePoolEntry;
typedef struct {
    ArrayList /*<ResourcePoolEntry>*/ textures;
    ArrayList /*<ResourcePoolEntry>*/ atlas;
} ResourcePool;
typedef struct {
    FontHolder fontholder;
    char* name;
} Font;
typedef struct {
    char* name;
    AttachedValueType type;
    LayoutAttachedValue value;
} ExternalValue;
typedef struct {
    bool running;
    bool reject_recursive;
    float64 progress_delay;
    bool loop_waiting;
    int32_t loop_count;
} TriggerContext;
typedef struct {
    char* name;

    char* action_name;
    char* camera_name;
    char* trigger_name;
    char* stop_trigger_name;

    int32_t loop;

    float loop_delay;
    float loop_delay_beats;
    bool loop_delay_beats_in_beats;

    float start_delay;
    float start_delay_beats;
    bool start_delay_beats_in_beats;

    TriggerContext context;
} Trigger;
typedef struct {
    int32_t type;
    int32_t id;
    char* target_name;
    char* action_name;
    char* trigger_name;
    char* stop_trigger_name;
    char* camera_name;
} MacroAction;
typedef struct {
    char* name;
    MacroAction* actions;
    int32_t actions_size;
    int32_t loop;
    bool loop_by_beats;
} Macro;
typedef struct {
    int32_t macro_list_size;
    Macro* macro_list;
    AnimList animlist;
    ArrayList /*<Item>*/ sound_list;
    ArrayList /*<Item>*/ video_list;
    ArrayList /*<Item>*/ vertex_list;
    ArrayPointerList /*<Group>*/ group_list;
    ArrayList /*<Trigger>*/ trigger_list;
    ExternalValue* values;
    int32_t values_size;
    ArrayList /*<CameraPlaceholder>*/ camera_list;
    Font* fonts;
    int32_t fonts_size;
    ResourcePool resource_pool;
} LayoutContext;
typedef struct {
    int32_t type;
    bool enable;
    MiscValue misc;

    // char* uniform_name;

    float max_width, max_height;
    bool cover, center;
    float size;
    RGBA rgba;
    bool has_enable;
    bool restart, stop_in_loop;
    bool override_size;
    bool visible;
    float width, height;
    bool has_resize;
    float x, y, z;
    Align align_vertical, align_horizontal;
    int32_t property;
    float value;
    bool enabled;
    Blend blend_src_rgb;
    Blend blend_dst_rgb;
    Blend blend_src_alpha;
    Blend blend_dst_alpha;
    int32_t length;
    float trail_delay;
    float trail_alpha;
    bool darken;
    bool has_darken;
    float64 position;
} ActionEntry;
typedef struct {
    int32_t entries_size;
    ActionEntry* entries;
    char* name;
} Action;
typedef struct {
    Action* actions;
    int32_t actions_size;
    PVRVertex type;
    int32_t group_id;
    SoundPlayer soundplayer;
    VideoPlayer videoplayer;
    int32_t in_vertex_list_index;
    LayoutPlaceholder* placeholder;
    VertexPointer vertex;
    LayoutParallax parallax;
    bool static_camera;
    char* name;
    AnimSprite animation;
    bool was_playing;
    char* initial_action_name;
} Item;
typedef struct {
    SH4Matrix matrix;
    bool visible;
    float alpha;
    PVRFlag antialiasing;
    RGBA offsetcolor;
    LayoutParallax parallax;
    Group* next_child;
    Group* next_sibling;
    Group* parent_group;
    int32_t last_z_index;
} GroupContext;
typedef struct Group_s {
    GroupContext context;
    Action* actions;
    int32_t actions_size;
    Modifier modifier;
    char* name;
    float alpha;
    RGBA offsetcolor;
    bool visible;
    LayoutParallax parallax;
    AnimSprite animation;
    bool static_camera;
    float* static_screen;
    int32_t group_id;
    char* initial_action_name;
    PVRFlag antialiasing;
    float alpha2;

    // PSShader psshader;
    // PSFramebuffer psframebuffer;

    bool blend_enabled;
    Blend blend_src_rgb;
    Blend blend_src_alpha;
    Blend blend_dst_rgb;
    Blend blend_dst_alpha;
    float viewport_x;
    float viewport_y;
    float viewport_width;
    float viewport_height;
} Group;
typedef struct {
    Item* item;
    float z_index;
    bool visible;
} ZBufferEntry;


struct Layout_s {
    SH4Matrix MATRIX_VIEWPORT;
    SH4Matrix MATRIX_SCREEN;

    float animation_speed;
    Camera camera_helper;
    Camera camera_secondary_helper;
    Font* fonts;
    int32_t fonts_size;
    ExternalValue* values;
    int32_t values_size;
    Macro* macro_list;
    int32_t macro_list_size;
    bool keep_aspect_ratio;
    float viewport_width;
    float viewport_height;
    Modifier modifier_camera;
    Modifier modifier_camera_secondary;
    Modifier modifier_viewport;
    Item* vertex_list;
    int32_t vertex_list_size;
    Group** group_list;
    int32_t group_list_size;
    CameraPlaceholder* camera_list;
    int32_t camera_list_size;
    Trigger* trigger_list;
    int32_t trigger_list_size;
    Item* sound_list;
    int32_t sound_list_size;
    Item* video_list;
    int32_t video_list_size;
    Texture* textures;
    int32_t textures_size;
    ZBufferEntry* z_buffer;
    int32_t z_buffer_size;
    Item* external_vertex_list;
    int32_t external_vertex_list_size;
    Item* single_item;
    bool suspended;
    bool beatwatcher_synced_triggers;
    float64 beatwatcher_last_timestamp;
    int32_t beatwatcher_resyncs_count;
    BeatWatcher beatwatcher;
    bool antialiasing_disabled;
    int32_t resolution_changes;
    bool self_print_trigger_calls;

    // PSShader psshader;
};


static const char* LAYOUT_GROUP_ROOT = "root-group";
bool LAYOUT_DEBUG_PRINT_TRIGGER_CALLS = false;


static void layout_helper_destroy_actions(Action* actions, int32_t actions_size);
static void* layout_helper_get_resource(ResourcePool* resource_pool, const char* src, bool is_texture);
static void layout_helper_location(ActionEntry* action_entry, float width, float height, float v_width, float v_height, float* location_x, float* location_y);
static uint32_t layout_helper_parse_hex(XmlNode node, const char* attr_name, uint32_t def_value);
static float layout_helper_parse_float(XmlNode node, const char* attr_name, float def_value);
static Align layout_helper_parse_align(XmlNode node, bool is_vertical);
static Align layout_helper_parse_align2(XmlNode node, bool is_vertical, Align def_value);
static void layout_helper_parse_color(XmlNode node, RGBA rgba);
static void* layout_helper_get_vertex(Layout layout, PVRVertex type, const char* name);
static int32_t layout_helper_get_group_index(Layout layout, const char* name);
static void layout_helper_execute_action(Layout layout, Item* item, Action* action);
static void layout_helper_execute_trigger(Layout layout, Trigger* trigger);
static void layout_helper_set_parallax_info(LayoutParallax* parallax_info, ActionEntry* parallax_action);
static void layout_helper_stack_groups(Group* parent_group);
static int32_t layout_helper_group_animate(Group* group, float elapsed);
static void layout_helper_group_set_property(Group* group, int32_t property_id, float value);
static void layout_helper_parse_property(XmlNode unparsed_entry, int32_t property_id, const char* value_holder, ArrayList action_entries);
static void layout_herper_parse_offsetmovefromto(XmlNode unparsed_offsetmovefromto, float* x, float* y, float* z);
static void layout_helper_zbuffer_build(Layout layout);
static int layout_helper_zbuffer_sort(const void* entry1_ptr, const void* entry2_ptr);
static void layout_helper_add_group_to_parent(GroupContext* parent_context, Group* group);
static int32_t layout_helper_check_trigger_queue(Layout layout, float elapsed);
static void layout_helper_commit_trigger(Layout layout, Trigger* trigger);
static void layout_parse_placeholder(XmlNode unparsed_plchdlr, LayoutContext* layout_context, int32_t group_id);
static void layout_parse_sprite(XmlNode unparsed_sprite, LayoutContext* layout_context, int32_t group_id);
static void layout_parse_text(XmlNode unparsed_text, LayoutContext* layout_context, int32_t group_id);
static void layout_parse_group(XmlNode unparsed_group, LayoutContext* layout_context, GroupContext* parent_context);
static void layout_parse_fonts(XmlNode unparsed_root, LayoutContext* layout_context);
static void layout_parse_camera(XmlNode unparsed_camera, LayoutContext* layout_context);
static void layout_parse_externalvalues(XmlNode unparsed_root, LayoutContext* layout_context);
static void layout_parse_triggers(XmlNode unparsed_trigger, LayoutContext* layout_context);
static void layout_parse_sound(XmlNode unparsed_sound, LayoutContext* layout_context);
static void layout_parse_video(XmlNode unparsed_video, LayoutContext* layout_context, int32_t group_id);
static void layout_parse_macro(XmlNode unparsed_root, LayoutContext* layout_context);
static void layout_parse_sprite_action(XmlNode unparsed_action, AnimList animlist, Atlas atlas, ArrayList action_entries, bool from_video);
static void layout_parse_text_action(XmlNode unparsed_action, AnimList animlist, ArrayList action_entries);
static void layout_parse_group_action(XmlNode unparsed_action, AnimList animlist, ArrayList action_entries);
static void layout_parse_sound_action(XmlNode unparsed_action, AnimList animlist, ArrayList action_entries);
static void layout_parse_video_action(XmlNode unparsed_action, AnimList animlist, ArrayList action_entries);
static void layout_parse_macro_actions(XmlNode unparsed_macro, Macro* macro);
static void layout_helper_execute_action_in_sprite(Action* action, Item* item, float viewport_width, float viewport_height);
static void layout_helper_execute_action_in_textsprite(Action* action, Item* item, float viewport_width, float viewport_height);
static void layout_helper_execute_action_in_group(Action* action, Group* group);
static void layout_helper_execute_action_in_sound(Action* action, Item* item);
static void layout_helper_execute_action_in_video(Action* action, Item* item_video, Item* item_sprite, float viewport_width, float viewport_height);
static void layout_helper_add_action_property(XmlNode unparsed_entry, bool is_textsprite, ArrayList action_entries);
static void layout_helper_add_action_properties(XmlNode unparsed_entry, bool is_textsprite, ArrayList action_entries);
static void layout_helper_add_action_offsetcolor(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_color(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_modifier(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_parallax(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_location(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_size(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_visibility(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_atlasapply(XmlNode unparsed_entry, Atlas atlas, ArrayList action_entries);
static void layout_helper_add_action_animation(XmlNode unparsed_entry, AnimList animlist, ArrayList action_entries);
static void layout_helper_add_action_animationfromatlas(XmlNode unparsed_entry, Atlas atlas, ArrayList action_entries);
static void layout_helper_add_action_animationremove(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_animationend(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_textborder(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_resize(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_resetmatrix(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_static(XmlNode unparsed_entry, ArrayList action_entries);
static bool layout_helper_add_action_media(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_mediaproperty(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_mediaproperties(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_setshader(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_removeshader(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_setshaderuniform(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_setblending(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_viewport(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_textborderoffset(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_textbackground(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_textbackgroundcolor(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_spritetrailing(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_spritetrailingoffsetcolor(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_soundfade(XmlNode unparsed_entry, ArrayList action_entries);
static void layout_helper_add_action_seek(XmlNode unparsed_entry, ArrayList action_entries);


Layout layout_init(const char* src) {
    XmlParser xml = xmlparser_init(src);

    if (!xml) {
        logger_error("layout_init() error reading: %s", src);
        return NULL;
    }

    // change the current working folder but remember the old one
    fs_folder_stack_push();
    fs_set_working_folder(src, true);

    LayoutContext layout_context = (LayoutContext){
        .animlist = NULL,
        .vertex_list = arraylist_init(sizeof(Item)),
        .group_list = arraypointerlist_init(),
        .camera_list = arraylist_init(sizeof(CameraPlaceholder)),
        .trigger_list = arraylist_init(sizeof(Trigger)),
        .sound_list = arraylist_init(sizeof(Item)),
        .video_list = arraylist_init(sizeof(Item)),
        .resource_pool = {
            .textures = arraylist_init(sizeof(ResourcePoolEntry)),
            .atlas = arraylist_init(sizeof(ResourcePoolEntry))
        },
        .fonts = NULL,
        .fonts_size = -1,
        .values = NULL,
        .values_size = -1,
        .macro_list = NULL,
        .macro_list_size = 0
    };

    XmlNode root = xmlparser_get_root(xml);
    assert(root && string_equals(xmlparser_get_tag_name(root), "Layout"));

    // step 1: read attributes of the root
    float viewport_width = layout_helper_parse_float(root, "width", -1.0f);
    float viewport_height = layout_helper_parse_float(root, "height", -1.0f);
    float animation_speed = layout_helper_parse_float(root, "animationSpeed", 1.0f);
    bool keep_aspect_ratio = vertexprops_parse_boolean(root, "keepAspectRatio", false);
    bool self_print_trigger_calls = vertexprops_parse_boolean(root, "printTriggerCalls", false);

    if (viewport_width < 1.0f) {
        logger_error("Invalid/missing layout width");
        assert(viewport_width >= 1.0f);
    }
    if (viewport_height < 1.0f) {
        logger_error("Invalid/missing layout height");
        assert(viewport_height >= 1.0f);
    }

    const char* animlist_filename = xmlparser_get_attribute_value2(root, "animationList");
    if (animlist_filename) {
        layout_context.animlist = animlist_init(animlist_filename);
    }

    // step 2.1: read fonts
    layout_parse_fonts(root, &layout_context);

    // step 2.2: import all external values
    layout_parse_externalvalues(root, &layout_context);

    // step 2.3: import macros
    layout_parse_macro(root, &layout_context);

    // step 3: import all groups (including sprites, texts and placeholders)
    layout_parse_group(root, &layout_context, NULL);


    // step 4: build the layout object (must be 32 bit aligned)
    Layout layout = memalign_chk(32, sizeof(struct Layout_s));
    malloc_assert(layout, Layout);

    *layout = (struct Layout_s){
        .animation_speed = animation_speed,

        .camera_helper = NULL,
        .camera_secondary_helper = NULL,

        .fonts = layout_context.fonts,
        .fonts_size = layout_context.fonts_size,

        .values = layout_context.values,
        .values_size = layout_context.values_size,

        .macro_list = layout_context.macro_list,
        .macro_list_size = layout_context.macro_list_size,

        .keep_aspect_ratio = keep_aspect_ratio,

        .viewport_width = viewport_width,
        .viewport_height = viewport_height,

        .modifier_camera = {},
        .modifier_camera_secondary = {},
        .modifier_viewport = {},

        .vertex_list = NULL,
        .vertex_list_size = -1,

        .group_list = NULL,
        .group_list_size = -1,

        .camera_list = NULL,
        .camera_list_size = -1,

        .trigger_list = NULL,
        .trigger_list_size = -1,

        .sound_list = NULL,
        .sound_list_size = -1,

        .video_list = NULL,
        .video_list_size = -1,

        .textures = NULL,
        .textures_size = 0,

        .z_buffer = NULL,
        .z_buffer_size = 0,

        .external_vertex_list = NULL,
        .external_vertex_list_size = 0,

        .single_item = NULL,
        .suspended = false,

        .beatwatcher_synced_triggers = false,
        .beatwatcher_last_timestamp = 0.0,
        .beatwatcher_resyncs_count = 0,
        .beatwatcher = {},

        .antialiasing_disabled = false,
        .resolution_changes = 0,
        .self_print_trigger_calls = self_print_trigger_calls

        //.psshader = NULL
    };

    // step 5: build modifiers
    pvr_context_helper_clear_modifier(&layout->modifier_camera);
    pvr_context_helper_clear_modifier(&layout->modifier_camera_secondary);
    pvr_context_helper_clear_modifier(&layout->modifier_viewport);

    layout->modifier_camera.width = viewport_width;
    layout->modifier_camera.height = viewport_height;
    layout->camera_helper = camera_init(&layout->modifier_camera, viewport_width, viewport_height);
    layout->camera_secondary_helper = camera_init(&layout->modifier_camera_secondary, viewport_width, viewport_height);

    layout->modifier_viewport.x = 0.0f;
    layout->modifier_viewport.y = 0.0f;
    layout->modifier_viewport.width = layout->viewport_width;
    layout->modifier_viewport.height = layout->viewport_height;

    // step 6: build vertex and group arrays
    arraylist_destroy2(&layout_context.vertex_list, &layout->vertex_list_size, (void**)&layout->vertex_list);
    arraypointerlist_destroy2(&layout_context.group_list, &layout->group_list_size, (void**)&layout->group_list);
    arraylist_destroy2(&layout_context.camera_list, &layout->camera_list_size, (void**)&layout->camera_list);
    arraylist_destroy2(&layout_context.trigger_list, &layout->trigger_list_size, (void**)&layout->trigger_list);
    arraylist_destroy2(&layout_context.sound_list, &layout->sound_list_size, (void**)&layout->sound_list);
    arraylist_destroy2(&layout_context.video_list, &layout->video_list_size, (void**)&layout->video_list);

    // step 7: build textures array
    layout->textures_size = arraylist_size(layout_context.resource_pool.textures);
    layout->textures = malloc_for_array(Texture, layout->textures_size);

    ResourcePoolEntry* list = arraylist_peek_array(layout_context.resource_pool.textures);
    for (int32_t i = 0; i < layout->textures_size; i++) {
        layout->textures[i] = list[i].as_texture;
        free_chk(list[i].path);
    }

    // step 8: build z-buffer
    layout->z_buffer = malloc_for_array(ZBufferEntry, layout->vertex_list_size);
    layout->z_buffer_size = layout->vertex_list_size;
    layout_helper_zbuffer_build(layout);

    // step 9: cleanup
    foreach (ResourcePoolEntry*, definition, ARRAYLIST_ITERATOR, layout_context.resource_pool.atlas) {
        if (definition->as_atlas) atlas_destroy(&definition->as_atlas);
        free_chk(definition->path);
    }

    arraylist_destroy(&layout_context.resource_pool.textures);
    arraylist_destroy(&layout_context.resource_pool.atlas);

    if (layout_context.animlist) animlist_destroy(&layout_context.animlist);

    // configure layout viewport with the PVR's render size (screen resolution)
    layout->resolution_changes = pvr_context.resolution_changes;
    layout_update_render_size(layout, pvr_context.screen_width, pvr_context.screen_height);

    // step 10: execute all default actions (actions with no names) and initial actions
    for (int32_t i = 0; i < layout->vertex_list_size; i++) {
        char* initial_action_name = layout->vertex_list[i].initial_action_name;
        bool intial_action_found = initial_action_name == NULL;

        for (int32_t j = 0; j < layout->vertex_list[i].actions_size; j++) {
            char* action_name = layout->vertex_list[i].actions[j].name;

            if (action_name == NULL || string_equals(action_name, initial_action_name)) {
                if (string_equals(action_name, initial_action_name)) intial_action_found = true;

                layout_helper_execute_action(
                    layout, &layout->vertex_list[i], &layout->vertex_list[i].actions[j]
                );
            }
        }

        if (!intial_action_found)
            logger_warn("layout_init() initial action not found: %s", initial_action_name);
    }

    for (int32_t i = 0; i < layout->group_list_size; i++) {
        char* initial_action_name = layout->group_list[i]->initial_action_name;
        bool intial_action_found = initial_action_name == NULL;

        for (int32_t j = 0; j < layout->group_list[i]->actions_size; j++) {
            char* action_name = layout->group_list[i]->actions[j].name;

            if (action_name == NULL || string_equals(action_name, initial_action_name)) {
                if (string_equals(action_name, initial_action_name)) intial_action_found = true;

                layout_helper_execute_action_in_group(
                    &layout->group_list[i]->actions[j], layout->group_list[i]
                );
            }
        }

        if (!intial_action_found)
            logger_warn("layout_init() initial action not found: %s", initial_action_name);
    }

    for (int32_t i = 0; i < layout->sound_list_size; i++) {
        char* initial_action_name = layout->sound_list[i].initial_action_name;
        bool intial_action_found = initial_action_name == NULL;

        for (int32_t j = 0; j < layout->sound_list[i].actions_size; j++) {
            char* action_name = layout->sound_list[i].actions[j].name;

            if (action_name == NULL || string_equals(action_name, initial_action_name)) {
                if (string_equals(action_name, initial_action_name)) intial_action_found = true;

                layout_helper_execute_action_in_sound(
                    &layout->sound_list[i].actions[j], &layout->sound_list[i]
                );
            }
        }

        if (!intial_action_found)
            logger_warn("layout_init() initial action not found: %s", initial_action_name);
    }

    for (int32_t i = 0; i < layout->video_list_size; i++) {
        char* initial_action_name = layout->video_list[i].initial_action_name;
        bool intial_action_found = initial_action_name == NULL;

        for (int32_t j = 0; j < layout->video_list[i].actions_size; j++) {
            char* action_name = layout->video_list[i].actions[j].name;

            if (action_name == NULL || string_equals(action_name, initial_action_name)) {
                if (string_equals(action_name, initial_action_name)) intial_action_found = true;

                Item* item_sprite = &layout->vertex_list[layout->video_list[i].in_vertex_list_index];
                layout_helper_execute_action_in_video(
                    &layout->video_list[i].actions[j], &layout->video_list[i],
                    item_sprite, layout->viewport_width, layout->viewport_height
                );
            }
        }

        if (!intial_action_found)
            logger_warn("layout_init() initial action not found: %s", initial_action_name);
    }

    for (int32_t i = 0; i < layout->trigger_list_size; i++) {
        if (layout->trigger_list[i].name != NULL) continue;
        layout_helper_execute_trigger(layout, &layout->trigger_list[i]);
    }

    // only is possible to trigger the first NULL-named camera
    camera_from_layout(layout->camera_helper, layout, NULL);
    camera_set_parent_layout(layout->camera_helper, layout);
    camera_set_parent_layout(layout->camera_secondary_helper, layout);

    // set default beats per second
    float bpm = layout_helper_parse_float(root, "defaultBPM", 100.0f);
    beatwatcher_reset(&layout->beatwatcher, true, bpm);
    layout_set_bpm(layout, bpm);

    // restore previous working folder
    fs_folder_stack_pop();

    xmlparser_destroy(&xml);

    return layout;
}

void layout_destroy(Layout* layout_ptr) {
    Layout layout = *layout_ptr;
    if (!layout) return;

    for (int32_t i = 0; i < layout->vertex_list_size; i++) {
        layout_helper_destroy_actions(layout->vertex_list[i].actions, layout->vertex_list[i].actions_size);
        free_chk(layout->vertex_list[i].name);
        free_chk(layout->vertex_list[i].initial_action_name);

        if (layout->vertex_list[i].videoplayer) continue;

        switch (layout->vertex_list[i].type) {
            case VERTEX_SPRITE:
                sprite_destroy(&layout->vertex_list[i].vertex.as_sprite);
                break;
            case VERTEX_DRAWABLE:
                layout->vertex_list[i].placeholder->vertex = NULL; // external drawable ¡DO NOT DISPOSE!
                free_chk(layout->vertex_list[i].placeholder->name);
                free_chk(layout->vertex_list[i].placeholder);
                break;
            case VERTEX_TEXTSPRITE:
                textsprite_animation_set(layout->vertex_list[i].vertex.as_textsprite, NULL); // avoid dispose action animation
                textsprite_destroy(&layout->vertex_list[i].vertex.as_textsprite);
                break;
            case VERTEX_STATESPRITE:
            case VERTEX_SHAPE:
            case VERTEX_NONE:
                break;
        }
    }
    free_chk(layout->vertex_list);

    for (int32_t i = 0; i < layout->values_size; i++) {
        if (layout->values[i].type == AttachedValueType_STRING) free_chk((char*)layout->values[i].value.as_string);
        free_chk(layout->values[i].name);
    }
    free_chk(layout->values);

    for (int32_t i = 0; i < layout->fonts_size; i++) {
        free_chk(layout->fonts[i].name);
        fontholder_destroy(&layout->fonts[i].fontholder);
    }
    free_chk(layout->fonts);

    for (int32_t i = 0; i < layout->textures_size; i++) {
        texture_destroy(&layout->textures[i]);
    }
    free_chk(layout->textures);

    for (int32_t i = 0; i < layout->group_list_size; i++) {
        if (/*pointer equals*/ layout->group_list[i]->name != LAYOUT_GROUP_ROOT) free_chk(layout->group_list[i]->name);
        free_chk(layout->group_list[i]->initial_action_name);
        layout_helper_destroy_actions(layout->group_list[i]->actions, layout->group_list[i]->actions_size);
        // if (layout->group_list[i]->psframebuffer) layout->group_list[i]->psframebuffer.Destroy();

        free_chk(layout->group_list[i]);
    }
    free_chk(layout->group_list);

    for (int32_t i = 0; i < layout->camera_list_size; i++) {
        free_chk(layout->camera_list[i].name);
        if (layout->camera_list[i].animation) animsprite_destroy(&layout->camera_list[i].animation);
    }
    free_chk(layout->camera_list);

    for (int32_t i = 0; i < layout->trigger_list_size; i++) {
        free_chk(layout->trigger_list[i].name);
        free_chk(layout->trigger_list[i].action_name);
        free_chk(layout->trigger_list[i].camera_name);
        free_chk(layout->trigger_list[i].trigger_name);
        free_chk(layout->trigger_list[i].stop_trigger_name);
    }
    free_chk(layout->trigger_list);

    for (int32_t i = 0; i < layout->sound_list_size; i++) {
        layout_helper_destroy_actions(layout->sound_list[i].actions, layout->sound_list[i].actions_size);
        free_chk(layout->sound_list[i].name);
        free_chk(layout->sound_list[i].initial_action_name);
        soundplayer_destroy(&layout->sound_list[i].soundplayer);
    }
    free_chk(layout->sound_list);

    for (int32_t i = 0; i < layout->video_list_size; i++) {
        layout_helper_destroy_actions(layout->video_list[i].actions, layout->video_list[i].actions_size);
        free_chk(layout->video_list[i].name);
        free_chk(layout->video_list[i].initial_action_name);
        videoplayer_destroy(&layout->video_list[i].videoplayer);
    }
    free_chk(layout->video_list);

    for (int32_t i = 0; i < layout->macro_list_size; i++) {
        for (int32_t j = 0; j < layout->macro_list[i].actions_size; j++) {
            free_chk(layout->macro_list[i].actions[j].target_name);
            free_chk(layout->macro_list[i].actions[j].action_name);
            free_chk(layout->macro_list[i].actions[j].trigger_name);
            free_chk(layout->macro_list[i].actions[j].stop_trigger_name);
            free_chk(layout->macro_list[i].actions[j].camera_name);
        }
        free_chk(layout->macro_list[i].actions);
    }
    free_chk(layout->macro_list);

    camera_destroy(&layout->camera_helper);
    camera_destroy(&layout->camera_secondary_helper);

    free_chk(layout->external_vertex_list);
    free_chk(layout->z_buffer);

    luascript_drop_shared(&layout->modifier_camera);
    luascript_drop_shared(&layout->modifier_camera_secondary);
    luascript_drop_shared(layout);

    free_chk(layout);
    *layout_ptr = NULL;
}


int32_t layout_trigger_any(Layout layout, const char* action_triger_camera_interval_name) {
    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS || layout->self_print_trigger_calls) {
        logger_log("layout_trigger_any() target='%s'", action_triger_camera_interval_name);
    }
    int32_t res = 0;
    res += layout_trigger_action(layout, NULL, action_triger_camera_interval_name);
    res += layout_trigger_camera(layout, action_triger_camera_interval_name) ? 1 : 0;
    res += layout_trigger_trigger(layout, action_triger_camera_interval_name);

    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS || layout->self_print_trigger_calls) {
        logger_log("layout_trigger_any() target='%s' result=" FMT_I4, action_triger_camera_interval_name, res);
    }

    return res;
}

int32_t layout_trigger_action(Layout layout, const char* target_name, const char* action_name) {
    int32_t count = 0;
    const char* initial_action_name;

    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS || layout->self_print_trigger_calls) {
        logger_log("layout_trigger_action() target='%s' action='%s'", target_name, action_name);
    }

    for (int32_t i = 0; i < layout->vertex_list_size; i++) {
        if (target_name != NULL && !string_equals(layout->vertex_list[i].name, target_name)) continue;

        if (action_name == NULL && layout->vertex_list[i].initial_action_name != NULL)
            initial_action_name = layout->vertex_list[i].initial_action_name;
        else
            initial_action_name = action_name;

        for (int32_t j = 0; j < layout->vertex_list[i].actions_size; j++) {
            Action* action = &layout->vertex_list[i].actions[j];

            if (string_equals(action->name, initial_action_name) || string_equals(action->name, action_name)) {
                layout_helper_execute_action(layout, &layout->vertex_list[i], action);
                count++;
            }
        }
    }

    for (int32_t i = 0; i < layout->group_list_size; i++) {
        if (target_name != NULL && !string_equals(layout->group_list[i]->name, target_name)) continue;

        for (int32_t j = 0; j < layout->group_list[i]->actions_size; j++) {
            Action* action = &layout->group_list[i]->actions[j];

            if (action_name == NULL && layout->group_list[i]->initial_action_name != NULL)
                initial_action_name = layout->group_list[i]->initial_action_name;
            else
                initial_action_name = action_name;

            if (string_equals(action->name, initial_action_name) || string_equals(action->name, action_name)) {
                layout_helper_execute_action_in_group(action, layout->group_list[i]);
                count++;
            }
        }
    }

    for (int32_t i = 0; i < layout->sound_list_size; i++) {
        if (target_name != NULL && !string_equals(layout->sound_list[i].name, target_name)) continue;

        for (int32_t j = 0; j < layout->sound_list[i].actions_size; j++) {
            Action* action = &layout->sound_list[i].actions[j];

            if (action_name == NULL && layout->sound_list[i].initial_action_name != NULL)
                initial_action_name = layout->sound_list[i].initial_action_name;
            else
                initial_action_name = action_name;

            if (string_equals(action->name, initial_action_name) || string_equals(action->name, action_name)) {
                layout_helper_execute_action_in_sound(action, &layout->sound_list[i]);
                count++;
            }
        }
    }

    for (int32_t i = 0; i < layout->video_list_size; i++) {
        if (target_name != NULL && !string_equals(layout->video_list[i].name, target_name)) continue;

        for (int32_t j = 0; j < layout->video_list[i].actions_size; j++) {
            Action* action = &layout->video_list[i].actions[j];

            if (action_name == NULL && layout->video_list[i].initial_action_name != NULL)
                initial_action_name = layout->video_list[i].initial_action_name;
            else
                initial_action_name = action_name;

            if (string_equals(action->name, initial_action_name) || string_equals(action->name, action_name)) {
                Item* item_video = &layout->video_list[i];
                Item* item_sprite = &layout->vertex_list[item_video->in_vertex_list_index];
                layout_helper_execute_action_in_video(
                    action, item_video,
                    item_sprite, layout->viewport_width, layout->viewport_height
                );
                count++;
            }
        }
    }

    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS || layout->self_print_trigger_calls) {
        logger_log("layout_trigger_action() target='%s' action='%s' res=" FMT_I4, target_name, action_name, count);
    }

    return count;
}

int32_t layout_contains_action(Layout layout, const char* target_name, const char* action_name) {
    if (action_name == NULL) {
        logger_error("action_name is required");
        assert(action_name);
    }

    int32_t count = 0;

    for (int32_t i = 0; i < layout->vertex_list_size; i++) {
        if (!string_equals(layout->vertex_list[i].name, target_name)) continue;
        for (int32_t j = 0; j < layout->vertex_list[i].actions_size; j++) {
            if (string_equals(layout->vertex_list[i].actions[j].name, action_name)) count++;
        }
    }

    for (int32_t i = 0; i < layout->group_list_size; i++) {
        if (!string_equals(layout->group_list[i]->name, target_name)) continue;
        for (int32_t j = 0; j < layout->group_list[i]->actions_size; j++) {
            if (string_equals(layout->group_list[i]->actions[j].name, action_name)) count++;
        }
    }

    for (int32_t i = 0; i < layout->sound_list_size; i++) {
        if (!string_equals(layout->sound_list[i].name, target_name)) continue;
        for (int32_t j = 0; j < layout->sound_list[i].actions_size; j++) {
            if (string_equals(layout->sound_list[i].actions[j].name, action_name)) count++;
        }
    }

    for (int32_t i = 0; i < layout->video_list_size; i++) {
        if (!string_equals(layout->video_list[i].name, target_name)) continue;
        for (int32_t j = 0; j < layout->video_list[i].actions_size; j++) {
            if (string_equals(layout->video_list[i].actions[j].name, action_name)) count++;
        }
    }

    return count;
}

int32_t layout_animation_is_completed(Layout layout, const char* item_name) {
    if (item_name == NULL) {
        logger_error("entry_name is required");
        assert(item_name);
    }

    int32_t type = -1;
    void* vertex = NULL;
    AnimSprite animsprite = NULL;

    for (int32_t i = 0; i < layout->vertex_list_size; i++) {
        if (string_equals(layout->vertex_list[i].name, item_name)) {
            type = layout->vertex_list[i].type;
            vertex = layout->vertex_list[i].vertex.ptr;
            animsprite = layout->vertex_list[i].animation;
            break;
        }
    }

    if (!vertex) {
        // check if a group with this name exists
        for (int32_t i = 0; i < layout->group_list_size; i++) {
            if (string_equals(layout->group_list[i]->name, item_name)) {
                vertex = NULL;
                animsprite = layout->vertex_list[i].animation;
                break;
            }
        }
    }

    if (!vertex && !animsprite) return 2;

    if (animsprite != NULL) {
        if (animsprite_is_completed(animsprite)) return 1;
    } else {
        // call *_animate() to check if was completed.
        int32_t ret = 0;
        switch (type) {
            case VERTEX_DRAWABLE:
                ret = drawable_animate(vertex, 0.0f);
                break;
            case VERTEX_SPRITE:
                ret = sprite_animate(vertex, 0.0f);
                break;
            case VERTEX_STATESPRITE:
                ret = statesprite_animate(vertex, 0.0f);
                break;
            case VERTEX_TEXTSPRITE:
                ret = textsprite_animate(vertex, 0.0f);
                break;
            default:
                return -1;
        }

        if (ret > 0) return 1;
    }

    return 0;
}

void layout_update_render_size(Layout layout, float screen_width, float screen_height) {
    if (layout->keep_aspect_ratio) {
        float scale_x = screen_width / layout->viewport_width;
        float scale_y = screen_height / layout->viewport_height;
        float scale = math2d_min_float(scale_x, scale_y);

        layout->modifier_viewport.scale_x = layout->modifier_viewport.scale_y = scale;
        layout->modifier_viewport.translate_x = (screen_width - layout->viewport_width * scale) / 2.0f;
        layout->modifier_viewport.translate_y = (screen_height - layout->viewport_height * scale) / 2.0f;
    } else {
        layout->modifier_viewport.scale_x = screen_width / layout->viewport_width;
        layout->modifier_viewport.scale_y = screen_height / layout->viewport_height;
        layout->modifier_viewport.translate_x = layout->modifier_viewport.translate_y = 0.0f;
    }
}

void layout_screen_to_layout_coordinates(Layout layout, float screen_x, float screen_y, bool calc_with_camera, float* layout_x, float* layout_y) {
    // screen aspect ratio correction
    // screen_x -= pvr_context.screen_stride - pvr_context.screen_width;

    SH4Matrix temp;
    sh4matrix_clear(temp);
    sh4matrix_apply_modifier(temp, &layout->modifier_viewport);

    if (calc_with_camera) {
        sh4matrix_apply_modifier(temp, &layout->modifier_camera_secondary);
        sh4matrix_apply_modifier(temp, &layout->modifier_camera);
    }

    *layout_x = screen_x;
    *layout_y = screen_y;
    sh4matrix_multiply_point(temp, layout_x, layout_y);
}

void layout_camera_set_view(Layout layout, float x, float y, float depth) {
    camera_set_absolute(layout->camera_helper, x, y, depth);
}

bool layout_camera_is_completed(Layout layout) {
    return camera_is_completed(layout->camera_helper);
}

bool layout_trigger_camera(Layout layout, const char* camera_name) {
    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS || layout->self_print_trigger_calls) {
        logger_log("layout_trigger_camera() target='%s'", camera_name);
    }
    return camera_from_layout(layout->camera_helper, layout, camera_name);
}

int32_t layout_trigger_trigger(Layout layout, const char* trigger_name) {
    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS || layout->self_print_trigger_calls) {
        logger_log("layout_trigger_trigger() target='%s'", trigger_name);
    }

    int32_t count = 0;
    for (int32_t i = 0; i < layout->trigger_list_size; i++) {
        if (string_equals(layout->trigger_list[i].name, trigger_name)) {
            layout_helper_execute_trigger(layout, &layout->trigger_list[i]);
        }
    }

    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS || layout->self_print_trigger_calls) {
        logger_log("layout_trigger_trigger() target='%s' res=" FMT_I4, trigger_name, count);
    }

    return count;
}

void layout_stop_trigger(Layout layout, const char* trigger_name) {
    for (int32_t i = 0; i < layout->trigger_list_size; i++) {
        if (string_equals(layout->trigger_list[i].name, trigger_name))
            layout->trigger_list[i].context.running = false;
    }
}

void layout_stop_all_triggers(Layout layout) {
    for (int32_t i = 0; i < layout->trigger_list_size; i++) {
        layout->trigger_list[i].context.running = false;
    }
}

bool layout_set_placeholder_drawable_by_id(Layout layout, int32_t id, Drawable drawable) {
    if (id < 0 || id >= layout->vertex_list_size) return false;
    if (layout->vertex_list[id].type != VERTEX_DRAWABLE) return false;

    layout->vertex_list[id].placeholder->vertex = drawable;
    return true;
}

void layout_sync_triggers_with_global_beatwatcher(Layout layout, bool enable) {
    layout->beatwatcher_synced_triggers = enable;
    layout->beatwatcher.last_global_timestamp = -1;
    layout->beatwatcher.resyncs = 1;
}

void layout_set_bpm(Layout layout, float beats_per_minute) {
    float duration = math2d_beats_per_minute_to_beat_per_milliseconds(beats_per_minute);

    camera_set_bpm(layout->camera_helper, beats_per_minute);
    camera_set_bpm(layout->camera_secondary_helper, beats_per_minute);

    beatwatcher_change_bpm(&layout->beatwatcher, beats_per_minute);

    for (int32_t i = 0; i < layout->trigger_list_size; i++) {
        Trigger* trigger = &layout->trigger_list[i];

        if (trigger->loop_delay_beats_in_beats) {
            trigger->loop_delay = trigger->loop_delay_beats * duration;
        }
        if (trigger->start_delay_beats_in_beats) {
            trigger->start_delay = trigger->start_delay_beats * duration;
        }
    }
}


LayoutPlaceholder* layout_get_placeholder(Layout layout, const char* name) {
    for (int32_t i = 0; i < layout->vertex_list_size; i++) {
        if (layout->vertex_list[i].placeholder && string_equals(layout->vertex_list[i].placeholder->name, name)) {
            return layout->vertex_list[i].placeholder;
        }
    }
    return NULL;
}

int32_t layout_get_placeholder_id(Layout layout, const char* name) {
    for (int32_t i = 0; i < layout->vertex_list_size; i++) {
        if (layout->vertex_list[i].placeholder && string_equals(layout->vertex_list[i].placeholder->name, name)) {
            return i;
        }
    }
    return -1;
}


TextSprite layout_get_textsprite(Layout layout, const char* name) {
    return layout_helper_get_vertex(layout, VERTEX_TEXTSPRITE, name);
}

Sprite layout_get_sprite(Layout layout, const char* name) {
    return layout_helper_get_vertex(layout, VERTEX_SPRITE, name);
}

SoundPlayer layout_get_soundplayer(Layout layout, const char* name) {
    for (int32_t i = 0; i < layout->sound_list_size; i++) {
        if (string_equals(layout->sound_list[i].name, name)) {
            return layout->sound_list[i].soundplayer;
        }
    }
    return NULL;
}

VideoPlayer layout_get_videoplayer(Layout layout, const char* name) {
    for (int32_t i = 0; i < layout->video_list_size; i++) {
        if (string_equals(layout->video_list[i].name, name)) {
            return layout->video_list[i].videoplayer;
        }
    }
    return NULL;
}

Camera layout_get_camera_helper(Layout layout) {
    return layout->camera_helper;
}

Camera layout_get_secondary_camera_helper(Layout layout) {
    return layout->camera_secondary_helper;
}


FontHolder layout_get_attached_font(Layout layout, const char* font_name) {
    if (font_name == NULL)
        return layout->fonts_size < 1 ? NULL : layout->fonts[0].fontholder;

    for (int32_t i = 0; i < layout->fonts_size; i++) {
        if (string_equals(layout->fonts[i].name, font_name))
            return layout->fonts[i].fontholder;
    }

    return NULL;
}

LayoutAttachedValue layout_get_attached_value(Layout layout, const char* name, AttachedValueType expected_type, LayoutAttachedValue default_value) {
    for (int32_t i = 0; i < layout->values_size; i++) {
        if (!string_equals(layout->values[i].name, name)) continue;

        if ((layout->values[i].type & expected_type) == 0x00) {
            logger_warn("layout_get_attached_value() type missmatch of: %s", name);

            if (expected_type == AttachedValueType_FLOAT && layout->values[i].type == AttachedValueType_INTEGER)
                return layout->values[i].value;

            break; // type missmatch
        }
        return layout->values[i].value;
    }
    // logger_log("layout_get_attached_value() value not found: %s", name);
    return default_value;
}

AttachedValueType layout_get_attached_value2(Layout layout, const char* name, void* result_ptr) {
    for (int32_t i = 0; i < layout->values_size; i++) {
        if (!string_equals(layout->values[i].name, name)) continue;

        LayoutAttachedValue* result = result_ptr;

        *result = layout->values[i].value;
        return layout->values[i].type;
    }

    return AttachedValueType_NOTFOUND;
}

float layout_get_attached_value_as_float(Layout layout, const char* name, float default_value) {
    const AttachedValueType LIKE_NUMBER = AttachedValueType_INTEGER | AttachedValueType_FLOAT | AttachedValueType_HEX;

    for (int32_t i = 0; i < layout->values_size; i++) {
        if (!string_equals(layout->values[i].name, name)) continue;

        if ((layout->values[i].type & LIKE_NUMBER) == 0x00) break; // type missmatch

        // convert the value to float (IEEE 754)
        switch (layout->values[i].type) {
            case AttachedValueType_INTEGER:
                return (float)layout->values[i].value.as_long;
            case AttachedValueType_FLOAT:
                return (float)layout->values[i].value.as_double;
            case AttachedValueType_HEX:
                return (float)layout->values[i].value.as_unsigned;
            case AttachedValueType_BOOLEAN:
            case AttachedValueType_NOTFOUND:
            case AttachedValueType_STRING:
                break;
        }
    }
    return default_value;
}

AttachedValueType layout_get_attached_value_type(Layout layout, const char* name) {
    for (int32_t i = 0; i < layout->values_size; i++) {
        if (string_equals(layout->values[i].name, name)) return layout->values[i].type;
    }
    return AttachedValueType_NOTFOUND;
}

Modifier* layout_get_modifier_viewport(Layout layout) {
    return &layout->modifier_viewport;
}

Modifier* layout_get_modifier_camera(Layout layout) {
    return &layout->modifier_camera;
}

void layout_get_viewport_size(Layout layout, float* viewport_width, float* viewport_height) {
    *viewport_width = layout->viewport_width;
    *viewport_height = layout->viewport_height;
}

void layout_external_vertex_create_entries(Layout layout, int32_t amount) {
    if (amount < 0) amount = 0;

    layout->external_vertex_list = realloc_for_array(layout->external_vertex_list, amount, Item);

    for (int32_t i = layout->external_vertex_list_size; i < amount; i++) {
        Item item = (Item){
            .vertex = NULL,
            .type = -1,
            .group_id = 0, // layout root
            .parallax = (LayoutParallax){.x = 1.0f, .y = 1.0f, .z = 1.0f},
            .static_camera = false,
            .placeholder = NULL
        };
        layout->external_vertex_list[i] = item;
    }
    layout->external_vertex_list_size = amount;

    layout->z_buffer_size = layout->vertex_list_size + layout->external_vertex_list_size;
    layout->z_buffer = realloc_for_array(layout->z_buffer, layout->z_buffer_size, ZBufferEntry);

    // re-build z-buffer
    layout_helper_zbuffer_build(layout);
}

bool layout_external_vertex_set_entry(Layout layout, int32_t index, PVRVertex vertex_type, void* vertex, int32_t group_id) {
    if (index < 0 || index >= layout->external_vertex_list_size) return false;
    if (group_id < 0 || group_id >= layout->group_list_size) group_id = 0;
    layout->external_vertex_list[index].vertex.ptr = vertex;
    layout->external_vertex_list[index].type = vertex_type;
    layout->external_vertex_list[index].group_id = group_id;
    return true;
}

int32_t layout_external_create_group(Layout layout, const char* group_name, int32_t parent_group_id) {
    if (group_name != NULL) {
        int32_t index = layout_helper_get_group_index(layout, group_name);
        if (index >= 0) {
            logger_warn("layout_external_create_group() the group '%s' already exists", group_name);
            return -1;
        }
    }

    if (parent_group_id < 0 || parent_group_id >= layout->group_list_size) {
        // layout root
        parent_group_id = 0;
    }

    // increase group_list size
    int32_t group_id = layout->group_list_size++;
    layout->group_list = realloc_for_array(layout->group_list, layout->group_list_size, Group*);

    // SH4Matrix needs to be at least 8-byte aligned to avoid performance drops
    layout->group_list[group_id] = memalign_chk(32, sizeof(Group));
    malloc_assert(layout->group_list[group_id], Group);

    *layout->group_list[group_id] = (Group){
        .context = (GroupContext){
            .matrix = {},
            .visible = true,
            .alpha = 1.0f,
            .antialiasing = PVRCTX_FLAG_DEFAULT,
            .offsetcolor = {},
            .parallax = {.x = 1.0f, .y = 1.0f, .z = 1.0f},

            .next_child = NULL,
            .next_sibling = NULL,
            .parent_group = NULL,
            .last_z_index = -1,
        },

        .name = string_duplicate(group_name),
        .group_id = parent_group_id,
        .actions = NULL,
        .actions_size = 0,
        .initial_action_name = NULL,
        .antialiasing = PVRCTX_FLAG_DEFAULT,

        .visible = true,
        .alpha = 1.0f,
        .alpha2 = 1.0f,
        .offsetcolor = {},
        .modifier = {},
        .parallax = {.x = 1.0f, .y = 1.0f, .z = 1.0f},
        .static_camera = false,
        .static_screen = NULL,

        .animation = NULL,

        //.psshader = NULL,
        //.psframebuffer = NULL,

        .blend_enabled = true, // obligatory
        .blend_src_rgb = BLEND_DEFAULT,
        .blend_dst_rgb = BLEND_DEFAULT,
        .blend_src_alpha = BLEND_DEFAULT,
        .blend_dst_alpha = BLEND_DEFAULT,

        .viewport_x = -1.0f,
        .viewport_y = -1.0f,
        .viewport_width = -1.0f,
        .viewport_height = -1.0f,
    };

    // append to parent group
    layout_helper_add_group_to_parent(
        &layout->group_list[parent_group_id]->context, layout->group_list[group_id]
    );

    // sh4matrix_reset(&layout->group_list[group_id]->matrix);
    pvr_context_helper_clear_modifier(&layout->group_list[group_id]->modifier);
    pvr_context_helper_clear_offsetcolor(layout->group_list[group_id]->offsetcolor);

    return group_id;
}

bool layout_set_group_static_to_camera(Layout layout, const char* group_name, bool enable) {
    int32_t index = layout_helper_get_group_index(layout, group_name);
    if (index < 0) return false;
    layout->group_list[index]->static_camera = enable;
    return true;
}

bool layout_set_group_static_to_camera_by_id(Layout layout, int32_t group_id, bool enable) {
    if (group_id < 0 || group_id >= layout->group_list_size) return false;
    layout->group_list[group_id]->static_camera = enable;
    return true;
}

bool layout_set_group_static_to_screen_by_id(Layout layout, int32_t group_id, SH4Matrix sh4matrix) {
    if (group_id < 0 || group_id >= layout->group_list_size) return false;
    layout->group_list[group_id]->static_screen = sh4matrix;
    return true;
}

bool layout_external_vertex_set_entry_static(Layout layout, int32_t vertex_index, bool enable) {
    if (vertex_index < 0 || vertex_index >= layout->external_vertex_list_size) return false;
    layout->external_vertex_list[vertex_index].static_camera = enable;
    return true;
}

void layout_set_group_visibility(Layout layout, const char* group_name, bool visible) {
    int32_t index = layout_helper_get_group_index(layout, group_name);
    if (index >= 0) layout->group_list[index]->visible = visible;
}

void layout_set_group_visibility_by_id(Layout layout, int32_t group_id, bool visible) {
    if (group_id < 0 || group_id >= layout->group_list_size) return;
    layout->group_list[group_id]->visible = visible;
}

void layout_set_group_alpha(Layout layout, const char* group_name, float alpha) {
    if (math2d_is_float_NaN(alpha)) alpha = 0.0f;
    int32_t index = layout_helper_get_group_index(layout, group_name);
    if (index >= 0) layout->group_list[index]->alpha = math2d_clamp_float(alpha, 0.0f, 1.0f);
}

void layout_set_group_alpha_by_id(Layout layout, int32_t group_id, float alpha) {
    if (math2d_is_float_NaN(alpha)) alpha = 0.0f;
    if (group_id < 0 || group_id >= layout->group_list_size) return;
    layout->group_list[group_id]->alpha = math2d_clamp_float(alpha, 0.0f, 1.0f);
}

void layout_set_group_antialiasing(Layout layout, const char* group_name, PVRFlag antialiasing) {
    int32_t index = layout_helper_get_group_index(layout, group_name);
    if (index >= 0) layout->group_list[index]->antialiasing = antialiasing;
}

void layout_set_group_antialiasing_by_id(Layout layout, int32_t group_id, PVRFlag antialiasing) {
    if (group_id < 0 || group_id >= layout->group_list_size) return;
    layout->group_list[group_id]->antialiasing = antialiasing;
}

void layout_set_group_offsetcolor(Layout layout, const char* group_name, float r, float g, float b, float a) {
    int32_t index = layout_helper_get_group_index(layout, group_name);
    if (index >= 0) {
        if (!math2d_is_float_NaN(r)) layout->group_list[index]->offsetcolor[0] = math2d_clamp_float(r, 0.0f, 1.0f);
        if (!math2d_is_float_NaN(g)) layout->group_list[index]->offsetcolor[1] = math2d_clamp_float(g, 0.0f, 1.0f);
        if (!math2d_is_float_NaN(b)) layout->group_list[index]->offsetcolor[2] = math2d_clamp_float(b, 0.0f, 1.0f);
        if (!math2d_is_float_NaN(a)) layout->group_list[index]->offsetcolor[3] = math2d_clamp_float(a, 0.0f, 1.0f);
    }
}

Modifier* layout_get_group_modifier(Layout layout, const char* group_name) {
    int32_t index = layout_helper_get_group_index(layout, group_name);
    return index < 0 ? NULL : &layout->group_list[index]->modifier;
}

Modifier* layout_get_group_modifier_by_id(Layout layout, int32_t group_id) {
    if (group_id < 0 || group_id >= layout->group_list_size) return NULL;
    return &layout->group_list[group_id]->modifier;
}

int32_t layout_get_group_id(Layout layout, const char* group_name) {
    return layout_helper_get_group_index(layout, group_name);
}

bool layout_get_group_visibility(Layout layout, const char* group_name) {
    int32_t index = layout_helper_get_group_index(layout, group_name);
    if (index >= 0) return layout->group_list[index]->visible;
    return false;
}

bool layout_get_group_visibility_by_id(Layout layout, int32_t group_id) {
    if (group_id < 0 || group_id >= layout->group_list_size) return false;
    return layout->group_list[group_id]->visible;
}

const CameraPlaceholder* layout_get_camera_placeholder(Layout layout, const char* camera_name) {
    for (int32_t i = 0; i < layout->camera_list_size; i++) {
        if (string_equals(layout->camera_list[i].name, camera_name)) {
            return &layout->camera_list[i];
        }
    }
    return NULL;
}

bool layout_set_single_item_to_draw(Layout layout, const char* item_name) {
    layout->single_item = NULL;
    if (item_name == NULL) return true;
    for (int32_t i = 0; i < layout->vertex_list_size; i++) {
        const char* name;
        if (layout->vertex_list[i].type == VERTEX_DRAWABLE) {
            name = layout->vertex_list[i].placeholder->name;
        } else {
            name = layout->vertex_list[i].name;
        }
        if (string_equals(name, item_name)) {
            layout->single_item = &layout->vertex_list[i];
            return true;
        }
    }
    return false;
}

void layout_suspend(Layout layout) {
    for (int32_t i = 0; i < layout->sound_list_size; i++) {
        SoundPlayer soundplayer = layout->sound_list[i].soundplayer;
        layout->sound_list[i].was_playing = soundplayer_is_playing(soundplayer);
        if (layout->sound_list[i].was_playing) {
            soundplayer_pause(soundplayer);
            if (soundplayer_has_fading(soundplayer) == FADING_OUT) soundplayer_set_volume(soundplayer, 0.0f);
        }
    }
    for (int32_t i = 0; i < layout->video_list_size; i++) {
        VideoPlayer videoplayer = layout->video_list[i].videoplayer;
        layout->video_list[i].was_playing = videoplayer_is_playing(videoplayer);
        if (layout->video_list[i].was_playing) {
            videoplayer_pause(videoplayer);
            if (videoplayer_has_fading_audio(videoplayer) == FADING_OUT) videoplayer_set_volume(videoplayer, 0.0f);
        }
    }
    layout->suspended = true;
}

void layout_resume(Layout layout) {
    for (int32_t i = 0; i < layout->sound_list_size; i++) {
        SoundPlayer soundplayer = layout->sound_list[i].soundplayer;
        if (layout->sound_list[i].was_playing) soundplayer_play(soundplayer);
    }
    for (int32_t i = 0; i < layout->video_list_size; i++) {
        VideoPlayer videoplayer = layout->video_list[i].videoplayer;
        if (layout->video_list[i].was_playing) videoplayer_play(videoplayer);
    }
    layout->suspended = false;
}

void layout_disable_antialiasing(Layout layout, bool disable) {
    layout->antialiasing_disabled = disable;
}

bool layout_is_antialiasing_disabled(Layout layout) {
    return layout->antialiasing_disabled;
}

PVRFlag layout_get_layout_antialiasing(Layout layout) {
    return layout->group_list[0]->antialiasing;
}

void layout_set_layout_antialiasing(Layout layout, PVRFlag flag) {
    layout->group_list[0]->antialiasing = flag;
}

/*void layout_set_shader(Layout layout, PSShader psshader) {
    layout->psshader = psshader;
}

PSShader layout_get_group_shader(Layout layout, const char* group_name) {
    int32_t index = layout_helper_get_group_index(layout, group_name);
    if (index < 0) return NULL;

    return layout->group_list[index]->psshader;
}

bool layout_set_group_shader(Layout layout, const char* group_name, PSShader psshader) {
    int32_t index = layout_helper_get_group_index(layout, group_name);
    if (index < 0) return false;

    layout->group_list[index]->psshader = psshader;
    return true;
}*/


int32_t layout_animate(Layout layout, float elapsed) {
    if (layout->suspended) return 0;
    if (layout->animation_speed != 1.0f) elapsed *= layout->animation_speed;

    int32_t completed = 0;

    completed += layout_helper_check_trigger_queue(layout, elapsed);

    for (int32_t i = 0; i < layout->vertex_list_size; i++) {
        switch (layout->vertex_list[i].type) {
            case VERTEX_SPRITE:
                completed += sprite_animate(layout->vertex_list[i].vertex.as_sprite, elapsed);
                break;
            case VERTEX_TEXTSPRITE:
                completed += textsprite_animate(layout->vertex_list[i].vertex.as_textsprite, elapsed);
                break;
            case VERTEX_DRAWABLE:
                LayoutPlaceholder* placeholder = layout->vertex_list[i].placeholder;
                if (placeholder->vertex)
                    completed += drawable_animate(placeholder->vertex, elapsed);
                break;
                CASE_UNUSED(VERTEX_STATESPRITE)
                CASE_UNUSED(VERTEX_SHAPE)
                CASE_UNUSED(VERTEX_NONE)
        }
    }

    for (int32_t i = 0; i < layout->external_vertex_list_size; i++) {
        void* vertex = layout->external_vertex_list[i].vertex.ptr;
        if (!vertex) continue;

        switch (layout->external_vertex_list[i].type) {
            case VERTEX_SPRITE:
                completed += sprite_animate(vertex, elapsed);
                break;
            case VERTEX_TEXTSPRITE:
                completed += textsprite_animate(vertex, elapsed);
                break;
            case VERTEX_DRAWABLE:
                if (vertex)
                    completed += drawable_animate(vertex, elapsed);
                break;
                CASE_UNUSED(VERTEX_STATESPRITE)
                CASE_UNUSED(VERTEX_SHAPE)
                CASE_UNUSED(VERTEX_NONE)
        }
    }

    for (int32_t i = 0; i < layout->group_list_size; i++) {
        completed += layout_helper_group_animate(layout->group_list[i], elapsed);
    }

    for (int32_t i = 0; i < layout->fonts_size; i++) {
        FontHolder fontholder = layout->fonts[i].fontholder;
        if (fontholder->font_from_atlas) completed += fontglyph_animate(fontholder->font, elapsed);
    }

    if (camera_animate(layout->camera_helper, elapsed) < 1) {
        camera_apply(layout->camera_helper, NULL);
    }

    if (camera_animate(layout->camera_secondary_helper, elapsed) < 1) {
        camera_apply(layout->camera_secondary_helper, NULL);
    }

    return completed;
}

void layout_draw(Layout layout, PVRContext pvrctx) {
    SH4Matrix MATRIX_SCREEN;
    SH4Matrix MATRIX_VIEWPORT;

    pvr_context_save(pvrctx);
    // if (layout->psshader) pvr_context_add_shader(pvrctx, layout->psshader);

    if (layout->antialiasing_disabled) pvr_context_set_global_antialiasing(pvrctx, PVRCTX_FLAG_DISABLE);

    if (layout->resolution_changes != pvrctx->resolution_changes) {
        layout_update_render_size(layout, pvrctx->screen_width, pvrctx->screen_height);
        /*for (int32_t i = 0; i < layout->group_list_size; i++) {
            if (layout->group_list[i]->psframebuffer)
                layout->group_list[i]->psframebuffer.Resize();
        }*/

        layout->resolution_changes = pvrctx->resolution_changes;
    }

    // backup PVR screen matrix required for groups marked as "static_screen"
    sh4matrix_copy_to(pvrctx->current_matrix, MATRIX_SCREEN);

    // apply viewport modifier to PVR screen matrix and backup for elements marked as "static_camera"
    pvr_context_apply_modifier(pvrctx, &layout->modifier_viewport);
    sh4matrix_copy_to(pvrctx->current_matrix, MATRIX_VIEWPORT);

    // transform PVR screen matrix with secondary camera offset+focus
    camera_apply_offset(layout->camera_secondary_helper, pvrctx->current_matrix);
    pvr_context_apply_modifier(pvrctx, &layout->modifier_camera_secondary);

    // transform PVR screen matrix with primary camera offset. Note: the focus is used later as parallax
    camera_apply_offset(layout->camera_helper, pvrctx->current_matrix);

    // step 1: sort z_buffer
    for (int32_t i = 0; i < layout->z_buffer_size; i++) {
        Item* item = layout->z_buffer[i].item;
        void* vertex = layout->z_buffer[i].item->vertex.ptr;
        PVRVertex vertex_type = layout->z_buffer[i].item->type;

        switch (vertex_type) {
            case VERTEX_SPRITE:
                if (vertex) {
                    layout->z_buffer[i].z_index = sprite_get_z_index(vertex);
                    layout->z_buffer[i].visible = sprite_is_visible(vertex);
                    continue;
                }
                break;
            case VERTEX_TEXTSPRITE:
                if (vertex) {
                    layout->z_buffer[i].z_index = textsprite_get_z_index(vertex);
                    layout->z_buffer[i].visible = textsprite_is_visible(vertex);
                    continue;
                }
                break;
            case VERTEX_DRAWABLE:
                if (item->placeholder) vertex = item->placeholder->vertex;
                if (vertex) {
                    layout->z_buffer[i].z_index = drawable_get_z_index(vertex);
                    layout->z_buffer[i].visible = drawable_is_visible(vertex);
                    continue;
                }
                break;
                CASE_UNUSED(VERTEX_STATESPRITE)
                CASE_UNUSED(VERTEX_SHAPE)
                CASE_UNUSED(VERTEX_NONE)
        }

        // ignore
        layout->z_buffer[i].z_index = FLOAT_Inf;
        layout->z_buffer[i].visible = false;
    }
    qsort(layout->z_buffer, (size_t)layout->z_buffer_size, sizeof(ZBufferEntry), layout_helper_zbuffer_sort);

    // step 2: find top-most item of each group
    for (int32_t i = 0; i < layout->z_buffer_size; i++) {
        Group* group = layout->group_list[layout->z_buffer[i].item->group_id];
        group->context.last_z_index = i;
    }

    // step 3: build root group context
    Group* layout_root = layout->group_list[0];

    // sh4matrix_copy_to(layout_root->matrix, layout_root->context.matrix);
    sh4matrix_clear(layout_root->context.matrix);
    sh4matrix_apply_modifier(layout_root->context.matrix, &layout_root->modifier);

    layout_root->context.alpha = layout_root->alpha;
    layout_root->context.antialiasing = layout_root->antialiasing;
    layout_root->context.visible = layout_root->visible;
    layout_root->context.parallax.x = layout_root->parallax.x;
    layout_root->context.parallax.y = layout_root->parallax.y;
    layout_root->context.parallax.z = layout_root->parallax.z;
    for (int32_t i = 0; i < 4; i++) layout_root->context.offsetcolor[i] = layout_root->offsetcolor[i];

    // step 4: stack all groups
    layout_helper_stack_groups(layout_root);

    // step 5: draw all layout items
    bool has_single_item = layout->single_item != NULL;
    for (int32_t i = 0; i < layout->z_buffer_size; i++) {
        if (!layout->z_buffer[i].visible) continue;
        if (has_single_item && layout->z_buffer[i].item != layout->single_item) continue;

        void* vertex = layout->z_buffer[i].item->vertex.ptr;
        PVRVertex vertex_type = layout->z_buffer[i].item->type;
        Group* group = layout->group_list[layout->z_buffer[i].item->group_id];
        LayoutParallax* item_parallax = &layout->z_buffer[i].item->parallax;
        bool item_is_static_to_camera = layout->z_buffer[i].item->static_camera;

        if (layout->z_buffer[i].item->placeholder != NULL) {
            item_is_static_to_camera = layout->z_buffer[i].item->placeholder->static_camera;
            vertex = (void*)layout->z_buffer[i].item->placeholder->vertex;
            item_parallax = &layout->z_buffer[i].item->placeholder->parallax;
        }
        if (layout->z_buffer[i].item->videoplayer != NULL) {
            videoplayer_poll_streams(layout->z_buffer[i].item->videoplayer);
        }

        if (!group->context.visible) continue;

        pvr_context_save(pvrctx);

        /*// check whatever the current and/or parent group has framebuffer
        if (group->psframebuffer) {
            pvr_context_set_framebuffer(pvrctx, group->psframebuffer);
        } else {
            // use group and parent group shaders
            layout_helper_stack_groups_shaders(group, pvrctx);
            pvr_context_set_framebuffer(pvrctx, NULL);
        }*/

        float draw_x = 0.0f, draw_y = 0.0f;
        float* matrix = pvrctx->current_matrix;
        DelegateDraw draw_fn = NULL;

        // apply group context
        pvr_context_set_global_alpha(pvrctx, /*group->psframebuffer ? 1.0 : */ group->context.alpha);
        pvr_context_set_global_antialiasing(pvrctx, group->context.antialiasing);
        pvr_context_set_global_offsetcolor(pvrctx, group->context.offsetcolor);

        if (item_is_static_to_camera || group->static_camera || group->static_screen) {
            if (group->static_screen) {
                // restore backup for the current group marked as "static_screen"
                sh4matrix_copy_to(MATRIX_SCREEN, matrix);
                sh4matrix_multiply_with_matrix(matrix, group->static_screen);
            } else {
                // restore backup with viewport transform applied for elements marked as "static_camera"
                sh4matrix_copy_to(MATRIX_VIEWPORT, matrix);
            }

            // apply group context matrix
            sh4matrix_multiply_with_matrix(matrix, group->context.matrix);
            pvr_context_flush(pvrctx);

            switch (vertex_type) {
                case VERTEX_SPRITE:
                    sprite_draw(vertex, pvrctx);
                    break;
                case VERTEX_TEXTSPRITE:
                    textsprite_draw(vertex, pvrctx);
                    break;
                case VERTEX_DRAWABLE:
                    drawable_draw(vertex, pvrctx);
                    break;
                    CASE_UNUSED(VERTEX_STATESPRITE)
                    CASE_UNUSED(VERTEX_SHAPE)
                    CASE_UNUSED(VERTEX_NONE)
            }

            pvr_context_restore(pvrctx);
            continue;
        }

        // pick the draw location and location
        switch (vertex_type) {
            case VERTEX_SPRITE:
                sprite_get_draw_location(vertex, &draw_x, &draw_y);
                draw_fn = (DelegateDraw)sprite_draw;
                break;
            case VERTEX_TEXTSPRITE:
                textsprite_get_draw_location(vertex, &draw_x, &draw_y);
                draw_fn = (DelegateDraw)textsprite_draw;
                break;
            case VERTEX_DRAWABLE:
                //
                // There two posible ways to obtain the draw location:
                //       1. using the placeholder values
                //       2. from the drawable modifier
                //
                drawable_get_draw_location(vertex, &draw_x, &draw_y);
                draw_fn = (DelegateDraw)drawable_draw;
                break;
                CASE_UNUSED(VERTEX_STATESPRITE)
                CASE_UNUSED(VERTEX_SHAPE)
                CASE_UNUSED(VERTEX_NONE)
        }

        // apply group context matrix
        sh4matrix_multiply_with_matrix(matrix, group->context.matrix);

        float translate_x = layout->modifier_camera.translate_x;
        float translate_y = layout->modifier_camera.translate_y;
        float px = group->context.parallax.x * item_parallax->x;
        float py = group->context.parallax.y * item_parallax->y;
        float pz = group->context.parallax.z * item_parallax->z;

        // parallax z as percent of camera scale
        float scale_x = 1.0f - ((1.0f - layout->modifier_camera.scale_x) * pz);
        float scale_y = 1.0f - ((1.0f - layout->modifier_camera.scale_y) * pz);

        // parallax translation
        float tx = translate_x * px;
        float ty = translate_y * py;

        // camera translation+scale position correction
        tx += px * ((translate_x * scale_x) - translate_x);
        ty += py * ((translate_y * scale_y) - translate_y);

        // camera with parallax scale correction
        tx += (layout->viewport_width * (fabsf(scale_x) - 1.0f) * math2d_sign(scale_x)) / -2.0f;
        ty += (layout->viewport_height * (fabsf(scale_y) - 1.0f) * math2d_sign(scale_y)) / -2.0f;

        // apply translation (with all parallax corrections)
        sh4matrix_translate(matrix, tx, ty);

        // parallax scale
        sh4matrix_scale(matrix, scale_x, scale_y);

        pvr_context_flush(pvrctx);

        draw_fn(vertex, pvrctx);

        pvr_context_restore(pvrctx);

        /*// if the last item of the current group was drawn, flush the group framebuffer
        if (group->psframebuffer && group->context.last_z_index == i) {

            pvr_context_save(pvrctx);
            sh4matrix_copy_to(MATRIX_VIEWPORT, pvrctx->current_matrix);

            // draw group framebuffer
            pvr_context_set_framebuffer(pvrctx, NULL);

            // use group and parent group shaders
            layout_helper_stack_groups_shaders(group, pvrctx);

            pvr_context_set_vertex_blend(
                pvrctx, group->blend_enabled, group->blend_src_rgb, group->blend_dst_rgb, group->blend_src_alpha, group->blend_dst_alpha
            );

            // draw group framebuffer in the screen
            float x = group->viewport_x > 0.0f ? group->viewport_x : 0.0f;
            float y = group->viewport_y > 0.0f ? group->viewport_y : 0.0f;
            float width = group->viewport_width > 0.0f ? group->viewport_width : layout->viewport_width;
            float height = group->viewport_height > 0.0f ? group->viewport_height : layout->viewport_height;

            //pvr_context_apply_modifier(pvrctx, layout->modifier_viewport);
            pvr_context_set_vertex_alpha(pvrctx, group->context.alpha);

            float sx = x * layout->modifier_viewport.scale_x;
            float sy = y * layout->modifier_viewport.scale_y;
            float sw = width * layout->modifier_viewport.scale_x;
            float sh = height * layout->modifier_viewport.scale_y;

            pvr_context_draw_framebuffer(pvrctx, group->psframebuffer, sx, sy, sw, sh, x, y, width, height);

            pvr_context_restore(pvrctx);
            group->psframebuffer.Invalidate();
        }*/
    }

    pvr_context_restore(pvrctx);
}



//////////////////////////////////
///          HELPERS           ///
//////////////////////////////////

static void layout_helper_destroy_actions(Action* actions, int32_t actions_size) {
    for (int32_t i = 0; i < actions_size; i++) {
        Action* action = &actions[i];

        for (int32_t j = 0; j < action->entries_size; j++) {
            switch (action->entries[j].type) {
                case LAYOUT_ACTION_ANIMATION:
                    if (action->entries[j].misc.animsprite) animsprite_destroy(&action->entries[j].misc.animsprite);
                    break;
                case LAYOUT_ACTION_ATLASAPPLY:
                    free_chk(action->entries[j].misc.atlas_entry);
                    break;
                case LAYOUT_ACTION_PROPERTY:
                    if (action->entries[j].property == TEXTSPRITE_PROP_STRING)
                        free_chk(action->entries[j].misc.string);
                    break;
                case LAYOUT_ACTION_SETSHADER:
                    // action->entries[j].misc.shader.Destroy();
                    break;
                case LAYOUT_ACTION_SETSHADERUNIFORM:
                    // free_chk(action->entries[j].uniform_name);
                    // free_chk(action->entries[j].misc.uniform_values);
                    break;
            }
        }
        free_chk(action->entries);
        free_chk(action->name);
    }
    free_chk(actions);
}

static void* layout_helper_get_resource(ResourcePool* resource_pool, const char* src, bool is_texture) {
    char* path = fs_get_full_path_and_override(src);
    ArrayList pool = is_texture ? resource_pool->textures : resource_pool->atlas;

    foreach (ResourcePoolEntry*, definition, ARRAYLIST_ITERATOR, pool) {
        if (string_equals(definition->path, path)) {
            free_chk(path);
            return is_texture ? (void*)definition->as_texture : (void*)definition->as_atlas;
        }
    }

    // resource not found in the pool load it
    void* data;

    if (is_texture)
        data = texture_init_deferred(path, true /* do not upload to the PVR VRAM */);
    else
        data = atlas_init(path);

    if (!data) {
        logger_warn("layout_helper_get_resource() missing resource '%s' (%s)", path, src);
    }

    if (data) {
        ResourcePoolEntry new_definition = (ResourcePoolEntry){.path = path, .is_texture = is_texture};
        if (is_texture)
            new_definition.as_texture = data;
        else
            new_definition.as_atlas = data;

        arraylist_add(pool, &new_definition);
    }

    return data;
}

static void layout_helper_location(ActionEntry* action_entry, float width, float height, float v_width, float v_height, float* location_x, float* location_y) {
    const Align align_vertical = action_entry->align_vertical;
    const Align align_horizontal = action_entry->align_horizontal;

    float offset_x = 0.0f, offset_y = 0.0f;
    // Note: align center means relative to the viewport

    // vertical align
    switch (align_vertical) {
        case ALIGN_START:
            offset_y = 0.0f;
            break;
        case ALIGN_CENTER:
            offset_y = (v_height - height) / 2.0f;
            break;
        case ALIGN_END:
            offset_y = v_height - height;
            break;
            CASE_UNUSED(ALIGN_BOTH)
            CASE_UNUSED(ALIGN_NONE)
            CASE_UNUSED(ALIGN_INVALID)
    }

    // horizontal align
    switch (align_horizontal) {
        case ALIGN_START:
            offset_x = 0.0f;
            break;
        case ALIGN_CENTER:
            offset_x = (v_width - width) / 2.0f;
            break;
        case ALIGN_END:
            offset_x = v_width - width;
            break;
            CASE_UNUSED(ALIGN_BOTH)
            CASE_UNUSED(ALIGN_NONE)
            CASE_UNUSED(ALIGN_INVALID)
    }

    *location_x = offset_x + action_entry->x;
    *location_y = offset_y + action_entry->y;
}

static uint32_t layout_helper_parse_hex(XmlNode node, const char* attr_name, uint32_t def_value) {
    uint32_t value = 0x0000;
    if (!vertexprops_parse_hex(xmlparser_get_attribute_value2(node, attr_name), &value, false)) {
        logger_error_xml("layout_helper_parse_hex() invalid value of '%s': ", node, attr_name);
        return def_value;
    }

    return value;
}

static float layout_helper_parse_float(XmlNode node, const char* attr_name, float def_value) {
    if (!xmlparser_get_attribute_value2(node, attr_name)) return def_value;

    float value = vertexprops_parse_float(node, attr_name, FLOAT_NaN);

    if (math2d_is_float_NaN(value)) {
        logger_error("layout_parse_float(): invalid value: %s", xmlparser_get_attribute_value2(node, attr_name));
        return def_value;
    }

    return value;
}

static Align layout_helper_parse_align(XmlNode node, bool is_vertical) {
    const char* attribute = is_vertical ? "alignVertical" : "alignHorizontal";
    return vertexprops_parse_align(node, attribute, false, true);
}

static Align layout_helper_parse_align2(XmlNode node, bool is_vertical, Align def_value) {
    const char* attribute = is_vertical ? "alignVertical" : "alignHorizontal";
    if (!xmlparser_has_attribute(node, attribute)) return def_value;

    Align align = vertexprops_parse_align2(xmlparser_get_attribute_value2(node, attribute));
    if (align == ALIGN_BOTH) {
        align = ALIGN_START;
        logger_warn_xml("layout_helper_parse_align2() invalid align found at:", node);
    }

    return align;
}

static void layout_helper_parse_color(XmlNode node, RGBA rgba) {
    if (xmlparser_has_attribute(node, "color")) {
        //
        // Check if the color is rgba (0xRRGGBBAA format) or rgb (0xRRGGBB format)
        //
        const char* value = xmlparser_get_attribute_value2(node, "color");
        size_t length = strlen(value);
        if (string_starts_with(value, "0x") || string_starts_with(value, "0X")) length -= 2;

        uint32_t raw_value = layout_helper_parse_hex(node, "color", 0xFFFFFF);
        bool has_alpha = length == 8;
        math2d_color_bytes_to_floats(raw_value, has_alpha, rgba);
    } else if (xmlparser_has_attribute(node, "rgb")) {
        uint32_t rgb8_color = layout_helper_parse_hex(node, "rgb", 0xFFFFFF);
        math2d_color_bytes_to_floats(rgb8_color, false, rgba);
    } else if (xmlparser_has_attribute(node, "rgba")) {
        uint32_t rgba8_color = layout_helper_parse_hex(node, "rgba", 0xFFFFFFFF);
        math2d_color_bytes_to_floats(rgba8_color, true, rgba);
    } else if (xmlparser_has_attribute(node, "argb")) {
        uint32_t rgba8_color = layout_helper_parse_hex(node, "argb", 0xFFFFFFFF);
        math2d_color_bytes_to_floats(rgba8_color, true, rgba);
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
        rgba[0] = layout_helper_parse_float(node, "r", FLOAT_NaN);
        rgba[1] = layout_helper_parse_float(node, "g", FLOAT_NaN);
        rgba[2] = layout_helper_parse_float(node, "b", FLOAT_NaN);
        rgba[3] = layout_helper_parse_float(node, "a", FLOAT_NaN);
    }
}

static void* layout_helper_get_vertex(Layout layout, PVRVertex type, const char* name) {
    for (int32_t i = 0; i < layout->vertex_list_size; i++) {
        if (layout->vertex_list[i].videoplayer) continue;
        if (layout->vertex_list[i].type == type && string_equals(layout->vertex_list[i].name, name)) {
            return layout->vertex_list[i].vertex.ptr;
        }
    }
    return NULL;
}

static int32_t layout_helper_get_group_index(Layout layout, const char* name) {
    if (/*pointer equals*/ name == LAYOUT_GROUP_ROOT) {
        return 0;
    } else {
        int32_t size = layout->group_list_size;
        for (int32_t i = 1; i < size; i++) {
            if (string_equals(layout->group_list[i]->name, name)) {
                return i;
            }
        }
    }
    return -1;
}

static void layout_helper_execute_action(Layout layout, Item* item, Action* action) {
    switch (item->type) {
        case VERTEX_SPRITE:
            layout_helper_execute_action_in_sprite(
                action, item, layout->viewport_width, layout->viewport_height
            );
            break;
        case VERTEX_TEXTSPRITE:
            layout_helper_execute_action_in_textsprite(
                action, item, layout->viewport_width, layout->viewport_height
            );
            break;
            CASE_UNUSED(VERTEX_STATESPRITE)
            CASE_UNUSED(VERTEX_SHAPE)
            CASE_UNUSED(VERTEX_NONE)
            CASE_UNUSED(VERTEX_DRAWABLE)
    }
}

static void layout_helper_execute_trigger(Layout layout, Trigger* trigger) {
    trigger->context.running = true;
    trigger->context.progress_delay = 0.0;
    trigger->context.loop_waiting = false;
    trigger->context.loop_count = 0;

    if (trigger->start_delay > 0.0f) return;

    // there no start delay, commit now
    layout_helper_commit_trigger(layout, trigger);
}

static void layout_helper_set_parallax_info(LayoutParallax* parallax_info, ActionEntry* parallax_action) {
    if (!math2d_is_float_NaN(parallax_action->x)) parallax_info->x = parallax_action->x;
    if (!math2d_is_float_NaN(parallax_action->y)) parallax_info->y = parallax_action->y;
    if (!math2d_is_float_NaN(parallax_action->z)) parallax_info->z = parallax_action->z;
}

static void layout_helper_stack_groups(Group* parent_group) {
    bool parent_visible = parent_group->context.visible && parent_group->context.alpha > 0.0f;
    Group* group = parent_group->context.next_child;

    while (group) {
        float group_alpha = group->alpha * group->alpha2;
        group->context.visible = parent_visible && group->visible && group_alpha > 0.0f;
        group->context.parent_group = parent_group;

        if (group->context.visible) {
            // interpolate the parent context in the current context
            math2d_color_blend_normal(
                group->offsetcolor, parent_group->context.offsetcolor, group->context.offsetcolor
            );

            sh4matrix_copy_to(parent_group->context.matrix, group->context.matrix);
            sh4matrix_apply_modifier(group->context.matrix, &group->modifier);

            group->context.alpha = group_alpha * parent_group->context.alpha;

            if (group->antialiasing == PVRCTX_FLAG_DEFAULT)
                group->context.antialiasing = parent_group->context.antialiasing;
            else
                group->context.antialiasing = group->antialiasing;

            group->context.parallax.x = group->parallax.x * parent_group->context.parallax.x;
            group->context.parallax.y = group->parallax.y * parent_group->context.parallax.y;
            group->context.parallax.z = group->parallax.z * parent_group->context.parallax.z;
        }

        if (group->context.next_child) layout_helper_stack_groups(group);

        group = group->context.next_sibling;
    }
}

/*static void layout_helper_stack_groups_shaders(Group* group, PVRContext pvrctx) {
    // if the parent has framebuffer, stop going up
    while (group) {
        if (group->psshader) {
            if (!pvr_context_add_shader(pvrctx, group->psshader)) {
                // limit reached
                break;
            }
        }
        group = group->context.parent_group;
    }
}*/

static int32_t layout_helper_group_animate(Group* group, float elapsed) {
    if (!group->animation) return 1;

    int32_t completed = animsprite_animate(group->animation, elapsed);
    animsprite_update_using_callback(group->animation, group, (PropertySetter)layout_helper_group_set_property, true);

    return completed;
}

static void layout_helper_group_set_property(Group* group, int32_t property_id, float value) {
    switch (property_id) {
        case SPRITE_PROP_X:
            group->modifier.x = value;
            break;
        case SPRITE_PROP_Y:
            group->modifier.y = value;
            break;
        case SPRITE_PROP_WIDTH:
            group->modifier.width = value;
            break;
        case SPRITE_PROP_HEIGHT:
            group->modifier.height = value;
            break;
        case SPRITE_PROP_ALPHA:
            group->alpha = math2d_clamp_float(value, 0.0f, 1.0f);
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_R:
            group->offsetcolor[0] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_G:
            group->offsetcolor[1] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_B:
            group->offsetcolor[2] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_A:
            group->offsetcolor[3] = value;
            break;
        case SPRITE_PROP_ANTIALIASING:
            group->antialiasing = (PVRFlag)(int)value;
            break;
        case SPRITE_PROP_ALPHA2:
            group->alpha2 = value;
            break;
        default:
            pvr_context_helper_set_modifier_property(&group->modifier, property_id, value);
            break;
    }
}

static void layout_helper_parse_property(XmlNode unparsed_entry, int32_t property_id, const char* value_holder, ArrayList action_entries) {

    ActionEntry action_entry = (ActionEntry){
        .type = LAYOUT_ACTION_PROPERTY,
        .property = property_id,
        .value = FLOAT_NaN,
        .misc = {NULL},
    };

    if (vertexprops_is_property_boolean(property_id)) {
        nbool value = vertexprops_parse_boolean(unparsed_entry, value_holder, unset);
        if (value != unset) {
            action_entry.value = value ? 1.0f : 0.0f;
            arraylist_add(action_entries, &action_entry);
            return;
        }
    }
    switch (property_id) {
        case TEXTSPRITE_PROP_ALIGN_V:
        case TEXTSPRITE_PROP_ALIGN_H:
        case TEXTSPRITE_PROP_ALIGN_PARAGRAPH:
            action_entry.value = vertexprops_parse_align(unparsed_entry, value_holder, true, true);
            break;
        case TEXTSPRITE_PROP_FORCE_CASE:
            action_entry.value = vertexprops_parse_textsprite_forcecase(unparsed_entry, value_holder, true);
            break;
        case TEXTSPRITE_PROP_FONT_COLOR:
            action_entry.value = layout_helper_parse_hex(unparsed_entry, value_holder, 0xFFFFFF);
            break;
        case TEXTSPRITE_PROP_STRING:
            action_entry.misc.string = ATTR_STRDUP(unparsed_entry, value_holder);
            break;
        case MEDIA_PROP_PLAYBACK:
            action_entry.value = vertexprops_parse_playback(unparsed_entry, value_holder, true);
            break;
        case SPRITE_PROP_ANTIALIASING:
            action_entry.value = vertexprops_parse_flag(unparsed_entry, value_holder, PVRCTX_FLAG_DEFAULT);
            break;
        case FONT_PROP_WORDBREAK:
            action_entry.value = vertexprops_parse_wordbreak(unparsed_entry, value_holder, true);
            break;
        default:
            action_entry.value = layout_helper_parse_float(unparsed_entry, value_holder, 0.0f);
            break;
    }

    arraylist_add(action_entries, &action_entry);
}

static void layout_herper_parse_offsetmovefromto(XmlNode unparsed_offsetmovefromto, float* x, float* y, float* z) {
    *x = layout_helper_parse_float(unparsed_offsetmovefromto, "x", FLOAT_NaN);
    *y = layout_helper_parse_float(unparsed_offsetmovefromto, "y", FLOAT_NaN);
    *z = layout_helper_parse_float(unparsed_offsetmovefromto, "z", FLOAT_NaN);
}

static void layout_helper_zbuffer_build(Layout layout) {
    int32_t j = 0;
    for (int32_t i = 0; i < layout->z_buffer_size; i++) {
        layout->z_buffer[i] = (ZBufferEntry){.item = NULL, .visible = false, .z_index = 0.0f};
    }
    for (int32_t i = 0; i < layout->vertex_list_size; i++) {
        layout->z_buffer[j++].item = &layout->vertex_list[i];
    }
    for (int32_t i = 0; i < layout->external_vertex_list_size; i++) {
        layout->z_buffer[j++].item = &layout->external_vertex_list[i];
    }
}

static int layout_helper_zbuffer_sort(const void* entry1_ptr, const void* entry2_ptr) {
    ZBufferEntry* entry1 = (ZBufferEntry*)entry1_ptr;
    ZBufferEntry* entry2 = (ZBufferEntry*)entry2_ptr;

    return math2d_float_comparer(entry1->z_index, entry2->z_index);
}

static void layout_helper_add_group_to_parent(GroupContext* parent_context, Group* group) {
    if (parent_context->next_child) {
        Group* sibling_group = parent_context->next_child;
        while (true) {
            if (!sibling_group->context.next_sibling) break;
            sibling_group = sibling_group->context.next_sibling;
        }
        sibling_group->context.next_sibling = group;
    } else {
        parent_context->next_child = group;
    }
}

static int32_t layout_helper_check_trigger_queue(Layout layout, float elapsed) {
    int32_t completed_count = 0;

    // complex timestamp based checker
    if (layout->beatwatcher_synced_triggers) {
        bool do_sync = false;
        bool has_beat = beatwatcher_poll(&layout->beatwatcher);

        // check if the beatwatcher was resynchronized
        if (layout->beatwatcher.resyncs > 0) {
            // snapshot current beatwatcher timestamp
            if (layout->beatwatcher_resyncs_count != layout->beatwatcher.resyncs) {
                layout->beatwatcher_resyncs_count = layout->beatwatcher.resyncs;
                layout->beatwatcher_last_timestamp = layout->beatwatcher.last_global_timestamp;
                return completed_count;
            }

            // wait for a beat, this keep harmonic all beat-dependent animations (in theory)
            if (!has_beat) return 0;

            do_sync = true;
            layout->beatwatcher.resyncs = layout->beatwatcher_resyncs_count = 0;
        }

        // calc time elasped since last layout_helper_check_trigger_queue() call
        elapsed = (float)(layout->beatwatcher.last_global_timestamp - layout->beatwatcher_last_timestamp);
        layout->beatwatcher_last_timestamp = layout->beatwatcher.last_global_timestamp;

        if (do_sync) {
            // resync all trigger progress and check later the queue
            for (int32_t i = 0; i < layout->trigger_list_size; i++) {
                layout->trigger_list[i].context.progress_delay = 0.0;
            }
        }

        for (int32_t i = 0; i < layout->trigger_list_size; i++) {
            bool trigger_is_completed = false;
            Trigger* trigger = &layout->trigger_list[i];

            if (!trigger->context.running) {
                completed_count++;
                continue;
            }

            bool commit = false;
            trigger->context.progress_delay += elapsed;

            while (true) {
                float target_delay;
                if (trigger->context.loop_waiting) {
                    target_delay = trigger->loop_delay;
                } else {
                    target_delay = trigger->start_delay;
                }

                if (trigger->context.progress_delay >= target_delay) {
                    trigger->context.progress_delay -= target_delay; // keep in sync
                    commit = true;
                    trigger->context.loop_waiting = true;
                    trigger->context.loop_count++;
                } else {
                    break;
                }

                if (trigger->loop < 1) {
                    trigger_is_completed = true;
                    break;
                }
                if (trigger->context.loop_count >= trigger->loop) {
                    trigger->context.running = false;
                    trigger_is_completed = true;
                    break;
                }
            }

            if (trigger_is_completed) trigger->context.progress_delay = 0.0;

            if (commit) {
                layout_helper_commit_trigger(layout, trigger);
            }
        }

        return completed_count;
    }

    // classic way
    for (int32_t i = 0; i < layout->trigger_list_size; i++) {
        Trigger* trigger = &layout->trigger_list[i];

        if (!trigger->context.running) {
            completed_count++;
            continue;
        }

        float target_delay;

        if (trigger->context.loop_waiting) {
            target_delay = trigger->loop_delay;
        } else {
            target_delay = trigger->start_delay;
        }

        if (trigger->context.progress_delay >= target_delay) {
            trigger->context.progress_delay -= target_delay; // keep in sync
            layout_helper_commit_trigger(layout, trigger);
        }

        trigger->context.progress_delay += elapsed;
    }

    return completed_count;
}

static void layout_helper_commit_trigger(Layout layout, Trigger* trigger) {
    // increase the number of loops (or queue for the next loop) and check if still should run
    trigger->context.loop_count++;
    trigger->context.loop_waiting = true;
    trigger->context.running = trigger->loop < 1 || trigger->context.loop_count < trigger->loop;

    if (trigger->action_name != NULL && layout_trigger_action(layout, NULL, trigger->action_name) < 1) {
        logger_warn("layout_helper_commit_trigger() no actions with name: %s", trigger->action_name);
    }
    if (trigger->camera_name != NULL && !layout_trigger_camera(layout, trigger->camera_name)) {
        logger_warn("layout_helper_commit_trigger() no camera with name: %s", trigger->camera_name);
    }
    if (trigger->stop_trigger_name != NULL) {
        layout_stop_trigger(layout, trigger->stop_trigger_name);
    }
    if (trigger->trigger_name == NULL) {
        return;
    }

    trigger->context.reject_recursive = true; // avoid self-trigger

    for (int32_t i = 0; i < layout->trigger_list_size; i++) {
        Trigger* trigger2 = &layout->trigger_list[i];
        if (!string_equals(trigger2->name, trigger->trigger_name)) continue;

        if (!trigger2->context.reject_recursive) {
            layout_helper_execute_trigger(layout, trigger2);
            continue;
        }

        logger_warn("layout_helper_commit_trigger() self-trigger avoided: %s", trigger->action_name);
    }

    trigger->context.reject_recursive = false;
    return;
}

/*static void layout_helper_set_shader_uniform(PSShader psshader, ActionEntry* action_entry) {
    if (!psshader) {
        logger_warn("layout_helper_set_shader_uniform() can not set %s, there no shader", action_entry->uniform_name);
        return;
    }

    int32_t ret = psshader.SetUniformAny(action_entry->uniform_name, action_entry->misc.uniform_values);

    switch (ret) {
        case 0:
            console_warn("layout_helper_set_shader_uniform() the shader does not have %s", action_entry->uniform_name);
            break;
        case -1:
            console_warn("layout_helper_set_shader_uniform() type of %s is not supported", action_entry->uniform_name);
            break;
        case -2:
            console_error("layout_helper_set_shader_uniform() bad setter for %s", action_entry->uniform_name);
            break;
    }
}*/


//////////////////////////////////
///        VERTEX PARSER       ///
//////////////////////////////////

static void layout_parse_placeholder(XmlNode unparsed_plchdlr, LayoutContext* layout_context, int32_t group_id) {
    const char* name = xmlparser_get_attribute_value2(unparsed_plchdlr, "name");
    if (!name) {
        logger_warn_xml("Missing placeholder name: ", unparsed_plchdlr);
        return;
    }

    LayoutPlaceholder* placeholder = malloc_chk(sizeof(LayoutPlaceholder));
    malloc_assert(placeholder, LayoutPlaceholder);

    *placeholder = (LayoutPlaceholder){
        .type = VERTEX_DRAWABLE,
        .group_id = group_id,

        .name = string_duplicate(name),

        .align_vertical = layout_helper_parse_align2(unparsed_plchdlr, true, ALIGN_NONE),
        .align_horizontal = layout_helper_parse_align2(unparsed_plchdlr, false, ALIGN_NONE),

        .x = layout_helper_parse_float(unparsed_plchdlr, "x", 0.0f),
        .y = layout_helper_parse_float(unparsed_plchdlr, "y", 0.0f),
        .z = layout_helper_parse_float(unparsed_plchdlr, "z", 0.0f),
        .height = layout_helper_parse_float(unparsed_plchdlr, "height", -1.0f),
        .width = layout_helper_parse_float(unparsed_plchdlr, "width", -1.0f),

        .parallax = {.x = 1.0f, .y = 1.0f, .z = 1.0f},
        .static_camera = vertexprops_parse_boolean(unparsed_plchdlr, "static", false),

        .vertex = NULL
    };

    float x = 0.0f, y = 0.0f, z = 0.0f;

    XmlNode unparsed_parallax = xmlparser_get_first_children(unparsed_plchdlr, "Parallax");
    if (unparsed_parallax) {
        layout_herper_parse_offsetmovefromto(unparsed_parallax, &x, &y, &z);
        if (!math2d_is_float_NaN(x)) placeholder->parallax.x = x;
        if (!math2d_is_float_NaN(y)) placeholder->parallax.y = y;
        if (!math2d_is_float_NaN(z)) placeholder->parallax.z = z;
    }

    XmlNode unparsed_location = xmlparser_get_first_children(unparsed_plchdlr, "Location");
    if (unparsed_location) {
        layout_herper_parse_offsetmovefromto(unparsed_location, &x, &y, &z);
        if (!math2d_is_float_NaN(x)) placeholder->x = x;
        if (!math2d_is_float_NaN(y)) placeholder->y = y;
        if (!math2d_is_float_NaN(z)) placeholder->z = z;
        placeholder->align_vertical = layout_helper_parse_align2(
            unparsed_location, true, placeholder->align_vertical
        );
        placeholder->align_horizontal = layout_helper_parse_align2(
            unparsed_location, false, placeholder->align_horizontal
        );
    }

    XmlNode unparsed_size = xmlparser_get_first_children(unparsed_plchdlr, "Size");
    if (unparsed_size) {
        placeholder->width = layout_helper_parse_float(unparsed_size, "width", placeholder->width);
        placeholder->height = layout_helper_parse_float(unparsed_size, "height", placeholder->height);
    }

    XmlNode unparsed_static = xmlparser_get_first_children(unparsed_plchdlr, "Static");
    if (unparsed_static) {
        placeholder->static_camera = vertexprops_parse_boolean(
            unparsed_static, "enable", true
        );
    }

    Item item = (Item){
        .name = NULL, // STUB, the placeholder field contains the name
        .actions_size = 0,
        .type = VERTEX_DRAWABLE,
        .group_id = group_id,
        .placeholder = placeholder,
        .parallax = {}, // STUB, the placeholder field contains the parallax
        .initial_action_name = NULL
    };
    arraylist_add(layout_context->vertex_list, &item);
}

static void layout_parse_sprite(XmlNode unparsed_sprite, LayoutContext* layout_context, int32_t group_id) {
    const char* texture_filename = xmlparser_get_attribute_value2(unparsed_sprite, "texture");
    const char* atlas_filename = xmlparser_get_attribute_value2(unparsed_sprite, "atlas");
    Sprite sprite;

    Atlas atlas;
    char* atlas_texture_path = NULL;
    if (atlas_filename) {
        atlas = layout_helper_get_resource(&layout_context->resource_pool, atlas_filename, false);
        if (atlas && !texture_filename) {
            atlas_texture_path = string_duplicate(atlas_get_texture_path(atlas));
            if (!fs_file_exists(atlas_texture_path)) {
                // the imagePath attribute has an invalid filename
                logger_warn("layout_parse_sprite() texture pointed by imagePath='%s' not found in atlas '%s'", atlas_texture_path, atlas_filename);
                char* temp_value = fs_get_filename_without_extension(atlas_filename);
                char* temp_texture_filename = string_concat(2, temp_value, ".png");
                atlas_texture_path = fs_build_path2(atlas_filename, temp_texture_filename);
                free_chk(temp_value);
                free_chk(temp_texture_filename);
            }
        }
    } else {
        atlas = NULL;
    }

    if (texture_filename || atlas_texture_path) {
        const char* src = texture_filename ? texture_filename : atlas_texture_path;
        Texture texture = layout_helper_get_resource(&layout_context->resource_pool, src, true);
        sprite = sprite_init(texture);
        if (!atlas) sprite_set_draw_size_from_source_size(sprite);
    } else {
        RGBA rgba = {0.0f, 0.0f, 0.0f, FLOAT_NaN};
        layout_helper_parse_color(unparsed_sprite, rgba);

        sprite = sprite_init_from_rgb8(0xFFFFFF);

        sprite_set_vertex_color(sprite, rgba[0], rgba[1], rgba[2]);
        if (!math2d_is_float_NaN(rgba[3])) sprite_set_alpha(sprite, rgba[3]);
    }

    free_chk(atlas_texture_path);

    Item item = (Item){
        .type = VERTEX_SPRITE,
        .name = ATTR_STRDUP(unparsed_sprite, "name"),
        .vertex = sprite,
        .group_id = group_id,
        .actions_size = -1,
        .actions = NULL,
        .initial_action_name = ATTR_STRDUP(unparsed_sprite, "initialAction"),
        .parallax = (LayoutParallax){.x = 1.0f, .y = 1.0f, .z = 1.0f},
        .static_camera = false,
        .animation = NULL,
        .placeholder = NULL
    };

    ArrayList actions_arraylist = arraylist_init2(sizeof(Action), xmlparser_get_children_count(unparsed_sprite));
    foreach (XmlNode, action, XMLPARSER_CHILDREN_ITERATOR, unparsed_sprite) {
        if (string_equals(xmlparser_get_tag_name(action), "Action")) {
            layout_parse_sprite_action(action, layout_context->animlist, atlas, actions_arraylist, false);
        }
    }
    arraylist_destroy2(&actions_arraylist, &item.actions_size, (void**)&item.actions);

    arraylist_add(layout_context->vertex_list, &item);
}

static void layout_parse_text(XmlNode unparsed_text, LayoutContext* layout_context, int32_t group_id) {
    FontHolder fontholder = NULL;
    const char* font_name = xmlparser_get_attribute_value2(unparsed_text, "fontName");
    float font_size = layout_helper_parse_float(unparsed_text, "fontSize", 12.0f);
    RGBA font_color = {1.0f, 1.0f, 1.0f, FLOAT_NaN};

    layout_helper_parse_color(unparsed_text, font_color);

    for (int32_t i = 0; i < layout_context->fonts_size; i++) {
        if (string_equals(layout_context->fonts[i].name, font_name)) {
            fontholder = layout_context->fonts[i].fontholder;
            break;
        }
    }

    if (!fontholder) {
        logger_error("layout_parse_text() the font '%s' is not attached", font_name);
        return;
    }

    Item item = (Item){
        .name = ATTR_STRDUP(unparsed_text, "name"),
        .type = VERTEX_TEXTSPRITE,
        .vertex.as_textsprite = textsprite_init2(fontholder, font_size, 0x000000),
        .actions_size = -1,
        .actions = NULL,
        .group_id = group_id,
        .initial_action_name = ATTR_STRDUP(unparsed_text, "initialAction"),
        .parallax = {.x = 1.0f, .y = 1.0f, .z = 1.0f},
        .static_camera = false,
        .animation = NULL,
        .placeholder = NULL
    };

    textsprite_set_color(item.vertex.as_textsprite, font_color[0], font_color[1], font_color[2]);
    if (!math2d_is_float_NaN(font_color[3])) textsprite_set_alpha(item.vertex.as_textsprite, font_color[3]);

    ArrayList actions_arraylist = arraylist_init2(sizeof(Action), xmlparser_get_children_count(unparsed_text));
    foreach (XmlNode, action, XMLPARSER_CHILDREN_ITERATOR, unparsed_text) {
        if (string_equals(xmlparser_get_tag_name(action), "Action")) {
            layout_parse_text_action(action, layout_context->animlist, actions_arraylist);
        }
    }
    arraylist_destroy2(&actions_arraylist, &item.actions_size, (void**)&item.actions);

    arraylist_add(layout_context->vertex_list, &item);
}

static void layout_parse_group(XmlNode unparsed_group, LayoutContext* layout_context, GroupContext* parent_context) {
    ArrayList actions_arraylist = arraylist_init2(sizeof(Action), xmlparser_get_children_count(unparsed_group));

    // SH4Matrix needs to be at least 8-byte aligned to avoid performance drops
    Group* group = memalign_chk(32, sizeof(Group));
    malloc_assert(group, Group);

    *group = (Group){
        .context = {
            .matrix = {},
            .visible = true,
            .alpha = 1.0f,
            .antialiasing = PVRCTX_FLAG_DEFAULT,
            .offsetcolor = {},
            .parallax = {.x = 1.0f, .y = 1.0f, .z = 1.0f},
            .last_z_index = -1,

            .next_child = NULL,
            .next_sibling = NULL,
            .parent_group = NULL
        },

        .name = parent_context ? ATTR_STRDUP(unparsed_group, "name") : (char*)LAYOUT_GROUP_ROOT,
        .group_id = arraypointerlist_size(layout_context->group_list),
        .actions = NULL,
        .actions_size = -1,
        .initial_action_name = ATTR_STRDUP(unparsed_group, "initialAction"),

        .visible = vertexprops_parse_boolean(unparsed_group, "visible", true),
        .alpha = layout_helper_parse_float(unparsed_group, "alpha", 1.0f),
        .alpha2 = 1.0f,
        .antialiasing = vertexprops_parse_flag(unparsed_group, "antialiasing", PVRCTX_FLAG_DEFAULT),
        .offsetcolor = {},
        .modifier = {},
        .parallax = {.x = 1.0f, .y = 1.0f, .z = 1.0f},
        .static_camera = false,
        .static_screen = NULL,

        .animation = NULL,

        //.psshader = NULL,
        //.psframebuffer = NULL,

        .blend_enabled = true, // obligatory
        .blend_src_rgb = BLEND_DEFAULT,
        .blend_dst_rgb = BLEND_DEFAULT,
        .blend_src_alpha = BLEND_DEFAULT,
        .blend_dst_alpha = BLEND_DEFAULT,

        .viewport_x = -1.0f,
        .viewport_y = -1.0f,
        .viewport_width = -1.0f,
        .viewport_height = -1.0f
    };

    arraypointerlist_add(layout_context->group_list, group);

    // sh4matrix_reset(group->matrix);
    pvr_context_helper_clear_modifier(&group->modifier);
    pvr_context_helper_clear_offsetcolor(group->offsetcolor);

    /*if (vertexprops_parse_boolean(unparsed_group, "framebuffer", false)) {
        // assume layout as part of the main PVRContext renderer
        group->psframebuffer = new PSFramebuffer(pvr_context);
    }*/

    foreach (XmlNode, item, XMLPARSER_CHILDREN_ITERATOR, unparsed_group) {
        const char* tagName = xmlparser_get_tag_name(item);

        if (string_equals(tagName, "Action")) {
            /*if (!parent_context) {
                logger_warn(
                    "layout_parse_group() action found in the layout root, "
                    "will be imported as root group action."
                );
            }*/
            layout_parse_group_action(item, layout_context->animlist, actions_arraylist);
        } else if (string_equals(tagName, "Sprite")) {
            layout_parse_sprite(item, layout_context, group->group_id);
        } else if (string_equals(tagName, "Text")) {
            layout_parse_text(item, layout_context, group->group_id);
        } else if (string_equals(tagName, "Camera")) {
            if (parent_context) {
                logger_warn(
                    "layout_parse_group() groups can not contains cameras"
                );
            }
            layout_parse_camera(item, layout_context);
        } else if (string_equals(tagName, "Group")) {
            layout_parse_group(item, layout_context, &group->context);
        } else if (string_equals(tagName, "Placeholder")) {
            layout_parse_placeholder(item, layout_context, group->group_id);
        } else if (string_equals(tagName, "Sound")) {
            layout_parse_sound(item, layout_context);
        } else if (string_equals(tagName, "Video")) {
            layout_parse_video(item, layout_context, group->group_id);
        } else if (string_equals(tagName, "AttachValue")) {
            // done in layout_parse_externalvalues()
        } else if (string_equals(tagName, "Font")) {
            // done in layout_parse_fonts()
        } else if (string_equals(tagName, "Trigger")) {
            layout_parse_triggers(item, layout_context);
        } else {
            logger_warn("layout_parse_group() unknown element: %s", tagName);
        }
    }

    arraylist_destroy2(&actions_arraylist, &group->actions_size, (void**)&group->actions);

    // add to group tree
    if (parent_context) {
        layout_helper_add_group_to_parent(parent_context, group);
    }
}

static void layout_parse_fonts(XmlNode unparsed_root, LayoutContext* layout_context) {
    XmlRecursiveIterator iterator = xmlparser_iterator_get_recursive(unparsed_root, "Font");
    ArrayList fonts_arraylist = arraylist_init(sizeof(Font));

    foreach (XmlNode, item, XMLPARSER_RECURSIVE_ITERATOR, iterator) {
        const char* name = xmlparser_get_attribute_value2(item, "name");
        const char* path = xmlparser_get_attribute_value2(item, "path");
        bool glyph_animate = vertexprops_parse_boolean(item, "glyphAnimate", true);
        const char* glyph_suffix = xmlparser_get_attribute_value2(item, "glyphSuffix");
        bool glyph_color_by_addition = vertexprops_parse_boolean(item, "colorByAddition", false);

        if (!name) {
            logger_error_xml("layout_parse_fonts() missing font name: ", item);
            continue;
        }
        if (!path) {
            logger_error_xml("layout_parse_fonts() missing font path: ", item);
            continue;
        }

        void* font;
        bool is_atlas = atlas_utils_is_known_extension(path);

        if (is_atlas) {
            font = fontglyph_init(path, glyph_suffix, glyph_animate);
        } else {
            font = fonttype_init(path);
            glyph_color_by_addition = false;
        }

        if (font) {
            FontHolder fontholder = fontholder_init2(font, is_atlas, -1, glyph_color_by_addition);
            arraylist_add(fonts_arraylist, &(Font){.name = string_duplicate(name), .fontholder = fontholder});
        } else {
            logger_error("layout_parse_fonts() unable to read the font: %s", path);
        }
    }

    arraylist_destroy2(&fonts_arraylist, &layout_context->fonts_size, (void**)&layout_context->fonts);
}

static void layout_parse_camera(XmlNode unparsed_camera, LayoutContext* layout_context) {
    float x = 0.0f, y = 0.0f, z = 0.0f;

    float duration_beats = layout_helper_parse_float(unparsed_camera, "durationInBeats", FLOAT_NaN);
    float duration_milliseconds = layout_helper_parse_float(unparsed_camera, "duration", FLOAT_NaN);
    bool enable_offset_zoom = vertexprops_parse_boolean(unparsed_camera, "offsetZoom", true);

    bool duration_in_beats, has_duration;
    float duration;

    if (math2d_is_float_NaN(duration_beats) && math2d_is_float_NaN(duration_milliseconds)) {
        duration_in_beats = true;
        duration = 1.0f;
        has_duration = false;
    } else {
        duration_in_beats = math2d_is_float_NaN(duration_milliseconds);
        duration = duration_in_beats ? duration_beats : duration_milliseconds;
        has_duration = true;
    }

    CameraPlaceholder camera_placeholder = (CameraPlaceholder){
        .name = ATTR_STRDUP(unparsed_camera, "name"),

        .enable_offset_zoom = enable_offset_zoom,

        .has_duration = has_duration,
        .duration_in_beats = duration_in_beats,
        .duration = duration,

        .move_only = false,
        .has_from = false,

        .is_empty = false,

        .has_parallax_offset_only = false,

        .move_offset_only = false,
        .has_offset_from = false,
        .has_offset_to = false,

        .to_offset_x = FLOAT_NaN,
        .to_offset_y = FLOAT_NaN,
        .to_offset_z = FLOAT_NaN,

        .from_offset_x = FLOAT_NaN,
        .from_offset_y = FLOAT_NaN,
        .from_offset_z = FLOAT_NaN,

        .animation = NULL,

        .from_x = FLOAT_NaN,
        .from_y = FLOAT_NaN,
        .from_z = FLOAT_NaN,
        .to_x = FLOAT_NaN,
        .to_y = FLOAT_NaN,
        .to_z = FLOAT_NaN,
        .offset_x = 0.0f,
        .offset_y = 0.0f,
        .offset_z = 1.0f
    };

    const char* anim_name = xmlparser_get_attribute_value2(unparsed_camera, "animationName");
    XmlNode unparsed_move = xmlparser_get_first_children(unparsed_camera, "Move");
    XmlNode unparsed_from = xmlparser_get_first_children(unparsed_camera, "From");
    XmlNode unparsed_to = xmlparser_get_first_children(unparsed_camera, "To");

    XmlNode unparsed_offset = xmlparser_get_first_children(unparsed_camera, "Offset");
    XmlNode unparsed_move_offset = xmlparser_get_first_children(unparsed_camera, "OffsetMove");
    XmlNode unparsed_from_offset = xmlparser_get_first_children(unparsed_camera, "FromOffset");
    XmlNode unparsed_to_offset = xmlparser_get_first_children(unparsed_camera, "ToOffset");

    if (
        !anim_name &&
        !unparsed_move && !unparsed_from && !unparsed_to &&
        !unparsed_offset &&
        !unparsed_move_offset && !unparsed_from_offset && !unparsed_to_offset
    ) {
        // no animation or tween is defined
        camera_placeholder.is_empty = true;

        arraylist_add(layout_context->camera_list, &camera_placeholder);
        return;
    }


    if (unparsed_move) {
        camera_placeholder.move_only = true;
        layout_herper_parse_offsetmovefromto(unparsed_move, &x, &y, &z);
        camera_placeholder.from_x = camera_placeholder.to_x = x;
        camera_placeholder.from_y = camera_placeholder.to_y = y;
        camera_placeholder.from_z = camera_placeholder.to_z = z;
    } else {
        if (unparsed_from) {
            layout_herper_parse_offsetmovefromto(unparsed_from, &x, &y, &z);
            camera_placeholder.from_x = x;
            camera_placeholder.from_y = y;
            camera_placeholder.from_z = z;
            camera_placeholder.has_from = true;
        }
        if (unparsed_to) {
            layout_herper_parse_offsetmovefromto(unparsed_to, &x, &y, &z);
            camera_placeholder.to_x = x;
            camera_placeholder.to_y = y;
            camera_placeholder.to_z = z;
        }
    }

    if (unparsed_offset) {
        layout_herper_parse_offsetmovefromto(unparsed_offset, &x, &y, &z);
        camera_placeholder.offset_x = x;
        camera_placeholder.offset_y = y;
        camera_placeholder.offset_z = z;
        camera_placeholder.has_parallax_offset_only = true;
    } else if (unparsed_move_offset) {
        layout_herper_parse_offsetmovefromto(unparsed_move_offset, &x, &y, &z);
        camera_placeholder.to_offset_x = x;
        camera_placeholder.to_offset_y = y;
        camera_placeholder.to_offset_z = z;
        camera_placeholder.move_offset_only = true;
    } else {
        if (unparsed_from_offset) {
            layout_herper_parse_offsetmovefromto(unparsed_from_offset, &x, &y, &z);
            camera_placeholder.from_offset_x = x;
            camera_placeholder.from_offset_y = y;
            camera_placeholder.from_offset_z = z;
            camera_placeholder.has_offset_from = true;
        }
        if (unparsed_to_offset) {
            layout_herper_parse_offsetmovefromto(unparsed_to_offset, &x, &y, &z);
            camera_placeholder.to_offset_x = x;
            camera_placeholder.to_offset_y = y;
            camera_placeholder.to_offset_z = z;
            camera_placeholder.has_offset_to = true;
        }
    }

    if (anim_name) {
        if (layout_context->animlist) {
            camera_placeholder.animation = animsprite_init_from_animlist(
                layout_context->animlist, anim_name
            );
        } else {
            logger_warn("layout_parse_camera() can not import '%s', layout does not have an animlist", anim_name);
        }
    }

    arraylist_add(layout_context->camera_list, &camera_placeholder);
}

static void layout_parse_externalvalues(XmlNode unparsed_root, LayoutContext* layout_context) {
    XmlRecursiveIterator list = xmlparser_iterator_get_recursive(unparsed_root, "AttachValue");

    ArrayList values_arraylist = arraylist_init(sizeof(ExternalValue));

    foreach (XmlNode, item, XMLPARSER_RECURSIVE_ITERATOR, list) {
        char* name = ATTR_STRDUP(item, "name");
        const char* unparsed_type = xmlparser_get_attribute_value2(item, "type");
        const char* unparsed_value = xmlparser_get_attribute_value2(item, "value");

        if (!name) {
            logger_error_xml("layout_parse_externalvalues() missing AttachValue name:", item);
            continue;
        }
        if (!unparsed_type) {
            logger_error_xml("layout_parse_externalvalues() missing AttachValue type:", item);
            continue;
        }

        LayoutAttachedValue value;
        AttachedValueType type;
        bool invalid;

        if (string_equals_ignore_case(unparsed_type, "string")) {
            value.as_string = string_duplicate(unparsed_value);
            type = AttachedValueType_STRING;
            invalid = false;
        } else if (string_equals_ignore_case(unparsed_type, "float")) {
            value.as_double = vertexprops_parse_double2(unparsed_value, DOUBLE_NaN);
            type = AttachedValueType_FLOAT;
            invalid = math2d_is_double_NaN(value.as_double);
        } else if (string_equals_ignore_case(unparsed_type, "integer")) {
            value.as_long = vertexprops_parse_integer2(unparsed_value, 0);
            type = AttachedValueType_INTEGER;
            invalid = !vertexprops_is_integer(unparsed_value);
        } else if (string_equals_ignore_case(unparsed_type, "hex")) {
            invalid = !vertexprops_parse_hex(unparsed_value, &value.as_unsigned, false);
            type = AttachedValueType_HEX;
        } else if (string_equals_ignore_case(unparsed_type, "boolean")) {
            nbool bolean_value = vertexprops_parse_boolean2(unparsed_value, unset);
            value.as_boolean = bolean_value;
            type = AttachedValueType_BOOLEAN;
            invalid = bolean_value == unset;
        } else {
            invalid = true;
            type = AttachedValueType_NOTFOUND;
            logger_error_xml("layout_parse_externalvalues() unknown AttachValue type:", item);
        }

        if (invalid) {
            logger_error_xml("layout_parse_externalvalues() value in: ", item);
            continue;
        }

        ExternalValue entry = (ExternalValue){.name = name, .value = value, .type = type};
        arraylist_add(values_arraylist, &entry);
    }

    arraylist_destroy2(&values_arraylist, &layout_context->values_size, (void**)&layout_context->values);
}

static void layout_parse_triggers(XmlNode unparsed_trigger, LayoutContext* layout_context) {
    Trigger trigger = (Trigger){
        .name = ATTR_STRDUP(unparsed_trigger, "name"),

        .action_name = ATTR_STRDUP(unparsed_trigger, "action"),
        .camera_name = ATTR_STRDUP(unparsed_trigger, "camera"),
        .trigger_name = ATTR_STRDUP(unparsed_trigger, "trigger"),
        .stop_trigger_name = ATTR_STRDUP(unparsed_trigger, "stopTrigger"),

        .loop = vertexprops_parse_integer(unparsed_trigger, "loop", 1), // 1 means execute once

        .loop_delay = 0.0f,
        .loop_delay_beats = 0.0f,
        .loop_delay_beats_in_beats = false,

        .start_delay = 0.0f,
        .start_delay_beats = 0.0f,
        .start_delay_beats_in_beats = false,

        .context = (TriggerContext){
            .running = false,
            .reject_recursive = false,
            .progress_delay = 0.0,
            .loop_waiting = false,
            .loop_count = false,
        }
    };

    if (xmlparser_has_attribute(unparsed_trigger, "loopDelayBeats")) {
        trigger.loop_delay_beats_in_beats = true;
        trigger.loop_delay_beats = layout_helper_parse_float(unparsed_trigger, "loopDelayBeats", 0.0f);
    } else if (xmlparser_has_attribute(unparsed_trigger, "loopDelay")) {
        trigger.loop_delay_beats_in_beats = false;
        trigger.loop_delay = layout_helper_parse_float(unparsed_trigger, "loopDelay", 0.0f);
    }

    if (xmlparser_has_attribute(unparsed_trigger, "startDelayBeats")) {
        trigger.start_delay_beats_in_beats = true;
        trigger.start_delay_beats = layout_helper_parse_float(unparsed_trigger, "startDelayBeats", 0.0f);
    } else if (xmlparser_has_attribute(unparsed_trigger, "startDelay")) {
        trigger.start_delay_beats_in_beats = false;
        trigger.start_delay = layout_helper_parse_float(unparsed_trigger, "startDelay", 0.0f);
    }

    arraylist_add(layout_context->trigger_list, &trigger);
}

static void layout_parse_sound(XmlNode unparsed_sound, LayoutContext* layout_context) {
    const char* src = xmlparser_get_attribute_value2(unparsed_sound, "src");
    if (!src) {
        logger_error("layout_parse_sound() missing attribute 'src'");
        return;
    }

    SoundPlayer soundplayer = soundplayer_init(src);
    if (!soundplayer) {
        logger_warn("layout_parse_sound() can not load: %s", src);
        return;
    }

    float volume = layout_helper_parse_float(unparsed_sound, "volume", 1.0f);
    bool looped = vertexprops_parse_boolean(unparsed_sound, "looped", false);
    // float pan = layout_helper_parse_float(unparsed_sound, "pan", 0.0f);
    // bool muted = vertexprops_parse_boolean(unparsed_sound, "muted", false);
    XmlRecursiveIterator actions = xmlparser_iterator_get_recursive(unparsed_sound, "Action");
    ArrayList actions_arraylist = arraylist_init(sizeof(Action));

    Item sound = (Item){
        .name = ATTR_STRDUP(unparsed_sound, "name"),
        .initial_action_name = ATTR_STRDUP(unparsed_sound, "initialAction"),
        .soundplayer = soundplayer,
        .type = VERTEX_NONE,

        .actions_size = 0,
        .actions = NULL,

        .was_playing = false
    };

    soundplayer_set_volume(sound.soundplayer, volume);
    soundplayer_loop_enable(sound.soundplayer, looped);

    foreach (XmlNode, action, XMLPARSER_RECURSIVE_ITERATOR, actions) {
        layout_parse_sound_action(action, layout_context->animlist, actions_arraylist);
    }
    arraylist_destroy2(&actions_arraylist, &sound.actions_size, (void**)&sound.actions);

    arraylist_add(layout_context->sound_list, &sound);
}

static void layout_parse_video(XmlNode unparsed_video, LayoutContext* layout_context, int32_t group_id) {
    const char* src = xmlparser_get_attribute_value2(unparsed_video, "src");
    if (!src) {
        logger_error("layout_parse_video() missing attribute 'src'");
        return;
    }

    VideoPlayer videoplayer = videoplayer_init(src);
    if (!videoplayer) {
        logger_warn("layout_parse_video() can not load: %s", src);
        return;
    }

    float volume = layout_helper_parse_float(unparsed_video, "volume", 1.0f);
    bool looped = vertexprops_parse_boolean(unparsed_video, "looped", false);
    // float pan = layout_helper_parse_float(unparsed_video, "pan", 0.0f);
    // bool muted = vertexprops_parse_boolean(unparsed_video, "muted", false);
    XmlRecursiveIterator actions = xmlparser_iterator_get_recursive(unparsed_video, "Action");
    ArrayList actions_arraylist = arraylist_init(sizeof(Action));

    Item video = (Item){
        .name = ATTR_STRDUP(unparsed_video, "name"),
        .initial_action_name = ATTR_STRDUP(unparsed_video, "initialAction"),
        .videoplayer = videoplayer,

        .actions_size = 0,
        .actions = NULL,

        .was_playing = false,
        .in_vertex_list_index = arraylist_size(layout_context->vertex_list)
    };

    videoplayer_set_volume(video.videoplayer, volume);
    videoplayer_loop_enable(video.videoplayer, looped);

    foreach (XmlNode, action, XMLPARSER_RECURSIVE_ITERATOR, actions) {
        layout_parse_video_action(action, layout_context->animlist, actions_arraylist);
    }
    arraylist_destroy2(&actions_arraylist, &video.actions_size, (void**)&video.actions);

    Item sprite = (Item){
        .actions = NULL,
        .actions_size = 0,
        .animation = NULL,
        .group_id = group_id,
        .initial_action_name = NULL,
        .name = NULL,
        .parallax = (LayoutParallax){.x = 1.0f, .y = 1.0f, .z = 1.0f},
        .placeholder = NULL,
        .soundplayer = NULL,
        .videoplayer = videoplayer,
        .static_camera = false,
        .type = VERTEX_SPRITE,
        .vertex.as_sprite = videoplayer_get_sprite(videoplayer),
        .was_playing = false
    };

    arraylist_add(layout_context->vertex_list, &sprite);
    arraylist_add(layout_context->video_list, &video);
}

static void layout_parse_macro(XmlNode unparsed_root, LayoutContext* layout_context) {
    XmlRecursiveIterator list = xmlparser_iterator_get_recursive(unparsed_root, "Macro");
    ArrayList macro_arraylist = arraylist_init(sizeof(Macro));

    foreach (XmlNode, unparsed_macro, XMLPARSER_RECURSIVE_ITERATOR, list) {
        if (!xmlparser_has_attribute(unparsed_macro, "name")) {
            logger_warn_xml("layout_parse_macro() missing name in:", unparsed_macro);
            continue;
        }

        const char* name = xmlparser_get_attribute_value2(unparsed_macro, "name");
        const char* animation = xmlparser_get_attribute_value2(unparsed_macro, "animation");
        if (!animation) animation = name;
        AnimSprite animsprite = animsprite_init_from_animlist(layout_context->animlist, animation);

        if (!animsprite) {
            logger_warn("layout_parse_macro() missing animation: %s", animation);
            continue;
        }

        Macro macro = (Macro){
            .name = string_duplicate(name),
            .loop = vertexprops_parse_integer(unparsed_macro, "loopByBeats", 0),
            .loop_by_beats = vertexprops_parse_boolean(unparsed_macro, "loopByBeats", false),
            .actions = NULL,
            .actions_size = 0
        };

        layout_parse_macro_actions(unparsed_macro, &macro);
        arraylist_add(macro_arraylist, &macro);
    }

    arraylist_destroy2(&macro_arraylist, &layout_context->macro_list_size, (void**)&layout_context->macro_list);
}


//////////////////////////////////
///        ACTION PARSER       ///
//////////////////////////////////

static void layout_parse_sprite_action(XmlNode unparsed_action, AnimList animlist, Atlas atlas, ArrayList action_entries, bool from_video) {
    ArrayList entries = arraylist_init2(sizeof(ActionEntry), xmlparser_get_children_count(unparsed_action));

    foreach (XmlNode, unparsed_entry, XMLPARSER_CHILDREN_ITERATOR, unparsed_action) {
        const char* tagName = xmlparser_get_tag_name(unparsed_entry);

        if (string_equals(tagName, "Location")) {
            layout_helper_add_action_location(unparsed_entry, entries);
        } else if (string_equals(tagName, "Size")) {
            layout_helper_add_action_size(unparsed_entry, entries);
        } else if (string_equals(tagName, "OffsetColor")) {
            layout_helper_add_action_offsetcolor(unparsed_entry, entries);
        } else if (string_equals(tagName, "Color")) {
            layout_helper_add_action_color(unparsed_entry, entries);
        } else if (string_equals(tagName, "Property")) {
            layout_helper_add_action_property(unparsed_entry, 0, entries);
        } else if (string_equals(tagName, "Properties")) {
            layout_helper_add_action_properties(unparsed_entry, 0, entries);
        } else if (string_equals(tagName, "AtlasApply")) {
            layout_helper_add_action_atlasapply(unparsed_entry, atlas, entries);
        } else if (string_equals(tagName, "Resize")) {
            layout_helper_add_action_resize(unparsed_entry, entries);
        } else if (string_equals(tagName, "Animation")) {
            layout_helper_add_action_animation(unparsed_entry, animlist, entries);
        } else if (string_equals(tagName, "AnimationFromAtlas")) {
            layout_helper_add_action_animationfromatlas(unparsed_entry, atlas, entries);
        } else if (string_equals(tagName, "AnimationRemove")) {
            layout_helper_add_action_animationremove(unparsed_entry, entries);
        } else if (string_equals(tagName, "AnimationEnd")) {
            layout_helper_add_action_animationend(unparsed_entry, entries);
        } else if (string_equals(tagName, "Parallax")) {
            layout_helper_add_action_parallax(unparsed_entry, entries);
        } else if (string_equals(tagName, "Modifier")) {
            layout_helper_add_action_modifier(unparsed_entry, entries);
        } else if (string_equals(tagName, "Static")) {
            layout_helper_add_action_static(unparsed_entry, entries);
        } else if (string_equals(tagName, "Hide") || string_equals(tagName, "Show")) {
            layout_helper_add_action_visibility(unparsed_entry, entries);
        } else if (string_equals(tagName, "SetShader")) {
            layout_helper_add_action_setshader(unparsed_entry, entries);
        } else if (string_equals(tagName, "RemoveShader")) {
            layout_helper_add_action_removeshader(unparsed_entry, entries);
        } else if (string_equals(tagName, "SetShaderUniform")) {
            layout_helper_add_action_setshaderuniform(unparsed_entry, entries);
        } else if (string_equals(tagName, "SetBlending")) {
            layout_helper_add_action_setblending(unparsed_entry, entries);
        } else if (string_equals(tagName, "SetTrailing")) {
            layout_helper_add_action_spritetrailing(unparsed_entry, entries);
        } else if (string_equals(tagName, "SetTrailingOffsetcolor")) {
            layout_helper_add_action_spritetrailingoffsetcolor(unparsed_entry, entries);
        } else if (!from_video) {
            logger_warn("layout_parse_sprite_action() unknown action entry: %s", tagName);
        }
    }

    Action action = (Action){
        name : ATTR_STRDUP(unparsed_action, "name"),
        entries : NULL,
        entries_size : -1
    };

    arraylist_destroy2(&entries, &action.entries_size, (void**)&action.entries);
    arraylist_add(action_entries, &action);
}

static void layout_parse_text_action(XmlNode unparsed_action, AnimList animlist, ArrayList action_entries) {
    ArrayList entries = arraylist_init2(sizeof(ActionEntry), xmlparser_get_children_count(unparsed_action));

    foreach (XmlNode, unparsed_entry, XMLPARSER_CHILDREN_ITERATOR, unparsed_action) {
        const char* tagName = xmlparser_get_tag_name(unparsed_entry);

        if (string_equals(tagName, "Property")) {
            layout_helper_add_action_property(unparsed_entry, true, entries);
        } else if (string_equals(tagName, "Properties")) {
            layout_helper_add_action_properties(unparsed_entry, true, entries);
        } else if (string_equals(tagName, "Location")) {
            layout_helper_add_action_location(unparsed_entry, entries);
        } else if (string_equals(tagName, "Color")) {
            layout_helper_add_action_color(unparsed_entry, entries);
        } else if (string_equals(tagName, "MaxSize")) {
            layout_helper_add_action_size(unparsed_entry, entries);
        } else if (string_equals(tagName, "String")) {
            ActionEntry entry = (ActionEntry){
                .type = LAYOUT_ACTION_PROPERTY,
                .property = TEXTSPRITE_PROP_STRING,
                .misc.string = xmlparser_get_textContext(unparsed_entry),
            };
            arraylist_add(entries, &entry);
        } else if (string_equals(tagName, "Border")) {
            layout_helper_add_action_textborder(unparsed_entry, entries);
        } else if (string_equals(tagName, "BorderOffset")) {
            layout_helper_add_action_textborderoffset(unparsed_entry, entries);
        } else if (string_equals(tagName, "Background")) {
            layout_helper_add_action_textbackground(unparsed_entry, entries);
        } else if (string_equals(tagName, "BackgroundColor")) {
            layout_helper_add_action_textbackgroundcolor(unparsed_entry, entries);
        } else if (string_equals(tagName, "Animation")) {
            layout_helper_add_action_animation(unparsed_entry, animlist, entries);
        } else if (string_equals(tagName, "AnimationRemove")) {
            layout_helper_add_action_animationremove(unparsed_entry, entries);
        } else if (string_equals(tagName, "AnimationEnd")) {
            layout_helper_add_action_animationend(unparsed_entry, entries);
        } else if (string_equals(tagName, "Parallax")) {
            layout_helper_add_action_parallax(unparsed_entry, entries);
        } else if (string_equals(tagName, "Modifier")) {
            layout_helper_add_action_modifier(unparsed_entry, entries);
        } else if (string_equals(tagName, "Static")) {
            layout_helper_add_action_static(unparsed_entry, entries);
        } else if (string_equals(tagName, "Hide") || string_equals(tagName, "Show")) {
            layout_helper_add_action_visibility(unparsed_entry, entries);
        } else if (string_equals(tagName, "SetShader")) {
            layout_helper_add_action_setshader(unparsed_entry, entries);
        } else if (string_equals(tagName, "RemoveShader")) {
            layout_helper_add_action_removeshader(unparsed_entry, entries);
        } else if (string_equals(tagName, "SetShaderUniform")) {
            layout_helper_add_action_setshaderuniform(unparsed_entry, entries);
        } else if (string_equals(tagName, "SetBlending")) {
            layout_helper_add_action_setblending(unparsed_entry, entries);
        } else {
            logger_warn("layout_parse_text_action() unknown Text action entry: %s", tagName);
        }
    }

    Action action = (Action){
        name : ATTR_STRDUP(unparsed_action, "name"),
        entries : NULL,
        entries_size : -1
    };

    arraylist_destroy2(&entries, &action.entries_size, (void**)&action.entries);
    arraylist_add(action_entries, &action);
}

static void layout_parse_group_action(XmlNode unparsed_action, AnimList animlist, ArrayList action_entries) {
    ArrayList entries = arraylist_init2(sizeof(ActionEntry), xmlparser_get_children_count(unparsed_action));

    foreach (XmlNode, unparsed_entry, XMLPARSER_CHILDREN_ITERATOR, unparsed_action) {
        const char* tagName = xmlparser_get_tag_name(unparsed_entry);

        if (string_equals(tagName, "ResetMatrix")) {
            layout_helper_add_action_resetmatrix(unparsed_entry, entries);
        } else if (string_equals(tagName, "Modifier")) {
            layout_helper_add_action_modifier(unparsed_entry, entries);
        } else if (string_equals(tagName, "Property")) {
            layout_helper_add_action_property(unparsed_entry, false, entries);
        } else if (string_equals(tagName, "Properties")) {
            layout_helper_add_action_properties(unparsed_entry, false, entries);
        } else if (string_equals(tagName, "OffsetColor")) {
            layout_helper_add_action_offsetcolor(unparsed_entry, entries);
        } else if (string_equals(tagName, "Parallax")) {
            layout_helper_add_action_parallax(unparsed_entry, entries);
        } else if (string_equals(tagName, "Static")) {
            layout_helper_add_action_static(unparsed_entry, entries);
        } else if (string_equals(tagName, "Hide") || string_equals(tagName, "Show")) {
            layout_helper_add_action_visibility(unparsed_entry, entries);
        } else if (string_equals(tagName, "Animation")) {
            layout_helper_add_action_animation(unparsed_entry, animlist, entries);
        } else if (string_equals(tagName, "AnimationRemove")) {
            layout_helper_add_action_animationremove(unparsed_entry, entries);
        } else if (string_equals(tagName, "AnimationEnd")) {
            layout_helper_add_action_animationend(unparsed_entry, entries);
        } else if (string_equals(tagName, "SetShader")) {
            layout_helper_add_action_setshader(unparsed_entry, entries);
        } else if (string_equals(tagName, "RemoveShader")) {
            layout_helper_add_action_removeshader(unparsed_entry, entries);
        } else if (string_equals(tagName, "SetShaderUniform")) {
            layout_helper_add_action_setshaderuniform(unparsed_entry, entries);
        } else if (string_equals(tagName, "SetBlending")) {
            layout_helper_add_action_setblending(unparsed_entry, entries);
        } else if (string_equals(tagName, "Viewport")) {
            layout_helper_add_action_viewport(unparsed_entry, entries);
        }
    }

    Action action = (Action){
        .name = ATTR_STRDUP(unparsed_action, "name"),
        .entries = NULL,
        .entries_size = -1
    };

    arraylist_destroy2(&entries, &action.entries_size, (void**)&action.entries);
    arraylist_add(action_entries, &action);
}

static void layout_parse_sound_action(XmlNode unparsed_action, AnimList animlist, ArrayList action_entries) {
    (void)animlist;

    ArrayList entries = arraylist_init2(sizeof(ActionEntry), xmlparser_get_children_count(unparsed_action));

    foreach (XmlNode, unparsed_entry, XMLPARSER_CHILDREN_ITERATOR, unparsed_action) {
        const char* tagName = xmlparser_get_tag_name(unparsed_entry);

        if (string_equals(tagName, "Property")) {
            layout_helper_add_action_mediaproperty(unparsed_entry, entries);
        } else if (string_equals(tagName, "Properties")) {
            layout_helper_add_action_mediaproperties(unparsed_entry, entries);
        } else if (string_equals(tagName, "FadeIn") || string_equals(tagName, "FadeOut")) {
            layout_helper_add_action_soundfade(unparsed_entry, entries);
        } else if (string_equals(tagName, "Seek")) {
            layout_helper_add_action_seek(unparsed_entry, entries);
            /*} else if (string_equals(tagName, "Animation")) {
                layout_helper_add_action_animation(unparsed_entry, animlist, entries);
            } else if (string_equals(tagName, "AnimationRemove")) {
                layout_helper_add_action_animationremove(unparsed_entry, entries);
            } else if (string_equals(tagName, "AnimationEnd")) {
                layout_helper_add_action_animationend(unparsed_entry, entries);*/
        } else if (layout_helper_add_action_media(unparsed_entry, entries)) {
            logger_warn("layout_parse_sound_action() unknown Sound action entry: %s", tagName);
        }
    }

    Action action = (Action){
        .name = ATTR_STRDUP(unparsed_action, "name"),
        .entries = NULL,
        .entries_size = -1
    };

    arraylist_destroy2(&entries, &action.entries_size, (void**)&action.entries);
    arraylist_add(action_entries, &action);
}

static void layout_parse_video_action(XmlNode unparsed_action, AnimList animlist, ArrayList action_entries) {
    ArrayList entries = arraylist_init2(sizeof(ActionEntry), xmlparser_get_children_count(unparsed_action));

    foreach (XmlNode, unparsed_entry, XMLPARSER_CHILDREN_ITERATOR, unparsed_action) {
        const char* tagName = xmlparser_get_tag_name(unparsed_entry);

        if (string_equals(tagName, "Property")) {
            layout_helper_add_action_mediaproperty(unparsed_entry, entries);
        } else if (string_equals(tagName, "Properties")) {
            layout_helper_add_action_mediaproperties(unparsed_entry, entries);
        } else if (string_equals(tagName, "FadeIn") || string_equals(tagName, "FadeOut")) {
        } else if (string_equals(tagName, "Seek")) {
            layout_helper_add_action_seek(unparsed_entry, entries);
        } else if (layout_helper_add_action_media(unparsed_entry, entries)) {
            layout_parse_sprite_action(unparsed_action, animlist, NULL, action_entries, true);
        }
    }

    Action action = (Action){
        .name = ATTR_STRDUP(unparsed_action, "name"),
        .entries = NULL,
        .entries_size = -1
    };

    arraylist_destroy2(&entries, &action.entries_size, (void**)&action.entries);
    arraylist_add(action_entries, &action);
}

static void layout_parse_macro_actions(XmlNode unparsed_macro, Macro* macro) {
    ArrayList actions_arraylist = arraylist_init2(sizeof(MacroAction), xmlparser_get_children_count(unparsed_macro));

    foreach (XmlNode, unparsed_action, XMLPARSER_CHILDREN_ITERATOR, unparsed_macro) {
        int32_t type;
        char* target_name = NULL;
        char* action_name = NULL;
        char* trigger_name = NULL;
        char* stop_trigger_name = NULL;
        char* camera_name = NULL;

        const char* tagName = xmlparser_get_tag_name(unparsed_action);
        if (string_equals(tagName, "PuppetGroup")) {
            type = LAYOUT_ACTION_PUPPETGROUP;
            target_name = ATTR_STRDUP(unparsed_action, "target");
        } else if (string_equals(tagName, "Puppet")) {
            type = LAYOUT_ACTION_PUPPETITEM;
            target_name = ATTR_STRDUP(unparsed_action, "target");
        } else if (string_equals(tagName, "Execute")) {
            type = LAYOUT_ACTION_EXECUTE;
            target_name = ATTR_STRDUP(unparsed_action, "target");
            action_name = ATTR_STRDUP(unparsed_action, "action");
            trigger_name = ATTR_STRDUP(unparsed_action, "trigger");
            stop_trigger_name = ATTR_STRDUP(unparsed_action, "stopTrigger");
            camera_name = ATTR_STRDUP(unparsed_action, "camera");
        } else {
            logger_warn("layout_parse_macro_actions() unknown Macro action: %s", tagName);
            continue;
        }

        if (!xmlparser_has_attribute(unparsed_action, "id")) {
            logger_warn_xml("layout_parse_macro_actions() missing event id in Macro action:", unparsed_action);
            continue;
        }

        int32_t id = vertexprops_parse_integer(unparsed_action, "id", 0);
        MacroAction action = (MacroAction){
            .type = type,
            .id = id,
            .target_name = target_name,
            .action_name = action_name,
            .trigger_name = trigger_name,
            .stop_trigger_name = stop_trigger_name,
            .camera_name = camera_name
        };

        arraylist_add(actions_arraylist, &action);
    }

    arraylist_destroy2(&actions_arraylist, &macro->actions_size, (void**)&macro->actions);
}


//////////////////////////////////
///      ACTION EXECUTERS      ///
//////////////////////////////////

static void layout_helper_execute_action_in_sprite(Action* action, Item* item, float viewport_width, float viewport_height) {
    float location_x = 0.0f, location_y = 0.0f;
    float draw_width = 0.0f, draw_height = 0.0f;

    Sprite sprite = item->vertex.as_sprite;

    for (int32_t i = 0; i < action->entries_size; i++) {
        ActionEntry* entry = &action->entries[i];

        switch (entry->type) {
            case LAYOUT_ACTION_LOCATION:

                // Note: The sprite must have a draw size
                sprite_get_draw_size(sprite, &draw_width, &draw_height);

                if (sprite_is_textured(sprite)) {
                    layout_helper_location(
                        entry, draw_width, draw_height, viewport_width, viewport_height, &location_x, &location_y
                    );
                } else {
                    sprite_get_draw_location(sprite, &location_x, &location_y);
                    if (!math2d_is_float_NaN(entry->x)) location_x = entry->x;
                    if (!math2d_is_float_NaN(entry->y)) location_y = entry->y;
                }
                sprite_set_draw_location(sprite, location_x, location_y);
                if (!math2d_is_float_NaN(entry->z)) sprite_set_z_index(sprite, entry->z);
                break;
            case LAYOUT_ACTION_SIZE:
                if (entry->has_resize)
                    sprite_resize_draw_size(sprite, entry->width, entry->height, NULL, NULL);
                else
                    sprite_set_draw_size(sprite, entry->width, entry->height);
                break;
            case LAYOUT_ACTION_OFFSETCOLOR:
                sprite_set_offsetcolor(
                    sprite, entry->rgba[0], entry->rgba[1], entry->rgba[2], entry->rgba[3]
                );
                break;
            case LAYOUT_ACTION_COLOR:
                sprite_set_vertex_color(sprite, entry->rgba[0], entry->rgba[1], entry->rgba[2]);
                if (!math2d_is_float_NaN(entry->rgba[3])) sprite_set_alpha(sprite, entry->rgba[3]);
                break;
            case LAYOUT_ACTION_PROPERTY:
                sprite_set_property(sprite, entry->property, entry->value);
                break;
            case LAYOUT_ACTION_ATLASAPPLY:
                atlas_apply_from_entry(sprite, entry->misc.atlas_entry, entry->override_size);
                break;
            case LAYOUT_ACTION_RESIZE:
                imgutils_calc_resize_sprite(
                    sprite, entry->max_width, entry->max_height, entry->cover, entry->center
                );
                break;
            case LAYOUT_ACTION_ANIMATION:
                if (!entry->misc.animsprite && !item->animation) break;
                if (entry->misc.animsprite) item->animation = entry->misc.animsprite;
                if (entry->restart) animsprite_restart(item->animation);
                if (entry->stop_in_loop) animsprite_disable_loop(item->animation);

                sprite_animation_play_by_animsprite(sprite, item->animation, false);
                sprite_animate(sprite, 0.0f);
                break;
            case LAYOUT_ACTION_ANIMATIONREMOVE:
                sprite_animation_play_by_animsprite(sprite, NULL, false);
                break;
            case LAYOUT_ACTION_ANIMATIONEND:
                sprite_animation_end(sprite);
                break;
            case LAYOUT_ACTION_VISIBILITY:
                sprite_set_visible(sprite, entry->visible);
                break;
            case LAYOUT_ACTION_PARALLAX:
                layout_helper_set_parallax_info(&item->parallax, entry);
                break;
            case LAYOUT_ACTION_MODIFIER:
                pvr_context_helper_copy_modifier(entry->misc.modifier, sprite_matrix_get_modifier(sprite));
                break;
            case LAYOUT_ACTION_STATIC:
                item->static_camera = entry->enable;
                break;
            case LAYOUT_ACTION_SETSHADER:
                // sprite_set_shader(sprite, entry->misc.shader);
                break;
            case LAYOUT_ACTION_REMOVESHADER:
                // sprite_set_shader(sprite, NULL);
                break;
            case LAYOUT_ACTION_SETSHADERUNIFORM:
                /*PSShader psshader = sprite_get_shader(sprite);
                layout_helper_set_shader_uniform(psshader, entry);*/
                break;
            case LAYOUT_ACTION_SETBLENDING:
                if (entry->has_enable) sprite_blend_enable(sprite, entry->enable);
                sprite_blend_set(sprite, entry->blend_src_rgb, entry->blend_dst_rgb, entry->blend_src_alpha, entry->blend_dst_alpha);
                break;
            case LAYOUT_ACTION_SPRITE_TRAILING:
                if (entry->has_enable) sprite_trailing_enabled(sprite, entry->enable);
                sprite_trailing_set_params(sprite, entry->length, entry->trail_delay, entry->trail_alpha, entry->has_darken ? entry->darken : unset);
                break;
            case LAYOUT_ACTION_SPRITE_TRAILINGOFFSETCOLOR:
                sprite_trailing_set_offsetcolor(sprite, entry->rgba[0], entry->rgba[1], entry->rgba[2]);
                break;
        }
    }
}

static void layout_helper_execute_action_in_textsprite(Action* action, Item* item, float viewport_width, float viewport_height) {
    TextSprite textsprite = item->vertex.as_textsprite;
    for (int32_t i = 0; i < action->entries_size; i++) {
        ActionEntry* entry = &action->entries[i];
        switch (entry->type) {
            case LAYOUT_ACTION_TEXTBORDER:
                if (entry->has_enable) textsprite_border_enable(textsprite, entry->enable);
                if (!math2d_is_float_NaN(entry->size)) textsprite_border_set_size(textsprite, entry->size);
                textsprite_border_set_color(
                    textsprite, entry->rgba[0], entry->rgba[1], entry->rgba[2], entry->rgba[3]
                );
                break;
            case LAYOUT_ACTION_TEXTBORDEROFFSET:
                textsprite_border_set_offset(textsprite, entry->x, entry->y);
                break;
            case LAYOUT_ACTION_TEXTBACKGROUND:
                if (entry->has_enable) textsprite_background_enable(textsprite, entry->enable);
                textsprite_background_set_offets(textsprite, entry->x, entry->y);
                if (!math2d_is_float_NaN(entry->size)) textsprite_background_set_size(textsprite, entry->size);
                break;
            case LAYOUT_ACTION_TEXTBACKGROUNDCOLOR:
                textsprite_background_set_color(textsprite, entry->rgba[0], entry->rgba[1], entry->rgba[2], entry->rgba[3]);
                break;
            case LAYOUT_ACTION_PROPERTY:
                if (entry->property == TEXTSPRITE_PROP_STRING)
                    textsprite_set_text_intern(textsprite, true, (const char* const*)&entry->misc.string);
                else
                    textsprite_set_property(textsprite, entry->property, entry->value);
                break;
            case LAYOUT_ACTION_LOCATION:
                float location_x = 0.0f, location_y = 0.0f;
                float draw_width = 0.0f, draw_height = 0.0f;

                // NOTE: there must be text and size already set, otherwise
                // this will not work !
                textsprite_get_draw_size(textsprite, &draw_width, &draw_height);
                layout_helper_location(
                    entry, draw_width, draw_height, viewport_width, viewport_height, &location_x, &location_y
                );
                textsprite_set_draw_location(textsprite, location_x, location_y);
                if (!math2d_is_float_NaN(entry->z)) textsprite_set_z_index(textsprite, entry->z);
                break;
            case LAYOUT_ACTION_COLOR:
                textsprite_set_color(textsprite, entry->rgba[0], entry->rgba[1], entry->rgba[2]);
                if (!math2d_is_float_NaN(entry->rgba[3])) textsprite_set_alpha(textsprite, entry->rgba[3]);
                break;
            case LAYOUT_ACTION_SIZE:
                textsprite_set_max_draw_size(textsprite, entry->width, entry->height);
                break;
            case LAYOUT_ACTION_ANIMATION:
                if (!entry->misc.animsprite && !item->animation) break;
                if (entry->misc.animsprite) item->animation = entry->misc.animsprite;
                if (entry->restart) animsprite_restart(item->animation);
                if (entry->stop_in_loop) animsprite_disable_loop(item->animation);

                textsprite_animation_set(textsprite, entry->misc.animsprite);
                textsprite_calculate_paragraph_alignment(textsprite);
                break;
            case LAYOUT_ACTION_ANIMATIONREMOVE:
                textsprite_animation_set(textsprite, NULL);
                break;
            case LAYOUT_ACTION_ANIMATIONEND:
                textsprite_animation_end(textsprite);
                break;
            case LAYOUT_ACTION_VISIBILITY:
                textsprite_set_visible(textsprite, entry->visible);
                break;
            case LAYOUT_ACTION_PARALLAX:
                layout_helper_set_parallax_info(&item->parallax, entry);
                break;
            case LAYOUT_ACTION_MODIFIER:
                pvr_context_helper_copy_modifier(entry->misc.modifier, textsprite_matrix_get_modifier(textsprite));
                break;
            case LAYOUT_ACTION_STATIC:
                item->static_camera = entry->enable;
                break;
            case LAYOUT_ACTION_SETSHADER:
                // textsprite_set_shader(textsprite, entry->misc.shader);
                break;
            case LAYOUT_ACTION_REMOVESHADER:
                // textsprite_set_shader(textsprite, NULL);
                break;
            case LAYOUT_ACTION_SETSHADERUNIFORM:
                /*PSShader psshader = textsprite_get_shader(textsprite);
                layout_helper_set_shader_uniform(psshader, entry);*/
                break;
            case LAYOUT_ACTION_SETBLENDING:
                if (entry->has_enable) textsprite_blend_enable(textsprite, entry->enable);
                textsprite_blend_set(textsprite, entry->blend_src_rgb, entry->blend_dst_rgb, entry->blend_src_alpha, entry->blend_dst_alpha);
                break;
        }
    }
}

static void layout_helper_execute_action_in_group(Action* action, Group* group) {
    for (int32_t i = 0; i < action->entries_size; i++) {
        ActionEntry* entry = &action->entries[i];
        switch (entry->type) {
            case LAYOUT_ACTION_RESETMATRIX:
                pvr_context_helper_clear_modifier(&group->modifier);
                // sh4matrix_reset(group->matrix);
                break;
            case LAYOUT_ACTION_MODIFIER:
                pvr_context_helper_copy_modifier(entry->misc.modifier, &group->modifier);
                // sh4matrix_apply_modifier(group->matrix, entry.misc.modifier);
                break;
            case LAYOUT_ACTION_PROPERTY:
                layout_helper_group_set_property(group, entry->property, entry->value);
                break;
            case LAYOUT_ACTION_OFFSETCOLOR:
                group->offsetcolor[0] = entry->rgba[0];
                group->offsetcolor[1] = entry->rgba[1];
                group->offsetcolor[2] = entry->rgba[2];
                group->offsetcolor[3] = entry->rgba[3];
                break;
            case LAYOUT_ACTION_PARALLAX:
                layout_helper_set_parallax_info(&group->parallax, entry);
                break;
            case LAYOUT_ACTION_VISIBILITY:
                group->visible = entry->visible;
                break;
            case LAYOUT_ACTION_ANIMATION:
                if (!entry->misc.animsprite && !group->animation) break;
                if (entry->misc.animsprite) group->animation = entry->misc.animsprite;
                if (entry->restart) animsprite_restart(group->animation);
                if (entry->stop_in_loop) animsprite_disable_loop(group->animation);

                layout_helper_group_animate(group, 0.0f);
                break;
            case LAYOUT_ACTION_ANIMATIONREMOVE:
                group->animation = NULL;
                break;
            case LAYOUT_ACTION_ANIMATIONEND:
                if (group->animation) {
                    animsprite_force_end(group->animation);
                    animsprite_update_using_callback(
                        group->animation, group, (PropertySetter)layout_helper_group_set_property, true
                    );
                }
                break;
            case LAYOUT_ACTION_STATIC:
                group->static_camera = entry->enable;
                break;
            case LAYOUT_ACTION_SETSHADER:
                // group->psshader = entry->misc.shader;
                break;
            case LAYOUT_ACTION_REMOVESHADER:
                // group->psshader = NULL;
                break;
            case LAYOUT_ACTION_SETSHADERUNIFORM:
                // layout_helper_set_shader_uniform(group->psshader, entry);
                break;
            case LAYOUT_ACTION_SETBLENDING:
                if (entry->has_enable) group->blend_enabled = entry->enable;
                group->blend_src_rgb = entry->blend_src_rgb;
                group->blend_dst_rgb = entry->blend_dst_rgb;
                group->blend_src_alpha = entry->blend_src_alpha;
                group->blend_dst_alpha = entry->blend_dst_alpha;
                break;
            case LAYOUT_ACTION_VIEWPORT:
                if (!math2d_is_float_NaN(entry->x)) group->viewport_x = entry->x;
                if (!math2d_is_float_NaN(entry->y)) group->viewport_y = entry->y;
                if (!math2d_is_float_NaN(entry->width)) group->viewport_width = entry->width;
                if (!math2d_is_float_NaN(entry->height)) group->viewport_height = entry->height;
                break;
        }
    }
}

static void layout_helper_execute_action_in_sound(Action* action, Item* item) {
    SoundPlayer soundplayer = item->soundplayer;
    for (int32_t i = 0; i < action->entries_size; i++) {
        ActionEntry* entry = &action->entries[i];
        switch (entry->type) {
            case LAYOUT_ACTION_PROPERTY:
                soundplayer_set_property(soundplayer, entry->property, entry->value);
                break;
            case LAYOUT_ACTION_SOUNDFADE:
                soundplayer_fade(soundplayer, entry->enable, entry->size);
                break;
            case LAYOUT_ACTION_SEEK:
                soundplayer_seek(soundplayer, entry->position);
                break;
                /*case LAYOUT_ACTION_ANIMATION:
                    if (!entry->misc.animsprite && !item->animation) break;
                    if (entry->misc.animsprite) item->animation = entry->misc.animsprite;
                    if (entry->restart) animsprite_restart(item->animation);
                    if (entry->stop_in_loop) animsprite_disable_loop(item->animation);

                    soundplayer_animation_set(soundplayer, entry->misc.animsprite);
                    soundplayer_animate(soundplayer, 0.0f);
                    break;
                case LAYOUT_ACTION_ANIMATIONREMOVE:
                    soundplayer_animation_set(soundplayer, NULL);
                    break;
                case LAYOUT_ACTION_ANIMATIONEND:
                    soundplayer_animation_end(soundplayer);
                    break;*/
        }
    }
}

static void layout_helper_execute_action_in_video(Action* action, Item* item_video, Item* item_sprite, float viewport_width, float viewport_height) {
    VideoPlayer videoplayer = item_video->videoplayer;
    for (int32_t i = 0; i < action->entries_size; i++) {
        ActionEntry* entry = &action->entries[i];
        switch (entry->type) {
            case LAYOUT_ACTION_PROPERTY:
                videoplayer_set_property(videoplayer, entry->property, entry->value);
                break;
            case LAYOUT_ACTION_SOUNDFADE:
                videoplayer_fade_audio(videoplayer, entry->enable, entry->size);
                break;
            case LAYOUT_ACTION_SEEK:
                videoplayer_seek(videoplayer, entry->position);
                break;
            default:
                layout_helper_execute_action_in_sprite(action, item_sprite, viewport_width, viewport_height);
                break;
        }
    }
}




//////////////////////////////////
///    ACTION ENTRY PARSERS    ///
//////////////////////////////////

static void layout_helper_add_action_property(XmlNode unparsed_entry, bool is_textsprite, ArrayList action_entries) {
    int32_t property_id = vertexprops_parse_sprite_property(unparsed_entry, "name", !is_textsprite);
    if (property_id == -1 && is_textsprite) {
        property_id = vertexprops_parse_textsprite_property(unparsed_entry, "name", true);
    } else if (property_id == -1) {
        property_id = vertexprops_parse_layout_property(unparsed_entry, "name", true);
    }

    if (property_id < 0) return;

    layout_helper_parse_property(unparsed_entry, property_id, "value", action_entries);
}

static void layout_helper_add_action_properties(XmlNode unparsed_entry, bool is_textsprite, ArrayList action_entries) {
    if (xmlparser_get_attribute_count(unparsed_entry) < 1) {
        logger_warn_xml(
            "layout_helper_add_action_properties() 'Properties' was empty:", unparsed_entry
        );
        return;
    }

    foreach (XmlAttribute, attribute, XMLPARSER_ATTRIBUTES_ITERATOR, unparsed_entry) {
        const char* name = attribute.name;
        int32_t property_id;

        property_id = vertexprops_parse_sprite_property2(name);
        if (property_id == -1 && is_textsprite) {
            property_id = vertexprops_parse_textsprite_property2(name);
        } else if (property_id == -1) {
            property_id = vertexprops_parse_layout_property2(name);
        }

        if (property_id < 0) {
            logger_warn_xml(
                "layout_helper_add_action_properties() unknown property '%s' in:",
                unparsed_entry, name
            );
            continue;
        }

        layout_helper_parse_property(unparsed_entry, property_id, name, action_entries);
    }
}

static void layout_helper_add_action_offsetcolor(XmlNode unparsed_entry, ArrayList action_entries) {
    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_OFFSETCOLOR,
        .rgba = {FLOAT_NaN, FLOAT_NaN, FLOAT_NaN, FLOAT_NaN}
    };
    layout_helper_parse_color(unparsed_entry, entry.rgba);
    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_color(XmlNode unparsed_entry, ArrayList action_entries) {
    ActionEntry entry = (ActionEntry){
        type : LAYOUT_ACTION_COLOR,
        rgba : {FLOAT_NaN, FLOAT_NaN, FLOAT_NaN, FLOAT_NaN}
    };
    layout_helper_parse_color(unparsed_entry, entry.rgba);
    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_modifier(XmlNode unparsed_entry, ArrayList action_entries) {
    Modifier* modifier_mask = malloc_chk(sizeof(Modifier));
    malloc_assert(modifier_mask, Modifier);

    ActionEntry action_entry = (ActionEntry){.type = LAYOUT_ACTION_MODIFIER, .misc.modifier = modifier_mask};

    pvr_context_helper_clear_modifier(modifier_mask);
    pvr_context_helper_invalidate_modifier(modifier_mask);

    foreach (XmlAttribute, unparsed_attribute, XMLPARSER_ATTRIBUTES_ITERATOR, unparsed_entry) {
        const char* attribute = unparsed_attribute.name;
        float value;

        if (
            string_equals_ignore_case(unparsed_attribute.name, "rotatepivotenabled") ||
            string_equals_ignore_case(unparsed_attribute.name, "scalesize") ||
            string_equals_ignore_case(unparsed_attribute.name, "scaletranslation") ||
            string_equals_ignore_case(unparsed_attribute.name, "translaterotation")
        ) {
            value = vertexprops_parse_boolean(unparsed_entry, attribute, false);
        } else {
            value = layout_helper_parse_float(unparsed_entry, attribute, FLOAT_NaN);
            if (math2d_is_float_NaN(value)) continue;
        }

        if (string_equals_ignore_case(unparsed_attribute.name, "x")) {
            modifier_mask->x = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "y")) {
            modifier_mask->y = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "translate")) {
            modifier_mask->translate_x = modifier_mask->translate_y = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "translatex")) {
            modifier_mask->translate_x = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "translatey")) {
            modifier_mask->translate_y = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "scale")) {
            modifier_mask->scale_x = modifier_mask->scale_y = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "scalex")) {
            modifier_mask->scale_x = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "scaley")) {
            modifier_mask->scale_y = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "skew")) {
            modifier_mask->skew_x = modifier_mask->skew_y = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "skewx")) {
            modifier_mask->skew_x = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "skewy")) {
            modifier_mask->skew_y = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "rotate")) {
            modifier_mask->rotate = value * MATH2D_DEG_TO_RAD;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "scaledirection")) {
            modifier_mask->scale_direction_x = modifier_mask->scale_direction_y = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "scaledirectionx")) {
            modifier_mask->scale_direction_x = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "scaledirectiony")) {
            modifier_mask->scale_direction_y = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "rotatepivot")) {
            modifier_mask->rotate_pivot_u = modifier_mask->rotate_pivot_v = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "rotatepivotu")) {
            modifier_mask->rotate_pivot_u = value;
        } else if (string_equals_ignore_case(unparsed_attribute.name, "rotatepivotv")) {
            modifier_mask->rotate_pivot_v = value;
        }
    }

    arraylist_add(action_entries, &action_entry);
}

static void layout_helper_add_action_parallax(XmlNode unparsed_entry, ArrayList action_entries) {
    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_PARALLAX,
        .x = FLOAT_NaN,
        .y = FLOAT_NaN,
        .z = FLOAT_NaN
    };

    foreach (XmlAttribute, unparsed_attribute, XMLPARSER_ATTRIBUTES_ITERATOR, unparsed_entry) {
        const char* name = unparsed_attribute.name;
        float value = layout_helper_parse_float(unparsed_entry, name, FLOAT_NaN);

        if (math2d_is_float_NaN(value)) continue;

        if (string_equals(name, "x")) {
            entry.x = value;
        } else if (string_equals(name, "y")) {
            entry.y = value;
        } else if (string_equals(name, "z")) {
            entry.z = value;
        }
    }

    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_location(XmlNode unparsed_entry, ArrayList action_entries) {
    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_LOCATION,
        .align_vertical = layout_helper_parse_align(unparsed_entry, true),
        .align_horizontal = layout_helper_parse_align(unparsed_entry, false),
        .x = layout_helper_parse_float(unparsed_entry, "x", 0.0f),
        .y = layout_helper_parse_float(unparsed_entry, "y", 0.0f),
        .z = layout_helper_parse_float(unparsed_entry, "z", FLOAT_NaN),
    };
    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_size(XmlNode unparsed_entry, ArrayList action_entries) {
    float width = layout_helper_parse_float(unparsed_entry, "width", FLOAT_NaN);
    float height = layout_helper_parse_float(unparsed_entry, "height", FLOAT_NaN);
    if (math2d_is_float_NaN(width) && math2d_is_float_NaN(height)) {
        logger_error_xml("layout_helper_add_action_size() invalid size:", unparsed_entry);
        return;
    }

    bool has_resize = width < 0.0f || height < 0.0f;
    if (has_resize && (math2d_is_float_NaN(width) || math2d_is_float_NaN(height))) {
        logger_warn_xml("layout_helper_add_action_size() invalid resize:", unparsed_entry);
        return;
    }

    ActionEntry entry = (ActionEntry){.type = LAYOUT_ACTION_SIZE, .width = width, .height = height, .has_resize = has_resize};
    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_visibility(XmlNode unparsed_entry, ArrayList action_entries) {
    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_VISIBILITY,
        .visible = !string_equals(xmlparser_get_tag_name(unparsed_entry), "Hide")
    };
    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_atlasapply(XmlNode unparsed_entry, Atlas atlas, ArrayList action_entries) {
    if (!atlas) {
        logger_error_xml(
            "layout_helper_add_action_atlasapply() missing atlas, can not import:",
            unparsed_entry
        );
        return;
    }

    const char* atlas_entry_name = xmlparser_get_attribute_value2(unparsed_entry, "entry");
    const AtlasEntry* atlas_entry = atlas_get_entry(atlas, atlas_entry_name);

    if (!atlas_entry) {
        logger_warn_xml("layout_helper_add_action_atlasapply() missing atlas entry name '%s':", unparsed_entry, atlas_entry_name);
        return;
    }

    AtlasEntry* atlas_entry_copy = CLONE_STRUCT(struct AtlasEntry_s, AtlasEntry*, atlas_entry);
    atlas_entry_copy->name = NULL; // unused

    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_ATLASAPPLY,
        .misc.atlas_entry = atlas_entry_copy,
        .override_size = vertexprops_parse_boolean(unparsed_entry, "overrideSize", false)
    };

    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_animation(XmlNode unparsed_entry, AnimList animlist, ArrayList action_entries) {
    if (!animlist) {
        logger_error("layout_helper_add_action_animation() failed, missing animlist");
        return;
    }

    const char* anim_name = xmlparser_get_attribute_value2(unparsed_entry, "name");
    AnimSprite animsprite = animsprite_init_from_animlist(animlist, anim_name);

    if (!animsprite) {
        logger_warn_xml("layout_helper_add_action_animation() missing animation '%s':", unparsed_entry, anim_name);
        return;
    }

    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_ANIMATION,
        .misc.animsprite = animsprite,
        .restart = vertexprops_parse_boolean(unparsed_entry, "restart", true),
        .stop_in_loop = vertexprops_parse_boolean(unparsed_entry, "stopOnLoop", false)
    };

    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_animationfromatlas(XmlNode unparsed_entry, Atlas atlas, ArrayList action_entries) {
    if (!atlas) {
        logger_error_xml(
            "layout_helper_add_action_animationfromatlas() failed, sprite has no atlas.", unparsed_entry
        );
        return;
    }

    const char* anim_name = xmlparser_get_attribute_value2(unparsed_entry, "name");
    if (!anim_name) {
        logger_error_xml("layout_helper_add_action_animationfromatlas() missing animation name on:", unparsed_entry);
        return;
    }

    int32_t loop = vertexprops_parse_integer(unparsed_entry, "loop", 1);
    bool has_number_suffix = vertexprops_parse_boolean(unparsed_entry, "hasNumberSuffix", true);
    float fps = vertexprops_parse_float(unparsed_entry, "fps", 0.0f);

    if (fps < 1.0f) {
        fps = atlas_get_glyph_fps(atlas);
        if (fps < 1.0f) fps = FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE;
    }

    AnimSprite animsprite = animsprite_init_from_atlas(fps, loop, atlas, anim_name, has_number_suffix);
    bool override_size = vertexprops_parse_boolean(unparsed_entry, "overrideSize", false);

    if (!animsprite) {
        logger_warn_xml("layout_helper_add_action_animationfromatlas() missing animation '%s':", unparsed_entry, anim_name);
        return;
    }

    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_ANIMATION,
        .misc.animsprite = animsprite,
        .restart = vertexprops_parse_boolean(unparsed_entry, "restart", true),
        .stop_in_loop = vertexprops_parse_boolean(unparsed_entry, "stopOnLoop", false)
    };

    animsprite_allow_override_sprite_size(animsprite, override_size);

    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_animationremove(XmlNode unparsed_entry, ArrayList action_entries) {
    (void)unparsed_entry;
    ActionEntry entry = (ActionEntry){.type = LAYOUT_ACTION_ANIMATIONREMOVE};
    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_animationend(XmlNode unparsed_entry, ArrayList action_entries) {
    (void)unparsed_entry;
    ActionEntry entry = (ActionEntry){.type = LAYOUT_ACTION_ANIMATIONEND};
    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_textborder(XmlNode unparsed_entry, ArrayList action_entries) {
    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_TEXTBORDER,
        .size = layout_helper_parse_float(unparsed_entry, "size", FLOAT_NaN),
        .has_enable = xmlparser_has_attribute(unparsed_entry, "enable"),
        .enable = vertexprops_parse_boolean(unparsed_entry, "enable", false),
        .rgba = {FLOAT_NaN, FLOAT_NaN, FLOAT_NaN, FLOAT_NaN}
    };

    float alpha = layout_helper_parse_float(unparsed_entry, "alpha", FLOAT_NaN);
    layout_helper_parse_color(unparsed_entry, entry.rgba);

    if (!math2d_is_float_NaN(alpha)) entry.rgba[3] = alpha;

    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_resize(XmlNode unparsed_entry, ArrayList action_entries) {
    float max_width = layout_helper_parse_float(unparsed_entry, "maxWidth", FLOAT_NaN);
    float max_height = layout_helper_parse_float(unparsed_entry, "maxHeight", FLOAT_NaN);

    if (math2d_is_float_NaN(max_width) && math2d_is_float_NaN(max_height)) {
        logger_error_xml("layout_helper_add_action_resize() invalid resize:", unparsed_entry);
        return;
    }

    if (math2d_is_float_NaN(max_width)) max_width = -1.0f;
    if (math2d_is_float_NaN(max_height)) max_height = -1.0f;

    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_RESIZE,
        .max_width = max_width,
        .max_height = max_height,
        .cover = vertexprops_parse_boolean(unparsed_entry, "cover", false),
        .center = vertexprops_parse_boolean(unparsed_entry, "center", false)
    };

    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_resetmatrix(XmlNode unparsed_entry, ArrayList action_entries) {
    (void)unparsed_entry;
    ActionEntry entry = (ActionEntry){.type = LAYOUT_ACTION_RESETMATRIX};
    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_static(XmlNode unparsed_entry, ArrayList action_entries) {
    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_STATIC,
        .enable = vertexprops_parse_boolean(unparsed_entry, "enable", true)
    };
    arraylist_add(action_entries, &entry);
}

static bool layout_helper_add_action_media(XmlNode unparsed_entry, ArrayList action_entries) {
    float value;

    const char* tagName = xmlparser_get_tag_name(unparsed_entry);
    if (string_equals(tagName, "Play")) {
        value = PLAYBACK_PLAY;
    } else if (string_equals(tagName, "Pause")) {
        value = PLAYBACK_PAUSE;
    } else if (string_equals(tagName, "Stop")) {
        value = PLAYBACK_STOP;
    } else if (string_equals(tagName, "Mute")) {
        value = PLAYBACK_MUTE;
    } else if (string_equals(tagName, "Unmute")) {
        value = PLAYBACK_UNMUTE;
    } else {
        return true;
    }

    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_PROPERTY,
        .property = MEDIA_PROP_PLAYBACK,
        .value = value,
        .misc = {NULL},
    };

    arraylist_add(action_entries, &entry);
    return false;
}

static void layout_helper_add_action_mediaproperty(XmlNode unparsed_entry, ArrayList action_entries) {
    int32_t property_id = vertexprops_parse_media_property(unparsed_entry, "name", false);
    if (property_id < 0) return;
    layout_helper_parse_property(unparsed_entry, property_id, "value", action_entries);
}

static void layout_helper_add_action_mediaproperties(XmlNode unparsed_entry, ArrayList action_entries) {
    if (xmlparser_get_attribute_count(unparsed_entry) < 1) {
        logger_warn_xml(
            "layout_helper_add_action_properties() 'Properties' was empty:", unparsed_entry
        );
        return;
    }

    foreach (XmlAttribute, attribute, XMLPARSER_ATTRIBUTES_ITERATOR, unparsed_entry) {
        const char* name = attribute.name;
        int32_t property_id;

        property_id = vertexprops_parse_media_property2(name);

        if (property_id < 0) {
            logger_warn_xml(
                "layout_helper_add_action_mediaproperties() unknown property '%s' in:",
                unparsed_entry, name
            );
            continue;
        }

        layout_helper_parse_property(unparsed_entry, property_id, name, action_entries);
    }
}

static void layout_helper_add_action_setshader(XmlNode unparsed_entry, ArrayList action_entries) {
    const char* shader_vertex_src = xmlparser_get_attribute_value2(unparsed_entry, "vertexSrc");
    const char* shader_fragment_src = xmlparser_get_attribute_value2(unparsed_entry, "fragmentSrc");

    if (!shader_fragment_src && !shader_vertex_src && xmlparser_get_children_count(unparsed_entry) < 1) {
        layout_helper_add_action_removeshader(unparsed_entry, action_entries);
        return;
    }

    StringBuilder sourcecode_vertex = stringbuilder_init(0);
    StringBuilder sourcecode_fragment = stringbuilder_init(0);

    if (shader_vertex_src) {
        char* tmp = fs_readtext(shader_vertex_src);
        if (tmp) stringbuilder_add(sourcecode_vertex, tmp);
        free_chk(tmp);
    }

    if (shader_fragment_src) {
        char* tmp = fs_readtext(shader_fragment_src);
        if (tmp) stringbuilder_add(sourcecode_fragment, tmp);
        free_chk(tmp);
    }

    // parse source elements
    foreach (XmlNode, source, XMLPARSER_CHILDREN_ITERATOR, unparsed_entry) {
        StringBuilder target;
        const char* tagName = xmlparser_get_tag_name(source);
        if (string_equals(tagName, "VertexSource")) {
            target = sourcecode_vertex;
        } else if (string_equals(tagName, "FragmentSource")) {
            target = sourcecode_fragment;
        } else {
            logger_warn_xml("layout_helper_add_action_setshader() unknown element:", source);
            continue;
        }

        char* textContent = xmlparser_get_textContext(source);
        stringbuilder_add_char_codepoint(target, 0x0A); // newline char
        stringbuilder_add(target, textContent);
        free_chk(textContent);
    }

    char* str_vertex = stringbuilder_finalize(&sourcecode_vertex);
    char* str_fragment = stringbuilder_finalize(&sourcecode_fragment);

    if (!str_vertex && !str_fragment) {
        free_chk(str_vertex);
        free_chk(str_fragment);
        logger_warn_xml("layout_helper_add_action_setshader() empty shader:", unparsed_entry);
        return;
    }

    // assume layout as part of the main PVRContext renderer
    // PSShader psshader = PSShader.BuildFromSource(pvr_context, str_vertex, str_fragment);
    void* psshader = NULL;
    logger_info("layout_helper_add_action_setshader() GLSL shaders are not supported in the dreamcast");

    free_chk(str_vertex);
    free_chk(str_fragment);

    if (!psshader) {
        logger_warn_xml("layout_helper_add_action_setshader() compilation failed:", unparsed_entry);
        return;
    }

    // ActionEntry entry = (ActionEntry){.type = LAYOUT_ACTION_SETSHADER, .misc.shader = psshader};
    ActionEntry entry = (ActionEntry){.type = LAYOUT_ACTION_SETSHADER, .misc = {NULL}};
    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_removeshader(XmlNode unparsed_entry, ArrayList action_entries) {
    (void)unparsed_entry;
    ActionEntry entry = (ActionEntry){.type = LAYOUT_ACTION_REMOVESHADER};
    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_setshaderuniform(XmlNode unparsed_entry, ArrayList action_entries) {
    float64* values = calloc_for_array(16, float64);

    const char* name = xmlparser_get_attribute_value2(unparsed_entry, "name");
    float64 value = vertexprops_parse_double(unparsed_entry, "value", DOUBLE_NaN);
    const char* unparsed_values = xmlparser_get_attribute_value2(unparsed_entry, "values");

    if (!name) {
        logger_error_xml("layout_helper_add_action_setshaderuniform() missing name:", unparsed_entry);
        return;
    }

    if (unparsed_values != NULL) {
        // separator: white-space, hard-space, tabulation, carrier-return, new-line
        Tokenizer tokenizer = tokenizer_init("\x20\u00A0\t\r\n", true, false, (char*)unparsed_values);
        int32_t index = 0;
        char* str;

        while ((str = tokenizer_read_next(tokenizer)) != NULL) {
            float64 temp_value = vertexprops_parse_double2(str, DOUBLE_NaN);
            if (math2d_is_double_NaN(temp_value)) {
                logger_warn("layout_helper_add_action_setshaderuniform() invalid value: %s", str);
                temp_value = 0.0;
            }

            free_chk(str);
            values[index++] = temp_value;
            if (index >= 16) break;
        }
        tokenizer_destroy(&tokenizer);
    } else if (!math2d_is_double_NaN(value)) {
        values[0] = value;
    }

    free_chk(values);
    logger_info("layout_helper_add_action_setshaderuniform() GLSL shaders are not supported in the dreamcast");

    ActionEntry entry = (ActionEntry){.type = LAYOUT_ACTION_SETSHADERUNIFORM, /*.uniform_name = name, .misc = values*/};
    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_setblending(XmlNode unparsed_entry, ArrayList action_entries) {
    bool has_enable = xmlparser_has_attribute(unparsed_entry, "enabled");
    bool enabled = vertexprops_parse_boolean(unparsed_entry, "enabled", true); // must be true by default

    Blend blend_src_rgb = vertexprops_parse_blending(xmlparser_get_attribute_value2(unparsed_entry, "srcRGB"));
    Blend blend_dst_rgb = vertexprops_parse_blending(xmlparser_get_attribute_value2(unparsed_entry, "dstRGB"));
    Blend blend_src_alpha = vertexprops_parse_blending(xmlparser_get_attribute_value2(unparsed_entry, "srcAlpha"));
    Blend blend_dst_alpha = vertexprops_parse_blending(xmlparser_get_attribute_value2(unparsed_entry, "dstAlpha"));

    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_SETBLENDING,
        .enabled = enabled,
        .has_enable = has_enable,
        .blend_src_rgb = blend_src_rgb,
        .blend_dst_rgb = blend_dst_rgb,
        .blend_src_alpha = blend_src_alpha,
        .blend_dst_alpha = blend_dst_alpha
    };

    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_viewport(XmlNode unparsed_entry, ArrayList action_entries) {
    float x = vertexprops_parse_float(unparsed_entry, "x", FLOAT_NaN);
    float y = vertexprops_parse_float(unparsed_entry, "y", FLOAT_NaN);
    float width = vertexprops_parse_float(unparsed_entry, "width", FLOAT_NaN);
    float height = vertexprops_parse_float(unparsed_entry, "height", FLOAT_NaN);

    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_VIEWPORT,
        .x = x,
        .y = y,
        .width = width,
        .height = height
    };

    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_textborderoffset(XmlNode unparsed_entry, ArrayList action_entries) {
    float offset_x = layout_helper_parse_float(unparsed_entry, "offsetX", FLOAT_NaN);
    float offset_y = layout_helper_parse_float(unparsed_entry, "offsetY", FLOAT_NaN);
    if (math2d_is_float_NaN(offset_x) && math2d_is_float_NaN(offset_y)) {
        logger_error_xml("layout_helper_add_action_borderoffser() invalid offset:", unparsed_entry);
        return;
    }

    ActionEntry entry = (ActionEntry){.type = LAYOUT_ACTION_TEXTBORDEROFFSET, .x = offset_x, .y = offset_y};
    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_textbackground(XmlNode unparsed_entry, ArrayList action_entries) {
    bool enable = vertexprops_parse_boolean(unparsed_entry, "enable", false);
    bool has_enable = xmlparser_has_attribute(unparsed_entry, "enable");
    float size = layout_helper_parse_float(unparsed_entry, "size", FLOAT_NaN);
    float offset_x = layout_helper_parse_float(unparsed_entry, "offsetX", FLOAT_NaN);
    float offset_y = layout_helper_parse_float(unparsed_entry, "offsetY", FLOAT_NaN);

    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_TEXTBACKGROUND,
        .size = size,
        .x = offset_x,
        .y = offset_y,
        .has_enable = has_enable,
        .enable = enable
    };
    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_textbackgroundcolor(XmlNode unparsed_entry, ArrayList action_entries) {
    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_TEXTBACKGROUNDCOLOR,
        .rgba = {FLOAT_NaN, FLOAT_NaN, FLOAT_NaN, FLOAT_NaN}
    };

    layout_helper_parse_color(unparsed_entry, entry.rgba);

    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_spritetrailing(XmlNode unparsed_entry, ArrayList action_entries) {
    bool enable = vertexprops_parse_boolean(unparsed_entry, "enable", false);
    bool has_enable = xmlparser_has_attribute(unparsed_entry, "enable");
    int32_t length = vertexprops_parse_integer(unparsed_entry, "length", -1);
    float trail_delay = vertexprops_parse_float(unparsed_entry, "trailDelay", FLOAT_NaN);
    float trail_alpha = vertexprops_parse_float(unparsed_entry, "trailAlpha", FLOAT_NaN);
    bool darken = vertexprops_parse_boolean(unparsed_entry, "darkenColors", false);
    bool has_darken = xmlparser_has_attribute(unparsed_entry, "darkenColors");

    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_SPRITE_TRAILING,
        .enable = enable,
        .has_enable = has_enable,
        .length = length,
        .trail_delay = trail_delay,
        .trail_alpha = trail_alpha,
        .darken = darken,
        .has_darken = has_darken
    };

    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_spritetrailingoffsetcolor(XmlNode unparsed_entry, ArrayList action_entries) {
    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_SPRITE_TRAILINGOFFSETCOLOR,
        .rgba = {FLOAT_NaN, FLOAT_NaN, FLOAT_NaN, FLOAT_NaN}
    };

    layout_helper_parse_color(unparsed_entry, entry.rgba);

    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_soundfade(XmlNode unparsed_entry, ArrayList action_entries) {
    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_SOUNDFADE,
        .enable = string_equals(xmlparser_get_tag_name(unparsed_entry), "FadeIn"),
        .size = layout_helper_parse_float(unparsed_entry, "duration", 1000.0f)
    };

    arraylist_add(action_entries, &entry);
}

static void layout_helper_add_action_seek(XmlNode unparsed_entry, ArrayList action_entries) {
    ActionEntry entry = (ActionEntry){
        .type = LAYOUT_ACTION_SEEK,
        .position = vertexprops_parse_double(unparsed_entry, "position", 0.0)
    };

    arraylist_add(action_entries, &entry);
}
