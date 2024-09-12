#include "game/helpers/weekselector_mdlselect.h"

#include <kos/thread.h>

#include "animlist.h"
#include "animsprite.h"
#include "beatwatcher.h"
#include "drawable.h"
#include "fs.h"
#include "game/common/funkin.h"
#include "game/common/funkinsave.h"
#include "game/gameplay/helpers/charactermanifest.h"
#include "game/main.h"
#include "game/weekselector.h"
#include "imgutils.h"
#include "jsonparser.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "modelholder.h"
#include "sh4matrix_types.h"
#include "statesprite.h"
#include "stringutils.h"
#include "textsprite.h"
#include "texture.h"


typedef struct {
    char* name;
    bool is_locked;
    bool imported;
    bool week_selector_left_facing;
    bool week_selector_enable_beat;
    char* week_selector_model;
    char* week_selector_idle_anim_name;
    char* week_selector_choosen_anim_name;
    char* manifest;
} CharacterInfo;

struct WeekSelectorMdlSelect_s {
    AnimSprite animsprite;
    int32_t index;
    CharacterInfo* list;
    int32_t list_size;
    StateSprite preview;
    Sprite icon_locked;
    Sprite icon_up;
    Sprite icon_down;
    bool has_up;
    bool has_down;
    bool is_locked;
    bool is_enabled;
    bool is_boyfriend;
    bool hey_playing;
    volatile int32_t load_character_id;
    volatile int32_t running_threads;
    Drawable drawable;
    Drawable drawable_character;
    LayoutPlaceholder* placeholder_character;
    TextSprite label;
    BeatWatcher beatwatcher;
};


const char* WEEKSELECTOR_MDLSELECT_HEY = "choosen";
const char* WEEKSELECTOR_MDLSELECT_IDLE = "idle";

static const char* WEEKSELECTOR_MDLSELECT_UI_BF_LABEL = "ui_character_selector_player";
static const char* WEEKSELECTOR_MDLSELECT_UI_GF_LABEL = "ui_character_selector_girlfriend";
static const char* WEEKSELECTOR_MDLSELECT_UI_BF_PREVIEW = "ui_character_player";
static const char* WEEKSELECTOR_MDLSELECT_UI_GF_PREVIEW = "ui_character_girlfriend";

const char* WEEKSELECTOR_MDLSELECT_MODELS_BF = "/assets/common/data/boyfriend_models.json";
const char* WEEKSELECTOR_MDLSELECT_MODELS_GF = "/assets/common/data/girlfriend_models.json";


static void weekselector_mdlselect_internal_place_arrow(Sprite sprite, float arrow_size, LayoutPlaceholder* placeholder, bool end);
static void* weekselector_mdlselect_internal_load_async(WeekSelectorMdlSelect mdlselect);
void weekselector_mdlselect_helper_import(StateSprite statesprite, ModelHolder mdlhldr, LayoutPlaceholder* placeholder, bool enable_beat, const char* anim_name, const char* name);
static void weekselector_mdlselect_helper_import_from_manifest(const char* src, CharacterInfo* list_item);


