#include "game/settingsmenu.h"

#include <arch/timer.h>

#include "animlist.h"
#include "animsprite.h"
#include "fs.h"
#include "game/common/funkinsave.h"
#include "game/common/menu.h"
#include "game/gameplay/helpers/menumanifest.h"
#include "game/main.h"
#include "game/modding.h"
#include "game/savemanager.h"
#include "gamepad.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "soundplayer.h"
#include "sprite.h"
#include "stringutils.h"
#include "textsprite.h"


typedef struct {
    const char* name;
    const SettingValue value_id;
} SettingOptionListItem;

typedef struct {
    const uint16_t id;
    const char* name;
    const char* description;
    bool description_changed;

    const bool is_bool;
    const bool is_int;
    const bool is_list;

    bool value_bool;
    int32_t value_int;
    int32_t value_list_index;

    const int32_t number_max;
    const int32_t number_min;

    const SettingOptionListItem* list;
    const int32_t list_size;

    const bool hidden;
} SettingOption;

/*
#warning "not implemented"
typedef struct {
    const char* const* name;
    unsigned int scancode;
    char key;
} KeyCode;
typedef void (*KeyboardCallback)(KeyCode keycode, bool state_keydown, uint16_t modKeys);*/


// static const char* SETTINGSMENU_KEY_NONE = "(none)";
// static const char* SETTINGSMENU_KEY_BIND = "(waiting)";

static const uint16_t SETTINGSMENU_DELAY_MS = 250;
static const char* SETTINGSMENU_MODDING_SCRIPT = "/assets/common/data/scripts/settigsmenu.lua";
static const char* SETTINGSMENU_MODDING_MENU = "/assets/common/data/menus/settigsmenu-main.json";
#ifndef _arch_dreamcast
static const char* SETTINGSMENU_LAYOUT_BINDS_GAMEPLAY = "/assets/common/image/settings-menu/binds_gameplay.xml";
static const char* SETTINGSMENU_LAYOUT_BINDS_MENUS = "/assets/common/image/settings-menu/binds_menus.xml";
#endif
static const char* SETTINGSMENU_LAYOUT_MAIN = "/assets/common/image/settings-menu/main.xml";
static const char* SETTINGSMENU_LAYOUT_COMMON = "/assets/common/image/settings-menu/common.xml";
static const char* SETTINGSMENU_LAYOUT_SAVEDATA = "/assets/common/image/settings-menu/savedata.xml";
#ifndef _arch_dreamcast
static const char* SETTINGSMENU_LAYOUT_BINDS_GAMEPLAY_DREAMCAST = "/assets/common/image/settings-menu/binds_gameplay~dreamcast.xml";
static const char* SETTINGSMENU_LAYOUT_BINDS_MENUS_DREAMCAST = "/assets/common/image/settings-menu/binds_menus~dreamcast.xml";
#endif
static const char* SETTINGSMENU_LAYOUT_MAIN_DREAMCAST = "/assets/common/image/settings-menu/main~dreamcast.xml";
static const char* SETTINGSMENU_LAYOUT_COMMON_DREAMCAST = "/assets/common/image/settings-menu/common~dreamcast.xml";
static const char* SETTINGSMENU_LAYOUT_SAVEDATA_DREAMCAST = "/assets/common/image/settings-menu/savedata~dreamcast.xml";

static struct MenuManifest_s SETTINGSMENU_MENU = {
    .parameters = {
        .suffix_selected = NULL,  // unused
        .suffix_idle = NULL,      // unused
        .suffix_choosen = NULL,   // unused
        .suffix_discarded = NULL, // unused
        .suffix_rollback = NULL,  // unused
        .suffix_in = NULL,        // unused
        .suffix_out = NULL,       // unused

        .atlas = NULL,
        .animlist = "/assets/common/anims/settings-menu.xml",

        .anim_selected = "menu_item_selected",
        .anim_idle = "menu_item_idle",
        .anim_choosen = NULL,   // unused
        .anim_discarded = NULL, // unused
        .anim_rollback = NULL,  // unused
        .anim_in = NULL,        // unused
        .anim_out = NULL,       // unused

        .anim_transition_in_delay = 0.0f,  // unused
        .anim_transition_out_delay = 0.0f, // unused

        .font = "/assets/common/font/Alphabet.xml",
        .font_glyph_suffix = "bold",
        .font_color_by_addition = false, // unused
        .font_size = 46.0f,
        .font_color = 0xFFFFFF,
        .font_border_color = 0x00,     // unused
        .font_border_size = FLOAT_NaN, // unused

        .is_sparse = false, // unused
        .is_vertical = true,
        .is_per_page = false,
        .static_index = 1,

        .items_align = ALIGN_START,
        .items_gap = 38.0f,
        .items_dimmen = 0.0f,                     // unused
        .texture_scale = FLOAT_NaN,               // unused
        .enable_horizontal_text_correction = true // unused
    },
    .items = (MenuManifestItem[]){
#ifndef _arch_dreamcast
        {
            .name = "keyboard-bindings-gameplay",
            .text = "KEYBOARD BINDINGS IN GAMEPLAY",                          // unused
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,                                           // unused
            .anim_idle = NULL,                                                // unused
            .anim_rollback = NULL,                                            // unused
            .anim_in = NULL,                                                  // unused
            .anim_out = NULL,                                                 // unused
            .hidden = false,
            .description = NULL // unused
        },
        {
            .name = "keyboard-bindings-menu",
            .text = "KEYBOARD BINDINGS IN MENUS",                             // unused
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,                                           // unused
            .anim_idle = NULL,                                                // unused
            .anim_rollback = NULL,                                            // unused
            .anim_in = NULL,                                                  // unused
            .anim_out = NULL,                                                 // unused
            .hidden = false,
            .description = NULL // unused
        },
#endif
        {
            .name = "gameplay-settings",
            .text = "GAMEPLAY SETTINGS",                                      // unused
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,                                           // unused
            .anim_idle = NULL,                                                // unused
            .anim_rollback = NULL,                                            // unused
            .anim_in = NULL,                                                  // unused
            .anim_out = NULL,                                                 // unused
            .hidden = false,
            .description = NULL // unused
        },
#ifndef _arch_dreamcast
        {
            .name = "miscellaneous",
            .text = "MISCELLANEOUS",                                          // unused
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,                                           // unused
            .anim_idle = NULL,                                                // unused
            .anim_rollback = NULL,                                            // unused
            .anim_in = NULL,                                                  // unused
            .anim_out = NULL,                                                 // unused
            .hidden = false,
            .description = NULL // unused
        },
#endif
        {
            .name = "savedata",
            .text = "SAVEDATA",                                               // unused
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,                                           // unused
            .anim_idle = NULL,                                                // unused
            .anim_rollback = NULL,                                            // unused
            .anim_in = NULL,                                                  // unused
            .anim_out = NULL,                                                 // unused
            .hidden = false,
            .description = NULL // unused
        },
        {
            .name = "return-main-menu",
            .text = "RETURN TO THE MAIN MENU",                                // unused
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,                                           // unused
            .anim_idle = NULL,                                                // unused
            .anim_rollback = NULL,                                            // unused
            .anim_in = NULL,                                                  // unused
            .anim_out = NULL,                                                 // unused
            .hidden = false,
            .description = NULL // unused
        },
    },
#ifndef _arch_dreamcast
    .items_size = 6
#else
    .items_size = 3
#endif
};
static struct MenuManifest_s SETTINGSMENU_SAVEDATA = {
    .parameters = {
        .suffix_selected = NULL,  // unused
        .suffix_idle = NULL,      // unused
        .suffix_choosen = NULL,   // unused
        .suffix_discarded = NULL, // unused
        .suffix_rollback = NULL,  // unused
        .suffix_in = NULL,        // unused
        .suffix_out = NULL,       // unused

        .atlas = NULL,
        .animlist = "/assets/common/anims/settings-menu.xml",

        .anim_selected = "menu_item_selected",
        .anim_idle = "menu_item_idle",
        .anim_choosen = NULL,   // unused
        .anim_discarded = NULL, // unused
        .anim_rollback = NULL,  // unused
        .anim_in = NULL,        // unused
        .anim_out = NULL,       // unused

        .anim_transition_in_delay = 0,  // unused
        .anim_transition_out_delay = 0, // unused

        .font = "/assets/common/font/Alphabet.xml",
        .font_glyph_suffix = "bold",
        .font_color_by_addition = false, // unused
        .font_size = 46.0f,
        .font_color = 0xFFFFFF,
        .font_border_color = 0x00,     // unused
        .font_border_size = FLOAT_NaN, // unused

        .is_sparse = false, // unused
        .is_vertical = true,
        .is_per_page = false,
        .static_index = 1,

        .items_align = ALIGN_START,
        .items_gap = 38.0f,
        .items_dimmen = 0.0f,                     // unused
        .texture_scale = FLOAT_NaN,               // unused
        .enable_horizontal_text_correction = true // unused
    },
    .items = (MenuManifestItem[]){
        {
            .name = "load-only",
            .text = "LOAD SAVEDATA",                                          // unused
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,                                           // unused
            .anim_idle = NULL,                                                // unused
            .anim_rollback = NULL,                                            // unused
            .anim_in = NULL,                                                  // unused
            .anim_out = NULL,                                                 // unused
            .hidden = false,
            .description = NULL // unused
        },
        {
            .name = "save-or-delete",
            .text = "SAVE OR DELETE FROM VMU",                                // unused
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,                                           // unused
            .anim_idle = NULL,                                                // unused
            .anim_rollback = NULL,                                            // unused
            .anim_in = NULL,                                                  // unused
            .anim_out = NULL,                                                 // unused
            .hidden = false,
            .description = NULL // unused
        },
        {
            .name = "return",
            .text = "RETURN TO THE SETTINGS MENU",                            // unused
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,                                           // unused
            .anim_idle = NULL,                                                // unused
            .anim_rollback = NULL,                                            // unused
            .anim_in = NULL,                                                  // unused
            .anim_out = NULL,                                                 // unused
            .hidden = false,
            .description = NULL // unused
        },
    },
    .items_size = 3
};
static struct MenuManifest_s SETTINGSMENU_MENU_COMMON = {
    .parameters = {
        .suffix_selected = NULL,  // unused
        .suffix_idle = NULL,      // unused
        .suffix_choosen = NULL,   // unused
        .suffix_discarded = NULL, // unused
        .suffix_rollback = NULL,  // unused
        .suffix_in = NULL,        // unused
        .suffix_out = NULL,       // unused

        .atlas = NULL,
        .animlist = "/assets/common/anims/settings-menu.xml",

        .anim_selected = NULL,
        .anim_idle = NULL,
        .anim_choosen = NULL,   // unused
        .anim_discarded = NULL, // unused
        .anim_rollback = NULL,  // unused
        .anim_in = NULL,        // unused
        .anim_out = NULL,       // unused

        .anim_transition_in_delay = 0.0f,  // unused
        .anim_transition_out_delay = 0.0f, // unused

        .font = "/assets/common/font/pixel.otf",
        .font_glyph_suffix = NULL,       // unused
        .font_color_by_addition = false, // unused
        .font_size = 28.0f,
        .font_color = 0xFFFFFF,
        .font_border_color = 0x000000FF, // unused
        .font_border_size = 4.0f,        // unused

        .is_sparse = false, // unused
        .is_vertical = true,
        .is_per_page = true,
        .static_index = 0,

        .items_align = ALIGN_START,
        .items_gap = 58.0f,
        .items_dimmen = 0.0f,                     // unused
        .texture_scale = FLOAT_NaN,               // unused
        .enable_horizontal_text_correction = true // unused
    },
    .items = NULL,
    .items_size = 0
};


