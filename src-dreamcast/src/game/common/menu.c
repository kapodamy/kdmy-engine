#include "game/common/menu.h"

#include "animsprite.h"
#include "drawable.h"
#include "externals/luascript.h"
#include "fontglyph.h"
#include "fonttype.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "modelholder.h"
#include "statesprite.h"
#include "stringutils.h"
#include "textsprite.h"
#include "tweenlerp.h"


typedef union {
    StateSprite as_statesprite;
    TextSprite as_textsprite;
} VertexPointer;

typedef struct {
    char* name;
    AnimSprite anim_self;
    AnimSprite anim_choosen;
    AnimSprite anim_discarded;
    AnimSprite anim_idle;
    AnimSprite anim_rollback;
    AnimSprite anim_selected;
    AnimSprite anim_in;
    AnimSprite anim_out;
    bool is_text;
    VertexPointer vertex;
    float center_x;
    float center_y;
    float cell_dimmen;
    float cell_gap;
    float placement_x;
    float placement_y;
    bool rollback_active;
    float render_distance;
    bool has_scale;
    bool hidden;
} MenuItem;

struct Menu_s {
    MenuItem* items;
    int32_t items_size;
    FontHolder fontholder;
    float transition_delay_in;
    float transition_delay_out;
    bool is_vertical;
    Align align;
    float gap;
    bool sparse;
    int32_t static_index;
    TweenLerp tweenlerp;
    Drawable drawable;
    float x;
    float y;
    float width;
    float height;
    int32_t index_selected;
    bool item_choosen;
    bool transition_active;
    bool transition_out;
    int32_t render_start;
    int32_t render_end;
    float render_distance;
    float render_distance_end;
    float render_distance_last;
    bool per_page;
    int32_t* map;
    int32_t map_size;
    bool drawcallback_before;
    void* drawcallback_privatedata;
    MenuDrawCallback drawcallback;
};


static void menu_internal_build_item(MenuItem* item, MenuManifestItem* src_item, MenuManifestParameters* params, ModelHolder modelholder, FontHolder fontholder, RGBA border);
static AnimSprite menu_internal_load_anim(ModelHolder modelholder, const char* absolute_name, const char* prefix, const char* suffix);
static void menu_internal_calc_item_bounds(Menu menu, int32_t index);
static int32_t menu_internal_animate(Menu menu, int32_t index, AnimSprite anim, float elapsed);
static void menu_internal_set_index_selected(Menu menu, int32_t new_index);
static void menu_internal_transition(Menu menu);
static bool menu_internal_set_selected_sparse(Menu menu, int32_t offset_x, int32_t offset_y);
static void menu_internal_build_map(Menu menu);
static bool menu_internal_scroll(Menu menu, int32_t offset);
static bool menu_internal_draw_callback(Menu menu, PVRContext pvrctx, int32_t index);


