"use strict";

const SETTINGSMENU_KEY_NONE = "(none)";
const SETTINGSMENU_KEY_BIND = "(waiting)";
const SETTINGSMENU_DELAY_SECONDS = 0.25;
const SETTINGSMENU_MODDING_SCRIPT = "/assets/common/data/scripts/settigsmenu.lua";
const SETTINGSMENU_MODDING_MENU = "/assets/common/data/menus/settigsmenu-main.json";

const SETTINGSMENU_LAYOUT_BINDS_GAMEPLAY = "/assets/common/image/settings-menu/binds_gameplay.xml";
const SETTINGSMENU_LAYOUT_BINDS_MENUS = "/assets/common/image/settings-menu/binds_menus.xml";
const SETTINGSMENU_LAYOUT_MAIN = "/assets/common/image/settings-menu/main.xml";
const SETTINGSMENU_LAYOUT_COMMON = "/assets/common/image/settings-menu/common.xml";
const SETTINGSMENU_LAYOUT_SAVEDATA = "/assets/common/image/settings-menu/savedata.xml";
const SETTINGSMENU_LAYOUT_BINDS_GAMEPLAY_DREAMCAST = "/assets/common/image/settings-menu/binds_gameplay~dreamcast.xml";
const SETTINGSMENU_LAYOUT_BINDS_MENUS_DREAMCAST = "/assets/common/image/settings-menu/binds_menus~dreamcast.xml";
const SETTINGSMENU_LAYOUT_MAIN_DREAMCAST = "/assets/common/image/settings-menu/main~dreamcast.xml";
const SETTINGSMENU_LAYOUT_COMMON_DREAMCAST = "/assets/common/image/settings-menu/common~dreamcast.xml";
const SETTINGSMENU_LAYOUT_SAVEDATA_DREAMCAST = "/assets/common/image/settings-menu/savedata~dreamcast.xml";

const SETTINGSMENU_MENU = {
    parameters: {
        suffix_selected: null,// unused
        suffix_idle: null,// unused
        suffix_choosen: null,// unused
        suffix_discarded: null,// unused
        suffix_rollback: null,// unused
        suffix_in: null,// unused
        suffix_out: null,// unused

        atlas: null,
        animlist: "/assets/common/anims/settings-menu.xml",

        anim_selected: "menu_item_selected",
        anim_idle: "menu_item_idle",
        anim_choosen: null,// unused
        anim_discarded: null,// unused
        anim_rollback: null,// unused
        anim_in: null,// unused
        anim_out: null,// unused

        anim_transition_in_delay: 0.0,// unused
        anim_transition_out_delay: 0.0,// unused

        font: "/assets/common/font/Alphabet.xml",
        font_glyph_suffix: "bold",
        font_color_by_addition: false,// unused
        font_size: 46.0,
        font_color: 0xFFFFFF,
        font_border_color: 0x00,// unused
        font_border_size: NaN,// unused

        is_sparse: false,// unused
        is_vertical: true,
        is_per_page: false,
        static_index: 1,

        items_align: ALIGN_START,
        items_gap: 38.0,
        items_dimmen: 0.0,// unused
        texture_scale: NaN,// unused
        enable_horizontal_text_correction: true// unused
    },
    items: [
        {
            name: "keyboard-bindings-gameplay",
            text: "KEYBOARD BINDINGS IN GAMEPLAY",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null// unused
        },
        {
            name: "keyboard-bindings-menu",
            text: "KEYBOARD BINDINGS IN MENUS",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null// unused
        },
        {
            name: "gameplay-settings",
            text: "GAMEPLAY SETTINGS",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null// unused
        },
        {
            name: "miscellaneous",
            text: "MISCELLANEOUS",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null// unused
        },
        {
            name: "savedata",
            text: "SAVEDATA",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null // unused
        },
        {
            name: "return-main-menu",
            text: "RETURN TO THE MAIN MENU",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null// unused
        }
    ],
    items_size: 6
};
const SETTINGSMENU_SAVEDATA = {
    parameters: {
        suffix_selected: null,// unused
        suffix_idle: null,// unused
        suffix_choosen: null,// unused
        suffix_discarded: null,// unused
        suffix_rollback: null,// unused
        suffix_in: null,// unused
        suffix_out: null,// unused

        atlas: null,
        animlist: "/assets/common/anims/settings-menu.xml",

        anim_selected: "menu_item_selected",
        anim_idle: "menu_item_idle",
        anim_choosen: null,// unused
        anim_discarded: null,// unused
        anim_rollback: null,// unused
        anim_in: null,// unused
        anim_out: null,// unused

        anim_transition_in_delay: 0,// unused
        anim_transition_out_delay: 0,// unused

        font: "/assets/common/font/Alphabet.xml",
        font_glyph_suffix: "bold",
        font_color_by_addition: false,// unused
        font_size: 46.0,
        font_color: 0xFFFFFF,
        font_border_color: 0x00,// unused
        font_border_size: NaN,// unused

        is_sparse: false,// unused
        is_vertical: true,
        is_per_page: false,
        static_index: 1,

        items_align: ALIGN_START,
        items_gap: 38.0,
        items_dimmen: 0.0,// unused
        texture_scale: NaN,// unused
        enable_horizontal_text_correction: true// unused
    },
    items: [
        {
            name: "load-only",
            text: "LOAD SAVEDATA",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null// unused
        },
        {
            name: "save-or-delete",
            text: "SAVE OR DELETE FROM VMU",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null// unused
        },
        {
            name: "return",
            text: "RETURN TO THE SETTINGS MENU",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null// unused
        },
    ],
    items_size: 3
};
const SETTINGSMENU_MENU_COMMON = {
    parameters: {
        suffix_selected: null,// unused
        suffix_idle: null,// unused
        suffix_choosen: null,// unused
        suffix_discarded: null,// unused
        suffix_rollback: null,// unused
        suffix_in: null,// unused
        suffix_out: null,// unused

        atlas: null,
        animlist: "/assets/common/anims/settings-menu.xml",

        anim_selected: null,
        anim_idle: null,
        anim_choosen: null,// unused
        anim_discarded: null,// unused
        anim_rollback: null,// unused
        anim_in: null,// unused
        anim_out: null,// unused

        anim_transition_in_delay: 0.0,// unused
        anim_transition_out_delay: 0.0,// unused

        font: "/assets/common/font/pixel.otf",
        font_glyph_suffix: null,// unused
        font_color_by_addition: false,// unused
        font_size: 28.0,
        font_color: 0xFFFFFF,
        font_border_color: 0x000000FF,// unused
        font_border_size: 4.0,// unused

        is_sparse: false,// unused
        is_vertical: true,
        is_per_page: true,
        static_index: 0,

        items_align: ALIGN_START,
        items_gap: 58.0,
        items_dimmen: 0.0,// unused
        texture_scale: NaN,// unused
        enable_horizontal_text_correction: true// unused
    },
    items: null,
    items_size: 0
};


