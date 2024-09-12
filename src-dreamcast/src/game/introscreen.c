#include "game/introscreen.h"

#include "fs.h"
#include "game/common/weekenumerator.h"
#include "game/main.h"
#include "game/modding.h"
#include "gamepad.h"
#include "layout.h"
#include "malloc_utils.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "soundplayer.h"
#include "stringbuilder.h"
#include "stringutils.h"
#include "textsprite.h"


static const float INTROSCREEN_DELAY = 600.0f;
static const float INTROSCREEN_TEXT_DURATION = 0.0f;
static const char* INTROSCREEN_TEXT_SPARSE = "--";
static const char* INTROSCREEN_FUNKY = "Friday--Night--Funkin";
static const char* INTROSCREEN_LAYOUT = "/assets/common/image/intro-screen/layout.xml";
static const char* INTROSCREEN_LAYOUT_DREAMCAST = "/assets/common/image/intro-screen/layout~dreamcast.xml";
static const GamepadButtons INTROSCREEN_SKIP_BUTTONS = GAMEPAD_A | GAMEPAD_START;
static const char* INTROSCREEN_MODDING_SCRIPT = "/assets/common/data/scripts/introscreen.lua";


static void introscreen_draw_sparse_text(const char* text, float delay, float duration, Modding modding, Gamepad maple_pad);
static char* introscreen_read_intro_text(const char* path);
static char* introscreen_read_week_gretings();


void introscreen_main() {
    Layout layout = layout_init(pvr_context_is_widescreen() ? INTROSCREEN_LAYOUT : INTROSCREEN_LAYOUT_DREAMCAST);
    if (!layout) return;

    Modding modding = modding_init(layout, INTROSCREEN_MODDING_SCRIPT);
    Gamepad maple_pad = gamepad_init(-1);
    gamepad_clear_buttons(maple_pad);

    modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN, ModdingValueType_string);

    // read delay/durations from layout
    float delay = layout_get_attached_value_as_float(layout, "delay", INTROSCREEN_DELAY);
    float64 custom_duration = layout_get_attached_value_as_float(
        layout, "custom_duration", INTROSCREEN_TEXT_DURATION
    );

    if (custom_duration > 0.0) {
        modding_helper_notify_event(modding, "custom-intro");

        // custom intro detected, wait the desired time
        float64 progress = 0.0;
        while (progress < custom_duration) {
            float elapsed = pvrctx_wait_ready();
            pvr_context_reset(&pvr_context);

            ModdingHelperResult res = modding_helper_handle_custom_menu(modding, maple_pad, elapsed);
            if (modding->has_exit || res != ModdingHelperResult_CONTINUE) break;

            layout_animate(layout, elapsed);
            layout_draw(layout, &pvr_context);
            if (modding->has_halt) continue;

            progress += elapsed;
            if (gamepad_has_pressed(maple_pad, INTROSCREEN_SKIP_BUTTONS) != 0x00) break;
        }

        modding_helper_notify_exit2(modding);

        modding_destroy(&modding);
        layout_destroy(&layout);
        gamepad_destroy(&maple_pad);
        return;
    }

    char* self_text = fs_readtext("/assets/engineText.txt");
    char* intro_text = introscreen_read_intro_text("/assets/common/data/introText.txt");
    char* week_greetings = NULL;
    const char* funky_intro = layout_get_attached_value_string(layout, "funky", INTROSCREEN_FUNKY);

    // 25% chance of displaying choosen week greetings
    if (math2d_random_float() <= 0.25f) week_greetings = introscreen_read_week_gretings();

    // if there an camera animation called "camera_animation", use it
    layout_trigger_camera(layout, "camera_animation");

    float engine_duration = delay * 4.0f;
    float greetings_duration = delay * 4.0f;
    float funkin_duration = delay * 5.0f;

    // start this game
    if (background_menu_music) soundplayer_play(background_menu_music);

    layout_trigger_any(layout, "intro-engine");
    modding_helper_notify_event(modding, "intro-engine");
    introscreen_draw_sparse_text(self_text, delay, engine_duration, modding, maple_pad);

    if (week_greetings) {
        layout_trigger_any(layout, "intro-week-grettings");
        modding_helper_notify_event(modding, "intro-week-grettings");
        introscreen_draw_sparse_text(week_greetings, delay, greetings_duration, modding, maple_pad);
    } else {
        layout_trigger_any(layout, "intro-greetings");
        modding_helper_notify_event(modding, "intro-greetings");
        introscreen_draw_sparse_text(intro_text, delay, greetings_duration, modding, maple_pad);
    }

    layout_trigger_any(layout, "intro-funkin");
    modding_helper_notify_event(modding, "intro-funkin");
    introscreen_draw_sparse_text(funky_intro, delay, funkin_duration, modding, maple_pad);

    layout_trigger_any(layout, "transition-out");
    modding_helper_notify_event(modding, "transition-out");
    while (true) {
        float elapsed = pvrctx_wait_ready();
        modding_helper_notify_frame(modding, elapsed, -1.0);

        pvr_context_reset(&pvr_context);
        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        if (layout_animation_is_completed(layout, "transition_effect") > 0) {
            // flush framebuffer again with last fade frame
            pvrctx_wait_ready();
            break;
        }
    }

    modding_helper_notify_exit2(modding);

    // dispose resources used
    free_chk(self_text);
    free_chk(intro_text);
    free_chk(week_greetings);
    modding_destroy(&modding);
    layout_destroy(&layout);
    gamepad_destroy(&maple_pad);
}