WeekSelectorMdlSelect weekselector_mdlselect_init(AnimList animlist, ModelHolder modelholder, Layout layout, bool is_boyfriend) {
    float draw_width = 0.0f, draw_height = 0.0f;

    WeekSelectorMdlSelect mdlselect = malloc_chk(sizeof(struct WeekSelectorMdlSelect_s));
    malloc_assert(mdlselect, WeekSelectorMdlSelect);

    const char* name;
    *mdlselect = (struct WeekSelectorMdlSelect_s){
        .animsprite = animsprite_init_from_animlist(animlist, WEEKSELECTOR_ARROW_SPRITE_NAME),

        .index = -1,
        .list = NULL,
        .list_size = 0,

        .preview = statesprite_init_from_texture(NULL),

        .icon_locked = modelholder_create_sprite(modelholder, WEEKSELECTOR_LOCKED, true),
        .icon_up = modelholder_create_sprite(modelholder, WEEKSELECTOR_ARROW_SPRITE_NAME, true),
        .icon_down = modelholder_create_sprite(modelholder, WEEKSELECTOR_ARROW_SPRITE_NAME, true),

        .has_up = true,
        .has_down = true,
        .is_locked = false,
        .is_enabled = false,
        .is_boyfriend = is_boyfriend,
        .hey_playing = false,

        .load_character_id = 0,
        .running_threads = 0,

        .drawable = NULL,
        .drawable_character = NULL,
        .placeholder_character = NULL,
        .label = NULL,

        //.beatwatcher = (BeatWatcher){}
    };

    beatwatcher_reset(&mdlselect->beatwatcher, true, 100.0f);

    mdlselect->drawable = drawable_init(
        -1.0f, mdlselect, (DelegateDraw)weekselector_mdlselect_draw, (DelegateAnimate)weekselector_mdlselect_animate
    );
    mdlselect->drawable_character = drawable_init(
        -1.0f, mdlselect->preview, (DelegateDraw)statesprite_draw, (DelegateAnimate)statesprite_animate
    );

    sprite_set_visible(mdlselect->icon_locked, false);
    sprite_set_visible(mdlselect->icon_up, false);
    sprite_set_visible(mdlselect->icon_down, false);

    name = is_boyfriend ? WEEKSELECTOR_MDLSELECT_UI_BF_PREVIEW : WEEKSELECTOR_MDLSELECT_UI_GF_PREVIEW;
    LayoutPlaceholder* placeholder_character = layout_get_placeholder(layout, name);
    assert(placeholder_character);

    int32_t index = 0;

    const char* models = is_boyfriend ? WEEKSELECTOR_MDLSELECT_MODELS_BF : WEEKSELECTOR_MDLSELECT_MODELS_GF;
    JSONToken models_array = json_load_from(models);
    int32_t models_array_size = json_read_array_length(models_array);

    // count all character models
    for (int32_t i = 0; i < models_array_size; i++) {
        JSONToken json_obj = json_read_array_item_object(models_array, i);
        const char* unlock_directive = json_read_string(json_obj, "unlockDirectiveName", NULL);
        bool hide_if_locked = json_read_boolean(json_obj, "hideIfLocked", false);
        bool is_locked = !funkinsave_contains_unlock_directive(unlock_directive);

        if (is_locked && hide_if_locked) continue;
        index++;
    }
    for (int32_t i = 0; i < weeks_array.size; i++) {
        WeekInfoUnlockableCharacter* array;
        int32_t array_size;

        if (is_boyfriend) {
            array = weeks_array.array[i].unlockables.boyfriend_models;
            array_size = weeks_array.array[i].unlockables.boyfriend_models_size;
        } else {
            array = weeks_array.array[i].unlockables.girlfriend_models;
            array_size = weeks_array.array[i].unlockables.girlfriend_models_size;
        }

        for (int32_t j = 0; j < array_size; j++) {
            bool is_locked = !funkinsave_contains_unlock_directive(array[j].unlock_directive);
            if (is_locked && array[j].hide_if_locked) continue;
            index++;
        }
    }

    // build list
    mdlselect->list_size = index;
    mdlselect->list = malloc_for_array(CharacterInfo, mdlselect->list_size);

    index = 0;
    for (int32_t i = 0; i < models_array_size; i++) {
        JSONToken json_obj = json_read_array_item_object(models_array, i);
        const char* unlock_directive = json_read_string(json_obj, "unlockDirectiveName", NULL);
        bool hide_if_locked = json_read_boolean(json_obj, "hideIfLocked", false);
        bool is_locked = !funkinsave_contains_unlock_directive(unlock_directive);

        if (is_locked && hide_if_locked) {
            continue;
        }

        // get path of the manifest file
        const char* manifest = json_read_string(json_obj, "manifest", NULL);
        if (!manifest) {
            logger_error("weekselector_mdlselect_init() invalid/missing 'manifest' in %s", models);
            assert(manifest);
        }
        char* manifest_path = fs_build_path2(models, manifest);

        mdlselect->list[index] = (CharacterInfo){
            .name = json_read_string2(json_obj, "name", NULL),
            .is_locked = is_locked,
            .imported = true,
            .week_selector_left_facing = false,
            .week_selector_enable_beat = false,
            .week_selector_model = NULL,
            .week_selector_idle_anim_name = NULL,
            .week_selector_choosen_anim_name = NULL,
            .manifest = manifest_path
        };
        weekselector_mdlselect_helper_import_from_manifest(
            manifest_path, &mdlselect->list[index]
        );
        index++;
    }
    json_destroy(&models_array);
    for (int32_t i = 0; i < weeks_array.size; i++) {
        WeekInfoUnlockableCharacter* array;
        int32_t array_size;

        if (is_boyfriend) {
            array = weeks_array.array[i].unlockables.boyfriend_models;
            array_size = weeks_array.array[i].unlockables.boyfriend_models_size;
        } else {
            array = weeks_array.array[i].unlockables.girlfriend_models;
            array_size = weeks_array.array[i].unlockables.girlfriend_models_size;
        }

        for (int32_t j = 0; j < array_size; j++) {
            bool is_locked = !funkinsave_contains_unlock_directive(array[j].unlock_directive);
            if (is_locked && array[j].hide_if_locked) continue;

            mdlselect->list[index] = (CharacterInfo){
                .name = (char*)array[j].name,
                .is_locked = is_locked,
                .imported = false,
                .week_selector_left_facing = false,
                .week_selector_enable_beat = false,
                .week_selector_model = NULL,
                .week_selector_idle_anim_name = NULL,
                .week_selector_choosen_anim_name = NULL,
                .manifest = (char*)array[j].manifest
            };
            weekselector_mdlselect_helper_import_from_manifest(
                array[j].manifest, &mdlselect->list[index]
            );
            index++;
        }
    }

    float arrows_height = (float)layout_get_attached_value_double(
        layout, "ui_character_selector_arrowsHeight", 0.0
    );
    float font_size = (float)layout_get_attached_value_double(
        layout, "ui_character_selector_fontSize", 18.0
    );
    const char* font_name = layout_get_attached_value_string(
        layout, "ui_character_selector_fontName", NULL
    );
    uint32_t font_color = layout_get_attached_value_hex(
        layout, "ui_character_selector_fontColor", 0x00FFFF
    );

    name = is_boyfriend ? WEEKSELECTOR_MDLSELECT_UI_BF_LABEL : WEEKSELECTOR_MDLSELECT_UI_GF_LABEL;
    LayoutPlaceholder* placeholder = layout_get_placeholder(layout, name);
    assert(placeholder);

    // resize&rotate&locate arrows
    weekselector_mdlselect_internal_place_arrow(mdlselect->icon_up, arrows_height, placeholder, false);
    weekselector_mdlselect_internal_place_arrow(mdlselect->icon_down, arrows_height, placeholder, true);

    sprite_resize_draw_size(
        mdlselect->icon_locked,
        placeholder->width, placeholder->height - (arrows_height * 1.5f), &draw_width, &draw_height
    );
    sprite_center_draw_location(
        mdlselect->icon_locked,
        placeholder->x, placeholder->y,
        placeholder->width, placeholder->height,
        NULL,
        NULL
    );

    mdlselect->label = textsprite_init2(
        layout_get_attached_font(layout, font_name), font_size, font_color
    );
    textsprite_set_draw_location(mdlselect->label, placeholder->x, placeholder->y);
    textsprite_set_max_draw_size(mdlselect->label, placeholder->width, placeholder->height);
    textsprite_set_align(mdlselect->label, ALIGN_CENTER, ALIGN_CENTER);
    textsprite_set_visible(mdlselect->label, false);

    mdlselect->placeholder_character = placeholder_character;
    placeholder_character->vertex = mdlselect->drawable_character;
    placeholder->vertex = mdlselect->drawable;
    drawable_helper_update_from_placeholder(
        mdlselect->drawable_character, placeholder_character
    );
    drawable_helper_update_from_placeholder(mdlselect->drawable, placeholder);

    drawable_set_visible(mdlselect->drawable_character, false);
    statesprite_set_draw_location(mdlselect->preview, placeholder_character->x, placeholder_character->y);
    statesprite_set_draw_size(
        mdlselect->preview, placeholder_character->width, placeholder_character->height
    );
    statesprite_state_add2(mdlselect->preview, NULL, NULL, NULL, 0XFFFFFF, NULL);

    weekselector_mdlselect_select(mdlselect, 0);

    return mdlselect;
}

