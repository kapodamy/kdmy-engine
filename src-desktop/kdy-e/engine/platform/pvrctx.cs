using System;
using System.Diagnostics;
using System.IO;
using CsharpWrapper;
using Engine.Externals;
using Engine.Externals.GLFW;
using Engine.Externals.LuaScriptInterop;
using Engine.Externals.SoundBridge;
using Engine.Font;
using Engine.Image;
using Engine.Utils;
using KallistiOS.MAPLE;
using KallistiOS.VBLANK;

namespace Engine.Platform;

public enum PVRFlag : int {
    /// <summary>
    /// private, do not use
    /// </summary>
    INVALID_VALUE = -1,
    DISABLE = 0,
    ENABLE = 1,
    DEFAULT = 2
}

public enum PVRContextVertex {
    NONE,
    SPRITE,
    TEXTSPRITE,
    STATESPRITE,
    DRAWABLE,
    SHAPE// not implemented
}

/**
 * Sega Dreamcast PowerVR graphics processor backend
 */
public class PVRContext {

    public static readonly float[] CLEAR_COLOR = { 0.5f, 0.5f, 0.5f, 1.0f };

    public static readonly float[] DEFAULT_OFFSET_COLOR = { 0.0f, 0.0f, 0.0f, -1.0f };

    private static readonly float[] FPS_BG_COLOR = { 0.0f, 0.0f, 0.0f, 0.5f };// rgba

    public const int STACK_LENGTH = 128;
    public const float MIN_FRAME_TIME = 1000f / 15f;
    private const double WIDESCREEN_ASPECT_RATIO = 16.0 / 9.0;
    private const double DREAMCAST_ASPECT_RATIO = 4.0 / 3.0;
    private const float FPS_FONT_SIZE_RATIO = 40.0f / 720.0f;// 40px @ 1280x720
    private const double AUTOHIDE_CURSOR_SECONDS = 3.0;
    private const int SHADER_STACK_LENGTH = 64;

    public static PVRContext global_context;

    private readonly IconifyCallback iconifyCallback;
    private readonly SizeCallback sizeCallback;
    private readonly WindowCallback closeCallback;

    /**
     * The PVR system allocates enough space for two frames: one in
       data collection mode, and another in rendering mode. If a frame is 
       currently rendering, and another frame has already been closed, then the
       caller cannot do anything else until the rendering frame completes. Note
       also that the new frame cannot be activated except during a vertical
       blanking period, so this essentially waits until a rendered frame is
       complete __and__ a vertical blank happens.
     * @summary Block the caller until the PVR system is ready for another frame to be
       submitted
     * @returns {float}	On success. Returns the amount of milliseconds
       since the last frame (last call to {@link pvr_wait_ready}), also the PVR system is ready for another
       frame's data to be collected.
       On error. Returns -1, something is probably very wrong...
     * @async
     */
    public float WaitReady() {
        if (this.is_closed) {
            Logger.Log("The main window was closed, forcing exit...");
            Glfw.Terminate();
            Environment.Exit(0);
            return Single.NaN;
        }

        if (this.webopengl.has_texture_uploads) {
            // wait until all textures are completely uploaded
            this.webopengl.has_texture_uploads = false;
            this.webopengl.gl.flush();
        }

        // flush front framebuffer if there anything drawn
        this.FlushFramebuffer();
        this.shader_framebuffer_front.Invalidate();
        this.shader_framebuffer_back.Invalidate();

        if (EngineSettings.show_fps) DrawFPS();// if enabled draw it
        Engine.Game.MasterVolume.Draw(this);

        // swap the buffers if something was drawn (avoid screen flickering)
        if (Engine.Externals.WebGL2RenderingContext.KDY_draw_calls_count > 0) {
            Glfw.MakeContextCurrent(this.nativeWindow);
            Glfw.SwapBuffers(this.nativeWindow);

            Engine.Externals.WebGL2RenderingContext.KDY_draw_calls_count = 0;
        }

        if (this.is_deterministic)
            System.Threading.Thread.Sleep(1);
        /*else if (!this.vsync_enabled)
            System.Threading.Thread.Sleep(0);*/

        Glfw.PollEvents();

        double now = Glfw.GetTime();
        float elapsed;
        if (last_timestamp <= 0)
            elapsed = 0.0f;
        else
            elapsed = (float)((now - last_timestamp) * 1000.0);

        last_timestamp = now;

        //
        // Notes:
        //          * the engine should not run below 15fps, or the beat synchronization will be lost.
        //          * set elapsed to zero is bigger than 200ms, this happen between load screens,
        //          * do not fire threshold twice, check if last elapsed value was zero.
        //
        if (elapsed >= 200.0f && this.last_elapsed != 0.0f)
            elapsed = 0.0f;
        else if (elapsed > MIN_FRAME_TIME)
            elapsed = MIN_FRAME_TIME;

        this.last_elapsed = elapsed;
        this.frame_rendered++;

        // run all registered vblank handlers
        vblank.handlers_poll();

        // notify all lua scripts about keyboard/mouse/window changes
        Engine.Externals.LuaScriptInterop.LuascriptPlatform.PollWindowState();

        // resize framebuffers if the screen size has changed
        this.CheckFrameBufferSize();

        // check if necessary hide the cursor
        if (now > this.cursor_hide_timestamp) {
            Glfw.SetInputMode(this.nativeWindow, Glfw.CURSOR, Glfw.CURSOR_HIDDEN);
            this.cursor_hide_timestamp = Double.PositiveInfinity;
            this.cursor_is_hidden = true;
        }

        return elapsed;
    }

