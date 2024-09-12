--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
-- Lua Library inline imports
local __TS__StringSplit
do
    local sub = string.sub
    local find = string.find
    function __TS__StringSplit(source, separator, limit)
        if limit == nil then
            limit = 4294967295
        end
        if limit == 0 then
            return {}
        end
        local result = {}
        local resultIndex = 1
        if separator == nil or separator == "" then
            for i = 1, #source do
                result[resultIndex] = sub(source, i, i)
                resultIndex = resultIndex + 1
            end
        else
            local currentPos = 1
            while resultIndex <= limit do
                local startPos, endPos = find(source, separator, currentPos, true)
                if not startPos then
                    break
                end
                result[resultIndex] = sub(source, currentPos, startPos - 1)
                resultIndex = resultIndex + 1
                currentPos = endPos + 1
            end
            if resultIndex <= limit then
                result[resultIndex] = sub(source, currentPos)
            end
        end
        return result
    end
end

local __TS__StringReplaceAll
do
    local sub = string.sub
    local find = string.find
    function __TS__StringReplaceAll(source, searchValue, replaceValue)
        if type(replaceValue) == "string" then
            local concat = table.concat(
                __TS__StringSplit(source, searchValue),
                replaceValue
            )
            if #searchValue == 0 then
                return (replaceValue .. concat) .. replaceValue
            end
            return concat
        end
        local parts = {}
        local partsIndex = 1
        if #searchValue == 0 then
            parts[1] = replaceValue(nil, "", 0, source)
            partsIndex = 2
            for i = 1, #source do
                parts[partsIndex] = sub(source, i, i)
                parts[partsIndex + 1] = replaceValue(nil, "", i, source)
                partsIndex = partsIndex + 2
            end
        else
            local currentPos = 1
            while true do
                local startPos, endPos = find(source, searchValue, currentPos, true)
                if not startPos then
                    break
                end
                parts[partsIndex] = sub(source, currentPos, startPos - 1)
                parts[partsIndex + 1] = replaceValue(nil, searchValue, startPos - 1, source)
                partsIndex = partsIndex + 2
                currentPos = endPos + 1
            end
            parts[partsIndex] = sub(source, currentPos)
        end
        return table.concat(parts)
    end
end

-- End of Lua Library inline imports
function thanks_callback()
    local bg_music = layout:get_soundplayer("bg-music")
    if bg_music then
        bg_music:fade(false, thanks_duration)
    end
    timer_callback_timeout(thanks_duration, shoot_callback)
end
function shoot_callback()
    layout:trigger_any("shoot")
    timer_callback_timeout(
        shoot_duration,
        function()
            layout:trigger_any("outro")
            check_outro = true
        end
    )
end
CREDITS_BUTTONS = 256 | 1 | 2 | 4 | 8
introText = fs_readfile("/assets/common/introText.txt")
layout = modding_get_layout()
introTextHolder = layout:get_textsprite("intro-texts")
introTextGroupModifier = layout:get_group_modifier("intro-texts-group")
base_scrolled_to = layout:get_attached_value("scripted_base_scrolled_to")
base_scrolled_duration = layout:get_attached_value("scripted_base_scrolled_duration")
thanks_scroll_duration = layout:get_attached_value("scripted_thanks_scroll_duration")
thanks_duration = layout:get_attached_value("scripted_thanks_duration")
shoot_duration = layout:get_attached_value("scripted_shoot_duration")
do_scroll = true
check_outro = false
wait_duration = base_scrolled_duration
scroll_duration = 0
scroll_distance = 0
scroll_elapsed = 0
if introText ~= nil and introTextHolder ~= nil then
    local introTextList = __TS__StringSplit(introText, "\n")
    introText = ""
    do
        local i = 0
        while i < #introTextList do
            introText = introText .. __TS__StringReplaceAll(introTextList[i + 1], "--", "\r\n") .. "\r\n\r\n"
            i = i + 1
        end
    end
    introTextHolder:set_text(introText)
    local ____, holder_height = introTextHolder:get_draw_size()
    scroll_distance = holder_height
    scroll_duration = base_scrolled_duration * holder_height / base_scrolled_to
else
    scroll_distance = 0
    scroll_duration = 0
end
function f_frame(elapsed)
    if check_outro then
        if layout:animation_is_completed("transition_effect") > 0 then
            modding_exit()
        end
        return
    end
    if not do_scroll then
        return
    end
    if wait_duration > 0 then
        wait_duration = wait_duration - elapsed
        return
    end
    local current_distance = math2d_lerp(0, scroll_distance, scroll_elapsed / scroll_duration)
    scroll_elapsed = scroll_elapsed + elapsed
    if scroll_elapsed >= scroll_duration or introTextGroupModifier == nil then
        do_scroll = false
        current_distance = scroll_distance
        layout:trigger_any("show-thanks")
        timer_callback_timeout(thanks_scroll_duration, thanks_callback)
    end
    if introTextGroupModifier then
        introTextGroupModifier.translateY = -current_distance
    end
end
function f_buttons(player_id, buttons)
    if check_outro then
        return
    end
    if buttons & CREDITS_BUTTONS then
        layout:trigger_any("outro")
        check_outro = true
    end
end