Menu menu_init(MenuManifest menumanifest, float x, float y, float z, float width, float height) {
    MenuManifestParameters* params = &menumanifest->parameters;
    MenuItem* items = malloc_for_array(MenuItem, menumanifest->items_size);
    ModelHolder modelholder = modelholder_init2(params->font_color, params->atlas, params->animlist);

    FontHolder fontholder = NULL;
    if (params->font) {
        fontholder = fontholder_init(params->font, params->font_size, params->font_glyph_suffix, params->font_color_by_addition);
        if (!fontholder) {
            logger_error("Missing or invalid font: %s", params->font);
            assert(fontholder);
        }
    }

    RGBA border = {0.0f, 0.0f, 0.0f, 0.0f};
    math2d_color_bytes_to_floats(params->font_border_color, true, border);

    AnimSprite anim_discarded = NULL;
    AnimSprite anim_idle = NULL;
    AnimSprite anim_rollback = NULL;
    AnimSprite anim_selected = NULL;
    AnimSprite anim_choosen = NULL;
    AnimSprite anim_in = NULL;
    AnimSprite anim_out = NULL;

    if (params->anim_discarded)
        anim_discarded = modelholder_create_animsprite(modelholder, params->anim_discarded, false, false);
    if (params->anim_idle)
        anim_idle = modelholder_create_animsprite(modelholder, params->anim_idle, false, false);
    if (params->anim_rollback)
        anim_rollback = modelholder_create_animsprite(modelholder, params->anim_rollback, false, false);
    if (params->anim_selected)
        anim_selected = modelholder_create_animsprite(modelholder, params->anim_selected, false, false);
    if (params->anim_choosen)
        anim_choosen = modelholder_create_animsprite(modelholder, params->anim_choosen, false, false);
    if (params->anim_in)
        anim_in = modelholder_create_animsprite(modelholder, params->anim_in, false, false);
    if (params->anim_out)
        anim_out = modelholder_create_animsprite(modelholder, params->anim_out, false, false);

    for (int32_t i = 0; i < menumanifest->items_size; i++) {
        MenuManifestItem* src_item = &menumanifest->items[i];

        items[i] = (MenuItem){
            .name = NULL,

            .anim_self = NULL,

            .anim_choosen = NULL,
            .anim_discarded = NULL,
            .anim_idle = NULL,
            .anim_rollback = NULL,
            .anim_selected = NULL,
            .anim_in = NULL,
            .anim_out = NULL,

            .is_text = false,
            .vertex = {NULL},

            .center_x = 0.0f,
            .center_y = 0.0f,

            .cell_dimmen = FLOAT_NaN,
            .cell_gap = menumanifest->items[i].placement.gap,
            .placement_x = menumanifest->items[i].placement.x,
            .placement_y = menumanifest->items[i].placement.y,

            .rollback_active = false,
            .render_distance = 0.0f,
            .has_scale = false,

            .hidden = false
        };

        menu_internal_build_item(&items[i], src_item, params, modelholder, fontholder, border);

        if (!items[i].anim_choosen && anim_choosen) items[i].anim_choosen = animsprite_clone(anim_choosen);
        if (!items[i].anim_discarded && anim_discarded) items[i].anim_discarded = animsprite_clone(anim_discarded);
        if (!items[i].anim_idle && anim_idle) items[i].anim_idle = animsprite_clone(anim_idle);
        if (!items[i].anim_rollback && anim_rollback) items[i].anim_rollback = animsprite_clone(anim_rollback);
        if (!items[i].anim_selected && anim_selected) items[i].anim_selected = animsprite_clone(anim_selected);
        if (!items[i].anim_in && anim_in) items[i].anim_in = animsprite_clone(anim_in);
        if (!items[i].anim_out && anim_out) items[i].anim_out = animsprite_clone(anim_out);
    }

    if (anim_discarded) animsprite_destroy(&anim_discarded);
    if (anim_idle) animsprite_destroy(&anim_idle);
    if (anim_rollback) animsprite_destroy(&anim_rollback);
    if (anim_selected) animsprite_destroy(&anim_selected);
    if (anim_choosen) animsprite_destroy(&anim_choosen);
    if (anim_in) animsprite_destroy(&anim_in);
    if (anim_out) animsprite_destroy(&anim_out);

    modelholder_destroy(&modelholder);

    Align align;
    switch (params->items_align) {
        case ALIGN_START:
        case ALIGN_CENTER:
        case ALIGN_END:
            align = params->items_align;
            break;
        case ALIGN_BOTH:
        case ALIGN_NONE:
            // unused enumerations
        default:
            align = ALIGN_START;
            break;
    }

    Menu menu = malloc_chk(sizeof(struct Menu_s));
    malloc_assert(menu, Menu);

    *menu = (struct Menu_s){
        .items = items,
        .items_size = menumanifest->items_size,

        .fontholder = fontholder,

        .transition_delay_in = params->anim_transition_in_delay,
        .transition_delay_out = params->anim_transition_out_delay,

        .is_vertical = params->is_vertical,
        .align = align,
        .gap = params->items_gap,
        .sparse = params->is_sparse,
        .static_index = params->static_index,
        .tweenlerp = NULL,

        .drawable = NULL,
        .x = x,
        .y = y,
        .width = width,
        .height = height,

        .index_selected = -1,
        .item_choosen = false,

        .transition_active = false,
        .transition_out = false,

        .render_start = 0,
        .render_end = -1,
        .render_distance = 0.0f,
        .render_distance_end = 0.0f,
        .render_distance_last = 0.0f,

        .per_page = params->is_per_page,
        .map = NULL,
        .map_size = 0,
        .drawcallback_before = false,
        .drawcallback_privatedata = NULL,
        .drawcallback = NULL
    };

    if (params->static_index) {
        menu->tweenlerp = tweenlerp_init();
        tweenlerp_add_linear(menu->tweenlerp, 0, 0.0f, 0.0f, 100.0f);
    }

    menu->drawable = drawable_init(z, menu, (DelegateDraw)menu_draw, (DelegateAnimate)menu_animate);

    Modifier* modifier = drawable_get_modifier(menu->drawable);
    modifier->x = x;
    modifier->y = y;
    modifier->width = width;
    modifier->height = height;

    if (menu->sparse) {
        for (int32_t i = 0; i < menu->items_size; i++) {
            menu->items[i].center_x = menumanifest->items[i].placement.x;
            menu->items[i].center_y = menumanifest->items[i].placement.y;
        }
    } else {
        menu_internal_build_map(menu);
    }

    // select the first visible item
    for (int32_t i = 0; i < menu->items_size; i++) {
        if (!menu->items[i].hidden) {
            menu_internal_set_index_selected(menu, i);
            break;
        }
    }

    return menu;
}