// static uint64_t settingsmenu_next_pressed_timestamp = 0;
// static KeyCode settingsmenu_last_detected_keycode = NULL;
// static KeyboardCallback settingsmenu_previous_key_callback = NULL;

static Menu settingsmenu_current_menu = NULL;
static const SettingOption* settingsmenu_current_setting_options = NULL;
static bool settingsmenu_current_menu_choosen = false;
static char* settingsmenu_current_menu_choosen_custom = NULL;
static bool settingsmenu_is_running = false;


static int32_t settingsmenu_in_common_menu(const char** title, Layout layout, Gamepad gamepad, Menu menu, const SettingOption options[], Modding modding);
static void settingsmenu_in_save_settings(Gamepad gamepad, Modding modding);
static void settingsmenu_in_gameplay_settings(Gamepad gamepad, Modding modding);
static void settingsmenu_show_common(const char** title, Gamepad gamepad, SettingOption options[], const size_t options_count, Modding modding);
static int32_t settingsmenu_in_common_change_number(Layout layout, Gamepad gamepad, int32_t min, int32_t max, int32_t value);
static bool settingsmenu_in_common_change_bool(Layout layout, Gamepad gamepad, bool value);
static int32_t settingsmenu_in_common_change_list(Layout layout, Gamepad gamepad, const SettingOptionListItem list[], int32_t list_size, int32_t index);
static void settingsmenu_internal_save_option(const SettingOption* option);
static void settingsmenu_internal_load_option(SettingOption* option);
static bool settingsmenu_internal_handle_option(void* obj, const char* option_name);


