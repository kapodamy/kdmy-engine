using System;
using System.Runtime.CompilerServices;
using System.Runtime.Intrinsics.X86;
using System.Text;

namespace Engine.Utils;

public static class Math2D {

    public static readonly float DEG_TO_RAD = (float)(Math.PI / 180.0);
    public const int MAX_INT32 = 0x7FFFFFFF;
    public static readonly float LOG100 = MathF.Log(100);
    public const float HALF_PI = MathF.PI / 2f;

    private static readonly Random random;

    static Math2D() {
        random = new Random(Guid.NewGuid().GetHashCode());
    }

    public static float RandomFloat() {
        //return (float)(random.Next() / (double)Int32.MaxValue);
        return (float)random.NextDouble();
    }

    public static double RandomDouble() {
        return random.NextDouble();
    }

    public static float Random(float min, float max) {
        return (float)(random.NextDouble() * (max - min + 1) + min);
    }

    public static double Random(double min, double max) {
        return random.NextDouble() * (max - min + 1) + min;
    }

    public static int RandomInt(int min, int max) {
        return random.Next(min, max + 1);
    }


    public static float Lerp(float start, float end, float step) {
        return start + (end - start) * step;
    }

    public static float InverseLerp(float start, float end, float value) {
        return (value - start) / (end - start);
    }
    public static double InverseLerp(double start, double end, double value) {
        return (value - start) / (end - start);
    }

    public static float NearestDown(float value, float step) {
        return value - (value % step);
    }

    public static float CubicBezier(float offset, float point0, float point1, float point2, float point3) {
        float neg = 1 - offset;
        return (neg * neg * neg) * point0 +
            3 * offset * (neg * neg) * point1 +
            3 * (offset * offset) * neg * point2 +
            (offset * offset * offset) * point3;
    }

