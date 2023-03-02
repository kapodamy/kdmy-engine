--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
philly = nil
trainCooldown = 0
trainMoving = false
girlfriendAnimationTimeout = -1
song = nil
function f_weekinit(freeplay_index)
    if freeplay_index >= 0 then
        return
    end
    local ____, difficult = week_get_current_song_info()
    if difficult == "HARD" then
        week_unlockdirective_create("WEEK3_HARD", true, false, 1)
    end
end
function f_beforeready()
    trainCooldown = 0
    philly = week_get_stage_layout()
    girlfriend = week_get_girlfriend()
    song = week_get_songplayer()
    if philly then
        philly:trigger_trigger("lights_trigger")
    end
end
function f_frame(elapsed)
    if girlfriendAnimationTimeout > 0 and song and song:get_timestamp() >= girlfriendAnimationTimeout then
        girlfriendAnimationTimeout = -1
        if girlfriend then
            girlfriend:play_extra("hair", true)
        end
    end
end
function f_beat(curBeat, since)
    if not philly then
        return
    end
    if philly:animation_is_completed("train-engine") ~= 1 then
        return
    end
    if trainMoving and girlfriendAnimationTimeout < 0 then
        trainMoving = false
        if girlfriend then
            girlfriend:play_idle()
        end
    end
    trainCooldown = trainCooldown + 1
    if curBeat % 8 ~= 4 then
        return
    end
    if trainCooldown <= 8 then
        return
    end
    if math.random() >= 0.3 then
        return
    end
    trainCooldown = math.random(-4, 0)
    trainMoving = true
    if philly then
        philly:trigger_action(nil, "train_passes")
    end
    girlfriendAnimationTimeout = song:get_timestamp() + 4800
end
