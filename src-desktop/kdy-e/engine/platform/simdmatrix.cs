using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics;
using System.Runtime.Intrinsics.X86;
using System.Text;
using Engine.Utils;

namespace Engine.Platform;

//
// Matrix computation using x86 SSE instructions
//
public class SIMDMatrix {
    private const int LENGTH = 16;
    private const int BYTE_SIZE = sizeof(float) * LENGTH;
    private const int SSE_ALIGNMENT = 0x10;
    private const int ROW0 = 0x00;
    private const int ROW1 = 0x04;
    private const int ROW2 = 0x08;
    private const int ROW3 = 0x0C;

    internal unsafe float* matrix;
    private unsafe float* matrix_temp;

    public readonly int Length = LENGTH;

    public SIMDMatrix() {
        unsafe {
            matrix = (float*)NativeMemory.AlignedAlloc(BYTE_SIZE, SSE_ALIGNMENT);
            // this matrix should allocated once if SIMDMatrix is not used in multithreaded envirioment
            matrix_temp = (float*)NativeMemory.AlignedAlloc(BYTE_SIZE, SSE_ALIGNMENT);

            Clear();
        }
    }

    public void Destroy() {
        unsafe {
            if (this.matrix == null) return;

            NativeMemory.AlignedFree(matrix);
            NativeMemory.AlignedFree(matrix_temp);
            this.matrix = null;
            this.matrix_temp = null;
        }
    }

    unsafe ~SIMDMatrix() {
        if (matrix != null) Destroy();
    }


    public void FlipX() {
        Scale(-1f, 1f);
    }

    public void FlipY() {
        Scale(1f, -1f);
    }


    public void RotateByDegs(float angle) {
        RotateByRads(angle * Math2D.DEG_TO_RAD);
    }

    public void RotateByRads(float angle) {
        if (angle == 0f) return;

        unsafe {
            // TODO: ¿SIMD optimization?
            float sin = (float)Math.Sin(angle);
            float cos = (float)Math.Cos(angle);

            float a = this.matrix[0];
            float b = this.matrix[1];
            float c = this.matrix[2];
            float d = this.matrix[3];
            float e = this.matrix[4];
            float g = this.matrix[5];
            float h = this.matrix[6];
            float i = this.matrix[7];

            this.matrix[0] = a * cos + e * sin;
            this.matrix[1] = b * cos + g * sin;
            this.matrix[2] = c * cos + h * sin;
            this.matrix[3] = d * cos + i * sin;
            this.matrix[4] = e * cos - a * sin;
            this.matrix[5] = g * cos - b * sin;
            this.matrix[6] = h * cos - c * sin;
            this.matrix[7] = i * cos - d * sin;
        }
    }

    public void Scale(float sx, float sy) {
        if (sx == 1f && sy == 1f) return;

        unsafe {
            float* tmp = this.matrix_temp;
            tmp[00] = sx; tmp[01] = 0f; tmp[02] = 0f; tmp[03] = 0f;
            tmp[04] = 0f; tmp[05] = sy; tmp[06] = 0f; tmp[07] = 0f;
            tmp[08] = 0f; tmp[09] = 0f; tmp[10] = 1f; tmp[11] = 0f;
            tmp[12] = 0f; tmp[13] = 0f; tmp[14] = 0f; tmp[15] = 1f;

            MultiplyInternal(tmp);
        }
    }

    public void ScaleX(float sx) {
        Scale(sx, 1f);
    }

    public void ScaleY(float sy) {
        Scale(1f, sy);
    }

    public void Skew(float sx, float sy) {
        if (sx == 0f && sy == 0f) return;

        unsafe {
            float* tmp = this.matrix_temp;
            tmp[00] = 1f; tmp[01] = sy; tmp[02] = 0f; tmp[03] = 0f;
            tmp[04] = sy; tmp[05] = 1f; tmp[06] = 0f; tmp[07] = 0f;
            tmp[08] = 0f; tmp[09] = 0f; tmp[10] = 1f; tmp[11] = 0f;
            tmp[12] = 0f; tmp[13] = 0f; tmp[14] = 0f; tmp[15] = 1f;

            MultiplyInternal(tmp);
        }
    }

    public void SkewX(float sx) {
        Skew(sx, 0f);
    }

    public void SkewY(float sy) {
        Skew(0f, sy);
    }

