#include "game/savemanager.h"

#include <dc/maple.h>

#include "animlist.h"
#include "drawable.h"
#include "game/common/funkinsave.h"
#include "game/common/menu.h"
#include "game/common/messagebox.h"
#include "game/gameplay/helpers/menumanifest.h"
#include "game/helpers/weekselector_helptext.h"
#include "game/mainmenu.h"
#include "game/modding.h"
#include "game/weekselector.h"
#include "gamepad.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "modelholder.h"
#include "pvrcontext_global.h"
#include "sprite.h"
#include "textsprite.h"


typedef struct {
    bool has_savedata;
    int8_t port;
    int8_t unit;
} VMUInfo;

struct SaveManager_s {
    int32_t error_code;
    bool save_only;
    bool allow_delete;
    bool no_leave_confirm;
    bool warn_if_no_vmu;
    Drawable drawable_wrapper;
    VMUInfo* vmu_array;
    int32_t vmu_size;
    Menu menu;
    MessageBox messagebox;
    ModelHolder modelholder;
    Layout layout;
    const LayoutPlaceholder* placeholder;
    Sprite selected_background;
    TextSprite selected_label;
    Gamepad maple_pad;
    float padding;
    WeekSelectorHelpText help_cancel;
    WeekSelectorHelpText help_ok;
    WeekSelectorHelpText help_delete;
};


static const char* SAVEMANAGER_EMPTY = "empty";
static const char* SAVEMANAGER_FUNKINSAVE = "funkinsave";
static const float SAVEMANAGER_SCAN_INTERVAL = 1000.0f;

static const char* SAVEMANAGER_LAYOUT = "/assets/common/image/save-manager/layout.xml";
static const char* SAVEMANAGER_LAYOUT_DREAMCAST = "/assets/common/image/save-manager/layout~dreamcast.xml";
static const char* SAVEMANAGER_MODDING_SCRIPT = "/assets/common/data/scripts/savemanager.lua";

static struct MenuManifest_s SAVEMANAGER_MENU_MANIFEST = {
    .parameters = {
        .suffix_selected = NULL,
        .suffix_choosen = NULL,
        .suffix_discarded = NULL,
        .suffix_idle = NULL,     // unused
        .suffix_rollback = NULL, // unused
        .suffix_in = NULL,       // unused
        .suffix_out = NULL,      // unused

        .atlas = "/assets/common/image/save-manager/vmu.xml",
        .animlist = "/assets/common/image/save-manager/anims.xml",

        .anim_selected = "selected", // unused
        .anim_choosen = NULL,        // unused
        .anim_discarded = NULL,      // unused
        .anim_idle = "idle",         // unused
        .anim_rollback = NULL,       // unused
        .anim_in = NULL,             // unused
        .anim_out = NULL,            // unused

        .anim_transition_in_delay = -50.0f,
        .anim_transition_out_delay = -50.0f, // negative means start delays from the bottom

        .font = NULL,                    // unused
        .font_glyph_suffix = NULL,       // unused
        .font_color_by_addition = false, // unused
        .font_size = 0,                  // unused
        .font_color = 0x00,              // unused
        .font_border_color = 0x00,       // unused
        .font_border_size = FLOAT_NaN,   // unused

        .is_sparse = false, // unused
        .is_vertical = false,
        .is_per_page = true,

        .items_align = ALIGN_CENTER,              // readed from the layout
        .items_gap = 40.0f,                       // readed from the layout
        .items_dimmen = 80.0f,                    // readed from the layout
        .static_index = 0,                        // unused
        .texture_scale = FLOAT_NaN,               // unused
        .enable_horizontal_text_correction = true // unused
    },
    .items = NULL,
    .items_size = 0
};

static char* label_text = "VMU pu";

/**
 * This variable is set to true if the user does not want load and/or save
 * the progress. The savemanager_should_show() function will always return -1 if true
 */
static bool savemanager_game_withoutsavedata = false;