    public static void HelperClearOffsetColor(float[] offsetcolor) {
        offsetcolor[0] = 0.0f;
        offsetcolor[1] = 0.0f;
        offsetcolor[2] = 0.0f;
        offsetcolor[3] = -1.0f;
    }

    public static void UnHideCursor() {
        PVRContext pvr_context = PVRContext.global_context;

        if (pvr_context.cursor_is_hidden) {
            Glfw.SetInputMode(pvr_context.nativeWindow, Glfw.CURSOR, Glfw.CURSOR_NORMAL);
        }

        if (EngineSettings.autohide_cursor)
            pvr_context.cursor_hide_timestamp = Glfw.GetTime() + PVRContext.AUTOHIDE_CURSOR_SECONDS;
        else
            pvr_context.cursor_hide_timestamp = Double.PositiveInfinity;

        pvr_context.cursor_is_hidden = false;
    }

    public bool IsWidescreen() {
        return this.force_widescreen;
    }



    private class PVRContextState {

        public readonly SIMDMatrix matrix = new SIMDMatrix();
        public float global_alpha = 1.0f;
        public readonly float[] offsetcolor = new float[] { 0.0f, 0.0f, 0.0f, 0.0f };
        public PVRFlag global_antialiasing = PVRFlag.DEFAULT;
        public PVRFlag global_offsetcolor_multiply = PVRFlag.DEFAULT;
        public int added_shaders = 0;

        public PVRContextState() {
            PVRContext.HelperClearOffsetColor(this.offsetcolor);
        }
    }


    private int PVR_WIDTH, PVR_HEIGHT;
    private int last_windowed_x, last_windowed_y;
    private int last_windowed_width, last_windowed_height;
    private bool force_widescreen;
    public int resolution_changes = 0;
    public WebGLContext webopengl = null;
    private PVRContextState[] stack = new PVRContextState[STACK_LENGTH];

    private double last_timestamp;
    private Window nativeWindow;
    private bool is_minimized;
    private bool is_closed;
    private bool is_deterministic;
    private bool vsync_enabled;
    private double fps_next_time;
    private double fps_count;
    private double fps_resolution_changes;
    private TextSprite fps_text;
    internal float last_elapsed = 0;
    internal int frame_rendered = 0;
    internal readonly string native_window_title;

    // this variable is here because LuascriptPlatform has the mouse state callbacks
    private bool cursor_is_hidden;
    private double cursor_hide_timestamp;

    private PSFramebuffer shader_framebuffer_front;
    private PSFramebuffer shader_framebuffer_back;
    private PSFramebuffer target_framebuffer;
    internal bool shader_needs_flush;
    internal StackList<PSShader> shader_stack = new StackList<PSShader>(SHADER_STACK_LENGTH);
    private int shader_last_resolution_changes = 0;
    private int screen_stride;
    private bool mute_on_minimize;


