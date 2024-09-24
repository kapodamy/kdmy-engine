#include "game/gameplay/dialogue.h"

#include "animsprite.h"
#include "arraylist.h"
#include "drawable.h"
#include "externals/luascript.h"
#include "fontglyph.h"
#include "fontholder.h"
#include "fonttype.h"
#include "fs.h"
#include "game/common/funkin.h"
#include "game/gameplay/weekscript.h"
#include "gamepad.h"
#include "imgutils.h"
#include "jsonparser.h"
#include "linkedlist.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "pvrcontext.h"
#include "sh4matrix.h"
#include "soundplayer.h"
#include "statesprite.h"
#include "stringbuilder.h"
#include "stringutils.h"
#include "textsprite.h"
#include "texture.h"
#include "tokenizer.h"
#include "unused_switch_case.h"
#include "vertexprops.h"
#include "xmlparser.h"


#define HAS_DUPL_ENTRY(ARRAYLIST, INDEX, TYPE) string_equals( \
    (                                                         \
        (TYPE*)arraylist_get(ARRAYLIST, INDEX)                \
    )                                                         \
        ->name,                                               \
    name                                                      \
)

#define XML_TAG_NAME_EQUALS(NODE, VALUE) string_equals(xmlparser_get_tag_name(NODE), VALUE)
#define XML_ATTR_VAL_EQUALS(NODE, NAME, VALUE) string_equals(xmlparser_get_attribute_value2(NODE, NAME), VALUE)


static const char* DIALOGUE_IDLE = "idle";
static const char* DIALOGUE_SPEAK = "speak";
static const char* DIALOGUE_ICON = "icon";
static const char* DIALOGUE_OPEN = "open";
// static const char* DIALOGUE_LAYOUT = "/assets/common/image/dialogue/layout.xml";
// static const char* DIALOGUE_LAYOUT_DREAMCAST = "/assets/common/image/dialogue/layout~dreamcast.xml";
static const char* DIALOGUE_DEFAULTS_DEFINITIONS = "/assets/common/data/dialogue_defaults.xml";


typedef enum {
    Type_AUDIO_PLAY,
    Type_AUDIO_PAUSE,
    Type_AUDIO_FADEIN,
    Type_AUDIO_FADEOUT,
    Type_AUDIO_STOP,
    Type_AUDIO_VOLUME,
    Type_BACKGROUND_SET,
    Type_BACKGROUND_CHANGE,
    Type_BACKGROUND_REMOVE,
    Type_LUA,
    Type_EXIT,
    Type_PORTRAIT_ADD,
    Type_PORTRAIT_REMOVE,
    Type_PORTRAIT_REMOVEALL,
    Type_AUDIO_UI,
    Type_TEXT_SPEED,
    Type_TEXT_SKIP,
    Type_TEXT_INSTANTPRINT,
    Type_SPEECH_BACKGROUND,
    Type_TEXT_FONT,
    Type_TEXT_COLOR,
    Type_TEXT_BORDERCOLOR,
    Type_TEXT_BORDEROFFSET,
    Type_TEXT_PARAGRAPHSPACE,
    Type_TEXT_SIZE,
    Type_TEXT_ALIGN,
    Type_RUNMULTIPLECHOICE,
    Type_TITLE,
    Type_NOWAIT,
    Type_TEXT_BORDERSIZE,
    Type_TEXT_BORDERENABLE
} Type;

typedef enum {
    RepeatAnim_ONCE,
    RepeatAnim_WHILESPEAKS,
    RepeatAnim_ALWAYS,
    RepeatAnim_NONE
} RepeatAnim;

typedef struct {
    char* name;
    bool was_playing;
    SoundPlayer soundplayer;
} Audio;

typedef struct {
    char* name;
    StateSprite statesprite;
    float position;
    bool is_added;
    bool is_removed;
    bool is_speaking;
    float offset_speak_x;
    float offset_speak_y;
    float offset_idle_x;
    float offset_idle_y;
    bool has_speak;
    bool has_idle;
    bool speak_anim_looped;
    bool idle_anim_looped;
} Portrait;

typedef struct {
    AnimSprite portrait_left_in;
    AnimSprite portrait_center_in;
    AnimSprite portrait_right_in;
    AnimSprite portrait_left_out;
    AnimSprite portrait_center_out;
    AnimSprite portrait_right_out;
    AnimSprite background_in;
    AnimSprite background_out;
    AnimSprite open;
    AnimSprite close;
} AnimsUI;

typedef struct {
    Type type;
    RepeatAnim repeat_anim;
    char* name;
    char* random_from_prefix;
    char* click_text;
    char* click_char;
    char* lua_eval;
    char* lua_function;
    char* title;
    float duration;
    float volume;
    int32_t chars_per_second;
    int32_t char_delay;
    RGBA rgba;
    float size;
    Align align_vertical;
    Align align_horizontal;
    Align align_paragraph;
    bool no_speak;
    bool animate_remove;
    bool enabled;
    float offset_x;
    float offset_y;
} Action;

typedef struct {
    char* name;
    char* if_line;
    bool initial;
    Action* actions;
    int32_t actions_size;
} State;

typedef struct {
    int32_t dialog_id;
    bool exit;
    char* lua_eval;
    char* lua_function;
    char* run_multiple_choice;
    char* text;
} Choice;

typedef struct {
    Sprite icon;
    Sprite hint;
    char* state_on_leave;
    char* title;
    char* name;
    bool can_leave;
    bool is_vertical;
    int32_t default_index;
    float font_size;
    Choice* choices;
    int32_t choices_size;
} MultipleChoice;

typedef struct {
    char* target_state_name;
    char* text;
} DialogLine;

typedef struct {
    char* full_path;
    DialogLine* lines;
    int32_t lines_size;
} Dialog;

typedef struct {
    char* name;
    FontHolder fontholder;
} Font;

typedef struct {
    char* name;
    Sprite sprite;
} Background;

typedef struct {
    char* name;
    StateSprite statesprite;
    float offset_x;
    float offset_y;
    float text_x;
    float text_y;
    float text_width;
    float text_height;
    float title_x;
    float title_y;
    float portrait_line_x;
    float portrait_line_y;
    float portrait_line_width;
    Align align_vertical;
    Align align_horizontal;
    bool portrait_line_is_relative;
    bool title_is_relative;
    bool text_is_relative;
} SpeechImage;


struct Dialogue_s {
    SH4Matrix matrix_viewport;

    Audio* audios;
    int32_t audios_size;
    Font* fonts;
    int32_t fonts_size;
    Background* backgrounds;
    int32_t backgrounds_size;
    Portrait* portraits;
    int32_t portraits_size;
    Dialog* dialogs;
    int32_t dialogs_size;
    State* states;
    int32_t states_size;
    MultipleChoice* multiplechoices;
    int32_t multiplechoices_size;
    SpeechImage* speechimages;
    int32_t speechimages_size;
    AnimsUI anims_ui;

    LinkedList /*<Portrait>*/ visible_portraits;
    bool is_completed;
    Dialog* dialog_external;

    int32_t current_background;
    int32_t change_background_from;
    Luascript script;
    bool do_exit;
    SoundPlayer click_text;
    SoundPlayer click_char;
    int32_t char_delay;
    int32_t chars_per_second;
    bool do_skip;
    bool do_instant_print;
    bool do_no_wait;
    MultipleChoice* do_multiplechoice;
    TextSprite texsprite_speech;
    TextSprite texsprite_title;
    SpeechImage* current_speechimage;
    bool current_speechimage_is_opening;
    RepeatAnim current_speechimage_repeat;
    bool is_speaking;
    int32_t current_dialog_codepoint_index;
    int32_t current_dialog_codepoint_length;
    float64 current_dialog_duration;
    float64 current_dialog_elapsed;
    char* current_dialog_mask;
    Dialog* current_dialog;
    int32_t current_dialog_line;
    StringBuilder current_dialog_buffer;
    bool draw_portraits_on_top;
    Gamepad gamepad;
    Drawable self_drawable;
    bool self_hidden;
};


static bool dialogue_internal_prepare_dialog(Dialogue dialogue);
static bool dialogue_internal_apply_state(Dialogue dialogue, State* state);
static void dialogue_internal_draw_background(Dialogue dialogue, PVRContext pvrctx);
static void dialogue_internal_draw_portraits(Dialogue dialogue, PVRContext pvrctx);
static void dialogue_internal_draw_speech(Dialogue dialogue, PVRContext pvrctx);
static void dialogue_internal_prepare_print_text(Dialogue dialogue);
static void dialogue_internal_print_text(Dialogue dialogue);
static void dialogue_internal_toggle_idle(Dialogue dialogue);
static void dialogue_internal_destroy_external_dialog(Dialogue dialogue);
static void dialogue_internal_parse_external_dialog(Dialogue dialogue, const char* source);
static void dialogue_internal_notify_script(Dialogue dialogue, bool is_line_start);
static void dialogue_internal_parse_audiolist(XmlNode root_node, ArrayList audios);
static void dialogue_internal_parse_backgroundlist(XmlNode root_node, ArrayList backgrounds);
static void dialogue_internal_parse_portraitlist(XmlNode root_node, ArrayList portraits);
static void dialogue_internal_parse_animationui(XmlNode root_node, AnimsUI* anims_ui);
static void dialogue_internal_parse_state(XmlNode root_node, ArrayList states);
static void dialogue_internal_parse_multiplechoice(XmlNode root_node, AnimList animlist, ArrayList dialogs, ArrayList multiplechoices);
static void dialogue_internal_parse_speech_imagelist(XmlNode root_node, ArrayList speechimages);
static void dialogue_internal_parse_import_portraitlist(XmlNode root_node, ArrayList portraits);
static void dialogue_internal_parse_audio(XmlNode node, ArrayList audios);
static void dialogue_internal_parse_image(XmlNode node, float max_width, float max_height, const char* base_src, ArrayList backgrounds);
static void dialogue_internal_parse_color(XmlNode node, float max_width, float max_height, ArrayList backgrounds);
static void dialogue_internal_parse_font(XmlNode node, ArrayList fonts);
static void dialogue_internal_parse_portrait(XmlNode node, const char* base_model, ArrayList portraits);
static void dialogue_internal_parse_animation_uiset(XmlNode node, AnimList animlist, AnimsUI* anims_ui);
static void dialogue_internal_parse_choice(XmlNode node, ArrayList dialogs, ArrayList choices);
static SpeechImage* dialogue_internal_parse_speech_image(XmlNode node, const char* base_src, ArrayList speechimages);
static void dialogue_internal_load_psych_character_json(const char* src, ArrayList portraits);
static int32_t dialogue_internal_parse_dialog_from_file(const char* src, ArrayList dialogs);
static void dialogue_internal_parse_dialog_from_string(const char* source, Dialog* dialog_ref);
static void dialogue_internal_read_color(XmlNode node, float* rgba);
static Align dialogue_internal_read_align(XmlNode node, const char* attribute);
static void dialogue_internal_read_offset(JSONToken json_obj, const char* property, float* offset_x, float* offset_y);
static void dialogue_internal_compute_state(ModelHolder mdlhldr, StateSprite sttsprt, AnimSprite anim, float offst_x, float offst_y, bool is_speak);
static void dialogue_internal_add_state(StateSprite statesprite, ModelHolder modelholder, const char* anim_name, const char* state_name, float scale, bool looped);
static void dialogue_internal_stop_portrait_animation(Portrait* portrait);
static Audio* dialogue_internal_get_audio(Dialogue dialogue, const char* name);
static FontHolder dialogue_internal_get_font(Dialogue dialogue, const char* name);
static int32_t dialogue_internal_get_background_index(Dialogue dialogue, const char* name);
static MultipleChoice* dialogue_internal_get_multiplechoice(Dialogue dialogue, const char* name);


