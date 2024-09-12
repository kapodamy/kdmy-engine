#include <ctype.h>

#include "animlist.h"
#include "arraylist.h"
#include "externals/luascript.h"
#include "fs.h"
#include "game/common/funkin.h"
#include "malloc_utils.h"
#include "map.h"
#include "math2d.h"
#include "stringutils.h"
#include "tokenizer.h"
#include "vertexprops.h"
#include "xmlparser.h"


typedef struct {
    int32_t index;
    int32_t length;
} AlternateEntry;

typedef struct {
    char* absolute_path;
    Atlas atlas;
} CachedAtlas;


// const size_t AnimInterpolator_MODIFIER_SINE = 0;
// const size_t AnimInterpolator_MODIFIER_COSINE = 1;
// const size_t AnimInterpolator_MODIFIER_LOG = 2;
// const size_t AnimInterpolator_MODIFIER_EXP = 3;

static Map ANIMLIST_POOL = NULL;
static int32_t ANIMLIST_IDS = 0;


static Atlas animlist_load_required_atlas(XmlNode animlist_item, LinkedList atlas_list, Atlas def_atlas, bool is_macro);
static Atlas animlist_load_atlas(const char* src);
static AnimListItem* animlist_read_frame_animation(XmlNode entry, Atlas atlas, float default_fps);
static void animlist_copy_entries_to_frames_array(LinkedList frame_list, const char* name, bool has_number_suffix, Atlas atlas);
static void animlist_add_entry_from_atlas(LinkedList frame_list, const char* name, Atlas atlas);
static bool animlist_add_alternate_entry(ArrayList array, int32_t frame_count, int32_t index);
static AnimListItem* animlist_read_macro_animation(XmlNode entry, Atlas atlas);
static MacroExecutorValue* animlist_parse_randomexact(XmlNode unparsed_randomexact, int32_t* out_size);
static int32_t animlist_parse_property(XmlNode node, const char* name, bool warn);
static AnimListItem* animlist_read_tweenkeyframe_animation(XmlNode entry);



void __attribute__((constructor)) __ctor_animlist() {
    ANIMLIST_POOL = map_init();
}


AnimList animlist_init(const char* src) {
    char* full_path = fs_get_full_path_and_override(src);

    // find a previous loaded animlist
    foreach (AnimList, obj, MAP_ITERATOR, ANIMLIST_POOL) {
        if (string_equals_ignore_case(obj->src, full_path)) {
            obj->references++;
            free_chk(full_path);
            return obj;
        }
    }

    XmlParser xml = xmlparser_init(src);
    if (!xml) {
        free_chk(full_path);
        logger_error("animlist_init() error loading '%s'", src);
        return NULL;
    }

    fs_folder_stack_push();
    fs_set_working_folder(full_path, true);


    XmlNode anims_list = xmlparser_get_root(xml);
    assert(anims_list && string_equals(xmlparser_get_tag_name(anims_list), "AnimationList"));

    LinkedList atlas_cache = linkedlist_init();
    LinkedList parsed_animations = linkedlist_init();
    float default_fps = vertexprops_parse_float(anims_list, "frameRate", FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE);
    Atlas default_atlas;
    bool is_macro;

    if (xmlparser_has_attribute(anims_list, "atlasPath")) {
        char* default_atlas_path = fs_build_path2(full_path, xmlparser_get_attribute_value2(anims_list, "atlasPath"));
        default_atlas = animlist_load_atlas(default_atlas_path);
        free_chk(default_atlas_path);
    } else {
        default_atlas = NULL;
    }

    foreach (XmlNode, unparsed_anim, XMLPARSER_CHILDREN_ITERATOR, anims_list) {
        const char* tagName = xmlparser_get_tag_name(unparsed_anim);

        if (string_equals(tagName, "Animation")) {
            is_macro = 0;
        } else if (string_equals(tagName, "AnimationMacro")) {
            is_macro = 1;
        } else if (string_equals(tagName, "TweenKeyframe")) {
            linkedlist_add_item(parsed_animations, animlist_read_tweenkeyframe_animation(unparsed_anim));
            continue;
        } else {
            logger_warn("animlist_init() unknown animation: %s", tagName);
            continue;
        }

        Atlas atlas = animlist_load_required_atlas(
            unparsed_anim, atlas_cache, default_atlas, is_macro
        );
        AnimListItem* animlist_item;

        if (is_macro)
            animlist_item = animlist_read_macro_animation(unparsed_anim, atlas);
        else
            animlist_item = animlist_read_frame_animation(unparsed_anim, atlas, default_fps);

        if (animlist_item) linkedlist_add_item(parsed_animations, animlist_item);
    }

    AnimList animlist = malloc_chk(sizeof(struct AnimList_s));
    malloc_assert(animlist, AnimList);

    animlist->entries_count = linkedlist_count(parsed_animations);
    animlist->entries = linkedlist_to_solid_array(parsed_animations, sizeof(AnimListItem));

    // destroy the list and release all allocated items
    linkedlist_destroy2(&parsed_animations, free_chk);

    if (default_atlas) atlas_destroy(&default_atlas);

    // dispose atlas cache
    foreach (CachedAtlas*, entry, LINKEDLIST_ITERATOR, atlas_cache) {
        if (entry->atlas) atlas_destroy(&entry->atlas);
        free_chk(entry->absolute_path);
        free_chk(entry);
    }

    linkedlist_destroy(&atlas_cache);
    xmlparser_destroy(&xml);

    animlist->src = full_path;
    animlist->id = ANIMLIST_IDS++;
    animlist->references = 1;
    map_add(ANIMLIST_POOL, animlist->id, animlist);

    fs_folder_stack_pop();

    return animlist;
}