void weekselector_mdlselect_destroy(WeekSelectorMdlSelect* mdlselect_ptr) {
    if (!mdlselect_ptr || !*mdlselect_ptr) return;

    WeekSelectorMdlSelect mdlselect = *mdlselect_ptr;

    mdlselect->load_character_id++;
    while (mdlselect->running_threads > 0) {
        // wait until all async operations are done
        thd_pass();
    }

    if (mdlselect->animsprite) animsprite_destroy(&mdlselect->animsprite);

    drawable_destroy(&mdlselect->drawable);
    drawable_destroy(&mdlselect->drawable_character);

    statesprite_destroy(&mdlselect->preview);
    textsprite_destroy(&mdlselect->label);

    sprite_destroy_full(&mdlselect->icon_locked);
    sprite_destroy_full(&mdlselect->icon_up);
    sprite_destroy_full(&mdlselect->icon_down);

    for (int32_t i = 0; i < mdlselect->list_size; i++) {
        if (mdlselect->list[i].imported) {
            free_chk(mdlselect->list[i].name);
            free_chk(mdlselect->list[i].manifest);
        }
        free_chk(mdlselect->list[i].week_selector_model);
        free_chk(mdlselect->list[i].week_selector_idle_anim_name);
        free_chk(mdlselect->list[i].week_selector_choosen_anim_name);
    }

    free_chk(mdlselect->list);

    free_chk(mdlselect);
    *mdlselect_ptr = NULL;
}


