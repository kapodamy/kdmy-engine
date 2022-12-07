export { };


declare global {

    //
    // Constants
    //
    const ENGINE_NAME: string;
    const ENGINE_VERSION: string;

    //
    // Gamepad button constants (use GamepadButtons enum instead)
    // used inside of "f_buttons()" and "f_pause_optionselected" functions
    //
    const GAMEPAD_A: number;
    const GAMEPAD_B: number;
    const GAMEPAD_X: number;
    const GAMEPAD_Y: number;
    const GAMEPAD_DPAD_UP: number;
    const GAMEPAD_DPAD_DOWN: number;
    const GAMEPAD_DPAD_RIGHT: number;
    const GAMEPAD_DPAD_LEFT: number;
    const GAMEPAD_START: number;
    const GAMEPAD_SELECT: number;
    const GAMEPAD_TRIGGER_LEFT: number;
    const GAMEPAD_TRIGGER_RIGHT: number;
    const GAMEPAD_BUMPER_LEFT: number;
    const GAMEPAD_BUMPER_RIGHT: number;
    const GAMEPAD_APAD_UP: number;
    const GAMEPAD_APAD_DOWN: number;
    const GAMEPAD_APAD_RIGHT: number;
    const GAMEPAD_APAD_LEFT: number;
    const GAMEPAD_DPAD2_UP: number;
    const GAMEPAD_DPAD2_DOWN: number;
    const GAMEPAD_DPAD2_RIGHT: number;
    const GAMEPAD_DPAD2_LEFT: number;
    const GAMEPAD_DPAD3_UP: number;
    const GAMEPAD_DPAD3_DOWN: number;
    const GAMEPAD_DPAD3_RIGHT: number;
    const GAMEPAD_DPAD3_LEFT: number;
    const GAMEPAD_DPAD4_UP: number;
    const GAMEPAD_DPAD4_DOWN: number;
    const GAMEPAD_DPAD4_RIGHT: number;
    const GAMEPAD_DPAD4_LEFT: number;
    const GAMEPAD_BACK: number;

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
        miss = "miss",
        extra = "extra",
        idle = "idle",
        sing = "sing",
        none = "none"
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

    //
    // Helpers
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
        set_shader(sprite: Sprite, psshader: PSShader): void;
        get_shader(sprite: Sprite): PSShader;

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
        set_antialiasing(antialiasing: PVRFlag): void;
        set_wordbreak(wordbreak: FontWordBreak): void;
        set_shader(textsprite: TextSprite, psshader: PSShader): void;
        get_shader(textsprite: TextSprite): PSShader;

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
        is_completed(): boolean;
        is_hidden(): boolean;
        show_dialog(dialog_src: string): boolean;
        close(): void;
        hide(hidden: boolean): void;
        get_modifier(): Modifier;
        set_offsetcolor(r: number, g: number, b: number, a: number): void;
        set_alpha(alpha: number): void;
        set_antialiasing(antialiasing: PVRFlag): void;
    }
    interface AnimSprite {
        init(animlist_item: AnimListItem): AnimSprite;
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
    interface Atlas {
        destroy(): void;
        get_index_of(name: string): number;
        get_entry(name: string): AtlasEntry;
        get_entry_with_number_suffix(name_prefix: string): AtlasEntry;
        get_glyph_fps(): number;
        get_texture_resolution(): LuaMultiReturn<[number, number]>;
        utils_is_known_extension(src: string): boolean;
    }
    interface AnimListItem {
        readonly name: string;
        readonly is_frame_animation: boolean;
        readonly is_item_macro_animation: boolean;
        readonly is_item_tweenlerp_animation: boolean;
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
    interface AtlasConstructor {
        init(src: string): Atlas;
    } const AtlasConstructor: AtlasConstructor;
    interface AnimListConstructor {
        init(): AnimList;

    } const AnimList: AnimListConstructor;



    //
    // Global functions (commented functions are not implemented) (gameplay only)
    //
    function unlockdirective_create(name: string, completed_round: boolean, completed_week: boolean, value: number): void;
    function unlockdirective_remove(name: string, completed_round: boolean, completed_week: boolean): void;
    function unlockdirective_get(name: boolean): number | null;
    function week_set_halt(halt: boolean): void;
    function ui_set_visibility(visible: boolean): void;
    function ui_get_layout(): Layout;
    function week_get_stage_layout(): Layout;
    function ui_get_camera(): Camera;
    function ui_get_strums_count(): number;
    //function ui_get_strums(strums_id:number): Strums;
    //function week_ui_get_roundstats(): Roundstats;
    //function week_ui_get_rankingcounter(): Rankingcounter;
    //function week_ui_get_streakcounter(): Streakcounter;
    function ui_get_trackinfo(): TextSprite;
    //function ui_get_songprogressbar(): SongProgressbar;
    function week_update_bpm(bpm: number): void;
    function week_update_speed(speed: number): void;
    function week_get_messagebox(): Messagebox;
    function week_get_girlfriend(): Character;
    function week_get_character_count(): number;
    function week_get_character(index: number): Character;
    function week_get_songplayer(): SongPlayer;
    function week_get_current_chart_info(): LuaMultiReturn<[number, number]>;
    function week_get_current_track_info(): LuaMultiReturn<[string, string, number]>;
    function week_change_character_camera_name(opponent_or_player: boolean, new_name: string): void;
    function week_disable_layout_rollback(disable: boolean): void;
    function week_override_common_folder(custom_common_path: string): void;
    function week_enable_credits_on_completed(): void;
    function week_end(round_or_week: boolean, loose_or_win: boolean): void;
    function week_get_dialogue(): void;
    function week_set_ui_shader(psshader: PSShader): void;


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

    // constructors
    function engine_create_shader(vertex_sourcecode: string, fragment_sourcecode: string): PSShader;

    //
    // Modding UI (engine menus only)
    //
    function modding_get_layout(): Layout;
    function modding_exit(): void;

}
