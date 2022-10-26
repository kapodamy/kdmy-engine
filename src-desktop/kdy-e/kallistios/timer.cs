using Engine.Externals.GLFW;

namespace KallistiOS {
    public static class timer {
        public static double ms_gettime64() {
            return Glfw.GetTime() * 1000.0;
        }
    }
}