var settingsmenu_next_pressed_timestamp = 0.0;
var settingsmenu_last_detected_keycode = null;
var settingsmenu_previous_key_callback = null;
var settingsmenu_current_menu = null;
var settingsmenu_current_setting_options = null;
var settingsmenu_current_menu_choosen = false;
var settingsmenu_current_menu_choosen_custom = null;
var settingsmenu_is_running = false;


async function settingsmenu_main() {
    const title = "Settings";
    let main_options_help = [
        {
            name: "keyboard-bindings-gameplay",
            description: "Change the assigned key for each strum.\nDirectonial keys are always assigned"
        },
        {
            name: "keyboard-bindings-menu",
            description: "Change the keys assigned to navigate between menus.\nDirectonial keys are always assigned"
        },
        {
            name: "gameplay-settings",
            description: "Gameplay settings like ghost-tapping and input offset"
        },
        {
            name: "miscellaneous",
            description: "Specific engine settings"
        },
        {
            name: "return-main-menu",
            description: "Returns back to the main menu.\n¡Settings are automatically saved!"
        },
        {
            name: "savedata",
            description: "Load, save or delete savedata.\nAlso sets what VMU should be used",
            hidden: false,
        }
    ];

    if (settingsmenu_is_running) {
        console.error("settingsmenu_main() is already running, only a single instance is allowed");
        return;
    }

    let submenus_font = null;
    if (await fs_file_exists(SETTINGSMENU_MENU_COMMON.parameters.font)) {
        // little improvement, keep loaded the "pixel.otf" font to improve loading times
        submenus_font = await fontholder_init(SETTINGSMENU_MENU_COMMON.parameters.font, -1.0, null, false);
    }

    let animlist = await animlist_init("/assets/common/anims/settings-menu.xml");
    let anim_binding, anim_binding_rollback;

    if (animlist) {
        anim_binding = animsprite_init_from_animlist(animlist, "binding");
        anim_binding_rollback = animsprite_init_from_animlist(animlist, "binding_rollback");
        animlist_destroy(animlist);
    } else {
        anim_binding = anim_binding_rollback = null;
    }

    let layout = await layout_init(pvr_context_is_widescreen() ? SETTINGSMENU_LAYOUT_MAIN : SETTINGSMENU_LAYOUT_MAIN_DREAMCAST);
    if (!layout) {
        console.error("settingsmenu_main() can not load the layout");
        return;
    }

    SETTINGSMENU_MENU.parameters.is_vertical = layout_get_attached_value(
        layout, "menu_isVertical", LAYOUT_TYPE_BOOLEAN, SETTINGSMENU_MENU.parameters.is_vertical
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
    let menu_placeholder = layout_get_placeholder(layout, "menu");
    if (!menu_placeholder) throw new Error("Missing menu placeholder");

    let menumanifest = SETTINGSMENU_MENU;
    let options_help = main_options_help;

    if (await fs_file_exists(SETTINGSMENU_MODDING_MENU)) {
        menumanifest = await menumanifest_init(SETTINGSMENU_MODDING_MENU);
        if (!menumanifest) throw new Error("failed to load " + SETTINGSMENU_MODDING_MENU);

        // since a custom menu was provided, remap option descriptions
        options_help = new Array[menumanifest.items_size];

        for (let i = 0; i < menumanifest.items_size; i++) {
            options_help[i].description = menumanifest.items[i].description;

            if (options_help[i].description) {
                options_help[i].description_changed = true;
                continue;
            }

            for (let j = 0; j < main_options_help.length; j++) {
                if (main_options_help[i].name == menumanifest.items[i].name) {
                    options_help[i].description = main_options_help[i].description;
                    options_help[i].description_changed = false;
                    break;
                }
            }
        }
    }

    let menu = await menu_init(
        menumanifest,
        menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
        menu_placeholder.width, menu_placeholder.height
    );
    menu_placeholder.vertex = menu_get_drawable(menu);

    let backgroud_music_filename = layout_get_attached_value(
        layout, "backgroud_music_filename", LAYOUT_TYPE_STRING, null
    );
    let backgroud_music_volume = layout_get_attached_value(
        layout, "backgroud_music_volume", LAYOUT_TYPE_DOUBLE, 0.5
    );

    let bg_music = null;
    if (backgroud_music_filename) bg_music = await soundplayer_init(backgroud_music_filename);
    if (bg_music) soundplayer_set_volume(bg_music, backgroud_music_volume);

    KOS_MAPLE_KEYBOARD.delegate_callback = settingsmenu_internal_key_callback;

    let gamepad = gamepad_init(-1);
    gamepad_set_buttons_delay(gamepad, 75);
    gamepad_clear_buttons(gamepad);

    if (bg_music) {
        if (background_menu_music) soundplayer_pause(background_menu_music);
        soundplayer_loop_enable(bg_music, true);
        soundplayer_play(bg_music);
    } else if (background_menu_music) {
        if (!backgroud_music_filename && (backgroud_music_volume <= 0.0 || Number.isNaN(backgroud_music_volume)))
            soundplayer_pause(background_menu_music);
        else
            soundplayer_set_volume(background_menu_music, 0.5);
    }

    settingsmenu_current_menu = null;
    settingsmenu_current_setting_options = null;
    settingsmenu_current_menu_choosen = false;
    settingsmenu_current_menu_choosen_custom = null;
    settingsmenu_is_running = true;

    let modding = await modding_init(layout, SETTINGSMENU_MODDING_SCRIPT);
    modding.callback_option = settingsmenu_internal_handle_option;
    await modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN);

    while (!modding.has_exit) {
        let selected_index = await settingsmenu_in_common_menu(title, layout, gamepad, menu, options_help, modding);
        let selected_name = selected_index < 0 ? null : menumanifest.items[selected_index].name;
        switch (selected_name) {
            case "keyboard-bindings-gameplay":
                await settingsmenu_in_gameplay_binding(anim_binding, anim_binding_rollback);
                continue;
            case "keyboard-bindings-menu":
                await settingsmenu_in_menus_binding(anim_binding, anim_binding_rollback);
                continue;
            case "gameplay-settings":
                await settingsmenu_in_gameplay_settings(gamepad, modding);
                continue;
            case "miscellaneous":
                await settingsmenu_in_misc_settings(gamepad, modding);
                continue;
            case "savedata":
                await settingsmenu_in_save_settings(gamepad, modding);
                continue;
            case "return-main-menu":
                break;
            default:
                // custom option selected
                if (settingsmenu_current_menu_choosen_custom == null) break;
                await modding_helper_notify_handle_custom_option(modding, settingsmenu_current_menu_choosen_custom);
                settingsmenu_current_menu_choosen_custom = undefined;
                settingsmenu_current_menu_choosen_custom = null;
                continue;
        }
        break;
    }

    await modding_helper_notify_exit2(modding);

    animsprite_destroy(anim_binding);
    animsprite_destroy(anim_binding_rollback);
    menu_destroy(menu);
    layout_destroy(layout);
    modding_destroy(modding);
    gamepad_destroy(gamepad);
    if (submenus_font) fontholder_destroy(submenus_font);

    settingsmenu_is_running = false;

    if (options_help != main_options_help) options_help = undefined;
    if (menumanifest != SETTINGSMENU_MENU) menumanifest_destroy(menumanifest);

    // reload settings
    SETTINGS.reload();

    KOS_MAPLE_KEYBOARD.delegate_callback = null;

    if (bg_music) {
        soundplayer_destroy(bg_music);
        if (background_menu_music) soundplayer_play(background_menu_music);
    } else if (background_menu_music) {
        soundplayer_set_volume(background_menu_music, 1.0);
        soundplayer_play(background_menu_music);
    }
}