static void savemanager_internal_build_list(SaveManager savemanager);
static void savemanager_internal_commit_delete(SaveManager savemanager);
static bool savemanager_internal_find_changes(SaveManager savemanager);
static int32_t savemanager_internal_animate(SaveManager savemanager, float elapsed);
static void savemanager_internal_draw(SaveManager savemanager, PVRContext pvrctx);
static bool savemanager_internal_commit(SaveManager savemanager, int32_t selected_index);
static bool savemanager_internal_show_error(SaveManager savemanager, int32_t error_code);


SaveManager savemanager_init(bool save_only, int32_t error_code) {
    Layout layout = layout_init(pvr_context_is_widescreen() ? SAVEMANAGER_LAYOUT : SAVEMANAGER_LAYOUT_DREAMCAST);
    if (!layout) {
        logger_error("missing savemanager layout");
        assert(layout);
    }

    float label_height = (float)layout_get_attached_value_double(layout, "label_height", 24.0);
    uint32_t selected_bg_color = layout_get_attached_value_hex(layout, "selected_background_color", 0x000000);
    uint32_t selected_lbl_color = layout_get_attached_value_hex(layout, "selected_label_color", 0xFFFFFF);

    FontHolder font = layout_get_attached_font(layout, "font");
    Sprite selected_background = sprite_init_from_rgb8(selected_bg_color);
    TextSprite selected_label = textsprite_init2(font, label_height, selected_lbl_color);
    Gamepad maple_pad = gamepad_init(-1);

    float dimmen = (float)layout_get_attached_value_double(layout, "menu_itemDimmen", 80.0);
    float gap = (float)layout_get_attached_value_double(layout, "menu_itemGap", 40.0);
    float scale = (float)layout_get_attached_value_double(layout, "menu_itemScale", 0.0);
    float padding = dimmen * 0.1;

    gamepad_set_buttons_delay(maple_pad, 200);
    textsprite_set_align(selected_label, ALIGN_NONE, ALIGN_CENTER);
    textsprite_set_max_draw_size(selected_label, dimmen * 1.2f, -1.0f);

    SAVEMANAGER_MENU_MANIFEST.parameters.items_dimmen = dimmen;
    SAVEMANAGER_MENU_MANIFEST.parameters.items_gap = gap;
    SAVEMANAGER_MENU_MANIFEST.parameters.texture_scale = scale;

    LayoutPlaceholder* placeholder = layout_get_placeholder(layout, "menu");
    if (!placeholder) {
        logger_error("missing 'menu' placeholder");
        assert(placeholder);
    }

    ModelHolder modelholder = modelholder_init2(
        0x00,
        SAVEMANAGER_MENU_MANIFEST.parameters.atlas,
        SAVEMANAGER_MENU_MANIFEST.parameters.animlist
    );

    MessageBox messagebox = messagebox_init();
    messagebox_hide_image(messagebox, true);
    messagebox_use_small_size(messagebox, true);
    messagebox_show_buttons_icons(messagebox, false);

    ModelHolder button_icons = modelholder_init(WEEKSELECTOR_BUTTONS_MODEL);
    if (!button_icons) {
        logger_error("can not load %s", WEEKSELECTOR_BUTTONS_MODEL);
        assert(button_icons);
    }

    WeekSelectorHelpText help_cancel = weekselector_helptext_init(
        button_icons, layout, 2, false, "b", "Continue without save", NULL
    );
    WeekSelectorHelpText help_ok = weekselector_helptext_init(
        button_icons, layout, 4, false, "a", "Choose VMU", NULL
    );
    WeekSelectorHelpText help_delete = weekselector_helptext_init(
        button_icons, layout, 6, false, "x", "Delete savedata", NULL
    );
    modelholder_destroy(&button_icons);

    SaveManager savemanager = malloc_chk(sizeof(struct SaveManager_s));
    malloc_assert(savemanager, SaveManager);

    *savemanager = (struct SaveManager_s){
        .error_code = error_code < 1 ? 0 : error_code,
        .save_only = save_only,
        .allow_delete = false,
        .no_leave_confirm = false,
        .warn_if_no_vmu = true,
        .drawable_wrapper = NULL,
        .vmu_array = NULL,
        .vmu_size = 0,
        .menu = NULL,
        .messagebox = messagebox,

        .modelholder = modelholder,
        .layout = layout,
        .placeholder = placeholder,
        .selected_background = selected_background,
        .selected_label = selected_label,
        .maple_pad = maple_pad,
        .padding = padding,

        .help_cancel = help_cancel,
        .help_ok = help_ok,
        .help_delete = help_delete
    };

    weekselector_helptext_set_visible(help_delete, false);

    placeholder->vertex = savemanager->drawable_wrapper = drawable_init(
        placeholder->z, savemanager, (DelegateDraw)savemanager_internal_draw, (DelegateAnimate)savemanager_internal_animate
    );

    layout_external_vertex_create_entries(layout, 3);
    layout_external_vertex_set_entry(
        layout, 0, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_ok), 0
    );
    layout_external_vertex_set_entry(
        layout, 1, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_cancel), 0
    );
    layout_external_vertex_set_entry(
        layout, 2, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_delete), 0
    );

    return savemanager;
}