Dialogue dialogue_init(const char* src, float viewport_width, float viewport_height) {
    src = fs_get_full_path(src);
    XmlParser xml = xmlparser_init(src);
    if (!xml) {
        logger_error("dialogue_init() can not load dialogue xml file: %s", src);
        free_chk((char*)src);
        return NULL;
    }

    XmlNode xml_root = xmlparser_get_root(xml);
    AnimList animlist = animlist_init("/assets/common/anims/dialogue-ui.xml");
    ArrayList audios = arraylist_init(sizeof(Audio));
    ArrayList fonts = arraylist_init(sizeof(Font));
    ArrayList backgrounds = arraylist_init(sizeof(Background));
    ArrayList portraits = arraylist_init(sizeof(Portrait));
    ArrayList dialogs = arraylist_init(sizeof(Dialog));
    ArrayList states = arraylist_init(sizeof(State));
    ArrayList multiplechoices = arraylist_init(sizeof(MultipleChoice));
    ArrayList speechimages = arraylist_init(sizeof(SpeechImage));

    AnimsUI anims_ui = {
        .portrait_left_in = NULL,
        .portrait_center_in = NULL,
        .portrait_right_in = NULL,
        .portrait_left_out = NULL,
        .portrait_center_out = NULL,
        .portrait_right_out = NULL,
        .background_in = NULL,
        .background_out = NULL
    };

    if (animlist) {
        anims_ui.portrait_left_in = animsprite_init_from_animlist(animlist, "portrait-add-left");
        anims_ui.portrait_center_in = animsprite_init_from_animlist(animlist, "portrait-add-center");
        anims_ui.portrait_right_in = animsprite_init_from_animlist(animlist, "portrait-add-right");
        anims_ui.portrait_left_out = animsprite_init_from_animlist(animlist, "portrait-remove-left");
        anims_ui.portrait_center_out = animsprite_init_from_animlist(animlist, "portrait-remove-center");
        anims_ui.portrait_right_out = animsprite_init_from_animlist(animlist, "portrait-remove-right");
        anims_ui.background_in = animsprite_init_from_animlist(animlist, "background-in");
        anims_ui.background_out = animsprite_init_from_animlist(animlist, "background-out");
        anims_ui.open = animsprite_init_from_animlist(animlist, "open");
        anims_ui.close = animsprite_init_from_animlist(animlist, "close");
        animlist_destroy(&animlist);
    }

    fs_folder_stack_push();
    fs_set_working_folder(src, true);
    free_chk((char*)src);

    bool import_defaults = vertexprops_parse_boolean(xml_root, "importDefaultDefinition", true);
    XmlParser xml_defaults = NULL;
    XmlNode xml_root_defaults = NULL;
    int32_t xml_child_count_defaults = 0;

    // import default character portraits and speech images
    if (import_defaults) {
        xml_defaults = xmlparser_init(DIALOGUE_DEFAULTS_DEFINITIONS);
        if (xml_defaults) {
            xml_root_defaults = xmlparser_get_root(xml_defaults);
            xml_child_count_defaults = xmlparser_get_children_count(xml_root_defaults);
        }
    }

    int32_t index = 0;
    bool self_parse = true;
    int32_t xml_child_count = xmlparser_get_children_count(xml_root);

    while (true) {
        XmlNode node;

        // parse first the desired xml, and later the default xml
        if (self_parse) {
            if (index >= xml_child_count) {
                if (!xml_root_defaults) break;
                index = 0;
                self_parse = false;
                continue;
            }
            node = xmlparser_get_children_at(xml_root, index++);
        } else {
            if (index >= xml_child_count_defaults) break;
            node = xmlparser_get_children_at(xml_root_defaults, index++);
        }

        if (XML_TAG_NAME_EQUALS(node, "Definition")) {
            foreach (XmlNode, node2, XMLPARSER_CHILDREN_ITERATOR, node) {
                if (XML_TAG_NAME_EQUALS(node2, "AudioList")) {
                    dialogue_internal_parse_audiolist(node2, audios);
                } else if (XML_TAG_NAME_EQUALS(node2, "BackgroundList")) {
                    dialogue_internal_parse_backgroundlist(node2, backgrounds);
                } else if (XML_TAG_NAME_EQUALS(node2, "PortraitList")) {
                    dialogue_internal_parse_portraitlist(node2, portraits);
                } else if (XML_TAG_NAME_EQUALS(node2, "AnimationsUI")) {
                    dialogue_internal_parse_animationui(node2, &anims_ui);
                } else if (XML_TAG_NAME_EQUALS(node2, "Font")) {
                    dialogue_internal_parse_font(node2, fonts);
                } else if (XML_TAG_NAME_EQUALS(node2, "SpeechImageList")) {
                    dialogue_internal_parse_speech_imagelist(node2, speechimages);
                } else if (XML_TAG_NAME_EQUALS(node2, "ImportPortraitList")) {
                    dialogue_internal_parse_import_portraitlist(node2, portraits);
                } else {
                    logger_error("dialogue_init() unknown definition: %s", xmlparser_get_tag_name(node2));
                }
            }
        } else if (XML_TAG_NAME_EQUALS(node, "State")) {
            dialogue_internal_parse_state(node, states);
        } else if (XML_TAG_NAME_EQUALS(node, "MultipleChoice")) {
            dialogue_internal_parse_multiplechoice(node, animlist, dialogs, multiplechoices);
        }
    }

    if (xml_defaults) xmlparser_destroy(&xml_defaults);

    fs_folder_stack_pop();
    xmlparser_destroy(&xml);

    //
    // Note: use memalign because the "matrix_viewport" field must be 32-bit aligned
    //       to avoid performance penality.
    //
    Dialogue dialogue = memalign_chk(32, sizeof(struct Dialogue_s));
    malloc_assert(dialogue, Dialogue);

    *dialogue = (struct Dialogue_s){
        //.matrix_viewport = {},

        .audios = NULL,
        .audios_size = 0,
        .fonts = NULL,
        .fonts_size = 0,
        .backgrounds = NULL,
        .backgrounds_size = 0,
        .portraits = NULL,
        .portraits_size = 0,
        .dialogs = NULL,
        .dialogs_size = 0,
        .states = NULL,
        .states_size = 0,
        .multiplechoices = NULL,
        .multiplechoices_size = 0,
        .speechimages = NULL,
        .speechimages_size = 0,

        .anims_ui = anims_ui,
        .visible_portraits = linkedlist_init(),
        .texsprite_speech = NULL,
        .texsprite_title = NULL,
        .draw_portraits_on_top = false,

        .current_background = -1,
        .change_background_from = -1,
        .script = NULL,
        .do_exit = false,
        .click_text = NULL,
        .click_char = NULL,
        .char_delay = 0,
        .do_skip = false,
        .do_instant_print = false,
        .do_no_wait = false,
        .do_multiplechoice = NULL,
        .current_speechimage = NULL,
        .current_speechimage_is_opening = false,
        .current_speechimage_repeat = RepeatAnim_ALWAYS,
        .is_speaking = false,
        .current_dialog_codepoint_index = 0,
        .current_dialog_codepoint_length = 0,
        .current_dialog_duration = 0.0,
        .current_dialog_elapsed = 0.0,
        .current_dialog_mask = NULL,
        .current_dialog_buffer = stringbuilder_init(64),
        .current_dialog = NULL,
        .current_dialog_line = -1,
        .gamepad = gamepad_init(-1),
        .dialog_external = NULL,
        .is_completed = true,
        .chars_per_second = 0,
        .self_drawable = NULL,
        .self_hidden = false
    };

    dialogue->self_drawable = drawable_init(300.0f, dialogue, (DelegateDraw)dialogue_draw, (DelegateAnimate)dialogue_animate);
    sh4matrix_clear(dialogue->matrix_viewport);
    gamepad_set_buttons_delay(dialogue->gamepad, 200);

    arraylist_destroy2(&audios, &dialogue->audios_size, (void**)&dialogue->audios);
    arraylist_destroy2(&fonts, &dialogue->fonts_size, (void**)&dialogue->fonts);
    arraylist_destroy2(&backgrounds, &dialogue->backgrounds_size, (void**)&dialogue->backgrounds);
    arraylist_destroy2(&portraits, &dialogue->portraits_size, (void**)&dialogue->portraits);
    arraylist_destroy2(&dialogs, &dialogue->dialogs_size, (void**)&dialogue->dialogs);
    arraylist_destroy2(&states, &dialogue->states_size, (void**)&dialogue->states);
    arraylist_destroy2(&multiplechoices, &dialogue->multiplechoices_size, (void**)&dialogue->multiplechoices);
    arraylist_destroy2(&speechimages, &dialogue->speechimages_size, (void**)&dialogue->speechimages);

    // set defaults
    Audio* tmp_audio;

    tmp_audio = dialogue_internal_get_audio(dialogue, "click_text");
    if (tmp_audio) dialogue->click_text = tmp_audio->soundplayer;

    tmp_audio = dialogue_internal_get_audio(dialogue, "click_char");
    if (tmp_audio) dialogue->click_char = tmp_audio->soundplayer;

    dialogue->current_background = dialogue_internal_get_background_index(dialogue, "default_background");
    dialogue->chars_per_second = 0;
    dialogue->char_delay = 40;

    for (int32_t i = 0; i < dialogue->speechimages_size; i++) {
        SpeechImage* speechimage = &dialogue->speechimages[i];
        if (!string_equals(speechimage->name, "normal-left")) continue;

        dialogue->current_speechimage = speechimage;
        bool toggle_default = statesprite_state_toggle(dialogue->current_speechimage->statesprite, NULL);
        bool toggle_open = statesprite_state_toggle(dialogue->current_speechimage->statesprite, DIALOGUE_OPEN);
        dialogue->current_speechimage_is_opening = toggle_default || toggle_open;

        if (!dialogue->current_speechimage_is_opening) {
            // switch to idle animation
            statesprite_state_toggle(dialogue->current_speechimage->statesprite, DIALOGUE_IDLE);
        }

        break;
    }

    // create textsprite speech if not customized
    dialogue->texsprite_speech = textsprite_init(NULL, false, false, 34.0f, 0x00000);
    textsprite_set_paragraph_space(dialogue->texsprite_speech, 8.0f);
    textsprite_set_wordbreak(dialogue->texsprite_speech, FONT_WORDBREAK_LOOSE);

    FontHolder fontholder = dialogue_internal_get_font(dialogue, "font");
    if (fontholder) textsprite_change_font(dialogue->texsprite_speech, fontholder);


    // create textsprite title
    dialogue->texsprite_title = textsprite_init(NULL, false, false, 24.0f, 0x00000);
    textsprite_set_paragraph_space(dialogue->texsprite_title, 8.0f);
    textsprite_set_wordbreak(dialogue->texsprite_title, FONT_WORDBREAK_LOOSE);

    fontholder = dialogue_internal_get_font(dialogue, "font");
    if (fontholder) textsprite_change_font(dialogue->texsprite_speech, fontholder);

    // calculate viewport, dialogue UI is designed for a 1280x720@16:9 screen
    if (viewport_width != FUNKIN_SCREEN_RESOLUTION_WIDTH || viewport_height != FUNKIN_SCREEN_RESOLUTION_HEIGHT) {
        float scale_x = viewport_width / FUNKIN_SCREEN_RESOLUTION_WIDTH;
        float scale_y = viewport_height / FUNKIN_SCREEN_RESOLUTION_HEIGHT;
        float scale = math2d_min_float(scale_x, scale_y);

        sh4matrix_scale(dialogue->matrix_viewport, scale, scale);
        sh4matrix_translate(
            dialogue->matrix_viewport,
            (viewport_width - FUNKIN_SCREEN_RESOLUTION_WIDTH * scale) / 2.0f,
            (viewport_height - FUNKIN_SCREEN_RESOLUTION_HEIGHT * scale) / 2.0f
        );
    }

    return dialogue;
}

void dialogue_destroy(Dialogue* dialogue_ptr) {
    if (!dialogue_ptr || !*dialogue_ptr) return;

    Dialogue dialogue = *dialogue_ptr;

    luascript_drop_shared(dialogue);
    dialogue_internal_destroy_external_dialog(dialogue);

    for (int32_t i = 0; i < dialogue->audios_size; i++) {
        free_chk(dialogue->audios[i].name);
        soundplayer_destroy(&dialogue->audios[i].soundplayer);
    }
    for (int32_t i = 0; i < dialogue->fonts_size; i++) {
        free_chk(dialogue->fonts[i].name);
        fontholder_destroy(&dialogue->fonts[i].fontholder);
    }
    for (int32_t i = 0; i < dialogue->backgrounds_size; i++) {
        free_chk(dialogue->backgrounds[i].name);
        sprite_destroy_full(&dialogue->backgrounds[i].sprite);
    }
    for (int32_t i = 0; i < dialogue->portraits_size; i++) {
        free_chk(dialogue->portraits[i].name);
        statesprite_destroy_texture_if_stateless(dialogue->portraits[i].statesprite);
        statesprite_destroy(&dialogue->portraits[i].statesprite);
    }
    for (int32_t i = 0; i < dialogue->dialogs_size; i++) {
        for (int32_t j = 0; j < dialogue->dialogs[i].lines_size; j++) {
            free_chk(dialogue->dialogs[i].lines[j].target_state_name);
            free_chk(dialogue->dialogs[i].lines[j].text);
        }
        free_chk(dialogue->dialogs[i].lines);
        free_chk(dialogue->dialogs[i].full_path);
    }
    for (int32_t i = 0; i < dialogue->states_size; i++) {
        for (int32_t j = 0; j < dialogue->states[i].actions_size; j++) {
            free_chk(dialogue->states[i].actions[j].name);
            free_chk(dialogue->states[i].actions[j].click_text);
            free_chk(dialogue->states[i].actions[j].click_char);
            free_chk(dialogue->states[i].actions[j].lua_eval);
            free_chk(dialogue->states[i].actions[j].lua_function);
            free_chk(dialogue->states[i].actions[j].random_from_prefix);
            free_chk(dialogue->states[i].actions[j].title);
        }
        free_chk(dialogue->states[i].name);
        free_chk(dialogue->states[i].if_line);
        free_chk(dialogue->states[i].actions);
    }
    for (int32_t i = 0; i < dialogue->multiplechoices_size; i++) {
        for (int32_t j = 0; j < dialogue->multiplechoices[i].choices_size; j++) {
            free_chk(dialogue->multiplechoices[i].choices[j].lua_eval);
            free_chk(dialogue->multiplechoices[i].choices[j].lua_function);
            free_chk(dialogue->multiplechoices[i].choices[j].run_multiple_choice);
            free_chk(dialogue->multiplechoices[i].choices[j].text);
        }
        sprite_destroy_full(&dialogue->multiplechoices[i].hint);
        sprite_destroy_full(&dialogue->multiplechoices[i].icon);
        free_chk(dialogue->multiplechoices[i].state_on_leave);
        free_chk(dialogue->multiplechoices[i].title);
        free_chk(dialogue->multiplechoices[i].name);
        free_chk(dialogue->multiplechoices[i].choices);
    }
    for (int32_t i = 0; i < dialogue->speechimages_size; i++) {
        free_chk(dialogue->speechimages[i].name);
        statesprite_destroy_texture_if_stateless(dialogue->speechimages[i].statesprite);
        statesprite_destroy(&dialogue->speechimages[i].statesprite);
    }

    free_chk(dialogue->audios);
    free_chk(dialogue->fonts);
    free_chk(dialogue->backgrounds);
    free_chk(dialogue->portraits);
    free_chk(dialogue->dialogs);
    free_chk(dialogue->states);
    free_chk(dialogue->multiplechoices);
    free_chk(dialogue->speechimages);

    if (dialogue->anims_ui.portrait_left_in) animsprite_destroy(&dialogue->anims_ui.portrait_left_in);
    if (dialogue->anims_ui.portrait_center_in) animsprite_destroy(&dialogue->anims_ui.portrait_center_in);
    if (dialogue->anims_ui.portrait_right_in) animsprite_destroy(&dialogue->anims_ui.portrait_right_in);
    if (dialogue->anims_ui.portrait_left_out) animsprite_destroy(&dialogue->anims_ui.portrait_left_out);
    if (dialogue->anims_ui.portrait_center_out) animsprite_destroy(&dialogue->anims_ui.portrait_center_out);
    if (dialogue->anims_ui.portrait_right_out) animsprite_destroy(&dialogue->anims_ui.portrait_right_out);
    if (dialogue->anims_ui.background_in) animsprite_destroy(&dialogue->anims_ui.background_in);
    if (dialogue->anims_ui.background_out) animsprite_destroy(&dialogue->anims_ui.background_out);
    if (dialogue->anims_ui.open) animsprite_destroy(&dialogue->anims_ui.open);
    if (dialogue->anims_ui.close) animsprite_destroy(&dialogue->anims_ui.close);

    textsprite_destroy(&dialogue->texsprite_speech);
    textsprite_destroy(&dialogue->texsprite_title);

    linkedlist_destroy(&dialogue->visible_portraits);
    gamepad_destroy(&dialogue->gamepad);
    drawable_destroy(&dialogue->self_drawable);
    stringbuilder_destroy(&dialogue->current_dialog_buffer);
    free_chk(dialogue->current_dialog_mask);

    free_chk(dialogue);
    *dialogue_ptr = NULL;
}

bool dialogue_apply_state(Dialogue dialogue, const char* state_name) {
    return dialogue_apply_state2(dialogue, state_name, NULL);
}

bool dialogue_apply_state2(Dialogue dialogue, const char* state_name, const char* if_line_label) {
    if (dialogue->do_exit) return false;

    State* state = NULL;
    for (int32_t i = 0; i < dialogue->states_size; i++) {
        if (string_equals(dialogue->states[i].name, state_name) && string_equals(dialogue->states[i].if_line, if_line_label)) {
            state = &dialogue->states[i];
            break;
        }
    }
    if (!state) return false;

    return dialogue_internal_apply_state(dialogue, state);
}