async function settingsmenu_in_save_settings(gamepad, modding) {
    const title = "Savedata";
    const options_help = [
        {
            name: "load-only",
            description: "Load savedata from another VMU, this replaces the current loaded savedata.\n" +
                "Any future saves will be stored in the selected VMU",
            hidden: false,
        },
        {
            name: "save-or-delete",
            description: "Save the current loaded savedata in another VMU.\n" +
                "Allows override or delete any other savedata.\n" +
                "Any future saves will be stored in the lastest used VMU for save",
            hidden: false,
        },
        {
            name: "return",
            description: "Go back to settings menu",
            hidden: false,
        },
    ];

    let layout = await layout_init(pvr_context_is_widescreen() ? SETTINGSMENU_LAYOUT_SAVEDATA : SETTINGSMENU_LAYOUT_SAVEDATA_DREAMCAST);
    if (!layout) {
        console.error("settingsmenu_in_save_settings() can not load the layout");
        return;
    }

    SETTINGSMENU_SAVEDATA.parameters.is_vertical = layout_get_attached_value(
        layout, "menu_isVertical", LAYOUT_TYPE_BOOLEAN, SETTINGSMENU_SAVEDATA.parameters.is_vertical
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

    let menu_placeholder = layout_get_placeholder(layout, "menu");
    if (!menu_placeholder) throw new Error("Missing menu placeholder");

    const menumanifest = SETTINGSMENU_SAVEDATA;
    let menu = await menu_init(
        menumanifest,
        menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
        menu_placeholder.width, menu_placeholder.height
    );
    menu_placeholder.vertex = menu_get_drawable(menu);

    // initialize with default values
    let savemanager = await savemanager_init(false, 0);

    L_menu:
    while (!modding.has_exit) {
        let selected_index = await settingsmenu_in_common_menu(title, layout, gamepad, menu, options_help, modding);
        let selected_name = selected_index < 0 ? null : menumanifest.items[selected_index].name;
        let save_only;

        switch (selected_name) {
            case "load-only":
                save_only = false;
                break;
            case "save-or-delete":
                save_only = true;
                break;
            case "return":
                break L_menu;
            default:
                // custom option selected
                if (settingsmenu_current_menu_choosen_custom == null) break L_menu;
                await modding_helper_notify_handle_custom_option(modding, settingsmenu_current_menu_choosen_custom);
                settingsmenu_current_menu_choosen_custom = undefined;
                settingsmenu_current_menu_choosen_custom = null;
                continue;
        }

        // only allow delete when saving
        savemanager_change_actions(savemanager, save_only, save_only, true);
        await savemanager_show(savemanager);
    }

    menu_destroy(menu);
    layout_destroy(layout);
    savemanager_destroy(savemanager);
}

async function settingsmenu_in_gameplay_binding(anim_binding, anim_binding_rollback) {
    let layout = await layout_init(pvr_context_is_widescreen() ? SETTINGSMENU_LAYOUT_BINDS_GAMEPLAY : SETTINGSMENU_LAYOUT_BINDS_GAMEPLAY_DREAMCAST);

    if (!layout) {
        console.error("settingsmenu_in_gameplay_binding() can not load the layout");
        return;
    }

    // binding colors
    let color_bg_key = layout_get_attached_value(layout, "color_bg_key", LAYOUT_TYPE_HEX, 0x000000FF);
    let color_bg_key_selected = layout_get_attached_value(layout, "color_bg_key_selected", LAYOUT_TYPE_HEX, 0xFF0000FF);
    let color_bg_key_active = layout_get_attached_value(layout, "color_bg_key_active", LAYOUT_TYPE_HEX, 0x00FF00FF);

    // aquire labels
    const labels = [
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
    ];

    // adquire backgrounds
    const backs = [
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
    ];

    // keys
    const keycodes = [
        "KeyF",
        "KeyG",
        "KeyJ",
        "KeyH",
        null,
        null,
        null,
        null,
        null,
        null,
        null,
        null,

        "SPACE"
    ];

    let binds_count = 13;
    const ini_entries = [
        "left1", "down1", "up1", "right1",
        "left2", "down2", "up2", "right2",
        "left3", "down3", "up3", "right3",
        "diamond"
    ];

    // load gameplay bindings
    for (let i = 0; i < binds_count; i++) {
        keycodes[i] = settingsmenu_internal_load_key(labels[i], ini_entries[i], keycodes[i]);
    }

    let row = 0, column = 0, selected_index = 0;
    let is_binding = false;

    // select first holder
    sprite_set_vertex_color_rgba8(backs[selected_index], color_bg_key_selected);

    // anti-bounce from parent menu
    settingsmenu_last_detected_keycode = null;
    settingsmenu_next_pressed_timestamp = (timer_ms_gettime64() / 1000.0) + SETTINGSMENU_DELAY_SECONDS;

    while (true) {
        let elapsed = await pvrctx_wait_ready();
        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        if (is_binding && anim_binding) {
            animsprite_animate(anim_binding, elapsed);
            animsprite_update_sprite(anim_binding, backs[selected_index], true);
        }

        let pressed = settingsmenu_internal_read_keyboard();

        if (is_binding) {
            let keycode = settingsmenu_last_detected_keycode;

            if (pressed != GAMEPAD_BACK) {
                if (pressed == GAMEPAD_START) {
                    keycodes[selected_index] = null;// clear binding
                } else if (keycode == null) {
                    continue;// keep waiting
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
            settingsmenu_last_detected_keycode = null;
            settingsmenu_internal_update_holder(
                backs, labels, keycodes, selected_index,
                anim_binding, false, color_bg_key_active
            );
            textsprite_set_text_intern(labels[selected_index], true, SETTINGSMENU_KEY_BIND);
            if (anim_binding) animsprite_restart(anim_binding);
            continue;
        }

        // calculate movement
        let new_row = row, new_column = column, new_index = selected_index;

        if (pressed == GAMEPAD_DPAD_LEFT) new_column--;
        if (pressed == GAMEPAD_DPAD_RIGHT) new_column++;
        if (pressed == GAMEPAD_DPAD_UP) new_row--;
        if (pressed == GAMEPAD_DPAD_DOWN) new_row++;

        // unselect current holder
        settingsmenu_internal_update_holder(
            backs, labels, keycodes, selected_index,
            null, false, color_bg_key
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
            null, false, color_bg_key_selected
        );
    }

    // save bindings
    for (let i = 0; i < binds_count; i++) {
        settingsmenu_internal_save_key(ini_entries[i], keycodes[i]);
    }

    layout_destroy(layout);
    KOS_MAPLE_KEYBOARD.reloadMappings();
}

async function settingsmenu_in_menus_binding(anim_binding, anim_binding_rollback) {
    let layout = await layout_init(pvr_context_is_widescreen() ? SETTINGSMENU_LAYOUT_BINDS_MENUS : SETTINGSMENU_LAYOUT_BINDS_MENUS_DREAMCAST);

    if (!layout) {
        console.error("settingsmenu_in_menus_binding() can not load the layout");
        return;
    }

    // binding colors
    let color_bg_key = layout_get_attached_value(layout, "color_bg_key", LAYOUT_TYPE_HEX, 0x000000FF);
    let color_bg_key_selected = layout_get_attached_value(layout, "color_bg_key_selected", LAYOUT_TYPE_HEX, 0xFF0000FF);
    let color_bg_key_active = layout_get_attached_value(layout, "color_bg_key_active", LAYOUT_TYPE_HEX, 0x00FF00FF);

    // aquire labels
    let labels = [
        layout_get_textsprite(layout, "label-accept"),
        layout_get_textsprite(layout, "label-selectorLeft"),
        layout_get_textsprite(layout, "label-selectorRight"),
        layout_get_textsprite(layout, "label-alternativeTracks")
    ];

    // adquire backgrounds
    let backs = [
        layout_get_sprite(layout, "back-accept"),
        layout_get_sprite(layout, "back-selectorLeft"),
        layout_get_sprite(layout, "back-selectorRight"),
        layout_get_sprite(layout, "back-alternativeTracks")
    ];

    let binds_count = 4;
    let ini_entries = [
        "menuAccept", "menuSelectorLeft", "menuSelectorRight", "menuAlternativeTracks"
    ];

    // key + scancodes
    let keycodes = [
        "KeyA",
        "Digit1",
        "Digit2",
        "KeyX"
    ];

    // load gameplay bindings
    for (let i = 0; i < binds_count; i++) {
        keycodes[i] = settingsmenu_internal_load_key(labels[i], ini_entries[i], keycodes[i]);
    }

    let selected_index = 0;
    let is_binding = false;

    // select first holder
    sprite_set_vertex_color_rgba8(backs[selected_index], color_bg_key_selected);

    // anti-bounce from parent menu
    settingsmenu_last_detected_keycode = null;
    settingsmenu_next_pressed_timestamp = (timer_ms_gettime64() / 1000.0) + SETTINGSMENU_DELAY_SECONDS;

    while (true) {
        let elapsed = await pvrctx_wait_ready();
        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        if (is_binding && anim_binding) {
            animsprite_animate(anim_binding, elapsed);
            animsprite_update_sprite(anim_binding, backs[selected_index], true);
        }

        let pressed = settingsmenu_internal_read_keyboard();

        if (is_binding) {
            let keycode = settingsmenu_last_detected_keycode;

            if (pressed != GAMEPAD_BACK) {
                if (pressed == GAMEPAD_START) {
                    keycodes[selected_index] = null;// clear binding
                } else if (keycode == null) {
                    continue;// keep waiting
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
            settingsmenu_last_detected_keycode = null;
            settingsmenu_internal_update_holder(
                backs, labels, keycodes, selected_index,
                anim_binding, false, color_bg_key_active
            );
            textsprite_set_text_intern(labels[selected_index], true, SETTINGSMENU_KEY_BIND);
            if (anim_binding) animsprite_restart(anim_binding);
            continue;
        }

        // calculate movement
        let new_index = selected_index;

        if (pressed == GAMEPAD_DPAD_UP) new_index--;
        if (pressed == GAMEPAD_DPAD_DOWN) new_index++;

        if (new_index < 0 || new_index >= binds_count) {
            continue;
        }

        // unselect current holder
        settingsmenu_internal_update_holder(
            backs, labels, keycodes, selected_index,
            null, false, color_bg_key
        );

        // select the new holder
        settingsmenu_internal_update_holder(
            backs, labels, keycodes, new_index,
            null, false, color_bg_key_selected
        );
        selected_index = new_index;
    }

    // save bindings
    for (let i = 0; i < binds_count; i++) {
        settingsmenu_internal_save_key(ini_entries[i], keycodes[i]);
    }
    layout_destroy(layout);

    KOS_MAPLE_KEYBOARD.reloadMappings();
}

async function settingsmenu_in_common_menu(title, layout, gamepad, menu, options, modding) {
    let hint = layout_get_textsprite(layout, "hint");
    gamepad_set_buttons_delay(gamepad, (SETTINGSMENU_DELAY_SECONDS * 1000));

    settingsmenu_current_menu = modding.native_menu = modding.active_menu = menu;
    settingsmenu_current_setting_options = options;
    settingsmenu_current_menu_choosen = false;

    layout_trigger_any(layout, null);
    await modding_helper_notify_event(modding, title);

    let last_selected_index = -1;
    let selected_index = menu_get_selected_index(menu);
    if (selected_index >= 0 && selected_index < menu_get_items_count(menu) && hint) {
        textsprite_set_text_intern(hint, true, options[selected_index].description);
        last_selected_index = selected_index;
        main_helper_trigger_action_menu2(layout, SETTINGSMENU_MENU_COMMON, selected_index, title, true, false);
        await modding_helper_notify_option(modding, true);
    }

    let option = -1;
    gamepad_clear_buttons(gamepad);

    while (!modding.has_exit) {
        if (settingsmenu_current_menu_choosen) {
            if (settingsmenu_current_menu_choosen_custom != null) option = menu_get_selected_index(menu);
            break;
        }

        let elapsed = await pvrctx_wait_ready();

        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        let buttons = gamepad_has_pressed_delayed(
            gamepad,
            GAMEPAD_AD |
            GAMEPAD_START | GAMEPAD_BACK |
            GAMEPAD_B | GAMEPAD_A
        );

        let res = await modding_helper_handle_custom_menu(modding, gamepad, elapsed);
        if (res != MODDING_HELPER_RESULT_CONTINUE) break;
        if (modding.has_halt || menu != modding.active_menu) continue;

        if ((buttons & (GAMEPAD_BACK | GAMEPAD_B)) && !await modding_helper_notify_back(modding)) {
            break;
        } else if ((buttons & (GAMEPAD_START | GAMEPAD_A)) && !await modding_helper_notify_option(modding, false)) {
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
            textsprite_set_text_intern(hint, true, options[selected_index].description);
        }

        main_helper_trigger_action_menu2(layout, SETTINGSMENU_MENU_COMMON, last_selected_index, title, false, false);
        last_selected_index = selected_index;
        main_helper_trigger_action_menu2(layout, SETTINGSMENU_MENU_COMMON, selected_index, title, true, false);
        await modding_helper_notify_option(modding, true);
    }

    if (option >= 0) {
        main_helper_trigger_action_menu2(layout, SETTINGSMENU_MENU_COMMON, option, title, false, true);
    }

    modding.has_exit = false;
    return option;
}

async function settingsmenu_in_gameplay_settings(gamepad, modding) {
    const title = "GAMEPLAY SETTINGS";
    const options = [
        {
            name: "USE FUNKIN MARKER DURATION",
            description: "Uses a 166.67ms marker duration instead of a pixel accurate duration.\n" +
                "ENABLED: the hit and rank calculation is tight like Funkin,\n" +
                "DISABLED: the engine calculate the duration according to the screen",
            is_bool: true,
            value_bool: true,
            ini_key: "use_funkin_marker_duration",
            hidden: false
        },
        {
            name: "PENALITY ON EMPTY STRUM",
            description: "Penalizes the player if press a button on empty strum.\n" +
                "ENABLED: the penality is enforced. DISABLED: allows ghost-tapping",
            is_bool: true,
            value_bool: true,
            ini_key: "penality_on_empty_strum",
            hidden: false
        },
        {
            name: "INPUT OFFSET",
            description: "Offets all button press timestamps. Defaults to 0 milliseconds\n" +
                "useful for bluetooth headset or television screens",
            is_int: true,
            value_int: 0,
            number_min: -1000,
            number_max: 1000,
            ini_key: "input_offset",
            hidden: false
        },
        {
            name: "INVERSE STRUM SCROLLING",
            description: "In the default UI layout this switch from UPSCROLL to DOWNSCROLL,\n" +
                "In custom UI layouts, reverses the declared direction",
            is_bool: true,
            value_bool: false,
            ini_key: "inverse_strum_scroll",
            hidden: false
        },
        {
            name: "SHOW SONG PROGRESS BAR",
            description: "Shows a progress bar near the strums makers.\n" +
                "Indicates playback progress of the playing song",
            is_bool: true,
            value_bool: true,
            ini_key: "song_progressbar",
            hidden: false
        },
        {
            name: "SONG PROGRESS BAR REMAINING",
            description: "Shows the remaining song duration instead of time elapsed",
            is_bool: true,
            value_bool: false,
            ini_key: "song_progressbar_remaining",
            hidden: false
        },
        {
            name: "DISTRACTIONS",
            description: "Enables/disables stage distractions during the gameplay.\n" +
                "(Depends on the week if honours this setting)",
            is_bool: true,
            value_bool: true,
            ini_key: "gameplay_enabled_distractions",
            hidden: false
        },
        {
            name: "FLASHING LIGHTS",
            description: "Rhythm games often use flashing lights, sometimes they can be annoying or cause seizures.\n" +
                "(Depends on the week if honours this setting)",
            is_bool: true,
            value_bool: true,
            ini_key: "gameplay_enabled_flashinglights",
            hidden: false
        },
        {
            name: "UI COSMETICS",
            description: "Hides away the accuracy, rank, and streak counters.\n" +
                "These elements normally are shown near or surrounding girlfriend's speakers",
            is_bool: true,
            value_bool: true,
            ini_key: "gameplay_enabled_uicosmetics",
            hidden: false
        },

    ];
    const options_size = options.length;

    // load current settings
    for (let i = 0; i < options_size; i++) {
        settingsmenu_internal_load_option(options[i], SETTINGS.INI_GAMEPLAY_SECTION);
    }

    await settingsmenu_show_common(title, gamepad, options, options_size, modding);

    // save settings
    for (let i = 0; i < options_size; i++) {
        settingsmenu_internal_save_option(options[i], SETTINGS.INI_GAMEPLAY_SECTION);
    }
}

async function settingsmenu_in_misc_settings(gamepad, modding) {
    const options = [
        {
            name: "DISPLAY FPS",
            description: "Displays the current FPS in the top-left corner of the screen.\n",
            is_bool: true,
            value_bool: false,
            ini_key: "show_fps",
            hidden: true
        },
        {
            name: "FPS LIMIT",
            description: "Sets the frame limiter. V-SYNC: uses the screen frame-rate (default).\n" +
                "DETERMINISTIC: for constant input checking with low CPU overhead.\n" +
                "OFF: no fps limit, high CPU usage.",
            is_list: true,
            value_list_index: 0,
            list: [
                { name: "V-SYNC", ini_value: "vsync" },
                { name: "Deterministic", ini_value: "deterministic" },
                { name: "Off", ini_value: "off" }
            ],
            list_size: 3,
            ini_key: "fps_limit",
            hidden: true
        },
        {
            name: "START IN FULLSCREEN",
            description: "Runs the game in fullscreen.\n" +
                "Press F11 to toggle between windowed/fullscreen",
            is_bool: true,
            value_bool: false,
            ini_key: "fullscreen",
            hidden: true
        },
        {
            name: "AVAILABLE SAVESLOTS",
            description: "Emulates the specified number of VMUs.\n" +
                "Defaults to 1, this applies before the engine runs",
            is_int: true,
            value_int: 1,
            number_min: 1,
            number_max: 8,
            ini_key: "saveslots",
            hidden: false
        },
        {
            name: "AUTOHIDE CURSOR",
            description: "Hides your mouse cursor after 3 seconds of inactivity.\n" +
                "The mouse is never used by the engine, but some weeks can make use of it",
            is_bool: true,
            value_bool: true,
            ini_key: "autohide_cursor",
            hidden: false
        },
        {
            name: "SILENCE ON MINIMIZE",
            description: "Mutes audio output when the window is minimized.\n" +
                "Freeplay menu is exempt",
            is_bool: true,
            value_bool: true,
            ini_key: "mute_on_minimize",
            hidden: true
        },
        {
            name: "SHOW LOADING SCREEN",
            description: "Displays a sprite after starting the week/song gameplay.\n" +
                "Only applicable in Freeplay menu and Week selector",
            is_bool: true,
            value_bool: false,
            ini_key: "show_loading_screen",
            hidden: false
        }
    ];
    const options_size = options.length;

    // load current settings
    for (let i = 0; i < options_size; i++) {
        settingsmenu_internal_load_option(options[i], SETTINGS.INI_MISC_SECTION);
    }

    await settingsmenu_show_common("MISCELLANEOUS", gamepad, options, options_size, modding);

    // save settings
    for (let i = 0; i < options_size; i++) {
        settingsmenu_internal_save_option(options[i], SETTINGS.INI_MISC_SECTION);
    }
}

async function settingsmenu_show_common(title, gamepad, options, options_count, modding) {
    let layout = await layout_init(pvr_context_is_widescreen() ? SETTINGSMENU_LAYOUT_COMMON : SETTINGSMENU_LAYOUT_COMMON_DREAMCAST);

    if (!layout) {
        console.error("settingsmenu_show_common() can not load the layout");
        return;
    }

    textsprite_set_text_intern(layout_get_textsprite(layout, "title"), true, title);

    let setting_name = layout_get_textsprite(layout, "setting-name");

    // settings menu
    let is_vertical = layout_get_attached_value(
        layout, "menu_isVertical", LAYOUT_TYPE_BOOLEAN, false
    );
    let font_size = layout_get_attached_value_as_float(
        layout, "menu_fontSize", 28.0
    );
    let items_gap = layout_get_attached_value_as_float(
        layout, "menu_itemGap", 56.0
    );
    let font_border_size = layout_get_attached_value_as_float(
        layout, "menu_fontBorderSize", 4.0
    );
    let menu_placeholder = layout_get_placeholder(layout, "menu");
    if (!menu_placeholder) throw new Error("Missing menu placeholder");

    // create menu options
    SETTINGSMENU_MENU_COMMON.parameters.is_vertical = is_vertical;
    SETTINGSMENU_MENU_COMMON.parameters.font_size = font_size;
    SETTINGSMENU_MENU_COMMON.parameters.items_gap = items_gap;
    SETTINGSMENU_MENU_COMMON.parameters.items_align = is_vertical ? menu_placeholder.align_vertical : menu_placeholder.align_horizontal;
    SETTINGSMENU_MENU_COMMON.parameters.anim_selected = "menu_common_item_selected";
    SETTINGSMENU_MENU_COMMON.parameters.anim_idle = "menu_common_item_idle";
    SETTINGSMENU_MENU_COMMON.parameters.font_border_size = font_border_size;
    SETTINGSMENU_MENU_COMMON.items_size = options_count;
    SETTINGSMENU_MENU_COMMON.items = malloc_for_array(options_count);

    for (let i = 0; i < options_count; i++) {
        SETTINGSMENU_MENU_COMMON.items[i] = {
            name: null,// unused
            text: options[i].name,
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: options[i].hidden,
            description: null// unused
        };
    }

    // initialize menu
    let menu = await menu_init(
        SETTINGSMENU_MENU_COMMON,
        menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
        menu_placeholder.width, menu_placeholder.height
    );
    menu_placeholder.vertex = menu_get_drawable(menu);

    // items are not longer required, dispose
    SETTINGSMENU_MENU_COMMON.items = undefined;
    SETTINGSMENU_MENU_COMMON.items = null;
    SETTINGSMENU_MENU_COMMON.items_size = 0;

    if (setting_name) textsprite_set_text_intern(setting_name, true, options[0].name);

    while (!modding.has_exit) {
        let selected_option = await settingsmenu_in_common_menu(title, layout, gamepad, menu, options, modding);
        if (selected_option < 0) break;

        if (setting_name) textsprite_set_text_intern(setting_name, true, options[selected_option].name);

        layout_set_group_visibility(layout, "holder-boolean", false);
        layout_set_group_visibility(layout, "holder-numeric", false);
        layout_set_group_visibility(layout, "holder-list", false);
        layout_set_group_visibility(layout, "change-value", true);

        if (options[selected_option].is_bool) {
            options[selected_option].value_bool = await settingsmenu_in_common_change_bool(
                layout, gamepad, options[selected_option].value_bool
            );
        } else if (options[selected_option].is_int) {
            options[selected_option].value_int = await settingsmenu_in_common_change_number(
                layout, gamepad, options[selected_option].number_min, options[selected_option].number_max, options[selected_option].value_int
            );
        } else if (options[selected_option].is_list) {
            options[selected_option].value_list_index = await settingsmenu_in_common_change_list(
                layout, gamepad, options[selected_option].list, options[selected_option].list_size, options[selected_option].value_list_index
            );
        }

        layout_set_group_visibility(layout, "change-value", false);
    }

    if (settingsmenu_current_menu_choosen_custom) {
        await modding_helper_notify_handle_custom_option(modding, settingsmenu_current_menu_choosen_custom);
    }
    settingsmenu_current_menu_choosen_custom = undefined;
    settingsmenu_current_menu_choosen_custom = null;

    menu_destroy(menu);
    layout_destroy(layout);
}

async function settingsmenu_in_common_change_number(layout, gamepad, min, max, value) {
    value = math2d_clamp_int(value, min, max);
    let orig_value = value;

    layout_set_group_visibility(layout, "holder-numeric", true);
    let number = layout_get_textsprite(layout, "number-value");

    textsprite_set_text_formated(number, "$i", value);
    gamepad_clear_buttons(gamepad);

    while (true) {
        let elapsed = await pvrctx_wait_ready();

        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        let buttons = gamepad_has_pressed_delayed(
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

async function settingsmenu_in_common_change_bool(layout, gamepad, value) {
    let orig_value = value;

    layout_set_group_visibility(layout, "holder-boolean", true);
    layout_trigger_any(layout, value ? "bool-true" : "bool-false");
    gamepad_clear_buttons(gamepad);

    while (true) {
        let elapsed = await pvrctx_wait_ready();

        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        let buttons = gamepad_has_pressed_delayed(
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

async function settingsmenu_in_common_change_list(layout, gamepad, list, list_size, index) {
    if (list_size < 1) return index;

    layout_set_group_visibility(layout, "holder-list", true);
    let list_selected_back = layout_get_sprite(layout, "list-selected-back");

    let is_vertical = layout_get_attached_value(
        layout, "menu-selector_isVertical", LAYOUT_TYPE_BOOLEAN, false
    );
    let font_size = layout_get_attached_value_as_float(
        layout, "menu-selector_fontSize", 28.0
    );
    let items_gap = layout_get_attached_value_as_float(
        layout, "menu-selector_itemGap", 56.0
    );

    let menu_placeholder = layout_get_placeholder(layout, "menu-selector");
    if (!menu_placeholder) throw new Error("Missing menu-selector placeholder");

    // create selector menu
    SETTINGSMENU_MENU_COMMON.parameters.font_border_size = 0.0;
    SETTINGSMENU_MENU_COMMON.parameters.is_vertical = is_vertical;
    SETTINGSMENU_MENU_COMMON.parameters.font_size = font_size;
    SETTINGSMENU_MENU_COMMON.parameters.items_gap = items_gap;
    SETTINGSMENU_MENU_COMMON.parameters.items_align = is_vertical ? menu_placeholder.align_vertical : menu_placeholder.align_horizontal;
    SETTINGSMENU_MENU_COMMON.parameters.anim_selected = "menu_selector_item_selected";
    SETTINGSMENU_MENU_COMMON.parameters.anim_idle = "menu_selector_item_idle";
    SETTINGSMENU_MENU_COMMON.items_size = list_size;
    SETTINGSMENU_MENU_COMMON.items = malloc_for_array(list_size);

    for (let i = 0; i < list_size; i++) {
        SETTINGSMENU_MENU_COMMON.items[i] = {
            name: null,// unused
            text: list[i].name,
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null// unused
        };
    }

    // initialize menu
    let menu = await menu_init(
        SETTINGSMENU_MENU_COMMON,
        menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
        menu_placeholder.width, menu_placeholder.height
    );
    menu_placeholder.vertex = menu_get_drawable(menu);

    // items are not longer required, dispose
    SETTINGSMENU_MENU_COMMON.items = undefined;
    SETTINGSMENU_MENU_COMMON.items = null;
    SETTINGSMENU_MENU_COMMON.items_size = 0;

    menu_select_index(menu, index);
    gamepad_clear_buttons(gamepad);

    let first_run = true;
    let border_size = font_size * 1.25;

    while (true) {
        let elapsed = await pvrctx_wait_ready();

        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        let buttons = gamepad_has_pressed_delayed(
            gamepad,
            GAMEPAD_A | GAMEPAD_B |
            GAMEPAD_START | GAMEPAD_BACK |
            GAMEPAD_AD
        );

        if (buttons & (GAMEPAD_A | GAMEPAD_START)) {
            index = menu_get_selected_index(menu);
            console.assert(index >= 0 && index < menu_get_items_count(menu));
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

        if (list_selected_back == null) continue;

        let output_location = [0, 0], output_size = [0, 0];
        if (menu_get_selected_item_rect(menu, output_location, output_size)) {
            output_location[0] -= border_size;
            output_location[1] -= border_size;
            output_size[0] += border_size * 2.0;
            output_size[1] += border_size * 2.0;

            sprite_set_draw_location(list_selected_back, output_location[0], output_location[1]);
            sprite_set_draw_size(list_selected_back, output_size[0], output_size[1]);
            sprite_set_visible(list_selected_back, true);
        } else {
            sprite_set_visible(list_selected_back, false);
        }

        first_run = false;
    }

    menu_destroy(menu);
    return index;
}


function settingsmenu_internal_save_key(settings_entry, keycode) {
    SETTINGS.storage_save_string(SETTINGS.INI_BINDING_SECTION, settings_entry, keycode);
}

function settingsmenu_internal_load_key(label, settings_entry, default_keycode) {
    let keycode = SETTINGS.storage_load_string(SETTINGS.INI_BINDING_SECTION, settings_entry, default_keycode);

    settingsmenu_internal_set_key_in_label(label, keycode);
    return keycode;
}

function settingsmenu_internal_set_key_in_label(label, keycode) {
    let key_name = keycode ?? SETTINGSMENU_KEY_NONE;
    textsprite_set_text_intern(label, true, key_name);
}

function settingsmenu_internal_read_keyboard() {
    let now = (timer_ms_gettime64() / 1000.0);
    if (now < settingsmenu_next_pressed_timestamp) return 0x00;

    let buttons = 0x00;

    if (settingsmenu_last_detected_keycode == "ArrowLeft")
        buttons |= GAMEPAD_DPAD_LEFT;
    if (settingsmenu_last_detected_keycode == "ArrowDown")
        buttons |= GAMEPAD_DPAD_DOWN;
    if (settingsmenu_last_detected_keycode == "ArrowUp")
        buttons |= GAMEPAD_DPAD_UP;
    if (settingsmenu_last_detected_keycode == "ArrowRight")
        buttons |= GAMEPAD_DPAD_RIGHT;
    if (settingsmenu_last_detected_keycode == "Enter")
        buttons |= GAMEPAD_START;
    if (settingsmenu_last_detected_keycode == "Escape")
        buttons |= GAMEPAD_BACK;

    if (buttons)
        settingsmenu_next_pressed_timestamp = now + SETTINGSMENU_DELAY_SECONDS;

    return buttons;
}

function settingsmenu_internal_key_callback(/**@type {KeyboardEvent}*/evt) {
    if (evt.type == "keydown")
        settingsmenu_last_detected_keycode = evt.code;
    else
        settingsmenu_last_detected_keycode = null;
}

function settingsmenu_internal_rollback_anim(back, rollback) {
    if (!rollback) return;
    animsprite_restart(rollback);
    animsprite_force_end(rollback);
    animsprite_update_sprite(rollback, back, true);
}

function settingsmenu_internal_update_holder(backs, labels, keycodes, index, anim, rollback, color) {
    if (rollback) settingsmenu_internal_rollback_anim(backs[index], anim);
    sprite_set_vertex_color_rgba8(backs[index], color);
    settingsmenu_internal_set_key_in_label(labels[index], keycodes[index]);
}


function settingsmenu_internal_save_option(option, ini_section) {
    if (option.is_bool) {
        SETTINGS.storage_save_boolean(ini_section, option.ini_key, option.value_bool);
    } else if (option.is_int) {
        SETTINGS.storage_save_number(ini_section, option.ini_key, option.value_int);
    } else if (option.is_list) {
        let value_string = option.list[option.value_list_index].ini_value;
        SETTINGS.storage_save_string(ini_section, option.ini_key, value_string);
    }
}

function settingsmenu_internal_load_option(option, ini_section) {
    if (option.is_bool) {
        option.value_bool = SETTINGS.storage_load_boolean(
            ini_section, option.ini_key, option.value_bool
        );
    } else if (option.is_int) {
        option.value_int = SETTINGS.storage_load_number(
            ini_section, option.ini_key, option.value_int
        );
    } else if (option.is_list) {
        let value_string = option.list[option.value_list_index].ini_value;
        value_string = SETTINGS.storage_load_string(
            ini_section, option.ini_key, value_string
        );

        for (let i = 0; i < option.list_size; i++) {
            if (value_string == option.list[i].ini_value) {
                option.value_list_index = i;
                break;
            }
        }
    }
}

function settingsmenu_internal_handle_option(obj, option_name) {
    if (settingsmenu_current_menu == null || settingsmenu_current_setting_options == null) {
        return false;
    }

    settingsmenu_current_menu_choosen_custom = undefined;

    settingsmenu_current_menu_choosen = true;
    if (option_name == null) {
        settingsmenu_current_menu_choosen_custom = null;
        return false;
    }

    if (menu_has_item(settingsmenu_current_menu, option_name)) {
        menu_select_item(settingsmenu_current_menu, option_name);
        settingsmenu_current_menu_choosen = true;
        return true;
    } else {
        for (let i = 0, count = menu_get_items_count(settingsmenu_current_menu); i < count; i++) {
            if (settingsmenu_current_setting_options[i].name == option_name) {
                menu_select_index(settingsmenu_current_menu, i);
                return true;
            }
        }
    }

    settingsmenu_current_menu_choosen_custom = strdup(option_name);
    return false;
}
