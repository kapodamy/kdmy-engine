#include "game/credits.h"

#include <arch/timer.h>

#include "fs.h"
#include "game/gameplay/weekscript.h"
#include "game/modding.h"
#include "gamepad.h"
#include "layout.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "soundplayer.h"


static const char* CREDITS_LAYOUT_WIDESCREEN = "/assets/common/credits/layout.xml";
static const char* CREDITS_LAYOUT_DREAMCAST = "/assets/common/credits/layout~dreamcast.xml";


void credits_main() {
    GamepadButtons buttons = 0x0000;
    const char* src;
    if (!pvr_context_is_widescreen() && fs_file_exists(CREDITS_LAYOUT_DREAMCAST))
        src = CREDITS_LAYOUT_DREAMCAST;
    else
        src = CREDITS_LAYOUT_WIDESCREEN;

    Layout layout = layout_init(src);
    if (!layout) return;

    Modding moddingcontext = modding_init(layout, "/assets/common/credits/credits.lua");
    Gamepad gamepad = gamepad_init(-1);
    float64 end_timestamp_base = -1.0;
    float64 end_timestamp_before_shoot = -1.0;
    float64 end_timestamp_shoot = -1.0;
    float64 end_timestamp_ending = -1.0;
    float64 fade_duration = 0.0;
    bool is_scripted = true;
    int32_t state = 0;

    if (!moddingcontext->script) {
        end_timestamp_base = layout_get_attached_value_as_float(layout, "base_duration", 25000.0f);
        end_timestamp_before_shoot = layout_get_attached_value_as_float(layout, "before_shoot_duration", 5000.0f);
        end_timestamp_shoot = layout_get_attached_value_as_float(layout, "shoot_duration", 3000.0f);
        end_timestamp_ending = layout_get_attached_value_as_float(layout, "after_shoot_duration", 5000.0f);

        uint64_t timestamp = timer_ms_gettime64();
        is_scripted = false;
        fade_duration = end_timestamp_ending;

        end_timestamp_base += timestamp;
        end_timestamp_before_shoot += end_timestamp_base;
        end_timestamp_shoot += end_timestamp_before_shoot;
        end_timestamp_ending += end_timestamp_shoot;
    }

    gamepad_clear_buttons(gamepad);

    while (!moddingcontext->has_exit) {
        float elapsed = pvrctx_wait_ready();
        pvr_context_reset(&pvr_context);

        if (gamepad_get_managed_presses(gamepad, true, &buttons)) {
            if (moddingcontext->script) weekscript_notify_buttons(moddingcontext->script, -1, buttons);
        }
        if (moddingcontext->script) weekscript_notify_frame(moddingcontext->script, elapsed);

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        if (is_scripted) continue;

        uint64_t timestamp = timer_ms_gettime64();

        if (state != 3 && gamepad_has_pressed(gamepad, CREDITS_BUTTONS)) {
            if (state < 3) {
                timestamp = MATH2D_MAX_INT32;
                state = 3;
                gamepad_clear_buttons(gamepad);
            } else {
                break;
            }
        }

        switch (state) {
            case 0:
                if (timestamp >= end_timestamp_base) {
                    layout_trigger_any(layout, "next");
                    state = 1;
                }
                break;
            case 1:
                if (timestamp > end_timestamp_before_shoot) {
                    layout_trigger_any(layout, "before_shoot");
                    state = 2;
                }
                break;
            case 2:
                if (timestamp > end_timestamp_shoot) {
                    layout_trigger_any(layout, "shoot");
                    state = 3;
                }
                break;
            case 3:
                if (timestamp > end_timestamp_ending) {
                    layout_trigger_any(layout, "outro");
                    SoundPlayer soundplayer = layout_get_soundplayer(layout, "bg-music");
                    if (soundplayer) soundplayer_fade(soundplayer, false, (float)fade_duration);
                    state = 4;
                }
                // fall through
            case 4:
                if (layout_animation_is_completed(layout, "transition_effect")) {
                    break;
                }
                break;
        }
    }

    modding_destroy(&moddingcontext);
    gamepad_destroy(&gamepad);
    layout_destroy(&layout);
}