int32_t dialogue_animate(Dialogue dialogue, float elapsed) {
    if (dialogue->self_hidden || dialogue->is_completed) return 1;

    if (dialogue->anims_ui.open && animsprite_animate(dialogue->anims_ui.open, elapsed) < 1) {
        animsprite_update_drawable(dialogue->anims_ui.open, dialogue->self_drawable, true);
    }

    bool anim_portrait_left_in = !!dialogue->anims_ui.portrait_left_in;
    bool anim_portrait_center_in = !!dialogue->anims_ui.portrait_center_in;
    bool anim_portrait_right_in = !!dialogue->anims_ui.portrait_right_out;
    bool anim_portrait_left_out = !!dialogue->anims_ui.portrait_left_out;
    bool anim_portrait_center_out = !!dialogue->anims_ui.portrait_center_out;
    bool anim_portrait_right_out = !!dialogue->anims_ui.portrait_right_out;

    for (int32_t i = 0; i < dialogue->backgrounds_size; i++) {
        sprite_animate(dialogue->backgrounds[i].sprite, elapsed);
    }

    if (anim_portrait_left_in) {
        if (animsprite_animate(dialogue->anims_ui.portrait_left_in, elapsed) > 0) anim_portrait_left_in = false;
    }
    if (anim_portrait_center_in) {
        if (animsprite_animate(dialogue->anims_ui.portrait_center_in, elapsed) > 0) anim_portrait_center_in = false;
    }
    if (anim_portrait_right_in) {
        if (animsprite_animate(dialogue->anims_ui.portrait_right_in, elapsed) > 0) anim_portrait_right_in = false;
    }
    if (anim_portrait_left_out) {
        if (animsprite_animate(dialogue->anims_ui.portrait_left_out, elapsed) > 0) anim_portrait_left_out = false;
    }
    if (anim_portrait_center_out) {
        if (animsprite_animate(dialogue->anims_ui.portrait_center_out, elapsed) > 0) anim_portrait_center_out = false;
    }
    if (anim_portrait_right_out) {
        if (animsprite_animate(dialogue->anims_ui.portrait_right_out, elapsed) > 0) anim_portrait_right_out = false;
    }

    foreach (Portrait*, portrait, LINKEDLIST_ITERATOR, dialogue->visible_portraits) {
        bool exists;
        AnimSprite anim;

        int32_t completed = statesprite_animate(portrait->statesprite, elapsed);

        if (portrait->is_added) {
            // guess the correct animation direction
            if (portrait->position < 0.5f) {
                exists = anim_portrait_left_in;
                anim = dialogue->anims_ui.portrait_left_in;
            } else if (portrait->position == 0.5f) {
                exists = anim_portrait_center_in;
                anim = dialogue->anims_ui.portrait_center_in;
            } else if (portrait->position > 0.5f) {
                exists = anim_portrait_right_in;
                anim = dialogue->anims_ui.portrait_right_in;
            } else {
                // this never should happen
                continue;
            }

            if (exists) {
                animsprite_update_statesprite(anim, portrait->statesprite, false);
            } else {
                portrait->is_added = false;
                bool toggled_default = statesprite_state_toggle(portrait->statesprite, NULL);
                bool toggled_speak = statesprite_state_toggle(portrait->statesprite, DIALOGUE_SPEAK);

                if (dialogue->is_speaking && (toggled_default || toggled_speak)) {
                    portrait->is_speaking = true;
                    statesprite_animation_restart(portrait->statesprite);
                } else {
                    // no speak animation, fallback to idle
                    portrait->is_speaking = false;
                    statesprite_state_toggle(portrait->statesprite, DIALOGUE_IDLE);
                }
            }
        } else if (portrait->is_removed) {
            // guess the correct animation direction
            if (portrait->position < 0.5f) {
                exists = anim_portrait_left_out;
                anim = dialogue->anims_ui.portrait_left_out;
            } else if (portrait->position == 0.5f) {
                exists = anim_portrait_center_out;
                anim = dialogue->anims_ui.portrait_center_out;
            } else if (portrait->position > 0.5f) {
                exists = anim_portrait_right_out;
                anim = dialogue->anims_ui.portrait_right_out;
            } else {
                // this never should happen
                continue;
            }

            if (exists) {
                animsprite_update_statesprite(anim, portrait->statesprite, false);
            } else {
                portrait->is_removed = false;
                linkedlist_remove_item(dialogue->visible_portraits, portrait);
            }
        } else if (completed < 1) {
            // the animation is not completed, nothing to do
            continue;
        }

        // if the speak animation is completed and there not longer speech switch to idle
        if (!dialogue->is_speaking && portrait->is_speaking) {
            portrait->is_speaking = false;
            dialogue_internal_stop_portrait_animation(portrait);
            continue;
        }

        // check if the animation should be looped again
        if (!(portrait->is_speaking ? portrait->speak_anim_looped : portrait->idle_anim_looped)) {
            continue;
        }

        // only loop if the desired state is applied and exists
        bool can_loop = portrait->is_speaking ? portrait->has_speak : portrait->has_idle;

        // restart the animation if necessary
        if (can_loop || (!portrait->has_speak && !portrait->has_idle)) {
            statesprite_animation_restart(portrait->statesprite);
        }
    }

    if (dialogue->current_background >= 0 && dialogue->anims_ui.background_in) {
        Sprite sprite = dialogue->backgrounds[dialogue->current_background].sprite;
        if (animsprite_animate(dialogue->anims_ui.background_in, elapsed) < 1) {
            animsprite_update_sprite(dialogue->anims_ui.background_in, sprite, true);
        }
    }

    if (dialogue->change_background_from >= 0 && dialogue->anims_ui.background_out) {
        Sprite sprite = dialogue->backgrounds[dialogue->change_background_from].sprite;
        if (animsprite_animate(dialogue->anims_ui.background_out, elapsed) < 1) {
            animsprite_update_sprite(dialogue->anims_ui.background_out, sprite, true);
        } else {
            dialogue->change_background_from = -1;
        }
    }

    if (dialogue->current_speechimage) {
        bool is_opening = dialogue->current_speechimage_is_opening;
        StateSprite sprite = dialogue->current_speechimage->statesprite;

        bool completed = statesprite_animate(sprite, elapsed) > 0;

        // once opening animation is done, switch to idle animation
        if (completed && is_opening && statesprite_state_toggle(sprite, DIALOGUE_IDLE)) {
            statesprite_animation_restart(sprite);
            dialogue->current_speechimage_is_opening = false;
        } else if (completed && !is_opening) {
            switch (dialogue->current_speechimage_repeat) {
                case RepeatAnim_ALWAYS:
                    statesprite_animation_restart(sprite);
                    break;
                case RepeatAnim_WHILESPEAKS:
                    if (dialogue->is_speaking) statesprite_animation_restart(sprite);
                    break;
                    CASE_UNUSED(RepeatAnim_ONCE)
                    CASE_UNUSED(RepeatAnim_NONE)
            }
        }
    }

    return 0;
}

void dialogue_poll(Dialogue dialogue, bool elapsed) {
    if (dialogue->self_hidden || dialogue->is_completed) return;

    if (dialogue->do_exit) {
        if (dialogue->anims_ui.close && animsprite_animate(dialogue->anims_ui.close, elapsed) < 1) {
            animsprite_update_drawable(dialogue->anims_ui.close, dialogue->self_drawable, true);
        } else {
            if (dialogue->script != NULL) luascript_call_function(dialogue->script, "f_dialogue_exit");
            dialogue->is_completed = true;
            for (int32_t i = 0; i < dialogue->audios_size; i++) soundplayer_stop(dialogue->audios[i].soundplayer);
            return;
        }
    }

    if (dialogue->current_dialog) {
        dialogue->current_dialog_elapsed += elapsed;
        bool preapare_next_line = false;

        GamepadButtons buttons = gamepad_has_pressed_delayed(
            dialogue->gamepad, GAMEPAD_A | GAMEPAD_X | GAMEPAD_START | GAMEPAD_BACK
        );
        if (buttons & GAMEPAD_A) {
            dialogue->current_dialog_elapsed = dialogue->current_dialog_duration;
            preapare_next_line = true;
        } else if (buttons & GAMEPAD_X) {
            if (dialogue->is_speaking)
                dialogue->do_no_wait = true;
            else
                preapare_next_line = true;
        } else if (buttons & (GAMEPAD_START | GAMEPAD_BACK)) {
            dialogue->do_exit = true;
            dialogue_close(dialogue);
        }

        if (dialogue->is_speaking)
            dialogue_internal_print_text(dialogue);
        else if (!dialogue->do_exit && preapare_next_line)
            dialogue_internal_prepare_print_text(dialogue);
    }
}

void dialogue_draw(Dialogue dialogue, PVRContext pvrctx) {
    if (dialogue->self_hidden || dialogue->is_completed) return;

    // apply viewport matrix
    pvr_context_save(pvrctx);
    sh4matrix_multiply_with_matrix(pvrctx->current_matrix, dialogue->matrix_viewport);

    drawable_helper_apply_in_context(dialogue->self_drawable, pvrctx);
    pvr_context_save(pvrctx);

    dialogue_internal_draw_background(dialogue, pvrctx);
    if (!dialogue->draw_portraits_on_top) dialogue_internal_draw_portraits(dialogue, pvrctx);
    dialogue_internal_draw_speech(dialogue, pvrctx);
    if (dialogue->draw_portraits_on_top) dialogue_internal_draw_portraits(dialogue, pvrctx);

    pvr_context_restore(pvrctx);
    pvr_context_restore(pvrctx);
}

bool dialogue_is_completed(Dialogue dialogue) {
    return dialogue->is_completed;
}

bool dialogue_is_hidden(Dialogue dialogue) {
    return dialogue->self_hidden;
}

bool dialogue_show_dialog(Dialogue dialogue, const char* src_dialog) {
    if (!src_dialog) return false;

    char* full_path = fs_get_full_path_and_override(src_dialog);
    dialogue->current_dialog = NULL;

    if (dialogue->dialog_external && string_equals(dialogue->dialog_external->full_path, full_path)) {
        dialogue->current_dialog = dialogue->dialog_external;
    } else {
        // check if the dialog is already parsed
        for (int32_t i = 0; i < dialogue->dialogs_size; i++) {
            if (string_equals(dialogue->dialogs[i].full_path, full_path)) {
                dialogue_internal_destroy_external_dialog(dialogue);
                dialogue->current_dialog = &dialogue->dialogs[i];
                break;
            }
        }
    }

    if (dialogue->current_dialog) {
        free_chk(full_path);
        return dialogue_internal_prepare_dialog(dialogue);
    }

    // load and parse dialog txt file
    char* source = fs_readtext(full_path);
    if (source != NULL) {
        dialogue_internal_parse_external_dialog(dialogue, source);
        dialogue->dialog_external->full_path = full_path;
        free_chk(source);
    } else {
        logger_error("dialogue_show_dialog() can not read: %s", src_dialog);
        free_chk(full_path);
        return false;
    }

    return dialogue_internal_prepare_dialog(dialogue);
}

bool dialogue_show_dialog2(Dialogue dialogue, const char* text_dialog_content) {
    if (string_is_empty(text_dialog_content)) return false;

    dialogue_internal_parse_external_dialog(dialogue, text_dialog_content);
    return dialogue_internal_prepare_dialog(dialogue);
}

void dialogue_close(Dialogue dialogue) {
    if (dialogue->script != NULL) luascript_call_function(dialogue->script, "f_dialogue_closing");

    dialogue->do_exit = true;
    dialogue->current_dialog = NULL;

    for (int32_t i = 0; i < dialogue->audios_size; i++) {
        if (soundplayer_is_playing(dialogue->audios[i].soundplayer)) {
            if (dialogue->anims_ui.close)
                soundplayer_fade(dialogue->audios[i].soundplayer, false, 500.0f);
            else
                soundplayer_stop(dialogue->audios[i].soundplayer);
        }
    }
}

void dialogue_hide(Dialogue dialogue, bool hidden) {
    dialogue->self_hidden = hidden;
}

void dialogue_suspend(Dialogue dialogue) {
    if (dialogue->is_completed) return;
    for (int32_t i = 0; i < dialogue->audios_size; i++) {
        dialogue->audios[i].was_playing = soundplayer_is_playing(dialogue->audios[i].soundplayer);
        if (dialogue->audios[i].was_playing) {
            soundplayer_pause(dialogue->audios[i].soundplayer);
            if (soundplayer_has_fading(dialogue->audios[i].soundplayer) == FADING_OUT) soundplayer_set_volume(dialogue->audios[i].soundplayer, 0.0f);
        }
    }
}

void dialogue_resume(Dialogue dialogue) {
    if (dialogue->is_completed) return;
    for (int32_t i = 0; i < dialogue->audios_size; i++) {
        if (dialogue->audios[i].was_playing) soundplayer_play(dialogue->audios[i].soundplayer);
    }
    gamepad_clear_buttons(dialogue->gamepad);
}

Modifier* dialogue_get_modifier(Dialogue dialogue) {
    return drawable_get_modifier(dialogue->self_drawable);
}

Drawable dialogue_get_drawable(Dialogue dialogue) {
    return dialogue->self_drawable;
}

void dialogue_set_offsetcolor(Dialogue dialogue, float r, float g, float b, float a) {
    drawable_set_offsetcolor(dialogue->self_drawable, r, g, b, a);
}

void dialogue_set_antialiasing(Dialogue dialogue, PVRFlag pvrflag) {
    drawable_set_antialiasing(dialogue->self_drawable, pvrflag);
}

void dialogue_set_alpha(Dialogue dialogue, float alpha) {
    drawable_set_alpha(dialogue->self_drawable, alpha);
}

void dialogue_set_script(Dialogue dialogue, WeekScript weekscript) {
    if (weekscript != NULL)
        dialogue->script = weekscript_get_luascript(weekscript);
    else
        dialogue->script = NULL;
}


static bool dialogue_internal_prepare_dialog(Dialogue dialogue) {
    dialogue->do_skip = false;
    dialogue->do_instant_print = false;
    dialogue->do_multiplechoice = NULL;
    dialogue->do_exit = false;
    dialogue->current_dialog_line = 0;
    dialogue->self_hidden = false;
    dialogue->is_completed = false;
    drawable_set_antialiasing(dialogue->self_drawable, PVRCTX_FLAG_DEFAULT);
    drawable_set_alpha(dialogue->self_drawable, 1.0f);
    drawable_set_property(dialogue->self_drawable, SPRITE_PROP_ALPHA2, 1.0f);
    drawable_set_offsetcolor_to_default(dialogue->self_drawable);
    pvr_context_helper_clear_modifier(drawable_get_modifier(dialogue->self_drawable));
    linkedlist_clear(dialogue->visible_portraits, NULL);

    // apply any initial state
    dialogue_apply_state2(dialogue, NULL, NULL);
    for (int32_t i = 0; i < dialogue->states_size; i++) {
        if (dialogue->states[i].initial) dialogue_internal_apply_state(dialogue, &dialogue->states[i]);
    }

    if (!textsprite_has_font(dialogue->texsprite_speech)) {
        logger_error("dialogue_internal_prepare_dialog() speech textsprite does not have font");
        dialogue->do_exit = true;
        return false;
    }

    dialogue_internal_prepare_print_text(dialogue);

    if (dialogue->anims_ui.open) animsprite_restart(dialogue->anims_ui.open);
    if (dialogue->anims_ui.close) animsprite_restart(dialogue->anims_ui.close);

    if (!dialogue->current_speechimage && dialogue->speechimages_size > 0) {
        logger_warn("dialogue_internal_prepare_dialog() no speech background choosen, auto-choosing the first one declared");
        dialogue->current_speechimage = &dialogue->speechimages[0];
    }

    gamepad_enforce_buttons_delay(dialogue->gamepad);

    return true;
}