void menu_destroy(Menu* menu_ptr) {
    Menu menu = *menu_ptr;
    if (!menu) return;

    luascript_drop_shared(menu);

    if (menu->fontholder) fontholder_destroy(&menu->fontholder);

    for (int32_t i = 0; i < menu->items_size; i++) {
        if (menu->items[i].is_text) {
            textsprite_destroy(&menu->items[i].vertex.as_textsprite);
        } else {
            statesprite_destroy_texture_if_stateless(menu->items[i].vertex.as_statesprite); // important step
            statesprite_destroy(&menu->items[i].vertex.as_statesprite);
        }

        free_chk(menu->items[i].name);

        if (menu->items[i].anim_self) animsprite_destroy(&menu->items[i].anim_self);
        if (menu->items[i].anim_discarded) animsprite_destroy(&menu->items[i].anim_discarded);
        if (menu->items[i].anim_idle) animsprite_destroy(&menu->items[i].anim_idle);
        if (menu->items[i].anim_rollback) animsprite_destroy(&menu->items[i].anim_rollback);
        if (menu->items[i].anim_selected) animsprite_destroy(&menu->items[i].anim_selected);
        if (menu->items[i].anim_choosen) animsprite_destroy(&menu->items[i].anim_choosen);
        if (menu->items[i].anim_in) animsprite_destroy(&menu->items[i].anim_in);
        if (menu->items[i].anim_out) animsprite_destroy(&menu->items[i].anim_out);
    }

    if (menu->tweenlerp) tweenlerp_destroy(&menu->tweenlerp);

    drawable_destroy(&menu->drawable);

    free_chk(menu->items);
    free_chk(menu->map);

    free_chk(menu);
    *menu_ptr = NULL;
}


Drawable menu_get_drawable(Menu menu) {
    return menu->drawable;
}

void menu_trasition_in(Menu menu) {
    menu->transition_out = false;
    menu_internal_transition(menu);
}

void menu_trasition_out(Menu menu) {
    menu->transition_out = true;
    menu_internal_transition(menu);
}


bool menu_select_item(Menu menu, const char* name) {
    for (int32_t i = 0; i < menu->items_size; i++) {
        if (string_equals(menu->items[i].name, name)) {
            if (menu->items[i].hidden) return false;
            menu_internal_set_index_selected(menu, i);
            return true;
        };
    }
    menu_internal_set_index_selected(menu, -1);
    return false;
}

void menu_select_index(Menu menu, int32_t index) {
    if (index >= 0 && index < menu->items_size && menu->items[index].hidden) return;
    menu_internal_set_index_selected(menu, index);
}

bool menu_select_vertical(Menu menu, int32_t offset) {
    if (offset == 0) return true;
    if (menu->sparse) return menu_internal_set_selected_sparse(menu, 0, offset);
    if (!menu->is_vertical) return false;

    return menu_internal_scroll(menu, offset);
}

bool menu_select_horizontal(Menu menu, int32_t offset) {
    if (offset == 0) return true;
    if (menu->sparse) return menu_internal_set_selected_sparse(menu, offset, 0);
    if (menu->is_vertical) return false;

    return menu_internal_scroll(menu, offset);
}

void menu_toggle_choosen(Menu menu, bool enable) {
    menu->item_choosen = enable;
    for (int32_t i = 0; i < menu->items_size; i++) {
        if (menu->items[i].anim_choosen) animsprite_restart(menu->items[i].anim_choosen);
        if (menu->items[i].anim_discarded) animsprite_restart(menu->items[i].anim_discarded);

        if (menu->index_selected == i) {
            menu->items[i].rollback_active = false;

            if (menu->items[i].anim_rollback) {
                animsprite_restart(menu->items[i].anim_rollback);
                animsprite_force_end(menu->items[i].anim_rollback);

                if (menu->items[i].is_text) {
                    animsprite_update_textsprite(menu->items[i].anim_rollback, menu->items[i].vertex.as_textsprite, true);
                } else {
                    animsprite_update_statesprite(menu->items[i].anim_rollback, menu->items[i].vertex.as_statesprite, true);
                }
            }
        }
    }
}

int32_t menu_get_selected_index(Menu menu) {
    return menu->index_selected;
}

int32_t menu_get_items_count(Menu menu) {
    return menu->items_size;
}

bool menu_set_item_text(Menu menu, int32_t index, const char* text) {
    if (index < 0 || index >= menu->items_size) return false;
    if (!menu->items[index].is_text) return false;
    textsprite_set_text_intern(menu->items[index].vertex.as_textsprite, false, &text);
    return true;
}

bool menu_set_item_visibility(Menu menu, int32_t index, bool visible) {
    if (index < 0 || index >= menu->items_size) return false;

    menu->items[index].hidden = !visible;

    if (menu->index_selected == index) {
        for (index = menu->index_selected; index > 0; index--) {
            if (!menu->items[menu->index_selected].hidden) break;
        }
        menu_internal_set_index_selected(menu, menu->index_selected);
    }

    menu_internal_build_map(menu);
    return true;
}