void savemanager_destroy(SaveManager* savemanager_ptr) {
    SaveManager savemanager = *savemanager_ptr;
    if (!savemanager) return;

    if (savemanager->menu) menu_destroy(&savemanager->menu);
    drawable_destroy(&savemanager->drawable_wrapper);
    free_chk(savemanager->vmu_array);
    layout_destroy(&savemanager->layout);
    sprite_destroy(&savemanager->selected_background);
    textsprite_destroy(&savemanager->selected_label);
    gamepad_destroy(&savemanager->maple_pad);
    messagebox_destroy(&savemanager->messagebox);
    modelholder_destroy(&savemanager->modelholder);
    weekselector_helptext_destroy(&savemanager->help_ok);
    weekselector_helptext_destroy(&savemanager->help_cancel);
    weekselector_helptext_destroy(&savemanager->help_delete);

    free_chk(savemanager);
    *savemanager_ptr = NULL;
}

void savemanager_show(SaveManager savemanager) {
    float location_x = 0.0f, location_y = 0.0f;
    float size_width = 0.0f, size_height = 0.0f;

    int32_t selected_index = -1;
    bool save_or_load_success = false;
    float next_scan = 0.0f;
    bool last_saved_selected = false;
    bool confirm_leave = false, confirm_delete = false;

    layout_trigger_any(savemanager->layout, savemanager->save_only ? "save-to" : "load-from");
    layout_trigger_any(savemanager->layout, savemanager->allow_delete ? "delete-allowed" : "delete-not-allowed");

    gamepad_clear_buttons(savemanager->maple_pad);

    Modding modding = modding_init(savemanager->layout, SAVEMANAGER_MODDING_SCRIPT);
    modding->native_menu = modding->active_menu = savemanager->menu;
    modding->callback_private_data = NULL;
    modding->callback_option = NULL;
    modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN, ModdingValueType_string);
    modding_helper_notify_event(modding, savemanager->save_only ? "do-save" : "do-load");
    modding_helper_notify_event(modding, savemanager->allow_delete ? "delete-enabled" : "delete-disabled");

    while (!modding->has_exit) {
        int32_t selection_offset_x = 0;
        int32_t selection_offset_y = 0;
        float elapsed = pvrctx_wait_ready();
        GamepadButtons buttons = gamepad_has_pressed_delayed(savemanager->maple_pad, MAINMENU_GAMEPAD_BUTTONS);

        // check for inserted VMUs
        next_scan -= elapsed;
        if (next_scan <= 0.0f) {
            int32_t last_vmu_size = savemanager->vmu_size;
            next_scan = SAVEMANAGER_SCAN_INTERVAL;

            if (savemanager_internal_find_changes(savemanager)) {
                savemanager_internal_build_list(savemanager);
                menu_select_index(savemanager->menu, -1);

                selected_index = -1;
                if (last_saved_selected) {
                    layout_trigger_any(savemanager->layout, "save-not-selected");
                    last_saved_selected = false;
                    confirm_delete = false;
                }

                if (modding->active_menu == modding->native_menu) modding->active_menu = savemanager->menu;
                modding->native_menu = savemanager->menu;
            }

            if (((last_vmu_size > 0) != (savemanager->vmu_size > 0)) || (savemanager->warn_if_no_vmu && last_vmu_size < 1)) {
                savemanager->warn_if_no_vmu = false;
                if (savemanager->vmu_size > 0)
                    layout_trigger_any(savemanager->layout, "no-detected-hide");
                else
                    layout_trigger_any(savemanager->layout, "no-detected-show");
            }
        }

        if (savemanager->menu && menu_get_selected_item_rect(savemanager->menu, &location_x, &location_y, &size_width, &size_height)) {
            location_x -= savemanager->padding;
            location_y -= savemanager->padding;
            size_width += savemanager->padding * 2.0f;
            size_height += savemanager->padding * 2.0f;

            sprite_set_draw_location(savemanager->selected_background, location_x, location_y);
            sprite_set_draw_size(savemanager->selected_background, size_width, size_height);
            sprite_set_visible(savemanager->selected_background, true);

            float label_height = textsprite_get_font_size(savemanager->selected_label);
            textsprite_set_draw_location(savemanager->selected_label, location_x, location_y - label_height);
            textsprite_set_visible(savemanager->selected_label, true);
        } else {
            sprite_set_visible(savemanager->selected_background, false);
            textsprite_set_visible(savemanager->selected_label, false);
        }

        ModdingHelperResult res = modding_helper_handle_custom_menu(modding, savemanager->maple_pad, elapsed);
        if (res != ModdingHelperResult_CONTINUE) break;
        if (modding->has_halt || modding->active_menu != savemanager->menu) buttons = 0x00;

        pvr_context_reset(&pvr_context);
        layout_animate(savemanager->layout, elapsed);
        layout_draw(savemanager->layout, &pvr_context);

        if (confirm_leave || confirm_delete) {
            messagebox_animate(savemanager->messagebox, elapsed);
            messagebox_draw(savemanager->messagebox, &pvr_context);

            if (buttons & MAINMENU_GAMEPAD_OK) {
                if (confirm_delete) {
                    savemanager_internal_commit_delete(savemanager);
                    confirm_delete = false;

                    menu_set_item_image(savemanager->menu, selected_index, savemanager->modelholder, SAVEMANAGER_EMPTY);
                    layout_trigger_any(savemanager->layout, "save-not-selected");
                    gamepad_enforce_buttons_delay(savemanager->maple_pad);
                    gamepad_clear_buttons(savemanager->maple_pad);
                    continue;
                }
                savemanager_game_withoutsavedata = true;
                break;
            }
            if (buttons & MAINMENU_GAMEPAD_CANCEL) {
                confirm_leave = confirm_delete = false;
                gamepad_clear_buttons(savemanager->maple_pad);
            }
            continue;
        }

        if (savemanager->error_code != 0) {
            messagebox_hide_buttons(savemanager->messagebox);
            messagebox_show(savemanager->messagebox, false);

            savemanager_internal_show_error(savemanager, savemanager->error_code);

            savemanager->error_code = 0;
            continue;
        }

        if (buttons & (GAMEPAD_A | GAMEPAD_START)) {
            selected_index = menu_get_selected_index(savemanager->menu);
            if (selected_index >= 0 && selected_index < menu_get_items_count(savemanager->menu)) {
                save_or_load_success = savemanager_internal_commit(savemanager, selected_index);
                savemanager_game_withoutsavedata = !save_or_load_success;
                if (save_or_load_success) break;
            }
        } else if (buttons & MAINMENU_GAMEPAD_CANCEL && !modding_helper_notify_back(modding)) {
            if (savemanager->no_leave_confirm) {
                // launched from settings menu, do not confirm leave
                break;
            }

            confirm_leave = true;
            messagebox_set_buttons_icons(savemanager->messagebox, "a", "b");
            messagebox_set_buttons_text(savemanager->messagebox, "Yes", "No");
            messagebox_set_title(savemanager->messagebox, "Confirm");
            messagebox_set_message(
                savemanager->messagebox, savemanager->save_only ? "¿Leave without saving?" : "¿Continue without load?"
            );
            messagebox_show(savemanager->messagebox, false);
            gamepad_clear_buttons(savemanager->maple_pad);
            continue;
        } else if (savemanager->allow_delete && (buttons & GAMEPAD_X)) {
            selected_index = menu_get_selected_index(savemanager->menu);
            if (selected_index < 0 || selected_index >= menu_get_items_count(savemanager->menu)) {
                continue;
            }
            if (!savemanager->vmu_array[selected_index].has_savedata) {
                continue;
            }

            confirm_delete = true;
            messagebox_set_buttons_icons(savemanager->messagebox, "a", "b");
            messagebox_set_buttons_text(savemanager->messagebox, "Yes", "No");
            messagebox_set_title(savemanager->messagebox, "Confirm Deletion");
            messagebox_set_message_formated(
                savemanager->messagebox,
                "¿Delete savedata on $s?\nThis operation can not be undone.",
                *(textsprite_get_string(savemanager->selected_label))
            );
            messagebox_show(savemanager->messagebox, false);
            gamepad_clear_buttons(savemanager->maple_pad);
            continue;
        } else if (buttons & GAMEPAD_AD_DOWN)
            selection_offset_y++;
        else if (buttons & GAMEPAD_AD_UP)
            selection_offset_y--;
        else if (buttons & GAMEPAD_AD_LEFT)
            selection_offset_x--;
        else if (buttons & GAMEPAD_AD_RIGHT)
            selection_offset_x++;

        if (selection_offset_x == 0 && selection_offset_y == 0) continue;

        bool success = false;

        if (selection_offset_x != 0 && menu_select_horizontal(savemanager->menu, selection_offset_x)) success = true;
        if (selection_offset_y != 0 && menu_select_vertical(savemanager->menu, selection_offset_y)) success = true;

        selected_index = menu_get_selected_index(savemanager->menu);
        if (!success || selected_index < 0 || selected_index >= savemanager->vmu_size) continue;

        const VMUInfo* vmu = &savemanager->vmu_array[selected_index];

        label_text[4] = 0x41 + vmu->port;
        label_text[5] = 0x30 + vmu->unit;

        textsprite_set_text_intern(savemanager->selected_label, true, (const char* const*)&label_text);

        if (last_saved_selected != vmu->has_savedata) {
            last_saved_selected = vmu->has_savedata;
            layout_trigger_any(savemanager->layout, vmu->has_savedata ? "save-selected" : "save-not-selected");
        }

        weekselector_helptext_set_visible(savemanager->help_delete, savemanager->allow_delete && vmu->has_savedata);
    }

    layout_trigger_any(savemanager->layout, "outro");
    modding_helper_notify_event(modding, "outro");

    if (save_or_load_success) {
        pvrctx_wait_ready();
        modding->has_exit = modding->has_halt = false;
        while (!modding->has_exit) {
            float elapsed = pvrctx_wait_ready();
            pvr_context_reset(&pvr_context);

            ModdingHelperResult res = modding_helper_handle_custom_menu(modding, savemanager->maple_pad, elapsed);
            if (res != ModdingHelperResult_CONTINUE) break;

            layout_animate(savemanager->layout, elapsed);
            layout_draw(savemanager->layout, &pvr_context);

            if (modding->has_halt) continue;
            if (layout_animation_is_completed(savemanager->layout, "transition_effect")) {
                // flush framebuffer again with last fade frame
                pvrctx_wait_ready();
                break;
            }
        }
    }

    modding_helper_notify_exit2(modding);
    modding_destroy(&modding);
}

