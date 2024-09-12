#include "game/screenmenu.h"

#include "externals/luascript.h"
#include "game/common/menu.h"
#include "game/gameplay/weekscript.h"
#include "game/mainmenu.h"
#include "game/modding.h"
#include "game/savemanager.h"
#include "gamepad.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"


struct ScreenMenu_s {
    Modding modding;
    Layout layout;
};


static const float SCREENMENU_BUTTONS_DELAY = 200.0f;
static const GamepadButtons SCREENMENU_BACK_BUTTONS = GAMEPAD_BACK | GAMEPAD_B;


ScreenMenu screenmenu_init(const char* layout_src, const char* script_src) {
    Layout layout = layout_init(layout_src);
    if (!layout) {
        logger_error("screenmenu_init() failed to load layout: %s", layout_src);
        return NULL;
    }

    Modding modding = modding_init(layout, script_src);
    modding->exit_delay_ms = layout_get_attached_value_as_float(layout, "exit_delay", 0.0f);

    ScreenMenu screenmenu = malloc_chk(sizeof(struct ScreenMenu_s));
    malloc_assert(screenmenu, ScreenMenu);

    *screenmenu = (struct ScreenMenu_s){
        .modding = modding,
        .layout = layout
    };

    return screenmenu;
}

void screenmenu_destroy(ScreenMenu* screenmenu_ptr) {
    if (!screenmenu_ptr || !*screenmenu_ptr) return;

    ScreenMenu screenmenu = *screenmenu_ptr;

    layout_destroy(&screenmenu->layout);
    modding_destroy(&screenmenu->modding);

    free_chk(screenmenu);
    *screenmenu_ptr = NULL;
}

void* screenmenu_display(ScreenMenu screenmenu, PVRContext pvrctx, const void* script_arg, const ModdingValueType script_arg_type, ModdingValueType* ret_type) {
    Luascript script = screenmenu->modding->script ? weekscript_get_luascript(screenmenu->modding->script) : NULL;
    Layout layout = screenmenu->layout;

    if (script) {
        luascript_notify_modding_init(script, script_arg, script_arg_type);
        if (screenmenu->modding->has_exit) return luascript_notify_modding_exit(script, ret_type);
    }

    float active_gamepad_delay = 0.0f;
    GamepadButtons last_pressed = 0x00;
    Gamepad gamepad = gamepad_init(-1);
    gamepad_clear_buttons(gamepad);

    while (!screenmenu->modding->has_exit) {
        float elapsed = pvrctx_wait_ready();
        pvr_context_reset(pvrctx);

        GamepadButtons pressed = gamepad_get_pressed(gamepad);
        bool back_pressed = (pressed & SCREENMENU_BACK_BUTTONS) != 0x00;

        // ignore back buttons if halt flag not is signalated and call "f_modding_back" instead
        if (back_pressed && !screenmenu->modding->has_halt) pressed &= ~SCREENMENU_BACK_BUTTONS;

        if (script) {
            if (last_pressed != pressed) {
                last_pressed = pressed;
                weekscript_notify_buttons(screenmenu->modding->script, -1, pressed);
            }
            weekscript_notify_frame(screenmenu->modding->script, elapsed);
        }


        if (screenmenu->modding->has_halt) goto L_layout;

        if (active_gamepad_delay > 0.0f) {
            active_gamepad_delay -= elapsed;
            if (active_gamepad_delay > 0.0f) goto L_layout;
        }

        Menu menu = screenmenu->modding->active_menu;

        bool go_back = false;
        bool has_selected = false;
        bool has_choosen = false;

        if (back_pressed)
            go_back = true;
        else if (!menu) {
            goto L_layout;
        } else if ((pressed & GAMEPAD_DALL_LEFT) != 0x00)
            has_selected = menu_select_horizontal(menu, -1);
        else if ((pressed & GAMEPAD_DALL_RIGHT) != 0x00)
            has_selected = menu_select_horizontal(menu, 1);
        else if ((pressed & GAMEPAD_DALL_UP) != 0x00)
            has_selected = menu_select_vertical(menu, -1);
        else if ((pressed & GAMEPAD_DALL_DOWN) != 0x00)
            has_selected = menu_select_vertical(menu, 1);
        else if ((pressed & MAINMENU_GAMEPAD_OK) != 0x00)
            has_choosen = menu_get_selected_index(menu) >= 0;


        if (go_back) {
            if (!script) break;
            if (!luascript_notify_modding_back(script)) break;
            active_gamepad_delay = SCREENMENU_BUTTONS_DELAY;
        }

        if (has_selected) {
            if (!script) {
                int32_t index = menu_get_selected_index(menu);
                const char* name = menu_get_selected_item_name(menu);
                luascript_notify_modding_menu_option_selected(script, menu, index, name);
            }
            active_gamepad_delay = SCREENMENU_BUTTONS_DELAY;
        }

        if (has_choosen) {
            if (!script) break;

            int32_t index = menu_get_selected_index(menu);
            const char* name = menu_get_selected_item_name(menu);

            if (!luascript_notify_modding_menu_option_choosen(script, menu, index, name)) {
                menu_toggle_choosen(menu, true);
                break;
            }

            active_gamepad_delay = SCREENMENU_BUTTONS_DELAY;
        }

    L_layout:
        layout_animate(layout, elapsed);
        layout_draw(layout, pvrctx);
    }

    // if there no script ¿exit_value should be NULL or pick the menu selected option name?
    void* exit_value = NULL;
    ModdingValueType exit_value_type = ModdingValueType_null;
    if (script) exit_value = luascript_notify_modding_exit(script, &exit_value_type);

    // delay exit (if applicable)
    float64 exit_delay = screenmenu->modding->exit_delay_ms;
    while (exit_delay > 0.0) {
        float elapsed = pvrctx_wait_ready();

        exit_delay -= elapsed;
        pvr_context_reset(pvrctx);

        if (script) {
            weekscript_notify_frame(screenmenu->modding->script, elapsed);
        }

        layout_animate(layout, elapsed);
        layout_draw(layout, pvrctx);
    }

    // flush framebuffer again with last fade frame
    pvrctx_wait_ready();

    // do antibounce before return (just in case)
    gamepad_clear_all_gamepads();
    gamepad_destroy(&gamepad);

    // write any unsaved changes back to the VMU
    savemanager_check_and_save_changes();

    return exit_value;
}