void animlist_destroy(AnimList* animlist_ptr) {
    AnimList animlist = *animlist_ptr;
    if (!animlist) return;

    animlist->references--;
    if (animlist->references > 0) return;

    for (int32_t i = 0; i < animlist->entries_count; i++) {
        free_chk(animlist->entries[i].name);
        free_chk(animlist->entries[i].frames);
        free_chk(animlist->entries[i].alternate_set);

        for (int32_t j = 0; j < animlist->entries[i].instructions_count; j++) {
            free_chk(animlist->entries[i].instructions[j].values);
        }

        if (animlist->entries[i].is_tweenkeyframe) free_chk(animlist->entries[i].tweenkeyframe_entries);

        free_chk(animlist->entries[i].instructions);

        luascript_drop_shared(&animlist->entries[i]);
    }

    free_chk(animlist->entries);
    free_chk(animlist->src);
    luascript_drop_shared(animlist);
    map_delete(ANIMLIST_POOL, animlist->id);

    free_chk(animlist);
    *animlist_ptr = NULL;
}

const AnimListItem* animlist_get_animation(AnimList animlist, const char* animation_name) {
    for (int32_t i = 0; i < animlist->entries_count; i++)
        if (string_equals(animlist->entries[i].name, animation_name))
            return &animlist->entries[i];

    return NULL;
}

bool animlist_is_item_macro_animation(const AnimListItem* animlist_item) {
    return animlist_item->instructions_count > 0;
}

bool animlist_is_item_frame_animation(const AnimListItem* animlist_item) {
    return !animlist_item->is_tweenkeyframe && animlist_item->instructions_count < 1;
}

bool animlist_is_item_tweenkeyframe_animation(const AnimListItem* animlist_item) {
    return animlist_item->is_tweenkeyframe;
}



static Atlas animlist_load_required_atlas(XmlNode animlist_item, LinkedList atlas_list, Atlas def_atlas, bool is_macro) {
    const char* filename = xmlparser_get_attribute_value2(animlist_item, "atlasPath");

    if (!filename) {
        if (is_macro) return NULL;

        if (!def_atlas) {
            logger_warn_xml(
                "animlist_load_required_atlas() animation without atlas:", animlist_item
            );
        }
        return def_atlas;
    }

    char* absolute_path = fs_get_full_path_and_override(filename);
    CachedAtlas* obj = NULL; // TODO use absolute path instead of filename

    foreach (CachedAtlas*, entry, LINKEDLIST_ITERATOR, atlas_list) {
        if (string_equals(entry->absolute_path, absolute_path)) {
            obj = entry;
            break;
        }
    }

    if (obj) {
        free_chk(absolute_path);
    } else {
        obj = malloc_chk(sizeof(CachedAtlas));
        malloc_assert(obj, CachedAtlas);

        *obj = (CachedAtlas){.absolute_path = absolute_path, .atlas = animlist_load_atlas(absolute_path)};
        linkedlist_add_item(atlas_list, obj);
    }

    if (!obj->atlas) {
        logger_warn_xml(
            "animlist_load_required_atlas() missing atlas %s:", animlist_item, filename
        );
    }

    return obj->atlas;
}

static Atlas animlist_load_atlas(const char* src) {
    return src ? atlas_init(src) : NULL;
}