    public void Translate(float tx, float ty) {
        if (tx == 0f && ty == 0f) return;

        unsafe {
            float* tmp = this.matrix_temp;
            tmp[00] = 1f; tmp[01] = 0f; tmp[02] = 0f; tmp[03] = 0f;
            tmp[04] = 0f; tmp[05] = 1f; tmp[06] = 0f; tmp[07] = 0f;
            tmp[08] = 0f; tmp[09] = 0f; tmp[10] = 1f; tmp[11] = 0f;
            tmp[12] = tx; tmp[13] = ty; tmp[14] = 0f; tmp[15] = 1f;

            MultiplyInternal(tmp);
        }
    }

    public void TranslateX(float tx) {
        Translate(tx, 0f);
    }

    public void TranslateY(float ty) {
        Translate(0f, ty);
    }


    public void MultiplyWithMatrix(SIMDMatrix matrix2) {
        unsafe {
            MultiplyInternal(matrix2.matrix);
        }
    }

    public void ApplyModifier(Modifier modifier) {
        ApplyModifier2(modifier, modifier.x, modifier.y, modifier.width, modifier.height);
    }

    public void ApplyModifier2(Modifier modifier, float draw_x, float draw_y, float draw_width, float draw_height) {
        bool valid_size = draw_width > 0f && draw_height > 0f;

        if (modifier.scale_direction_x != 0f && modifier.scale_x != 1f && valid_size) {
            float t = InternalCalcScaleOffset(draw_width, modifier.scale_x, modifier.scale_direction_x);
            TranslateX(t);
        }
        if (modifier.scale_direction_y != 0f && modifier.scale_y != 1f && valid_size) {
            float t = InternalCalcScaleOffset(draw_height, modifier.scale_y, modifier.scale_direction_y);
            TranslateY(t);
        }

        float tx = modifier.translate_x;
        float ty = modifier.translate_y;
        if (modifier.scale_translation == true) {
            // ¿should be absolute?
            tx *= Math.Abs(modifier.scale_x);
            ty *= Math.Abs(modifier.scale_y);
        }

        // combo: matrix translation + matrix rotation (with or without polygon u/v)
        bool translate = tx != 0f || ty != 0f;
        if (modifier.rotate != 0f) {
            /*if (!translate) {
                internal_rotate(floatdraw_x, draw_y, draw_width, draw_height, modifier);
            } else */
            if (modifier.translate_rotation == true) {
                InternalRotate(draw_x, draw_y, draw_width, draw_height, modifier);
                if (translate) Translate(tx, ty);
            } else {
                if (translate) Translate(tx, ty);
                InternalRotate(draw_x, draw_y, draw_width, draw_height, modifier);
            }
        } else if (translate) {
            Translate(tx, ty);
        }


        if (modifier.skew_x != 0f || modifier.skew_y != 0f)
            Skew(modifier.skew_x, modifier.skew_y);

        if (modifier.scale_x != 1f || modifier.scale_y != 1f) {
            if (modifier.scale_size == true)
                ScaleSize(draw_x, draw_y, modifier.scale_x, modifier.scale_y);
            else
                Scale(modifier.scale_x, modifier.scale_y);
        }
    }

    public void MultiplyPoint(ref float target_2d_x, ref float target_2d_y) {
        unsafe {
            float* tmp = this.matrix_temp;

            tmp[0] = target_2d_x;
            tmp[1] = target_2d_y;
            tmp[2] = 0f;// z
            tmp[3] = 1f;// w

            MultiplyInternalVector(tmp);

            target_2d_x = tmp[0];
            target_2d_y = tmp[1];
        }
    }


    public bool IsIdentity() {
        unsafe {
            return Math2D.FloatsAreNearEqual(this.matrix[0], 1f) &&
                Math2D.FloatsAreNearEqual(this.matrix[1], 0f) &&
                Math2D.FloatsAreNearEqual(this.matrix[2], 0f) &&
                Math2D.FloatsAreNearEqual(this.matrix[3], 0f) &&
                Math2D.FloatsAreNearEqual(this.matrix[4], 0f) &&
                Math2D.FloatsAreNearEqual(this.matrix[5], 1f) &&
                Math2D.FloatsAreNearEqual(this.matrix[6], 0f) &&
                Math2D.FloatsAreNearEqual(this.matrix[7], 0f) &&
                Math2D.FloatsAreNearEqual(this.matrix[8], 0f) &&
                Math2D.FloatsAreNearEqual(this.matrix[9], 0f) &&
                Math2D.FloatsAreNearEqual(this.matrix[10], 1f) &&
                Math2D.FloatsAreNearEqual(this.matrix[11], 0f) &&
                Math2D.FloatsAreNearEqual(this.matrix[12], 0f) &&
                Math2D.FloatsAreNearEqual(this.matrix[13], 0f) &&
                Math2D.FloatsAreNearEqual(this.matrix[14], 0f) &&
                Math2D.FloatsAreNearEqual(this.matrix[15], 1f);
        }
    }