void settingsmenu_main() {
    static const char* title = "Settings";
    static const SettingOption main_options_help[] = {
#ifndef _arch_dreamcast
        {
            .name = "keyboard-bindings-gameplay",
            .description = "Change the assigned key for each strum.\nDirectonial keys are always assigned",
        },
        {
            .name = "keyboard-bindings-menu",
            .description = "Change the keys assigned to navigate between menus.\nDirectonial keys are always assigned",
        },
#endif
        {
            .name = "gameplay-settings",
            .description = "Gameplay settings like ghost-tapping and input offset",
        },
#ifndef _arch_dreamcast
        {
            .name = "miscellaneous",
            .description = "Specific engine settings",
        },
#endif
        {
            .name = "savedata",
            .description = "Load, save or delete savedata.\nAlso sets what VMU should be used",
            .hidden = false,
        },
        {
            .name = "return-main-menu",
            .description = "Returns back to the main menu.\n¡Settings are automatically saved!",
        }
    };
    static const size_t main_options_help_length = sizeof(main_options_help) / sizeof(SettingOption);

    if (settingsmenu_is_running) {
        logger_error("settingsmenu_main() is already running, only a single instance is allowed");
        return;
    }

    FontHolder submenus_font = NULL;
    if (fs_file_exists(SETTINGSMENU_MENU_COMMON.parameters.font)) {
        // little improvement, keep loaded the "pixel.otf" font to improve loading times
        submenus_font = fontholder_init(SETTINGSMENU_MENU_COMMON.parameters.font, NULL, false);
    }

    AnimList animlist = animlist_init("/assets/common/anims/settings-menu.xml");
    AnimSprite anim_binding, anim_binding_rollback;

    if (animlist) {
        anim_binding = animsprite_init_from_animlist(animlist, "binding");
        anim_binding_rollback = animsprite_init_from_animlist(animlist, "binding_rollback");
        animlist_destroy(&animlist);
    } else {
        anim_binding = anim_binding_rollback = NULL;
    }

    Layout layout = layout_init(pvr_context_is_widescreen() ? SETTINGSMENU_LAYOUT_MAIN : SETTINGSMENU_LAYOUT_MAIN_DREAMCAST);
    if (!layout) {
        logger_error("settingsmenu_main() can not load the layout");
        return;
    }

    SETTINGSMENU_MENU.parameters.is_vertical = layout_get_attached_value_boolean(
        layout, "menu_isVertical", SETTINGSMENU_MENU.parameters.is_vertical
    );
    SETTINGSMENU_MENU.parameters.font_size = layout_get_attached_value_as_float(
        layout, "menu_fontSize", SETTINGSMENU_MENU.parameters.font_size
    );
    SETTINGSMENU_MENU.parameters.items_dimmen = layout_get_attached_value_as_float(
        layout, "menu_itemDimmen", SETTINGSMENU_MENU.parameters.items_dimmen
    );
    SETTINGSMENU_MENU.parameters.items_dimmen = layout_get_attached_value_as_float(
        layout, "menu_itemScale", SETTINGSMENU_MENU.parameters.texture_scale
    );
    SETTINGSMENU_MENU.parameters.items_gap = layout_get_attached_value_as_float(
        layout, "menu_itemGap", SETTINGSMENU_MENU.parameters.items_gap
    );
    LayoutPlaceholder* menu_placeholder = layout_get_placeholder(layout, "menu");
    if (!menu_placeholder) {
        logger_error("Missing menu placeholder");
        assert(menu_placeholder);
    }

    SettingOption* options_help = (SettingOption*)main_options_help;

    // load custom menumanifest if exists
    MenuManifest menumanifest = main_helper_init_menumanifest_for_dreamcast(SETTINGSMENU_MODDING_MENU);
    if (menumanifest) {
        // since a custom menu was provided, remap option descriptions
        options_help = malloc_for_array(SettingOption, menumanifest->items_size);

        for (int32_t i = 0; i < menumanifest->items_size; i++) {
            options_help[i].description = menumanifest->items[i].description;

            if (options_help[i].description) {
                options_help[i].description_changed = true;
                continue;
            }

            for (size_t j = 0; j < main_options_help_length; j++) {
                if (string_equals(main_options_help[i].name, menumanifest->items[i].name)) {
                    options_help[i].description = main_options_help[i].description;
                    options_help[i].description_changed = false;
                    break;
                }
            }
        }
    } else {
        menumanifest = &SETTINGSMENU_MENU;
    }

    Menu menu = menu_init(
        menumanifest,
        menu_placeholder->x, menu_placeholder->y, menu_placeholder->z,
        menu_placeholder->width, menu_placeholder->height
    );
    menu_placeholder->vertex = menu_get_drawable(menu);

    const char* backgroud_music_filename = layout_get_attached_value_string(
        layout, "backgroud_music_filename", NULL
    );
    float backgroud_music_volume = (float)layout_get_attached_value_double(
        layout, "backgroud_music_volume", 0.5
    );

    SoundPlayer bg_music = NULL;
    if (backgroud_music_filename) bg_music = soundplayer_init(backgroud_music_filename);
    if (bg_music) soundplayer_set_volume(bg_music, backgroud_music_volume);

    /*#warning "not implemented
    KOS_MAPLE_KEYBOARD.delegate_callback = settingsmenu_internal_key_callback;*/

    Gamepad gamepad = gamepad_init(-1);
    gamepad_set_buttons_delay(gamepad, 75);
    gamepad_clear_buttons(gamepad);

    if (bg_music) {
        if (background_menu_music) soundplayer_pause(background_menu_music);
        soundplayer_loop_enable(bg_music, true);
        soundplayer_play(bg_music);
    } else if (background_menu_music) {
        if (string_is_empty(backgroud_music_filename) && (backgroud_music_volume <= 0.0f || math2d_is_float_NaN(backgroud_music_volume)))
            soundplayer_pause(background_menu_music);
        else
            soundplayer_set_volume(background_menu_music, 0.5f);
    }

    settingsmenu_current_menu = NULL;
    settingsmenu_current_setting_options = NULL;
    settingsmenu_current_menu_choosen = false;
    settingsmenu_current_menu_choosen_custom = NULL;
    settingsmenu_is_running = true;

    Modding modding = modding_init(layout, SETTINGSMENU_MODDING_SCRIPT);
    modding->callback_option = settingsmenu_internal_handle_option;
    modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN, ModdingValueType_string);

    while (!modding->has_exit) {
        int32_t selected_index = settingsmenu_in_common_menu(&title, layout, gamepad, menu, options_help, modding);
        const char* selected_name = selected_index < 0 ? NULL : menumanifest->items[selected_index].name;

#ifndef _arch_dreamcast
        if (string_equals(selected_name, "keyboard-bindings-gameplay")) {
            settingsmenu_in_gameplay_binding(anim_binding, anim_binding_rollback);
            continue;
        }
        if (string_equals(selected_name, "keyboard-bindings-menu")) {
            settingsmenu_in_menus_binding(anim_binding, anim_binding_rollback);
            continue;
        }
#endif
        if (string_equals(selected_name, "gameplay-settings")) {
            settingsmenu_in_gameplay_settings(gamepad, modding);
            continue;
        }
#ifndef _arch_dreamcast
        if (string_equals(selected_name, "miscellaneous")) {
            settingsmenu_in_misc_settings(gamepad, modding);
            continue;
        }
#endif
        if (string_equals(selected_name, "savedata")) {
            settingsmenu_in_save_settings(gamepad, modding);
            continue;
        }
        if (string_equals(selected_name, "return-main-menu")) {
            break;
        }

        // custom option selected
        if (settingsmenu_current_menu_choosen_custom == NULL) break;
        modding_helper_notify_handle_custom_option(modding, settingsmenu_current_menu_choosen_custom);
        free_chk(settingsmenu_current_menu_choosen_custom);
        settingsmenu_current_menu_choosen_custom = NULL;
    }

    modding_helper_notify_exit2(modding);

    animsprite_destroy(&anim_binding);
    animsprite_destroy(&anim_binding_rollback);
    menu_destroy(&menu);
    layout_destroy(&layout);
    modding_destroy(&modding);
    gamepad_destroy(&gamepad);
    if (submenus_font) fontholder_destroy(&submenus_font);

    settingsmenu_is_running = false;

    if (options_help != main_options_help) free_chk(options_help);
    if (menumanifest != &SETTINGSMENU_MENU) menumanifest_destroy(&menumanifest);

    // reload settings
    SETTINGS.reload_settings();

    /*#warning "not implemented
    KOS_MAPLE_KEYBOARD.delegate_callback = NULL;*/

    if (bg_music) {
        soundplayer_destroy(&bg_music);
        if (background_menu_music) soundplayer_play(background_menu_music);
    } else if (background_menu_music) {
        soundplayer_set_volume(background_menu_music, 1.0);
        soundplayer_play(background_menu_music);
    }
}

static void settingsmenu_in_save_settings(Gamepad gamepad, Modding modding) {
    const char* title = "Savedata";
    SettingOption options_help[] = {
        {
            .name = "load-only",
            .description = "Load savedata from another VMU.\n"
                           "Any future saves will be stored in the selected VMU.",
            .hidden = false,
        },
        {
            .name = "save-or-delete",
            .description = "Save the current loaded savedata in another VMU.\n"
                           "Allows override or delete any other savedata.\n"
                           "Any future saves will be stored in the lastest used VMU for save.",
            .hidden = false,
        },
        {
            .name = "return",
            .description = "Go back to settings menu",
            .hidden = false,
        },
    };

    Layout layout = layout_init(pvr_context_is_widescreen() ? SETTINGSMENU_LAYOUT_SAVEDATA : SETTINGSMENU_LAYOUT_SAVEDATA_DREAMCAST);
    if (!layout) {
        logger_error("settingsmenu_in_save_settings() can not load the layout");
        return;
    }

    SETTINGSMENU_SAVEDATA.parameters.is_vertical = layout_get_attached_value_boolean(
        layout, "menu_isVertical", SETTINGSMENU_SAVEDATA.parameters.is_vertical
    );
    SETTINGSMENU_SAVEDATA.parameters.font_size = layout_get_attached_value_as_float(
        layout, "menu_fontSize", SETTINGSMENU_SAVEDATA.parameters.font_size
    );
    SETTINGSMENU_SAVEDATA.parameters.items_dimmen = layout_get_attached_value_as_float(
        layout, "menu_itemDimmen", SETTINGSMENU_SAVEDATA.parameters.items_dimmen
    );
    SETTINGSMENU_SAVEDATA.parameters.items_dimmen = layout_get_attached_value_as_float(
        layout, "menu_itemScale", SETTINGSMENU_SAVEDATA.parameters.texture_scale
    );
    SETTINGSMENU_SAVEDATA.parameters.items_gap = layout_get_attached_value_as_float(
        layout, "menu_itemGap", SETTINGSMENU_SAVEDATA.parameters.items_gap
    );

    LayoutPlaceholder* menu_placeholder = layout_get_placeholder(layout, "menu");
    assert(menu_placeholder);

    MenuManifest menumanifest = &SETTINGSMENU_SAVEDATA;
    Menu menu = menu_init(
        menumanifest,
        menu_placeholder->x, menu_placeholder->y, menu_placeholder->z,
        menu_placeholder->width, menu_placeholder->height
    );
    menu_placeholder->vertex = menu_get_drawable(menu);

    // initialize with default values
    SaveManager savemanager = savemanager_init(false, 0);

    while (!modding->has_exit) {
        int selected_index = settingsmenu_in_common_menu(&title, layout, gamepad, menu, options_help, modding);
        const char* selected_name = selected_index < 0 ? NULL : menumanifest->items[selected_index].name;
        bool save_only;

        if (string_equals(selected_name, "load-only")) {
            save_only = false;
        } else if (string_equals(selected_name, "save-or-delete")) {
            save_only = true;
        } else if (string_equals(selected_name, "return")) {
            goto L_prepare_return;
        } else {
            // custom option selected
            if (settingsmenu_current_menu_choosen_custom == NULL) goto L_prepare_return;
            modding_helper_notify_handle_custom_option(modding, settingsmenu_current_menu_choosen_custom);
            free_chk(settingsmenu_current_menu_choosen_custom);
            settingsmenu_current_menu_choosen_custom = NULL;
            continue;
        }

        // only allow delete when saving
        savemanager_change_actions(savemanager, save_only, save_only, true);
        savemanager_show(savemanager);
    }

L_prepare_return:
    menu_destroy(&menu);
    layout_destroy(&layout);
    savemanager_destroy(&savemanager);
}

