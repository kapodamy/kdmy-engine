
let isBeforeEvilMall: boolean = false;
let stage: Layout = null;
let stageCamera: Camera = null;
let in_freeplay_mode: boolean = false;

function f_weekinit(freeplay_index: number) {
    in_freeplay_mode = freeplay_index >= 0;
    if (freeplay_index == 2) place_lemon_demon(week_get_stage_layout());

    let [, difficult,] = week_get_current_song_info();
    if (difficult == "HARD" && freeplay_index < 0) week_unlockdirective_create("WEEK5_HARD", false, true, 0.0);
}

function f_beforeready(from_retry: boolean): void {
    stage = week_get_stage_layout();
    stageCamera = stage.get_camera_helper();
    let [, , song_index] = week_get_current_song_info();

    isBeforeEvilMall = song_index == 1;

    if (song_index == 0) stage.trigger_camera("mall_start");

    if (song_index < 2) {
        //
        // Notes:
        //      * The engines attempts to restore the original layout state (executes all not named actions)
        //        is not necessary "rollback" the mall's evil actions.
        //      * Also restores the character camera names
        //
        stage.trigger_trigger("bops_trigger");
        return;
    }

    // stop bop trigger
    stage.stop_all_triggers();

    // move lemon demon (spawns in parents location)
    place_lemon_demon(stage);

    // change cameras focus
    week_change_character_camera_name(true, "camera_evil_opponent");
    week_change_character_camera_name(false, "camera_evil_player");

    if (from_retry || in_freeplay_mode) {
        // do not bother the player, skip intro animation
        if (!in_freeplay_mode) {
            stage.trigger_camera("mall_evil");
            stage.trigger_camera("mall_evil_zoom_out");
        }
        stage.trigger_action(null, "evil");
        return;
    }


    //
    // switch to "evil" mall
    //

    // step 1: halt "Ready?"
    week_set_halt(true);

    // trigger "fade_lights" action again, beacuse the engine execute all not named
    // actions before the round starts
    stage.trigger_action(null, "fade_lights");

    week_ui_set_visibility(false);

    // step 4: play "lights_on" sound 
    stage.trigger_action(null, "lights_on");

    // step 5: wait a bit
    timer_callback_timeout(800, function (): void {
        // step 5: replace sprites, turn the "lights" on and focus the evil tree
        stage.trigger_action(null, "evil");
        stage.trigger_camera("mall_evil");

        // step 6: wait again
        timer_callback_timeout(2500, function (): void {
            // step 7: zoom out and resume and resume the round startup
            stage.trigger_camera("mall_evil_zoom_out");
            week_ui_set_visibility(true);
            week_set_halt(false);
        });
    });
}

function f_roundend(loose: boolean) {
    if (!isBeforeEvilMall || loose || in_freeplay_mode) return;

    // LOOK HOW EFFORTLESSLY I TURN OFF THE LIGHTS
    stage.trigger_action(null, "lights_off");
    stage.trigger_action(null, "fade_lights");
    week_set_halt(true);
    timer_callback_timeout(2000, week_set_halt, false);
}


function place_lemon_demon(stage: Layout) {
    let placeholder: LayoutPlaceholderInfo = stage.get_placeholder("lemon_demon");
    let opponent: Character = week_get_character(0);
    opponent.set_draw_location(placeholder.x, placeholder.y);
}