    private int stack_index = 0;

    private float vertex_alpha = 1.0f;
    private readonly float[] vertex_offsetcolor = new float[] { 0.0f, 0.0f, 0.0f, 0.0f };

    internal float global_alpha = 1.0f;
    internal float[] global_offsetcolor = null;

    private PVRFlag global_antialiasing = PVRFlag.DEFAULT;
    private PVRFlag vertex_antialiasing = PVRFlag.DEFAULT;

    private PVRFlag global_offsetcolor_multiply = PVRFlag.DEFAULT;
    private PVRFlag vertex_offsetcolor_multiply = PVRFlag.DEFAULT;

    internal float render_alpha = 1.0f;
    internal readonly float[] render_offsetcolor = new float[] { 0.0f, 0.0f, 0.0f, 0.0f };
    internal PVRFlag render_offsetcolor_multiply = PVRFlag.DEFAULT;
    internal PVRFlag render_antialiasing = PVRFlag.DEFAULT;

    public SIMDMatrix CurrentMatrix { get => this.stack[this.stack_index].matrix; }

    public int ScreenWidth { get => PVR_WIDTH; }
    public int ScreenStride { get => screen_stride; }
    public int ScreenHeight { get => PVR_HEIGHT; }

    private PVRContextState PreviousState {
        get
        {
            if (this.stack_index > 0)
                return this.stack[this.stack_index - 1];
            else
                return null;
        }
    }

    private void IconifyCallback(Window window, bool focusing) {
        if (window == this.nativeWindow) {
            this.is_minimized = !focusing;
            if (this.mute_on_minimize) SoundBridge.SetMasterMuted(focusing);
        }
    }

    private void SizeCallback(Window window, int width, int height) {
        if (window != this.nativeWindow) return;
        if (width == 0 || height == 0) return;

        this.screen_stride = width;

        double aspect_ratio = EngineSettings.widescreen ? WIDESCREEN_ASPECT_RATIO : DREAMCAST_ASPECT_RATIO;
        double width_final = height * aspect_ratio;
        int x = (int)((width - width_final) / 2.0);
        width = (int)width_final;

        this.PVR_WIDTH = width;
        this.PVR_HEIGHT = height;
        this.webopengl.ResizeProjection(x, 0, width, height);

        if (Glfw.GetWindowMonitor(this.nativeWindow) == Monitor.None) {
            this.last_windowed_width = width;
            this.last_windowed_height = height;
        }

        this.resolution_changes++;
    }

    private void WindowCallback(Window window) {
        is_closed = true;
    }

    private void UpdateWindowMonitor(bool as_windowed) {
        Monitor primary = Glfw.GetPrimaryMonitor();
        int area_x, area_y;

        Glfw.GetMonitorWorkArea(primary, out area_x, out area_y, out _, out _);

        if (as_windowed) {
            Glfw.SetWindowMonitor(
                this.nativeWindow,
                Monitor.None,
                this.last_windowed_x,
                this.last_windowed_y,
                this.last_windowed_width,
                this.last_windowed_height,
                Glfw.DONT_CARE
            );
        } else {
            VideoMode videoMode = Glfw.GetVideoMode(primary);
            if (videoMode == null) return;

            Glfw.SetWindowMonitor(
                this.nativeWindow,
                primary,
                area_x,
                area_y,
                videoMode.width,
                videoMode.height,
                this.vsync_enabled ? videoMode.refreshRate : Glfw.DONT_CARE
            );
        }
    }