#ifndef _arch_dreamcast
static void settingsmenu_in_gameplay_binding(AnimSprite anim_binding, AnimSprite anim_binding_rollback) {
    Layout layout = layout_init(pvr_context_is_widescreen() ? SETTINGSMENU_LAYOUT_BINDS_GAMEPLAY : SETTINGSMENU_LAYOUT_BINDS_GAMEPLAY_DREAMCAST);

    if (!layout) {
        logger_error("settingsmenu_in_gameplay_binding() can not load the layout");
        return;
    }

    // binding colors
    uint32_t color_bg_key = layout_get_attached_value_hex(layout, "color_bg_key", 0x000000FF);
    uint32_t color_bg_key_selected = layout_get_attached_value_hex(layout, "color_bg_key_selected", 0xFF0000FF);
    uint32_t color_bg_key_active = layout_get_attached_value_hex(layout, "color_bg_key_active", 0x00FF00FF);

    // aquire labels
    TextSprite labels[] = {
        layout_get_textsprite(layout, "label-left0"),
        layout_get_textsprite(layout, "label-down0"),
        layout_get_textsprite(layout, "label-up0"),
        layout_get_textsprite(layout, "label-right0"),

        layout_get_textsprite(layout, "label-left1"),
        layout_get_textsprite(layout, "label-down1"),
        layout_get_textsprite(layout, "label-up1"),
        layout_get_textsprite(layout, "label-right1"),

        layout_get_textsprite(layout, "label-left2"),
        layout_get_textsprite(layout, "label-down2"),
        layout_get_textsprite(layout, "label-up2"),
        layout_get_textsprite(layout, "label-right2"),

        layout_get_textsprite(layout, "label-diamond")
    };

    // adquire backgrounds
    Sprite backs[] = {
        layout_get_sprite(layout, "back-left0"),
        layout_get_sprite(layout, "back-down0"),
        layout_get_sprite(layout, "back-up0"),
        layout_get_sprite(layout, "back-right0"),

        layout_get_sprite(layout, "back-left1"),
        layout_get_sprite(layout, "back-down1"),
        layout_get_sprite(layout, "back-up1"),
        layout_get_sprite(layout, "back-right1"),

        layout_get_sprite(layout, "back-left2"),
        layout_get_sprite(layout, "back-down2"),
        layout_get_sprite(layout, "back-up2"),
        layout_get_sprite(layout, "back-right2"),

        layout_get_sprite(layout, "back-diamond")
    };

    // keys
    KeyCode keycodes[] = {
        KEYCODE_KeyF,
        KEYCODE_KeyG,
        KEYCODE_KeyJ,
        KEYCODE_KeyH,
        KEYCODE_NULL,
        KEYCODE_NULL,
        KEYCODE_NULL,
        KEYCODE_NULL,
        KEYCODE_NULL,
        KEYCODE_NULL,
        KEYCODE_NULL,
        KEYCODE_NULL,

        KEYCODE_SPACE
    };

    static const int32_t binds_count = 13;
    static const GamepadButtons ini_entries[] = {
        GAMEPAD_DPAD_LEFT, GAMEPAD_DPAD_DOWN, GAMEPAD_DPAD_UP, GAMEPAD_DPAD_RIGHT,
        GAMEPAD_DPAD2_LEFT, GAMEPAD_DPAD2_DOWN, GAMEPAD_DPAD2_UP, GAMEPAD_DPAD2_RIGHT,
        GAMEPAD_DPAD3_LEFT, GAMEPAD_DPAD3_DOWN, GAMEPAD_DPAD3_UP, GAMEPAD_DPAD3_RIGHT,
        GAMEPAD_D // diamond strum line
    };

    // load gameplay bindings
    for (int32_t i = 0; i < binds_count; i++) {
        keycodes[i] = settingsmenu_internal_load_key(labels[i], ini_entries[i], keycodes[i]);
    }

    int32_t row = 0, column = 0, selected_index = 0;
    bool is_binding = false;

    // select first holder
    sprite_set_vertex_color_rgba8(backs[selected_index], color_bg_key_selected);

    // anti-bounce from parent menu
    settingsmenu_last_detected_keycode = NULL;
    settingsmenu_next_pressed_timestamp = timer_ms_gettime64() + SETTINGSMENU_DELAY_MS;

    while (true) {
        float elapsed = pvrctx_wait_ready();
        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        if (is_binding && anim_binding) {
            animsprite_animate(anim_binding, elapsed);
            animsprite_update_sprite(anim_binding, backs[selected_index], true);
        }

        GamepadButtons pressed = settingsmenu_internal_read_keyboard();

        if (is_binding) {
            KeyCode keycode = settingsmenu_last_detected_keycode;

            if (pressed != GAMEPAD_BACK) {
                if (pressed == GAMEPAD_START) {
                    keycodes[selected_index] = NULL; // clear binding
                } else if (keycode.key == KEYCODE_NULL.key) {
                    continue; // keep waiting
                } else {
                    keycodes[selected_index] = keycode;
                }
            }

            // update holder
            settingsmenu_internal_update_holder(
                backs, labels, keycodes, selected_index,
                anim_binding_rollback, true, color_bg_key_selected
            );
            is_binding = false;
            continue;
        }

        if (pressed == 0x00) {
            continue;
        } else if (pressed == GAMEPAD_BACK) {
            break;
        } else if (pressed == GAMEPAD_START) {
            is_binding = true;
            settingsmenu_last_detected_keycode = KEYCODE_NULL;
            settingsmenu_internal_update_holder(
                backs, labels, keycodes, selected_index,
                anim_binding, false, color_bg_key_active
            );
            textsprite_set_text_intern(labels[selected_index], true, &SETTINGSMENU_KEY_BIND);
            if (anim_binding) animsprite_restart(anim_binding);
            continue;
        }

        // calculate movement
        int32_t new_row = row, new_column = column, new_index = selected_index;

        if (pressed == GAMEPAD_DPAD_LEFT) new_column--;
        if (pressed == GAMEPAD_DPAD_RIGHT) new_column++;
        if (pressed == GAMEPAD_DPAD_UP) new_row--;
        if (pressed == GAMEPAD_DPAD_DOWN) new_row++;

        // unselect current holder
        settingsmenu_internal_update_holder(
            backs, labels, keycodes, selected_index,
            NULL, false, color_bg_key
        );

        if (new_row >= 0 && new_row < 4) row = new_row;
        if (new_column >= 0 && new_column < 4) column = new_column;

        if (new_row >= 4)
            selected_index = binds_count - 1;
        else
            selected_index = column + (row * 4);

        if (selected_index >= binds_count)
            selected_index = binds_count - 1;

        // select the new holder
        settingsmenu_internal_update_holder(
            backs, labels, keycodes, selected_index,
            NULL, false, color_bg_key_selected
        );
    }

    // save bindings
    for (int32_t i = 0; i < binds_count; i++) {
        settingsmenu_internal_save_key(ini_entries[i], keycodes[i]);
    }

    layout_destroy(&layout);
    KOS_MAPLE_KEYBOARD.reloadMappings();
}