static AnimListItem* animlist_read_frame_animation(XmlNode entry, Atlas atlas, float default_fps) {
    const char* name = xmlparser_get_attribute_value2(entry, "name");
    if (!name) {
        logger_error_xml("animlist_read_frame_animation() missing animation name:", entry);
        return NULL;
    }
    if (!atlas) {
        logger_error_xml("animlist_read_frame_animation() missing atlas:", entry);
        return NULL;
    }

    AnimListItem* anim = calloc_for_type(AnimListItem);
    malloc_assert(anim, AnimListItem);

    anim->is_tweenkeyframe = 0;
    anim->name = string_duplicate(name);
    anim->loop = vertexprops_parse_integer(entry, "loop", 1);
    anim->frame_rate = vertexprops_parse_float(entry, "frameRate", default_fps);
    anim->alternate_per_loop = vertexprops_parse_boolean(entry, "alternateInLoops", false);
    anim->alternate_no_random = !vertexprops_parse_boolean(entry, "alternateRandomize", false);
    anim->loop_from_index = 0;

    LinkedList parsed_frames = linkedlist_init();
    ArrayList parsed_alternates = arraylist_init(sizeof(AlternateEntry));
    int32_t last_alternate_index = 0;

    foreach (XmlNode, unparsed_frame, XMLPARSER_CHILDREN_ITERATOR, entry) {
        const char* tagName = xmlparser_get_tag_name(unparsed_frame);

        if (string_equals(tagName, "FrameArray")) {
            const char* name_prefix = xmlparser_get_attribute_value2(unparsed_frame, "entryPrefixName");
            const char* name_suffix = xmlparser_get_attribute_value2(unparsed_frame, "entrySuffixName");
            bool has_number_suffix = vertexprops_parse_boolean(unparsed_frame, "hasNumberSuffix", true);
            int32_t index_start = (int32_t)vertexprops_parse_integer(unparsed_frame, "indexStart", 0);
            int32_t index_end = (int32_t)vertexprops_parse_integer(unparsed_frame, "indexEnd", -1);

            const char* const_frame_name = string_is_not_empty(name_prefix) ? name_prefix : name;

            char* frame_name;
            if (string_is_not_empty(name_suffix))
                frame_name = string_concat(3, const_frame_name, " ", name_suffix); // add space before suffix
            else
                frame_name = string_duplicate(const_frame_name);

            animlist_read_entries_to_frames_array(
                parsed_frames, frame_name, has_number_suffix, atlas, index_start, index_end
            );
            free_chk(frame_name);
        } else if (string_equals(tagName, "Frame")) {
            const char* entry_name = xmlparser_get_attribute_value2(unparsed_frame, "entryName");
            if (!entry_name) entry_name = name;

            animlist_add_entry_from_atlas(parsed_frames, entry_name, atlas);
        } else if (string_equals(tagName, "Pause")) {
            float duration = vertexprops_parse_float(unparsed_frame, "duration", 1.0f);
            const AtlasEntry* last_frame = linkedlist_get_last_item(parsed_frames);
            if (last_frame) {
                while (duration-- > 0) linkedlist_add_item(parsed_frames, last_frame);
            }
        } else if (string_equals(tagName, "AlternateSet")) {
            int32_t frame_count = linkedlist_count(parsed_frames);
            if (animlist_add_alternate_entry(parsed_alternates, frame_count, last_alternate_index))
                logger_error("animlist_read_frame_animation() consecutive AlternateSet found (no enough frames in '%s')", name);
            else
                last_alternate_index = frame_count;
        } else if (string_equals(tagName, "LoopMark")) {
            int32_t offset = (int32_t)vertexprops_parse_integer(unparsed_frame, "lastFrames", 0);
            anim->loop_from_index = linkedlist_count(parsed_frames) - offset;
        } else {
            logger_error_xml("animlist_read_frame_animation() unknown frame type '%s':", unparsed_frame, tagName);
        }
    }

    anim->frame_count = linkedlist_count(parsed_frames);
    anim->frames = linkedlist_to_solid_array(parsed_frames, sizeof(AtlasEntry));

    anim->instructions = NULL;
    anim->instructions_count = 0;

    if (arraylist_size(parsed_alternates) > 0) {
        // add the last frames set
        int32_t frames_count = linkedlist_count(parsed_frames);
        animlist_add_alternate_entry(parsed_alternates, frames_count, last_alternate_index);
    }
    arraylist_destroy2(&parsed_alternates, &anim->alternate_set_size, (void**)&anim->alternate_set);

    if (anim->loop_from_index >= anim->frame_count || anim->loop_from_index < 0) {
        anim->loop_from_index = 0;
    }

    linkedlist_destroy(&parsed_frames);

    return anim;
}

