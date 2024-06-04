"use strict";

const COUNTDOWN_READY_CONFIRM = "ready-confirm";
const COUNTDOWN_THREE = "three";// in the funkin only the sounds "2", "1", "go!" have sprites
const COUNTDOWN_READY = "ready";
const COUNTDOWN_SET = "set";
const COUNTDOWN_GO = "go";
const COUNTDOWN_DEFAULT_ANIMATION = "countdown";

async function countdown_init(modelholder, height) {
    fs_folder_stack_push();
    fs_set_working_folder("/assets/common/sound/", false);

    let countdown = {
        valid_model: !modelholder_is_invalid(modelholder),
        valid_state: false,

        statesprite: statesprite_init_from_texture(null),
        beat_duration: math2d_beats_per_minute_to_beat_per_milliseconds(100.0),

        height: height,
        default_animation: null,
        default_animate: false,
        animation_speed: 1.0,

        static_ready: false,
        ready_only: false,
        progress: 4,
        timer: 0.0,

        sound_three: await soundplayer_init("intro3.ogg"),
        sound_two: await soundplayer_init("intro2.ogg"),
        sound_one: await soundplayer_init("intro1.ogg"),
        sound_go: await soundplayer_init("introGo.ogg"),

        drawable: null
    };

    countdown.drawable = drawable_init(200.0, countdown, countdown_draw, countdown_animate);

    fs_folder_stack_pop();

    if (!countdown.valid_model) return countdown;

    statesprite_set_visible(countdown.statesprite, false);
    statesprite_state_add(
        countdown.statesprite, modelholder, COUNTDOWN_READY_CONFIRM, COUNTDOWN_READY_CONFIRM
    );
    statesprite_state_add(
        countdown.statesprite, modelholder, COUNTDOWN_THREE, COUNTDOWN_THREE
    );
    statesprite_state_add(
        countdown.statesprite, modelholder, COUNTDOWN_READY, COUNTDOWN_READY
    );
    statesprite_state_add(
        countdown.statesprite, modelholder, COUNTDOWN_SET, COUNTDOWN_SET
    );
    statesprite_state_add(
        countdown.statesprite, modelholder, COUNTDOWN_GO, COUNTDOWN_GO
    );

    return countdown;
}

function countdown_destroy(countdown) {
    luascript_drop_shared(countdown);

    statesprite_destroy(countdown.statesprite);
    if (countdown.default_animation) tweenkeyframe_destroy(countdown.default_animation);
    if (countdown.sound_three) soundplayer_destroy(countdown.sound_three);
    if (countdown.sound_two) soundplayer_destroy(countdown.sound_two);
    if (countdown.sound_one) soundplayer_destroy(countdown.sound_one);
    if (countdown.sound_go) soundplayer_destroy(countdown.sound_go);
    drawable_destroy(countdown.drawable);
    countdown = undefined;
}


function countdown_set_layout_viewport(countdown, width, height) {
    if (!countdown.valid_model) return;
    const DRAW_SIZE = [0.0, 0.0];

    let state_list = statesprite_state_list(countdown.statesprite);

    for (let state of linkedlist_iterate4(state_list)) {
        imgutils_get_statesprite_original_size(state, DRAW_SIZE);
        imgutils_calc_size(DRAW_SIZE[0], DRAW_SIZE[1], -1.0, countdown.height, DRAW_SIZE);
        state.draw_width = DRAW_SIZE[0];
        state.draw_height = DRAW_SIZE[1];
        state.offset_x = state.draw_width / -2.0;
        state.offset_y = state.draw_height / -2.0;
    }

    statesprite_set_draw_location(countdown.statesprite, width / 2, height / 2);
}

function countdown_set_bpm(countdown, bpm) {
    countdown.beat_duration = math2d_beats_per_minute_to_beat_per_milliseconds(bpm);
    countdown.animation_speed = 1000.0 / countdown.beat_duration;
}

function countdown_set_default_animation(countdown, animlist) {
    if (!animlist) return;
    let animlist_item = animlist_get_animation(animlist, COUNTDOWN_DEFAULT_ANIMATION);
    if (!animlist_item) return;

    countdown.default_animation = tweenkeyframe_init2(animlist_item);
}

