using System;
using System.Runtime.InteropServices;
using System.Text;

namespace Engine.Externals.GLFW;

internal static class Glfw {
    private const string GLFW = "glfw";

    public const int JOYSTICK_LAST = 15;
    public const int CLIENT_API_OPENGL = 0x30001;
    public const int DONT_CARE = -1;
    public const int PROFILE_CORE = 0x00032001;
    public const int WINDOWATTRIBUTE_FOCUSED = 0x00020001;
    public const int WINDOWATTRIBUTE_ICONIFIED = 0x00020002;
    public const int CURSOR = 0x00033001;
    public const int CURSOR_NORMAL = 0x00034001;
    public const int CURSOR_HIDDEN = 0x00034002;

    public const string GAMECONTROLLERDB = "gamecontrollerdb.txt";


    [DllImport(GLFW, EntryPoint = "glfwInit", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool Init();

    [DllImport(GLFW, EntryPoint = "glfwTerminate", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Terminate();

    [DllImport(GLFW, EntryPoint = "glfwGetGamepadState", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetGamepadState(int id, out GamePadState state);

    [DllImport(GLFW, CallingConvention = CallingConvention.Cdecl)]
    private static extern nint glfwGetJoystickName(int joystick);

    [DllImport(GLFW, EntryPoint = "glfwGetKey", CallingConvention = CallingConvention.Cdecl)]
    public static extern InputState GetKey(Window window, Keys key);

    [DllImport(GLFW, CallingConvention = CallingConvention.Cdecl)]
    private static extern nint glfwGetKeyName(Keys key, int scanCode);

    [DllImport(GLFW, EntryPoint = "glfwGetKeyScancode", CallingConvention = CallingConvention.Cdecl)]
    public static extern int GetKeyScanCode(Keys key);

    [DllImport(GLFW, CallingConvention = CallingConvention.Cdecl)]
    private static extern nint glfwGetVideoMode(Monitor monitor);

    [DllImport(GLFW, EntryPoint = "glfwGetWindowMonitor", CallingConvention = CallingConvention.Cdecl)]
    public static extern Monitor GetWindowMonitor(Window window);

    [DllImport(GLFW, EntryPoint = "glfwGetWindowPos", CallingConvention = CallingConvention.Cdecl)]
    public static extern void GetWindowPosition(Window window, out int x, out int y);

    [DllImport(GLFW, EntryPoint = "glfwJoystickIsGamepad", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool JoystickIsGamepad(int joystickId);

    [DllImport(GLFW, EntryPoint = "glfwPollEvents", CallingConvention = CallingConvention.Cdecl)]
    public static extern void PollEvents();

    [DllImport(GLFW, EntryPoint = "glfwGetPrimaryMonitor", CallingConvention = CallingConvention.Cdecl)]
    public static extern Monitor GetPrimaryMonitor();

    [DllImport(GLFW, EntryPoint = "glfwSetJoystickCallback", CallingConvention = CallingConvention.Cdecl)]
    public static extern JoystickCallback SetJoystickCallback(JoystickCallback callback);

    [DllImport(GLFW, EntryPoint = "glfwSetKeyCallback", CallingConvention = CallingConvention.Cdecl)]
    public static extern KeyCallback SetKeyCallback(Window window, KeyCallback keyCallback);

    [DllImport(GLFW, EntryPoint = "glfwSetWindowIconifyCallback", CallingConvention = CallingConvention.Cdecl)]
    public static extern IconifyCallback SetWindowIconifyCallback(Window window, IconifyCallback callback);

    [DllImport(GLFW, EntryPoint = "glfwSetWindowSizeCallback", CallingConvention = CallingConvention.Cdecl)]
    public static extern SizeCallback SetWindowSizeCallback(Window window, SizeCallback sizeCallback);

    [DllImport(GLFW, EntryPoint = "glfwSetWindowMonitor", CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetWindowMonitor(Window window, Monitor monitor, int x, int y, int width, int height, int refreshRate);

    [DllImport(GLFW, EntryPoint = "glfwSwapInterval", CallingConvention = CallingConvention.Cdecl)]
    public static extern void SwapInterval(int interval);

    [DllImport(GLFW, CallingConvention = CallingConvention.Cdecl)]
    private static extern nint glfwGetVersionString();

    [DllImport(GLFW, EntryPoint = "glfwWindowHint", CallingConvention = CallingConvention.Cdecl)]
    public static extern void WindowHint(Hint hint, int value);

    [DllImport(GLFW, EntryPoint = "glfwGetTime", CallingConvention = CallingConvention.Cdecl)]
    public static extern double GetTime();

    [DllImport(GLFW, EntryPoint = "glfwGetMonitorWorkarea", CallingConvention = CallingConvention.Cdecl)]
    public static extern void GetMonitorWorkArea(Monitor monitor, out int x, out int y, out int width, out int height);

    [DllImport(GLFW, CallingConvention = CallingConvention.Cdecl)]
    private static extern nint glfwCreateWindow(int width, int height, byte[] title, Monitor monitor, Window share);

    [DllImport(GLFW, EntryPoint = "glfwMakeContextCurrent", CallingConvention = CallingConvention.Cdecl)]
    public static extern void MakeContextCurrent(Window window);

    [DllImport(GLFW, EntryPoint = "glfwSwapBuffers", CallingConvention = CallingConvention.Cdecl)]
    public static extern void SwapBuffers(Window window);

    [DllImport(GLFW, EntryPoint = "glfwGetWindowAttrib", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetWindowAttrib(Window window, int attribute);

    [DllImport(GLFW, EntryPoint = "glfwSetWindowIcon", CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetWindowIcon(Window window, int count, Icon[] icons);

    [DllImport(GLFW, EntryPoint = "glfwShowWindow", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ShowWindow(Window window);

    [DllImport(GLFW, EntryPoint = "glfwHideWindow", CallingConvention = CallingConvention.Cdecl)]
    public static extern void HideWindow(Window window);

    [DllImport(GLFW, EntryPoint = "glfwSetWindowCloseCallback", CallingConvention = CallingConvention.Cdecl)]
    public static extern WindowCallback SetCloseCallback(Window window, WindowCallback closeCallback);

    [DllImport(GLFW, EntryPoint = "glfwUpdateGamepadMappings", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool UpdateGamepadMappings([MarshalAs(UnmanagedType.LPStr)] string mappings);

    [DllImport(GLFW, EntryPoint = "glfwSetCursorPosCallback", CallingConvention = CallingConvention.Cdecl)]
    public static extern MouseCallback SetCursorPositionCallback(Window window, MouseCallback mouseCallback);

    [DllImport(GLFW, EntryPoint = "glfwSetCursorEnterCallback", CallingConvention = CallingConvention.Cdecl)]
    public static extern MouseEnterCallback SetCursorEnterCallback(Window window, MouseEnterCallback mouseCallback);

    [DllImport(GLFW, EntryPoint = "glfwSetMouseButtonCallback", CallingConvention = CallingConvention.Cdecl)]
    public static extern MouseButtonCallback SetMouseButtonCallback(Window window, MouseButtonCallback mouseCallback);

    [DllImport(GLFW, EntryPoint = "glfwSetScrollCallback", CallingConvention = CallingConvention.Cdecl)]
    public static extern MouseCallback SetScrollCallback(Window window, MouseCallback mouseCallback);

    [DllImport(GLFW, EntryPoint = "glfwRequestWindowAttention", CallingConvention = CallingConvention.Cdecl)]
    public static extern void RequestWindowAttention(Window window);

    [DllImport(GLFW, EntryPoint = "glfwSetWindowTitle", CallingConvention = CallingConvention.Cdecl)]
    private static extern void glfwSetWindowTitle(Window window, byte[] title);

    [DllImport(GLFW, EntryPoint = "glfwSetInputMode", CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetInputMode(Window window, int mode, int value);

    [DllImport(GLFW, EntryPoint = "glfwGetInputMode", CallingConvention = CallingConvention.Cdecl)]
    public static extern int GetInputMode(Window window, int mode);


    public static string GetJoystickName(int joystick) {
        return StringFromPtr(glfwGetJoystickName(joystick));
    }

    public static string GetKeyName(Keys key, int scanCode) {
        return StringFromPtr(glfwGetKeyName(key, scanCode));
    }

    public static VideoMode GetVideoMode(Monitor monitor) {
        return Marshal.PtrToStructure<VideoMode>(glfwGetVideoMode(monitor));
    }

    public static void WindowHint(Hint hint, bool value) {
        WindowHint(hint, value ? 1 : 0);
    }

    public static string GetVersionString() {
        return StringFromPtr(glfwGetVersionString());
    }

    public static Window CreateWindow(int width, int height, string title, Monitor monitor, Window share) {
        byte[] title_ptr;
        if (title != null) {
            title_ptr = new byte[Encoding.UTF8.GetByteCount(title) + 1];
            Encoding.UTF8.GetBytes(title, 0, title.Length, title_ptr, 0);
        } else {
            title_ptr = null;
        }

        return new Window(glfwCreateWindow(width, height, title_ptr, monitor, share));
    }

    public static void SetWindowTitle(Window window, string title) {
        byte[] buffer;

        if (title == null) {
            buffer = null;
        } else {
            buffer = new byte[Encoding.UTF8.GetByteCount(title) + 1];
            Encoding.UTF8.GetBytes(title, 0, title.Length, buffer, 0);
        }

        glfwSetWindowTitle(window, buffer);
    }

    private static string StringFromPtr(nint ptr) {
        if (ptr == 0x00) return null;

        int length = 0;
        while (Marshal.ReadByte(ptr, length) != 0x00) length++;

        if (length < 1) return String.Empty;

        byte[] buffer = new byte[length];
        Marshal.Copy(ptr, buffer, 0, length);

        return Encoding.UTF8.GetString(buffer);
    }

}