    private void DrawFPS() {
        if (this.fps_text == null) {
            FontHolder font = new FontHolder("/assets/common/font/vcr.ttf", -1, null, false);
            this.fps_text = TextSprite.Init2(font, 64f, 0xAAFFAA);
            this.fps_text.SetDrawLocation(2f, 0f);
        }

        if (this.fps_resolution_changes != this.resolution_changes) {
            // update font size
            this.fps_resolution_changes = this.resolution_changes;
            this.fps_text.SetFontSize(this.PVR_HEIGHT * PVRContext.FPS_FONT_SIZE_RATIO);
        }

        double now = Glfw.GetTime();
        if (now >= this.fps_next_time) {
            this.fps_next_time = now + 1.0;
            this.fps_text.SetText(this.fps_count.ToString());
            this.fps_count = 0;
        }
        this.fps_count++;

        float width, height;
        this.fps_text.GetDrawSize(out width, out height);

        this.render_alpha = PVRContext.FPS_BG_COLOR[3];
        DrawSolidColor(PVRContext.FPS_BG_COLOR, 0, 0, width * 1.1f, height * 1.1f);

        float global_alpha = this.global_alpha;// backup global alpha
        this.global_alpha = 1.0f;
        this.fps_text.Draw(this);
        this.global_alpha = global_alpha;
    }


    private PVRContext() {
        for (int i = 0 ; i < PVRContext.STACK_LENGTH ; i++) this.stack[i] = new PVRContextState();
        this.global_offsetcolor = this.stack[0].offsetcolor;
        PVRContext.HelperClearOffsetColor(this.vertex_offsetcolor);
        PVRContext.HelperClearOffsetColor(this.render_offsetcolor);

        Glfw.Init();
        Glfw.WindowHint(Hint.ClientApi, Glfw.CLIENT_API_OPENGL);
        Glfw.WindowHint(Hint.ContextVersionMajor, 3);
        Glfw.WindowHint(Hint.ContextVersionMinor, 3);
        Glfw.WindowHint(Hint.OpenglProfile, Glfw.PROFILE_CORE);
        Glfw.WindowHint(Hint.Doublebuffer, true);
        Glfw.WindowHint(Hint.Decorated, true);

        int width;
        int height;
        Monitor monitor;

        if (EngineSettings.fullscreen) {
            monitor = Glfw.GetPrimaryMonitor();
            VideoMode videoMode = Glfw.GetVideoMode(monitor);
            width = videoMode.width;
            height = videoMode.height;
        } else {
            monitor = Monitor.None;
            width = EngineSettings.widescreen ? 960 : 640;
            height = EngineSettings.widescreen ? 540 : 480;
        }

        string title;
        if (Expansions.startup_title != null) {
            title = Expansions.startup_title;
            Expansions.startup_title = null;
        } else {
#if DEBUG
            title = $"{Engine.Game.GameMain.ENGINE_NAME} {Engine.Game.GameMain.ENGINE_VERSION}";
#else
            title = Funkin.FUNKY.Replace("--", "\x20");
#endif
        }

        this.last_timestamp = 0;
        this.native_window_title = title;
        this.nativeWindow = Glfw.CreateWindow(width, height, title, monitor, Window.None);
        Glfw.MakeContextCurrent(this.nativeWindow);
        Glfw.GetWindowPosition(this.nativeWindow, out this.last_windowed_x, out this.last_windowed_y);

        Logger.Info("GLFW: " + Glfw.GetVersionString());

        this.last_windowed_width = width;
        this.last_windowed_height = height;
        this.force_widescreen = EngineSettings.widescreen;

        this.last_timestamp = -1;
        this.fps_next_time = 0.0;
        this.fps_count = 0;
        this.fps_resolution_changes = -1;

        SetFPSLimit(EngineSettings.fps_limit, false);

        this.webopengl = new WebGLContext();

        SizeCallback(this.nativeWindow, width, height);

        this.iconifyCallback = IconifyCallback;
        this.sizeCallback = SizeCallback;
        this.closeCallback = WindowCallback;

        Glfw.SetWindowIconifyCallback(nativeWindow, this.iconifyCallback);
        Glfw.SetWindowSizeCallback(nativeWindow, this.sizeCallback);
        Glfw.SetCloseCallback(this.nativeWindow, this.closeCallback);

        this.webopengl.ClearScreen(PVRContext.CLEAR_COLOR);
        Glfw.SwapBuffers(this.nativeWindow);
        this.webopengl.ClearScreen(PVRContext.CLEAR_COLOR);

        using (IconLoader icon = new IconLoader(Expansions.startup_icon ?? Engine.Properties.Resources.icon)) {
            Icon[] icons = new Icon[icon.icons.Length];
            for (int i = 0 ; i < icons.Length ; i++) {
                icons[i] = new Icon(
                    icon.icons[i].width,
                    icon.icons[i].height,
                    icon.icons[i].pixels
                );
            }

            Expansions.startup_icon = null;
            Glfw.SetWindowIcon(this.nativeWindow, icons.Length, icons);
        }

        LuascriptPlatform.__initialize(this.nativeWindow, this);
        maple.__initialize(this.nativeWindow);
        SoundBridge.SetMasterMuted(EngineSettings.mute_on_minimize && this.is_minimized);

        this.shader_framebuffer_front = new PSFramebuffer(this);
        this.shader_framebuffer_back = new PSFramebuffer(this);
        this.shader_last_resolution_changes = this.resolution_changes;
        PSFramebuffer.ResizeQuadScreen(this);

        if (EngineSettings.pixelbufferobjects) {
            // partial implementation, needs async texture loading
            TextureLoader.SetPixelBufferBuilder(new WGLUnPackBufferBuilder(this.webopengl.gl));
        }

        this.cursor_is_hidden = false;
        if (EngineSettings.autohide_cursor)
            this.cursor_hide_timestamp = Glfw.GetTime() + AUTOHIDE_CURSOR_SECONDS;
        else
            this.cursor_hide_timestamp = Double.PositiveInfinity;

        Logger.Info("PowerVR backend init completed\n");
    }


