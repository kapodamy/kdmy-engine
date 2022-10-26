--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
function f_beforeready(from_retry)
    girlfriend = week_get_character(0)
    boyfriend = week_get_character(1)
end
function f_beat(curBeat, since)
    if curBeat % 16 == 15 and curBeat > 16 and curBeat < 48 then
        boyfriend:play_hey()
        girlfriend:play_hey()
    end
end