static bool dialogue_internal_apply_state(Dialogue dialogue, State* state) {
    if (dialogue->do_exit) return false;

    Audio* audio;
    int32_t background;
    bool background_changed = false;
    bool speechimage_changed = false;

    for (int32_t action_index = 0; action_index < state->actions_size; action_index++) {
        Action* action = &state->actions[action_index];

        switch (action->type) {
            case Type_AUDIO_PLAY:
                audio = dialogue_internal_get_audio(dialogue, action->name);
                if (audio) soundplayer_play(audio->soundplayer);
                break;
            case Type_AUDIO_PAUSE:
                audio = dialogue_internal_get_audio(dialogue, action->name);
                if (audio) {
                    soundplayer_pause(audio->soundplayer);
                } else if (action->name == NULL) {
                    for (int32_t i = 0; i < dialogue->audios_size; i++) {
                        soundplayer_pause(dialogue->audios[i].soundplayer);
                    }
                }
                break;
            case Type_AUDIO_FADEIN:
                audio = dialogue_internal_get_audio(dialogue, action->name);
                if (audio) soundplayer_fade(audio->soundplayer, true, action->duration);
                break;
            case Type_AUDIO_FADEOUT:
                audio = dialogue_internal_get_audio(dialogue, action->name);
                if (audio) soundplayer_fade(audio->soundplayer, false, action->duration);
                break;
            case Type_AUDIO_STOP:
                audio = dialogue_internal_get_audio(dialogue, action->name);
                if (audio) {
                    soundplayer_stop(audio->soundplayer);
                } else if (action->name == NULL) {
                    for (int32_t i = 0; i < dialogue->audios_size; i++) {
                        soundplayer_stop(dialogue->audios[i].soundplayer);
                    }
                }
                break;
            case Type_AUDIO_VOLUME:
                audio = dialogue_internal_get_audio(dialogue, action->name);
                if (audio) soundplayer_set_volume(audio->soundplayer, action->volume);
                break;
            case Type_BACKGROUND_SET:
                background = dialogue_internal_get_background_index(dialogue, action->name);
                if (background >= 0) {
                    dialogue->current_background = background;
                    dialogue->change_background_from = -1;
                    background_changed = true;
                }
                break;
            case Type_BACKGROUND_CHANGE:
                dialogue->change_background_from = dialogue->current_background;
                dialogue->current_background = dialogue_internal_get_background_index(dialogue, action->name);
                background_changed = true;
                break;
            case Type_BACKGROUND_REMOVE:
                dialogue->current_background = -1;
                dialogue->change_background_from = -1;
                background_changed = true;
                break;
            case Type_LUA:
                if (dialogue->script == NULL) {
                    logger_error("dialogue_internal_apply_state() no lua script attached");
                    break;
                }
                if (action->lua_function)
                    luascript_call_function(dialogue->script, action->lua_function);
                if (action->lua_eval)
                    luascript_eval(dialogue->script, action->lua_eval);
                break;
            case Type_EXIT:
                dialogue_close(dialogue);
                return true;
            case Type_PORTRAIT_ADD:
                int32_t portrait_index = -1;
                if (action->random_from_prefix != NULL && dialogue->portraits_size > 0) {
                    if (action->random_from_prefix[0] == '\0') {
                        // random choose
                        portrait_index = math2d_random_int(0, dialogue->portraits_size);
                    } else {
                        portrait_index = -1;
                        int32_t count = 0;
                        int32_t j = 0;

                        for (int32_t i = 0; i < dialogue->portraits_size; i++) {
                            if (string_starts_with(dialogue->portraits[i].name, action->name)) count++;
                        }

                        // random choose
                        int32_t choosen = math2d_random_int(0, count);

                        for (int32_t i = 0; i < dialogue->portraits_size; i++) {
                            if (string_starts_with(dialogue->portraits[i].name, action->name)) {
                                if (j == choosen) {
                                    portrait_index = i;
                                    break;
                                }
                                j++;
                            }
                        }
                    }
                } else {
                    for (int32_t i = 0; i < dialogue->portraits_size; i++) {
                        if (string_equals(dialogue->portraits[i].name, action->name)) {
                            portrait_index = i;
                            break;
                        }
                    }
                }

                if (portrait_index < 0 || portrait_index >= dialogue->portraits_size) {
                    break;
                }

                if (dialogue->anims_ui.portrait_left_in) animsprite_restart(dialogue->anims_ui.portrait_left_in);
                if (dialogue->anims_ui.portrait_center_in) animsprite_restart(dialogue->anims_ui.portrait_center_in);
                if (dialogue->anims_ui.portrait_right_in) animsprite_restart(dialogue->anims_ui.portrait_right_in);

                linkedlist_remove_item(dialogue->visible_portraits, &dialogue->portraits[portrait_index]);
                linkedlist_add_item(dialogue->visible_portraits, &dialogue->portraits[portrait_index]);
                dialogue->portraits[portrait_index].is_added = true;
                dialogue->portraits[portrait_index].is_removed = false;
                dialogue->portraits[portrait_index].is_speaking = false;
                statesprite_state_toggle(dialogue->portraits[portrait_index].statesprite, NULL);
                statesprite_state_toggle(
                    dialogue->portraits[portrait_index].statesprite, action->no_speak ? DIALOGUE_IDLE : DIALOGUE_SPEAK
                );
                break;
            case Type_PORTRAIT_REMOVE:
                for (int32_t i = 0; i < dialogue->portraits_size; i++) {
                    if (string_equals(dialogue->portraits[i].name, action->name)) {
                        dialogue->portraits[i].is_added = false;
                        dialogue->portraits[i].is_removed = true;

                        if (dialogue->anims_ui.portrait_left_out)
                            animsprite_restart(dialogue->anims_ui.portrait_left_out);
                        if (dialogue->anims_ui.portrait_center_out)
                            animsprite_restart(dialogue->anims_ui.portrait_center_out);
                        if (dialogue->anims_ui.portrait_right_out)
                            animsprite_restart(dialogue->anims_ui.portrait_right_out);

                        break;
                    }
                }
                break;
            case Type_PORTRAIT_REMOVEALL:
                if (action->animate_remove) {
                    foreach (Portrait*, portrait, LINKEDLIST_ITERATOR, dialogue->visible_portraits) {
                        portrait->is_added = false;
                        portrait->is_removed = true;
                    }

                    if (dialogue->anims_ui.portrait_left_out)
                        animsprite_restart(dialogue->anims_ui.portrait_left_out);
                    if (dialogue->anims_ui.portrait_center_out)
                        animsprite_restart(dialogue->anims_ui.portrait_center_out);
                    if (dialogue->anims_ui.portrait_right_out)
                        animsprite_restart(dialogue->anims_ui.portrait_right_out);
                } else {
                    linkedlist_clear(dialogue->visible_portraits, NULL);
                }
                break;
            case Type_AUDIO_UI:
                if (action->click_char) {
                    audio = dialogue_internal_get_audio(dialogue, action->click_char);
                    if (dialogue->click_char) soundplayer_stop(dialogue->click_char);

                    if (audio)
                        dialogue->click_char = audio->soundplayer;
                    else
                        dialogue->click_char = NULL;
                }
                if (action->click_text) {
                    audio = dialogue_internal_get_audio(dialogue, action->click_text);
                    if (dialogue->click_text) soundplayer_stop(dialogue->click_text);

                    if (audio)
                        dialogue->click_text = audio->soundplayer;
                    else
                        dialogue->click_text = NULL;
                }
                break;
            case Type_TEXT_SPEED:
                if (action->chars_per_second >= 0) dialogue->chars_per_second = action->chars_per_second;
                if (action->char_delay >= 0) dialogue->char_delay = action->char_delay;
                break;
            case Type_TEXT_SKIP:
                dialogue->do_skip = true;
                break;
            case Type_TEXT_INSTANTPRINT:
                dialogue->do_instant_print = true;
                break;
            case Type_SPEECH_BACKGROUND:
                if (action->name == NULL) {
                    // random choose
                    int32_t index = math2d_random_int(0, dialogue->backgrounds_size);
                    dialogue->current_speechimage = &dialogue->speechimages[index];
                    speechimage_changed = true;
                } else if (string_equals(action->name, "none")) {
                    speechimage_changed = false;
                    dialogue->current_speechimage = NULL;
                    dialogue->current_speechimage_is_opening = false;
                } else {
                    for (int32_t i = 0; i < dialogue->speechimages_size; i++) {
                        if (string_equals(dialogue->speechimages[i].name, action->name)) {
                            dialogue->current_speechimage = &dialogue->speechimages[i];
                            speechimage_changed = true;
                            break;
                        }
                    }
                }
                if (speechimage_changed && action->repeat_anim != RepeatAnim_NONE)
                    dialogue->current_speechimage_repeat = action->repeat_anim;
                break;
            case Type_TEXT_FONT:
                if (!dialogue->texsprite_speech) break;
                FontHolder fontholder = dialogue_internal_get_font(dialogue, action->name);
                if (fontholder) textsprite_change_font(dialogue->texsprite_speech, fontholder);
                break;
            case Type_TEXT_COLOR:
                if (!dialogue->texsprite_speech) break;
                textsprite_set_color(dialogue->texsprite_speech, action->rgba[0], action->rgba[1], action->rgba[2]);
                if (!math2d_is_float_NaN(action->rgba[3])) textsprite_set_alpha(dialogue->texsprite_speech, action->rgba[3]);
                break;
            case Type_TEXT_BORDERCOLOR:
                if (!dialogue->texsprite_speech) break;
                textsprite_border_set_color(dialogue->texsprite_speech, action->rgba[0], action->rgba[1], action->rgba[2], action->rgba[3]);
                break;
            case Type_TEXT_BORDEROFFSET:
                if (!dialogue->texsprite_speech) break;
                textsprite_border_set_offset(dialogue->texsprite_speech, action->offset_x, action->offset_y);
                break;
            case Type_TEXT_SIZE:
                if (!dialogue->texsprite_speech) break;
                if (!math2d_is_float_NaN(action->size)) textsprite_set_font_size(dialogue->texsprite_speech, action->size);
                break;
            case Type_TEXT_BORDERSIZE:
                if (!dialogue->texsprite_speech) break;
                if (!math2d_is_float_NaN(action->size)) textsprite_border_set_size(dialogue->texsprite_speech, action->size);
                break;
            case Type_TEXT_BORDERENABLE:
                if (!dialogue->texsprite_speech) break;
                textsprite_border_enable(dialogue->texsprite_speech, action->enabled);
                break;
            case Type_TEXT_PARAGRAPHSPACE:
                if (!dialogue->texsprite_speech) break;
                if (!math2d_is_float_NaN(action->size)) textsprite_set_paragraph_space(dialogue->texsprite_speech, action->size);
                break;
            case Type_TEXT_ALIGN:
                if (!dialogue->texsprite_speech) break;
                textsprite_set_align(dialogue->texsprite_speech, action->align_vertical, action->align_horizontal);
                if (action->align_paragraph != ALIGN_NONE) textsprite_set_paragraph_align(dialogue->texsprite_speech, action->align_paragraph);
                break;
            case Type_RUNMULTIPLECHOICE:
                dialogue->do_multiplechoice = dialogue_internal_get_multiplechoice(dialogue, action->name);
                break;
            case Type_TITLE:
                if (!dialogue->texsprite_title) break;
                textsprite_set_text_intern(dialogue->texsprite_title, true, (const char* const*)&action->title);
                break;
            case Type_NOWAIT:
                dialogue->do_no_wait = true;
                break;
        }
    }

    if (background_changed) {
        if (dialogue->anims_ui.background_in) animsprite_restart(dialogue->anims_ui.background_in);
        if (dialogue->anims_ui.background_out) animsprite_restart(dialogue->anims_ui.background_out);
    }

    if (speechimage_changed && dialogue->current_speechimage) {
        bool toggle_default = statesprite_state_toggle(dialogue->current_speechimage->statesprite, NULL);
        bool toggle_open = statesprite_state_toggle(dialogue->current_speechimage->statesprite, DIALOGUE_OPEN);

        dialogue->current_speechimage_is_opening = toggle_default || toggle_open;

        if (!dialogue->current_speechimage_is_opening) {
            // switch to idle animation
            statesprite_state_toggle(dialogue->current_speechimage->statesprite, DIALOGUE_IDLE);
        }

        statesprite_animation_restart(dialogue->current_speechimage->statesprite);

        // set speech background location
        statesprite_set_draw_location(
            dialogue->current_speechimage->statesprite,
            dialogue->current_speechimage->offset_x,
            dialogue->current_speechimage->offset_y
        );

        // set speech text bounds
        float text_x = dialogue->current_speechimage->text_x;
        float text_y = dialogue->current_speechimage->text_y;
        if (dialogue->current_speechimage->text_is_relative) {
            text_x += dialogue->current_speechimage->offset_x;
            text_y += dialogue->current_speechimage->offset_y;
        }
        textsprite_set_draw_location(dialogue->texsprite_speech, text_x, text_y);
        textsprite_set_max_draw_size(
            dialogue->texsprite_speech,
            dialogue->current_speechimage->text_width,
            dialogue->current_speechimage->text_height
        );

        // set title location
        float title_x = dialogue->current_speechimage->title_x;
        float title_y = dialogue->current_speechimage->title_y;
        if (dialogue->current_speechimage->title_is_relative) {
            title_x += dialogue->current_speechimage->offset_x;
            title_y += dialogue->current_speechimage->offset_y;
        }
        textsprite_set_draw_location(dialogue->texsprite_title, title_x, title_y);
    }

    return true;
}

static void dialogue_internal_draw_background(Dialogue dialogue, PVRContext pvrctx) {
    if (dialogue->change_background_from >= 0)
        sprite_draw(dialogue->backgrounds[dialogue->change_background_from].sprite, pvrctx);

    if (dialogue->current_background >= 0)
        sprite_draw(dialogue->backgrounds[dialogue->current_background].sprite, pvrctx);
}

static void dialogue_internal_draw_portraits(Dialogue dialogue, PVRContext pvrctx) {
    float draw_width = -1.0f, draw_height = -1.0f;
    float portrait_line_x, portrait_line_y, portrait_line_width;

    if (dialogue->current_speechimage) {
        portrait_line_x = dialogue->current_speechimage->portrait_line_x;
        portrait_line_y = dialogue->current_speechimage->portrait_line_y;
        portrait_line_width = dialogue->current_speechimage->portrait_line_width;
        if (dialogue->current_speechimage->portrait_line_is_relative) {
            portrait_line_x += dialogue->current_speechimage->offset_x;
            portrait_line_y += dialogue->current_speechimage->offset_y;
        }
    } else {
        portrait_line_x = 0.0f;
        portrait_line_y = FUNKIN_SCREEN_RESOLUTION_HEIGHT / 2.0f;
        portrait_line_width = 0.9f * FUNKIN_SCREEN_RESOLUTION_WIDTH;
    }

    foreach (Portrait*, portrait, LINKEDLIST_ITERATOR, dialogue->visible_portraits) {
        statesprite_get_draw_size(portrait->statesprite, &draw_width, &draw_height);

        float draw_x = portrait->position * portrait_line_width;
        float draw_y = 0.0f;

        switch (dialogue->current_speechimage->align_horizontal) {
            case ALIGN_NONE:
                if (portrait->position == 0.5f) {
                    draw_x -= draw_width / 2.0f;
                } else if (portrait->position > 0.5f) {
                    draw_x -= draw_width;
                }
                break;
            case ALIGN_END:
                draw_x -= draw_width;
                break;
            case ALIGN_CENTER:
                draw_x -= draw_width / 2.0f;
                break;
                CASE_UNUSED(ALIGN_INVALID)
                CASE_UNUSED(ALIGN_START)
                CASE_UNUSED(ALIGN_BOTH)
        }

        switch (dialogue->current_speechimage->align_vertical) {
            case ALIGN_CENTER:
                draw_y = draw_height / -2.0f;
                break;
            case ALIGN_NONE:
            case ALIGN_END:
                draw_y -= draw_height;
                break;
                CASE_UNUSED(ALIGN_INVALID)
                CASE_UNUSED(ALIGN_START)
                CASE_UNUSED(ALIGN_BOTH)
        }

        if (portrait->is_speaking) {
            draw_x += portrait->offset_speak_x;
            draw_y += portrait->offset_speak_y;
        } else {
            draw_x += portrait->offset_idle_x;
            draw_y += portrait->offset_idle_y;
        }

        pvr_context_save(pvrctx);
        sh4matrix_translate(pvrctx->current_matrix, draw_x + portrait_line_x, draw_y + portrait_line_y);
        statesprite_draw(portrait->statesprite, pvrctx);
        pvr_context_restore(pvrctx);
    }
}

static void dialogue_internal_draw_speech(Dialogue dialogue, PVRContext pvrctx) {
    if (!dialogue->current_speechimage) return;

    statesprite_draw(dialogue->current_speechimage->statesprite, pvrctx);
    textsprite_draw(dialogue->texsprite_title, pvrctx);
    textsprite_draw(dialogue->texsprite_speech, pvrctx);
}