int32_t savemanager_should_show(bool attempt_to_save_or_load) {
    if (savemanager_game_withoutsavedata) return -1;
    if (funkinsave_is_vmu_missing()) return -1;

    // attempt to automatically load/save
    int32_t result;
    if (attempt_to_save_or_load) {
        result = funkinsave_write_to_vmu();
    } else {
        result = funkinsave_read_from_vmu();
        if (result == 1 || result == 2) result = -1;
    }

    savemanager_game_withoutsavedata = result != 0;

    return result;
}

void savemanager_check_and_save_changes() {
    if (!funkinsave_has_changes) return;

    int32_t ret = savemanager_should_show(true);
    if (ret == 0) return;

    SaveManager savemanager = savemanager_init(true, ret);
    savemanager_show(savemanager);
    savemanager_destroy(&savemanager);
}

bool savemanager_is_running_without_savedata() {
    return savemanager_game_withoutsavedata;
}

void savemanager_change_actions(SaveManager savemanager, bool save_only, bool allow_delete, bool no_leave_confirm) {
    savemanager->error_code = 0;
    savemanager->save_only = save_only;
    savemanager->allow_delete = allow_delete;
    savemanager->no_leave_confirm = no_leave_confirm;

    weekselector_helptext_set_visible(savemanager->help_delete, allow_delete);

    // trigger all default actions
    layout_trigger_any(savemanager->layout, NULL);

    int32_t selected_index = savemanager->menu ? menu_get_selected_index(savemanager->menu) : -1;

    if (selected_index >= 0 && selected_index < savemanager->vmu_size) {
        if (savemanager->vmu_array[selected_index].has_savedata)
            layout_trigger_any(savemanager->layout, "save-selected");
        else
            layout_trigger_any(savemanager->layout, "save-not-selected");
    }
}