static void animlist_copy_entries_to_frames_array(LinkedList frame_list, const char* name, bool has_number_suffix, Atlas atlas) {
    uint32_t start = strlen(name);

    for (int32_t i = 0; i < atlas->size; i++) {
        const char* atlas_entry_name = atlas->entries[i].name;
        if (!string_starts_with(atlas_entry_name, name)) continue;

        if (has_number_suffix && !atlas_name_has_number_suffix(atlas_entry_name, start))
            continue;

        linkedlist_add_item(frame_list, &atlas->entries[i]);
    }
}

void animlist_read_entries_to_frames_array(LinkedList frame_list, const char* name, bool has_number_suffix, Atlas atlas, int32_t start, int32_t end) {
    uint32_t index_name_start = strlen(name);

    if (end < 0) end = atlas->size;

    uint32_t found_index = 0;

    for (int32_t i = 0; i < atlas->size; i++) {
        const char* atlas_entry_name = atlas->entries[i].name;
        if (!string_starts_with(atlas_entry_name, name)) continue;

        if (has_number_suffix && !atlas_name_has_number_suffix(atlas_entry_name, index_name_start))
            continue;

        if (found_index >= start) {
            linkedlist_add_item(frame_list, &atlas->entries[i]);
            if (found_index >= end) break;
        }

        found_index++;
    }
}

static void animlist_add_entry_from_atlas(LinkedList frame_list, const char* name, Atlas atlas) {
    for (int32_t i = 0; i < atlas->size; i++) {
        const AtlasEntry* entry = &atlas->entries[i];

        if (string_equals(entry->name, name)) {
            linkedlist_add_item(frame_list, entry);
            return;
        }
    }

    logger_error("animlist_add_entry_from_atlas() Missing atlas entry: %s", name);
}


static AnimInterpolator animlist_parse_interpolator(XmlNode node, const char* name, AnimInterpolator default_interpolator) {
    const char* type = xmlparser_get_attribute_value2(node, name);

    if (!type)
        return default_interpolator;

    if (string_equals_ignore_case(type, "ease"))
        return AnimInterpolator_EASE;
    if (string_equals_ignore_case(type, "ease-in"))
        return AnimInterpolator_EASE_IN;
    if (string_equals_ignore_case(type, "ease-out"))
        return AnimInterpolator_EASE_OUT;
    if (string_equals_ignore_case(type, "ease-in-out"))
        return AnimInterpolator_EASE_IN_OUT;
    if (string_equals_ignore_case(type, "linear"))
        return AnimInterpolator_LINEAR;
    if (string_equals_ignore_case(type, "steps"))
        return AnimInterpolator_STEPS;
    if (string_equals_ignore_case(type, "cubic"))
        return AnimInterpolator_CUBIC;
    if (string_equals_ignore_case(type, "quad"))
        return AnimInterpolator_QUAD;
    if (string_equals_ignore_case(type, "expo"))
        return AnimInterpolator_EXPO;
    if (string_equals_ignore_case(type, "sin"))
        return AnimInterpolator_SIN;

    logger_warn("animlist_parse_interpolator() unknown interpolator type %s", type);
    return default_interpolator;
}

static int32_t animlist_parse_register(XmlNode node) {
    const char* value = xmlparser_get_attribute_value2(node, "register");
    if (value == NULL) {
        return -1;
    }

    size_t value_length = strlen(value);
    if (value_length < 4) goto L_return;

    if (tolower(value[0]) == 'r' && tolower(value[1]) == 'e' && tolower(value[2]) == 'g' && isdigit((uint8_t)value[3])) {
        uint8_t index = value[3] - 0x30;
        if (index < MACROEXECUTOR_REGISTER_COUNT) {
            return index;
        }
    }

L_return:
    logger_warn("animlist_parse_register() invalid register '%s'", value);
    return -1;
}

