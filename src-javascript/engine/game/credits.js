"use strict";

const CREDITS_BUTTONS = GAMEPAD_START | GAMEPAD_A | GAMEPAD_B | GAMEPAD_X | GAMEPAD_Y | GAMEPAD_BACK;
const CREDITS_LAYOUT_WIDESCREEN = "/assets/common/credits/layout.xml";
const CREDITS_LAYOUT_DREAMCAST = "/assets/common/credits/layout~dreamcast.xml";

async function credits_main() {
    const buttons = [0x0000];
    let src;
    if (!pvrctx_is_widescreen() && await fs_file_exists(CREDITS_LAYOUT_DREAMCAST))
        src = CREDITS_LAYOUT_DREAMCAST;
    else
        src = CREDITS_LAYOUT_WIDESCREEN;

    let layout = await layout_init(src);
    if (!layout) return;

    let moddingcontext = await modding_init(layout, "/assets/common/credits/credits.lua");
    let gamepad = gamepad_init(-1);
    let end_timestamp_base = -1;
    let end_timestamp_before_shoot = -1;
    let end_timestamp_shoot = -1;
    let end_timestamp_ending = -1;
    let fade_duration = 0;
    let is_scripted = 1;
    let state = 0;

    if (moddingcontext.script == null) {
        end_timestamp_base = layout_get_attached_value_as_float(layout, "base_duration", 25000);
        end_timestamp_before_shoot = layout_get_attached_value_as_float(layout, "before_shoot_duration", 5000);
        end_timestamp_shoot = layout_get_attached_value_as_float(layout, "shoot_duration", 3000);
        end_timestamp_ending = layout_get_attached_value_as_float(layout, "after_shoot_duration", 5000);

        let timestamp = timer_ms_gettime64();
        is_scripted = 0;
        fade_duration = end_timestamp_ending;

        end_timestamp_base += timestamp;
        end_timestamp_before_shoot += end_timestamp_base;
        end_timestamp_shoot += end_timestamp_before_shoot;
        end_timestamp_ending += end_timestamp_shoot;
    }

    gamepad_clear_buttons(gamepad);

    while (!moddingcontext.has_exit) {
        let elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);

        if (gamepad_get_managed_presses(gamepad, 1, buttons)) {
            if (moddingcontext.script) await weekscript_notify_buttons(moddingcontext.script, -1, buttons);
        }
        if (moddingcontext.script) await weekscript_notify_frame(moddingcontext.script, elapsed);

        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        if (is_scripted) continue;

        let timestamp = timer_ms_gettime64();

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
                    let soundplayer = layout_get_soundplayer("bg-music");
                    if (soundplayer) soundplayer_fade(soundplayer, 0, fade_duration);
                    state = 4;
                }
            case 4:
                if (layout_animation_is_completed(layout, "transition_effect")) {
                    break;
                }
                break;
        }
    }

    modding_destroy(moddingcontext);
    gamepad_destroy(gamepad);
    layout_destroy(layout);
}

