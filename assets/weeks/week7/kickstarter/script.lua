--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
function funkin_handle_nav(nav)
    if video_playing then
        if nav == "ok" or nav == "back" then
            video_playing = false
            video:pause()
            layout:trigger_action("trailer", "fade_out")
            if bg_music then
                bg_music:play()
                bg_music:set_volume(0.7)
                bg_music:fade(true, 500)
            end
        end
        return
    end
    if nav == "up" then
        if not www_selected then
            www_selected = true
            layout:trigger_action(nil, "select-www")
        end
        return
    elseif nav == "down" then
        if www_selected then
            www_selected = false
            layout:trigger_action(nil, "select-trailer")
        end
        return
    elseif nav == "back" then
        return
    end
    if not www_selected then
        video:stop()
        video:play()
        if bg_music then
            timer_callback_cancel(bg_music_fade_id)
            bg_music:pause()
        end
        video_playing = true
        layout:trigger_action("trailer", "fade_in")
        return
    end
    Environment:open_www_link("https://www.kickstarter.com/projects/funkin/friday-night-funkin-the-full-ass-game/")
end
www_selected = true
video_playing = false
video = nil
bg_music = modding_get_native_background_music()
bg_music_fade_id = -1
function f_modding_init(arg)
    layout = modding_get_layout()
    video = layout:get_videoplayer("trailer")
    modding_set_exit_delay(500)
    if bg_music then
        bg_music:fade(false, 2000)
        bg_music_fade_id = timer_callback_timeout(
            1000,
            function()
                bg_music:set_volume(0.7)
            end
        )
    end
end
function f_modding_back()
    if video_playing then
        funkin_handle_nav("back")
        return true
    end
    if bg_music then
        bg_music:set_volume(1)
    end
    layout:trigger_action(nil, "outro")
    return false
end
function f_buttons(player_id, buttons)
    if buttons & (1 | 4 | 256) ~= 0 then
        funkin_handle_nav("ok")
    elseif buttons & 8 ~= 0 then
        funkin_handle_nav("back")
    elseif buttons & 71565328 ~= 0 then
        funkin_handle_nav("up")
    elseif buttons & 143130656 ~= 0 then
        funkin_handle_nav("down")
    end
end
function f_frame(elapsed)
    if video_playing and video:has_ended() then
        funkin_handle_nav("ok")
    end
end