static void animlist_parse_complex_value(const char* unparsed_value, float def_value, MacroExecutorValue* value) {
    if (unparsed_value == NULL || unparsed_value[0] == '\0') {
        // returnd default value (as literal)
        value->kind = MacroExecutorValueKind_LITERAL;
        value->literal = def_value;
        return;
    }

    bool success;

    // special values used in macroexecutor
    value->kind = MacroExecutorValueKind_SPECIAL;
    success = true;

    if (string_equals_ignore_case(unparsed_value, "rnd"))
        value->reference = ANIM_MACRO_SPECIAL_RANDOM;
    else if (string_equals_ignore_case(unparsed_value, "reg0"))
        value->reference = ANIM_MACRO_SPECIAL_REGISTER0;
    else if (string_equals_ignore_case(unparsed_value, "reg1"))
        value->reference = ANIM_MACRO_SPECIAL_REGISTER1;
    else if (string_equals_ignore_case(unparsed_value, "reg2"))
        value->reference = ANIM_MACRO_SPECIAL_REGISTER2;
    else if (string_equals_ignore_case(unparsed_value, "reg3"))
        value->reference = ANIM_MACRO_SPECIAL_REGISTER3;
    else
        success = false;

    if (success) return;


    // boolean and numbers are literals
    value->kind = MacroExecutorValueKind_LITERAL;
    success = true;

    // check if is a boolean value
    if (string_equals_ignore_case(unparsed_value, "true"))
        value->literal = 1;
    else if (string_equals_ignore_case(unparsed_value, "false"))
        value->literal = 0;
    else
        success = false;

    if (success) return;


    uint32_t as_unsigned = 0x00;
    if (vertexprops_parse_hex(unparsed_value, &as_unsigned, true)) {
        value->literal = (float)as_unsigned;
        return;
    }

    // check if the value is just a number
    float as_number = vertexprops_parse_float2(unparsed_value, FLOAT_NaN);
    if (!math2d_is_float_NaN(as_number)) {
        value->literal = as_number;
        return;
    }

    // check if the value is a property name
    int32_t as_property = vertexprops_parse_textsprite_property2(unparsed_value);
    if (as_property < 0) as_property = vertexprops_parse_sprite_property2(unparsed_value);
    if (as_property < 0) as_property = vertexprops_parse_media_property2(unparsed_value);
    if (as_property < 0) as_property = vertexprops_parse_layout_property2(unparsed_value);

    if (as_property >= 0) {
        value->kind = MacroExecutorValueKind_PROPERTY;
        value->reference = as_property;
        return;
    }

    // check if the value is a property value enum
    int32_t as_enum = (int32_t)vertexprops_parse_flag2(unparsed_value, -1);
    if (as_enum < 0) as_enum = (int32_t)vertexprops_parse_textsprite_forcecase2(unparsed_value);
    if (as_enum < 0) as_enum = (int32_t)vertexprops_parse_wordbreak2(unparsed_value);
    if (as_enum < 0) as_enum = (int32_t)vertexprops_parse_playback2(unparsed_value);
    if (as_enum < 0) as_enum = (int32_t)vertexprops_parse_align2(unparsed_value);
    if (as_enum >= 0) {
        value->kind = MacroExecutorValueKind_LITERAL;
        value->literal = as_enum;
        return;
    }

    // no matches, return the default value
    value->kind = MacroExecutorValueKind_LITERAL;
    value->literal = def_value;
}

static void animlist_parse_complex_value2(XmlNode node, const char* name, float def_value, MacroExecutorValue* value) {
    animlist_parse_complex_value(xmlparser_get_attribute_value2(node, name), def_value, value);
}





static bool animlist_add_alternate_entry(ArrayList array, int32_t frame_count, int32_t index) {
    int32_t length = frame_count - index;
    if (length < 1) return true;
    arraylist_add(array, &(AlternateEntry){.index = index, .length = length});
    return false;
}