    public void SetFPSLimit(int type, bool force_apply) {
        if (type == 0) {
            // vsync
            Glfw.SwapInterval(1);
        } else {
            // deterministic/off
            Glfw.SwapInterval(0);
        }
        this.is_deterministic = type == 1;
        this.vsync_enabled = type == 0;

        if (force_apply) {
            Monitor monitor = Glfw.GetWindowMonitor(this.nativeWindow);
            bool is_windowed = monitor == Monitor.None;
            if (is_windowed) {
                Glfw.GetWindowPosition(
                    this.nativeWindow, out this.last_windowed_x, out this.last_windowed_y
                );
            }
            UpdateWindowMonitor(is_windowed);
        }
    }

    public void ResetElapsed() {
        this.last_timestamp = 0.0;
    }

    public void Reset() {
        this.stack_index = 0;
        this.stack[0].matrix.Clear();

        this.stack[0].global_alpha = 1.0f;
        this.global_offsetcolor = this.stack[0].offsetcolor;

        this.global_alpha = 1.0f;
        this.vertex_alpha = 1.0f;
        this.render_alpha = 1.0f;

        PVRContext.HelperClearOffsetColor(this.global_offsetcolor);
        PVRContext.HelperClearOffsetColor(this.vertex_offsetcolor);

        this.global_antialiasing = PVRFlag.ENABLE;
        this.vertex_antialiasing = PVRFlag.DEFAULT;
        this.render_antialiasing = PVRFlag.DEFAULT;

        this.global_offsetcolor_multiply = PVRFlag.ENABLE;
        this.vertex_offsetcolor_multiply = PVRFlag.DEFAULT;
        this.render_offsetcolor_multiply = PVRFlag.DEFAULT;

        this.webopengl.ClearScreen(PVRContext.CLEAR_COLOR);

        this.FlushFramebuffer();
        this.shader_stack.Clear();
        PSFramebuffer.UseScreenFramebuffer(this);
        this.webopengl.SetBlend(this, true, Blend.DEFAULT, Blend.DEFAULT, Blend.DEFAULT, Blend.DEFAULT);
        this.webopengl.program_textured.darken_enabled = false;
        this.webopengl.program_solid.darken_enabled = false;
        this.webopengl.program_yuv.darken_enabled = false;
    }

    public void ApplyModifier(Modifier modifier) {
        SIMDMatrix matrix = this.stack[this.stack_index].matrix;
        matrix.ApplyModifier(modifier);
    }

    public void ApplyModifier2(Modifier modifier, float draw_x, float draw_y, float draw_width, float draw_height) {
        SIMDMatrix matrix = this.stack[this.stack_index].matrix;
        matrix.ApplyModifier2(modifier, draw_x, draw_y, draw_width, draw_height);
    }

    public void ClearScreen(float[] rgba_color) {
        this.webopengl.ClearScreen(rgba_color);
    }