static void dialogue_internal_prepare_print_text(Dialogue dialogue) {
    if (!dialogue->current_dialog) return;

    if (dialogue->current_dialog_line < dialogue->current_dialog->lines_size) {
        DialogLine* dialog_line = &dialogue->current_dialog->lines[dialogue->current_dialog_line];
        dialogue_apply_state2(dialogue, dialog_line->target_state_name, NULL);
        dialogue_apply_state2(dialogue, dialog_line->target_state_name, dialog_line->text);
        if (dialogue->do_exit) return;
    }

    if (dialogue->do_skip) {
        dialogue->current_dialog_line++;
        dialogue->do_skip = false;
        dialogue_internal_prepare_print_text(dialogue);
        return;
    }

    if (dialogue->current_dialog_line >= dialogue->current_dialog->lines_size) {
        dialogue_close(dialogue);
        return;
    }

    DialogLine* line = &dialogue->current_dialog->lines[dialogue->current_dialog_line];

    if (dialogue->do_instant_print) {
        dialogue_internal_notify_script(dialogue, true);
        textsprite_set_text_intern(dialogue->texsprite_speech, true, (const char* const*)&line->text);
        dialogue_internal_notify_script(dialogue, false);
        dialogue_internal_toggle_idle(dialogue);
        dialogue->do_instant_print = false;
        dialogue->is_speaking = false;
        return;
    }

    StringBuilder buffer = dialogue->current_dialog_buffer;
    Grapheme grapheme = {.code = 0x00, .size = 0};
    int32_t index = 0;
    size_t line_text_length = strlen(line->text);

    stringbuilder_clear(buffer);
    free_chk(dialogue->current_dialog_mask);
    dialogue->current_dialog_mask = NULL;

    while (string_get_character_codepoint(line->text, index, line_text_length, &grapheme)) {
        switch (grapheme.code) {
            case 0x20: // space
            case 0x09: // tab
            case 0x0A: // new line
                // case 0x2E:// dot
                // case 0x3A:// double-dot
                // case 0x2C:// comma
                // case 0x3B:// dot-comma
                stringbuilder_add_char_codepoint(buffer, grapheme.code);
                break;
            default:
                stringbuilder_add_char_codepoint(buffer, 0xA0); // hard-space
                break;
        }
        index += grapheme.size;
    }
    dialogue->current_dialog_codepoint_index = 0;
    dialogue->current_dialog_codepoint_length = index;
    dialogue->current_dialog_mask = stringbuilder_get_copy(buffer);
    dialogue->is_speaking = true;

    if (dialogue->chars_per_second > 0)
        dialogue->current_dialog_duration = (index / dialogue->chars_per_second) * 1000.0;
    else
        dialogue->current_dialog_duration = 0.0;
    dialogue->current_dialog_duration += index * dialogue->char_delay;
    dialogue->current_dialog_elapsed = 0.0;

    if (dialogue->current_dialog_line > 0 && dialogue->click_text) soundplayer_replay(dialogue->click_text);

    dialogue_internal_notify_script(dialogue, true);
    textsprite_set_text_intern(dialogue->texsprite_speech, true, (const char* const*)&dialogue->current_dialog_mask);
}

static void dialogue_internal_print_text(Dialogue dialogue) {
    if (!dialogue->current_dialog) return;

    DialogLine* line = &dialogue->current_dialog->lines[dialogue->current_dialog_line];
    StringBuilder buffer = dialogue->current_dialog_buffer;
    int32_t length = dialogue->current_dialog_codepoint_length;

    int32_t next_index = (int32_t)(length * (dialogue->current_dialog_elapsed / dialogue->current_dialog_duration));
    if (next_index == dialogue->current_dialog_codepoint_index && !dialogue->do_no_wait) return;

    if (dialogue->click_char) soundplayer_replay(dialogue->click_char);
    dialogue->current_dialog_codepoint_index = next_index;

    if (next_index < length) {
        stringbuilder_clear(buffer);
        stringbuilder_add_substring(buffer, line->text, 0, next_index);
        stringbuilder_add_substring(buffer, dialogue->current_dialog_mask, next_index, length);

        textsprite_set_text_intern(dialogue->texsprite_speech, true, stringbuilder_intern(buffer));
        return;
    }

    dialogue_internal_notify_script(dialogue, false);
    textsprite_set_text_intern(dialogue->texsprite_speech, true, (const char* const*)&line->text);

    dialogue->is_speaking = false;
    dialogue->current_dialog_line++;

    if (dialogue->do_no_wait) {
        dialogue->do_no_wait = false;
        dialogue_internal_prepare_print_text(dialogue);
        return;
    }

    dialogue_internal_toggle_idle(dialogue);
}

static void dialogue_internal_toggle_idle(Dialogue dialogue) {
    foreach (Portrait*, portrait, LINKEDLIST_ITERATOR, dialogue->visible_portraits) {
        if (!portrait->is_speaking) continue;
        portrait->is_speaking = false;
        if (statesprite_state_toggle(portrait->statesprite, DIALOGUE_IDLE) || statesprite_state_toggle(portrait->statesprite, NULL)) {
            AnimSprite anim = statesprite_state_get(portrait->statesprite)->animation;
            if (anim) animsprite_restart(anim);
        } else {
            dialogue_internal_stop_portrait_animation(portrait);
        }
    }
}

static void dialogue_internal_destroy_external_dialog(Dialogue dialogue) {
    if (!dialogue->dialog_external) return;

    for (int32_t i = 0; i < dialogue->dialogs_size; i++) {
        if (&dialogue->dialogs[i] == dialogue->dialog_external) return;
    }

    for (int32_t i = 0; i < dialogue->dialog_external->lines_size; i++) {
        free_chk(dialogue->dialog_external->lines[i].target_state_name);
        free_chk(dialogue->dialog_external->lines[i].text);
    }
    free_chk(dialogue->dialog_external->lines);
    free_chk(dialogue->dialog_external->full_path);
    free_chk(dialogue->dialog_external);
    dialogue->dialog_external = NULL;
}

static void dialogue_internal_parse_external_dialog(Dialogue dialogue, const char* source) {
    dialogue_internal_destroy_external_dialog(dialogue);

    Dialog* new_dialog = malloc_chk(sizeof(Dialog));
    malloc_assert(new_dialog, Dialog);

    *new_dialog = (Dialog){.lines = NULL, .lines_size = 0, .full_path = NULL};

    dialogue_internal_parse_dialog_from_string(source, new_dialog);
    dialogue->dialog_external = new_dialog;
    dialogue->current_dialog = dialogue->dialog_external;
}

static void dialogue_internal_notify_script(Dialogue dialogue, bool is_line_start) {
    if (dialogue->script == NULL) return;

    int32_t current_dialog_line = dialogue->current_dialog_line;
    const char* state_name = dialogue->current_dialog->lines[dialogue->current_dialog_line].target_state_name;
    const char* text = dialogue->current_dialog->lines[dialogue->current_dialog_line].text;

    if (is_line_start)
        luascript_notify_dialogue_line_starts(dialogue->script, current_dialog_line, state_name, text);
    else
        luascript_notify_dialogue_line_ends(dialogue->script, current_dialog_line, state_name, text);
}



static void dialogue_internal_parse_audiolist(XmlNode root_node, ArrayList audios) {
    foreach (XmlNode, node, XMLPARSER_CHILDREN_ITERATOR, root_node) {
        if (XML_TAG_NAME_EQUALS(node, "Audio")) {
            dialogue_internal_parse_audio(node, audios);
        } else {
            logger_error("dialogue_internal_parse_audiolist() unknown node: %s", xmlparser_get_tag_name(node));
        }
    }
}

static void dialogue_internal_parse_backgroundlist(XmlNode root_node, ArrayList backgrounds) {
    float max_width = FUNKIN_SCREEN_RESOLUTION_WIDTH, max_height = FUNKIN_SCREEN_RESOLUTION_HEIGHT;

    const char* base_src = xmlparser_get_attribute_value2(root_node, "baseSrc");

    foreach (XmlNode, node, XMLPARSER_CHILDREN_ITERATOR, root_node) {
        if (XML_TAG_NAME_EQUALS(node, "Image")) {
            dialogue_internal_parse_image(node, max_width, max_height, base_src, backgrounds);
            break;
        } else if (XML_TAG_NAME_EQUALS(node, "Color")) {
            dialogue_internal_parse_color(node, max_width, max_height, backgrounds);
            break;
        } else {
            logger_error("dialogue_internal_parse_backgroundlist() unknown node: %s", xmlparser_get_tag_name(node));
        }
    }
}

static void dialogue_internal_parse_portraitlist(XmlNode root_node, ArrayList portraits) {
    const char* base_model = xmlparser_get_attribute_value2(root_node, "baseModel");

    foreach (XmlNode, node, XMLPARSER_CHILDREN_ITERATOR, root_node) {
        if (XML_TAG_NAME_EQUALS(node, "Portrait")) {
            dialogue_internal_parse_portrait(node, base_model, portraits);
        } else {
            logger_error("dialogue_internal_parse_portraitlist() unknown node: %s", xmlparser_get_tag_name(node));
        }
    }
}

static void dialogue_internal_parse_animationui(XmlNode root_node, AnimsUI* anims_ui) {
    const char* animation_list = xmlparser_get_attribute_value2(root_node, "animationList");
    AnimList animlist = NULL;

    if (string_is_not_empty(animation_list)) {
        animlist = animlist_init(animation_list);
        if (!animlist) {
            logger_error_xml("dialogue_internal_parse_animationui() can not initialize:", root_node);
            return;
        }
    }

    foreach (XmlNode, node, XMLPARSER_CHILDREN_ITERATOR, root_node) {
        if (XML_TAG_NAME_EQUALS(node, "Set") || XML_TAG_NAME_EQUALS(node, "UnSet")) {
            dialogue_internal_parse_animation_uiset(node, animlist, anims_ui);
        } else {
            logger_error("dialogue_internal_parse_animationui() unknown node: %s", xmlparser_get_tag_name(node));
        }
    }

    if (animlist) animlist_destroy(&animlist);
}

static void dialogue_internal_parse_state(XmlNode root_node, ArrayList states) {
    bool initial = vertexprops_parse_boolean(root_node, "initial", false);
    const char* name = xmlparser_get_attribute_value2(root_node, "name");
    const char* if_line = xmlparser_get_attribute_value2(root_node, "ifLine");
    ArrayList actions = arraylist_init(sizeof(Action));

    foreach (XmlNode, node, XMLPARSER_CHILDREN_ITERATOR, root_node) {
        Action action = {
            .name = string_duplicate(xmlparser_get_attribute_value2(node, "name")),
            .rgba = {FLOAT_NaN, FLOAT_NaN, FLOAT_NaN, FLOAT_NaN}
        };

        if (XML_TAG_NAME_EQUALS(node, "AudioPlay")) {
            action.type = Type_AUDIO_PLAY;
        } else if (XML_TAG_NAME_EQUALS(node, "AudioPause")) {
            action.type = Type_AUDIO_PAUSE;
        } else if (XML_TAG_NAME_EQUALS(node, "AudioFadeIn")) {
            action.type = Type_AUDIO_FADEIN;
            action.duration = vertexprops_parse_float(node, "duration", 1000.0f);
        } else if (XML_TAG_NAME_EQUALS(node, "AudioFadeOut")) {
            action.type = Type_AUDIO_FADEOUT;
            action.duration = vertexprops_parse_float(node, "duration", 1000.0f);
        } else if (XML_TAG_NAME_EQUALS(node, "AudioStop")) {
            action.type = Type_AUDIO_STOP;
        } else if (XML_TAG_NAME_EQUALS(node, "AudioVolume")) {
            action.type = Type_AUDIO_VOLUME;
            action.volume = vertexprops_parse_float(node, "volume", 1.0f);
        } else if (XML_TAG_NAME_EQUALS(node, "BackgroundSet")) {
            action.type = Type_BACKGROUND_SET;
        } else if (XML_TAG_NAME_EQUALS(node, "BackgroundChange")) {
            action.type = Type_BACKGROUND_CHANGE;
        } else if (XML_TAG_NAME_EQUALS(node, "BackgroundRemove")) {
            action.type = Type_BACKGROUND_REMOVE;
        } else if (XML_TAG_NAME_EQUALS(node, "Lua")) {
            action.type = Type_LUA;
            action.lua_eval = string_duplicate(xmlparser_get_text(node));
            action.lua_function = string_duplicate(xmlparser_get_attribute_value2(node, "function"));
        } else if (XML_TAG_NAME_EQUALS(node, "Exit")) {
            action.type = Type_EXIT;
        } else if (XML_TAG_NAME_EQUALS(node, "PortraitAdd")) {
            action.type = Type_PORTRAIT_ADD;
            action.random_from_prefix = string_duplicate(xmlparser_get_attribute_value2(node, "randomFromPrefix"));
            action.no_speak = vertexprops_parse_boolean(node, "noSpeak", false);
        } else if (XML_TAG_NAME_EQUALS(node, "PortraitRemove")) {
            action.type = Type_PORTRAIT_REMOVE;
        } else if (XML_TAG_NAME_EQUALS(node, "PortraitRemoveAll")) {
            action.type = Type_PORTRAIT_REMOVEALL;
            action.animate_remove = vertexprops_parse_boolean(node, "animateRemove", false);
        } else if (XML_TAG_NAME_EQUALS(node, "AudioUI")) {
            action.type = Type_AUDIO_UI;
            action.click_text = string_duplicate(xmlparser_get_attribute_value2(node, "clickText"));
            action.click_char = string_duplicate(xmlparser_get_attribute_value2(node, "clickChar"));
        } else if (XML_TAG_NAME_EQUALS(node, "TextSpeed")) {
            action.type = Type_TEXT_SPEED;
            action.chars_per_second = vertexprops_parse_integer(node, "charsPerSecond", 0);
            action.char_delay = vertexprops_parse_integer(node, "charDelay", 75);
        } else if (XML_TAG_NAME_EQUALS(node, "TextSkip")) {
            action.type = Type_TEXT_SKIP;
        } else if (XML_TAG_NAME_EQUALS(node, "TextInstantPrint")) {
            action.type = Type_TEXT_INSTANTPRINT;
        } else if (XML_TAG_NAME_EQUALS(node, "SpeechBackground")) {
            action.type = Type_SPEECH_BACKGROUND;
            if (action.name == NULL) {
                const char* tmp = xmlparser_get_attribute_value2(node, "fromCommon");
                if (!string_equals(tmp, "none")) action.name = string_duplicate(tmp);
            }

            const char* unparsed_repeat_anim = xmlparser_get_attribute_value2(node, "repeatAnim");
            if (string_is_empty(unparsed_repeat_anim)) {
                action.repeat_anim = RepeatAnim_NONE;
            } else if (string_equals_ignore_case(unparsed_repeat_anim, "once")) {
                action.repeat_anim = RepeatAnim_ONCE;
            } else if (string_equals_ignore_case(unparsed_repeat_anim, "whilespeaks")) {
                action.repeat_anim = RepeatAnim_WHILESPEAKS;
            } else if (string_equals_ignore_case(unparsed_repeat_anim, "always")) {
                action.repeat_anim = RepeatAnim_ALWAYS;
            } else {
                action.repeat_anim = RepeatAnim_NONE;
                logger_error_xml("dialogue_internal_parse_state() unknown repeatAnim value:", node);
            }
        } else if (XML_TAG_NAME_EQUALS(node, "TextFont")) {
            action.type = Type_TEXT_FONT;
        } else if (XML_TAG_NAME_EQUALS(node, "TextColor")) {
            action.type = Type_TEXT_COLOR;
            dialogue_internal_read_color(node, action.rgba);
        } else if (XML_TAG_NAME_EQUALS(node, "TextBorderColor")) {
            action.type = Type_TEXT_BORDERCOLOR;
            dialogue_internal_read_color(node, action.rgba);
        } else if (XML_TAG_NAME_EQUALS(node, "TextBorderOffset")) {
            action.type = Type_TEXT_BORDEROFFSET;
            action.offset_x = vertexprops_parse_float(node, "x", FLOAT_NaN);
            action.offset_y = vertexprops_parse_float(node, "y", FLOAT_NaN);
        } else if (XML_TAG_NAME_EQUALS(node, "TextSize")) {
            action.type = Type_TEXT_SIZE;
            action.size = vertexprops_parse_float(node, "size", 18.0f);
        } else if (XML_TAG_NAME_EQUALS(node, "TextBorderSize")) {
            action.type = Type_TEXT_BORDERSIZE;
            action.size = vertexprops_parse_float(node, "size", 2.0f);
        } else if (XML_TAG_NAME_EQUALS(node, "TextBorderEnable")) {
            action.type = Type_TEXT_BORDERENABLE;
            action.enabled = vertexprops_parse_boolean(node, "enabled", false);
        } else if (XML_TAG_NAME_EQUALS(node, "TextParagraphSpace")) {
            action.type = Type_TEXT_PARAGRAPHSPACE;
            action.size = vertexprops_parse_float(node, "size", 0.0f);
        } else if (XML_TAG_NAME_EQUALS(node, "TextAlign")) {
            action.type = Type_TEXT_ALIGN;
            action.align_vertical = dialogue_internal_read_align(node, "vertical");
            action.align_horizontal = dialogue_internal_read_align(node, "horizontal");
            action.align_paragraph = dialogue_internal_read_align(node, "paragraph");
        } else if (XML_TAG_NAME_EQUALS(node, "RunMultipleChoice")) {
            action.type = Type_RUNMULTIPLECHOICE;
        } else if (XML_TAG_NAME_EQUALS(node, "Title")) {
            action.type = Type_TITLE;
            action.title = string_duplicate(xmlparser_get_text(node));
        } else if (XML_TAG_NAME_EQUALS(node, "NoWait")) {
            action.type = Type_NOWAIT;
        } else {
            logger_error_xml("dialogue_internal_parse_state() unknown state action:", node);
            free_chk(action.name);
            continue;
        }

        arraylist_add(actions, &action);
    }

    State state = {
        .name = string_duplicate(name),
        .if_line = string_duplicate(if_line),
        .initial = initial,
        .actions = NULL,
        .actions_size = 0
    };

    arraylist_destroy2(&actions, &state.actions_size, (void**)&state.actions);
    arraylist_add(states, &state);
}