static void savemanager_internal_build_list(SaveManager savemanager) {
    int32_t selected_index = -1;
    if (savemanager->menu) {
        selected_index = menu_get_selected_index(savemanager->menu);
        menu_destroy(&savemanager->menu);
    }

    SAVEMANAGER_MENU_MANIFEST.items = malloc_for_array(MenuManifestItem, savemanager->vmu_size);
    SAVEMANAGER_MENU_MANIFEST.items_size = savemanager->vmu_size;

    for (int32_t i = 0; i < savemanager->vmu_size; i++) {
        SAVEMANAGER_MENU_MANIFEST.items[i] = (MenuManifestItem){
            .name = NULL,
            .text = NULL,
            .placement = (MenuManifestPlacement){.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f},
            .anim_selected = NULL,
            .anim_choosen = NULL,
            .anim_discarded = NULL,
            .anim_idle = NULL,
            .anim_rollback = NULL,
            .anim_in = NULL,
            .anim_out = NULL,
            .hidden = false,
            .description = NULL,
            .texture_scale = 0.0f,
        };

        if (savemanager->vmu_array[i].has_savedata)
            SAVEMANAGER_MENU_MANIFEST.items[i].name = (char*)SAVEMANAGER_FUNKINSAVE;
        else
            SAVEMANAGER_MENU_MANIFEST.items[i].name = (char*)SAVEMANAGER_EMPTY;
    }

    savemanager->menu = menu_init(
        &SAVEMANAGER_MENU_MANIFEST,
        savemanager->placeholder->x,
        savemanager->placeholder->y,
        savemanager->placeholder->z,
        savemanager->placeholder->width,
        savemanager->placeholder->height
    );

    if (selected_index >= 0 && selected_index < savemanager->vmu_size) {
        menu_select_index(savemanager->menu, selected_index);
    }

    // note: nothing is allocated inside of items[]
    free_chk(SAVEMANAGER_MENU_MANIFEST.items);
    SAVEMANAGER_MENU_MANIFEST.items_size = 0;
}