    public bool IsEqual(SIMDMatrix matrix2) {
        if (this == matrix2) return true;
        unsafe {
            for (int i = 0 ; i < LENGTH ; i++) {
                if (!Math2D.FloatsAreNearEqual(this.matrix[i], matrix2.matrix[i])) return false;
            }
        }
        return true;
    }

    public void Clear() {
        unsafe {
            this.matrix[0] = 1f; this.matrix[1] = 0f; this.matrix[2] = 0f; this.matrix[3] = 0f;
            this.matrix[4] = 0f; this.matrix[5] = 1f; this.matrix[6] = 0f; this.matrix[7] = 0f;
            this.matrix[8] = 0f; this.matrix[9] = 0f; this.matrix[10] = 1f; this.matrix[11] = 0f;
            this.matrix[12] = 0f; this.matrix[13] = 0f; this.matrix[14] = 0f; this.matrix[15] = 1f;
        }
    }

    public void SetTransform(float a, float b, float c, float d, float e, float f, float g, float h, float i, float j, float k, float l, float m, float n, float ñ, float o) {
        unsafe {
            this.matrix[0] = a; this.matrix[1] = b; this.matrix[2] = c; this.matrix[3] = d;
            this.matrix[4] = e; this.matrix[5] = f; this.matrix[6] = g; this.matrix[7] = h;
            this.matrix[8] = i; this.matrix[9] = j; this.matrix[10] = k; this.matrix[11] = l;
            this.matrix[12] = m; this.matrix[13] = n; this.matrix[14] = ñ; this.matrix[15] = o;
        }
    }

    public void CopyTo(SIMDMatrix dest) {
        unsafe {
            Buffer.MemoryCopy(this.matrix, dest.matrix, BYTE_SIZE, BYTE_SIZE);
        }
    }

    public void CornerRotate(CornerRotation matrix_corner, float x, float y, float width, float height) {
        Translate(x, y);
        RotateByRads(matrix_corner.angle);
        Translate(-x, -y);
        Translate(matrix_corner.x * width, matrix_corner.y * height);
    }

    public void ScaleSize(float draw_x, float draw_y, float scale_x, float scale_y) {
        Translate(draw_x - (draw_x * scale_x), draw_y - (draw_y * scale_y));
        Scale(scale_x, scale_y);
    }