static AnimListItem* animlist_read_macro_animation(XmlNode entry, Atlas atlas) {

    AnimListItem* anim = calloc_for_type(AnimListItem);
    malloc_assert(anim, AnimListItem);

    anim->is_tweenkeyframe = false;
    anim->name = string_duplicate(xmlparser_get_attribute_value2(entry, "name"));
    anim->loop = (int32_t)vertexprops_parse_integer(entry, "loop", 1);
    anim->frames = NULL;
    anim->frames_count = 0;
    anim->frame_restart_index = (int32_t)vertexprops_parse_integer(entry, "frameRestartIndex", -1);
    anim->frame_allow_size_change = vertexprops_parse_boolean(entry, "frameAllowChangeSize", false);

    anim->alternate_set = NULL;
    anim->alternate_set_size = 0;

    const char* atlasPrefixEntryName = xmlparser_get_attribute_value2(entry, "atlasPrefixEntryName");
    bool atlasHasNumberSuffix = vertexprops_parse_boolean(entry, "atlasHasNumberSuffix", true);

    int32_t unparsed_ins_count = xmlparser_get_children_count(entry);
    ArrayList parsed_instructions = arraylist_init2(sizeof(MacroExecutorInstruction), unparsed_ins_count);

    MacroExecutorInstruction instruction;
    int32_t property_id;

    foreach (XmlNode, unparsed_ins, XMLPARSER_CHILDREN_ITERATOR, entry) {
        const char* tagName = xmlparser_get_tag_name(unparsed_ins);

        if (string_equals(tagName, "Interpolator")) {
            property_id = animlist_parse_property(unparsed_ins, "property", true);
            if (property_id < 0) continue;

            instruction = (MacroExecutorInstruction){
                .type = AnimMacroType_INTERPOLATOR,
                .interpolator = animlist_parse_interpolator(unparsed_ins, "type", AnimInterpolator_LINEAR),
                .property = property_id,
                .start = {.reference = -1, .literal = FLOAT_NaN, .kind = MacroExecutorValueKind_LITERAL},
                .end = {.reference = -1, .literal = FLOAT_NaN, .kind = MacroExecutorValueKind_LITERAL},
                .duration = {.reference = -1, .literal = FLOAT_NaN, .kind = MacroExecutorValueKind_LITERAL},
                .steps_count = {.reference = -1, .literal = FLOAT_NaN, .kind = MacroExecutorValueKind_LITERAL},
                .steps_method = {.reference = -1, .literal = FLOAT_NaN, .kind = MacroExecutorValueKind_LITERAL}
            };

            animlist_parse_complex_value2(unparsed_ins, "start", FLOAT_NaN, &instruction.start);
            animlist_parse_complex_value2(unparsed_ins, "end", FLOAT_NaN, &instruction.end);
            animlist_parse_complex_value2(unparsed_ins, "duration", 0.0f, &instruction.duration);
            animlist_parse_complex_value2(unparsed_ins, "steps_count", 1.0f, &instruction.steps_count);
            animlist_parse_complex_value2(unparsed_ins, "steps_method", 0.0f, &instruction.steps_method);

            arraylist_add(parsed_instructions, &instruction);
        } else if (string_equals(tagName, "Set")) {
            property_id = animlist_parse_property(unparsed_ins, "property", true);
            if (property_id < 0) continue;

            instruction = (MacroExecutorInstruction){
                .type = AnimMacroType_SET,
                .property = property_id,
                .value = {.reference = -1, .literal = FLOAT_NaN, .kind = MacroExecutorValueKind_LITERAL}
            };

            animlist_parse_complex_value2(unparsed_ins, "value", 0.0f, &instruction.value);

            arraylist_add(parsed_instructions, &instruction);
        } else if (string_equals(tagName, "Yield")) {
            instruction = (MacroExecutorInstruction){
                .type = AnimMacroType_YIELD,
                .value = {.reference = -1, .literal = FLOAT_NaN, .kind = MacroExecutorValueKind_LITERAL}
            };

            animlist_parse_complex_value2(unparsed_ins, "duration", 0.0f, &instruction.value);

            arraylist_add(parsed_instructions, &instruction);
        } else if (string_equals(tagName, "Pause")) {
            instruction = (MacroExecutorInstruction){
                .type = AnimMacroType_PAUSE,
                .value = {.reference = -1, .literal = FLOAT_NaN, .kind = MacroExecutorValueKind_LITERAL}
            };

            animlist_parse_complex_value2(unparsed_ins, "duration", 1.0f, &instruction.value);

            arraylist_add(parsed_instructions, &instruction);
        } else if (string_equals(tagName, "Reset")) {
            instruction = (MacroExecutorInstruction){
                .type = AnimMacroType_RESET
            };
            arraylist_add(parsed_instructions, &instruction);
        } else if (string_equals(tagName, "RandomSetup")) {
            instruction = (MacroExecutorInstruction){
                .type = AnimMacroType_RANDOM_SETUP,
                .start = {.reference = -1, .literal = FLOAT_NaN, .kind = MacroExecutorValueKind_LITERAL},
                .end = {.reference = -1, .literal = FLOAT_NaN, .kind = MacroExecutorValueKind_LITERAL}
            };

            animlist_parse_complex_value2(unparsed_ins, "start", 0.0f, &instruction.start);
            animlist_parse_complex_value2(unparsed_ins, "end", 1.0f, &instruction.end);

            arraylist_add(parsed_instructions, &instruction);
        } else if (string_equals(tagName, "RandomChoose")) {
            instruction = (MacroExecutorInstruction){
                .type = AnimMacroType_RANDOM_CHOOSE
            };
            arraylist_add(parsed_instructions, &instruction);
        } else if (string_equals(tagName, "RandomExact")) {
            int32_t values_size = 0;
            MacroExecutorValue* values_array = animlist_parse_randomexact(
                unparsed_ins, &values_size
            );
            if (values_array == NULL) continue;

            instruction = (MacroExecutorInstruction){
                .type = AnimMacroType_RANDOM_EXACT,
                .values = values_array,
                .values_size = values_size
            };
            arraylist_add(parsed_instructions, &instruction);
        } else if (string_equals(tagName, "RegisterProp")) {
            property_id = animlist_parse_property(unparsed_ins, "property", true);
            if (property_id < 0) continue;

            instruction = (MacroExecutorInstruction){
                .type = AnimMacroType_REGISTER_PROP,
                .register_index = animlist_parse_register(unparsed_ins),
                .property = property_id
            };
            arraylist_add(parsed_instructions, &instruction);
        } else if (string_equals(tagName, "RegisterSet")) {
            instruction = (MacroExecutorInstruction){
                .type = AnimMacroType_REGISTER_SET,
                .register_index = animlist_parse_register(unparsed_ins),
                .value = {.reference = -1, .literal = FLOAT_NaN, .kind = MacroExecutorValueKind_LITERAL}
            };

            animlist_parse_complex_value2(unparsed_ins, "value", 0.0, &instruction.value);

            arraylist_add(parsed_instructions, &instruction);
        } else {
            logger_warn_xml(
                "animlist_read_macro_animation() unknown instruction %s:",
                unparsed_ins, tagName
            );
        }
    }

    // note: keep "instruction[].values" allocated
    arraylist_destroy2(&parsed_instructions, &anim->instructions_count, (void**)&anim->instructions);

    if (atlasPrefixEntryName) {
        LinkedList parsed_frames = linkedlist_init();

        animlist_copy_entries_to_frames_array(
            parsed_frames, atlasPrefixEntryName, atlasHasNumberSuffix, atlas
        );
        anim->frame_count = linkedlist_count(parsed_frames);
        anim->frames = linkedlist_to_solid_array(parsed_frames, sizeof(AtlasEntry));

        linkedlist_destroy(&parsed_frames);
    } else {
        anim->frame_count = 0;
        anim->frames = NULL;
    }

    return anim;
}