static void settingsmenu_in_menus_binding(AnimSprite anim_binding, AnimSprite anim_binding_rollback) {
    Layout layout = layout_init(pvr_context_is_widescreen() ? SETTINGSMENU_LAYOUT_BINDS_MENUS : SETTINGSMENU_LAYOUT_BINDS_MENUS_DREAMCAST);

    if (!layout) {
        logger_error("settingsmenu_in_menus_binding() can not load the layout");
        return;
    }

    // binding colors
    uint32_t color_bg_key = layout_get_attached_value_hex(layout, "color_bg_key", 0x000000FF);
    uint32_t color_bg_key_selected = layout_get_attached_value_hex(layout, "color_bg_key_selected", 0xFF0000FF);
    uint32_t color_bg_key_active = layout_get_attached_value_hex(layout, "color_bg_key_active", 0x00FF00FF);

    // aquire labels
    TextSprite labels[] = {
        layout_get_textsprite(layout, "label-accept"),
        layout_get_textsprite(layout, "label-selectorLeft"),
        layout_get_textsprite(layout, "label-selectorRight"),
        layout_get_textsprite(layout, "label-alternativeTracks")
    };

    // adquire backgrounds
    Sprite backs[] = {
        layout_get_sprite(layout, "back-accept"),
        layout_get_sprite(layout, "back-selectorLeft"),
        layout_get_sprite(layout, "back-selectorRight"),
        layout_get_sprite(layout, "back-alternativeTracks")
    };

    static const binds_count = 4;
    static const GamepadButtons ini_entries[] = {
        "menuAccept", "menuSelectorLeft", "menuSelectorRight", "menuAlternativeTracks"
    };

    // key + scancodes
    KeyCode keycodes[] = {
        KeyCode_KeyA,
        KeyCode_KeyQ,
        KeyCode_KeyW,
        KeyCode_KeyS
    };

    // load gameplay bindings
    for (int32_t i = 0; i < binds_count; i++) {
        keycodes[i] = settingsmenu_internal_load_key(labels[i], ini_entries[i], keycodes[i]);
    }

    int32_t selected_index = 0;
    bool is_binding = false;

    // select first holder
    sprite_set_vertex_color_rgba8(backs[selected_index], color_bg_key_selected);

    // anti-bounce from parent menu
    settingsmenu_last_detected_keycode = NULL;
    settingsmenu_next_pressed_timestamp = timer_ms_gettime64() + SETTINGSMENU_DELAY_MS;

    while (true) {
        float elapsed = pvrctx_wait_ready();
        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        if (is_binding && anim_binding) {
            animsprite_animate(anim_binding, elapsed);
            animsprite_update_sprite(anim_binding, backs[selected_index], true);
        }

        GamepadButtons pressed = settingsmenu_internal_read_keyboard();

        if (is_binding) {
            KeyCode keycode = settingsmenu_last_detected_keycode;

            if (pressed != GAMEPAD_BACK) {
                if (pressed == GAMEPAD_START) {
                    keycodes[selected_index] = NULL; // clear binding
                } else if (keycode.key == KEYCODE_NULL.key) {
                    continue; // keep waiting
                } else {
                    keycodes[selected_index] = keycode;
                }
            }

            // update holder
            settingsmenu_internal_update_holder(
                backs, labels, keycodes, selected_index,
                anim_binding_rollback, true, color_bg_key_selected
            );
            is_binding = false;
            continue;
        }

        if (pressed == 0x00) {
            continue;
        } else if (pressed == GAMEPAD_BACK) {
            break;
        } else if (pressed == GAMEPAD_START) {
            is_binding = true;
            settingsmenu_last_detected_keycode = NULL;
            settingsmenu_internal_update_holder(
                backs, labels, keycodes, selected_index,
                anim_binding, false, color_bg_key_active
            );
            textsprite_set_text_intern(labels[selected_index], true, &SETTINGSMENU_KEY_BIND);
            if (anim_binding) animsprite_restart(anim_binding);
            continue;
        }

        // calculate movement
        int32_t new_index = selected_index;

        if (pressed == GAMEPAD_DPAD_UP) new_index--;
        if (pressed == GAMEPAD_DPAD_DOWN) new_index++;

        if (new_index < 0 || new_index >= binds_count) {
            continue;
        }

        // unselect current holder
        settingsmenu_internal_update_holder(
            backs, labels, keycodes, selected_index,
            NULL, false, color_bg_key
        );

        // select the new holder
        settingsmenu_internal_update_holder(
            backs, labels, keycodes, new_index,
            NULL, false, color_bg_key_selected
        );
        selected_index = new_index;
    }

    // save bindings
    for (int32_t i = 0; i < binds_count; i++) {
        settingsmenu_internal_save_key(ini_entries[i], keycodes[i]);
    }
    layout_destroy(&layout);

    KOS_MAPLE_KEYBOARD.reloadMappings();
}
#endif

static int32_t settingsmenu_in_common_menu(const char** title, Layout layout, Gamepad gamepad, Menu menu, const SettingOption options[], Modding modding) {
    TextSprite hint = layout_get_textsprite(layout, "hint");
    gamepad_set_buttons_delay(gamepad, SETTINGSMENU_DELAY_MS);

    settingsmenu_current_menu = modding->native_menu = modding->active_menu = menu;
    settingsmenu_current_setting_options = options;
    settingsmenu_current_menu_choosen = false;

    layout_trigger_any(layout, NULL);
    modding_helper_notify_event(modding, *title);

    int32_t last_selected_index = -1;
    int32_t selected_index = menu_get_selected_index(menu);
    if (selected_index >= 0 && selected_index < menu_get_items_count(menu) && hint) {
        textsprite_set_text_intern(hint, true, &options[selected_index].description);
        last_selected_index = selected_index;
        main_helper_trigger_action_menu2(layout, &SETTINGSMENU_MENU_COMMON, selected_index, *title, true, false);
        modding_helper_notify_option(modding, true);
    }

    int32_t option = -1;
    gamepad_clear_buttons(gamepad);

    while (!modding->has_exit) {
        if (settingsmenu_current_menu_choosen) {
            if (settingsmenu_current_menu_choosen_custom != NULL) option = menu_get_selected_index(menu);
            break;
        }

        float elapsed = pvrctx_wait_ready();

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        GamepadButtons buttons = gamepad_has_pressed_delayed(
            gamepad,
            GAMEPAD_AD |
                GAMEPAD_START | GAMEPAD_BACK |
                GAMEPAD_B | GAMEPAD_A
        );

        ModdingHelperResult res = modding_helper_handle_custom_menu(modding, gamepad, elapsed);
        if (res != ModdingHelperResult_CONTINUE) break;
        if (modding->has_halt || menu != modding->active_menu) continue;

        if ((buttons & (GAMEPAD_BACK | GAMEPAD_B)) && !modding_helper_notify_back(modding)) {
            break;
        } else if ((buttons & (GAMEPAD_START | GAMEPAD_A)) && !modding_helper_notify_option(modding, false)) {
            option = menu_get_selected_index(menu);
            break;
        } else if (buttons & GAMEPAD_AD_UP) {
            menu_select_vertical(menu, -1);
        } else if (buttons & GAMEPAD_AD_DOWN) {
            menu_select_vertical(menu, 1);
        } else if (buttons & GAMEPAD_AD_LEFT) {
            menu_select_horizontal(menu, -1);
        } else if (buttons & GAMEPAD_AD_RIGHT) {
            menu_select_horizontal(menu, 1);
        } else {
            continue;
        }

        selected_index = menu_get_selected_index(menu);
        if (selected_index >= 0 && selected_index < menu_get_items_count(menu) && hint) {
            textsprite_set_text_intern(hint, true, &options[selected_index].description);
        }

        main_helper_trigger_action_menu2(layout, &SETTINGSMENU_MENU_COMMON, last_selected_index, *title, false, false);
        last_selected_index = selected_index;
        main_helper_trigger_action_menu2(layout, &SETTINGSMENU_MENU_COMMON, selected_index, *title, true, false);
        modding_helper_notify_option(modding, true);
    }

    if (option >= 0) {
        main_helper_trigger_action_menu2(layout, &SETTINGSMENU_MENU_COMMON, option, *title, false, true);
    }

    modding->has_exit = false;
    return option;
}

