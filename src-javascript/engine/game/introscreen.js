"use strict";

const INTROSCREEN_DELAY = 600;
const INTROSCREEN_TEXT_SPARSE = "--";
const INTROSCREEN_FUNKY = "Friday--Night--Funkin";
const INTROSCREEN_LAYOUT = "/assets/common/image/intro-screen/layout.xml";
const INTROSCREEN_LAYOUT_DREAMCAST = "/assets/common/image/intro-screen/layout~dreamcast.xml";
const INTROSCREEN_SKIP_BUTTONS = GAMEPAD_A | GAMEPAD_START;
const INTROSCREEN_MODDING_SCRIPT = "/assets/common/data/scripts/introscreen.lua";


async function introscreen_main() {
    let layout = await layout_init(pvrctx_is_widescreen() ? INTROSCREEN_LAYOUT : INTROSCREEN_LAYOUT_DREAMCAST);
    if (!layout) return;

    let modding = await modding_init(layout, INTROSCREEN_MODDING_SCRIPT);
    let maple_pad = gamepad_init(-1);
    gamepad_clear_buttons(maple_pad);

    await modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN);

    // read delay/durations from layout
    let delay = layout_get_attached_value_as_float(layout, "delay", INTROSCREEN_DELAY);
    let custom_duration = layout_get_attached_value_as_float(
        layout, "custom_duration", 0.0
    );

    if (custom_duration > 0) {
        await modding_helper_notify_event(modding, "custom-intro");

        // custom intro detected wait the requeted time
        let progress = 0;
        while (progress < custom_duration) {
            let elapsed = await pvrctx_wait_ready();
            pvr_context_reset(pvr_context);

            let res = await modding_helper_handle_custom_menu(modding, maple_pad, elapsed);
            if (modding.has_exit || res != MODDING_HELPER_RESULT_CONTINUE) break;

            layout_animate(layout, elapsed);
            layout_draw(layout, pvr_context);
            if (modding.has_halt) continue;

            progress += elapsed;
            if (gamepad_has_pressed(maple_pad, INTROSCREEN_SKIP_BUTTONS) != 0x00) break;
        }

        await modding_helper_notify_exit2(modding);

        await modding_destroy(modding);
        layout_destroy(layout);
        gamepad_destroy(maple_pad);
        return;
    }

    let self_text = await fs_readtext("/assets/engineText.txt");
    let intro_text = await introscreen_read_intro_text("/assets/common/data/introText.txt");
    let week_greetings = null;
    let funky_intro = layout_get_attached_value(layout, "funky", LAYOUT_TYPE_STRING, INTROSCREEN_FUNKY);

    // 25% chance of displaying choosen week greetings
    if (Math.random() <= 0.25) week_greetings = await introscreen_read_week_gretings();

    // if there an camera animation called "camera_animation", use it
    layout_trigger_camera(layout, "camera_animation");

    let engine_duration = delay * 4;
    let greetings_duration = delay * 4;
    let funkin_duration = delay * 5;

    // start this game
    if (background_menu_music) soundplayer_play(background_menu_music);

    layout_trigger_any(layout, "intro-engine");
    await modding_helper_notify_event(modding, "intro-engine");
    await introscreen_draw_sparse_text(self_text, delay, engine_duration, modding, maple_pad);

    if (week_greetings) {
        layout_trigger_any(layout, "intro-week-grettings");
        await modding_helper_notify_event(modding, "intro-week-grettings");
        await introscreen_draw_sparse_text(week_greetings, delay, greetings_duration, modding, maple_pad);
    } else {
        layout_trigger_any(layout, "intro-greetings");
        await modding_helper_notify_event(modding, "intro-greetings");
        await introscreen_draw_sparse_text(intro_text, delay, greetings_duration, modding, maple_pad);
    }

    layout_trigger_any(layout, "intro-funkin");
    await modding_helper_notify_event(modding, "intro-funkin");
    await introscreen_draw_sparse_text(funky_intro, delay, funkin_duration, modding, maple_pad);

    layout_trigger_any(layout, "transition-out");
    await modding_helper_notify_event(modding, "transition-out");
    while (1) {
        let elapsed = await pvrctx_wait_ready();
        modding_helper_notify_frame(modding, elapsed, -1.0);

        pvr_context_reset(pvr_context);
        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        if (layout_animation_is_completed(layout, "transition_effect") > 0) {
            // flush framebuffer again with last fade frame
            await pvrctx_wait_ready();
            break;
        }
    }

    await modding_helper_notify_exit2(modding);

    // dispose resources used
    if (funky_intro !== INTROSCREEN_FUNKY) funky_intro = undefined;
    self_text = undefined;
    intro_text = undefined;
    week_greetings = undefined;
    await modding_destroy(modding);
    layout_destroy(layout);
    gamepad_destroy(maple_pad);
}

async function introscreen_pause(duration, layout) {
    let total_elapsed = 0;
    while (total_elapsed < duration) {
        let elapsed = await pvrctx_wait_ready();
        total_elapsed += elapsed;

        pvr_context_reset(pvr_context);
        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);
    }
}

async function introscreen_draw_sparse_text(text, delay, duration, modding, maple_pad) {
    const layout = modding.layout;
    let textsprite = layout_get_textsprite(layout, "greetings");
    if (!textsprite || !text || modding.has_exit) return;

    let text_length = text.length;
    let text_buffer = "";
    let lines = string_occurrences_of_string(text, INTROSCREEN_TEXT_SPARSE) + 1;
    let paragraph_duration = duration / lines;
    let progress = paragraph_duration - delay;
    let sparse_length = INTROSCREEN_TEXT_SPARSE.length;
    let last_index = 0;

    while (last_index < text_length) {
        if (progress >= paragraph_duration) {
            if (last_index >= text_length) break;
            if (last_index > 0) text_buffer += "\n";

            let index = text.indexOf(INTROSCREEN_TEXT_SPARSE, last_index);
            if (index < 0) index = text_length;

            text_buffer += text.substring(last_index, index);
            last_index = index + sparse_length;

            textsprite_set_text_intern(textsprite, 1, text_buffer);

            progress -= paragraph_duration;
        }

        let elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);

        let res = await modding_helper_handle_custom_menu(modding, maple_pad, elapsed);
        if (modding.has_exit || res != MODDING_HELPER_RESULT_CONTINUE) break;

        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);
        if (modding.has_halt) continue;

        progress += elapsed;
        if (gamepad_has_pressed(maple_pad, INTROSCREEN_SKIP_BUTTONS) != 0x00) break;
    }

    text_buffer = undefined;
}

async function introscreen_read_intro_text(path) {
    let lines = await fs_readtext(path);
    let line_count = 0;

    if (lines == null) return null;

    // count lines in the file
    let index = 0;
    while (1) {
        line_count++;
        index = lines.indexOf("\n", index);
        if (index < 0) break;
        index++;
    }

    // choose a random line
    let random_index = math2d_random(1, line_count);
    line_count = 0;

    // read the choosen line
    index = 0;
    while (line_count < random_index) {
        line_count++;
        index = lines.indexOf("\n", index) + 1;
    }

    let next_index = lines.indexOf("\n", index++);
    if (next_index < 0) next_index = lines.length;

    return lines.substring(index - 1, next_index);
}

async function introscreen_read_week_gretings() {
    if (!custom_style_from_week) return null;
    let greetings = weekenumerator_get_greetings(custom_style_from_week);
    if (!greetings) return null;
    let text = await introscreen_read_intro_text(greetings);
    greetings = undefined;
    return text;
}