bool menu_has_valid_selection(Menu menu) {
    return menu->index_selected >= 0 && menu->index_selected < menu->items_size;
}


int32_t menu_animate(Menu menu, float elapsed) {
    for (int32_t i = 0; i < menu->items_size; i++) {
        menu_internal_animate(menu, i, menu->items[i].anim_self, elapsed);

        if (menu->index_selected == i) {
            AnimSprite anim = menu->item_choosen ? menu->items[i].anim_choosen : menu->items[i].anim_selected;
            menu_internal_animate(menu, i, anim, elapsed);
        } else {
            if (menu->item_choosen) {
                menu_internal_animate(menu, i, menu->items[i].anim_discarded, elapsed);
            } else if (menu->items[i].rollback_active) {
                if (menu_internal_animate(menu, i, menu->items[i].anim_rollback, elapsed)) {
                    menu->items[i].rollback_active = false;
                }
            } else {
                menu_internal_animate(menu, i, menu->items[i].anim_idle, elapsed);
            }
        }

        if (menu->transition_active) {
            if (menu->transition_out)
                menu_internal_animate(menu, i, menu->items[i].anim_out, elapsed);
            else
                menu_internal_animate(menu, i, menu->items[i].anim_in, elapsed);
        }

        menu_internal_calc_item_bounds(menu, i);
    }

    if (menu->fontholder && menu->fontholder->font_from_atlas)
        fontglyph_animate(menu->fontholder->font, elapsed);

    if (menu->tweenlerp) tweenlerp_animate(menu->tweenlerp, elapsed);

    return 0;
}

void menu_draw(Menu menu, PVRContext pvrctx) {
    pvr_context_save(pvrctx);

    if (menu->sparse) {
        for (int32_t i = 0; i < menu->items_size; i++) {
            if (menu->items[i].hidden) continue;

            if (menu->drawcallback_before && menu_internal_draw_callback(menu, pvrctx, i)) continue;

            if (menu->items[i].is_text)
                textsprite_draw(menu->items[i].vertex.as_textsprite, pvrctx);
            else
                statesprite_draw(menu->items[i].vertex.as_statesprite, pvrctx);

            if (!menu->drawcallback_before && menu_internal_draw_callback(menu, pvrctx, i)) continue;
        }
    } else {
        float render_distance;

        if (menu->tweenlerp) {
            if (menu->render_distance_last != menu->render_distance) {
                if (tweenlerp_is_completed(menu->tweenlerp)) tweenlerp_restart(menu->tweenlerp);
                tweenlerp_change_bounds_by_index(
                    menu->tweenlerp, 0, menu->render_distance_last, menu->render_distance
                );
                menu->render_distance_last = menu->render_distance;
            }
            render_distance = tweenlerp_peek_value_by_index(menu->tweenlerp, 0);
        } else {
            render_distance = menu->render_distance;
        }

        if (menu->is_vertical)
            sh4matrix_translate_y(pvrctx->current_matrix, render_distance);
        else
            sh4matrix_translate_x(pvrctx->current_matrix, render_distance);

        pvr_context_flush(pvrctx);

        for (int32_t i = 0; i < menu->items_size; i++) {
            if (menu->items[i].hidden) continue;

//#ifdef DEBUG
//          if (i >= menu->render_start && i <= menu->render_end)
//              pvr_context_set_global_alpha(pvrctx, 1.0f);
//          else
//              pvr_context_set_global_alpha(pvrctx, 0.2f);
//#else
            // ignore items outside of the visible space
            if (i < menu->render_start || i > menu->render_end) continue;
//#endif

            if (menu->drawcallback_before && menu_internal_draw_callback(menu, pvrctx, i)) continue;

            if (menu->items[i].is_text)
                textsprite_draw(menu->items[i].vertex.as_textsprite, pvrctx);
            else
                statesprite_draw(menu->items[i].vertex.as_statesprite, pvrctx);

            if (!menu->drawcallback_before && menu_internal_draw_callback(menu, pvrctx, i)) continue;
        }
    }
    pvr_context_restore(pvrctx);
}


bool menu_get_item_rect(Menu menu, int32_t index, float* x, float* y, float* width, float* height) {
    if (index < 0 || index >= menu->items_size) return false;

    MenuItem* item = &menu->items[index];
    if (item->is_text) {
        textsprite_get_draw_location(item->vertex.as_textsprite, x, y);
        textsprite_get_draw_size(item->vertex.as_textsprite, width, height);
    } else {
        statesprite_get_draw_location(item->vertex.as_statesprite, x, y);
        statesprite_get_draw_size(item->vertex.as_statesprite, width, height);
    }

    if (menu->is_vertical)
        *y += menu->render_distance;
    else
        *x += menu->render_distance;

    return true;
}