void weekselector_mdlselect_draw(WeekSelectorMdlSelect mdlselect, PVRContext pvrctx) {
    pvr_context_save(pvrctx);

    if (mdlselect->is_locked) sprite_draw(mdlselect->icon_locked, pvrctx);

    if (mdlselect->is_enabled) {
        sprite_draw(mdlselect->icon_up, pvrctx);
        sprite_draw(mdlselect->icon_down, pvrctx);
    }

    if (mdlselect->is_locked)
        pvr_context_set_global_alpha(pvrctx, WEEKSELECTOR_PREVIEW_DISABLED_ALPHA);

    textsprite_draw(mdlselect->label, pvrctx);

    pvr_context_restore(pvrctx);
}

int32_t weekselector_mdlselect_animate(WeekSelectorMdlSelect mdlselect, float elapsed) {
    if (mdlselect->animsprite) animsprite_animate(mdlselect->animsprite, elapsed);

    sprite_animate(mdlselect->icon_up, elapsed);
    sprite_animate(mdlselect->icon_down, elapsed);

    if (!mdlselect->hey_playing && beatwatcher_poll(&mdlselect->beatwatcher)) {
        elapsed = mdlselect->beatwatcher.since;
        if (statesprite_animation_completed(mdlselect->preview)) {
            statesprite_animation_restart(mdlselect->preview);
        }
    }

    statesprite_animate(mdlselect->preview, elapsed);

    if (mdlselect->has_up)
        animsprite_update_sprite(mdlselect->animsprite, mdlselect->icon_up, false);
    if (mdlselect->has_down)
        animsprite_update_sprite(mdlselect->animsprite, mdlselect->icon_down, true);

    return 0;
}