static MacroExecutorValue* animlist_parse_randomexact(XmlNode unparsed_randomexact, int32_t* out_size) {
    const char* unparsed_values = xmlparser_get_attribute_value2(unparsed_randomexact, "values");

    Tokenizer tokenizer = tokenizer_init("\x20", true, false, (char*)unparsed_values);
    if (!tokenizer) {
        logger_warn_xml("animlist_parse_randomexact() missing attribute values in RandomExact:", unparsed_randomexact);

        *out_size = -1;
        return NULL;
    }

    ArrayList parsed_values = arraylist_init(sizeof(MacroExecutorValue));
    char* str;

    while ((str = tokenizer_read_next(tokenizer)) != NULL) {
        MacroExecutorValue parsed_value = {.reference = -1, .literal = FLOAT_NaN, .kind = MacroExecutorValueKind_LITERAL};
        animlist_parse_complex_value(str, FLOAT_NaN, &parsed_value);

        if (parsed_value.reference == TEXTSPRITE_PROP_STRING && parsed_value.kind == MacroExecutorValueKind_PROPERTY) {
            logger_error_xml(
                "animlist_read_macro_animation() illegal property used: string.", unparsed_randomexact
            );

            free_chk(str);
            continue;
        }

        if (math2d_is_float_NaN(parsed_value.literal) && parsed_value.kind == MacroExecutorValueKind_LITERAL) {
            logger_error_xml(
                "animlist_read_macro_animation() invalid or unreconized value found:", unparsed_randomexact
            );

            free_chk(str);
            continue;
        }

        arraylist_add(parsed_values, &parsed_value);
        free_chk(str);
    }

    tokenizer_destroy(&tokenizer);

    MacroExecutorValue* values_array;
    arraylist_destroy2(&parsed_values, out_size, (void**)&values_array);

    return values_array;
}

static int32_t animlist_parse_property(XmlNode node, const char* name, bool warn) {
    int32_t value = vertexprops_parse_textsprite_property(node, name, false);
    if (value < 0) value = vertexprops_parse_sprite_property(node, name, false);
    if (value < 0) value = vertexprops_parse_media_property(node, name, false);
    if (value < 0) value = vertexprops_parse_layout_property(node, name, false);
    if (value < 0) value = vertexprops_parse_camera_property(node, name, false);

    if (value == TEXTSPRITE_PROP_STRING) {
        logger_error_xml("animlist_parse_property() illegal property: string.", node);
        return -1;
    }

    if (value < 0 && warn) {
        logger_warn_xml("animlist_parse_property() unknown property:", node);
    }

    return value;
}