bool menu_get_selected_item_rect(Menu menu, float* x, float* y, float* width, float* height) {
    return menu_get_item_rect(menu, menu->index_selected, x, y, width, height);
}

const char* menu_get_selected_item_name(Menu menu) {
    if (menu->index_selected < 0 || menu->index_selected >= menu->items_size) return NULL;

    return menu->items[menu->index_selected].name;
}

void menu_set_text_force_case(Menu menu, int32_t none_or_lowercase_or_uppercase) {
    for (int32_t i = 0; i < menu->items_size; i++) {
        if (menu->items[i].is_text) {
            textsprite_force_case(menu->items[i].vertex.as_textsprite, none_or_lowercase_or_uppercase);
        }
    }
}

void menu_set_draw_callback(Menu menu, bool before_or_after, MenuDrawCallback callback, void* privatedata) {
    menu->drawcallback_before = before_or_after;
    menu->drawcallback = callback;
    menu->drawcallback_privatedata = privatedata;
}

bool menu_has_item(Menu menu, const char* name) {
    for (int32_t i = 0; i < menu->items_size; i++) {
        if (string_equals(menu->items[i].name, name)) {
            return true;
        }
    }
    return false;
}

int32_t menu_index_of_item(Menu menu, const char* name) {
    for (int32_t i = 0; i < menu->items_size; i++) {
        if (string_equals(menu->items[i].name, name)) {
            return i;
        }
    }
    return -1;
}

void menu_set_item_image(Menu menu, int32_t index, ModelHolder modelholder, const char* atlas_or_animlist_entry_name) {
    if (index < 0 || index > menu->items_size) return;

    MenuItem* item = &menu->items[index];
    if (item->is_text) return;

    StateSprite statesprite = item->vertex.as_statesprite;
    Texture new_texture = modelholder_get_texture(modelholder, true);
    Texture old_texture = statesprite_set_texture(statesprite, new_texture, false);

    if (!new_texture) statesprite_set_vertex_color_rgb8(statesprite, modelholder_get_vertex_color(modelholder));

    if (item->anim_self) animsprite_destroy(&item->anim_self);
    if (old_texture) texture_destroy(&old_texture);

    item->anim_self = menu_internal_load_anim(
        modelholder, atlas_or_animlist_entry_name, NULL, NULL
    );

    if (item->anim_self) {
        animsprite_update_statesprite(item->anim_self, item->vertex.as_statesprite, false);
    }
}


static void menu_internal_build_item(MenuItem* item, MenuManifestItem* src_item, MenuManifestParameters* params, ModelHolder modelholder, FontHolder fontholder, RGBA border) {
    bool custom_modelholder = false;
    item->is_text = string_is_not_empty(src_item->text);
    item->name = string_duplicate(src_item->name);
    item->hidden = src_item->hidden;

    float dimmen = item->is_text ? params->font_size : params->items_dimmen;
    if (src_item->placement.dimmen > 0.0f) dimmen = src_item->placement.dimmen;

    if (item->is_text) {
        uint32_t font_color = params->font_color;
        if (src_item->has_font_color) font_color = src_item->font_color;

        // important: do not intern the text
        item->vertex.as_textsprite = textsprite_init2(fontholder, dimmen, font_color);
        textsprite_set_text_intern(item->vertex.as_textsprite, false, (const char* const*)&src_item->text);
        if (params->font_border_size > 0.0f) {
            textsprite_border_enable(item->vertex.as_textsprite, true);
            textsprite_border_set_color(
                item->vertex.as_textsprite, border[0], border[1], border[2], border[3]
            );
            textsprite_border_set_size(item->vertex.as_textsprite, params->font_border_size);
        }
    } else {
        if (src_item->model) {
            ModelHolder temp = modelholder_init(src_item->model);
            if (temp) {
                custom_modelholder = true;
                modelholder = temp;
            }
        }
        StateSprite statesprite = statesprite_init_from_texture(modelholder_get_texture(modelholder, true));
        statesprite_set_vertex_color_rgb8(statesprite, modelholder_get_vertex_color(modelholder));
        if (modelholder_is_invalid(modelholder)) statesprite_set_alpha(statesprite, 0.0f);

        item->vertex.as_statesprite = statesprite;

        float scale = src_item->texture_scale > 0.0f ? src_item->texture_scale : params->texture_scale;
        if (scale > 0.0f) {
            item->has_scale = true;
            statesprite_change_draw_size_in_atlas_apply(statesprite, true, scale);
        }
    }

    item->anim_self = menu_internal_load_anim(
        modelholder, src_item->name, NULL, NULL
    );
    item->anim_selected = menu_internal_load_anim(
        modelholder, src_item->anim_selected, src_item->name, params->suffix_selected
    );
    item->anim_choosen = menu_internal_load_anim(
        modelholder, src_item->anim_choosen, src_item->name, params->suffix_choosen
    );
    item->anim_discarded = menu_internal_load_anim(
        modelholder, src_item->anim_discarded, src_item->name, params->suffix_discarded
    );
    item->anim_idle = menu_internal_load_anim(
        modelholder, src_item->anim_idle, src_item->name, params->suffix_idle
    );
    item->anim_rollback = menu_internal_load_anim(
        modelholder, src_item->anim_rollback, src_item->name, params->suffix_rollback
    );
    item->anim_in = menu_internal_load_anim(
        modelholder, src_item->anim_in, src_item->name, params->suffix_in
    );
    item->anim_out = menu_internal_load_anim(
        modelholder, src_item->anim_out, src_item->name, params->suffix_out
    );

    if (custom_modelholder) modelholder_destroy(&modelholder);

    if (item->anim_self) {
        if (item->is_text)
            animsprite_update_textsprite(item->anim_self, item->vertex.as_textsprite, 0);
        else
            animsprite_update_statesprite(item->anim_self, item->vertex.as_statesprite, 0);
    }

    if (!params->is_vertical && item->is_text && params->enable_horizontal_text_correction) {
        float draw_width = 0.0f, draw_height = 0.0f;
        textsprite_get_draw_size(item->vertex.as_textsprite, &draw_width, &draw_height);
        dimmen = draw_width;
    }

    item->cell_dimmen = dimmen;
    item->cell_gap = math2d_is_double_NaN(src_item->placement.gap) ? 0.0f : src_item->placement.gap;
}

