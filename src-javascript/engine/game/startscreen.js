"use strict";

const STARTSCREEN_LAYOUT = "/assets/common/image/start-screen/layout.xml";
const STARTSCREEN_LAYOUT_DREAMCAST = "/assets/common/image/start-screen/layout~dreamcast.xml";

async function startscreen_main() {
    let soundplayer_confirm = await soundplayer_init("/assets/common/sound/confirmMenu.ogg");
    let maple_pad = gamepad_init(-1);
    let layout = await layout_init(pvrctx_output_is_widescreen() ? STARTSCREEN_LAYOUT : STARTSCREEN_LAYOUT_DREAMCAST);

    let total_elapsed = 0;
    let enter_pressed = 0;
    let trigger_fade_away = 1;
    let exit_to_bios = 0;
    
    gamepad_clear_buttons(maple_pad);

    while (1) {
        let elapsed = await pvrctx_wait_ready();

        layout_animate(layout, elapsed);

        if (!enter_pressed && gamepad_has_pressed(maple_pad, GAMEPAD_START | GAMEPAD_A)) {
            enter_pressed = 1;
            if (soundplayer_confirm) soundplayer_play(soundplayer_confirm);
            layout_trigger_any(layout, "start_pressed");
        } else if (enter_pressed) {
            if (total_elapsed >= 2000) {
                if (trigger_fade_away) {
                    trigger_fade_away = 0;
                    layout_trigger_any(layout, "outro");
                } else {
                    let state = layout_animation_is_completed(layout, "transition-intro-outro-effect");

                    if (state == -1) break;// infinite looped animation

                    // check if:
                    //          the animation is completed (1)
                    //          there no animation on the sprite (2)
                    //          there no sprite in the layout with that name (3)
                    else if (state != 0) break;
                }
            }

            total_elapsed += elapsed;
        } else if (gamepad_has_pressed(maple_pad, GAMEPAD_B | GAMEPAD_BACK)) {
            layout_trigger_any(layout, "outro");
            enter_pressed = 1;
            exit_to_bios = 1;
        }

        pvr_context_reset(pvr_context);
        layout_draw(layout, pvr_context);
    };

    layout_destroy(layout);
    gamepad_destroy(maple_pad);
    if (soundplayer_confirm) soundplayer_destroy(soundplayer_confirm);

    if (exit_to_bios) {
        // boot dreamcast BIOS menu
        if (background_menu_music) soundplayer_stop(background_menu_music);
        arch_menu();
        return 1;
    }

    return 0;
}

