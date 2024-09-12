#include "externals/luascript_platform.h"

#include <arch/arch.h>
#include <dc/flashrom.h>
#include <dc/syscalls.h>

#include "arraypointerlist.h"
#include "externals/luascript.h"
#include "logger.h"
#include "math2d.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "stringutils.h"


static ArrayPointerList luascript_instances;
// static float64 last_mouse_position_x = DOUBLE_NaN;
// static float64 last_mouse_position_y = DOUBLE_NaN;
static bool last_window_focused = true;
// static bool last_window_minimized = false;
static int32_t last_resolution_changes = 0;
// static bool title_was_changed = false;


// from <stdio.h>
extern int snprintf(char* str, size_t n, const char* format, ...);


static void __attribute__((constructor)) ctor() {
    luascript_instances = arraypointerlist_init2(4);
}


void luascript_platform_add_luascript(Luascript luascript) {
    if (!luascript || arraypointerlist_has(luascript_instances, luascript)) return;
    arraypointerlist_add(luascript_instances, luascript);

    /*
    // notify input states if they do not have default values
    if (!math2d_is_double_NaN(last_mouse_position_x) && !math2d_is_double_NaN(last_mouse_position_y)) {
        luascript_notify_input_mouse_position(luascript, last_mouse_position_x, last_mouse_position_y);
    }*/
    if (!last_window_focused) luascript_notify_modding_window_focus(luascript, last_window_focused);
    // if (last_window_minimized) luascript_notify_modding_window_focus(luascript, last_window_minimized);
}

void luascript_platform_remove_luascript(Luascript luascript) {
    arraypointerlist_remove(luascript_instances, luascript);
}

void luascript_platform_poll_window_state() {
    bool focused = !pvr_context_is_offscreen(&pvr_context);
    // bool minimized = pvr_context_is_minimized(&pvr_context);
    int32_t resolution_changes = pvr_context.resolution_changes;

    bool focused_updated = focused != last_window_focused;
    // bool minimize_updated = minimized != last_window_minimized;
    bool resolution_updated = resolution_changes != last_resolution_changes;

    if (focused_updated /* || minimize_updated*/ || resolution_updated) {
        Luascript* array = arraypointerlist_peek_array(luascript_instances);
        int32_t size = arraypointerlist_size(luascript_instances);
        for (int32_t i = 0; i < size; i++) {
            Luascript luascript = array[i];
            if (focused_updated) luascript_notify_modding_window_focus(luascript, focused);
            // if (minimize_updated) luascript_notify_modding_window_minimized(luascript, minimized);
            if (resolution_updated) luascript_notify_window_size_changed(luascript, pvr_context.screen_width, pvr_context.screen_height);
        }

        last_window_focused = focused;
        // last_window_minimized = minimized;
    }
}

void luascript_platform_restore_window_title_if_changed() {
    /*
    if (!title_was_changed) return;
    pvr_context.native_window_title = NULL;// restore original title
    */
}


void luascript_platform_open_www_link(const char* url) {
    if (string_is_empty(url)) return;
    logger_info("luascript_platform_open_www_link() is not available in the current platform");
}

void luascript_platform_request_window_attention() {
    logger_info("luascript_platform_request_window_attention() is not available in the current platform");
}

void luascript_platform_change_window_title(const char* title, bool changed_from_modding_context) {
    (void)title;
    (void)changed_from_modding_context;

    /*if (title == NULL) title = pvr_context.native_window_title;

    if (!changed_from_modding_context && title != NULL)
        title_was_changed = true;
    else if (title == NULL)
        title_was_changed = false;*/

    logger_info("luascript_platform_request_window_attention() is not available in the current platform");
}

const char* luascript_platform_get_language() {
    flashrom_syscfg_t syscfg;
    flashrom_get_syscfg(&syscfg);

    switch (syscfg.language) {
        case FLASHROM_LANG_JAPANESE:
            return "ja";
        case FLASHROM_LANG_ENGLISH:
            return "en";
        case FLASHROM_LANG_GERMAN:
            return "de";
        case FLASHROM_LANG_FRENCH:
            return "fr";
        case FLASHROM_LANG_SPANISH:
            return "es";
        case FLASHROM_LANG_ITALIAN:
            return "it";
    }

    return "<unknown>";
}

const char* luascript_platform_get_username() {
    uint64_t hwid = syscall_sysinfo_id();
    static char fake_username[56];

#ifdef _arch_dreamcast
    int len = snprintf(fake_username, sizeof(fake_username), "dreamcast [HWID=%016llx]", hwid);
#else
    int len = snprintf(fake_username, sizeof(fake_username), "dreamcast [HWID=%016lx]", hwid);
#endif

    if (len < 1 || len >= sizeof(fake_username)) {
        return NULL;
    }

    fake_username[len] = '\0';
    return fake_username;
}

void luascript_platform_exit(int exit_code) {
    arch_return(exit_code);
}

void luascript_platform_get_screen_size(int32_t* screen_width, int32_t* screen_height) {
    *screen_width = pvr_context.screen_width;
    *screen_height = pvr_context.screen_height;
}
