--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
-- Lua Library inline imports
local function __TS__New(target, ...)
    local instance = setmetatable({}, target.prototype)
    instance:____constructor(...)
    return instance
end

local function __TS__ArraySort(self, compareFn)
    if compareFn ~= nil then
        table.sort(
            self,
            function(a, b) return compareFn(nil, a, b) < 0 end
        )
    else
        table.sort(self)
    end
    return self
end

local function __TS__Class(self)
    local c = {prototype = {}}
    c.prototype.__index = c.prototype
    c.prototype.constructor = c
    return c
end

-- End of Lua Library inline imports
function moveTank(elapsed)
    if not inCutscene then
        local daAngleOffset = 1
        tankAngle = tankAngle + elapsed * tankSpeed
        local angle = tankAngle - 90 + 15
        local x = tankX + math.cos(math.rad(tankAngle * daAngleOffset + 180)) * 1500
        local y = 1300 + math.sin(math.rad(tankAngle * daAngleOffset + 180)) * 1100
        tankGroundModifier.rotate = math.rad(angle)
        tankGround:set_draw_location(x, y)
    end
end
function loadMappedAnims()
    local swagshit = JSON:parse_from_file("funkin/preload/data/stress/picospeaker.json").song
    local notes = swagshit.notes
    for ____, section in ipairs(notes) do
        for ____, idk in ipairs(section.sectionNotes) do
            animationNotes[#animationNotes + 1] = idk
        end
    end
    TankmenBG.animationNotes = animationNotes
    __TS__ArraySort(
        animationNotes,
        function(____, val1, val2) return sortAnims(val1, val2) end
    )
end
function sortAnims(val1, val2)
    return val1[1] - val2[1]
end
function tankmanRun_recycle()
    for ____, tankman in ipairs(tankmanRun) do
        if not tankman.inUse then
            return tankman
        end
    end
    return nil
end
function scroll_picospeaker_chart(songPosition)
    songPosition = songPosition + 4000
    do
        local i = animationNotesIndexTankman
        while i < #TankmenBG.animationNotes do
            if TankmenBG.animationNotes[i + 1][1] > songPosition then
                return
            end
            animationNotesIndexTankman = animationNotesIndexTankman + 1
            if math2d_random_boolean(16) then
                local tankman = tankmanRun_recycle()
                if tankman == nil then
                    return
                end
                tankman.strumTime = TankmenBG.animationNotes[i + 1][1]
                tankman:resetShit(
                    500,
                    200 + math.random(50, 100),
                    TankmenBG.animationNotes[i + 1][2] < 2
                )
            end
            i = i + 1
        end
    end
end
function play_picospeaker_anim(songPosition)
    if animationNotesIndexPico < #animationNotes then
        if songPosition > animationNotes[animationNotesIndexPico + 1][1] then
            local shootAnim = 1
            if animationNotes[animationNotesIndexPico + 1][2] >= 2 then
                shootAnim = 3
            end
            shootAnim = shootAnim + math.random(0, 1)
            picospeaker:play_extra(
                "shoot" .. tostring(shootAnim),
                true
            )
            animationNotesIndexPico = animationNotesIndexPico + 1
        end
    end
end
function play_cutscene(what)
    local video_name = "video-" .. what
    local camera_name = "cutscene-" .. what
    week_set_halt(true)
    week_ui_set_visibility(false)
    inCutscene = true
    skip_cutscene = false
    local video = layout:get_videoplayer(video_name)
    layout:trigger_action(video_name, "video_fade_in_and_play")
    layout:trigger_camera(camera_name)
    if what == "ugh" then
        wait_for_video_end(video, true)
    else
        wait_for_camera(video, what == "guns")
    end
end
function wait_for_camera(video, bounce_fade)
    local id
    id = timer_callback_interval(
        1,
        function()
            if not layout:camera_is_completed() then
                return
            end
            timer_callback_cancel(id)
            wait_for_video_end(video, bounce_fade)
        end
    )
end
function wait_for_video_end(video, bounce_fade)
    local id
    id = timer_callback_interval(
        1,
        function()
            if not video:has_ended() and not skip_cutscene then
                return
            end
            if skip_cutscene then
                video:pause()
            end
            timer_callback_cancel(id)
            inCutscene = false
            if bounce_fade then
                week_ui_set_visibility(true)
                do_bounce_fade(video)
            else
                do_fade_out()
            end
        end
    )
end
function do_bounce_fade(video)
    layout:trigger_action(nil, "bounce_fade")
    local id
    id = timer_callback_interval(
        1,
        function()
            if layout:animation_is_completed("fade-effect") < 1 then
                return
            end
            video:get_sprite():set_visible(false)
            week_set_halt(false)
            timer_callback_cancel(id)
        end
    )
end
function do_fade_out()
    layout:trigger_action(nil, "fade_out")
    local id
    id = timer_callback_interval(
        1,
        function()
            if layout:animation_is_completed("fade-effect") < 1 then
                return
            end
            week_set_halt(false)
            timer_callback_cancel(id)
        end
    )
end
SKIP_CUTSCENE_BUTTONS = 256 | 1073741824 | 1 | 2
tankAngle = math.random(-90, 45)
tankSpeed = math.random(5, 7)
tankX = 400
tankmanRun = nil
inCutscene = false
inPauseMenu = false
animationNotes = nil
no_distractions = not Settings.distractionsEnabled
last_jeff_voiceline = nil
function f_weekinit(freeplay_index)
    freeplay_mode = freeplay_index >= 0
    week_override_common_folder("./kickstater_mainmenu_common_assets")
    if not freeplay_mode then
        week_enable_credits_on_completed()
        week_unlockdirective_create("FNF_WEEK7_COMPLETED", false, true, 9942069)
    end
end
function f_beforeready(from_retry)
    layout = week_get_stage_layout()
    songplayer = week_get_songplayer()
    tankGround = layout:get_sprite("tankGround")
    tankGroundModifier = tankGround:matrix_get_modifier()
    local ____, ____, track_index = week_get_current_song_info()
    tankGroundModifier.rotatePivotEnabled = true
    tankGroundModifier.rotatePivotU = 0.5
    tankGroundModifier.rotatePivotV = 0.5
    animationNotesIndexPico = 0
    animationNotesIndexTankman = 0
    if tankmanRun == nil then
        local max_tankman_killed = layout:get_attached_value("max_tankman_killed")
        tankmanRun = {}
        do
            local i = 0
            while i < max_tankman_killed do
                local tempTankman = __TS__New(
                    TankmenBG,
                    i,
                    20,
                    500,
                    true
                )
                tempTankman.strumTime = 10
                tempTankman:resetShit(20, 600, true)
                tankmanRun[#tankmanRun + 1] = tempTankman
                tempTankman:hide()
                i = i + 1
            end
        end
    else
        for ____, tankman in ipairs(tankmanRun) do
            tankman:hide()
        end
    end
    if animationNotes == nil then
        animationNotes = {}
        loadMappedAnims()
    end
    is_ugh_song = track_index == 0
    is_guns_song = track_index == 1
    is_stress_song = track_index == 2
    picospeaker = week_get_girlfriend()
    tankman_captain = week_get_character(0)
    if not freeplay_mode and not from_retry then
        if is_ugh_song then
            play_cutscene("ugh")
        elseif is_guns_song then
            play_cutscene("guns")
        end
    end
end
function f_frame(elapsed)
    if inPauseMenu or no_distractions then
        return
    end
    moveTank(elapsed / 1000)
    if not is_stress_song then
        return
    end
    for ____, tankman in ipairs(tankmanRun) do
        if tankman.inUse then
            tankman:update()
        end
    end
    local songPosition = songplayer:get_timestamp()
    scroll_picospeaker_chart(songPosition)
    play_picospeaker_anim(songPosition)
end
function f_pause(pause_or_resume)
    inPauseMenu = pause_or_resume
end
function f_roundend(loose)
    if not loose and is_guns_song then
        play_cutscene("stress")
    end
end
function f_gameoverloop()
    if not Settings.distractionsEnabled then
        return
    end
    if last_jeff_voiceline ~= nil then
        last_jeff_voiceline:destroy()
    end
    local index = math.random(1, 25)
    last_jeff_voiceline = SoundPlayer:init(("./funkin/week7/sounds/jeffGameover/jeffGameover-" .. tostring(index)) .. ".ogg")
    if last_jeff_voiceline ~= nil then
        last_jeff_voiceline:play()
    end
end
function f_gameoverdecision(retry_or_giveup, changed_difficult)
    if last_jeff_voiceline ~= nil and last_jeff_voiceline:is_playing() then
        last_jeff_voiceline:fade(false, 500)
    end
end
function f_buttons(player_id, buttons)
    if not inCutscene then
        return
    end
    if buttons & SKIP_CUTSCENE_BUTTONS ~= 0 then
        skip_cutscene = true
    end
end
function f_input_keyboard(key, scancode, is_pressed, mods)
    if not inCutscene or not is_pressed then
        return
    end
    if key == 335 or key == 257 or key == 259 or key == 32 then
        skip_cutscene = true
    end
end
TankmenBG = __TS__Class()
TankmenBG.name = "TankmenBG"
function TankmenBG.prototype.____constructor(self, spr_index, x, y, isGoingRight)
    self.strumTime = 0
    self.goingRight = false
    self.tankSpeed = 0.7
    self.inUse = false
    self.sprite_name = "tankmankilled" .. tostring(spr_index)
    self.sprite = layout:get_sprite(self.sprite_name)
    self.sprite:flip_rendered_texture_enable_correction(true)
    self.layout_width = layout:get_viewport_size()
    local ____self_2 = self.sprite
    local ____self_2_set_draw_location_3 = ____self_2.set_draw_location
    local ____x_0 = x
    self.x = ____x_0
    local ____y_1 = y
    self.y = ____y_1
    ____self_2_set_draw_location_3(____self_2, ____x_0, ____y_1)
end
function TankmenBG.prototype.resetShit(self, x, y, isGoingRight)
    local ____self_6 = self.sprite
    local ____self_6_set_draw_location_7 = ____self_6.set_draw_location
    local ____x_4 = x
    self.x = ____x_4
    local ____y_5 = y
    self.y = ____y_5
    ____self_6_set_draw_location_7(____self_6, ____x_4, ____y_5)
    self.goingRight = isGoingRight
    self.endingOffset = math.random(50, 200)
    self.tankSpeed = math2d_random(0.6, 1)
    self.sprite:flip_rendered_texture(isGoingRight, nil)
    self.shot_action_name = "shot" .. tostring(math.random(1, 2))
    self.curAnimIsRun = layout:trigger_action(self.sprite_name, "run") > 0
    self.curAnimIsShoot = false
    self.sprite:set_visible(false)
    self.inUse = true
end
function TankmenBG.prototype.update(self)
    local songPosition = songplayer:get_timestamp()
    local offset_x = 0
    local offset_y = 0
    if self.x >= self.layout_width * 1.2 or self.x <= self.layout_width * -0.5 then
        self.sprite:set_visible(false)
    else
        self.sprite:set_visible(true)
    end
    if self.curAnimIsRun then
        local endDirection = self.layout_width * 0.74 + self.endingOffset
        if self.goingRight then
            endDirection = self.layout_width * 0.02 - self.endingOffset
            self.x = endDirection + (songPosition - self.strumTime) * self.tankSpeed
        else
            self.x = endDirection - (songPosition - self.strumTime) * self.tankSpeed
        end
        self.sprite:set_draw_location(self.x, self.y)
    end
    if songPosition > self.strumTime then
        self.curAnimIsShoot = layout:trigger_action(self.sprite_name, self.shot_action_name) > 0
        self.curAnimIsRun = false
        self.strumTime = math.huge
        if self.goingRight then
            offset_x = -300
        end
        self.sprite:set_draw_location(self.x + offset_x, self.y + offset_y)
    end
    if self.curAnimIsShoot and layout:animation_is_completed(self.sprite_name) > 0 then
        local ____self_8 = self.sprite
        local ____self_8_set_visible_9 = ____self_8.set_visible
        self.inUse = false
        ____self_8_set_visible_9(____self_8, false)
    end
end
function TankmenBG.prototype.hide(self)
    local ____self_10 = self.sprite
    local ____self_10_set_visible_11 = ____self_10.set_visible
    self.inUse = false
    ____self_10_set_visible_11(____self_10, false)
end
TankmenBG.animationNotes = nil
