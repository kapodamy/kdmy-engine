using Engine.Externals.GLFW;

#pragma warning disable CS8981

namespace KallistiOS;

public static class timer {
    public static double ms_gettime64() {
        return Glfw.GetTime() * 1000.0;
    }
}
