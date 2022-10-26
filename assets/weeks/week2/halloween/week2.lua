--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
lightningStrikeBeat = 24
function f_beforeready(from_restart)
    lightningStrikeBeat = 24
    if from_restart then
        return
    end
    local track_name, track_difficult, track_index = week_get_current_track_info()
    print((((("track: name=" .. track_name) .. " difficult=") .. track_difficult) .. " number=") .. tostring(track_index))
end
function f_beat(curBeat, since)
    if math.random(0, 1) < 0.1 then
        return
    end
    if curBeat <= lightningStrikeBeat then
        return
    end
    local random_boolean = math.random() < 0.5
    lightningStrikeBeat = curBeat + math.random(8, 24)
    local stage = week_get_stage_layout()
    local girlfriend = week_get_girlfriend()
    local boyfriend = week_get_character(1)
    if stage then
        stage:trigger_any("lightstrike")
        stage:trigger_action(nil, random_boolean and "thunder_1" or "thunder_2")
    end
    girlfriend:play_extra("fear", false)
    boyfriend:play_extra("shaking", false)
end