static void settingsmenu_in_gameplay_settings(Gamepad gamepad, Modding modding) {
    static const char* title = "GAMEPLAY SETTINGS";
    static SettingOption options[] = {
        {
            .name = "USE FUNKIN MARKER DURATION",
            .description = "Uses a 166.67ms marker duration instead of a pixel accurate duration.\n"
                           "ENABLED = the hit and rank calculation is tight like Funkin,\n"
                           "DISABLED = the engine calculate the duration according to the screen",
            .is_bool = true,
            .value_bool = true,
            .id = SETTINGS_ID_USE_FUNKIN_MARKER_DURATION, //,ini_key: "use_funkin_marker_duration",
            .hidden = false,
        },
        {
            .name = "PENALITY ON EMPTY STRUM",
            .description = "Penalizes the player if press a button on empty strum.\n"
                           "ENABLED: the penality is enforced. DISABLED: allows ghost-tapping",
            .is_bool = true,
            .value_bool = true,
            .id = SETTINGS_ID_PENALITY_ON_EMPTY_STRUM, // ini_key: "penality_on_empty_strum",
            .hidden = false,
        },
        {
            .name = "INPUT OFFSET",
            .description = "Offets all button press timestamps. Defaults to 0 milliseconds\n"
                           "useful for bluetooth headset or television screens",
            .is_int = true,
            .value_int = 0,
            .number_min = -1000,
            .number_max = 1000,
            .id = SETTINGS_ID_INPUT_OFFSET, // ini_key: "input_offset",
            .hidden = false,
        },
        {
            .name = "INVERSE STRUM SCROLLING",
            .description = "In the default UI layout this switch from UPSCROLL to DOWNSCROLL,\n"
                           "In custom UI layouts, reverses the declared direction",
            .is_bool = true,
            .value_bool = false,
            .id = SETTINGS_ID_INVERSE_STRUM_SCROLL, // ini_key: "inverse_strum_scroll",
            .hidden = false,
        },
        {
            .name = "SHOW SONG PROGRESS BAR",
            .description = "Shows a progress bar near the strums makers.\n"
                           "Indicates playback progress of the playing song",
            .is_bool = true,
            .value_bool = true,
            .id = SETTINGS_ID_SONG_PROGRESSBAR, // ini_key: "song_progressbar",
            .hidden = false,
        },
        {
            .name = "SONG PROGRESS BAR REMAINING",
            .description = "Shows the remaining song duration instead of time elapsed",
            .is_bool = true,
            .value_bool = false,
            .id = SETTINGS_ID_SONG_PROGRESSBAR_REMAINING, // ini_key: "song_progressbar_remaining",
            .hidden = false,
        },
        {
            .name = "DISTRACTIONS",
            .description = "Enables/disables stage distractions during the gameplay.\n"
                           "(Depends on the week if honours this setting)",
            .is_bool = true,
            .value_bool = true,
            .id = SETTINGS_ID_GAMEPLAY_ENABLED_DISTRACTIONS, // ini_key: "gameplay_enabled_distractions",
            .hidden = false,
        },
        {
            .name = "FLASHING LIGHTS",
            .description = "Rhythm games often use flashing lights, sometimes they can be annoying or cause seizures.\n"
                           "(Depends on the week if honours this setting)",
            .is_bool = true,
            .value_bool = true,
            .id = SETTINGS_ID_GAMEPLAY_ENABLED_FLASHINGLIGHTS, // ini_key: "gameplay_enabled_flashinglights",
            .hidden = false,
        },
        {
            .name = "UI COSMETICS",
            .description = "Hides away the accuracy, rank, and streak counters.\n"
                           "These elements normally are shown near or surrounding girlfriend's speakers",
            .is_bool = true,
            .value_bool = true,
            .id = SETTINGS_ID_GAMEPLAY_ENABLED_UICOSMETICS, // ini_key: "gameplay_enabled_uicosmetics",
            .hidden = false,
        },

    };
    static const size_t options_size = sizeof(options) / sizeof(SettingOption);

    // load current settings
    for (size_t i = 0; i < options_size; i++) {
        settingsmenu_internal_load_option(&options[i] /*, SETTINGS.INI_GAMEPLAY_SECTION*/);
    }

    settingsmenu_show_common(&title, gamepad, options, options_size, modding);

    // save settings
    for (size_t i = 0; i < options_size; i++) {
        settingsmenu_internal_save_option(&options[i] /*, SETTINGS.INI_GAMEPLAY_SECTION*/);
    }
}

#ifndef _arch_dreamcast
static void settingsmenu_in_misc_settings(Gamepad gamepad, Modding modding) {
    static const SettingOption options[] = {
        {
            .name = "DISPLAY FPS",
            .description = "Displays the current FPS in the top-left corner of the screen.\n",
            .is_bool = true,
            .value_bool = false,
            .id = SETTINGS_ID_SHOW_FPS, //.ini_key = "show_fps",
            .hidden = true,
        },
        {
            .name = "FPS LIMIT",
            .description = "Sets the frame limiter. V-SYNC: uses the screen frame-rate (default).\n"
                           "DETERMINISTIC: for constant input checking with low CPU overhead.\n"
                           "OFF: no fps limit, high CPU usage.",
            .is_list = true,
            .value_list_index = 0,
            .list = (SettingOptionListItem[]){
                {
                    .name = "V-SYNC", .value_id = {.value_byte = SETTINGS_ID_FPS_LIMIT_VALUE_VSYNC}, // .ini_value = "vsync" ,
                },
                {
                    .name = "Deterministic", .value_id = {.value_byte = SETTINGS_ID_FPS_LIMIT_VALUE_DETERMINISTIC}, // .ini_value = "deterministic" ,
                },
                {
                    .name = "Off", .value_id = {.value_byte = SETTINGS_ID_FPS_LIMIT_VALUE_OFF} // .ini_value = "off"
                },
            },
            .list_size = 3,
            .id = SETTINGS_ID_FPS_LIMIT, //.ini_key = "fps_limit",
            .hidden = true,
        },
        {
            .name = "START IN FULLSCREEN",
            .description = "Runs the game in fullscreen.\n"
                           "Press F11 to toggle between windowed/fullscreen",
            .is_bool = true,
            .value_bool = false,
            .id = SETTINGS_ID_FULLSCREEN, //.ini_key = "fullscreen",
            .hidden = true,
        },
        {
            .name = "AVAILABLE SAVESLOTS",
            .description = "Emulates the specified number of VMUs.\n"
                           "Defaults to 1, this applies before the engine runs",
            .is_int = true,
            .value_int = 1,
            .number_min = 1,
            .number_max = 8,
            .id = SETTINGS_ID_SAVESLOTS, //.ini_key = "saveslots",
            .hidden = false,
        },
        {
            .name = "AUTOHIDE CURSOR",
            .description = "Hides your mouse cursor after 3 seconds of inactivity.\n"
                           "The mouse is never used by the engine, but some weeks can make use of it",
            .is_bool = true,
            .value_bool = true,
            .id = SETTINGS_ID_AUTOHIDE_CURSOR, //.ini_key = "autohide_cursor",
            .hidden = false,
        },
        {
            .name = "SILENCE ON MINIMIZE",
            .description = "Mutes audio output when the window is minimized.\n"
                           "Freeplay menu is exempt",
            .is_bool = true,
            .value_bool = true,
            .id = SETTINGS_ID_MUTE_ON_MINIMIZE, //.ini_key = "mute_on_minimize",
            .hidden = true,
        },
        {
            .name = "SHOW LOADING SCREEN",
            .description = "Displays a sprite after starting the week/song gameplay.\n"
                           "Only applicable in Freeplay menu and Week selector",
            .is_bool = true,
            .value_bool = false,
            .id = SETTINGS_ID_SHOW_LOADING_SCREEN, //.ini_key = "show_loading_screen",
            .hidden = false,
        }
    };
    static const size_t options_size = sizeof(options) / sizeof(SettingOption);

    // load current settings
    for (size_t i = 0; i < options_size; i++) {
        settingsmenu_internal_load_option(&options[i] /*, SETTINGS.INI_MISC_SECTION*/);
    }

    settingsmenu_show_common("MISCELLANEOUS", gamepad, options, options_size, modding);

    // save settings
    for (size_t i = 0; i < options_size; i++) {
        settingsmenu_internal_save_option(&options[i] /*, SETTINGS.INI_MISC_SECTION*/);
    }
}
#endif

