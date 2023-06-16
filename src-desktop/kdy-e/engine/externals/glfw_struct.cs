using System;
using System.Runtime.InteropServices;

namespace Engine.Externals.GLFW;

[StructLayout(LayoutKind.Sequential)]
internal struct GamePadState {
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 15)]
    public readonly byte[] buttons;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
    public readonly float[] axes;
}

[StructLayout(LayoutKind.Sequential)]
internal struct Window {
    private readonly IntPtr handle;
    public static readonly Window None;

    public Window(IntPtr handle) {
        this.handle = handle;
    }

    public override bool Equals(object obj) {
        return obj is Window window && this.handle == ((Window)obj).handle;
    }

    public override int GetHashCode() {
        return handle.GetHashCode();
    }

    public static bool operator ==(Window w1, Window w2) {
        return w1.handle == w2.handle;
    }

    public static bool operator !=(Window w1, Window w2) {
        return w1.handle != w2.handle;
    }
}

[StructLayout(LayoutKind.Sequential)]
internal struct Monitor {
    private readonly IntPtr handle;
    public static readonly Monitor None;

    public override bool Equals(object obj) {
        return obj is Monitor window && this.handle == ((Monitor)obj).handle;
    }

    public override int GetHashCode() {
        return handle.GetHashCode();
    }

    public static bool operator ==(Monitor m1, Monitor m2) {
        return m1.handle == m2.handle;
    }

    public static bool operator !=(Monitor m1, Monitor m2) {
        return m1.handle != m2.handle;
    }
}

[StructLayout(LayoutKind.Sequential)]
internal struct VideoMode {
    public readonly int width;
    public readonly int height;
    public readonly int redBits;
    public readonly int greenBits;
    public readonly int blueBits;
    public readonly int refreshRate;
}

[StructLayout(LayoutKind.Sequential)]
internal struct Icon {
    public readonly int width;
    public readonly int height;
    public readonly IntPtr pixels;

    public Icon(int width, int height, IntPtr pixels) {
        this.width = width;
        this.height = height;
        this.pixels = pixels;
    }
}