static bool savemanager_internal_find_changes(SaveManager savemanager) {
    int32_t index = 0;
    int32_t count = 0;

    // count all attached VMUs
    while (true) {
        maple_device_t* dev = maple_enum_type(index++, MAPLE_FUNC_MEMCARD);
        if (!dev || !dev->valid) break;
        count++;
    }

    // build new scan
    VMUInfo* new_vmu_array = malloc_for_array(VMUInfo, count);
    for (int32_t i = 0; i < count; i++) {
        maple_device_t* dev = maple_enum_type(i, MAPLE_FUNC_MEMCARD);
        if (!dev->valid) continue;

        int32_t found = funkinsave_has_savedata_in_vmu((int8_t)dev->port, (int8_t)dev->unit);
        new_vmu_array[i] = (VMUInfo){.has_savedata = found, .port = dev->port, .unit = dev->unit};
    }

    // check changes with the previous scan
    int32_t changes = math2d_max_int(count, savemanager->vmu_size);
    for (int32_t i = 0; i < savemanager->vmu_size; i++) {
        VMUInfo* old_vmu = &savemanager->vmu_array[i];
        for (int32_t j = 0; j < count; j++) {
            if (
                old_vmu->port == new_vmu_array[j].port &&
                old_vmu->unit == new_vmu_array[j].unit &&
                old_vmu->has_savedata == new_vmu_array[j].has_savedata
            ) {
                changes--;
            }
        }
    }

    if (changes < 1) {
        free_chk(new_vmu_array);
        return false;
    }

    // drop old VMU scan
    free_chk(savemanager->vmu_array);
    savemanager->vmu_array = new_vmu_array;
    savemanager->vmu_size = count;

    return true;
}