static void settingsmenu_show_common(const char** title, Gamepad gamepad, SettingOption options[], const size_t options_count, Modding modding) {
    Layout layout = layout_init(pvr_context_is_widescreen() ? SETTINGSMENU_LAYOUT_COMMON : SETTINGSMENU_LAYOUT_COMMON_DREAMCAST);

    if (!layout) {
        logger_error("settingsmenu_show_common() can not load the layout");
        return;
    }

    textsprite_set_text_intern(layout_get_textsprite(layout, "title"), true, title);

    TextSprite setting_name = layout_get_textsprite(layout, "setting-name");

    // settings menu
    bool is_vertical = layout_get_attached_value_boolean(
        layout, "menu_isVertical", false
    );
    float font_size = layout_get_attached_value_as_float(
        layout, "menu_fontSize", 28.0f
    );
    float items_gap = layout_get_attached_value_as_float(
        layout, "menu_itemGap", 56.0f
    );
    float font_border_size = layout_get_attached_value_as_float(
        layout, "menu_fontBorderSize", 4.0f
    );
    LayoutPlaceholder* menu_placeholder = layout_get_placeholder(layout, "menu");
    if (!menu_placeholder) {
        logger_error("Missing menu placeholder");
        assert(menu_placeholder);
    }

    // create menu options
    SETTINGSMENU_MENU_COMMON.parameters.is_vertical = is_vertical;
    SETTINGSMENU_MENU_COMMON.parameters.font_size = font_size;
    SETTINGSMENU_MENU_COMMON.parameters.items_gap = items_gap;
    SETTINGSMENU_MENU_COMMON.parameters.items_align = is_vertical ? menu_placeholder->align_vertical : menu_placeholder->align_horizontal;
    SETTINGSMENU_MENU_COMMON.parameters.anim_selected = "menu_common_item_selected";
    SETTINGSMENU_MENU_COMMON.parameters.anim_idle = "menu_common_item_idle";
    SETTINGSMENU_MENU_COMMON.parameters.font_border_size = font_border_size;
    SETTINGSMENU_MENU_COMMON.items_size = (int32_t)options_count;
    SETTINGSMENU_MENU_COMMON.items = malloc_for_array(MenuManifestItem, options_count);

    for (int32_t i = 0; i < options_count; i++) {
        SETTINGSMENU_MENU_COMMON.items[i] = (MenuManifestItem){
            name : NULL, // unused
            text : (char*)options[i].name,
            placement : {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            anim_selected : NULL,                                            // unused
            anim_choosen : NULL,                                             // unused
            anim_discarded : NULL,                                           // unused
            anim_idle : NULL,                                                // unused
            anim_rollback : NULL,                                            // unused
            anim_in : NULL,                                                  // unused
            anim_out : NULL,                                                 // unused
            hidden : options[i].hidden,
            description : NULL // unused
        };
    }

    // initialize menu
    Menu menu = menu_init(
        &SETTINGSMENU_MENU_COMMON,
        menu_placeholder->x, menu_placeholder->y, menu_placeholder->z,
        menu_placeholder->width, menu_placeholder->height
    );
    menu_placeholder->vertex = menu_get_drawable(menu);

    // items are not longer required, dispose
    free_chk(SETTINGSMENU_MENU_COMMON.items);
    SETTINGSMENU_MENU_COMMON.items = NULL;
    SETTINGSMENU_MENU_COMMON.items_size = 0;

    if (setting_name) textsprite_set_text_intern(setting_name, true, &options[0].name);

    while (!modding->has_exit) {
        int32_t selected_option = settingsmenu_in_common_menu(title, layout, gamepad, menu, options, modding);
        if (selected_option < 0) break;

        if (setting_name) textsprite_set_text_intern(setting_name, true, &options[selected_option].name);

        layout_set_group_visibility(layout, "holder-boolean", false);
        layout_set_group_visibility(layout, "holder-numeric", false);
        layout_set_group_visibility(layout, "holder-list", false);
        layout_set_group_visibility(layout, "change-value", true);

        if (options[selected_option].is_bool) {
            options[selected_option].value_bool = settingsmenu_in_common_change_bool(
                layout, gamepad, options[selected_option].value_bool
            );
        } else if (options[selected_option].is_int) {
            options[selected_option].value_int = settingsmenu_in_common_change_number(
                layout, gamepad, options[selected_option].number_min, options[selected_option].number_max, options[selected_option].value_int
            );
        } else if (options[selected_option].is_list) {
            options[selected_option].value_list_index = settingsmenu_in_common_change_list(
                layout, gamepad, options[selected_option].list, options[selected_option].list_size, options[selected_option].value_list_index
            );
        }

        layout_set_group_visibility(layout, "change-value", false);
    }

    if (settingsmenu_current_menu_choosen_custom) {
        modding_helper_notify_handle_custom_option(modding, settingsmenu_current_menu_choosen_custom);
    }
    free_chk(settingsmenu_current_menu_choosen_custom);
    settingsmenu_current_menu_choosen_custom = NULL;

    menu_destroy(&menu);
    layout_destroy(&layout);
}

static int32_t settingsmenu_in_common_change_number(Layout layout, Gamepad gamepad, int32_t min, int32_t max, int32_t value) {
    value = math2d_clamp_int(value, min, max);
    int32_t orig_value = value;

    layout_set_group_visibility(layout, "holder-numeric", true);
    TextSprite number = layout_get_textsprite(layout, "number-value");

    textsprite_set_text_formated(number, "$i", value);
    gamepad_clear_buttons(gamepad);

    while (true) {
        float elapsed = pvrctx_wait_ready();

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        GamepadButtons buttons = gamepad_has_pressed_delayed(
            gamepad,
            GAMEPAD_A | GAMEPAD_B |
                GAMEPAD_START | GAMEPAD_BACK |
                GAMEPAD_AD
        );

        if (buttons & (GAMEPAD_A | GAMEPAD_START)) {
            break;
        }
        if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
            value = orig_value;
            break;
        }

        if (buttons & GAMEPAD_AD_UP)
            value++;
        else if (buttons & GAMEPAD_AD_DOWN)
            value--;
        else if (buttons & GAMEPAD_AD_LEFT)
            value -= 10;
        else if (buttons & GAMEPAD_AD_RIGHT)
            value += 10;
        else
            continue;

        value = math2d_clamp_int(value, min, max);
        textsprite_set_text_formated(number, "$i", value);
    }

    return value;
}

static bool settingsmenu_in_common_change_bool(Layout layout, Gamepad gamepad, bool value) {
    bool orig_value = value;

    layout_set_group_visibility(layout, "holder-boolean", true);
    layout_trigger_any(layout, value ? "bool-true" : "bool-false");
    gamepad_clear_buttons(gamepad);

    while (true) {
        float elapsed = pvrctx_wait_ready();

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        GamepadButtons buttons = gamepad_has_pressed_delayed(
            gamepad,
            GAMEPAD_A | GAMEPAD_B |
                GAMEPAD_START | GAMEPAD_BACK |
                GAMEPAD_AD
        );

        if (buttons == 0x00) continue;

        if (buttons & (GAMEPAD_A | GAMEPAD_START)) {
            break;
        }
        if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
            value = orig_value;
            break;
        }

        value = !value;
        layout_trigger_any(layout, value ? "bool-true" : "bool-false");
    }

    return value;
}