static AnimSprite menu_internal_load_anim(ModelHolder modelholder, const char* absolute_name, const char* prefix, const char* suffix) {
    char* temp;
    if (absolute_name)
        temp = (char*)absolute_name;
    else if (prefix && suffix)
        temp = string_concat_for_state_name(2, prefix, suffix);
    else
        return NULL;

    if (temp == NULL) return NULL;

    AnimSprite animsprite = modelholder_create_animsprite(modelholder, temp, true, !prefix && !suffix);
    if (!absolute_name) free_chk(temp);

    return animsprite;
}

static void menu_internal_calc_item_bounds(Menu menu, int32_t index) {
    float draw_width = 0.0f, draw_height = 0.0f;
    MenuItem* menu_item = &menu->items[index];
    float offset_x, offset_y;

    if (menu_item->is_text) {
        textsprite_get_draw_size(menu->items[index].vertex.as_textsprite, &draw_width, &draw_height);
    } else if (menu_item->has_scale) {
        statesprite_get_draw_size(menu_item->vertex.as_statesprite, &draw_width, &draw_height);
    } else {
        float max_width, max_height;
        if (menu->is_vertical) {
            max_width = -1.0f;
            max_height = menu_item->cell_dimmen;
        } else {
            max_width = menu_item->cell_dimmen;
            max_height = -1.0f;
        }
        statesprite_resize_draw_size(menu_item->vertex.as_statesprite, max_width, max_height, &draw_width, &draw_height);
    }

    if (menu->sparse) {
        offset_x = (draw_width / -2.0f) + menu->x;
        offset_y = (draw_height / -2.0f) + menu->y;
    } else if (menu->align == ALIGN_START) {
        offset_x = 0.0f;
        offset_y = 0.0f;
    } else {
        if (menu->is_vertical) {
            offset_x = menu->width - draw_width;
            if (menu->align == ALIGN_CENTER) offset_x /= 2.0f;
            offset_y = 0.0f;
        } else {
            offset_y = menu->height - draw_height;
            if (menu->align == ALIGN_CENTER) offset_y /= 2.0f;
            offset_x = 0.0f;
        }
    }

    offset_x += menu->items[index].center_x;
    offset_y += menu->items[index].center_y;

    if (menu->items[index].is_text)
        textsprite_set_draw_location(menu->items[index].vertex.as_textsprite, offset_x, offset_y);
    else
        statesprite_set_draw_location(menu->items[index].vertex.as_statesprite, offset_x, offset_y);
}

static int32_t menu_internal_animate(Menu menu, int32_t index, AnimSprite anim, float elapsed) {
    if (!anim) return 1;

    int32_t completed = animsprite_animate(anim, elapsed);

    if (menu->items[index].is_text)
        animsprite_update_textsprite(anim, menu->items[index].vertex.as_textsprite, true);
    else
        animsprite_update_statesprite(anim, menu->items[index].vertex.as_statesprite, true);

    return completed;
}