    [Obsolete]
    public void Flush() {
        webopengl.Flush();
    }

    public bool Save() {
        int next_index = this.stack_index + 1;

        if (this.stack_index >= STACK_LENGTH) {
            Logger.Warn("pvr_context_save() the PVRContext stack is full");
            return false;
        }

        //int old_irq = SH4_INTERRUPS_DISABLE();

        PVRContextState previous_state = this.stack[this.stack_index];
        PVRContextState next_state = this.stack[next_index];

        // copy previous matrix
        previous_state.matrix.CopyTo(next_state.matrix);

        // backup
        previous_state.global_alpha = this.global_alpha;
        previous_state.global_antialiasing = this.global_antialiasing;
        previous_state.global_offsetcolor_multiply = this.global_offsetcolor_multiply;
        for (int i = 0 ; i < 4 ; i++) next_state.offsetcolor[i] = previous_state.offsetcolor[i];

        // set previous values as default
        this.vertex_alpha = this.global_alpha;
        this.render_alpha = this.global_alpha;

        this.vertex_antialiasing = PVRFlag.DEFAULT;
        this.render_antialiasing = previous_state.global_antialiasing;

        this.vertex_offsetcolor_multiply = this.global_offsetcolor_multiply;
        this.render_offsetcolor_multiply = this.global_offsetcolor_multiply;

        this.global_offsetcolor = next_state.offsetcolor;// change reference
        for (int i = 0 ; i < 4 ; i++) {
            this.vertex_offsetcolor[i] = this.global_offsetcolor[i];
            this.render_offsetcolor[i] = this.global_offsetcolor[i];
        }

        this.stack_index = next_index;

        //SH4_INTERRUPS_ENABLE(old_irq);

        // remember the last count of added shaders
        previous_state.added_shaders = this.shader_stack.Length;

        return true;
    }

    public bool Restore() {
        if (this.stack_index < 1) {
            if (this.shader_stack.Length > 0) {
                Logger.Warn("pvr_context_restore() the current PVRContext has stacked shaders on empty stack");
            }
            Logger.Warn("pvr_context_restore() the PVRContext stack was empty");
            return false;
        }

        //let old_irq = SH4_INTERRUPS_DISABLE();

        this.stack_index--;

        PVRContextState previous_state = this.stack[this.stack_index];

        this.global_antialiasing = previous_state.global_antialiasing;
        this.vertex_antialiasing = PVRFlag.DEFAULT;
        this.render_antialiasing = previous_state.global_antialiasing;

        this.global_offsetcolor_multiply = previous_state.global_offsetcolor_multiply;
        this.vertex_offsetcolor_multiply = previous_state.global_offsetcolor_multiply;
        this.render_offsetcolor_multiply = previous_state.global_offsetcolor_multiply;

        this.global_alpha = previous_state.global_alpha;
        this.vertex_alpha = previous_state.global_alpha;
        this.render_alpha = previous_state.global_alpha;

        this.global_offsetcolor = previous_state.offsetcolor;// change reference
        for (int i = 0 ; i < 4 ; i++) {
            this.vertex_offsetcolor[i] = previous_state.offsetcolor[i];// restore
            this.render_offsetcolor[i] = previous_state.offsetcolor[i];// restore
        }

        //SH4_INTERRUPS_ENABLE(old_irq);

        // remove all shaders added in the current state
        int added_shaders = this.shader_stack.Length - previous_state.added_shaders;
        Debug.Assert(added_shaders >= 0);
        if (added_shaders > 0) this.ShaderStackPop(added_shaders);
        this.webopengl.SetBlend(this, true, Blend.DEFAULT, Blend.DEFAULT, Blend.DEFAULT, Blend.DEFAULT);

        return true;
    }



    public void SetVertexAlpha(float alpha) {
        this.vertex_alpha = alpha;
        this.render_alpha = alpha * this.global_alpha;
    }


    public void SetVertexAntialiasing(PVRFlag flag) {
        this.vertex_antialiasing = flag;
        this.render_antialiasing = flag == PVRFlag.DEFAULT ? this.global_antialiasing : flag;
    }