static int32_t settingsmenu_in_common_change_list(Layout layout, Gamepad gamepad, const SettingOptionListItem list[], int32_t list_size, int32_t index) {
    if (list_size < 1) return index;

    layout_set_group_visibility(layout, "holder-list", true);
    Sprite list_selected_back = layout_get_sprite(layout, "list-selected-back");

    bool is_vertical = layout_get_attached_value_boolean(
        layout, "menu-selector_isVertical", false
    );
    float font_size = layout_get_attached_value_as_float(
        layout, "menu-selector_fontSize", 28.0f
    );
    float items_gap = layout_get_attached_value_as_float(
        layout, "menu-selector_itemGap", 56.0f
    );

    LayoutPlaceholder* menu_placeholder = layout_get_placeholder(layout, "menu-selector");
    if (!menu_placeholder) {
        logger_error("Missing menu-selector placeholder");
        assert(menu_placeholder);
    }

    // create selector menu
    SETTINGSMENU_MENU_COMMON.parameters.font_border_size = 0.0;
    SETTINGSMENU_MENU_COMMON.parameters.is_vertical = is_vertical;
    SETTINGSMENU_MENU_COMMON.parameters.font_size = font_size;
    SETTINGSMENU_MENU_COMMON.parameters.items_gap = items_gap;
    SETTINGSMENU_MENU_COMMON.parameters.items_align = is_vertical ? menu_placeholder->align_vertical : menu_placeholder->align_horizontal;
    SETTINGSMENU_MENU_COMMON.parameters.anim_selected = "menu_selector_item_selected";
    SETTINGSMENU_MENU_COMMON.parameters.anim_idle = "menu_selector_item_idle";
    SETTINGSMENU_MENU_COMMON.items_size = list_size;
    SETTINGSMENU_MENU_COMMON.items = malloc_for_array(MenuManifestItem, list_size);

    for (int32_t i = 0; i < list_size; i++) {
        SETTINGSMENU_MENU_COMMON.items[i] = (MenuManifestItem){
            .name = NULL, // unused
            .text = (char*)list[i].name,
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,                                           // unused
            .anim_idle = NULL,                                                // unused
            .anim_rollback = NULL,                                            // unused
            .anim_in = NULL,                                                  // unused
            .anim_out = NULL,                                                 // unused
            .hidden = false,
            .description = NULL // unused
        };
    }

    // initialize menu
    Menu menu = menu_init(
        &SETTINGSMENU_MENU_COMMON,
        menu_placeholder->x, menu_placeholder->y, menu_placeholder->z,
        menu_placeholder->width, menu_placeholder->height
    );
    menu_placeholder->vertex = menu_get_drawable(menu);

    // items are not longer required, dispose
    free_chk(SETTINGSMENU_MENU_COMMON.items);
    SETTINGSMENU_MENU_COMMON.items = NULL;
    SETTINGSMENU_MENU_COMMON.items_size = 0;

    menu_select_index(menu, index);
    gamepad_clear_buttons(gamepad);

    bool first_run = true;
    float border_size = font_size * 1.25f;

    while (true) {
        float elapsed = pvrctx_wait_ready();

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        GamepadButtons buttons = gamepad_has_pressed_delayed(
            gamepad,
            GAMEPAD_A | GAMEPAD_B |
                GAMEPAD_START | GAMEPAD_BACK |
                GAMEPAD_AD
        );

        if (buttons & (GAMEPAD_A | GAMEPAD_START)) {
            index = menu_get_selected_index(menu);
            assert(index >= 0 && index < menu_get_items_count(menu));
            break;
        }
        if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
            break;
        }

        if (buttons & GAMEPAD_AD_UP)
            menu_select_vertical(menu, -1);
        else if (buttons & GAMEPAD_AD_DOWN)
            menu_select_vertical(menu, 1);
        else if (buttons & GAMEPAD_AD_LEFT)
            menu_select_horizontal(menu, -1);
        else if (buttons & GAMEPAD_AD_RIGHT)
            menu_select_horizontal(menu, 1);
        else if (!first_run)
            continue;

        if (!list_selected_back) continue;

        float x = 0.0f, y = 0.0f, width = 0.0f, height = 0.0f;
        if (menu_get_selected_item_rect(menu, &x, &y, &width, &height)) {
            x -= border_size;
            y -= border_size;
            width += border_size * 2.0f;
            height += border_size * 2.0f;

            sprite_set_draw_location(list_selected_back, x, y);
            sprite_set_draw_size(list_selected_back, width, height);
            sprite_set_visible(list_selected_back, true);
        } else {
            sprite_set_visible(list_selected_back, false);
        }

        first_run = false;
    }

    menu_destroy(&menu);
    return index;
}


#ifndef _arch_dreamcast
static void settingsmenu_internal_save_key(GamepadButtons target_button, KeyCode keycode) {
#warning "not implemented"
    funkinsave_set_setting_binding(target_button, keycode);
}

static KeyCode settingsmenu_internal_load_key(TextSprite label, GamepadButtons target_button, KeyCode default_keycode) {
#warning "not implemented"
    KeyCode keycode = funkinsave_get_setting_binding(target_button, default_keycode);

    settingsmenu_internal_set_key_in_label(label, keycode.name);
    return keycode;
}

static void settingsmenu_internal_set_key_in_label(TextSprite label, KeyCode keycode) {
#warning "not implemented"
    const char* const* key_name = keycode.name ? keycode.name : ((const char* const*)&SETTINGSMENU_KEY_NONE);
    textsprite_set_text_intern(label, true, key_name);
}

static GamepadButtons settingsmenu_internal_read_keyboard() {
    uint64_t now = timer_ms_gettime64();
    if (now < settingsmenu_next_pressed_timestamp) return 0x00;

    GamepadButtons buttons = 0x00;

    if (settingsmenu_last_detected_keycode.key == KEYCODE_ArrowLeft.key)
        buttons |= GAMEPAD_DPAD_LEFT;
    if (settingsmenu_last_detected_keycode.key == KEYCODE_ArrowDown.key)
        buttons |= GAMEPAD_DPAD_DOWN;
    if (settingsmenu_last_detected_keycode.key == KEYCODE_ArrowUp.key)
        buttons |= GAMEPAD_DPAD_UP;
    if (settingsmenu_last_detected_keycode.key == KEYCODE_ArrowRight.key)
        buttons |= GAMEPAD_DPAD_RIGHT;
    if (settingsmenu_last_detected_keycode.key == KEYCODE_Enter.key)
        buttons |= GAMEPAD_START;
    if (settingsmenu_last_detected_keycode.key == KEYCODE_Escape.key)
        buttons |= GAMEPAD_BACK;

    if (buttons)
        settingsmenu_next_pressed_timestamp = now + SETTINGSMENU_DELAY_MS;

    return buttons;
}

static void settingsmenu_internal_key_callback(KeyCode keycode, bool state_keydown, uint16_t modKeys) {
    if (state_keydown)
        settingsmenu_last_detected_keycode = keycode;
    else
        memset(settingsmenu_last_detected_keycode, 0x00, sizeof(KeyCode));
}

static void settingsmenu_internal_rollback_anim(Sprite back, AnimSprite rollback) {
    if (!rollback) return;
    animsprite_restart(rollback);
    animsprite_force_end(rollback);
    animsprite_update_sprite(rollback, back, true);
}

static void settingsmenu_internal_update_holder(Sprite backs[], TextSprite labels[], KeyCode keycodes[], int32_t index, AnimSprite anim, bool rollback, uint32_t color) {
    if (rollback) settingsmenu_internal_rollback_anim(backs[index], anim);
    sprite_set_vertex_color_rgba8(backs[index], color);
    settingsmenu_internal_set_key_in_label(labels[index], keycodes[index]);
}
#endif


static void settingsmenu_internal_save_option(const SettingOption* option) {
    if (option->is_bool) {
        funkinsave_set_setting_bool(option->id, option->value_bool);
    } else if (option->is_int) {
        funkinsave_set_setting_int(option->id, option->value_int);
    } else if (option->is_list) {
        uint8_t value_id = option->list[option->value_list_index].value_id.value_byte;
        funkinsave_set_setting_int(option->id, value_id);
    }
}

static void settingsmenu_internal_load_option(SettingOption* option) {
    if (option->is_bool) {
        option->value_bool = funkinsave_get_setting_bool(option->id, option->value_bool);
    } else if (option->is_int) {
        option->value_int = funkinsave_get_setting_int(option->id, option->value_int);
    } else if (option->is_list) {
        uint8_t value_id = option->list[option->value_list_index].value_id.value_byte;
        value_id = funkinsave_get_setting_int(option->id, value_id);

        for (int32_t i = 0; i < option->list_size; i++) {
            if (value_id == option->list[i].value_id.value_byte) {
                option->value_list_index = i;
                break;
            }
        }
    }
}

static bool settingsmenu_internal_handle_option(void* obj, const char* option_name) {
    (void)obj;

    if (settingsmenu_current_menu == NULL || settingsmenu_current_setting_options == NULL) {
        return false;
    }

    free_chk(settingsmenu_current_menu_choosen_custom);

    settingsmenu_current_menu_choosen = true;
    if (option_name == NULL) {
        settingsmenu_current_menu_choosen_custom = NULL;
        return false;
    }

    if (menu_has_item(settingsmenu_current_menu, option_name)) {
        menu_select_item(settingsmenu_current_menu, option_name);
        settingsmenu_current_menu_choosen = true;
        return true;
    } else {
        for (int32_t i = 0, count = menu_get_items_count(settingsmenu_current_menu); i < count; i++) {
            if (string_equals(settingsmenu_current_setting_options[i].name, option_name)) {
                menu_select_index(settingsmenu_current_menu, i);
                return true;
            }
        }
    }

    settingsmenu_current_menu_choosen_custom = string_duplicate(option_name);
    return false;
}