static int32_t savemanager_internal_animate(SaveManager savemanager, float elapsed) {
    int32_t completed = 0;
    completed += sprite_animate(savemanager->selected_background, elapsed);
    if (savemanager->menu) completed += menu_animate(savemanager->menu, elapsed);
    completed += textsprite_animate(savemanager->selected_label, elapsed);

    return completed;
}

static void savemanager_internal_draw(SaveManager savemanager, PVRContext pvrctx) {
    if (!savemanager->menu) return;
    if (sprite_is_visible(savemanager->selected_background)) sprite_draw(savemanager->selected_background, pvrctx);
    menu_draw(savemanager->menu, pvrctx);
    if (textsprite_is_visible(savemanager->selected_label)) textsprite_draw(savemanager->selected_label, pvrctx);
}

static bool savemanager_internal_commit(SaveManager savemanager, int32_t selected_index) {
    const VMUInfo* vmu = &savemanager->vmu_array[selected_index];

    if (!savemanager->save_only && !vmu->has_savedata) {
        messagebox_set_title(savemanager->messagebox, *textsprite_get_string(savemanager->selected_label));
        messagebox_set_message(savemanager->messagebox, "This vmu is empty ¿Create a new save?");
        messagebox_set_buttons_icons(savemanager->messagebox, "a", "b");
        messagebox_set_buttons_text(savemanager->messagebox, "Yes", "Pick another");
        messagebox_show(savemanager->messagebox, false);

        gamepad_clear_buttons(savemanager->maple_pad);

        while (true) {
            float elapsed = pvrctx_wait_ready();
            GamepadButtons buttons = gamepad_has_pressed_delayed(savemanager->maple_pad, MAINMENU_GAMEPAD_BUTTONS);

            pvr_context_reset(&pvr_context);
            layout_animate(savemanager->layout, elapsed);
            layout_draw(savemanager->layout, &pvr_context);
            messagebox_animate(savemanager->messagebox, elapsed);
            messagebox_draw(savemanager->messagebox, &pvr_context);

            if (buttons & MAINMENU_GAMEPAD_CANCEL) return false;
            if (buttons & MAINMENU_GAMEPAD_OK) break;
        }
    }

    funkinsave_set_vmu((int8_t)vmu->port, (int8_t)vmu->unit);

    bool create_new = !savemanager->save_only && !vmu->has_savedata;

    if (create_new) {
        messagebox_set_title(savemanager->messagebox, "Creating new save...");
        savemanager->save_only = true;
    } else {
        messagebox_set_title(savemanager->messagebox, savemanager->save_only ? "Storing save..." : "Loading save...");
    }

    messagebox_hide_buttons(savemanager->messagebox);
    messagebox_use_full_title(savemanager->messagebox, true);
    messagebox_set_message(savemanager->messagebox, NULL);
    messagebox_show(savemanager->messagebox, false);

    pvr_context_reset(&pvr_context);
    layout_draw(savemanager->layout, &pvr_context);
    messagebox_draw(savemanager->messagebox, &pvr_context);
    pvrctx_wait_ready();

    messagebox_use_full_title(savemanager->messagebox, false);

    int32_t result;
    if (savemanager->save_only)
        result = funkinsave_write_to_vmu();
    else
        result = funkinsave_read_from_vmu();

    bool ret = savemanager_internal_show_error(savemanager, result);

    if (create_new) savemanager->save_only = false;

    return ret;
}