function countdown_set_default_animation2(countdown, tweenkeyframe) {
    if (countdown.default_animation) tweenkeyframe_destroy(countdown.default_animation);
    countdown.default_animation = tweenkeyframe ? tweenkeyframe_clone(tweenkeyframe) : null;
}


function countdown_get_drawable(countdown) {
    return countdown.drawable;
}


function countdown_ready(countdown) {
    if (!countdown.valid_model) return false;

    countdown.progress = 0;
    countdown.ready_only = true;
    countdown.static_ready = false;
    countdown.valid_state = statesprite_state_toggle(countdown.statesprite, COUNTDOWN_READY_CONFIRM);

    // restore statesprite alpha
    statesprite_set_alpha(countdown.statesprite, 1.0);

    if (!countdown.valid_state) {
        countdown.static_ready = true;
        countdown.valid_state = statesprite_state_toggle(countdown.statesprite, COUNTDOWN_READY);
    }

    if (countdown.valid_state) {
        statesprite_animation_restart(countdown.statesprite);
        statesprite_animate(countdown.statesprite, 0.0);
    }

    return countdown.valid_state;
}

function countdown_start(countdown) {
    if (countdown.sound_three) soundplayer_replay(countdown.sound_three);
    if (!countdown.valid_model) return false;
    countdown_internal_toggle(countdown, COUNTDOWN_THREE);
    countdown.ready_only = false;
    countdown.static_ready = false;
    countdown.progress = 0;
    countdown.timer = 0.0;

    return countdown.valid_state;
}

function countdown_has_ended(countdown) {
    return (countdown.progress > 3) && (!countdown.ready_only);
}


function countdown_animate(countdown, elapsed) {
    if (countdown.static_ready) return 1;
    if (countdown.ready_only) return statesprite_animate(countdown.statesprite, elapsed);
    if (countdown.progress > 4) return 1;

    let completed;

    if (countdown.default_animate) {
        let percent = countdown.timer / countdown.beat_duration;
        if (percent > 1.0) percent = 1.0;

        tweenkeyframe_animate_percent(countdown.default_animation, percent);
        tweenkeyframe_vertex_set_properties(
            countdown.default_animation, countdown.statesprite, statesprite_set_property
        );
        completed = percent >= 1.0 ? 1 : 0;
    } else {
        completed = statesprite_animate(countdown.statesprite, elapsed * countdown.animation_speed);
    }

    if (countdown.timer >= countdown.beat_duration) {
        countdown.timer -= countdown.beat_duration;// this keeps the timer in sync
        countdown.progress++;

        switch (countdown.progress) {
            case 1:
                if (countdown.sound_three) soundplayer_stop(countdown.sound_three);
                if (countdown.sound_two) soundplayer_replay(countdown.sound_two);
                countdown_internal_toggle(countdown, COUNTDOWN_READY);
                break;
            case 2:
                if (countdown.sound_two) soundplayer_stop(countdown.sound_two);
                if (countdown.sound_one) soundplayer_replay(countdown.sound_one);
                countdown_internal_toggle(countdown, COUNTDOWN_SET);
                break;
            case 3:
                if (countdown.sound_one) soundplayer_stop(countdown.sound_one);
                if (countdown.sound_go) soundplayer_replay(countdown.sound_go);
                countdown_internal_toggle(countdown, COUNTDOWN_GO);
                break;
        }

        statesprite_animation_restart(countdown.statesprite);
    }

    countdown.timer += elapsed;

    return completed;
}

function countdown_draw(countdown, pvrctx) {
    if (!countdown.valid_model || !countdown.valid_state) return;
    if (!countdown.ready_only && countdown.progress > 3) return;

    statesprite_draw(countdown.statesprite, pvrctx);
}



function countdown_internal_toggle(countdown, state_name) {
    countdown.valid_state = statesprite_state_toggle(countdown.statesprite, state_name);
    if (!countdown.valid_state) return;

    if (statesprite_state_get(countdown.statesprite).animation) {
        countdown.default_animate = false;
    } else if (countdown.default_animation) {
        countdown.default_animate = true;
    } else {
        countdown.default_animate = false;
    }
}

