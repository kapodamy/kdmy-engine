--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
function place_lemon_demon(stage)
    local placeholder = stage:get_placeholder("lemon_demon")
    local opponent = week_get_character(0)
    opponent:set_draw_location(placeholder.x, placeholder.y)
end
isBeforeEvilMall = false
stage = nil
stageCamera = nil
in_freeplay_mode = false
function f_weekinit(freeplay_index)
    in_freeplay_mode = freeplay_index >= 0
    if freeplay_index == 2 then
        place_lemon_demon(week_get_stage_layout())
    end
end
function f_beforeready(from_retry)
    stage = week_get_stage_layout()
    stageCamera = stage:get_camera_helper()
    local ____, ____, track_index = week_get_current_track_info()
    isBeforeEvilMall = track_index == 1
    if track_index == 0 then
        stage:trigger_camera("mall_start")
    end
    if track_index < 2 then
        stage:trigger_trigger("bops_trigger")
        return
    end
    stage:stop_all_triggers()
    week_change_character_camera_name(true, "camera_evil_opponent")
    week_change_character_camera_name(false, "camera_evil_player")
    if from_retry or in_freeplay_mode then
        if not in_freeplay_mode then
            stage:trigger_camera("mall_evil")
            stage:trigger_camera("mall_evil_zoom_out")
        end
        stage:trigger_action(nil, "evil")
        return
    end
    week_set_halt(true)
    stage:trigger_action(nil, "fade_lights")
    week_ui_set_visibility(false)
    place_lemon_demon(stage)
    stage:trigger_action(nil, "lights_on")
    timer_callback_timeout(
        800,
        function()
            stage:trigger_action(nil, "evil")
            stage:trigger_camera("mall_evil")
            timer_callback_timeout(
                2500,
                function()
                    stage:trigger_camera("mall_evil_zoom_out")
                    week_ui_set_visibility(true)
                    week_set_halt(false)
                end
            )
        end
    )
end
function f_roundend(loose)
    if not isBeforeEvilMall or loose or in_freeplay_mode then
        return
    end
    stage:trigger_action(nil, "lights_off")
    stage:trigger_action(nil, "fade_lights")
    week_set_halt(true)
    timer_callback_timeout(2000, week_set_halt, false)
end
