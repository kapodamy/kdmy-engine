export { };
//
// There alot to document here
//

declare global {

    //
    // Constants
    //
    /** Name of the engine running this script */
    const ENGINE_NAME: string;
    /** Version of the engine running this script */
    const ENGINE_VERSION: string;

    //
    // Enumerations (literals in lua)
    //
    /**
     * Enum version of gamepad buttons, this unifies all GAMEPAD_*** global variables here. Note:
     * In lua values will appear as numbers. Example: "GAMEPAD_DPAD_UP" is replaced by "16" or "0x10"
     */
    const enum GamepadButtons {
        /**
         * Indicates that nothing is pressed.   
         * This value is never emitted, is just a placeholder for simplify comparisons
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

        //
        // combinations
        //
        /** Combination of {@link DPAD2_UP}|{@link APAD_UP} */
        AD_UP = DPAD_UP | APAD_UP,
        /** Combination of {@link DPAD2_DOWN}|{@link APAD_DOWN} */
        AD_DOWN = DPAD_DOWN | APAD_DOWN,
        /** Combination of {@link DPAD2_RIGHT}|{@link APAD_RIDPAD2_RIGHT} */
        AD_RIGHT = DPAD_RIGHT | APAD_RIGHT,
        /** Combination of {@link DPAD2_LEFT}|{@link APAD_LDPAD2_LEFT} */
        AD_LEFT = DPAD_LEFT | APAD_LEFT,
        /** Combination of {@link AD_DOWN}|{@link AD_UP}|{@link AD_LEFT}|{@link AD_RIGHT} */
        AD = AD_DOWN | AD_UP | AD_LEFT | AD_RIGHT,
        /** Combination of {@link TRIGGER_LEFT}|{@link TRIGGER_RIGHT} */
        T_LR = TRIGGER_LEFT | TRIGGER_RIGHT,
        /** Combination of {@link BUMPER_LEFT}|{@link BUMPER_RIGHT} */
        B_LR = BUMPER_LEFT | BUMPER_RIGHT,
        /** Combination of {@link DPAD_UP}|{@link DPAD2_UP}|{@link DPAD3_UP}|{@link DPAD4_UP} */
        DALL_UP = DPAD_UP | DPAD2_UP | DPAD3_UP | DPAD4_UP,
        /** Combination of {@link DPAD_DOWN}|{@link DPAD2_DOWN}|{@link DPAD3_DOWN}|{@link DPAD4_DOWN} */
        DALL_DOWN = DPAD_DOWN | DPAD2_DOWN | DPAD3_DOWN | DPAD4_DOWN,
        /** Combination of {@link DPAD_RIGHT}|{@link DPAD2_RIGHT}|{@link DPAD3_RIGHT}|{@link DPAD4_RIGHT}*/
        DALL_RIGHT = DPAD_RIGHT | DPAD2_RIGHT | DPAD3_RIGHT | DPAD4_RIGHT,
        /** Combination of {@link DPAD_LEFT}|{@link DPAD2_LEFT}|{@link DPAD3_LEFT}|{@link DPAD4_LEFT} */
        DALL_LEFT = DPAD_LEFT | DPAD2_LEFT | DPAD3_LEFT | DPAD4_LEFT,
    }
    /** Denotes a mathematical function that describes the rate at which a numerical value changes */
    const enum AnimInterpolator {
        /** Indicates that the interpolation starts slowly, accelerates sharply, and then slows gradually towards the end */
        EASE = "ease",
        /** Indicates that the interpolation starts slowly, then progressively speeds up until the end, at which point it stops abruptly */
        EASE_IN = "ease-in",
        /** Indicates that the interpolation starts slowly, speeds up, and then slows down towards the end */
        EASE_OUT = "ease-out",
        /** Indicates that the interpolation starts abruptly and then progressively slows down towards the end */
        EASE_IN_OUT = "ease-in-out",
        /** Indicates that the interpolation is done linearly between its points */
        LINEAR = "linear",
        /** Divides the domain of output values in equidistant steps */
        STEPS = "steps",
        /** easing using cubic curve */
        CUBIC = "cubic",
        /** easing using quadratic curve */
        QUAD = "quad",
        /** easing using exponential curve */
        EXPO = "expo",
        /** easing using sine curve */
        SIN = "sin"
    }
    /** Forces {@link TextSprite} to render the text in the specific case */
    const enum TextSpriteForceCase {
        /** Use the original case */
        NONE = "none",
        /** Force uppercase */
        LOWER = "lower",
        /** Force lowercase */
        UPPER = "upper"
    }
    /** Describes the alignment used */
    const enum Align {
        NONE = "none",
        START = "start",
        CENTER = "center",
        END = "end"
    }
    /** Flags used to configure the graphics backend */
    const enum PVRFlag {
        ENABLE = "enable",
        DISABLE = "disable",
        DEFAULT = "default"
    }
    /** Indicates in {@link TextSprite} how breaks words if does not fit in the draw bounds*/
    const enum FontWordBreak {
        /** Does not break overflowing words (is drawn out-of-bounds) */
        NONE = "none",
        /** Breaks the words before the first overflowing character and draws the remaining characters 
         * in the next paragraph */
        BREAK = "break",
        /** The overflowing word is send used in the next paragraph */
        LOOSE = "loose"
    }
    /** Indicates what type of action (and what animation) are currently used in {@link Character} */
    const enum CharacterActionType {
        /** Missed note and/or penality animation */
        MISS = "miss",
        /** An extra animation declared in the character manifest, for example "HEY!" */
        EXTRA = "extra",
        /** Idle animation */
        IDLE = "idle",
        /** Sing animation */
        SING = "sing",
        /** Nothing is being played, this normally represents a missing animation */
        NONE = "none"
    }
    /** Indicates the method used in a step interpolator */
    const enum StepsMethod {
        /** Denotes that jumps occur at both the 0% and 100% marks, 
         * effectively adding a step during the interpolation iteration */
        BOTH = "both",
        /** Denotes no jump on either end.  
         * Instead, holding at both the 0% mark and the 100% mark, each for 1/n of the duration */
        NONE = "none",
        /** Denotes that the first step or jump happens when the interpolation begins */
        START = "start",
        /** Denotes that the last step or jump happens when the interpolation ends */
        END = "end"
    }
    /** Blending type (from OpenGL/WebGL2/OpenGLES). Note: some types are not supported by the PowerVR */
    const enum Blend {
        /** Default blending used by the engine */
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
    /** Describes the type of strum scrolling, there no CENTERSCROLL here. */
    const enum ScrollDirection {
        UPSCROLL = "UPSCROLL",
        LEFTSCROLL = "LEFTSCROLL",
        DOWNSCROLL = "DOWNSCROLL",
        RIGHTSCROLL = "RIGHTSCROLL"
    }
    /** Indicates in what moment the animation is used */
    const enum StrumOn {
        ON_HIT_DOWN = "on_hit_down",
        ON_HIT_UP = "on_hit_up",
        ON_MISS = "on_miss",
        ON_PENALITY = "on_penality",
        ON_IDLE = "on_idle",
        /** Use in all previous events */
        ON_ALL = "on_all"
    }
    /** Indicates in what part of the {@link Strum} is attached */
    const enum StrumTarget {
        MARKER = "marker",
        SICK_EFFECT = "sick_effect",
        BACKGROUND = "background",
        /** in the whole strum */
        STRUM_LINE = "strum_line",
        /** In each visible note */
        NOTE = "note",
        /** In All previous parts */
        ALL = "all"
    }
    /** Indicates the current {@link Strum} press state */
    const enum StrumPressState {
        /** nothing has been pressed or a sustain note was released */
        NONE = 0,
        /** A normal note was cleared */
        HIT = 1,
        /** A sustain note is begin cleared */
        HIT_SUSTAIN = 2,
        /** A normal note was cleared with the wrong buttons */
        PENALTY_NOTE = 3,
        /** A button was pressed without notes (hit on empty strum) */
        PENALTY_HIT = 4,
        /** Missed note or sustain part is getting missed */
        MISS = 5
    }
    /** Indicates the Rank of the last cleared note */
    const enum Ranking {
        NONE = 0,
        SICK = 1,
        GOOD = 2,
        BAD = 3,
        SHIT = 4,
        MISS = 5,
        PENALITY = 6
    }
    /** Indicates which fade is active in the sound  */
    const enum Fading {
        /** No fade active */
        NONE = 0,
        IN = 1,
        OUT = 2
    }
    /** Indicates which mouse button was pressed (not available on the dreamcast) */
    const enum MouseButton {
        BUTTON1 = 0,
        BUTTON2 = 1,
        BUTTON3 = 2,
        BUTTON4 = 3,
        BUTTON5 = 4,
        BUTTON6 = 5,
        BUTTON7 = 6,
        BUTTON8 = 7
    }
    /** Indicates what modifier keys are pressed (not available on the dreamcast) */
    const enum ModKeys {
        Shift = 0x0001,
        Control = 0x0002,
        Alt = 0x0004,
        Super = 0x0008,
        CapsLock = 0x0010,
        NumLock = 0x0020
    }
    /** Indicates which keyboard key is pressed (not available on the dreamcast) */
    const enum Keys {
        Unknown = -1,
        Space = 32,
        Apostrophe = 39,
        Comma = 44,
        Minus = 45,
        Period = 46,
        Slash = 47,
        Numpad0 = 48,
        Numpad1 = 49,
        Numpad2 = 50,
        Numpad3 = 51,
        Numpad4 = 52,
        Numpad5 = 53,
        Numpad6 = 54,
        Numpad7 = 55,
        Numpad8 = 56,
        Numpad9 = 57,
        Semicolon = 59,
        Equal = 61,
        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,
        LeftBracket = 91,
        Backslash = 92,
        RightBracket = 93,
        GraveAccent = 96,
        World_1 = 161,
        World_2 = 162,
        Escape = 256,
        Enter = 257,
        Tab = 258,
        Backspace = 259,
        Insert = 260,
        Delete = 261,
        Right = 262,
        Left = 263,
        Down = 264,
        Up = 265,
        PageUp = 266,
        PageDown = 267,
        Home = 268,
        End = 269,
        CapsLock = 280,
        ScrollLock = 281,
        NumLock = 282,
        PrintScreen = 283,
        Pause = 284,
        F1 = 290,
        F2 = 291,
        F3 = 292,
        F4 = 293,
        F5 = 294,
        F6 = 295,
        F7 = 296,
        F8 = 297,
        F9 = 298,
        F10 = 299,
        F11 = 300,
        F12 = 301,
        F13 = 302,
        F14 = 303,
        F15 = 304,
        F16 = 305,
        F17 = 306,
        F18 = 307,
        F19 = 308,
        F20 = 309,
        F21 = 310,
        F22 = 311,
        F23 = 312,
        F24 = 313,
        F25 = 314,
        KP_0 = 320,
        KP_1 = 321,
        KP_2 = 322,
        KP_3 = 323,
        KP_4 = 324,
        KP_5 = 325,
        KP_6 = 326,
        KP_7 = 327,
        KP_8 = 328,
        KP_9 = 329,
        KP_Decimal = 330,
        KP_Divide = 331,
        KP_Multiply = 332,
        KP_Subtract = 333,
        KP_Add = 334,
        KP_Enter = 335,
        KP_Equal = 336,
        LeftShift = 340,
        LeftControl = 341,
        LeftAlt = 342,
        LeftSuper = 343,
        RightShift = 344,
        RightControl = 345,
        RightAlt = 346,
        RightSuper = 347,
        Menu = 348
    }

    //
    // Global sub-metatables (classes in typescript/javascript)
    //
    /** {@link Layout} placeholder */
    interface LayoutPlaceholderInfo {
        /** Id of the layout Group, zero means the root layout */
        groupId: number;
        /** Item vertical alignment, used in engine componentes or as hint */
        alignVertical: Align;
        /** Item horizontal alignment, used in engine componentes or as hint */
        alignHorizontal: Align;

        /** X position in pixels in the layout, 0 if absent */
        x: number;
        /** Y position in pixels in the layout, 0 if absent */
        y: number;
        /** Z-index in the layout, 0 if absent */
        z: number;

        /** Height in pixels, -1 if absent */
        height: number;
        /** Width in pixels, -1 if absent */
        width: number;

        /** Parallax X coefficient, 1.0 if absent */
        parallaxX: number;
        /** Parallax Y coefficient, 1.0 if absent */
        parallaxY: number;
        /** Parallax Z coefficient, 1.0 if absent.  
         *  Like translation coefficients, but applied to camera zooms */
        parallaxZ: number;

        /** Indicates if this placeholder should be static to cameras */
        staticCamera: boolean;
    }
    /** Read-only information about an {@link AnimList} item. The described item can be only a specific type */
    interface AnimListItem {
        /** Name of this entry (also used as animation name) */
        readonly name: string;
        /** Indicates if this item includes only {@link Atlas} entries. */
        readonly isFrameAnimation: boolean;
        /** Indicates if this item uses a Markup scripted animation. */
        readonly isMacroAnimation: boolean;
        /** Indicates if this item contains instructions required to build an {@link TweenKeyframe}. */
        readonly isTweenKeyframeAnimation: boolean;
    }
    /** Copy of {@link Atlas} entry, all values here are expressed in pixels (except the pivots) */
    interface AtlasEntry {
        /** Entry name */
        name: string;
        /** X position in the texture */
        x: number;
        /** Y position in the texture */
        y: number;
        /** Width of the sub-texture */
        width: number;
        /** Height of the sub-texture */
        height: number;
        /** Frame offset x. */
        frameX: number;
        /** Frame offset y. */
        frameY: number;
        /** Frame offset real width. */
        frameWidth: number;
        /** Frame offset real height. */
        frameHeight: number;
        /** Pivot X value, the purpose of this field is unknown and is always ignored by the engine */
        pivotX: number;
        /** Pivot Y value, the purpose of this field is unknown and is always ignored by the engine */
        pivotY: number;
    }
    /** Accumulated statistics */
    interface WeekResult_Stats {
        sick: number;
        good: number;
        bads: number;
        shits: number;
        miss: number;
        penalties: number;
        score: number;
        accuracy: number;
        notesPerSeconds: number;
        comboBreaks: number;
        highestStreak: number;
    }

    //
    // Global metatables (classes in typescript/javascript)
    //
    /** 2D Camera used by {@link Layout}, this camera contais two type of focus and both
     * can be used at the same time:
     * - Offset: scrolls the layout inside of his viewport.
     * - Parallax: same as offset but gives displacement in specific layout items.
     */
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
        disable_offset_zoom(disable: boolean): void;
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
    /**
     * 2D graphics scene, a layout contains graphical items similar to SVG/HMTL. This incredible piece of software 
     * is widely used on all engine screens.  
     * Items wich layout can have are: sprite, text, sound, video, and placeholders.  
     * Layout also features cameras, actions, triggers, groups, shaders, and embedding values.
     */
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
        get_group_visibility(group_name: string): boolean;
        suspend(): void;
        resume(): void;
        get_placeholder(placeholder_name: string): LayoutPlaceholderInfo;
        disable_antialiasing(antialiasing: boolean): void;
        set_group_antialiasing(group_name: string, antialiasing: PVRFlag): void;
        get_group_modifier(group_name: string): Modifier;
        get_group_shader(name: string): PSShader;
        set_group_shader(name: string, psshader: PSShader): boolean;
        screen_to_layout_coordinates(screen_x: number, screen_y: number, calc_with_camera: boolean): LuaMultiReturn<[number, number]>;
    }
    /** 2D Sprite */
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
    /** 2D Text rendering */
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
    /** An OGG Vorbis sound player  */
    interface SoundPlayer {
        destroy(): void;
        play(): void;
        pause(): void;
        stop(): void;
        loop_enable(enable: boolean): void;
        fade(in_or_out: boolean, duration: number): void;
        set_volume(volume: number): void;
        set_mute(muted: boolean): void;
        has_fading(): Fading;
        is_muted(): boolean;
        is_playing(): boolean;
        get_duration(): number;
        get_position(): number;
        seek(timestamp: number): void;
        has_ended(): boolean;
    }
    /** Video player, any format is accepted.  
     * Note: requires FFmpeg libraries to work */
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
    /** A dialog box component, to distinguish it from character dialogs ({@link Dialogue}) it was named MessageBox */
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
    /** Describes a FNF character, this component is composed of various animations and states */
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
        get_commited_animations_count(): number;
        get_current_action(): CharacterActionType;
        freeze_animation(enabled: boolean): void;
        trailing_enabled(enabled: boolean): void;
        trailing_set_params(length: number, trail_delay: number, trail_alpha: number, darken_colors?: boolean): void;
        trailing_set_offsetcolor(r: number, g: number, b: number): void;
        schedule_idle(): void;
    }
    /**
     * An OGG Vorbis sound player, the difference with {@link SoundPlayer} is that it allows the use 
     * of two tracks (instrumental and voices).  
     * Also allows syncronize the start of the playback.
     *  */
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
    /** Matrix modifier, this component provides an easy way to manipulate transform matrices */
    interface Modifier {
        translateX: number;
        translateY: number;

        rotate: number;
        rotateByDegrees: number;

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
    /**
     * Decribes the parts of an 2D Model. This is widely used to build sprites and animations. 
     * The parts are: {@link Atlas}, {@link AnimList}, Texture and an optional vertex color.
     */
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
    /**
     * Represents an interface to displays character dialogs. In order to use this component, is required 
     * an xml file (the dialogue params file) describing all parameters like:
     * - Portraits
     * - Backgrounds
     * - Sounds
     * - Font
     * - Animations
     * - Speech texture
     * - Positions, offsets, size, etc.
     */
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
    /** Abstract represention of an animation, the underlying animation can be a tween, macro or frames */
    interface AnimSprite {
        init(animlist_item: AnimListItem): AnimSprite;
        init_from_tweenlerp(tweenlerp: TweenLerp): AnimSprite;
        destroy(): void;
        set_loop(loop: number): void;
        get_name(): string;
        is_frame_animation(): boolean;
        set_delay(delay_milliseconds: number): void;
    }
    /** Post-processing OpenGL shader */
    interface PSShader {
        destroy(psshader: PSShader): void;
        set_uniform_any(psshader: PSShader, name: string, ...values: number[]): number;
        set_uniform1f(psshader: PSShader, name: string, value: number): boolean;
        set_uniform1i(psshader: PSShader, name: string, value: number): boolean;
    }
    /** Component used to create tweens. The actual LUA API does not allow animate other components like sprite */
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
        add_cubic(id: number, start: number, end: number, duration: number): number;
        add_quad(id: number, start: number, end: number, duration: number): number;
        add_expo(id: number, start: number, end: number, duration: number): number;
        add_sin(id: number, start: number, end: number, duration: number): number;
        add_interpolator(id: number, start: number, end: number, duration: number, type: AnimInterpolator): number;
    }
    /** A reduced version of TweenLerp, this uses "keyframes" instead in similar way like in CSS.  
     * @see {@link https://developer.mozilla.org/docs/Web/CSS/@keyframes} for further information.
     */
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
        add_ease(at: number, id: number, value: number): number;
        add_easein(at: number, id: number, value: number): number;
        add_cubic(at: number, id: number, value: number): number;
        add_quad(at: number, id: number, value: number): number;
        add_expo(at: number, id: number, value: number): number;
        add_sin(at: number, id: number, value: number): number;
        add_interpolator(at: number, id: number, value: number, type: AnimInterpolator): number;

    }
    /** Describes an texture atlas in xml format (Starling Texture Atlas) */
    interface Atlas {
        destroy(): void;
        get_index_of(name: string): number;
        get_entry(name: string): AtlasEntry;
        get_entry_with_number_suffix(name_prefix: string): AtlasEntry;
        get_glyph_fps(): number;
        get_texture_resolution(): LuaMultiReturn<[number, number]>;
        utils_is_known_extension(src: string): boolean;
    }
    /** Animation list, each item on this list contains the animation "sourcecode"  */
    interface AnimList {
        destroy(): void;
        get_animation(animation_name: string): AnimListItem;
    }
    /** Represents a 2D graphical component, this is used for referencing software-rendered components like strums */
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
    /** Represents a single strum line, each strum contains is own set of notes to scroll and check */
    interface Strum {
        update_draw_location(x: number, y: number): void;
        set_scroll_speed(speed: number): void;
        set_scroll_direction(direction: ScrollDirection): void;
        set_marker_duration_multiplier(multipler: number): void;
        reset(scroll_speed: number, state_name: string): void;
        force_key_release(): void;
        get_press_state_changes(): number;
        get_press_state(): StrumPressState;
        get_press_state_use_alt_anim(): boolean;
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
        set_extra_animation(strum_script_target: StrumTarget, strum_script_on: StrumOn, undo: boolean, animsprite: AnimSprite): void;
        set_extra_animation_continuous(strum_script_target: StrumTarget, animsprite: AnimSprite): void;
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
    /**
     * Represents a pack of various {@link Strum} lines (normally 4), also provides an handy way to manipulate
     * various strum lines
     */
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
    /**
     * Engine component responsible for reading {@link Strum} states and playing the corresponding {@link Character} 
     * animations. Also provides an API to bind multiple strum lines to a specific character action/animation.
    */
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
    /** An UI elements used for the countdown, also responsible for asking "__Ready?__" */
    interface Countdown {
        set_default_animation2(tweenkeyframe: TweenKeyframe): void;
        set_bpm(bpm: number): void;
        get_drawable(): Drawable;
        ready(): boolean;
        start(): boolean;
        has_ended(): boolean;
    }
    /** Player statistics, this compents tracks the player health as well the amount of cleared notes */
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
    /** UI cosmetic, displays the amount of note hits, the streak always starts at 10 */
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
    /** Displays an informative text about the current song statistics like the precission and score */
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
    /** UI cosmetic, stacks on the screen the rank of each note hit */
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
    /** Play a sound when the player commits a penality or presses the wrong note buttons */
    interface MissNoteFX {
        missnotefx_disable(disabled: boolean): void;
        stop(): void;
        play_effect(): void;
    }
    /** Displays a progress bar indicating the shared health between two characters */
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
        set_opponent_bar_color(r: number, g: number, b: number): void;
        set_opponent_bar_color_rgb8(color_rgb8: number): void;
        set_player_bar_color(r: number, g: number, b: number): void;
        set_player_bar_color_rgb8(color_rgb8: number): void;
        state_toggle(state_name: string): number;
        state_toggle_background(state_name: string): boolean;
        state_toggle_player(state_name: string): number;
        state_toggle_opponent(state_name: string): number;
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
    /** Manages the health of all opponents and players on the round, also ends the round when a player dies. */
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
    /** Displays a progress bar showing the current round progress */
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
    /** Provides read-only access to relevant engine settings */
    interface EngineSettings {
        readonly distractionsEnabled: boolean;
        readonly flahsingLightsEnabled: boolean;
        readonly inversedScrollEnabled: boolean;
        readonly songProgressbarEnabled: boolean;
    }
    /** Provides access to platform-specific features. Note: some values are not available on the dreamcast */
    interface Environment {
        get_language(): string;
        get_username(): string;
        get_cmdargs(): string[];
        exit(exit_code: number): string;
        change_window_title(title: string): void;
        require_window_attention(): void;
        open_www_link(url: string): void;
        get_screensize(): LuaMultiReturn<[number, number]>;
    }
    /** Represents a list of options which can be selected by the user */
    interface Menu {
        //destroy(): void;
        get_drawable(): Drawable;
        trasition_in(): void;
        trasition_out(): void;
        select_item(name: string): boolean;
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
        index_of_item(name: string): boolean;
        set_item_image(index: number, modelholder: ModelHolder, atlas_or_animlist_entry_name: string): void;
    }
    /** Provides a JSON parser */
    interface JSON {
        /**
         * Open and parse a JSON file.  
         * Note: properties or array items with null values are discarded by lua, this means doing
         * "obj.someprop == null" or "obj[123] == null" is always evaluated as false.
         * @param json_filename json filename
         * @returns the parsed content or null if fails
         */
        parse_from_file(json_filename: string): any | null;

        /**
         * parses a JSON string.  
         * Note: properties or array items with null values are discarded by lua, this means doing
         * "obj.someprop == null" or "obj[123] == null" is always evaluated as false.
         * @param json_sourcecode json string to parse
         * @returns the parsed content or null if fails
         */
        parse(json_sourcecode: string): any | null;
    }


    //
    // Global metatables initializers (class static contructors in typescript)
    //

    interface ModelHolderConstructor {
        /**
         * Initializes a ModelHolder instance.  
         * If a similar model exists this function returns a shared reference.
         * @param src the filename of the atlas file or 2D Model json file.
         * @returns a new {@link ModelHolder} instance, otherwise, null if fails
         */
        init(src: string): ModelHolder;
        /**
         * Initializes a new ModelHolder instance using the specified atlas and animlist. The texture filename
         * is obtained from the atlas or atlas filename.  
         * If a similar model exists this function returns a shared reference.
         * @param vertex_color_rgb8 the vertex color (used in {@link Sprite}), normally this value is never used
         * @param atlas_src filename of an atlas xml file
         * @param animlist_src filename of an animlist xml file
         * @returns a new {@link ModelHolder} instance, otherwise, null if fails
         */
        init2(vertex_color_rgb8: number, atlas_src: string, animlist_src: string): ModelHolder;
    } const ModelHolder: ModelHolderConstructor;
    interface AnimSpriteConstructor {
        /**
         * Creates a new animation from the specified atlas
         * @param frame_rate FPS or 0 to use the default frame rate (which is 24)
         * @param loop number of times the animation will be played, use 0 to loop forever
         * @param atlas An {@link Atlas} instance to take the frames, can not be null
         * @param prefix Prefix name of the atlas entries to load
         * @param has_number_suffix true to only import atlas entries ending with numbers, otherwise, false
         * @returns a new {@link AnimSprite} instance, otherwise, null if fails
         */
        init_from_atlas(frame_rate: number, loop: number, atlas: Atlas, prefix: string, has_number_suffix: boolean): AnimSprite;
        /**
         * Creates a new animation from the specified animlist
         * @param animlist An {@link AnimList} instance to read the animation, can not be null
         * @param animation_name Name of the animation, can not be null
         * @returns a new {@link AnimSprite} instance, otherwise, null if fails
         */
        init_from_animlist(animlist: AnimList, animation_name: string): AnimSprite;
        /**
         * Creates an empty animation
         * @param name a STUB name for this animation
         * @returns a new {@link AnimSprite} instance, never returns null
         */
        init_as_empty(name: string): AnimSprite;
    } const AnimSprite: AnimSpriteConstructor;
    interface TweenLerpConstructor {
        /**
         * Initializes a new TweenLerp instance
         * @returns a new {@link TweenLerp} instance, never returns null
         */
        init(): TweenLerp;
    } const TweenLerp: TweenLerpConstructor;
    interface TweenKeyframeConstructor {
        /**
         * Creates a new empty TweenKeyframe instance
         * @returns a {@link TweenKeyframe}, never returns null
         */
        init(): TweenKeyframe;
        /**
         * Creates a new empty TweenKeyframe instance from an AnimListItem
         * @param animlist_item An {@link AnimListItem} describing the keyframes, can not be null
         * @returns a {@link TweenKeyframe}, never returns null
         */
        init2(animlist_item: AnimListItem): TweenKeyframe;
    } const TweenKeyframe: TweenKeyframeConstructor;
    interface AtlasConstructor {
        /**
         * Initializes a new {@link Atlas} instance
         * @param src filename of the atlas xml file
         * @returns a {@link Atlas}, otherwise, null if fails
         */
        init(src: string): Atlas;
    } const AtlasConstructor: AtlasConstructor;
    interface AnimListConstructor {
        /**
         * Initilize a new {@link AnimList} instance
         * @param src filename of the animlist xml file
         * @returns a {@link AnimList}, otherwise, null if fails
         */
        init(src: string): AnimList;
    } const AnimList: AnimListConstructor;
    interface ConductorConstructor {
        /**
         * Initializes a new empty {@link Conductor} instance
         * @returns {@link Conductor}
         */
        init(): Conductor;
    } const Conductor: ConductorConstructor;
    interface MenuConstructor {
        /**
         * Initializes a new {@link Menu} with all items and parameters described in the {@link MenuManifest}.  
         * If the creation fails, the engine will crash. This only happens if exists text-only items without
         * a font (declared in {@link MenuManifestParameters.font} field)
         * @param menumanifest An {@link MenuManifest} object, can not be null
         * @param x X position in the screen and/or layout
         * @param y Y position in the screen and/or layout
         * @param z Z-index in the layout
         * @param width Width of the menu drawable
         * @param height Height of the menu drawable
         * @returns a new {@link Menu} instance, otherwise, ADIOS (there not error handling here)
         */
        init(menumanifest: MenuManifest, x: number, y: number, z: number, width: number, height: number): Menu;
    } const Menu: MenuConstructor;
    interface SoundPlayerConstructor {
        /**
         * Initilizes a new sound player
         * @param filename the ogg filename, the file must be OGG Vorbis
         * @returns a new {@link SoundPlayer} instance, otherwise, null if an error ocurrs
         */
        init(filename: string): SoundPlayer;
    } const SoundPlayer: SoundPlayerConstructor;


    //
    // Global week functions
    //
    /**
     * Changes the UI visibility, this includes: strums lines, healthbar, song progressbar, roundstats and song info.  
     * All UI cosmetics like the streak counter are not affected.
     * @param visible true to show the ui elements on screen, otherwise, false
     */
    function week_ui_set_visibility(visible: boolean): void;
    /**
     * Obtains the layout used to build the UI, thats is. This layout is never rendered because the engine only
     * extracts the position and parameters of all UI elements.
     * @returns the layout used to build the UI, never is null
     */
    function week_ui_get_layout(): Layout;
    /**
     * Obtains the camera used for UI bumping (also now as beat zoom)
     * @returns  the camera used for bumps, never is null
     */
    function week_ui_get_camera(): Camera;
    /**
     * Halts the engine execution, in this state no input is processed or strums are scrolled. This also can be used
     * to make a break, for example, showing a cutscene before the next song.  
     * The engine will keep calling functions like {@link f_frame} or {@link f_buttons}.
     * @param halt true to halt, otherwise, false to resume
     */
    function week_set_halt(halt: boolean): void;
    /**
     * Disables the "Results" screen, this screen is shown after completing a week. In freeplay mode also is shown
     * after completing the song.
     * @param disable true to skip the result screen, otherwise, false
     */
    function week_disable_week_end_results(disable: boolean): void;
    /**
     * Disables the girlfriend cry animation when the streak is lost.
     * @param disable true to disable, otherwise false
     */
    function week_disable_girlfriend_cry(disable: boolean): void;
    /**
     * Skip the legend "__Ready?__" used at the start of a new round
     * @param disable true to disable, otherwise false
     */
    function week_disable_ask_ready(disable: boolean): void;
    /**
     * Skips the countdown used before playing the song
     * @param disable true to disable, otherwise false
     */
    function week_disable_countdown(disable: boolean): void;
    /**
     * Disables the UI bumping, this can be useful to do custom bumping or nothing at all.
     * @param disable true to disable, otherwise false
     */
    function week_disable_camera_bumping(disable: boolean): void;
    /**
     * Get the amount of strums (not individual lines), most of the time is 2 (opponent and player)
     * @returns the amount of strums available
     */
    function week_ui_get_strums_count(): number;
    /**
     * Obtains the strums lines, normally 0 is for opponent strums and 1 for player strums.
     * @param strums_id index base-zero if the strums to adquire
     * @returns the {@link Strums} instance, otherwise, null if the index is not valid
     */
    function week_ui_get_strums(strums_id: number): Strums;
    /**
     * Obtains the {@link RoundStats} used to display the score, combo breaks, etc. This is normally shown
     * under the healthbar.
     * @returns returns the {@link RoundStats} instance, can be null if not present in the UI layout
     */
    function week_ui_get_roundstats(): RoundStats;
    /**
     * Obtains the UI cosmetics used to display the ranking of each note hit.
     * @returns the {@link RankingCounter} instance, can be null if the engine fails to load his resources
     */
    function week_ui_get_rankingcounter(): RankingCounter;
    /**
     * Obtains the UI cosmetics used to display the number of notes hit in a single row, normally after 10 hits.
     * @returns the {@link StreakCounter} instance, can be null if the engine fails to load his resources
     */
    function week_ui_get_streakcounter(): StreakCounter;
    /**
     * Obtains the {@link TextSprite} used to display the current song name and difficult, the engine updates this  
     * TextSprite at the start of each round.
     * @returns the {@link TextSprite} instance, can be null if not present in the UI layout
     */
    function week_ui_get_round_textsprite(): TextSprite;
    /**
     * Obtains the {@link SongProgressbar} used the round/song progress, Can be null if was disabled in settings or
     * if was not present in the UI layout.
     * @returns the {@link SongProgressbar} instance, otherwise, null
     */
    function week_ui_get_songprogressbar(): SongProgressbar;
    /**
     * Obtains the {@link Countdown} instance.
     * @returns the {@link Countdown} instance, never is null
     */
    function week_ui_get_countdown(): Countdown;
    /**
     * Obtains the {@link HealthBar} instance.
     * @returns the {@link HealthBar} instance, never is null
     */
    function week_ui_get_healthbar(): HealthBar;
    /**
     * Obtains the layout used as stage which is drawn as background on the screen. If the stage
     * can not be loaded or the current round does not have a stage at all the UI layout is returned
     * @returns the stage layout, can be null, if the UI layout
     */
    function week_get_stage_layout(): Layout;
    /**
     * Obtains the {@link HealthWatcher}, this is a engine component to manage the health across the characters.  
     * This component is also in charge of updating the healthbar.
     * @returns the {@link HealthWatcher} instance, never is null
     */
    function week_get_healthwatcher(): HealthWatcher;
    /**
     * Obtains the {@link MissNoteFX} instance, this engine components play a sound when the player
     * commit a penality.
     * @returns the {@link MissNoteFX} instance, never is null
     */
    function week_get_missnotefx(): MissNoteFX;
    /**
     * Changes the bpm of the current song, this updates the bpm of each beat related components like
     * charaters, strums. The chart can change this value any time.
     * @param bpm the new bpm, this value is not checked
     */
    function week_update_bpm(bpm: number): void;
    /**
     * Changes the speed of the strums scrolling and the idle speed animations of some characters like
     * girlfriend.
     * @param speed the new speed value, this value is not checked
     */
    function week_update_speed(speed: number): void;
    /**
     * Obtains the {@link MessageBox} used to display messages, thats is. This component never responds to
     * inputs presses. Is up to the lua script close the MessageBox.  
     * Note: the pause menu has is own {@link MessageBox}
     * @returns the {@link MessageBox} instance, never is null
     */
    function week_get_messagebox(): Messagebox;
    /**
     * Returns the character in the background of the stage, which is normally girlfriend. This character is 
     * always girlfriend useless the gameplay manifest disables it or changes by another one.
     * @returns the {@link Character} instance, otherwise, null if there no such character
     */
    function week_get_girlfriend(): Character;
    /**
     * Returns the number of characters who can "sing", this includes opponents, players and even actors.  
     * This normally always returns 2 but the gameplay manifest can declare a variable amount.  
     * Note: girlfriend is not counted.
     * @returns the amount of characters
     */
    function week_get_character_count(): number;
    /**
     * Returns the {@link Conductor} of the specified character.  
     * This engine component reads the strum lines and plays an proper animation like singing.
     * The characters who have chart notes contains by default a Conductor which is normally the opponent and
     * boyfriend.  
     * Characters without chart notes to sing are called "actors" and they are conductor-less
     * 
     * @param character_index index base-zero of the character 
     * @returns the {@link Conductor} instance, otherwise, null if such character is an actor
     */
    function week_get_conductor(character_index: number): Conductor;
    /**
     * Obtains a {@link Character} who can "sing", to obtain the amount of characters 
     * call {@link week_get_character_count}.
     * Normally the index 0 is the oponent and 1 is boyfriend
     * @param character_index the index base-zero of the character to return
     * @returns the {@link Character} instance, otherwise, null if the index is not valid
     */
    function week_get_character(character_index: number): Character;
    /**
     * Returns a {@link PlayerStats} instance of the specified character, this engine component songs all
     * player/opponent note hit, misses, penalities, statistics and health.  
     * Can return null if such character is an "actor".
     * @param character_index index base-zero of the character
     * @returns the {@link PlayerStats} instance, otherwise, null
     */
    function week_get_playerstats(character_index: number): PlayerStats;
    /**
     * Returns a {@link SongPlayer} instance, this engine component manages and plays voice and instrumental tracks.
     * Can be null is song tracks can not be loaded or the gameplay manifest does not define a song
     * @returns the {@link SongPlayer}, otherwise, null
     */
    function week_get_songplayer(): SongPlayer;
    /**
     * Obtains the original and initial BPM and speed of the chart. This does not include the overridden speed
     * and/or BPM described in the current chart section.
     * @returns the BPM and the speed
     */
    function week_get_current_chart_info(): LuaMultiReturn<[number, number]>;
    /**
     * Obtains the current song name (display name, not filename), difficult and base-zero index in the 
     * gameplay manifest.
     * @returns song name, difficult and index
     */
    function week_get_current_song_info(): LuaMultiReturn<[string, string, number]>;
    /**
     * Changes the camera used (embed in stage layout) to focus a character, this focus happens before
     * any character starts singing.  
     * The engine uses the standard Funkin chart format which only have room for two characters, for
     * now, the engine picks from the stage layout the cameras named "camera_opponent" and "camera_player".
     * @param opponent_or_player target character, true for the opponent, otherwise false for the player
     * @param new_name the new camera name or null to restore default name
     */
    function week_change_character_camera_name(opponent_or_player: boolean, new_name: string): void;
    /**
     * By default after restarting a song or changing to another one, the engine executes all initial
     * actions, cameras and triggers in the stage layout. This leaves the layout as-is was loaded for first
     * time.  
     * If the layout was no designed with this in mind or to avoid loosing any change made you can disable the rollback
     * @param disable true to disable, otherwise, false
     */
    function week_disable_layout_rollback(disable: boolean): void;
    /**
     * Overrides the contents of "/assets/common" this is useful to build UI elements with custom sprites for example.  
     * In custom folder place the files you want to override using the same filenames (names are case-sensitive).  
     * Note: Does not reload any engine component using a non-custom resource useless it was necessary
     * @param custom_common_path folder path to use as common folder or null to restore
     */
    function week_override_common_folder(custom_common_path: string): void;
    /**
     * Shows a screen once the week is completed, this screen contains the original Funkin credits like
     * special thanks or original developers.
     */
    function week_enable_credits_on_completed(): void;
    /**
     * Force the current round or week to be ended.
     * - If the round is marked as clear the engine loads the next song, otherwise, shows the gameover screen.
     * - If the week is marked as completed the engine shows the results. Anyways, the engine returns to the
     * previous screen (normally is the week selector or freeplay menu).
     * @param round_or_week true to end the current round, otherwise the whole week
     * @param loose_or_win true to mark the current song or week as failed, otherwise, false
     */
    function week_end(round_or_week: boolean, loose_or_win: boolean): void;
    /**
     * Returns a {@link Dialogue} instance, this engine component manages and display all dialogs. While is shown
     * this component process any input gamepad which is required to close o display character dialog lines.  
     * Can return null if the gameplay manifest does not declare a dialogue params xml file.
     * @returns the {@link Dialogue} instance, otherwise, null
     */
    function week_get_dialogue(): Dialogue;
    /**
     * Set a shader to be used in the UI, this will impact all non-cosmetics ui elements.  
     * Note: due how the engine is designed, the UI is attached to the stage layout. This means that a shader applied 
     * to the root of the layout will affect the UI.
     * @param psshader the shader to use, otherwise, null
     */
    function week_set_ui_shader(psshader: PSShader): void;
    /**
     * Forces the UI to be rebuilded, this destroy all UI components and then recreates again. This is useful to
     * override common files (see {@link week_override_common_folder}) during the gameplay.     
     * Notes:
     * - this is a time-expensive operation, you should do this before the round starts.
     * - under normal circumstances this is never required.
     * - this feature is provided "as-is", can restore cleared notes and trigger an instant gameover.
     */
    function week_rebuild_ui(): void;
    /**
     * Create an unlock directive, you can control when this directive saved or forgotten.  
     * All directives are saved in the funkinsave.
     * @param name name of the directive to create, can not be null
     * @param completed_round true to save this directive only if the round is completed, otherwise, false
     * @param completed_week true to save this directive only if the week is completed, otherwise, false
     * @param value value of the directive, if unsure, use 0
     */
    function week_unlockdirective_create(name: string, completed_round: boolean, completed_week: boolean, value: number): void;
    /**
     * Reads an unlock directive from the funkinsave
     * @param name name of the directive to create, can not be null
     * @returns a value saved along with the directive, otherwise, null if does not exists
     */
    function week_unlockdirective_get(name: string): number | null;
    /**
     * Deletes an unlock directive from the funkinsave, you can control when this directive deleted or not.
     * @param name name of the directive to delete, can not be null
     * @param completed_round true to delete this directive only if the round is completed, otherwise, false
     * @param completed_week true to delete this directive only if the week is completed, otherwise, false
     */
    function week_unlockdirective_remove(name: string, completed_round: boolean, completed_week: boolean): void;
    /**
     * Reads a chunk of binary data from funkinsave, the returned blob is always an string because
     * lua does not have support for buffer or binary arrays
     * @param blob_name The name of the blob, can not be null
     * @returns the binary data, otherwise, null
     */
    function week_storage_get_blob(blob_name: string): string | null;
    /**
     * Writes a chunk of binary data in the funkinsave, the provided string is saved as an array bytes
     * @param blob_name The name of the blob, can not be null
     * @param blob A "binary string" representing the data
     * @returns true if was saved, otherwise, false if an error occurs
     */
    function week_storage_set_blob(blob_name: string, blob: string): boolean;
    /**
     * Removes the background music used in the gameover screen
     */
    function week_gameover_no_music(): void;
    /**
     * Removes the __die__ sound effect used in the gameover screen
     */
    function week_gameover_no_sfx_die(): void;
    /**
     * Removes the __retry__ sound effect used in the gameover screen
     */
    function week_gameover_no_sfx_retry(): void;
    /**
     * Set the duration for the __die__ animation used in the gameover screen.  
     * The die animation is played using {@link Character.play_extra} function. By default the engine waits 3200ms
     * @param duration_ms duration in milliseconds, NaN to use default duration or -1 to wait for the animation end
     */
    function week_gameover_set_die_anim_duration(duration_ms: number): void;
    /**
     * Set the duration for the __retry__  animation used in the gameover screen.  
     * The die animation is played using {@link Character.play_extra} function. By default the engine waits 3600ms
     * @param duration_ms duration in milliseconds, NaN to use default duration or -1 to wait for the animation end
     */
    function week_gameover_set_retry_anim_duration(duration_ms: number): void;
    /**
     * Set the duration for the __giveup__ animation used in the gameover screen.  
     * The die animation is played using {@link Character.play_extra} function. By default the engine waits 0ms
     * @param duration_ms duration in milliseconds, NaN to use default duration or -1 to wait for the animation end
     */
    function week_gameover_set_giveup_anim_duration(duration_ms: number): void;
    /**
     * Sets the millisecods before __giveup__ or __retry__ ends to fire the screen transition (fade to black).  
     * If the engine is waiting for __giveup__ or __retry__ animation to completed, this value is ignored.
     * @param duration_ms duration in milliseconds, NaN to use default duration
     */
    function week_gameover_set_before_anim_duration(duration_ms: number): void;
    /**
     * Same as {@link week_gameover_set_before_anim_duration}, but this value used instead if the player
     * skips the gameover screen (after pressing START button twice).  
     * If the engine is waiting for __giveup__ or __retry__ animation to completed, this value is ignored.
     * @param duration_ms duration in milliseconds, NaN to use default duration
     */
    function week_gameover_set_before_force_end_anim_duration(duration_ms: number): void;
    /**
     * Sets the music for the gameover screen
     * @param filename the sound filename or null to use/retore the default sound
     */
    function week_gameover_set_music(filename: string): void;
    /**
     * Sets the __die__ sound effect for the gameover screen
     * @param filename the sound filename or null to use/retore the default sound
     */
    function week_gameover_set_sfx_die(filename: string): void;
    /**
     * Sets the __retry__ sound effect for the gameover screen
     * @param filename the sound filename or null to use/retore the default sound
     */
    function week_gameover_set_sfx_confirm(filename: string): void;
    /**
    * Gets a copy of the accumulated stats of the first character (excludes opponents). When
    * a round/song ends the stats of the first playable character is accumulated
    */
    function week_get_accumulated_stats(): WeekResult_Stats;
    /**
    * Gets the gameover screen layout, can be null if not loaded.
    */
    function week_get_gameover_layout(): Layout;
    /**
    * Gets the pause screen layout, can be null if not loaded.
    */
    function week_get_pause_layout(): Layout;
    /**
    * Gets the results screen layout, this layout only is available after ending the week or freeplay song.
    * Always returns null during the gameplay, call inside of {@link f_beforeresults} or {@link f_afterresults} to adquire the layout.
    */
    function week_get_results_layout(): Layout;
    /**
     * Sets the background music for the pause screen
     * @param filename the sound filename or null to disable
     */
    function week_set_pause_background_music(filename: string): void;


    //
    // Global Timer functions
    //

    /** A type describing a callback used by timers functions */
    type TimerCallback = (...args: any[]) => any;
    /**
     * Resturns the platform high-precission time in milliseconds.
     * @returns the time is since the engine is started
     */
    function timer_ms_gettime(): number;
    /**
     * Schedule a function to be called in intervals, this function as the same beheavoir like JS "setInterval()"
     * @param delay each interval delay before run the callback in milliseconds
     * @param fn callback function
     * @param args arguments passed to the callback function
     * @returns an unique id which can be used to cancel the interval
     */
    function timer_callback_interval(delay: number, fn: TimerCallback, ...args: any[]): number;
    /**
     * Schedule a function to be called after the delay, this function as the same beheavoir like JS "setTimeout()"
     * @param delay the delay before run the callback in milliseconds
     * @param fn callback function
     * @param args arguments passed to the callback function
     * @returns an unique id which can be used to cancel the timeout
     */
    function timer_callback_timeout(delay: number, fn: TimerCallback, ...args: any[]): number;
    /**
     * Same as {@link timer_callback_interval} but uses the song timestamp instead, this avoids running the callback
     * if the gameplay or the song paused
     * @param delay each interval delay before run the callback in milliseconds
     * @param fn callback function
     * @param args arguments passed to the callback function
     * @returns an unique id which can be used to cancel the interval
     */
    function timer_songcallback_interval(delay: number, fn: TimerCallback, ...args: any[]): number;
    /**
     * Same as {@link timer_callback_timeout} but uses the song timestamp instead, this avoids running the callback
     * if the gameplay or the song paused
     * @param delay the delay before run the callback in milliseconds
     * @param fn callback function
     * @param args arguments passed to the callback function
     * @returns an unique id which can be used to cancel the interval
     */
    function timer_songcallback_timeout(delay: number, fn: TimerCallback, ...args: any[]): number;
    /**
     * Cancels a interval or timeout scheduled
     * @param the id of the callback to cancel (returned by any timer_*callback_*() functions)
     * @returns trues if the callbak exists and was canceled, otherwhise, false
     */
    function timer_callback_cancel(callback_id: number): boolean;
    /**
     * Cancels all intervals and timeouts scheduled
     * @returns the number of canceled callbacks
     */
    function timer_callback_cancel_all(): number;


    //
    // Global Math2D functions
    //

    /**
     * Generates a random number in a specified range, both inclusive.  
     * Arguments can be floating point numbers and/or integers.  
     * Note: This function uses the engine's preferred P-RNG. It is recommended as a replacement for the 
     * lua P-RNG because it produces less uniform values.
     * @param min minimum number to generate
     * @param max maximum number to generate
     * @returns A floating point number
     */
    function math2d_random(min: number, max: number): number;
    /**
     * Generates a floating-point, pseudo-random number between 0 (inclusive) and 1 (exclusive).  
     * Note: This function uses the engine's preferred P-RNG. It is recommended as a replacement for the 
     * lua P-RNG because it produces less uniform values.
     * @returns A floating point number between 0.0 and 1.0
     */
    function math2d_random(): number;
    /**
     * Chooses a random number between 0 and 100 (inclusive), if this value is less than chance returns true
     * @param chance a 0%-100% percent chance to return true
     * @returns true or false
     */
    function math2d_random_boolean(chance: number): boolean;
    /**
     * Interpolates two values
     * @param start start value
     * @param end end value
     * @param step a percent between 0.0 and 1.0
     * @returns the interpolated value
     */
    function math2d_lerp(start: number, end: number, step: number): number;
    /**
     * Does an inverse interpolation
     * @param start start range
     * @param end end range
     * @param value the point within the range
     * @returns a percent between 0.0 and 1.0
     */
    function math2d_inverselerp(start: number, end: number, value: number): number;
    /**
     * Rounds down the number to the lowest multiply value
     * @param value value
     * @param step multiply used to step
     * @return the rounded down value, otherwise, the original value
     */
    function math2d_nearestdown(value: number, step: number): number;
    /**
     * Rotates a 2D point
     * @param radians the angle expressed in radians
     * @param x X coordinates
     * @param y Y coordinates
     * @returns the new point
     */
    function math2d_rotate_point_by_degs(radians: number, x: number, y: number): LuaMultiReturn<[number, number]>;
    /**
     * Calculates the distance between two points
     * @param x1 the first point x coordinate
     * @param y1 the first point y coordinate
     * @param x2 the second point x coordinate
     * @param y2 the second point y coordinate
     * @returns the calculated distance
     */
    function math2d_points_distance(x1: number, y1: number, x2: number, y2: number): number;

    //
    // Filesystem global functions
    //
    function fs_readfile(src: string): string;
    function fs_get_full_path(path: string): string;
    
    
    //
    // PreloadCache global functions
    //
    /**
     * Removes all entries in the preload cache releasing used memory. Cache entries in use
     * will be freed once they are no longer referenced
     */
    function preloadcache_clear(): voice;
    /**
     * Removes entries in the preload cache until the desired amount of memory is released.
     * Note: this function normally releases slightly more than the desired amount
     * @param bytes_amount The amount of bytes to be released, must be a positive integer
     * @returns true if the desired amount was released or the cache was empty, otherwise, false
     */
    function preloadcache_flush(bytes_amount: number): boolean;
    

    //
    // Modding UI (engine screens/menus only)
    //

    /** Represents a simple type which can be serialized and passed to antoher lua script */
    type BasicValue = string | number | boolean | null;
    /** 
     * Adquires the current UI layout
     * @returns the layout object
     */
    function modding_get_layout(): Layout;
    /** Exits from the current screen, in most of the cases also dispose the calling lua script */
    function modding_exit(): void;
    /** 
     * Halts the engine execution, this ignores any gamepad, keyboard or mouse input
     * @param halt true to halt, otherwise, false to resume
     */
    function modding_set_halt(halt: boolean): void;
    /**
     * Creates a new directive in the funkinsave, which can be used to unlock weeks, character, songs, etc.
     * @param name Name of the directive, can not be null
     * @param value value of the directive, if unsure, use 0
     */
    function modding_unlockdirective_create(name: string, value: number): void;
    /**
     * Deletes an unlock directive from the funkinsave
     * @param name Name of the directive, can not be null
     */
    function modding_unlockdirective_remove(name: string): void;
    /**
     * Reads an unlock directive from funkinsave
     * @param name Name of the directive, can not be null
     * @returns null if the directive does not exists, otherwise, the directive value
     */
    function modding_unlockdirective_get(name: string): number | null;
    /**
     * Reads a chunk of binary data from funkinsave, the returned blob is always an string because
     * lua does not have support for buffer or binary arrays
     * @param week_name The week name where to read the binary data, otherwise, null
     * @param blob_name The name of the blob, can not be null
     * @returns the binary data, otherwise, null
     */
    function modding_storage_get_blob(week_name: string, blob_name: string): string | null;
    /**
     * Writes a chunk of binary data in the funkinsave, the provided string is saved as an array bytes
     * @param week_name The week name where write read the binary data, otherwise, null
     * @param blob_name The name of the blob, can not be null
     * @param blob A "binary string" representing the data
     * @returns true if was saved, otherwise, false if an error occurs
     */
    function modding_storage_set_blob(week_name: string, blob_name: string, blob: string): boolean;
    /**
     * Gets the active menu
     * @returns the active menu or null 
     */
    function modding_get_active_menu(): Menu;
    /**
     * Chooses an option, each engine screen menus has is own options, if the option is not recognized
     * the engine later calls {@link f_modding_handle_custom_option} function (if exists)
     * @param name name of the native option or custom option
     * @returns true if the option was accepted, otherwise, false
     */
    function modding_choose_native_menu_option(name: string): boolean;
    /**
     * Gets the native menu of the current engine screen
     * @returns the menu, otherwise, null if does not contains any menu
     */
    function modding_get_native_menu(): Menu;
    /**
     * Changes the active menu, once changes, all inputs will be applied to this menu
     * @param menu the menu instance
     */
    function modding_set_active_menu(menu: Menu): void;
    /**
     * Gets the current background menu music using in all engine menus/screens. Does not apply to
     * freeplay menu
     * @returns the music player, otherwise, null if there no background music
     */
    function modding_get_native_background_music(): SoundPlayer;
    /**
     * Replaces the background music used in all engine menus/screens, the old music player will be disposed.  
     * Note: the weekselector disposes the background music before the gameplay reloads the "freakyMenu.ogg"
     * once the gameplay ends
     * @param music_src the filename of the music to replace, should be an absolute path. Can be null
     * @returns the new music player, otherwise, null if there no background music
     */
    function modding_replace_native_background_music(music_src: string): SoundPlayer;
    /**
     * Creates a new screen menu and waits until returns
     * @param layout_src the filename of the layout to use, should be an absolute path. Can not be null
     * @param script_src the filename of the lua script to use, should be an absolute path. Can be null
     * @param arg an argument to pass, this calls {@link f_modding_exit} in the script with that value
     * @returns a value returned of {@link f_modding_back} 
     */
    function modding_spawn_screen(layout_src: string, script_src: string, arg: BasicValue): BasicValue;
    /**
     * Delay used before dispose the current screen menu, this is useful for transitions
     * @param delay_ms the delay in milliseconds
     */
    function modding_set_exit_delay(delay_ms: number): void;
    /**
     * Gets an messagebox instance, if not present, the engine instances a new one (this can take some time)
     * @returns the MessageBox instance
     */
    function modding_get_messagebox(): Messagebox;
    /**
     * Enumrates all weeks loaded from "/assets/weeks", this can include any extra weeks of loaded expansions
     * @returns a non-null array with all loaded weeks
     */
    function modding_get_loaded_weeks(): WeekInfo[];
    /**
     * Launch a gameplay in the specified week
     * @param week_name the name of the week (folder name under "/assets/weeks")
     * @param difficult the name of the difficult, for example "NORMAL"
     * @param use_alt_tracks true if alternative songs should be used, otherwise, false
     * @param boyfriend_character_manifest_src the boyfriend character manifest filename, otherwise, null to use the default
     * @param girlfriend_character_manifest_src the girlfried character manifest filename, otherwise, null to use the default
     * @param gameplay_manifest_src the gameplay manifest json filename
     * @param freeplay_song_index the song index in the gameplay manifest, otherwise, use -1 to play the whole week
     * @param exit_to_weekselector_label text to replace the "EXIT TO WEEK SELECTOR" label in the pause menu
     */
    function modding_launch_week(week_name: string, difficult: string, use_alt_tracks: boolean, boyfriend_character_manifest_src: string, girlfriend_character_manifest_src: string, gameplay_manifest_src: string, freeplay_song_index: number, exit_to_weekselector_label: string): number;
    /** Launch the credits screen */
    function modding_launch_credits(): void;
    /** Launch the start screen, this also rewinds the background music */
    function modding_launch_startscreen(): boolean;
    /** Launch the main menu screen */
    function modding_launch_mainmenu(): boolean;
    /** (Desktop only) Launch the settings screen (options), due how is coded, this screen can not be called twice */
    function modding_launch_settings(): void;
    /** Launch the freeplay menu screen */
    function modding_launch_freeplay(): void;
    /** Launch the week selector screen */
    function modding_launch_weekselector(): number;


    //
    // MenuManifest helpers (modding context only)
    //

    /** Describes the parameters used to build a {@link Menu} */
    type MenuManifestParameters = {
        /** Font color in RGBA8 format, used in text-only items */
        fontColor: number;
        /** Atlas filename required for textured items */
        atlas: string;
        /** AnimationList filename used for discarded, idle, rollback, selected, choosen, in and out animations */
        animlist: string;
        /** Font filename used for text-only items */
        font: string;
        /** Size of the font in pixels, used in text-only items */
        fontSize: number;
        /** In glyphs fonts (like alphabet.xml) indicates the suffix of each atlas entry (for example "bold") */
        fontGlyphSuffix: string;
        /** In glyphs fonts this inverts the colors, required for alphabet.xml font which has black glyphs  */
        fontColorByDifference: boolean;
        /** Font boder color in RGBA8 format, used in text-only items */
        fontBorderColor: number;
        /** Animation to play when the item is not choosed and discarded, loaded from the animlist */
        animDiscarded: string;
        /** Animation to play while is not selected, loaded from the animlist */
        animIdle: string;
        /** This special animation  to undo and/or restore changed properties, loaded from the animlist */
        animRollback: string;
        /** Animation to play when the item is selected, loaded from the animlist */
        animSelected: string;
        /** Animation to play when the item is choosen (pressing any confirm button), loaded from the animlist */
        animChoosen: string;
        /** Animation to play during the menu in transition, loaded from the animlist */
        animIn: string;
        /** Animation to play during the menu out transition, loaded from the animlist*/
        animOut: string;
        /** Delay before play animIn of each item, this delay is multiplied by each item index.  
         * If the value is negative, the delay is applied in reversed order.
         */
        animTransitionInDelay: number;
        /** Delay before play animOut of each item, this delay is multiplied by each item index.  
         * If the value is negative, the delay is applied in reversed order.
         */
        animTransitionOutDelay: number;
        /** Indicates if the menu should be vertical, defaults to true*/
        isVertical: boolean;
        /** Items alignment, defaults to {@link Align.CENTER} */
        itemsAlign: Align;
        /** Gaps between items in pixels */
        itemsGap: number;
        /** Indicates if the items are sparsed, if true, the menu is not rendered as list and each item
         * placements describes the relative position in the menu.
          */
        isSparse: boolean;
        /** Indicates the amount of items visible at the borders before scroll, defaults to 0 */
        staticIndex: number;
        /** Indicates if the items list are rendered as pages, once the last item on the list is
         * scrolled the page is scrolled */
        isPerPage: boolean;
        /** The width/height of the menus item, depends whatever is vertical or horizontal. Textures items only */
        itemsDimmen: number;
        /** Font border size, used in text-only items */
        fontBorderSize: number;
        /** If this number is positive, ignores the ItemsDimmen and scales each textured item instead */
        textureScale: number;
        /** If the orientation is horizontal, takes account of the text width and avoids overlapping */
        enableHorizontalTextCorrection: boolean;
        /** Suffix used with each item name as animation selected name */
        suffixSelected: string;
        /** Suffix used with each item name as animation choosen name */
        suffixChoosen: string;
        /** Suffix used with each item name as animation discarded name */
        suffixDiscarded: string;
        /** Suffix used with each item name as animation idle name */
        suffixIdle: string;
        /** Suffix used with each item name as animation rollback name (untested) */
        suffixRollback: string;
        /** Suffix used with each item name as animation "in" name (menu transition) */
        suffixIn: string;
        /** Suffix used with each item name as animation "out" name (menu transition) */
        suffixOut: string;
    }
    /** Placement of an menu item */
    type MenuManifestPlacement = {
        /** Overrides the itemsGap declared in parameters if the number is positive */
        gap: number;
        /** X offset or relative position in pixels */
        x: number;
        /** Y offset or relative position in pixels */
        y: number;
        /** Overrides the itemsDimmen declared in parameters if the number is positive */
        dimmen: number;
    }
    /** Describes a single menu item */
    type MenuManifestItem = {
        /** Item placement,
         * if the menu is sparse this describes the relative position inside the menu, otherwise, offsets.
         */
        placement: MenuManifestPlacement;
        /** Overrides the fontColor declared in parameters, value RGBA8 format */
        fontColor: number;
        /** Item text, used if untextured */
        text: string;
        /** Model filename, this overrides the atlas specified in parameters if not null */
        model: string;
        /** For textured items, indicates the scaled used or NaN if ignored */
        textureScale: number;
        /** Name of the item which is also used to adquire from the model or atlas */
        name: string;
        /** Overrides the name of the selected animation */
        animSelected: string;
        /** Overrides the name of the choosen animatiom */
        animChoosen: string;
        /** Overrides the name of the discarded animatiom */
        animDiscarded: string;
        /** Overrides the name of the idle animatiom */
        animIdle: string;
        /** Overrides the name of the rollback animatiom */
        animRollback: string;
        /** Overrides the name of the in animatiom */
        animIn: string;
        /** Overrides the name of the out animatiom */
        animOut: string;
        /** Indicates if this item is hidden */
        hidden: boolean;
    }
    /** Describes (in lua side) a manifest used to build an {@link Menu} */
    type MenuManifest = {
        /** Parameters required to build a menu */
        parameters: MenuManifestParameters;
        /** Items of the menu */
        items: MenuManifestItem[];
    }
    /**
     * Load and parse an menu manifest json file
     * @param src filename of the json file
     * @returns A menu manifest object
     */
    function menumanifest_parse_from_file(src: string): MenuManifest;

    /** Describes the relevat information about a week's song */
    type WeekInfoSong = {
        /** Name of song */
        readonly name: string;
        /** Indicates if the song is only available in the freeplay menu */
        readonly freeplay_only: boolean;
        /** Directive required to unlock this song */
        readonly freeplay_unlock_directive: string;
        /** Song filename, this path is absolute */
        readonly freeplay_song_filename: string;
        /** A brief description of the song */
        readonly freeplay_description: string;
    };
    /** Describes the relevant information about a week */
    type WeekInfo = {
        /** Folder name inside "/assets/weeks/" */
        name: string;
        /** Friendly name to display */
        display_name: string;
        /** A brief description about the week */
        description: string;
        /** Folder path used to override "/assets/common/" contents */
        custom_folder: string;
        /** Songs declared in the "about.json" file */
        songs: WeekInfoSong[];
        /** Directive required to unlock this week, can be null if not present */
        unlock_directive: string;
        /** Name of the directive to create once the week is completed */
        emit_directive: string;
    }

    //
    // Engine objects
    //

    /** Engine settings, includes options like inverse scrolling  */
    const Settings: EngineSettings;
    const Environment: Environment;

    /**
     * Loads and evaluates a lua script file
     * @param lua_script_filename script filename
     * @returns true if was succesful loaded, parsed and evaluated, otherwhise, false
     */
    function importScript(lua_script_filename: string): boolean;
    /**
     * Loads and evaluates a lua script file, its like {@link importScript} but returns the result
     * of the evaluated script.
     * @param lua_script_filename script filename
     * @returns {boolean} false if fails to load, parse and evaluate the script
     * @returns {any} any result returned from the evaluated script
     */
    function evalScript(lua_script_filename: string): any;


    //
    // Enums used in "f_input_*" callbacks implemented using GLFW
    //

    /**
    * Mouse buttons
    */
    const enum MouseButton {
        BUTTON1 = 0,
        BUTTON2 = 1,
        BUTTON3 = 2,
        BUTTON4 = 3,
        BUTTON5 = 4,
        BUTTON6 = 5,
        BUTTON7 = 6,
        BUTTON8 = 7,
        ButtonLeft = BUTTON1,
        ButtonRight = BUTTON2,
        ButtonMiddle = BUTTON3
    }
    /**
     * Keyboard modifiers keys
     */
    const enum ModKeys {
        None = 0x0000,
        Shift = 0x0001,
        Control = 0x0002,
        Alt = 0x0004,
        Super = 0x0008,
        CapsLock = 0x0010,
        NumLock = 0x0020
    }
    /**
     * Keyboard keys
     */
    const enum Keys {
        Unknown = -1,/* key not recognized, use scancode instead */
        Space = 32,
        Apostrophe = 39,/* ' */
        Comma = 44,/* , */
        Minus = 45,/* - */
        Period = 46,/* . */
        Slash = 47,/* / */
        Numpad0 = 48,
        Numpad1 = 49,
        Numpad2 = 50,
        Numpad3 = 51,
        Numpad4 = 52,
        Numpad5 = 53,
        Numpad6 = 54,
        Numpad7 = 55,
        Numpad8 = 56,
        Numpad9 = 57,
        Semicolon = 59,/* ; */
        Equal = 61,/* = */
        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,
        LeftBracket = 91,/* [ */
        Backslash = 92,/* \ */
        RightBracket = 93,/* ] */
        GraveAccent = 96,/* ` */
        World_1 = 161,/* non-US #1 */
        World_2 = 162,/* non-US #2 */
        Escape = 256,
        Enter = 257,
        Tab = 258,
        Backspace = 259,
        Insert = 260,
        Delete = 261,
        Right = 262,
        Left = 263,
        Down = 264,
        Up = 265,
        PageUp = 266,
        PageDown = 267,
        Home = 268,
        End = 269,
        CapsLock = 280,
        ScrollLock = 281,
        NumLock = 282,
        PrintScreen = 283,
        Pause = 284,
        F1 = 290,
        F2 = 291,
        F3 = 292,
        F4 = 293,
        F5 = 294,
        F6 = 295,
        F7 = 296,
        F8 = 297,
        F9 = 298,
        F10 = 299,
        F11 = 300,
        F12 = 301,
        F13 = 302,
        F14 = 303,
        F15 = 304,
        F16 = 305,
        F17 = 306,
        F18 = 307,
        F19 = 308,
        F20 = 309,
        F21 = 310,
        F22 = 311,
        F23 = 312,
        F24 = 313,
        F25 = 314,
        KP_0 = 320,
        KP_1 = 321,
        KP_2 = 322,
        KP_3 = 323,
        KP_4 = 324,
        KP_5 = 325,
        KP_6 = 326,
        KP_7 = 327,
        KP_8 = 328,
        KP_9 = 329,
        KP_Decimal = 330,
        KP_Divide = 331,
        KP_Multiply = 332,
        KP_Subtract = 333,
        KP_Add = 334,
        KP_Enter = 335,
        KP_Equal = 336,
        LeftShift = 340,
        LeftControl = 341,
        LeftAlt = 342,
        LeftSuper = 343,
        RightShift = 344,
        RightControl = 345,
        RightAlt = 346,
        RightSuper = 347,
        Menu = 348
    }


}