    public float this[int index] {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
#if DEBUG
            if (index < 0 || index > 16) throw new ArgumentOutOfRangeException();
#endif
            unsafe {
                return matrix[index];
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        set
        {
#if DEBUG
            if (index < 0 || index > 16) throw new ArgumentOutOfRangeException();
#endif
            unsafe {
                matrix[index] = value;
            }
        }
    }

    public void GetCopy(float[] dest) {
        unsafe {
            Marshal.Copy((nint)this.matrix, dest, 0, this.Length);
        }
    }

    public unsafe float* Pointer {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get => this.matrix;
    }

    public override string ToString() {
        unsafe {
            StringBuilder sb = new StringBuilder(256);

            sb.Append('{');
            for (int i = 0 ; i < LENGTH ; i++) {
                if (i > 0) Console.Write(", ");
                sb.Append(this.matrix[i]);
            }
            sb.Append('}');

            return sb.ToString();
        }
    }



    private void InternalRotate(float draw_x, float draw_y, float draw_width, float draw_height, Modifier modifier) {
        if (modifier.rotate == 0) return;

        if (!modifier.rotate_pivot_enabled == true || draw_width < 0f || draw_height < 0f) {
            RotateByRads(modifier.rotate);
            return;
        }

        // the pivot is calculated from inside of the polygon
        float x = draw_x + (modifier.rotate_pivot_u * draw_width);
        float y = draw_y + (modifier.rotate_pivot_v * draw_height);
        Translate(x, y);
        RotateByRads(modifier.rotate);
        Translate(-x, -y);
    }

    private static float InternalCalcScaleOffset(float dimmen, float scale_dimmen, float offset_dimmen) {
        float sign = Math.Sign(scale_dimmen);
        float offset = dimmen * ((float)Math.Abs(scale_dimmen) - 1.0f);
        float translate = offset * sign * offset_dimmen;

        return translate;
    }

    private unsafe void MultiplyInternal(float* mat) {
        float* result = this.matrix;
        float* mat2 = mat;
        float* mat1 = this.matrix;

        Vector128<float> mat2_row0 = Sse.LoadAlignedVector128(mat2 + ROW0);
        Vector128<float> mat2_row1 = Sse.LoadAlignedVector128(mat2 + ROW1);
        Vector128<float> mat2_row2 = Sse.LoadAlignedVector128(mat2 + ROW2);
        Vector128<float> mat2_row3 = Sse.LoadAlignedVector128(mat2 + ROW3);

        Vector128<float> result_row0 = Sse.Multiply(mat2_row0, Vector128.Create(mat1[0 + ROW0]));
        result_row0 = Sse.Add(result_row0, Sse.Multiply(mat2_row1, Vector128.Create(mat1[1 + ROW0])));
        result_row0 = Sse.Add(result_row0, Sse.Multiply(mat2_row2, Vector128.Create(mat1[2 + ROW0])));
        result_row0 = Sse.Add(result_row0, Sse.Multiply(mat2_row3, Vector128.Create(mat1[3 + ROW0])));

        Vector128<float> result_row1 = Sse.Multiply(mat2_row0, Vector128.Create(mat1[0 + ROW1]));
        result_row1 = Sse.Add(result_row1, Sse.Multiply(mat2_row1, Vector128.Create(mat1[1 + ROW1])));
        result_row1 = Sse.Add(result_row1, Sse.Multiply(mat2_row2, Vector128.Create(mat1[2 + ROW1])));
        result_row1 = Sse.Add(result_row1, Sse.Multiply(mat2_row3, Vector128.Create(mat1[3 + ROW1])));

        Vector128<float> result_row2 = Sse.Multiply(mat2_row0, Vector128.Create(mat1[0 + ROW2]));
        result_row2 = Sse.Add(result_row2, Sse.Multiply(mat2_row1, Vector128.Create(mat1[1 + ROW2])));
        result_row2 = Sse.Add(result_row2, Sse.Multiply(mat2_row2, Vector128.Create(mat1[2 + ROW2])));
        result_row2 = Sse.Add(result_row2, Sse.Multiply(mat2_row3, Vector128.Create(mat1[3 + ROW2])));

        Vector128<float> result_row3 = Sse.Multiply(mat2_row0, Vector128.Create(mat1[0 + ROW3]));
        result_row3 = Sse.Add(result_row3, Sse.Multiply(mat2_row1, Vector128.Create(mat1[1 + ROW3])));
        result_row3 = Sse.Add(result_row3, Sse.Multiply(mat2_row2, Vector128.Create(mat1[2 + ROW3])));
        result_row3 = Sse.Add(result_row3, Sse.Multiply(mat2_row3, Vector128.Create(mat1[3 + ROW3])));

        Sse.StoreAligned(result + ROW0, result_row0);
        Sse.StoreAligned(result + ROW1, result_row1);
        Sse.StoreAligned(result + ROW2, result_row2);
        Sse.StoreAligned(result + ROW3, result_row3);
    }

    private unsafe void MultiplyInternalVector(float* vector) {
        Vector128<float> x = Vector128.Create(vector[0]);
        Vector128<float> y = Vector128.Create(vector[1]);
        Vector128<float> z = Vector128.Create(vector[2]);
        Vector128<float> w = Vector128.Create(vector[3]);

        Vector128<float> mat2_row0 = Sse.LoadAlignedVector128(this.matrix + ROW0);
        Vector128<float> mat2_row1 = Sse.LoadAlignedVector128(this.matrix + ROW1);
        Vector128<float> mat2_row2 = Sse.LoadAlignedVector128(this.matrix + ROW2);
        Vector128<float> mat2_row3 = Sse.LoadAlignedVector128(this.matrix + ROW3);

        Vector128<float> p1 = Sse.Multiply(x, mat2_row0);
        Vector128<float> p2 = Sse.Multiply(y, mat2_row1);
        Vector128<float> p3 = Sse.Multiply(z, mat2_row2);
        Vector128<float> p4 = Sse.Multiply(w, mat2_row3);

        Vector128<float> r1 = Sse.Add(p1, p2);
        Vector128<float> r2 = Sse.Add(p3, p4);
        Vector128<float> res = Sse.Add(r1, r2);

        Sse.StoreAligned(vector, res);
    }

}