static void menu_internal_set_index_selected(Menu menu, int32_t new_index) {
    int32_t old_index = menu->index_selected;

    if (old_index >= 0 && old_index < menu->items_size) {
        menu->items[old_index].rollback_active = !!menu->items[old_index].anim_rollback;

        if (menu->items[old_index].anim_rollback) {
            animsprite_restart(menu->items[old_index].anim_rollback);
            if (menu->item_choosen) {
                animsprite_force_end(menu->items[old_index].anim_rollback);
                if (menu->items[old_index].is_text) {
                    animsprite_update_textsprite(
                        menu->items[old_index].anim_rollback, menu->items[old_index].vertex.as_textsprite, true
                    );
                } else {
                    animsprite_update_statesprite(
                        menu->items[old_index].anim_rollback, menu->items[old_index].vertex.as_statesprite, true
                    );
                }
                menu->items[old_index].rollback_active = false;
            }
        }
        if (menu->items[old_index].anim_idle)
            animsprite_restart(menu->items[old_index].anim_idle);
    }

    if (new_index >= 0 && new_index < menu->items_size) {
        menu->items[new_index].rollback_active = false;

        if (menu->items[new_index].anim_self)
            animsprite_restart(menu->items[new_index].anim_self);

        if (menu->items[new_index].anim_rollback) {
            animsprite_restart(menu->items[new_index].anim_rollback);
            animsprite_force_end(menu->items[new_index].anim_rollback);

            if (menu->items[new_index].is_text) {
                animsprite_update_textsprite(
                    menu->items[new_index].anim_rollback, menu->items[new_index].vertex.as_textsprite, true
                );
            } else {
                animsprite_update_statesprite(
                    menu->items[new_index].anim_rollback, menu->items[new_index].vertex.as_statesprite, true
                );
            }
        }

        if (menu->items[new_index].anim_selected)
            animsprite_restart(menu->items[new_index].anim_selected);
    }

    menu->index_selected = new_index;
    if (new_index < 0 || new_index >= menu->items_size) return;
    if (menu->sparse) return;


    // calculate the visible page of the list
    if (!menu->map) {
        bool has_static_index = menu->static_index != 0;

        if (has_static_index) {
            int32_t lower_index = new_index - menu->static_index;
            if (lower_index >= menu->items_size) lower_index = menu->items_size - 1;

            while (lower_index > 0 && menu->items[lower_index].hidden) lower_index--;

            if (lower_index < 0) {
                if (old_index < 0) old_index = 0;
                has_static_index = false;
                goto L_stop_checking_static_index;
            }

            menu->render_start = lower_index;
            menu->render_end = menu->items_size;
            menu->render_distance = menu->items[lower_index].render_distance;
        }

    L_stop_checking_static_index:
        if (!has_static_index) {
            if (menu->render_end > 0 && (new_index - old_index) == 1) {
                menu->render_start++;
                menu->render_end++;
                menu->render_distance = -menu->items[menu->render_start].render_distance;
                return;
            }

            menu->render_start = new_index;
            menu->render_end = menu->items_size;
            menu->render_distance = menu->items[new_index].render_distance;
        }

        float stop = menu->render_distance + (menu->is_vertical ? menu->height : menu->width);
        for (int32_t i = menu->items_size - 1; i >= 0; i--) {
            if (menu->items[i].render_distance < stop) {
                if (stop < menu->render_distance_end) menu->render_end = i - 1;
                break;
            }
        }
        menu->render_distance = -menu->render_distance;
        return;
    }

    if (new_index < menu->render_start && new_index > menu->render_end) return;

    menu->render_start = -1;
    menu->render_end = menu->items_size;
    menu->render_distance = 0.0f;

    for (int32_t i = 0; i < menu->map_size; i++) {
        if (menu->map[i] > menu->index_selected) {
            menu->render_end = menu->map[i] - 1;
            i--;
            if (i >= 0) {
                menu->render_start = menu->map[i];
                menu->render_distance = menu->items[menu->map[i]].render_distance;
            }
            break;
        }
    }

    if (menu->render_start < 0 && menu->map_size > 0) {
        menu->render_start = menu->map[menu->map_size - 1];
        menu->render_distance = menu->items[menu->render_start].render_distance;
    }

    menu->render_distance = -menu->render_distance;
}

static void menu_internal_transition(Menu menu) {
    AnimSprite anim;
    float transition_delay = menu->transition_out ? menu->transition_delay_out : menu->transition_delay_in;

    bool reverse = transition_delay < 0.0f;
    if (reverse) transition_delay = fabsf(transition_delay);

    float nonvisible_first_delay = transition_delay * menu->render_start;
    float nonvisible_last_delay = transition_delay * menu->render_end;

    menu->transition_active = true;

    for (int32_t i = 0; i < menu->items_size; i++) {
        if (menu->transition_out)
            anim = menu->items[i].anim_out;
        else
            anim = menu->items[i].anim_in;

        if (!anim) continue;
        animsprite_restart(anim);

        float delay;

        if (i < menu->render_start) {
            delay = reverse ? nonvisible_last_delay : nonvisible_first_delay;
        } else if (i > menu->render_end) {
            delay = reverse ? nonvisible_last_delay : nonvisible_first_delay;
        } else {
            if (reverse)
                delay = transition_delay * (menu->render_end - i);
            else
                delay = transition_delay * (i - menu->render_start);
        }

        animsprite_set_delay(anim, delay);
    }
}