static AnimListItem* animlist_read_tweenkeyframe_animation(XmlNode entry) {
    ArrayList arraylist = arraylist_init(sizeof(AnimListTweenKeyframeEntry));

    float reference_duration = 1.0f;
    if (xmlparser_has_attribute(entry, "referenceDuration")) {
        reference_duration = vertexprops_parse_float(entry, "referenceDuration", FLOAT_NaN);
        if (math2d_is_float_NaN(reference_duration)) {
            logger_warn_xml("animlist_read_tweenkeyframe_animation() invalid tweenkeyframe 'referenceDuration' value:", entry);
            reference_duration = 1.0f;
        }
    }

    AnimInterpolator default_interpolator = AnimInterpolator_LINEAR;
    if (xmlparser_has_attribute(entry, "defaultInterpolator")) {
        default_interpolator = animlist_parse_interpolator(entry, "defaultInterpolator", default_interpolator);
    }

    foreach (XmlNode, node, XMLPARSER_CHILDREN_ITERATOR, entry) {
        // <Keyframe at="80%" id="alpha" interpolator="steps" stepsMethod="both" stepsCount="34" value="1.0" />
        // <Keyframe at="1000" id="translateX" interpolator="ease" value="123" />

        if (!string_equals(xmlparser_get_tag_name(node), "Keyframe")) {
            continue;
        }

        const char* unparsed_at = xmlparser_get_attribute_value2(node, "at");
        if (!unparsed_at) {
            logger_warn_xml("animlist_read_tweenkeyframe_animation() missing Keyframe 'at' attribute:", node);
            continue;
        }

        float at = FLOAT_NaN;
        size_t unparsed_at_length = strlen(unparsed_at);

        if (string_index_of_char(unparsed_at, 0, '%') >= 0) {
            if (reference_duration > 1.0f) {
                logger_warn_xml("animlist_read_tweenkeyframe_animation() invalid Keyframe , 'at' is a percent value and TweenKeyframe have 'referenceDuration' attribute:", node);
                continue;
            }

            if (unparsed_at_length > 1) {
                char* str = string_substring(unparsed_at, 0, (int32_t)unparsed_at_length - 1);
                at = vertexprops_parse_float2(str, FLOAT_NaN);
                free_chk(str);
            }
        } else {
            if (reference_duration < 1.0f) {
                logger_warn_xml("animlist_read_tweenkeyframe_animation() invalid Keyframe , 'at' is a timestamp value and TweenKeyframe does not have 'referenceDuration' attribute:", node);
                continue;
            }
            at = vertexprops_parse_float2(unparsed_at, FLOAT_NaN);
        }

        if (math2d_is_float_NaN(at)) {
            logger_warn_xml("animlist_read_tweenkeyframe_animation() invalid 'at' value:", node);
            continue;
        }

        if (reference_duration > 1.0f)
            at /= reference_duration;
        else
            at /= 100.0f;


        int32_t id = animlist_parse_property(node, "id", false);

        AnimInterpolator keyframe_interpolator = animlist_parse_interpolator(node, "type", default_interpolator);

        int32_t steps_count = (int32_t)vertexprops_parse_integer(node, "stepsCount", -1);
        if (keyframe_interpolator == AnimInterpolator_STEPS && steps_count < 0) {
            logger_warn_xml("animlist_read_tweenkeyframe_animation() invalid o missing 'stepsCount' value:", node);
            continue;
        }

        Align steps_dir = vertexprops_parse_align2(xmlparser_get_attribute_value2(node, "stepsMethod"));
        if (keyframe_interpolator == AnimInterpolator_STEPS && (steps_dir == ALIGN_CENTER || (int)steps_dir < 0)) {
            logger_warn_xml("animlist_read_tweenkeyframe_animation() invalid o missing 'stepsMethod' value:", node);
            continue;
        }

        float value = vertexprops_parse_float(node, "value", FLOAT_NaN);
        if (math2d_is_float_NaN(value)) {
            logger_warn_xml("animlist_read_tweenkeyframe_animation() invalid 'value' value:", node);
            continue;
        }

        AnimListTweenKeyframeEntry keyframe = (AnimListTweenKeyframeEntry){
            .at = at, .id = id, .value = value, .interpolator = keyframe_interpolator, .steps_dir = steps_dir, .steps_count = steps_count
        };

        arraylist_add(arraylist, &keyframe);
    }

    AnimListItem* item = calloc_for_type(AnimListItem);
    malloc_assert(item, AnimListItem);

    *item = (AnimListItem){
        .name = string_duplicate(xmlparser_get_attribute_value2(entry, "name")),
        .is_tweenkeyframe = 1,
        .tweenkeyframe_entries = NULL,
        .tweenkeyframe_entries_count = 0
    };

    arraylist_destroy2(&arraylist, &item->tweenkeyframe_entries_count, (void**)&item->tweenkeyframe_entries);

    return item;
}