char* weekselector_mdlselect_get_manifest(WeekSelectorMdlSelect mdlselect) {
    if (mdlselect->index < 0 || mdlselect->index >= mdlselect->list_size) return NULL;
    CharacterInfo* mdl = &mdlselect->list[mdlselect->index];
    return mdl->is_locked ? NULL : string_duplicate(mdl->manifest);
}

bool weekselector_mdlselect_is_selected_locked(WeekSelectorMdlSelect mdlselect) {
    if (mdlselect->index < 0 || mdlselect->index >= mdlselect->list_size) return true;
    return mdlselect->list[mdlselect->index].is_locked;
}

void weekselector_mdlselect_toggle_choosen(WeekSelectorMdlSelect mdlselect) {
    mdlselect->hey_playing = statesprite_state_toggle(mdlselect->preview, WEEKSELECTOR_MDLSELECT_HEY);
}

void weekselector_mdlselect_enable_arrows(WeekSelectorMdlSelect mdlselect, bool enabled) {
    mdlselect->is_enabled = enabled;
}

void weekselector_mdlselect_select_default(WeekSelectorMdlSelect mdlselect) {
    if (mdlselect->index == 0) return;
    weekselector_mdlselect_select(mdlselect, 0);
}

bool weekselector_mdlselect_select(WeekSelectorMdlSelect mdlselect, int32_t new_index) {
    if (new_index < 0 || new_index >= mdlselect->list_size) return false;
    if (new_index == mdlselect->index) return true;

    mdlselect->is_locked = mdlselect->list[new_index].is_locked;
    mdlselect->index = new_index;
    mdlselect->has_up = new_index > 0;
    mdlselect->has_down = new_index < (mdlselect->list_size - 1);

    statesprite_state_toggle(mdlselect->preview, NULL);
    statesprite_state_remove(mdlselect->preview, WEEKSELECTOR_MDLSELECT_IDLE);
    statesprite_state_remove(mdlselect->preview, WEEKSELECTOR_MDLSELECT_HEY);
    drawable_set_visible(mdlselect->drawable_character, false);
    textsprite_set_text_intern(mdlselect->label, true, (const char* const*)&mdlselect->list[new_index].name);

    sprite_set_alpha(
        mdlselect->icon_up, mdlselect->has_up ? 1.0f : WEEKSELECTOR_ARROW_DISABLED_ALPHA
    );
    sprite_set_alpha(
        mdlselect->icon_down, mdlselect->has_down ? 1.0f : WEEKSELECTOR_ARROW_DISABLED_ALPHA
    );

    mdlselect->load_character_id++;
    mdlselect->running_threads++;
    main_thd_helper_spawn(true, (AsyncThread)weekselector_mdlselect_internal_load_async, mdlselect);

    return true;
}

bool weekselector_mdlselect_scroll(WeekSelectorMdlSelect mdlselect, int32_t offset) {
    return weekselector_mdlselect_select(mdlselect, mdlselect->index + offset);
}

Drawable weekselector_mdlselect_get_drawable(WeekSelectorMdlSelect mdlselect) {
    return mdlselect->drawable;
}

void weekselector_mdlselect_set_beats(WeekSelectorMdlSelect mdlselect, float bpm) {
    beatwatcher_change_bpm(&mdlselect->beatwatcher, bpm);
}


static void weekselector_mdlselect_internal_place_arrow(Sprite sprite, float arrow_size, LayoutPlaceholder* placeholder, bool end) {
    float draw_width = 0.0f, draw_height = 0.0f;

    sprite_resize_draw_size(sprite, arrow_size, -1.0f, &draw_width, &draw_height);

    // note: the sprite is rotated (corner rotation)
    float x = placeholder->x + ((placeholder->width - draw_height) / 2.0f);
    float y = placeholder->y;
    if (end) y += placeholder->height - draw_width;

    sprite_set_draw_location(sprite, x, y);
    sprite_matrix_corner_rotation(sprite, end ? Corner_TopRight : Corner_BottomLeft);
}