/*static void introscreen_pause(float64 duration, Layout layout) {
    float64 total_elapsed = 0.0;
    while (total_elapsed < duration) {
        float elapsed = pvrctx_wait_ready();
        total_elapsed += elapsed;

        pvr_context_reset(&pvr_context);
        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);
    }
}*/

static void introscreen_draw_sparse_text(const char* text, float delay, float duration, Modding modding, Gamepad maple_pad) {
    Layout layout = modding->layout;
    TextSprite textsprite = layout_get_textsprite(layout, "greetings");
    if (!textsprite || !text || modding->has_exit) return;

    size_t text_length = strlen(text);
    StringBuilder text_buffer = stringbuilder_init(text_length);
    int32_t lines = string_occurrences_of_string(text, INTROSCREEN_TEXT_SPARSE) + 1;
    float64 paragraph_duration = duration / lines;
    float64 progress = paragraph_duration - delay;
    int32_t sparse_length = (int32_t)strlen(INTROSCREEN_TEXT_SPARSE);
    int32_t last_index = 0;

    while (true) {
        if (progress >= paragraph_duration) {
            if (last_index >= text_length) break;
            if (last_index > 0) stringbuilder_add_char(text_buffer, '\n');

            int32_t index = string_index_of(text, last_index, INTROSCREEN_TEXT_SPARSE);
            if (index < 0) index = (int32_t)text_length;

            stringbuilder_add_substring(text_buffer, text, last_index, index);
            last_index = index + sparse_length;

            textsprite_set_text_intern(textsprite, true, stringbuilder_intern(text_buffer));
            progress -= paragraph_duration;
        }

        float elapsed = pvrctx_wait_ready();
        pvr_context_reset(&pvr_context);

        ModdingHelperResult res = modding_helper_handle_custom_menu(modding, maple_pad, elapsed);
        if (modding->has_exit || res != ModdingHelperResult_CONTINUE) break;

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);
        if (modding->has_halt) continue;

        progress += elapsed;
        if (gamepad_has_pressed(maple_pad, INTROSCREEN_SKIP_BUTTONS) != 0x00) break;
    }

    stringbuilder_destroy(&text_buffer);
}

static char* introscreen_read_intro_text(const char* path) {
    char* lines = fs_readtext(path);
    int32_t line_count = 0;

    if (lines == NULL) return NULL;

    // count lines in the file
    int32_t index = 0;
    while (true) {
        line_count++;
        index = string_index_of_char(lines, index, '\n');
        if (index < 0) break;
        index++;
    }

    // choose a random line
    int32_t random_index = math2d_random_int(0, line_count);
    line_count = 0;

    // read the choosen line
    index = 0;
    while (line_count < random_index) {
        line_count++;
        index = string_index_of_char(lines, index, '\n') + 1;
    }

    int32_t next_index = string_index_of_char(lines, index++, '\n');

    if (next_index > 1 && lines[next_index - 1] == '\r')
        next_index--;
    else if (next_index < 0)
        next_index = (int32_t)strlen(lines);

    return string_substring(lines, index - 1, next_index);
}

static char* introscreen_read_week_gretings() {
    if (!custom_style_from_week) return NULL;
    char* greetings = weekenumerator_get_greetings(custom_style_from_week);
    if (!greetings) return NULL;
    char* text = introscreen_read_intro_text(greetings);
    free_chk(greetings);
    return text;
}
