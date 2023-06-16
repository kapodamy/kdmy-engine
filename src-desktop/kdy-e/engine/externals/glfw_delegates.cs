using System.Runtime.InteropServices;

namespace Engine.Externals.GLFW;

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void KeyCallback(Window window, Keys key, int scancode, InputState action, ModKeys mods);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void JoystickCallback(int joystick, ConnectionStatus status);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void MonitorCallback(Monitor monitor, ConnectionStatus status);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void IconifyCallback(Window window, bool focusing);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void SizeCallback(Window window, int width, int height);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void WindowCallback(Window window);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void MouseCallback(Window window, double x, double y);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void MouseEnterCallback(Window window, bool entering);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void MouseButtonCallback(Window window, MouseButtons button, InputState state, ModKeys mods);
