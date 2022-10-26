--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
function resetFastCar()
    fastCarCanDrive = false
end
MILF_ZOOMS_STAGE_INCREMENT = 0.018
MILF_ZOOMS_UI_INCREMENT = 0.0003
fastCarCanDrive = true
limo = nil
cameraUI = nil
cameraStage = nil
isMILF = false
incrementalUIZoomMILF = 0
incrementalStageZoomMILF = 0
function f_beforeready()
    fastCarCanDrive = true
    limo = week_get_stage_layout()
    cameraUI = ui_get_camera()
    cameraStage = limo:get_camera_helper()
    local track_name, track_difficult, track_index = week_get_current_track_info()
    console:log("track: " .. track_name)
    isMILF = track_name == "Milf"
    incrementalUIZoomMILF = 1.05
    incrementalStageZoomMILF = 1.09
end
function f_beat(curBeat, since)
    if isMILF and curBeat >= 168 and curBeat < 200 and incrementalUIZoomMILF < 1.35 then
        local oldZoom = incrementalStageZoomMILF
        incrementalStageZoomMILF = incrementalStageZoomMILF + MILF_ZOOMS_STAGE_INCREMENT
        cameraStage:slide_z(oldZoom, incrementalStageZoomMILF)
        cameraUI:slide_z(1, incrementalUIZoomMILF)
        incrementalUIZoomMILF = incrementalUIZoomMILF + MILF_ZOOMS_UI_INCREMENT
    end
    if math2d_random_boolean(10) and fastCarCanDrive then
        local random_sound = math2d_random_boolean(50) and "carPass0" or "carPass1"
        fastCarCanDrive = false
        limo:trigger_action(nil, random_sound)
        limo:trigger_action(nil, "carPass")
        timer_callback_timeout(2000, resetFastCar)
    end
end
