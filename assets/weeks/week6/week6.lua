--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
-- Lua Library inline imports
local function __TS__StringSubstring(self, start, ____end)
    if ____end ~= ____end then
        ____end = 0
    end
    if ____end ~= nil and start > ____end then
        start, ____end = ____end, start
    end
    if start >= 0 then
        start = start + 1
    else
        start = 1
    end
    if ____end ~= nil and ____end < 0 then
        ____end = 0
    end
    return string.sub(self, start, ____end)
end

local function __TS__StringStartsWith(self, searchString, position)
    if position == nil or position < 0 then
        position = 0
    end
    return string.sub(self, position + 1, #searchString + position) == searchString
end

-- End of Lua Library inline imports
function dialog_show(dialog_lines)
    stage:set_group_visibility("hand", false)
    if dialog_is_evil then
        stage:set_group_visibility("dialogs", true)
        stage:trigger_action(nil, "dialog_anim")
    else
        dialog_mgr(true)
    end
    if dialog_is_angry then
        lyt_snd_angry:play()
    end
    do
        local i = 0
        while i < #dialog_lines do
            local text = dialog_lines[i + 1].text
            local text_length = #text
            local alt_snd = false
            local progress = i == 0 and -500 or -200
            local next_char = CHARS_DELAY
            local ____end = 0
            dialog_chrtr(dialog_lines[i + 1].isSenpai)
            lyt_dialog_text:set_text("")
            dialog_force_end = false
            dialog_button_pressed = false
            dialog_waiting_confirm = false
            while ____end <= text_length do
                local elapsed = coroutine.yield()
                if dialog_force_end then
                    break
                elseif progress > next_char then
                    ____end = math.floor(progress / CHARS_DELAY)
                    lyt_dialog_text:set_text(__TS__StringSubstring(text, 0, ____end))
                    next_char = progress + CHARS_DELAY
                    local snd = alt_snd and lyt_snd_char0 or lyt_snd_char1
                    snd:stop()
                    snd:play()
                    alt_snd = not alt_snd
                elseif dialog_button_pressed then
                    lyt_dialog_text:set_text(text)
                    dialog_button_pressed = false
                    break
                end
                progress = progress + elapsed
            end
            if dialog_force_end then
                lyt_snd_char0:play()
                lyt_snd_char1:play()
                break
            end
            stage:set_group_visibility("hand", true)
            dialog_waiting_confirm = true
            coroutine.yield()
            lyt_snd_click:play()
            stage:set_group_visibility("hand", false)
            i = i + 1
        end
    end
    dialog_corutine = nil
    if dialog_is_evil then
        stage:trigger_action(nil, "fade_out_group")
    else
        dialog_mgr(false)
    end
    lyt_bg_music:fade(false, 500)
    timer_callback_timeout(
        500,
        function()
            week_set_halt(false)
            ui_set_visibility(true)
        end
    )
end
function dialog_mgr(show)
    lyt_dialog_normal:set_visible(not dialog_is_angry)
    lyt_dialog_angry:set_visible(dialog_is_angry)
    if show then
        stage:set_group_visibility("dialogs", show)
        stage:trigger_action(nil, "dialog_anim")
    else
        lyt_icon_boyfriend:set_visible(false)
        lyt_icon_senpai:set_visible(false)
        stage:trigger_action(nil, "fade_out_group")
    end
end
function dialog_chrtr(isSenpai)
    if dialog_is_evil then
        return
    end
    lyt_icon_boyfriend:set_visible(not isSenpai)
    lyt_icon_senpai:set_visible(isSenpai and not dialog_is_angry)
    stage:trigger_action(nil, isSenpai and "dialog_anim_senpai" or "dialog_anim_boyfriend")
end
function dialog_parse(src_dialogue)
    local text = fs_readfile(src_dialogue)
    if text == nil then
        return nil
    end
    local length = #text
    local lines = {}
    local isSenpai = false
    do
        local i = 0
        while i < length do
            local ____end = (string.find(
                text,
                "\n",
                math.max(i + 1, 1),
                true
            ) or 0) - 1
            if ____end < 0 then
                ____end = length
            end
            local line = __TS__StringSubstring(text, i, ____end)
            local prefix
            if __TS__StringStartsWith(line, PREFIX_BF) then
                prefix = PREFIX_BF
                isSenpai = false
            elseif __TS__StringStartsWith(line, PREFIX_DAD) then
                prefix = PREFIX_DAD
                isSenpai = true
            else
                prefix = nil
            end
            if prefix ~= nil then
                lines[#lines + 1] = {
                    isSenpai = isSenpai,
                    text = __TS__StringSubstring(line, #prefix)
                }
            end
            i = ____end + 1
        end
    end
    local ____temp_0
    if #lines < 1 then
        ____temp_0 = nil
    else
        ____temp_0 = lines
    end
    return ____temp_0
end
PREFIX_DAD = ":dad:"
PREFIX_BF = ":bf:"
CHARS_DELAY = 40
GAMEPAD_XA = GAMEPAD_X | GAMEPAD_A
stage = nil
lyt_icon_boyfriend = nil
lyt_icon_senpai = nil
lyt_dialog_text = nil
lyt_snd_char0 = nil
lyt_snd_char1 = nil
lyt_snd_click = nil
lyt_bg_music = nil
lyt_dialog_normal = nil
lyt_dialog_angry = nil
lyt_snd_angry = nil
dialogs_senpai = nil
dialogs_roses = nil
dialogs_thorns = nil
dialog_corutine = nil
dialog_button_pressed = false
dialog_waiting_confirm = false
dialog_force_end = false
dialog_is_angry = false
dialog_is_evil = false
in_freeplay_mode = false
function f_weekinit(freeplay_index)
    if freeplay_index >= 0 then
        in_freeplay_mode = true
        return
    end
    dialogs_senpai = dialog_parse("/assets/weeks/week6/weeb/dialogs/senpaiDialogue.txt")
    dialogs_roses = dialog_parse("/assets/weeks/week6/weeb/dialogs/rosesDialogue.txt")
    dialogs_thorns = dialog_parse("/assets/weeks/week6/weeb_but_evil/dialogs/thornsDialogue.txt")
    week_enable_credits_on_completed()
    unlockdirective_create("FNF_COMPLETED", false, true, 9942069)
end
function f_beforeready(from_retry)
    stage = week_get_stage_layout()
    local ____, ____, track_index = week_get_current_track_info()
    repeat
        local ____switch5 = track_index
        local ____cond5 = ____switch5 == 0
        if ____cond5 then
            stage:trigger_trigger("girls_trigger")
            break
        end
        ____cond5 = ____cond5 or ____switch5 == 1
        if ____cond5 then
            stage:trigger_trigger("dissuaded_girls_trigger")
            break
        end
        ____cond5 = ____cond5 or ____switch5 == 2
        if ____cond5 then
            stage:stop_all_triggers()
            break
        end
        do
            return
        end
    until true
    if from_retry or in_freeplay_mode then
        return
    end
    lyt_icon_boyfriend = stage:get_sprite("dialog_icon_boyfriend")
    lyt_icon_senpai = stage:get_sprite("dialog_icon_senpai")
    lyt_dialog_text = stage:get_textsprite("dialog_text")
    lyt_snd_char0 = stage:get_soundplayer("sndChar0")
    lyt_snd_char1 = stage:get_soundplayer("sndChar1")
    lyt_snd_click = stage:get_soundplayer("sndClick")
    lyt_bg_music = stage:get_soundplayer("bgMusic")
    lyt_dialog_normal = stage:get_sprite("dialog_normal")
    lyt_dialog_angry = stage:get_sprite("dialog_angry")
    lyt_snd_angry = stage:get_soundplayer("angrySenpai")
    dialog_is_angry = track_index == 1
    dialog_is_evil = track_index == 2
    local dialog_lines
    repeat
        local ____switch7 = track_index
        local ____cond7 = ____switch7 == 0
        if ____cond7 then
            dialog_lines = dialogs_senpai
            break
        end
        ____cond7 = ____cond7 or ____switch7 == 1
        if ____cond7 then
            dialog_lines = dialogs_roses
            break
        end
        ____cond7 = ____cond7 or ____switch7 == 2
        if ____cond7 then
            dialog_lines = dialogs_thorns
            break
        end
        do
            return
        end
    until true
    if not dialog_lines then
        return
    end
    week_set_halt(true)
    ui_set_visibility(false)
    if track_index == 0 or track_index == 2 then
        lyt_bg_music:play()
    end
    if track_index == 2 then
        stage:trigger_action(nil, "fade_in_screen")
    end
    timer_callback_timeout(
        1830,
        function()
            dialog_corutine = coroutine.create(dialog_show)
            coroutine.resume(dialog_corutine, dialog_lines)
        end
    )
end
function f_frame(elapsed)
    if dialog_corutine ~= nil and not dialog_waiting_confirm then
        coroutine.resume(dialog_corutine, elapsed)
    end
end
function f_buttons(player_id, buttons)
    if dialog_corutine == nil or player_id == 0 then
        return
    end
    if buttons & (1 | 4) ~= 0 then
        dialog_button_pressed = true
    elseif buttons & 256 ~= 0 then
        dialog_force_end = true
    else
        return
    end
    if dialog_waiting_confirm then
        coroutine.resume(dialog_corutine)
    end
end
function f_roundend(loose)
    if loose or not dialog_is_angry or in_freeplay_mode then
        return
    end
    week_set_halt(true)
    ui_set_visibility(false)
    stage:trigger_camera("senpai_dies")
    stage:trigger_action(nil, "fade_in_red")
    timer_callback_timeout(
        350,
        function()
            stage:trigger_action(nil, "senpaiCrazy")
            stage:get_soundplayer("senpaiDies"):play()
        end
    )
    timer_callback_timeout(
        350 + 4700,
        function()
            stage:trigger_action(nil, "fade_in_white")
        end
    )
    timer_callback_timeout(
        350 + 4500 + 3100,
        function()
            week_set_halt(false)
        end
    )
end
