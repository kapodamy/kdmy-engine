
let isBeforeEvilMall: boolean = false;
let stage: Layout = null;
let stageCamera: Camera = null;

function f_beforeready(from_retry: boolean): void {
    stage = week_get_stage_layout();
    stageCamera = stage.get_camera_helper();
    let [, , track_index] = week_get_current_track_info();

    isBeforeEvilMall = track_index == 1;

    if (track_index == 0) stage.trigger_camera("mall_start");

    if (track_index < 2) {
        //
        // Notes:
        //      * The engines attempts to restore the original layout state (executes all unamed actions)
        //        is not necessary "rollback" the mall's evil actions.
        //      * Also restores the character camera names
        //
        stage.trigger_trigger("bops_trigger");
        return;
    }

    // stop bop trigger
    stage.stop_all_triggers();

	// change cameras focus
    week_change_charecter_camera_name(true, "camera_evil_opponent");
    week_change_charecter_camera_name(false, "camera_evil_player");

    if (from_retry) {
        // do not bother the player, skip intro animation
        stage.trigger_camera("mall_evil");
        stage.trigger_camera("mall_evil_zoom_out");
        stage.trigger_action(null, "evil");
        return;
    }


    //
    // switch to "evil" mall
    //

    // step 1: halt "Ready?"
    week_set_halt(true);
    
    // trigger "fade_lights" action again, beacuse the engine execute all unamed
    // actions before the round starts
    stage.trigger_action(null, "fade_lights");

    ui_set_visibility(false);

    // step 2: move lemon demon (spawns in parents location)
    let placeholder: LayoutPlaceholderInfo = stage.get_placeholder("lemon_demon");
    let opponent: Character = week_get_character(0);
    opponent.set_draw_location(placeholder.x, placeholder.y);

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
            ui_set_visibility(true);
            week_set_halt(false);
        });
    });
}

function f_roundend(loose: boolean) {
    if (!isBeforeEvilMall || loose) return;

    // LOOK HOW EFFORTLESSLY I TURN OFF THE LIGHTS
    stage.trigger_action(null, "lights_off");
    stage.trigger_action(null, "fade_lights");
    week_set_halt(true);
    timer_callback_timeout(2000, week_set_halt, false);
}