static bool savemanager_internal_show_error(SaveManager savemanager, int32_t error_code) {
    if (savemanager->save_only) {
        switch (error_code) {
            case 0:
                savemanager_game_withoutsavedata = false;
                return true;
            case 1:
                messagebox_set_message(
                    savemanager->messagebox, "The VMU was removed"
                );
                break;
            case 2:
                messagebox_set_message(
                    savemanager->messagebox, "No enough free space"
                );
                break;
            case 3:
                messagebox_set_message(
                    savemanager->messagebox, "Error while writing the save,\nthe old save could be lost."
                );
                break;
            case 4:
                messagebox_set_message(
                    savemanager->messagebox, "The write could not be completed successfully"
                );
                break;
            case 5:
                messagebox_set_message(
                    savemanager->messagebox, "Can not delete the previous save"
                );
                break;
        }
    } else {
        switch (error_code) {
            case 0:
                // success
                savemanager_game_withoutsavedata = false;
                return true;
            case 1:
                messagebox_set_message(
                    savemanager->messagebox, "The VMU was removed"
                );
                break;
            case 2:
                messagebox_set_message(
                    savemanager->messagebox, "The VMU has changed and the save is missing"
                );
                break;
            case 3:
                messagebox_set_message(
                    savemanager->messagebox, "Error while reading the VMU"
                );
                break;
            case 4:
                messagebox_set_message(
                    savemanager->messagebox, "This save is damaged and can not be loaded"
                );
                break;
            case 5:
                messagebox_set_message(
                    savemanager->messagebox, "The version on the save is different,\nCan not be loaded."
                );
                break;
        }
    }

    messagebox_set_title(savemanager->messagebox, "Error");
    messagebox_set_button_single(savemanager->messagebox, "OK");

    while (true) {
        float elapsed = pvrctx_wait_ready();
        GamepadButtons buttons = gamepad_has_pressed_delayed(savemanager->maple_pad, MAINMENU_GAMEPAD_BUTTONS);

        pvr_context_reset(&pvr_context);
        layout_animate(savemanager->layout, elapsed);
        layout_draw(savemanager->layout, &pvr_context);
        messagebox_animate(savemanager->messagebox, elapsed);
        messagebox_draw(savemanager->messagebox, &pvr_context);

        if (buttons) break;
    }

    return false;
}

static void savemanager_internal_commit_delete(SaveManager savemanager) {
    int selected_index = menu_get_selected_index(savemanager->menu);
    if (selected_index < 0 || selected_index >= menu_get_items_count(savemanager->menu)) {
        // this never should happen
        messagebox_set_message(
            savemanager->messagebox, "The VMU was changed or removed, nothing to do"
        );
        savemanager_internal_show_error(savemanager, -1);
        return;
    }

    VMUInfo* vmu = &savemanager->vmu_array[selected_index];
    if (!vmu->has_savedata) {
        // this never should happen
        messagebox_set_message(savemanager->messagebox, "The VMU was changed, nothing to do");
        savemanager_internal_show_error(savemanager, -1);
        return;
    }

    messagebox_set_title(savemanager->messagebox, "Deleting savedata...");
    messagebox_hide_buttons(savemanager->messagebox);
    messagebox_use_full_title(savemanager->messagebox, true);
    messagebox_set_message(savemanager->messagebox, NULL);
    messagebox_show(savemanager->messagebox, false);

    pvr_context_reset(&pvr_context);
    layout_draw(savemanager->layout, &pvr_context);
    messagebox_draw(savemanager->messagebox, &pvr_context);
    pvrctx_wait_ready();

    messagebox_use_full_title(savemanager->messagebox, false);

    int error_code = funkinsave_delete_from_vmu((int8_t)vmu->port, (int8_t)vmu->unit);

    switch (error_code) {
        case 1:
            messagebox_set_message_formated(
                savemanager->messagebox,
                "There nothing connected on $s", *(textsprite_get_string(savemanager->selected_label))
            );
            break;
        case 2:
            messagebox_set_message(
                savemanager->messagebox,
                "The VMU was removed"
            );
            break;
        case 3:
            messagebox_set_message(
                savemanager->messagebox,
                "The VMU has changed and the save is missing"
            );
            break;
        case 4:
            messagebox_set_message(
                savemanager->messagebox,
                "Failed to delete, the VMU may be corrupted"
            );
            break;
        case 5:
            messagebox_set_message(
                savemanager->messagebox,
                "Operation failed with an unknown error"
            );
            break;
        default:
            // success
            vmu->has_savedata = false;
            return;
    }

    savemanager_internal_show_error(savemanager, -1);
}