static void dialogue_internal_parse_multiplechoice(XmlNode root_node, AnimList animlist, ArrayList dialogs, ArrayList multiplechoices) {
    const char* title = xmlparser_get_attribute_value2(root_node, "title");
    const char* name = xmlparser_get_attribute_value2(root_node, "name");
    const char* orientation = xmlparser_get_attribute_value2(root_node, "orientation");
    bool can_leave = vertexprops_parse_boolean(root_node, "canLeave", false);
    const char* state_on_leave = xmlparser_get_attribute_value2(root_node, "stateOnLeave");
    uint32_t icon_color = vertexprops_parse_hex2(xmlparser_get_attribute_value2(root_node, "selectorIconColor"), 0x00FFF, false);
    const char* icon_model = xmlparser_get_attribute_value2(root_node, "selectorIconColor");
    const char* icon_model_name = xmlparser_get_attribute_value2(root_node, "selectorIconModelName");
    int32_t default_index = (int32_t)vertexprops_parse_integer(root_node, "defaultIndex", 0);
    float font_size = vertexprops_parse_float(root_node, "fontSize", -1.0f);

    bool is_vertical;

    if (string_is_empty(orientation) || string_equals(orientation, "vertical")) {
        is_vertical = true;
    } else if (string_equals(orientation, "horizontal")) {
        is_vertical = false;
    } else {
        is_vertical = true;
        logger_error("dialogue_internal_parse_multiple_choice() unknown orientation value: %s", orientation);
    }

    Texture texture = NULL;
    AnimSprite anim = NULL;

    if (icon_model) {
        if (modelholder_utils_is_known_extension(icon_model)) {
            ModelHolder modeholder = modelholder_init(icon_model);
            if (!modeholder) {
                logger_error("dialogue_internal_parse_multiple_choice() can not initialize: %s", icon_model);
            } else {
                texture = modelholder_get_texture(modeholder, true);
                anim = modelholder_create_animsprite(modeholder, icon_model_name ? icon_model_name : DIALOGUE_ICON, true, false);
                icon_color = modelholder_get_vertex_color(modeholder);
                modelholder_destroy(&modeholder);
            }
        } else {
            texture = texture_init(icon_model);
        }
    }

    if (!anim) anim = animsprite_init_from_animlist(animlist, DIALOGUE_ICON);

    Sprite icon = sprite_init(texture);
    sprite_set_vertex_color_rgb8(icon, icon_color);
    sprite_external_animation_set(icon, anim);

    Sprite hint = sprite_init_from_rgb8(0xF9CF51);
    sprite_set_alpha(hint, 0.0f);

    MultipleChoice multiplechoice = {
        .icon = icon,
        .hint = hint,
        .name = string_duplicate(name),
        .title = string_duplicate(title),
        .can_leave = can_leave,
        .state_on_leave = string_duplicate(state_on_leave),
        .default_index = default_index,
        .font_size = font_size,
        .choices = NULL,
        .choices_size = 0,
        .is_vertical = is_vertical
    };

    ArrayList choices = arraylist_init(sizeof(Choice));

    foreach (XmlNode, node, XMLPARSER_CHILDREN_ITERATOR, root_node) {
        if (XML_TAG_NAME_EQUALS(node, "Choice")) {
            dialogue_internal_parse_choice(node, dialogs, choices);
        } else {
            logger_error("dialogue_internal_parse_multiplechoice() unknown: %s", xmlparser_get_tag_name(node));
        }
    }

    arraylist_destroy2(&choices, &multiplechoice.choices_size, (void**)&multiplechoice.choices);
    arraylist_add(multiplechoices, &multiplechoice);
}

static void dialogue_internal_parse_speech_imagelist(XmlNode root_node, ArrayList speechimages) {
    const char* base_src = xmlparser_get_attribute_value2(root_node, "baseSrc");
    float text_x = 0.0f;
    float text_y = 0.0f;
    float text_width = 0.0f;
    float text_height = 0.0f;
    float portrait_line_x = 0.0f;
    float portrait_line_y = 0.0f;
    float portrait_line_width = 0.0f;
    float title_x = 0.0f;
    float title_y = 0.0f;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    float offset_idle_x = 0.0f;
    float offset_idle_y = 0.0f;
    float offset_open_x = 0.0f;
    float offset_open_y = 0.0f;
    Align align_vertical = ALIGN_NONE;
    Align align_horizontal = ALIGN_NONE;
    bool portrait_line_is_relative = false;
    bool title_is_relative = false;
    bool text_is_relative = false;

    foreach (XmlNode, node, XMLPARSER_CHILDREN_ITERATOR, root_node) {
        if (XML_TAG_NAME_EQUALS(node, "SpeechImage")) {
            SpeechImage* speechimage = dialogue_internal_parse_speech_image(node, base_src, speechimages);
            if (!speechimage) continue;

            speechimage->text_x = text_x;
            speechimage->text_y = text_y;
            speechimage->text_width = text_width;
            speechimage->text_height = text_height;
            speechimage->portrait_line_x = portrait_line_x;
            speechimage->portrait_line_y = portrait_line_y;
            speechimage->portrait_line_width = portrait_line_width;
            speechimage->title_x = title_x;
            speechimage->title_y = title_y;
            speechimage->offset_x = offset_x;
            speechimage->offset_y = offset_y;
            speechimage->portrait_line_is_relative = portrait_line_is_relative;
            speechimage->title_is_relative = title_is_relative;
            speechimage->text_is_relative = text_is_relative;
            speechimage->align_vertical = align_vertical;
            speechimage->align_horizontal = align_horizontal;

            foreach (StateSpriteState*, state, LINKEDLIST_ITERATOR, statesprite_state_list(speechimage->statesprite)) {
                if (string_equals(state->state_name, DIALOGUE_OPEN)) {
                    state->offset_x = offset_open_x;
                    state->offset_y = offset_open_y;
                } else if (string_equals(state->state_name, DIALOGUE_IDLE)) {
                    state->offset_x = offset_idle_x;
                    state->offset_y = offset_idle_y;
                }
            }
        } else if (XML_TAG_NAME_EQUALS(node, "TextBox")) {
            text_x = vertexprops_parse_float(node, "x", text_x);
            text_y = vertexprops_parse_float(node, "y", text_y);
            text_width = vertexprops_parse_float(node, "width", text_width);
            text_height = vertexprops_parse_float(node, "height", text_height);
            text_is_relative = vertexprops_parse_boolean(node, "isRelative", text_is_relative);
        } else if (XML_TAG_NAME_EQUALS(node, "PortraitLine")) {
            portrait_line_x = vertexprops_parse_float(node, "x", portrait_line_x);
            portrait_line_y = vertexprops_parse_float(node, "y", portrait_line_y);
            portrait_line_width = vertexprops_parse_float(node, "width", portrait_line_width);
            portrait_line_is_relative = vertexprops_parse_boolean(node, "isRelative", portrait_line_is_relative);
        } else if (XML_TAG_NAME_EQUALS(node, "TitleLocation")) {
            title_x = vertexprops_parse_float(node, "titleLeft", title_x);
            title_y = vertexprops_parse_float(node, "titleBottom", title_y);
            title_is_relative = vertexprops_parse_boolean(node, "isRelative", title_is_relative);
        } else if (XML_TAG_NAME_EQUALS(node, "Location")) {
            offset_x = vertexprops_parse_float(node, "x", offset_x);
            offset_y = vertexprops_parse_float(node, "y", offset_y);
        } else if (XML_TAG_NAME_EQUALS(node, "OffsetIdle")) {
            offset_idle_x = vertexprops_parse_float(node, "x", offset_idle_x);
            offset_idle_y = vertexprops_parse_float(node, "y", offset_idle_y);
        } else if (XML_TAG_NAME_EQUALS(node, "OffsetOpen")) {
            offset_open_x = vertexprops_parse_float(node, "x", offset_open_x);
            offset_open_y = vertexprops_parse_float(node, "y", offset_open_y);
        } else if (XML_TAG_NAME_EQUALS(node, "PortraitAlign")) {
            align_vertical = dialogue_internal_read_align(node, "vertical");
            align_horizontal = dialogue_internal_read_align(node, "horizontal");
        } else {
            logger_error("dialogue_internal_parse_speechimagelist() unknown node: %s", xmlparser_get_tag_name(node));
        }
    }
}


static void dialogue_internal_parse_import_portraitlist(XmlNode root_node, ArrayList portraits) {
    const char* dialogue_src = xmlparser_get_attribute_value2(root_node, "dialogueSrc");
    if (string_is_empty(dialogue_src)) {
        logger_error_xml("dialogue_internal_parse_importportraitlist() missing dialogueSrc: ", root_node);
        return;
    }

    if (string_lowercase_ends_with(dialogue_src, ".json")) {
        dialogue_internal_load_psych_character_json(dialogue_src, portraits);
        return;
    }

    XmlParser xmlparser = xmlparser_init(dialogue_src);
    if (!xmlparser) {
        logger_error("dialogue_internal_parse_importportraitlist() can not load: %s", dialogue_src);
        return;
    }

    fs_folder_stack_push();
    fs_set_working_folder(dialogue_src, true);

    foreach (XmlNode, node, XMLPARSER_CHILDREN_ITERATOR, root_node) {
        if (!XML_TAG_NAME_EQUALS(node, "Definition")) continue;
        foreach (XmlNode, node2, XMLPARSER_CHILDREN_ITERATOR, root_node) {
            if (!XML_TAG_NAME_EQUALS(node2, "PortraitList")) continue;
            dialogue_internal_parse_import_portraitlist(node2, portraits);
        }
    }

    fs_folder_stack_pop();
    xmlparser_destroy(&xmlparser);
}

static void dialogue_internal_parse_audio(XmlNode node, ArrayList audios) {
    //<Audio name="bg_music2" src="/assets/music/weeb.ogg" volume="1.0" looped="true" defaultAs="clickText|clickChar" />
    const char* name = xmlparser_get_attribute_value2(node, "name");
    const char* src = xmlparser_get_attribute_value2(node, "src");
    float volume = vertexprops_parse_float(node, "volume", 1.0f);
    bool looped = vertexprops_parse_boolean(node, "looped", false);

    // avoid duplicates
    for (int32_t i = 0, size = arraylist_size(audios); i < size; i++) {
        if (HAS_DUPL_ENTRY(audios, i, Audio)) return;
    }

    if (string_is_empty(name)) {
        logger_error_xml("dialogue_internal_parse_audio() missing name:", node);
        return;
    }
    if (string_is_empty(src)) {
        logger_error_xml("dialogue_internal_parse_audio() missing src:", node);
        return;
    }

    SoundPlayer soundplayer = soundplayer_init(src);
    if (!soundplayer) {
        logger_error_xml("dialogue_internal_parse_audio() can not initialize:", node);
        return;
    }

    soundplayer_set_volume(soundplayer, volume);
    soundplayer_loop_enable(soundplayer, looped);

    Audio audio = {
        .name = string_duplicate(name),
        .was_playing = false,
        .soundplayer = soundplayer
    };

    arraylist_add(audios, &audio);
}

static void dialogue_internal_parse_image(XmlNode node, float max_width, float max_height, const char* base_src, ArrayList backgrounds) {
    //<Image
    //          name="scene0"
    //          src="/assets/image/theater.xml"
    //          entryName="floor"
    //          center="true"
    //          cover="true"
    //          looped="true"
    //          alpha="0.7"
    //          />

    const char* name = xmlparser_get_attribute_value2(node, "name");
    const char* src = xmlparser_get_attribute_value2(node, "src");
    const char* entry_name = xmlparser_get_attribute_value2(node, "entryName");
    bool center = vertexprops_parse_boolean(node, "center", false);
    bool cover = vertexprops_parse_boolean(node, "cover", false);
    bool looped = vertexprops_parse_boolean(node, "looped", false);
    bool has_looped = xmlparser_has_attribute(node, "looped");
    float alpha = vertexprops_parse_float(node, "alpha", 1.0f);

    // avoid duplicates
    for (int32_t i = 0, size = arraylist_size(backgrounds); i < size; i++) {
        if (HAS_DUPL_ENTRY(backgrounds, i, Background)) return;
    }

    if (string_is_empty(name)) {
        logger_error_xml("dialogue_internal_parse_image() missing name:", node);
        return;
    }

    if (string_is_empty(src)) src = base_src;
    if (string_is_empty(src)) {
        logger_error_xml("dialogue_internal_parse_image() missing src:", node);
        return;
    }

    Texture texture = NULL;
    AnimSprite animsprite = NULL;
    uint32_t vertex_color_rgb8 = 0xFFFFFF;
    bool init_failed;

    if (modelholder_utils_is_known_extension(src)) {
        ModelHolder modelholder = modelholder_init(src);
        init_failed = !modelholder;
        if (modelholder) {
            texture = modelholder_get_texture(modelholder, true);
            animsprite = modelholder_create_animsprite(modelholder, entry_name, true, false);
            vertex_color_rgb8 = modelholder_get_vertex_color(modelholder);
            if (has_looped && animsprite) animsprite_set_loop(animsprite, looped ? -1 : 1);
            modelholder_destroy(&modelholder);
        }
    } else {
        texture = texture_init(src);
        init_failed = !texture;
    }

    if (init_failed) {
        logger_error_xml("dialogue_internal_parse_image() can not initialize:", node);
        return;
    }

    Sprite sprite = sprite_init(texture);
    sprite_set_vertex_color_rgb8(sprite, vertex_color_rgb8);
    sprite_external_animation_set(sprite, animsprite);
    sprite_set_draw_location(sprite, 0.0f, 0.0f);
    sprite_set_alpha(sprite, alpha);
    imgutils_calc_resize_sprite(sprite, max_width, max_height, cover, center);

    Background background = {
        .name = string_duplicate(name),
        .sprite = sprite
    };

    arraylist_add(backgrounds, &background);
}