static bool menu_internal_set_selected_sparse(Menu menu, int32_t offset_x, int32_t offset_y) {
    if (menu->index_selected < 0 || menu->index_selected >= menu->items_size) {
        menu_select_index(menu, 0);
        return true;
    }

    int32_t short_index = -1;
    float short_distance = FLOAT_Inf;

    float center_x = menu->items[menu->index_selected].center_x;
    float center_y = menu->items[menu->index_selected].center_y;

    for (int32_t i = 0; i < menu->items_size; i++) {
        if (menu->index_selected == i) continue;
        if (menu->items[i].hidden) continue;

        if (offset_x != 0) {
            if (offset_x > 0) {
                if (menu->items[i].center_x < center_x) continue;
            } else {
                if (menu->items[i].center_x > center_x) continue;
            }
        }
        if (offset_y != 0) {
            if (offset_y > 0) {
                if (menu->items[i].center_y < center_y) continue;
            } else {
                if (menu->items[i].center_y > center_y) continue;
            }
        }

        float distance = math2d_points_distance(
            menu->items[i].center_x, menu->items[i].center_y, center_x, center_y
        );

        if (distance < short_distance) {
            short_distance = distance;
            short_index = i;
        }
    }

    if (short_index < 0) return false;

    menu_internal_set_index_selected(menu, short_index);
    return true;
}

static void menu_internal_build_map(Menu menu) {
    if (menu->sparse) return;

    // dispose last map created (if defined)
    free_chk(menu->map);
    menu->map = NULL;
    menu->map_size = 0;

    float accumulator = 0;
    for (int32_t i = 0; i < menu->items_size; i++) {
        if (menu->items[i].hidden) continue;
        menu->items[i].render_distance = accumulator;
        accumulator += menu->items[i].cell_dimmen + (menu->items[i].cell_gap * 2.0f) + menu->gap;
    }

    float render_distance = FLOAT_Inf;
    for (int32_t i = menu->items_size - 1; i >= 0; i--) {
        if (menu->items[i].hidden)
            menu->items[i].render_distance = render_distance;
        else
            render_distance = menu->items[i].render_distance;
    }

    float dimmen = menu->is_vertical ? menu->height : menu->width;
    float pages = (accumulator - menu->gap) / dimmen;

    menu->render_distance_end = accumulator;

    if (menu->per_page && pages > 0) {
        menu->map_size = (int32_t)ceilf(pages);
        menu->map = malloc_for_array(int32_t, menu->map_size);
    }

    int32_t j = 0;
    float next_distance = -1.0f;
    for (int32_t i = 0; i < menu->items_size; i++) {
        if (menu->items[i].hidden) continue;

        float draw_x = menu->x + menu->items[i].placement_x;
        float draw_y = menu->y + menu->items[i].placement_y;

        if (menu->is_vertical) {
            draw_y += menu->items[i].render_distance + menu->items[i].cell_gap;
        } else {
            draw_x += menu->items[i].render_distance + menu->items[i].cell_gap;
        }

        if (menu->map && (menu->items[i].render_distance - menu->gap) > next_distance) {
            menu->map[j++] = math2d_max_int(0, i - 1);
            next_distance = dimmen * j;
        }

        menu->items[i].center_x = draw_x;
        menu->items[i].center_y = draw_y;
    }

    if (menu->map) {
        if (j < menu->map_size) menu->map[j] = menu->items_size - 1;

        if (menu->map_size > 1)
            menu->render_end = menu->map[1] - 1;
        else
            menu->render_end = menu->items_size;
    }
}

static bool menu_internal_scroll(Menu menu, int32_t offset) {
    int32_t new_index = offset + menu->index_selected;

    // skip hidden items
    while (new_index >= 0 && new_index < menu->items_size && menu->items[new_index].hidden) {
        new_index += offset;
    }

    if (new_index < 0 || new_index >= menu->items_size) return false;

    menu_internal_set_index_selected(menu, new_index);
    return true;
}

static bool menu_internal_draw_callback(Menu menu, PVRContext pvrctx, int32_t index) {
    if (menu->drawcallback == NULL) return false;

    float x = 0.0f, y = 0.0f;
    float width = 0.0f, height = 0.0f;
    MenuItem* item = &menu->items[index];

    if (item->is_text) {
        textsprite_get_draw_location(item->vertex.as_textsprite, &x, &y);
        textsprite_get_draw_size(item->vertex.as_textsprite, &width, &height);
    } else {
        statesprite_get_draw_location(item->vertex.as_statesprite, &x, &y);
        statesprite_get_draw_size(item->vertex.as_statesprite, &width, &height);
    }

    return !menu->drawcallback(
        menu->drawcallback_privatedata, pvrctx, menu, index, x, y, width, height
    );
}