    public void SetVertexOffsetColor(float[] offsetcolor) {
        for (int i = 0 ; i < 4 ; i++) this.vertex_offsetcolor[i] = offsetcolor[i];
        Math2D.ColorBlendNormal(offsetcolor, this.global_offsetcolor, this.render_offsetcolor);
    }


    public void SetVertexOffsetColorMultiply(PVRFlag flag) {
        this.vertex_offsetcolor_multiply = flag;
        this.render_offsetcolor_multiply = flag == PVRFlag.DEFAULT ? this.global_offsetcolor_multiply : flag;
    }

    public void SetVertexBlend(bool enabled, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha) {
        this.webopengl.SetBlend(this, enabled, src_rgb, dst_rgb, src_alpha, dst_alpha);
    }

    public void SetVertexTexturedDarken(bool enabled) {
        this.webopengl.program_textured.darken_enabled = enabled;
        this.webopengl.program_solid.darken_enabled = enabled;
        this.webopengl.program_yuv.darken_enabled = enabled;
    }


    public void SetGlobalAlpha(float alpha) {
        PVRContextState last_state = this.PreviousState;

        if (last_state != null) alpha *= last_state.global_alpha;

        this.global_alpha = alpha;
        this.SetVertexAlpha(this.vertex_alpha);
    }


    public void SetGlobalAntialiasing(PVRFlag flag) {
        PVRContextState last_state = this.PreviousState;

        if (last_state != null && flag == PVRFlag.DEFAULT) flag = last_state.global_antialiasing;

        this.global_antialiasing = flag;
        this.SetVertexAntialiasing(this.vertex_antialiasing);
    }


    public void SetGlobalOffsetColor(float[] offsetcolor) {
        PVRContextState last_state = this.PreviousState;

        if (last_state != null) {
            Math2D.ColorBlendNormal(offsetcolor, last_state.offsetcolor, this.global_offsetcolor);
        } else {
            for (int i = 0 ; i < 4 ; i++) this.global_offsetcolor[i] = offsetcolor[i];
        }

        this.SetVertexOffsetColor(this.vertex_offsetcolor);
    }


    public void SetGlobalOffsetColorMultiply(PVRFlag flag) {
        PVRContextState last_state = this.PreviousState;

        if (last_state != null && flag == PVRFlag.DEFAULT) flag = last_state.global_offsetcolor_multiply;

        this.global_offsetcolor_multiply = flag;
        this.SetVertexOffsetColorMultiply(this.vertex_offsetcolor_multiply);
    }



    public void DrawTexture(Texture texture, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
        if (texture.data_vram.IsNull) return;
        if (this.shader_stack.Length > 0) this.shader_needs_flush = true;

        if (texture.data_vram_crhoma_planes == null)
            this.webopengl.DrawTexture(this, texture, sx, sy, sw, sh, dx, dy, dw, dh);
        else
            this.webopengl.DrawTextureYUV(this, texture, sx, sy, sw, sh, dx, dy, dw, dh);
    }

    public void DrawSolidColor(float[] rgb_color, float dx, float dy, float dw, float dh) {
        if (this.shader_stack.Length > 0) this.shader_needs_flush = true;
        this.webopengl.DrawSolid(this, rgb_color, dx, dy, dw, dh);
    }

    public void DrawFramebuffer(PSFramebuffer psframebuffer, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
        this.shader_needs_flush = true;
        this.webopengl.DrawFramebuffer(this, psframebuffer, sx, sy, sw, sh, dx, dy, dw, dh);
    }


    public static void MuteAudioOutputOnMinimized(bool enable) {
        global_context.mute_on_minimize = enable;

        if (enable)
            SoundBridge.SetMasterMuted(global_context.is_minimized);
        else
            SoundBridge.SetMasterMuted(false);
    }

    public bool IsOffscreen() {
        return !Glfw.GetWindowAttrib(this.nativeWindow, Glfw.WINDOWATTRIBUTE_FOCUSED);
    }

    public bool IsMinimized() {
        return this.is_minimized;
    }

    public bool AddShader(PSShader psshader) {
        return this.ShaderStackPush(psshader);
    }