static void* weekselector_mdlselect_internal_load_async(WeekSelectorMdlSelect mdlselect) {
    int32_t load_character_id = mdlselect->load_character_id;
    CharacterInfo* character_info = &mdlselect->list[mdlselect->index];
    bool week_selector_left_facing = character_info->week_selector_left_facing;
    bool week_selector_enable_beat = character_info->week_selector_enable_beat;
    bool flip_x = false;

    if (mdlselect->is_boyfriend && !week_selector_left_facing) flip_x = true;
    // else if (!mdlselect->is_boyfriend && week_selector_left_facing) flip_x = true;

    ModelHolder modelholder = modelholder_init(character_info->week_selector_model);

    if (load_character_id != mdlselect->load_character_id) {
        if (modelholder) modelholder_destroy(&modelholder);
        goto L_return;
    }

    if (!modelholder) {
        logger_error("weekselector_mdlselect_internal_load_async() modelholder_init failed");
        goto L_return;
    }

    weekselector_mdlselect_helper_import(
        mdlselect->preview, modelholder,
        mdlselect->placeholder_character,
        week_selector_enable_beat,
        character_info->week_selector_idle_anim_name ? character_info->week_selector_idle_anim_name : WEEKSELECTOR_MDLSELECT_IDLE,
        WEEKSELECTOR_MDLSELECT_IDLE
    );
    weekselector_mdlselect_helper_import(
        mdlselect->preview, modelholder,
        mdlselect->placeholder_character,
        false,
        character_info->week_selector_choosen_anim_name ? character_info->week_selector_choosen_anim_name : WEEKSELECTOR_MDLSELECT_HEY,
        WEEKSELECTOR_MDLSELECT_HEY
    );
    modelholder_destroy(&modelholder);

    statesprite_state_toggle(mdlselect->preview, WEEKSELECTOR_MDLSELECT_IDLE);
    statesprite_animate(mdlselect->preview, beatwatcher_remaining_until_next(&mdlselect->beatwatcher));
    statesprite_flip_texture(mdlselect->preview, flip_x, false);
    drawable_set_visible(mdlselect->drawable_character, true);

L_return:
    mdlselect->running_threads--;
    return NULL;
}

void weekselector_mdlselect_helper_import(StateSprite statesprite, ModelHolder mdlhldr, LayoutPlaceholder* placeholder, bool enable_beat, const char* anim_name, const char* name) {
    if (!anim_name) return;
    StateSpriteState* statesprite_state = statesprite_state_add(statesprite, mdlhldr, anim_name, name);
    if (!statesprite_state) return;

    imgutils_calc_rectangle_in_statesprite_state(
        0.0f,
        0.0f,
        placeholder->width,
        placeholder->height,
        placeholder->align_horizontal,
        placeholder->align_vertical,
        statesprite_state
    );

    if (!statesprite_state->animation) return;

    if (enable_beat || string_equals(name, WEEKSELECTOR_MDLSELECT_HEY)) {
        animsprite_set_loop(statesprite_state->animation, true);
    }
}

static void weekselector_mdlselect_helper_import_from_manifest(const char* src, CharacterInfo* list_item) {
    CharacterManifest charactermanifest = charactermanifest_init(src, false);
    list_item->week_selector_left_facing = charactermanifest->week_selector_left_facing;
    list_item->week_selector_enable_beat = charactermanifest->week_selector_enable_beat;
    list_item->week_selector_model = string_duplicate(charactermanifest->week_selector_model);
    list_item->week_selector_idle_anim_name = string_duplicate(charactermanifest->week_selector_idle_anim_name);
    list_item->week_selector_choosen_anim_name = string_duplicate(charactermanifest->week_selector_choosen_anim_name);
    charactermanifest_destroy(&charactermanifest);
}