static void dialogue_internal_parse_color(XmlNode node, float max_width, float max_height, ArrayList backgrounds) {
    //<Color name="faded_white" rgba="0xFFFFFF7F" />
    //<Color name="faded_red" r="1.0" g="0.0" g="0.0" a="0.7" />

    const char* name = xmlparser_get_attribute_value2(node, "name");
    if (string_is_empty(name)) {
        logger_error_xml("dialogue_internal_parse_image() missing name:", node);
        return;
    }

    // avoid duplicates
    for (int32_t i = 0, size = arraylist_size(backgrounds); i < size; i++) {
        if (HAS_DUPL_ENTRY(backgrounds, i, Background)) return;
    }

    RGBA rgba = {0.0f, 0.0f, 0.0f, 1.0f};
    dialogue_internal_read_color(node, rgba);

    Sprite sprite = sprite_init(NULL);
    sprite_set_vertex_color(sprite, rgba[0], rgba[1], rgba[2]);
    sprite_set_alpha(sprite, rgba[3]);
    sprite_set_draw_size(sprite, max_width, max_height);

    Background background = {
        .name = string_duplicate(name),
        .sprite = sprite
    };

    arraylist_add(backgrounds, &background);
}

static void dialogue_internal_parse_font(XmlNode node, ArrayList fonts) {
    //<Font name="vcr" src="vcr.ttf" glyphSuffix="bold" glyphAnimated="false"  colorByAddition="true" />

    const char* name = xmlparser_get_attribute_value2(node, "name");
    const char* src = xmlparser_get_attribute_value2(node, "src");
    const char* glyph_suffix = xmlparser_get_attribute_value2(node, "glyphSuffix");
    bool glyph_animated = vertexprops_parse_boolean(node, "glyphAnimated", false);
    bool color_by_addition = vertexprops_parse_boolean(node, "colorByAddition", false);

    if (string_is_empty(name)) {
        logger_error_xml("dialogue_internal_parse_font() missing name:", node);
        return;
    }
    if (string_is_empty(src)) {
        logger_error_xml("dialogue_internal_parse_font() missing src:", node);
        return;
    }
    if (!fs_file_exists(src)) {
        logger_error_xml("dialogue_internal_parse_font() font file not found:", node);
        return;
    }

    // avoid duplicates
    for (int32_t i = 0, size = arraylist_size(fonts); i < size; i++) {
        if (HAS_DUPL_ENTRY(fonts, i, Font)) return;
    }

    void* instance;
    bool is_atlas;

    if (atlas_utils_is_known_extension(src)) {
        instance = fontglyph_init(src, glyph_suffix, glyph_animated);
        is_atlas = true;
    } else {
        instance = fonttype_init(src);
        is_atlas = false;
    }

    if (!instance) {
        logger_error_xml("dialogue_internal_parse_font() can not initialize:", node);
        return;
    }

    Font font = {
        .name = string_duplicate(name),
        .fontholder = fontholder_init2(instance, is_atlas, color_by_addition)
    };

    arraylist_add(fonts, &font);
}

static void dialogue_internal_parse_portrait(XmlNode node, const char* base_model, ArrayList portraits) {
    // <Portrait
    //          name="senpai" src="/assets/image/senpai.png" scale="1.0"
    //          speakAnim="talking" idleAnim="idle_anim" anim="talking_and_idle_anim"
    //          speakAnimLooped="true|false" idleAnimLooped="true|false" animLooped="true|false"
    //          mirror="true|false"
    //          positionPercent="0.0" position="left|center|right"
    //          x="0" y="0"
    //          offestSpeakX="0" offestSpeakY="0"
    //          offestIdleX="0" offestIdleY="0"
    // />

    const char* name = xmlparser_get_attribute_value2(node, "name");
    const char* src = xmlparser_get_attribute_value2(node, "src");
    float scale = vertexprops_parse_float(node, "scale", 1.0f);
    const char* speak_anim = xmlparser_get_attribute_value2(node, "speakAnim");
    const char* idle_anim = xmlparser_get_attribute_value2(node, "idleAnim");
    const char* simple_anim = xmlparser_get_attribute_value2(node, "anim");
    bool speak_anim_looped = vertexprops_parse_boolean(node, "speakAnimLooped", true);
    bool idle_anim_looped = vertexprops_parse_boolean(node, "idleAnimLooped", true);
    bool simple_anim_looped = vertexprops_parse_boolean(node, "animLooped", true);
    bool mirror = vertexprops_parse_boolean(node, "mirror", false);
    float position = vertexprops_parse_float(node, "positionPercent", 0.0f);
    const char* position_align = xmlparser_get_attribute_value2(node, "position");
    float x = vertexprops_parse_float(node, "x", 0.0f);
    float y = vertexprops_parse_float(node, "y", 0.0f);
    float offset_speak_x = vertexprops_parse_float(node, "offestSpeakX", 0.0f);
    float offset_speak_y = vertexprops_parse_float(node, "offestSpeakY", 0.0f);
    float offset_idle_x = vertexprops_parse_float(node, "offestIdleX", 0.0f);
    float offset_idle_y = vertexprops_parse_float(node, "offestIdleY", 0.0f);

    // avoid duplicates
    for (int32_t i = 0, size = arraylist_size(portraits); i < size; i++) {
        if (HAS_DUPL_ENTRY(portraits, i, Portrait)) return;
    }

    if (string_equals_ignore_case(position_align, "left")) {
        position = 0.0f;
    } else if (string_equals_ignore_case(position_align, "center")) {
        position = 0.5f;
    } else if (string_equals_ignore_case(position_align, "right")) {
        position = 1.0f;
    } else if (string_is_empty(position_align)) {
        // nothing to do
    } else {
        logger_warn_xml("dialogue_internal_parse_portrait() unknown position:", node);
    }

    if (string_is_empty(name)) {
        logger_error_xml("dialogue_internal_parse_portrait() missing name:", node);
        return;
    }

    if (string_is_empty(src)) src = base_model;
    if (string_is_empty(src)) {
        logger_error_xml("dialogue_internal_parse_portrait() missing src:", node);
        return;
    }

    StateSprite statesprite = NULL;

    if (modelholder_utils_is_known_extension(src)) {
        ModelHolder modelholder = modelholder_init(src);
        if (!modelholder) goto L_check_failed;

        statesprite = statesprite_init_from_texture(NULL);
        statesprite_change_draw_size_in_atlas_apply(statesprite, true, scale);
        statesprite_set_draw_location(statesprite, 0.0f, 0.0f);

        dialogue_internal_add_state(statesprite, modelholder, simple_anim, NULL, scale, simple_anim_looped);
        dialogue_internal_add_state(statesprite, modelholder, idle_anim, DIALOGUE_IDLE, scale, false);
        dialogue_internal_add_state(statesprite, modelholder, speak_anim, DIALOGUE_SPEAK, scale, false);

        modelholder_destroy(&modelholder);
    } else {
        Texture texture = texture_init(src);
        if (!texture) goto L_check_failed;

        statesprite = statesprite_init_from_texture(texture);
        statesprite_set_draw_location(statesprite, 0.0f, 0.0f);

        float orig_width = -1.0f, orig_height = -1.0f;
        texture_get_original_dimmensions(texture, &orig_width, &orig_height);
        statesprite_set_draw_size(statesprite, orig_width * scale, orig_height * scale);
    }

L_check_failed:
    if (!statesprite) {
        logger_error_xml("dialogue_internal_parse_portrait() can not initialize:", node);
        return;
    }

    foreach (StateSpriteState*, state, LINKEDLIST_ITERATOR, statesprite_state_list(statesprite)) {
        state->offset_x += x;
        state->offset_y += y;
    }
    statesprite_flip_texture(statesprite, mirror, unset);
    // statesprite_flip_texture_enable_correction(statesprite, false);

    Portrait portrait = {
        .name = string_duplicate(name),
        .statesprite = statesprite,
        .position = position,
        .is_removed = false,
        .is_added = false,
        .is_speaking = false,
        .offset_speak_x = offset_speak_x,
        .offset_speak_y = offset_speak_y,
        .offset_idle_x = offset_idle_x,
        .offset_idle_y = offset_idle_y,
        .has_speak = statesprite_state_has(statesprite, DIALOGUE_SPEAK),
        .has_idle = statesprite_state_has(statesprite, DIALOGUE_IDLE),
        .speak_anim_looped = speak_anim_looped,
        .idle_anim_looped = idle_anim_looped
    };

    arraylist_add(portraits, &portrait);
}

static void dialogue_internal_parse_animation_uiset(XmlNode node, AnimList animlist, AnimsUI* anims_ui) {
    //<Set name="backgroundIn|backgroundOut" anim="anim123" />
    //<Set name="portraitLeftIn|portraitCenterIn|portraitRightIn" anim="anim123" />
    //<Set name="portraitIn|portraitOut" anim="anim123" />
    //<Set name="portraitLeftOut|portraitCenterOut|portraitRightOut" anim="anim123" />

    const char* name = xmlparser_get_attribute_value2(node, "name");
    const char* anim = xmlparser_get_attribute_value2(node, "anim");

    if (string_is_empty(name)) {
        logger_error_xml("dialogue_internal_parse_animation_ui_set() missing name:", node);
        return;
    }

    AnimSprite animsprite = NULL;
    if (string_is_not_empty(anim)) {
        if (!animlist) {
            logger_error_xml("dialogue_internal_parse_animation_ui_set() can not initialize without animlist:", node);
            return;
        }
        animsprite = animsprite_init_from_animlist(animlist, anim);
        if (!animsprite) {
            logger_error_xml("dialogue_internal_parse_animation_ui_set() can not initialize:", node);
            return;
        }
    }

    AnimSprite old_anim = NULL;

    if (string_equals_ignore_case(name, "portraitin")) {
        if (anims_ui->portrait_left_in) animsprite_destroy(&anims_ui->portrait_left_in);
        if (anims_ui->portrait_center_in) animsprite_destroy(&anims_ui->portrait_center_in);
        if (anims_ui->portrait_right_in) animsprite_destroy(&anims_ui->portrait_right_in);

        anims_ui->portrait_left_in = animsprite ? animsprite_clone(animsprite) : NULL;
        anims_ui->portrait_center_in = animsprite ? animsprite_clone(animsprite) : NULL;
        anims_ui->portrait_right_in = animsprite ? animsprite_clone(animsprite) : NULL;
    }
    if (string_equals_ignore_case(name, "portraitout")) {
        if (anims_ui->portrait_center_out) animsprite_destroy(&anims_ui->portrait_center_out);
        if (anims_ui->portrait_right_out) animsprite_destroy(&anims_ui->portrait_right_out);
        if (anims_ui->portrait_right_out) animsprite_destroy(&anims_ui->portrait_right_out);

        anims_ui->portrait_left_out = animsprite ? animsprite_clone(animsprite) : NULL;
        anims_ui->portrait_center_out = animsprite ? animsprite_clone(animsprite) : NULL;
        anims_ui->portrait_right_out = animsprite ? animsprite_clone(animsprite) : NULL;
    } else if (string_equals_ignore_case(name, "portraitleftin")) {
        old_anim = anims_ui->portrait_left_in;
        anims_ui->portrait_left_in = animsprite;
    } else if (string_equals_ignore_case(name, "portraitcenterin")) {
        old_anim = anims_ui->portrait_center_in;
        anims_ui->portrait_center_in = animsprite;
    } else if (string_equals_ignore_case(name, "portrairighttin")) {
        old_anim = anims_ui->portrait_right_in;
        anims_ui->portrait_right_in = animsprite;
    } else if (string_equals_ignore_case(name, "portraitleftout")) {
        old_anim = anims_ui->portrait_left_out;
        anims_ui->portrait_left_out = animsprite;
    } else if (string_equals_ignore_case(name, "portraitcenterout")) {
        old_anim = anims_ui->portrait_center_out;
        anims_ui->portrait_center_out = animsprite;
    } else if (string_equals_ignore_case(name, "portraitrightout")) {
        old_anim = anims_ui->portrait_right_out;
        anims_ui->portrait_right_out = animsprite;
    } else if (string_equals_ignore_case(name, "backgroundin")) {
        old_anim = anims_ui->background_in;
        anims_ui->background_in = animsprite;
    } else if (string_equals_ignore_case(name, "backgroundout")) {
        old_anim = anims_ui->background_out;
        anims_ui->background_out = animsprite;
    } else if (string_equals_ignore_case(name, "open")) {
        old_anim = anims_ui->open;
        anims_ui->open = animsprite;
    } else if (string_equals_ignore_case(name, "close")) {
        old_anim = anims_ui->close;
        anims_ui->close = animsprite;
    } else {
        logger_error_xml("dialogue_internal_parse_animation_ui_set() unknown name:", node);
        if (animsprite) animsprite_destroy(&animsprite);
    }

    if (old_anim) animsprite_destroy(&old_anim);
}

static void dialogue_internal_parse_choice(XmlNode node, ArrayList dialogs, ArrayList choices) {
    // <Choice
    //          text="Exit"
    //          dialogFile="dialogs.txt"
    //          exit="true|false"
    //          runMultipleChoice="questions"
    //          luaFunction="somefunc"
    //          luaEval="doexit('from leave', true)"
    // >

    const char* text = xmlparser_get_attribute_value2(node, "text");
    const char* dialogs_file = xmlparser_get_attribute_value2(node, "dialogFile");
    bool exit = vertexprops_parse_boolean(node, "exit", false);
    const char* run_multiple_choice = xmlparser_get_attribute_value2(node, "runMultipleChoice");
    const char* lua_function = xmlparser_get_attribute_value2(node, "luaFunction");
    const char* lua_eval = xmlparser_get_attribute_value2(node, "luaEval");

    if (text == NULL) {
        logger_error_xml("dialogue_internal_parse_choice() missing text in:", node);
        return;
    }

    int32_t dialog_id = -1;

    if (dialogs_file)
        dialog_id = dialogue_internal_parse_dialog_from_file(dialogs_file, dialogs);

    Choice choice = {
        .text = string_duplicate(text),
        .dialog_id = dialog_id,
        .exit = exit,
        .lua_eval = string_duplicate(lua_eval),
        .lua_function = string_duplicate(lua_function),
        .run_multiple_choice = string_duplicate(run_multiple_choice)
    };

    arraylist_add(choices, &choice);
}