    public void SetFramebuffer(PSFramebuffer psframebuffer) {
        this.target_framebuffer = psframebuffer;
        if (this.shader_stack.Length < 1) this.UseDefaultFramebuffer();
    }

    public static void HideWindow(bool hide) {
        if (hide)
            Glfw.HideWindow(global_context.nativeWindow);
        else
            Glfw.ShowWindow(global_context.nativeWindow);
    }

    public static void ToggleFullscreen() {
        Monitor monitor = Glfw.GetWindowMonitor(global_context.nativeWindow);
        bool is_windowed = monitor == Monitor.None;

        if (is_windowed) {
            Glfw.GetWindowPosition(
                global_context.nativeWindow,
                out global_context.last_windowed_x,
                out global_context.last_windowed_y
            );
            // switch to fullscreen
            global_context.UpdateWindowMonitor(false);
        } else {
            // switch to windowed
            global_context.UpdateWindowMonitor(true);
            /*Glfw.SetWindowPosition(
                global_context.nativeWindow,
                global_context.last_windowed_x,
                global_context.last_windowed_y
            );*/
        }
    }

    public static void TakeScreenshot() {
        int width, height;
        nint ptr = PVRContext.global_context.webopengl.ReadFrameBufferBGRA(out width, out height);
        if (ptr == 0x00) return;

        string filename = DateTime.Now.ToString("yyyy-MM-ddTHHmmss.fff") + ".png";
        filename = $"{EngineSettings.EngineDir}screenshots{Path.DirectorySeparatorChar}{filename}";

        // async file writting to avoid suttering
        ImageWritter.WritePNGFromBGRAAndFreeAsync(ptr, width, height, true, filename);
    }

    public static void Init() {
        if (PVRContext.global_context != null) return;
        PVRContext.global_context = new PVRContext();
    }

    internal static Window InternalNativeWindow {
        get => PVRContext.global_context.nativeWindow;
    }

    public static bool IsRunningInFullscreen {
        get
        {
            Monitor monitor = Glfw.GetWindowMonitor(PVRContext.global_context.nativeWindow);
            return monitor == Monitor.None;
        }
    }


    internal void FlushFramebuffer() {
        if (!this.shader_needs_flush) return;

        PSFramebuffer front = this.shader_framebuffer_front;
        PSFramebuffer back = this.shader_framebuffer_back;
        int last_index = this.shader_stack.Length - 1;

        for (int i = 0 ; i < this.shader_stack.Length ; i++) {
            if (i == last_index)
                this.UseDefaultFramebuffer();
            else
                back.Use(true);

            this.shader_stack[i].Draw(front);

            PSFramebuffer tmp = front;
            front = back;
            back = tmp;
        }

        this.shader_needs_flush = false;
    }

    private bool ShaderStackPush(PSShader psshader) {
        if (psshader == null) throw new ArgumentNullException("psshader");

        this.FlushFramebuffer();

        if (this.shader_stack.Length >= PVRContext.SHADER_STACK_LENGTH) {
            Logger.Warn("PVRContext::ShaderStackPush() failed, the stack is full");
            return false;
        }

        this.shader_stack.Push(psshader);
        this.shader_framebuffer_front.Use(true);

        return true;
    }

    internal bool ShaderStackPop(int count) {
        this.FlushFramebuffer();

        if (this.shader_stack.Length < 1) {
            Logger.Warn("PVRContext::ShaderStackPop() failed, the stack is empty");
            return false;
        }

        while (count-- > 0) this.shader_stack.Pop();

        if (this.shader_stack.Length < 1)
            this.UseDefaultFramebuffer();
        else
            this.shader_framebuffer_front.Use(true);

        return true;
    }

    internal void CheckFrameBufferSize() {
        if (this.resolution_changes == this.shader_last_resolution_changes) return;

        this.shader_last_resolution_changes = this.resolution_changes;
        this.shader_framebuffer_front.Resize();
        this.shader_framebuffer_back.Resize();
        PSFramebuffer.ResizeQuadScreen(this);
    }

    internal void UseDefaultFramebuffer() {
        if (this.target_framebuffer != null)
            this.target_framebuffer.Use(false);
        else
            PSFramebuffer.UseScreenFramebuffer(this);
    }

}