    public static int Clamp(int value, int min, int max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    public static float Clamp(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
    public static double Clamp(double value, double min, double max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    public static void RotatePoint(float radians, ref float x, ref float y) {
        double s = Math.Sin(radians);
        double c = Math.Cos(radians);
        double X = x * c - y * s;
        double Y = x * s + y * c;

        x = (float)X;
        y = (float)Y;
    }


    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static float NormalAlphaCompose(float bg_alpha, float fg_alpha, float cmst_a, float bg_cmpt, float fg_cmpt) {
        float s = fg_alpha / cmst_a;
        float t = (1 - s) * bg_cmpt;
        float u = (1 - bg_alpha) * fg_cmpt + bg_alpha * fg_cmpt/*composite color component*/;

        /* u = math2d_roundf(u); */
        return t + (s * u);
    }

    public static void ColorBlendNormal(float[] back_color, float[] front_color, float[] output_color) {
        //
        // Avoid useless calculations if both colors are equal, also both alphas must be 1.0
        //

        for (int i = 0 ; i < 4 ; i++) {
            if (back_color[i] != front_color[i]) goto L_stop_checking_equal;
        }

        if (back_color[3] == 1.0/* && front_color[3] == 1.0*/) {
            // rgb components are equal an their alphas are 1.0
            if (front_color != output_color && back_color != output_color) {
                for (int i = 0 ; i < 4 ; i++) output_color[i] = front_color[i];
            }
            return;
        }

L_stop_checking_equal:
        if (back_color[3] <= 0) {
            // back color is transparent, return the front color
            if (front_color != output_color) {
                for (int i = 0 ; i < 4 ; i++) output_color[i] = front_color[i];
            }
            return;
        } else if (front_color[3] <= 0) {
            // front color is transparent, return the back color
            if (back_color != output_color) {
                for (int i = 0 ; i < 4 ; i++) output_color[i] = back_color[i];
            }
            return;
        }

        //
        // Color blending algorithm starts here
        //


        float ba = back_color[3], fa = front_color[3];

        if (ba <= 0.0) {
            output_color[0] = front_color[0];
            output_color[1] = front_color[1];
            output_color[2] = front_color[2];
            output_color[3] = fa;
            return;
        }

        float ca = fa + ba - fa * ba;
        float r = NormalAlphaCompose(ba, fa, ca, back_color[0], front_color[0]);
        float g = NormalAlphaCompose(ba, fa, ca, back_color[1], front_color[1]);
        float b = NormalAlphaCompose(ba, fa, ca, back_color[2], front_color[2]);

        /*
        // round final color components
        r = math2d_roundf(r);
        g = math2d_roundf(g);
        b = math2d_roundf(b);
        */

        output_color[0] = r;
        output_color[1] = g;
        output_color[2] = b;
        output_color[3] = ca;
    }

    public static uint ColorFloatsToBytes(float[] float_array, bool has_alpha) {
        int length = has_alpha ? 4 : 3;
        int shift = has_alpha ? 24 : 16;
        uint argb = 0x00;

        //
        // Array disposition:
        //      rgb8    [r, g, b]
        //      rgba8   [r, g, b, a]
        //

        for (int i = 0 ; i < length ; i++) {
            uint c = ((uint)(float_array[i] * 255)) & 0xFFU;
            argb |= c << shift;
            shift -= 8;
        }

        return argb;
    }

    public static void ColorBytesToFloats(uint rgba8_color, bool has_alpha, float[] output_float_array) {
        int length = has_alpha ? 4 : 3;
        int shift = has_alpha ? 24 : 16;

        for (int i = 0 ; i < length ; i++) {
            uint c = (rgba8_color >> shift) & 0xFF;
            output_float_array[i] = c / 255.0f;
            shift -= 8;
        }

    }

    public static uint BitCount(uint value) {
        if (Popcnt.IsSupported) {
            return Popcnt.PopCount(value);
        }

        // POPCNT .Net Impl
        value = value - ((value >> 1) & 0x55555555);
        value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
        return ((value + (value >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
    }

    public static float BeatsPerMinuteToBeatPerMilliseconds(float bpm) {
        return 60000.0f / bpm;
    }

    public static float PointsDistance(float x1, float y1, float x2, float y2) {
        return MathF.Sqrt(MathF.Pow(x1 - x2, 2f) + MathF.Pow(y1 - y2, 2f));
    }

    public static int PowerOfTwoCalc(int dimmen) {
        int size = 2;
        while (size < 0x80000) {
            if (size >= dimmen) break;
            size *= 2;
        }

        if (size >= 0x80000) throw new ArgumentOutOfRangeException("math2d_poweroftwo_calc() failed for: " + dimmen);
        return size;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool FloatsAreNearEqual(float float1, float float2) {
        return Math.Abs(float1 - float2) < Single.Epsilon;
    }

    public static bool FloatsAreNearZero(float number) {
        return Math.Abs(number) < Single.Epsilon;
    }

    public static string TimestampToString(double timestamp) {
        const string TIME_SECONDS = "$1ds";// 1.2s
        const string TIME_MINUTES = "$im$2is";// 1m23s
        const string TIME_HOURS = "$ih$2i$2i";// 1h23m45s

        if (Double.IsNaN(timestamp)) return "--:--.---";

        timestamp /= 1000.0;
        double h = Math.Floor(timestamp / 3600.0);
        double m = Math.Floor((timestamp - (h * 3600.0)) / 60.0);
        double s = timestamp - (h * 3600.0) - (m * 60.0);

        StringBuilder stringbuilder = new StringBuilder(9);

        if (h > 0.0)
            stringbuilder.AddFormatKDY(TIME_HOURS, (int)h, (int)m, (int)s);
        else if (m > 0.0)
            stringbuilder.AddFormatKDY(TIME_MINUTES, (int)m, (int)s);
        else
            stringbuilder.AddFormatKDY(TIME_SECONDS, s);

        string str = stringbuilder.GetCopyKDY();
        //stringbuilder.Destroy();

        return str;
    }

    public static float LerpCubic(float percent) {
        return MathF.Pow(percent, 3f);
    }

    public static float LerpQuad(float percent) {
        return MathF.Pow(percent, 2f);
    }

    public static float LerpExpo(float percent) {
        return MathF.Pow(2f, 10f * (percent - 1f));
    }

    public static float LerpSin(float percent) {
        return MathF.Sin(percent * Math2D.HALF_PI);
    }


}