static SpeechImage* dialogue_internal_parse_speech_image(XmlNode node, const char* base_src, ArrayList speechimages) {
    // <Image
    //          name="normal"
    //          mirror="true|false"
    //          openAnim="Speech Bubble Normal Open"
    //          idleAnim="speech bubble normal"
    //          idleLooped="true|false"
    //          src="/assets/image/model.xml"
    //          scale="1.0"
    //  >

    const char* name = xmlparser_get_attribute_value2(node, "name");
    float scale = vertexprops_parse_float(node, "scale", 1.0f);
    const char* open_anim = xmlparser_get_attribute_value2(node, "openAnim");
    const char* idle_anim = xmlparser_get_attribute_value2(node, "idleAnim");
    bool idle_looped = vertexprops_parse_boolean(node, "idleLooped", true);
    bool mirror = vertexprops_parse_boolean(node, "mirror", false);
    bool has_mirror = xmlparser_has_attribute(node, "mirror");
    const char* src = xmlparser_get_attribute_value2(node, "src");

    if (string_is_empty(name)) {
        logger_error_xml("dialogue_internal_parse_speechimage() missing name: ", node);
        return NULL;
    }

    // avoid duplicates
    for (int32_t i = 0, size = arraylist_size(speechimages); i < size; i++) {
        if (HAS_DUPL_ENTRY(speechimages, i, SpeechImage)) return NULL;
    }

    if (string_is_empty(src)) src = base_src;
    if (string_is_empty(src)) {
        logger_error_xml("dialogue_internal_parse_speechimage() missing src: ", node);
        return NULL;
    }

    StateSprite statesprite = NULL;

    if (modelholder_utils_is_known_extension(src)) {
        ModelHolder modelholder = modelholder_init(src);
        if (!modelholder) goto L_check_failed;

        statesprite = statesprite_init_from_texture(NULL);
        statesprite_set_draw_location(statesprite, 0.0f, 0.0f);

        dialogue_internal_add_state(statesprite, modelholder, open_anim, DIALOGUE_OPEN, scale, false);
        dialogue_internal_add_state(statesprite, modelholder, idle_anim, DIALOGUE_IDLE, scale, idle_looped);

        if (linkedlist_count(statesprite_state_list(statesprite)) < 1) {
            Texture texture = modelholder_get_texture(modelholder, true);
            if (texture) {
                float orig_width = -1.0f, orig_height = -1.0f;
                texture_get_original_dimmensions(texture, &orig_width, &orig_height);
                statesprite_set_texture(statesprite, texture, true);
                statesprite_set_draw_size(statesprite, orig_width * scale, orig_height * scale);
            } else {
                statesprite_destroy(&statesprite);
                statesprite = NULL;
            }
        }

        modelholder_destroy(&modelholder);
    } else {
        Texture texture = texture_init(src);
        if (!texture) goto L_check_failed;

        float orig_width = -1.0f, orig_height = -1.0f;
        texture_get_original_dimmensions(texture, &orig_width, &orig_height);

        if (scale >= 0.0f) {
            orig_width *= scale;
            orig_height *= scale;
        }

        statesprite = statesprite_init_from_texture(texture);
        statesprite_set_draw_location(statesprite, 0.0f, 0.0f);
        statesprite_set_draw_size(statesprite, orig_width, orig_height);
    }

L_check_failed:
    if (!statesprite) {
        logger_error_xml("dialogue_internal_parse_speechimage() can not initialize: ", node);
        return NULL;
    }

    if (has_mirror) statesprite_flip_texture(statesprite, mirror, unset);
    if (scale > 0.0f) statesprite_change_draw_size_in_atlas_apply(statesprite, true, scale);

    SpeechImage* speechimage = arraylist_add(
        speechimages,
        &(SpeechImage){
            .name = string_duplicate(name),
            .statesprite = statesprite,
        }
    );

    return speechimage;
}

static void dialogue_internal_load_psych_character_json(const char* src, ArrayList portraits) {
    JSONToken json = json_load_from(src);
    if (!json) {
        logger_error("dialogue_internal_load_psych_character_json() can not load: %s", src);
        return;
    }

    JSONToken animations = json_read_array(json, "animations");
    int32_t animations_length = json_read_array_length(animations);

    fs_folder_stack_push();
    fs_set_working_folder(src, true);

    float position_x = -1.0f, position_y = -1.0f;
    dialogue_internal_read_offset(json, "position", &position_x, &position_y);
    float scale = (float)json_read_number_double(json, "scale", 1.0);
    float dialogue_pos;

    const char* unparsed_dialogue_pos = json_read_string(json, "dialogue_pos", NULL);
    if (string_is_empty(unparsed_dialogue_pos) || string_equals(unparsed_dialogue_pos, "left")) {
        dialogue_pos = 0.0f;
    } else if (string_equals(unparsed_dialogue_pos, "center")) {
        dialogue_pos = 0.5f;
    } else if (string_equals(unparsed_dialogue_pos, "right")) {
        dialogue_pos = 1.0f;
    } else {
        dialogue_pos = 0.0f;
        logger_error("dialogue_internal_load_psych_character_json() unrecognized dialogue_pos: %s", src);
    }

    const char* image = json_read_string(json, "image", NULL);
    if (!image) {
        logger_error("dialogue_internal_load_psych_character_json() missing 'image' of json: %s", src);
        goto L_return;
    } else if (string_index_of_char(image, 0, '.') >= 0) {
        // append atlas extension
        char* tmp = string_concat(2, image, ".xml");
        image = tmp;
    }

    //
    // Note:
    //      There no such path like "/assets/shared/images/dialogue/required_atlas_file.xml"
    //      Attempt load the atlas from the current folder "./required_atlas_file.xml"
    //      or fallback to "/assets/common/image/dialogue/required_atlas_file.xml"
    //
    ModelHolder modelholder = NULL;

    if (fs_file_exists(image)) {
        modelholder = modelholder_init(src);
    } else {
        // try load from common folder
        char* tmp = string_concat(2, "/assets/common/image/dialogue/", image);
        if (fs_file_exists(tmp)) {
            modelholder = modelholder_init(tmp);
        }
        free_chk(tmp);
    }

    if (!modelholder) {
        logger_error("dialogue_internal_load_psych_character_json() unreconized image path: %s", image);
        goto L_return;
    }

    // parse animations
    for (int32_t i = 0; i < animations_length; i++) {
        JSONToken obj = json_read_array_item_object(animations, i);

        float idle_offset_x = -1.0f, idle_offset_y = -1.0f;
        float loop_offset_x = -1.0f, loop_offset_y = -1.0f;
        const char *anim, *idle_name, *loop_name;

        dialogue_internal_read_offset(obj, "idle_offsets", &idle_offset_x, &idle_offset_y);
        dialogue_internal_read_offset(obj, "loop_offsets", &loop_offset_x, &loop_offset_y);
        anim = json_read_string(obj, "anim", NULL);
        idle_name = json_read_string(obj, "idle_name", NULL);
        loop_name = json_read_string(obj, "loop_name", NULL);

        // prepare portrait
        AnimSprite anim_idle = modelholder_create_animsprite(modelholder, idle_name, true, false);
        AnimSprite anim_speak = modelholder_create_animsprite(modelholder, loop_name, true, false);
        StateSprite statesprite = statesprite_init_from_texture(NULL);
        statesprite_set_draw_location(statesprite, 0.0f, 0.0f);

        // accumulate offsets, assume 'position' as offset of portrait line
        idle_offset_x += position_x;
        idle_offset_y += position_y;
        loop_offset_x += position_x;
        loop_offset_y += position_y;

        if (scale >= 0.0f) statesprite_change_draw_size_in_atlas_apply(statesprite, true, scale);
        dialogue_internal_compute_state(modelholder, statesprite, anim_idle, idle_offset_x, idle_offset_y, false);
        dialogue_internal_compute_state(modelholder, statesprite, anim_speak, loop_offset_x, loop_offset_y, true);

        Portrait portrait = {
            .name = string_duplicate(anim),
            .is_added = false,
            .is_removed = false,
            .is_speaking = false,
            .position = dialogue_pos,
            .statesprite = statesprite
        };
        arraylist_add(portraits, &portrait);
    }

    modelholder_destroy(&modelholder);

L_return:
    fs_folder_stack_pop();
    json_destroy(&json);
}

static int32_t dialogue_internal_parse_dialog_from_file(const char* src, ArrayList dialogs) {
    char* full_path = fs_get_full_path_and_override(src);

    // check if is already loaded
    int32_t id = 0;
    foreach (Dialog*, existing_dialog, ARRAYLIST_ITERATOR, dialogs) {
        if (existing_dialog->full_path != NULL && string_equals(existing_dialog->full_path, full_path)) {
            free_chk(full_path);
            return id;
        }
        id++;
    }

    // load and parse txt file
    char* source = fs_readtext(full_path);

    if (string_is_empty(source)) {
        logger_error("dialogue_internal_parse_dialog() can not read: %s", src);
        free_chk(full_path);
        return -1;
    }

    Dialog dialog;
    dialogue_internal_parse_dialog_from_string(source, &dialog);

    // add to the arraylist
    id = arraylist_size(dialogs);
    dialog.full_path = full_path;
    arraylist_add(dialogs, &dialog);

    free_chk(source);
    return id;
}

static void dialogue_internal_parse_dialog_from_string(const char* source, Dialog* dialog_ref) {
    Tokenizer tokenizer = tokenizer_init("\n", false, false, (char*)source);
    assert(tokenizer);

    ArrayList lines = arraylist_init2(sizeof(DialogLine), tokenizer_count_occurrences(tokenizer));

    char* line;
    char* buffered_line = NULL;
    while ((line = tokenizer_read_next(tokenizer)) != NULL) {
        int32_t end_index = -1;
        int32_t line_length = (int32_t)strlen(line);

        if (line_length > 0 && line[line_length - 1] == '\r') {
            char* tmp = string_substring(line, 0, line_length - 1);
            free_chk(line);
            line = tmp;
            line_length--;
        }

        if (line_length > 0 && line[0] == ':') {
            end_index = string_index_of_char(line, 1, ':');
        }

        if (end_index < 0) {
            // buffer the current line
            char* tmp = string_concat(3, buffered_line, "\n", line);
            free_chk(buffered_line);
            buffered_line = tmp;
        } else {
            char* state = string_substring(line, 1, end_index);
            char* tmp_line = string_substring(line, end_index + 1, line_length);
            char* final_line = string_concat(2, buffered_line, tmp_line);

            free_chk(tmp_line);
            free_chk(buffered_line);
            buffered_line = NULL;

            DialogLine dialog_line = {
                .target_state_name = state,
                .text = final_line
            };

            arraylist_add(lines, &dialog_line);
        }

        free_chk(line);
    }

    if (buffered_line != NULL) {
        if (arraylist_size(lines) < 1) {
            DialogLine dialog_line = {
                .target_state_name = NULL,
                .text = buffered_line
            };
            arraylist_add(lines, &dialog_line);
        } else {
            DialogLine* dialog_line = arraylist_get(lines, arraylist_size(lines) - 1);
            char* tmp = string_concat(2, dialog_line->text, buffered_line);
            free_chk(dialog_line->text);
            free_chk(buffered_line);
            dialog_line->text = tmp;
        }
    }

    tokenizer_destroy(&tokenizer);

    dialog_ref->full_path = NULL;
    dialog_ref->lines = NULL;
    dialog_ref->lines_size = 0;

    arraylist_destroy2(&lines, &dialog_ref->lines_size, (void**)&dialog_ref->lines);
}

static void dialogue_internal_read_color(XmlNode node, float* rgba) {
    uint32_t color = 0x00;
    rgba[3] = 1.0f;
    if (vertexprops_parse_hex(xmlparser_get_attribute_value2(node, "rgb"), &color, false)) {
        math2d_color_bytes_to_floats(color, false, rgba);
        rgba[3] = vertexprops_parse_float(node, "alpha", 1.0f);
    } else if (vertexprops_parse_hex(xmlparser_get_attribute_value2(node, "rgba"), &color, false)) {
        math2d_color_bytes_to_floats(color, true, rgba);
    } else {
        rgba[0] = vertexprops_parse_float(node, "r", rgba[0]);
        rgba[1] = vertexprops_parse_float(node, "g", rgba[1]);
        rgba[2] = vertexprops_parse_float(node, "b", rgba[2]);
        rgba[3] = vertexprops_parse_float(node, "a", rgba[3]);
    }
}

static Align dialogue_internal_read_align(XmlNode node, const char* attribute) {
    const char* unparsed_align = xmlparser_get_attribute_value2(node, attribute);
    if (string_is_empty(unparsed_align) || string_equals(unparsed_align, "none")) {
        return ALIGN_NONE;
    }

    Align align = vertexprops_parse_align2(unparsed_align);
    switch (align) {
        case ALIGN_START:
        case ALIGN_CENTER:
        case ALIGN_END:
            break;
        default:
            logger_error("dialogue_internal_read_align() invalid align value: %s", unparsed_align);
            align = ALIGN_NONE;
            break;
    }

    return align;
}

static void dialogue_internal_read_offset(JSONToken json_obj, const char* property, float* offset_x, float* offset_y) {
    JSONToken json_array = json_read_array(json_obj, property);
    *offset_x = (float)json_read_array_item_number_double(json_array, 0, 0.0);
    *offset_y = (float)json_read_array_item_number_double(json_array, 1, 0.0);
}

static void dialogue_internal_compute_state(ModelHolder mdlhldr, StateSprite sttsprt, AnimSprite anim, float offst_x, float offst_y, bool is_speak) {
    if (!anim) return;

    const char* name = is_speak ? DIALOGUE_SPEAK : DIALOGUE_IDLE;
    StateSpriteState* state = statesprite_state_add2(sttsprt, modelholder_get_texture(mdlhldr, true), anim, NULL, 0x00, name);

    assert(state);

    state->offset_x = offst_x;
    state->offset_y = offst_y;
}

static void dialogue_internal_add_state(StateSprite statesprite, ModelHolder modelholder, const char* anim_name, const char* state_name, float scale, bool looped) {
    if (!anim_name) return;

    StateSpriteState* state = statesprite_state_add(statesprite, modelholder, anim_name, state_name);
    if (!state) return;

    if (state->animation && !modelholder_has_animlist(modelholder)) {
        AnimList animlist = modelholder_get_animlist(modelholder);
        if (!animlist_get_animation(animlist, anim_name)) {
            // the animation was builded from an atlas, explicit set the loop count
            animsprite_set_loop(state->animation, looped ? -1 : 1);
        }
    }

    float orig_width = -1.0f, orig_height = -1.0f;

    imgutils_get_statesprite_original_size(state, &orig_width, &orig_height);
    state->draw_width = orig_width * scale;
    state->draw_height = orig_height * scale;
}

static void dialogue_internal_stop_portrait_animation(Portrait* portrait) {
    StateSpriteState* state = statesprite_state_get(portrait->statesprite);
    if (state && state->animation) {
        animsprite_force_end3(state->animation, portrait->statesprite);
        animsprite_stop(state->animation);
    }
}

static Audio* dialogue_internal_get_audio(Dialogue dialogue, const char* name) {
    if (!name && dialogue->audios_size > 0) {
        // random choose
        int32_t index = math2d_random_int(0, dialogue->audios_size);
        return &dialogue->audios[index];
    }

    for (int32_t i = 0; i < dialogue->audios_size; i++) {
        if (string_equals(dialogue->audios[i].name, name)) {
            return &dialogue->audios[i];
        }
    }
    return NULL;
}

static FontHolder dialogue_internal_get_font(Dialogue dialogue, const char* name) {
    if (!name && dialogue->fonts_size > 0) {
        // random choose
        int32_t index = math2d_random_int(0, dialogue->fonts_size);
        return dialogue->fonts[index].fontholder;
    }

    for (int32_t i = 0; i < dialogue->fonts_size; i++) {
        if (string_equals(dialogue->fonts[i].name, name)) {
            return dialogue->fonts[i].fontholder;
        }
    }
    return NULL;
}

static int32_t dialogue_internal_get_background_index(Dialogue dialogue, const char* name) {
    if (!name && dialogue->backgrounds_size > 0) {
        // random choose
        return math2d_random_int(0, dialogue->backgrounds_size);
    }

    for (int32_t i = 0; i < dialogue->backgrounds_size; i++) {
        if (string_equals(dialogue->backgrounds[i].name, name)) {
            return i;
        }
    }
    return -1;
}

static MultipleChoice* dialogue_internal_get_multiplechoice(Dialogue dialogue, const char* name) {
    if (!name && dialogue->multiplechoices_size > 0) {
        // random choose
        int32_t index = math2d_random_int(0, dialogue->multiplechoices_size);
        return &dialogue->multiplechoices[index];
    }

    for (int32_t i = 0; i < dialogue->multiplechoices_size; i++) {
        if (string_equals(dialogue->multiplechoices[i].name, name)) {
            return &dialogue->multiplechoices[i];
        }
    }
    return NULL;
}
