export { };


declare global {

    //
    // Constants
    //
    const ENGINE_NAME: string;
    const ENGINE_VERSION: string;

    /**
     * Enum version of gamepad buttons, this unifies all GAMEPAD_*** global variables here. Note:
     * In lua values will appear as numbers. Example: "GAMEPAD_DPAD_UP" is replaced by "16" or "0x10"
     */
    const enum GamepadButtons {
        /**
         * This values is never emitted, is just a placeholder for simplify comparisons
         */
        NOTHING = 0 << 0,

        A = 1 << 0,
        B = 1 << 1,
        X = 1 << 2,
        Y = 1 << 3,

        DPAD_UP = 1 << 4,
        DPAD_DOWN = 1 << 5,
        DPAD_RIGHT = 1 << 6,
        DPAD_LEFT = 1 << 7,

        START = 1 << 8,
        SELECT = 1 << 9,

        TRIGGER_LEFT = 1 << 10,
        TRIGGER_RIGHT = 1 << 11,
        BUMPER_LEFT = 1 << 12,
        BUMPER_RIGHT = 1 << 13,

        APAD_UP = 1 << 14,
        APAD_DOWN = 1 << 15,
        APAD_RIGHT = 1 << 16,
        APAD_LEFT = 1 << 17,

        DPAD2_UP = 1 << 18,
        DPAD2_DOWN = 1 << 19,
        DPAD2_RIGHT = 1 << 20,
        DPAD2_LEFT = 1 << 21,

        DPAD3_UP = 1 << 22,
        DPAD3_DOWN = 1 << 23,
        DPAD3_RIGHT = 1 << 24,
        DPAD3_LEFT = 1 << 25,

        DPAD4_UP = 1 << 26,
        DPAD4_DOWN = 1 << 27,
        DPAD4_RIGHT = 1 << 28,
        DPAD4_LEFT = 1 << 29,

        BACK = 1 << 30,


        // combinations
        AD_UP = DPAD_UP | APAD_UP,
        AD_DOWN = DPAD_DOWN | APAD_DOWN,
        AD_RIGHT = DPAD_RIGHT | APAD_RIGHT,
        AD_LEFT = DPAD_LEFT | APAD_LEFT,
        AD = AD_DOWN | AD_UP | AD_LEFT | AD_RIGHT,
        T_LR = TRIGGER_LEFT | TRIGGER_RIGHT,
        B_LR = BUMPER_LEFT | BUMPER_RIGHT,
        DALL_UP = DPAD_UP | DPAD2_UP | DPAD3_UP | DPAD4_UP,
        DALL_DOWN = DPAD_DOWN | DPAD2_DOWN | DPAD3_DOWN | DPAD4_DOWN,
        DALL_RIGHT = DPAD_RIGHT | DPAD2_RIGHT | DPAD3_RIGHT | DPAD4_RIGHT,
        DALL_LEFT = DPAD_LEFT | DPAD2_LEFT | DPAD3_LEFT | DPAD4_LEFT,
    }

    //
    // Enumerations (literals in lua)
    //
    const enum AnimInterpolator {
        EASE = "ease",
        EASE_IN = "ease-in",
        EASE_OUT = "ease-out",
        EASE_IN_OUT = "ease-in-out",
        LINEAR = "linear",
        STEPS = "steps",
    }
    const enum TextSpriteForceCase {
        NONE = "none",
        LOWER = "lower",
        UPPER = "upper"
    }
    const enum Align {
        BOTH = "both",
        NONE = "none",
        START = "start",
        CENTER = "center",
        END = "end"
    }
    const enum NoteState {
        PENALITY = 0,
        MISS = 1,
        SHIT = 2,
        BAD = 3,
        GOOD = 4,
        SICK = 5
    }
    const enum PVRFlag {
        ENABLE = "enable",
        DISABLE = "disable",
        DEFAULT = "default"
    }
    const enum FontWordBreak {
        NONE = "none",
        BREAK = "break",
        LOOSE = "loose"
    }
    const enum CharacterActionType {
        MISS = "miss",
        EXTRA = "extra",
        IDLE = "idle",
        SING = "sing",
        NONE = "none"
    }
    const enum StepsMethod {
        BOTH = "both",
        NONE = "none",
        START = "start",
        END = "end"
    }
    const enum Blend {
        DEFAULT = "DEFAULT",
        ZERO = "ZERO",
        ONE = "ONE",
        SRC_COLOR = "SRC_COLOR",
        ONE_MINUS_SRC_COLOR = "ONE_MINUS_SRC_COLOR",
        DST_COLOR = "DST_COLOR",
        ONE_MINUS_DST_COLOR = "ONE_MINUS_DST_COLOR",
        SRC_ALPHA = "SRC_ALPHA",
        ONE_MINUS_SRC_ALPHA = "ONE_MINUS_SRC_ALPHA",
        DST_ALPHA = "DST_ALPHA",
        ONE_MINUS_DST_ALPHA = "ONE_MINUS_DST_ALPHA",
        CONSTANT_COLOR = "CONSTANT_COLOR",
        ONE_MINUS_CONSTANT_COLOR = "ONE_MINUS_CONSTANT_COLOR",
        CONSTANT_ALPHA = "CONSTANT_ALPHA",
        ONE_MINUS_CONSTANT_ALPHA = "ONE_MINUS_CONSTANT_ALPHA",
        SRC_ALPHA_SATURATE = "SRC_ALPHA_SATURATE"
    }
    const enum ScrollDirection {
        UPSCROLL = "UPSCROLL",
        LEFTSCROLL = "LEFTSCROLL",
        DOWNSCROLL = "DOWNSCROLL",
        RIGHTSCROLL = "RIGHTSCROLL"
    }
    const enum StrumScriptTarget {
        MARKER = "marker",
        SICK_EFFECT = "sick_effect",
        BACKGROUND = "background",
        STRUM_LINE = "strum_line",
        NOTE = "note",
        ALL = "all"
    }
    const enum StrumScriptOn {
        ON_HIT_DOWN = "on_hit_down",
        ON_HIT_UP = "on_hit_up",
        ON_MISS = "on_miss",
        ON_PENALITY = "on_penality",
        ON_IDLE = "on_idle",
        ON_ALL = "on_all"
    }
    const enum StrumPressState {
        NONE = "none",
        HIT = "hit",
        HIT_SUSTAIN = "hit_sustain",
        PENALTY_NOTE = "penalty_note",
        PENALTY_HIT = "penalty_hit"
    }
    const enum Ranking {
        NONE = 0,
        SICK = 1,
        GOOD = 2,
        BAD = 3,
        SHIT = 4,
        MISS = 5,
        PENALITY = 6
    }

    //
    // Global sub-metatables (classes in typescript/javascript)
    //
    interface LayoutPlaceholderInfo {
        group_id: number;
        align_vertical: Align;
        align_horizontal: Align;

        x: number;
        y: number;
        z: number;

        height: number;
        width: number;

        parallax_x: number;
        parallax_y: number;
        parallax_z: number;

        static_camera: boolean;
    }
    interface AnimListItem {
        readonly name: string;
        readonly isFrameAnimation: boolean;
        readonly isItemMacroAnimation: boolean;
        readonly isItemTweenKeyframeAnimation: boolean;
    }
    interface AtlasEntry {
        readonly name: string;
        readonly x: number;
        readonly y: number;
        readonly width: number;
        readonly height: number;
        readonly frame_x: number;
        readonly frame_y: number;
        readonly frame_width: number;
        readonly frame_height: number;
        readonly pivot_x: number;
        readonly pivot_y: number;
    }

    //
    // Global metatables (classes in typescript/javascript)
    //
    interface Camera {
        set_interpolator_type(type: AnimInterpolator): void;
        set_transition_duration(expresed_in_beats: boolean, value: number): void;
        set_absolute_zoom(z: number): void;
        set_absolute_position(x: number, y: number): void;
        set_offset(x: number, y: number, z: number): void;
        get_offset(): LuaMultiReturn<[number, number, number]>;
        get_modifier(): Modifier;
        move(end_x: number, end_y: number, end_z: number): void;
        slide(start_x: number, start_y: number, start_z: number, end_x: number, end_y: number, end_z: number): void;
        slide_x(start: number, end: number): void;
        slide_y(start: number, end: number): void;
        slide_z(start: number, end: number): void;
        slide_to(x: number, y: number, z: number): void;
        from_layout(camera_name: string): boolean;
        to_origin(should_slide: boolean): void;
        repeat(): void;
        stop(): void;
        end(): void;
        is_completed(): boolean;
        debug_log_info(): void;
        apply(): void;
        move_offset(camera: Camera, end_x: number, end_y: number, end_z: number): void;
        slide_offset(camera: Camera, start_x: number, start_y: number, start_z: number, end_x: number, end_y: number, end_z: number): void;
        slide_x_offset(camera: Camera, start: number, end: number): void;
        slide_y_offset(camera: Camera, start: number, end: number): void;
        slide_z_offset(camera: Camera, start: number, end: number): void;
        slide_to_offset(camera: Camera, x: number, y: number, z: number): void;
        to_origin_offset(camera: Camera, should_slide: boolean): void;
        set_animation(animsprite: AnimSprite): void;
    }
    interface Layout {
        trigger_any(action_triger_camera_interval_name: string): number;
        trigger_action(target_name: string, action_name: string): number;
        trigger_camera(camera_name: string): number;
        trigger_trigger(trigger_name: string): number;
        contains_action(target_name: string, action_name: string): number;
        stop_all_triggers(): void;
        stop_trigger(trigger_name: string): void;
        animation_is_completed(item_name: string): number;
        camera_set_view(x: number, y: number, z: number): void;
        camera_is_completed(): boolean;
        get_camera_helper(): Camera;
        get_secondary_camera_helper(): Camera;
        get_textsprite(name: string): TextSprite;
        get_sprite(name: string): Sprite;
        get_soundplayer(name: string): SoundPlayer;
        get_videoplayer(name: string): VideoPlayer;
        get_viewport_size(): LuaMultiReturn<[number, number]>;
        get_attached_value(name: string): number | string | boolean | null;
        set_group_visibility(group_name: string, visible: boolean): void;
        set_group_alpha(group_name: string, alpha: boolean): void;
        set_group_offsetcolor(group_name: string, r: number, g: number, b: number, a: number): void;
        suspend(): void;
        resume(): void;
        get_placeholder(placeholder_name: string): LayoutPlaceholderInfo;
        disable_antialiasing(antialiasing: boolean): void;
        set_group_antialiasing(group_name: string, antialiasing: PVRFlag): void;
        get_group_modifier(group_name: string): Modifier;
        get_group_shader(layout: Layout, name: string): PSShader;
        set_group_shader(layout: Layout, name: string, psshader: PSShader): boolean;
    }
    interface Sprite {
        matrix_get_modifier(): Modifier;
        set_offset_source(x: number, y: number, width: number, height: number): void;
        set_offset_frame(x: number, y: number, width: number, height: number): void;
        set_offset_pivot(x: number, y: number): void;
        matrix_reset(): void;
        set_draw_location(x: number, y: number): void;
        set_draw_size(width: number, height: number): void;
        set_draw_size_from_source_size(): void;
        set_alpha(alpha: number): void;
        set_visible(visible: boolean): void;
        set_z_index(index: number): void;
        set_z_offset(offset: number): void;
        get_source_size(): LuaMultiReturn<[number, number]>;
        set_vertex_color(r: number, g: number, b: number): void;
        set_offsetcolor(r: number, g: number, b: number, a: number): void;
        is_textured(): boolean;
        crop(dx: number, dy: number, dwidth: number, dheight: number): boolean;
        is_crop_enabled(): boolean;
        crop_enable(enable: boolean): void;
        resize_draw_size(max_width: number, max_height: number): LuaMultiReturn<[number, number]>;
        center_draw_location(x: number, y: number, ref_width: number, ref_height: number): LuaMultiReturn<[number, number]>;
        set_antialiasing(antialiasing: PVRFlag): void;
        flip_rendered_texture(flip_x?: boolean, flip_y?: boolean): void;
        flip_rendered_texture_enable_correction(enabled: boolean): void;
        set_shader(psshader: PSShader): void;
        get_shader(): PSShader;
        blend_enable(enabled: boolean): void;
        blend_set(src_rgb: Blend, dst_rgb: Blend, src_alpha: Blend, dst_alpha: Blend): void;
        trailing_enabled(enabled: boolean): void;
        trailing_set_params(length: number, trail_delay: number, trail_alpha: number, darken_colors?: boolean): void;
        trailing_set_offsetcolor(r: number, g: number, b: number): void;
    }
    interface TextSprite {
        set_text(text: string): void;
        set_font_size(font_size: number): void;
        force_case(none_or_lowercase_or_uppercase: TextSpriteForceCase): void;
        set_paragraph_align(align: Align): void;
        set_paragraph_space(space: number): void;
        set_maxlines(max_lines: number): void;
        set_color_rgba8(rbg8_color: number): void;
        set_color(r: number, g: number, b: number): void;
        set_alpha(alpha: number): void;
        set_visible(visible: boolean): void;
        set_draw_location(x: number, y: number): void;
        set_z_index(z_index: number): void;
        set_z_offset(offset: number): void;
        set_max_draw_size(max_width: number, max_height: number): void;
        matrix_flip(flip_x: boolean, flip_y: boolean): void;
        set_align(align_vertical: Align, align_horizontal: Align): void;
        matrix_get_modifier(): Modifier;
        matrix_reset(): void;
        get_font_size(): number;
        get_draw_size(): LuaMultiReturn<[number, number]>;
        border_enable(enable: boolean): void;
        border_set_size(border_size: number): void;
        border_set_color(r: number, g: number, b: number, a: number): void;
        border_set_offset(x: number, y: number): void;
        set_antialiasing(antialiasing: PVRFlag): void;
        set_wordbreak(wordbreak: FontWordBreak): void;
        set_shader(textsprite: TextSprite, psshader: PSShader): void;
        get_shader(textsprite: TextSprite): PSShader;
        blend_enable(enabled: boolean): void;
        blend_set(src_rgb: Blend, dst_rgb: Blend, src_alpha: Blend, dst_alpha: Blend): void;
        background_enable(enabled: boolean): void;
        background_set_size(size: number): void;
        background_set_offets(offset_x: number, offset_y: number): void;
        background_set_color(r: number, g: number, b: number, a: number): void;
    }
    interface SoundPlayer {
        play(): void;
        pause(): void;
        stop(): void;
        loop_enable(enable: boolean): void;
        fade(in_or_out: boolean, duration: number): void;
        set_volume(volume: number): void;
        set_mute(muted: boolean): void;
        is_muted(): boolean;
        is_playing(): boolean;
        get_duration(): number;
        get_position(): number;
        seek(timestamp: number): void;
        has_ended(): boolean;
    }
    interface VideoPlayer {
        get_sprite(): Sprite;
        replay(): void;
        play(): void;
        pause(): void;
        stop(): void;
        loop_enable(enable: boolean): void;
        fade_audio(in_or_out: boolean, duration: number): void;
        set_volume(volume: number): void;
        set_mute(muted: boolean): void;
        seek(timestamp: number): void;
        is_muted(): boolean;
        is_playing(): boolean;
        get_duration(): number;
        get_position(): number;
        has_ended(): boolean;
        has_video_track(): boolean;
        has_audio_track(): boolean;
    }
    interface Messagebox {
        set_buttons_text(left_text: string, right_text: string): void;
        set_button_single(center_text: string): void;
        set_buttons_icons(left_icon_name: string, right_icon_name: string): void;
        set_button_single_icon(center_icon_name: string): void;
        set_title(text: string): void;
        set_image_background_color(color_rgb8: number): void;
        set_image_background_color_default(): void;
        set_message(text: string): void;
        hide_image_background(hide: boolean): void;
        hide_image(hide: boolean): void;
        show_buttons_icons(show: boolean): void;
        use_small_size(small_or_normal: boolean): void;
        set_image_sprite(sprite: Sprite): void;
        set_image_from_texture(filename: string): void;
        set_image_from_atlas(filename: string, entry_name: string, is_animation: boolean): void;
        hide(animated: boolean): void;
        show(animated: boolean): void;
        set_z_index(z_index: number): void;
        get_modifier(): Modifier;
    }
    interface Character {
        use_alternate_sing_animations(enable: boolean): void;
        set_draw_location(x: number, y: number): void;
        set_draw_align(align_vertical: Align, align_horizontal: Align): void;
        update_reference_size(width: number, height: number): void;
        enable_reference_size(enable: boolean): void;
        set_offset(offset_x: number, offset_y: number): void;
        state_add(modelholder: ModelHolder, state_name: string): number;
        state_toggle(state_name: string): number;
        play_hey(): boolean;
        play_idle(): boolean;
        play_sing(direction: string, prefer_sustain: boolean): boolean;
        play_miss(direction: string, keep_in_hold: boolean): boolean;
        play_extra(extra_animation_name: string, prefer_sustain: boolean): boolean;
        set_idle_speed(speed: number): void;
        set_scale(scale_factor: number): void;
        reset(): void;
        enable_continuous_idle(enable: boolean): void;
        is_idle_active(): boolean;
        enable_flip_correction(character: boolean, enable: boolean): void;
        flip_orientation(character: boolean, enable: boolean): void;
        face_as_opponent(face_as_opponent: boolean): void;
        set_z_index(z: number): void;
        set_z_offset(z_offset: number): void;
        animation_set(animsprite: AnimSprite): void;
        animation_restart(): void;
        animation_end(): void;
        set_color_offset(r: number, g: number, b: number, a: number): void;
        set_color_offset_to_default(): void;
        set_alpha(alpha: number): void;
        set_visible(visible: boolean): void;
        get_modifier(): Modifier;
        has_direction(name: string, is_extra: boolean): boolean;
        get_play_calls(): number;
        get_current_action(): CharacterActionType;
        freeze_animation(enabled: boolean): void;
        trailing_enabled(enabled: boolean): void;
        trailing_set_params(length: number, trail_delay: number, trail_alpha: number, darken_colors?: boolean): void;
        trailing_set_offsetcolor(r: number, g: number, b: number): void;
    }
    interface SongPlayer {
        changesong(src: string, prefer_no_copyright: boolean): boolean;
        play(): void;
        pause(): void;
        seek(timestamp: number): void;
        get_duration(): number;
        is_completed(): boolean;
        get_timestamp(): number;
        mute_track(vocals_or_instrumental: boolean, muted: boolean): void;
        mute(muted: boolean): void;
        set_volume_track(vocals_or_instrumental: boolean, volume: number): void;
        set_volume(volume: number): void;
    }
    interface Modifier {
        translateX: number;
        translateY: number;

        rotate: number;

        skewX: number;
        skewY: number;

        scaleX: number;
        scaleY: number;

        scaleDirectionX: number;
        scaleDirectionY: number;

        rotatePivotEnabled: boolean;
        rotatePivotU: number;
        rotatePivotV: number;

        translateRotation: boolean;
        scaleSize: boolean;
        scaleTranslation: boolean;

        x: number;
        y: number;

        width: number;
        height: number;
    }
    interface ModelHolder {
        destroy(): void;
        is_invalid(): boolean;
        has_animlist(): boolean;
        create_animsprite(animation_name: string, fallback_static: boolean, no_return_null: boolean): AnimSprite;
        get_atlas(): Atlas;
        get_vertex_color(): number;
        get_animlist(): AnimList;
        get_atlas_entry(atlas_entry_name: string): AtlasEntry;
        get_atlas_entry2(atlas_entry_name: string): AtlasEntry;
        get_texture_resolution(): LuaMultiReturn<[number, number]>;
        utils_is_known_extension(): boolean;
    }
    interface Dialogue {
        apply_state(state_name: string): boolean;
        apply_state2(state_name: string, if_line_label: string): boolean;
        is_completed(): boolean;
        is_hidden(): boolean;
        show_dialog(dialog_src: string): boolean;
        show_dialog2(text_dialog_content: string): boolean;
        close(): void;
        hide(hidden: boolean): void;
        get_modifier(): Modifier;
        set_offsetcolor(r: number, g: number, b: number, a: number): void;
        set_alpha(alpha: number): void;
        set_antialiasing(antialiasing: PVRFlag): void;
    }
    interface AnimSprite {
        init(animlist_item: AnimListItem): AnimSprite;
        init_from_tweenlerp(tweenlerp: TweenLerp): AnimSprite;
        destroy(): void;
        set_loop(loop: number): void;
        get_name(): string;
        is_frame_animation(): boolean;
        set_delay(delay_milliseconds: number): void;
    }
    interface PSShader {
        destroy(psshader: PSShader): void;
        set_uniform_any(psshader: PSShader, name: string, ...values: number[]): number;
        set_uniform1f(psshader: PSShader, name: string, value: number): boolean;
        set_uniform1i(psshader: PSShader, name: string, value: number): boolean;
    }
    interface TweenLerp {
        destroy(): void;
        end(): void;
        mark_as_completed(): void;
        restart(): void;
        animate(elapsed: number): number;
        animate_percent(percent: number): number;
        is_completed(): boolean;
        get_elapsed(): number;
        get_entry_count(): number;
        peek_value(): number;
        peek_value_by_index(index: number): number;
        peek_entry_by_index(index: number): LuaMultiReturn<[number, number, number]> | null;
        peek_value_by_id(id: number): number;
        change_bounds_by_index(index: number, new_start: number, new_end: number): boolean;
        override_start_with_end_by_index(index: number): boolean;
        change_bounds_by_id(id: number, new_start: number, new_end: number): boolean;
        change_duration_by_index(index: number, new_duration: number): boolean;
        swap_bounds_by_index(index: number): boolean;
        add_ease(id: number, start: number, end: number, duration: number): number;
        add_easein(id: number, start: number, end: number, duration: number): number;
        add_easeout(id: number, start: number, end: number, duration: number): number;
        add_easeinout(id: number, start: number, end: number, duration: number): number;
        add_linear(id: number, start: number, end: number, duration: number): number;
        add_steps(id: number, start: number, end: number, duration: number, steps_count: number, steps_method: StepsMethod): number;
        add_interpolator(id: number, start: number, end: number, duration: number, type: AnimInterpolator): number;
    }
    interface TweenKeyframe {
        destroy(): void;
        animate_percent(percent: number): void;
        get_ids_count(): number;
        peek_value(): number;
        peek_value_by_index(index: number): number;
        peek_entry_by_index(index: number): LuaMultiReturn<[number, number]> | null;
        peek_value_by_id(at: number, id: number): number;
        add_easeout(at: number, id: number, value: number): number;
        add_easeinout(at: number, id: number, value: number): number;
        add_linear(at: number, id: number, value: number): number;
        add_steps(at: number, id: number, value: number, steps_count: number, steps_method: StepsMethod): number;
        add_interpolator(at: number, id: number, value: number, type: AnimInterpolator): number;

    }
    interface Atlas {
        destroy(): void;
        get_index_of(name: string): number;
        get_entry(name: string): AtlasEntry;
        get_entry_with_number_suffix(name_prefix: string): AtlasEntry;
        get_glyph_fps(): number;
        get_texture_resolution(): LuaMultiReturn<[number, number]>;
        utils_is_known_extension(src: string): boolean;
    }
    interface AnimList {
        destroy(): void;
        get_animation(animation_name: string): AnimListItem;

    }
    interface Drawable {
        set_z_index(z_index: number): void;
        get_z_index(): number;
        set_z_offset(offset: number): void;
        set_alpha(alpha: number): void;
        get_alpha(): number;
        set_offsetcolor(r: number, g: number, b: number, a: number): void;
        set_offsetcolor_to_default(): void;
        get_modifier(): Modifier;
        set_antialiasing(antialiasing: PVRFlag): void;
        set_shader(psshader: PSShader): void;
        get_shader(): PSShader;
        blend_enable(enabled: boolean): void;
        blend_set(src_rgb: Blend, dst_rgb: Blend, src_alpha: Blend, dst_alpha: Blend): void;

    }
    interface Strum {
        update_draw_location(x: number, y: number): void;
        set_scroll_speed(speed: number): void;
        set_scroll_direction(direction: ScrollDirection): void;
        set_marker_duration_multiplier(multipler: number): void;
        reset(scroll_speed: number, state_name: string): void;
        force_key_release(): void;
        get_press_state_changes(): number;
        get_press_state(): StrumPressState;
        get_name(): string;
        get_marker_duration(): number;
        set_player_id(player_id: number): void;
        enable_background(enable: boolean): void;
        enable_sick_effect(enable: boolean): void;
        state_add(mdlhldr_mrkr: ModelHolder, mdlhldr_sck_ffct: ModelHolder, mdlhldr_bckgrnd: ModelHolder, state_name: string): void;
        state_toggle(state_name: string): number;
        state_toggle_notes(state_name: string): number;
        state_toggle_sick_effect(state_name: string): boolean;
        state_toggle_marker(state_name: string): number;
        state_toggle_background(state_name: string): boolean;
        set_alpha_background(alpha: number): number;
        set_alpha_sick_effect(alpha: number): number;
        set_keep_aspect_ratio_background(enable: boolean): void;
        draw_sick_effect_apart(enable: boolean): void;
        set_extra_animation(strum_script_target: StrumScriptTarget, strum_script_on: StrumScriptOn, undo: boolean, animsprite: AnimSprite): void;
        set_extra_animation_continuous(strum_script_target: StrumScriptTarget, animsprite: AnimSprite): void;
        disable_beat_synced_idle_and_continous(disabled: boolean): void;
        set_bpm(bpm: number): void;
        set_notes_tweenkeyframe(tweenkeyframe: TweenKeyframe): void;
        set_sickeffect_size_ratio(size_ratio: number): void;
        set_alpha(alpha: number): void;
        set_visible(visible: boolean): void;
        set_draw_offset(offset_milliseconds: number): void;
        get_modifier(): Modifier;
        get_drawable(): Drawable;
        get_duration(): number;
        animation_restart(): void;
        animation_end(): void;
    }
    interface Strums {
        get_drawable(): Drawable;
        set_scroll_speed(speed: number): void;
        set_scroll_direction(direction: ScrollDirection): void;
        set_marker_duration_multiplier(multipler: number): void;
        disable_beat_synced_idle_and_continous(disabled: boolean): void;
        set_bpm(bpm: number): void;
        reset(scroll_speed: number, state_name: string): void;
        force_key_release(): void;
        set_alpha(alpha: number): number;
        enable_background(enable: boolean): void;
        set_keep_aspect_ratio_background(enable: boolean): void;
        set_alpha_background(alpha: number): void;
        set_alpha_sick_effect(alpha: number): void;
        set_draw_offset(offset_milliseconds: number): void;
        state_add(mdlhldr_mrkr: ModelHolder, mdlhldr_sck_ffct: ModelHolder, mdlhldr_bckgrnd: ModelHolder, state_name: string): void;
        state_toggle(state_name: string): number;
        state_toggle_notes(state_name: string): number;
        state_toggle_marker_and_sick_effect(state_name: string): void;
        get_lines_count(): number;
        get_strum_line(index: number): Strum;
        enable_post_sick_effect_draw(enable: boolean): void;
        animation_set(animsprite: AnimSprite): void;
        animation_restart(): void;
        animation_end(): void;
        strums_decorators_get_count(): number;
        strums_decorators_add(modelholder: ModelHolder, animation_name: string, timestamp: number): boolean;
        strums_decorators_add2(modelholder: ModelHolder, animation_name: string, timestamp: number, from_strum_index: number, to_strum_index: number): boolean;
        strums_decorators_set_scroll_speed(speed: number): void;
        strums_decorators_set_alpha(alpha: number): void;
        strums_decorators_set_visible(decorator_timestamp: number, visible: boolean): void;
        
    }
    interface Conductor {
        destroy(): void;
        poll_reset(): void;
        set_character(character: Character): void;
        use_strum_line(strum: Strum): void;
        use_strums(strums: Strums): void;
        disable_strum_line(strum: Strum, should_disable: boolean): boolean;
        remove_strum(strum: Strum): boolean;
        clear_mapping(): void;
        map_strum_to_player_sing_add(strum: Strum, sing_direction_name: string): void;
        map_strum_to_player_extra_add(strum: Strum, extra_animation_name: string): void;
        map_strum_to_player_sing_remove(strum: Strum, sing_direction_name: string): void;
        map_strum_to_player_extra_remove(strum: Strum, extra_animation_name: string): void;
        map_automatically(should_map_extras: boolean): number;
        poll(): void;
        disable(disable: boolean): void;
        play_idle(): void;
        play_hey(): void;
        get_character(): Character;

    }
    interface Countdown {
        set_default_animation2(tweenkeyframe: TweenKeyframe): void;
        set_bpm(bpm: number): void;
        get_drawable(): Drawable;
        ready(): boolean;
        start(): boolean;
        has_ended(): boolean;
    }
    interface PlayerStats {
        add_hit(multiplier: number, base_note_duration: number, hit_time_difference: number): Ranking;
        add_sustain(quarters: number, is_released: boolean): void;
        add_sustain_delayed_hit(multiplier: number, hit_time_difference: number): Ranking;
        add_penality(on_empty_strum: boolean): void;
        add_miss(multiplier: number): void;
        reset(): void;
        reset_notes_per_seconds(): void;
        add_extra_health(multiplier: number): void;
        enable_penality_on_empty_strum(enable: boolean): void;
        get_maximum_health(): number;
        get_health(): number;
        get_accuracy(): number;
        get_last_accuracy(): number;
        get_last_ranking(): Ranking;
        get_last_difference(): number;
        get_combo_streak(): number;
        get_highest_combo_streak(): number;
        get_combo_breaks(): number;
        get_notes_per_seconds(): number;
        get_notes_per_seconds_highest(): number;
        get_iterations(): number;
        get_score(): number;
        get_hits(): number;
        get_misses(): number;
        get_penalties(): number;
        get_shits(): number;
        get_bads(): number;
        get_goods(): number;
        get_sicks(): number;
        set_health(health: number): void;
        add_health(health: number, die_if_negative: boolean): number;
        raise(with_full_health: boolean): void;
        kill(): void;
        kill_if_negative_health(): void;
        is_dead(): boolean;

    }
    interface StreakCounter {
        reset(): void;
        hide_combo_sprite(hide: boolean): void;
        set_combo_draw_location(x: number, y: number): void;
        state_add(combo_modelholder: ModelHolder, number_modelholder: ModelHolder, state_name: string): number;
        state_toggle(state_name: string): boolean;
        set_alpha(alpha: number): void;
        get_drawable(): Drawable;
        animation_set(animsprite: AnimSprite): void;
        animation_restart(): void;
        animation_end(): void;
    }
    interface RoundStats {
        hide(hide: boolean): void;
        hide_nps(hide: boolean): void;
        set_draw_y(y: number): void;
        reset(): void;
        get_drawable(): Drawable;
        tweenkeyframe_set_on_beat(tweenkeyframe: TweenKeyframe, rollback_beats: number, beat_duration: number): void;
        tweenkeyframe_set_on_hit(tweenkeyframe: TweenKeyframe, rollback_beats: number, beat_duration: number): void;
        tweenkeyframe_set_on_miss(tweenkeyframe: TweenKeyframe, rollback_beats: number, beat_duration: number): void;
        tweenkeyframe_set_bpm(beats_per_minute: number): void;
    }
    interface RankingCounter {
        add_state(modelholder: ModelHolder, state_name: string): number;
        toggle_state(state_name: string): void;
        reset(): void;
        hide_accuracy(hide: boolean): void;
        use_percent_instead(use_accuracy_percenter: boolean): void;
        set_default_ranking_animation2(animsprite: AnimSprite): void;
        set_default_ranking_text_animation2(animsprite: AnimSprite): void;
        set_alpha(alpha: number): void;
        animation_set(animsprite: AnimSprite): void;
        animation_restart(): void;
        animation_end(): void;
    }
    interface MissNoteFX {
        play_effect(): void;
    }
    interface HealthBar {
        enable_extra_length(extra_enabled: boolean): void;
        enable_vertical(enable_vertical: boolean): void;
        state_opponent_add(icon_mdlhldr: ModelHolder, bar_mdlhldr: ModelHolder, state_name: string): number;
        state_opponent_add2(icon_mdlhldr: ModelHolder, bar_color_rgb8: number, state_name: string): number;
        state_player_add(icon_mdlhldr: ModelHolder, bar_mdlhldr: ModelHolder, state_name: string): number;
        state_player_add2(icon_modelholder: ModelHolder, bar_color_rgb8: number, state_name: string): number;
        state_background_add(modelholder: ModelHolder, state_name: string): boolean;
        state_background_add2(color_rgb8: number, animsprite: AnimSprite, state_name: string): boolean;
        load_warnings(modelholder: ModelHolder, use_alt_icons: boolean): boolean;
        set_opponent_bar_color(color_rgb8: number): void;
        set_player_bar_color(color_rgb8: number): void;
        state_toggle(state_name: string): number;
        state_toggle_background(state_name: string): boolean;
        state_toggle_player(state_name: string): boolean;
        state_toggle_opponent(state_name: string): boolean;
        set_bump_animation_opponent(animsprite: AnimSprite): void;
        set_bump_animation_player(animsprite: AnimSprite): void;
        bump_enable(enable_bump: boolean): void;
        set_bpm(beats_per_minute: number): void;
        set_alpha(alpha: number): void;
        set_visible(visible: boolean): void;
        get_drawable(): Drawable;
        animation_set(animsprite: AnimSprite): void;
        animation_restart(): void;
        animation_end(): void;
        disable_progress_animation(disable: boolean): void;
        set_health_position(max_health: number, health: number, opponent_recover: boolean): number;
        set_health_position2(percent: number): void;
        disable_icon_overlap(disable: boolean): void;
        disable_warnings(disable: boolean): void;
        enable_low_health_flash_warning(enable: boolean): void;
        hide_warnings(): void;
        show_drain_warning(use_fast_drain: boolean): void;
        show_locked_warning(): void;
        healthbar_get_bar_midpoint(): LuaMultiReturn<[number, number]>;
        healthbar_get_percent(): number;
    }
    interface HealthWatcher {
        add_opponent(playerstats: PlayerStats, can_recover: boolean, can_die: boolean): boolean;
        add_player(playerstats: PlayerStats, can_recover: boolean, can_die: boolean): boolean;
        has_deads(in_players_or_opponents: boolean): number;
        enable_dead(playerstats: PlayerStats, can_die: boolean): boolean;
        enable_recover(playerstats: PlayerStats, can_recover: boolean): boolean;
        clear(): void;
        balance(healthbar: HealthBar): void;
        reset_opponents(): void;
    }
    interface SongProgressbar {
        set_songplayer(songplayer: SongPlayer): void;
        set_duration(duration: number): void;
        get_drawable(): void;
        set_visible(visible: boolean): void;
        set_background_color(r: number, g: number, b: number, a: number): void;
        set_bar_back_color(r: number, g: number, b: number, a: number): void;
        set_bar_progress_color(r: number, g: number, b: number, a: number): void;
        set_text_color(r: number, g: number, b: number, a: number): void;
        hide_time(hidden: boolean): void;
        show_elapsed(elapsed_or_remain_time: boolean): void;
        manual_update_enable(enabled: boolean): void;
        manual_set_text(text: string): void;
        manual_set_position(elapsed: number, duration: number, should_update_time_text: boolean): number;
        animation_set(animsprite: AnimSprite): void;
        animation_restart(): void;
        animation_end(): void;
    }
    interface EngineSettings {
        readonly distractionsEnabled: boolean;
        readonly flahsingLightsEnabled: boolean;
        readonly inversedScrollEnabled: boolean;
        readonly songProgressbarEnabled: boolean;
    }
    interface Environment {
        get_language(): string;
        get_username(): string;
        get_cmdargs(): string;
        exit(exit_code: number): string;
    }
    interface Menu {
        //destroy(): void;
        get_drawable(): Drawable;
        trasition_in(): void;
        trasition_out(): void;
        select_item(name: string): void;
        select_index(index: number): void;
        select_vertical(offset: number): boolean;
        select_horizontal(offset: number): boolean;
        toggle_choosen(enable: boolean): void;
        get_selected_index(): number;
        get_items_count(): number;
        set_item_text(index: number, text: string): boolean;
        set_item_visibility(index: number, visible: boolean): boolean;
        get_item_rect(index: number): LuaMultiReturn<[number, number, number, number]> | null;
        get_selected_item_name(): string;
        set_text_force_case(none_or_lowercase_or_uppercase: TextSpriteForceCase): void;
        has_item(name: string): boolean;
    }

    //
    // Global metatables initializers (class static contructors in typescript/javascript)
    //
    interface ModelHolderConstructor {
        init(src: string): ModelHolder;
        init2(vertex_color_rgb8: number, atlas_src: string, animlist_src: string): ModelHolder;
    } const ModelHolder: ModelHolderConstructor;
    interface AnimSpriteConstructor {
        init_from_atlas(frame_rate: number, loop: number, atlas: Atlas, prefix: string, has_number_suffix: boolean): AnimSprite;
        init_from_animlist(animlist: AnimList, animation_name: string): AnimSprite;
        init_as_empty(name: string): AnimSprite;
    } const AnimSprite: AnimSpriteConstructor;
    interface TweenLerpConstructor {
        init(): TweenLerp;
    } const TweenLerp: TweenLerpConstructor;
    interface TweenKeyframeConstructor {
        init(): TweenKeyframe;
        init2(animlist_item: AnimListItem): TweenKeyframe;
    } const TweenKeyframe: TweenKeyframeConstructor;
    interface AtlasConstructor {
        init(src: string): Atlas;
    } const AtlasConstructor: AtlasConstructor;
    interface AnimListConstructor {
        init(): AnimList;
    } const AnimList: AnimListConstructor;
    interface ConductorConstructor {
        init(): Conductor;
    } const Conductor: ConductorConstructor;
    interface MenuConstructor {
        init(menumanifest: MenuManifest, x: number, y: number, z: number, width: number, height: number): Menu;
    } const Menu: MenuConstructor;


    //
    // Global functions
    //
    function week_ui_set_visibility(visible: boolean): void;
    function week_ui_get_layout(): Layout;
    function week_ui_get_camera(): Camera;
    function week_set_halt(halt: boolean): void;
    function week_ui_get_strums_count(): number;
    function week_ui_get_strums(strums_id: number): Strums;
    function week_ui_get_roundstats(): RoundStats;
    function week_ui_get_rankingcounter(): RankingCounter;
    function week_ui_get_streakcounter(): StreakCounter;
    function week_ui_get_trackinfo(): TextSprite;
    function week_ui_get_songprogressbar(): SongProgressbar;
    function week_ui_get_countdown(): Countdown;
    function week_ui_get_healthbar(): HealthBar;
    function week_get_stage_layout(): Layout;
    function week_get_healthwatcher(): HealthWatcher;
    function week_get_missnotefx(): MissNoteFX;
    function week_update_bpm(bpm: number): void;
    function week_update_speed(speed: number): void;
    function week_get_messagebox(): Messagebox;
    function week_get_girlfriend(): Character;
    function week_get_character_count(): number;
    function week_get_conductor(character_index: number): Conductor;
    function week_get_character(character_index: number): Character;
    function week_get_playerstats(character_index: number): PlayerStats;
    function week_get_songplayer(): SongPlayer;
    function week_get_current_chart_info(): LuaMultiReturn<[number, number]>;
    function week_get_current_track_info(): LuaMultiReturn<[string, string, number]>;
    function week_change_character_camera_name(opponent_or_player: boolean, new_name: string): void;
    function week_disable_layout_rollback(disable: boolean): void;
    function week_override_common_folder(custom_common_path: string): void;
    function week_enable_credits_on_completed(): void;
    function week_end(round_or_week: boolean, loose_or_win: boolean): void;
    function week_get_dialogue(): Dialogue;
    function week_set_ui_shader(psshader: PSShader): void;
    function week_rebuild_ui(): void;
    function week_unlockdirective_create(name: string, completed_round: boolean, completed_week: boolean, value: number): void;
    function week_unlockdirective_get(name: string): number | null;
    function week_unlockdirective_remove(name: string, completed_round: boolean, completed_week: boolean): void;
    function week_storage_get_blob(blob_name: string): string | null;
    function week_storage_set_blob(blob_name: string, blob: string): boolean;




    //
    // Global Timer functions
    //
    type TimerCallback = (...args: any[]) => any;
    function timer_ms_gettime(): number;
    function timer_callback_interval(delay: number, fn: TimerCallback, ...args: any[]): number;
    function timer_callback_timeout(delay: number, fn: TimerCallback, ...args: any[]): number;
    function timer_songcallback_interval(delay: number, fn: TimerCallback, ...args: any[]): number;
    function timer_songcallback_timeout(delay: number, fn: TimerCallback, ...args: any[]): number;
    function timer_callback_cancel(callback_id: number): boolean;
    function timer_callback_cancel_all(): number;


    //
    // Global Math2D functions
    //  
    function math2d_random_boolean(chance: number): boolean;
    function math2d_lerp(start: number, end: number, step: number): number;
    function math2d_inverselerp(start: number, end: number, value: number): number;
    function math2d_nearestdown(value: number, float: number): number;
    function math2d_rotate_point_by_degs(radians: number, x: number, y: number): LuaMultiReturn<[number, number]>;
    function math2d_points_distance(x1: number, y1: number, x2: number, y2: number): number;

    //
    // Filesystem global functions
    //
    function fs_readfile(path: string): string;

    //
    // Modding UI (engine screens/menus only)
    //
    type BasicValue = string | number | boolean | null;
    function modding_get_layout(): Layout;
    function modding_exit(): void;
    function modding_set_halt(halt: boolean): void;
    function modding_unlockdirective_create(name: string, value: number): void;
    function modding_unlockdirective_remove(name: string): void;
    function modding_unlockdirective_get(name: string): number | null;
    function modding_storage_get_blob(week_name: string, blob_name: string): string | null;
    function modding_storage_set_blob(week_name: string, blob_name: string, blob: string): boolean;
    function modding_get_active_menu(): Menu;
    function modding_choose_native_menu_option(name: string): boolean;
    function modding_get_native_menu(): Menu;
    function modding_set_active_menu(menu: Menu): void;
    function modding_get_native_background_music(): SoundPlayer;
    function modding_replace_native_background_music(music_src: string): SoundPlayer;
    function modding_spawn_screen(layout_src: string, script_src: string, arg: BasicValue): BasicValue;
    function modding_set_exit_delay(delay_ms: number): void;
    function modding_get_messagebox(): Messagebox;
    function modding_get_loaded_weeks(): WeekInfo[];
    function modding_launch_week(week_name: string, difficult: string, use_alt_tracks: boolean, boyfriend_character_manifest_src: string, girlfriend_character_manifest_src: string, gameplay_manifest_src: string, freeplay_track_index: number): number;
    function modding_launch_credits(): void;
    function modding_launch_startscreen(): boolean;
    function modding_launch_mainmenu(): boolean;
    function modding_launch_settings(): void;
    function modding_launch_freeplay(): void;
    function modding_launch_weekselector(): number;


    //
    // MenuManifest helpers (modding context only)
    //

    type MenuManifestParameters = {
        font_color: number;
        atlas: string;
        animlist: string;
        font: string;
        font_size: number;
        font_glyph_suffix: string;
        font_color_by_difference: boolean;
        font_border_color: number;
        anim_discarded: string;
        anim_idle: string;
        anim_rollback: string;
        anim_selected: string;
        anim_choosen: string;
        anim_in: string;
        anim_out: string;
        anim_transition_in_delay: number;
        anim_transition_out_delay: number;
        is_vertical: boolean;
        items_align: Align;
        items_gap: number;
        is_sparse: boolean;
        static_index: number;
        is_per_page: boolean;
        items_dimmen: number;
        font_border_size: number;
        texture_scale: number;
        enable_horizontal_text_correction: boolean;
        suffix_selected: string;
        suffix_choosen: string;
        suffix_discarded: string;
        suffix_idle: string;
        suffix_rollback: string;
        suffix_in: string;
        suffix_out: string;
    }
    type MenuManifestPlacement = {
        gap: number;
        x: number;
        y: number;
        dimmen: number;
    }
    type MenuManifestItem = {
        placement: MenuManifestPlacement;
        has_font_color: boolean;
        font_color: number;
        text: string;
        modelholder: string;
        texture_scale: number;
        name: string;
        anim_selected: string;
        anim_choosen: string;
        anim_discarded: string;
        anim_idle: string;
        anim_rollback: string;
        anim_in: string;
        anim_out: string;
        gap: number;
        hidden: boolean;
    }
    type MenuManifest = {
        parameters: MenuManifestParameters;
        items: MenuManifestItem[];
        items_size: number;
    }
    function menumanifest_parse_from_file(src: string): MenuManifest;

    type WeekInfoSong = {
        readonly name: string;
        readonly freeplay_only: boolean;
        readonly freeplay_unlock_directive: string;
        readonly freeplay_song_filename: string;
        readonly freeplay_description: string;
    };
    type WeekInfo = {
        name: string;
        display_name: string;
        description: string;
        custom_folder: string;
        songs: WeekInfoSong[];
        unlock_directive: string;
        emit_directive: string;
    }

    //
    // Engine objects
    //
    const Settings: EngineSettings;
    const Environment: Environment;

    function importScript(lua_script_filename: string): boolean;
    function evalScript(lua_script_filename: string): any;
}
