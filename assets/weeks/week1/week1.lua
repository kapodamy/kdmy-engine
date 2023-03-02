--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
function f_beforeready(from_retry)
    local name, difficult, index = week_get_current_song_info()
    curSong_bopeebo = index == 0
    boyfriend = week_get_character(1)
end
function f_beat(curBeat, since)
    if curBeat % 8 == 7 and curSong_bopeebo then
        boyfriend:play_hey()
    end
end
