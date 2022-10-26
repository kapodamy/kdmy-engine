using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Engine.Platform;

namespace Engine.Externals.GLFW {

    internal static class Glfw {
        private const string GLFW = "glfw";

        public const int JOYSTICK_LAST = 15;
        public const int CLIENT_API_OPENGL = 0x30001;
        public const int DONT_CARE = -1;
        public const int PROFILE_CORE = 0x00032001;
        public const int WINDOWATTRIBUTE_FOCUSED = 0x00020001;


        [DllImport(GLFW, EntryPoint = "glfwInit", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool Init();

        [DllImport(GLFW, EntryPoint = "glfwTerminate", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Terminate();

        [DllImport(GLFW, EntryPoint = "glfwGetGamepadState", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool GetGamepadState(int id, out GamePadState state);

        [DllImport(GLFW, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr glfwGetJoystickName(int joystick);

        [DllImport(GLFW, EntryPoint = "glfwGetKey", CallingConvention = CallingConvention.Cdecl)]
        public static extern InputState GetKey(Window window, Keys key);

        [DllImport(GLFW, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr glfwGetKeyName(Keys key, int scanCode);

        [DllImport(GLFW, EntryPoint = "glfwGetKeyScancode", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetKeyScanCode(Keys key);

        [DllImport(GLFW, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr glfwGetVideoMode(Monitor monitor);

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
        private static extern IntPtr glfwGetVersionString();

        [DllImport(GLFW, EntryPoint = "glfwWindowHint", CallingConvention = CallingConvention.Cdecl)]
        public static extern void WindowHint(Hint hint, int value);

        [DllImport(GLFW, EntryPoint = "glfwGetTime", CallingConvention = CallingConvention.Cdecl)]
        public static extern double GetTime();

        [DllImport(GLFW, EntryPoint = "glfwGetMonitorWorkarea", CallingConvention = CallingConvention.Cdecl)]
        public static extern void GetMonitorWorkArea(Monitor monitor, out int x, out int y, out int width, out int height);

        [DllImport(GLFW, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr glfwCreateWindow(int width, int height, byte[] title, Monitor monitor, Window share);

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


        private static string StringFromPtr(IntPtr ptr) {
            if (ptr == IntPtr.Zero) return null;

            int length = 0;
            while (Marshal.ReadByte(ptr, length) != 0x00) length++;

            if (length < 1) return String.Empty;

            byte[] buffer = new byte[length];
            Marshal.Copy(ptr, buffer, 0, length);

            return Encoding.UTF8.GetString(buffer);
        }

    }

}

