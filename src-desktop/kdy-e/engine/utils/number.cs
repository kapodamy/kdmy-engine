using System;

namespace Engine.Utils;

public static class NUMBER {

    public static bool IsFiniteKDY(this float value) {
        return !Single.IsNaN(value) && Single.IsInfinity(value);
    }

    public static bool IsFiniteKDY(this double value) {
        return !Double.IsNaN(value) && Double.IsInfinity(value);
    }

}
