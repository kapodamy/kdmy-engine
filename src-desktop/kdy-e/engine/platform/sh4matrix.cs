using System;
using Engine.Utils;

namespace Engine.Platform {

    public enum Corner {
        TOPLEFT,
        TOPRIGHT,
        BOTTOMLEFT,
        BOTTOMRIGHT
    }

    public struct SH4MatrixCorner {
        public float angle;
        public float x;
        public float y;

        public void HelperCalculateCornerRotation(Corner corner) {
            switch (corner) {
                default:
                case Corner.TOPLEFT:
                    angle = 0;
                    x = 0;
                    y = 0;
                    break;
                case Corner.TOPRIGHT:
                    angle = -90 * Math2D.DEG_TO_RAD;
                    x = -1;
                    y = 0;
                    break;
                case Corner.BOTTOMLEFT:
                    angle = 90 * Math2D.DEG_TO_RAD;
                    x = 0;
                    y = -1;
                    break;
                case Corner.BOTTOMRIGHT:
                    angle = 180 * Math2D.DEG_TO_RAD;
                    x = -1;
                    y = -1;
                    break;
            }
        }

        internal void Clear() {
            angle = x = y = 0;
        }
    }

    //
    // Matrix computation using Hitachi SH-4 CPU dedicated instructions
    //
    public class SH4Matrix {

        public const byte SIZE = 16;
        public readonly float[] matrix;

        public SH4Matrix() {
            matrix = new float[SIZE];
            this.matrix[0] = this.matrix[5] = this.matrix[10] = this.matrix[15] = 1;
        }

        public void Destroy() {
            //free(matrix);
        }


        public void FlipX() {
            this.ScaleX(-1);
        }

        public void FlipY() {
            this.ScaleY(-1);
        }


        public void RotateByDegs(float angle) {
            RotateByRads(angle * Math2D.DEG_TO_RAD);
        }

        public void RotateByRads(float angle) {
            if (angle == 0) return;

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

        public void Scale(float sx, float sy) {
            if (sx == 1 && sy == 1) return;
            Multiply(sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
        }

        public void ScaleX(float sx) {
            Scale(sx, 1);
        }

        public void ScaleY(float sy) {
            Scale(1, sy);
        }

        public void Skew(float sx, float sy) {
            if (sx == 0 && sy == 0) return;
            Multiply(1, sy, 0, 0, sx, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
        }

        public void SkewX(float sx) {
            Skew(sx, 0);
        }

        public void SkewY(float sy) {
            Skew(0, sy);
        }

        public void Translate(float tx, float ty) {
            if (tx == 0 && ty == 0) return;
            Multiply(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, tx, ty, 0, 1);
        }

        public void TranslateX(float tx) {
            Translate(tx, 0);
        }

        public void TranslateY(float ty) {
            Translate(0, ty);
        }


        public void MultiplyWithMatrix(SH4Matrix matrix2) {
            Multiply(
                matrix2.matrix[0], matrix2.matrix[1], matrix2.matrix[2], matrix2.matrix[3],
                matrix2.matrix[4], matrix2.matrix[5], matrix2.matrix[6], matrix2.matrix[7],
                matrix2.matrix[8], matrix2.matrix[9], matrix2.matrix[10], matrix2.matrix[11],
                matrix2.matrix[12], matrix2.matrix[13], matrix2.matrix[14], matrix2.matrix[15]
            );
        }

        public void Multiply(float a, float b, float c, float d, float e, float f, float g, float h, float i, float j, float k, float l, float m, float n, float ñ, float o) {
            float a1 = this.matrix[0]; float b1 = this.matrix[1]; float c1 = this.matrix[2]; float d1 = this.matrix[3];
            float e1 = this.matrix[4]; float f1 = this.matrix[5]; float g1 = this.matrix[6]; float h1 = this.matrix[7];
            float i1 = this.matrix[8]; float j1 = this.matrix[9]; float k1 = this.matrix[10]; float l1 = this.matrix[11];
            float m1 = this.matrix[12]; float n1 = this.matrix[13]; float ñ1 = this.matrix[14]; float o1 = this.matrix[15];

            this.matrix[0] = a * a1 + b * e1 + c * i1 + d * m1;
            this.matrix[1] = a * b1 + b * f1 + c * j1 + d * n1;
            this.matrix[2] = a * c1 + b * g1 + c * k1 + d * ñ1;
            this.matrix[3] = a * d1 + b * h1 + c * l1 + d * o1;

            this.matrix[4] = e * a1 + f * e1 + g * i1 + h * m1;
            this.matrix[5] = e * b1 + f * f1 + g * j1 + h * n1;
            this.matrix[6] = e * c1 + f * g1 + g * k1 + h * ñ1;
            this.matrix[7] = e * d1 + f * h1 + g * l1 + h * o1;

            this.matrix[8] = i * a1 + j * e1 + k * i1 + l * m1;
            this.matrix[9] = i * b1 + j * f1 + k * j1 + l * n1;
            this.matrix[10] = i * c1 + j * g1 + k * k1 + l * ñ1;
            this.matrix[11] = i * d1 + j * h1 + k * l1 + l * o1;

            this.matrix[12] = m * a1 + n * e1 + ñ * i1 + o * m1;
            this.matrix[13] = m * b1 + n * f1 + ñ * j1 + o * n1;
            this.matrix[14] = m * c1 + n * g1 + ñ * k1 + o * ñ1;
            this.matrix[15] = m * d1 + n * h1 + ñ * l1 + o * o1;
        }

        public void ApplyModifier(Modifier modifier) {
            ApplyModifier2(modifier, modifier.x, modifier.y, modifier.width, modifier.height);
        }

        public void ApplyModifier2(Modifier modifier, float draw_x, float draw_y, float draw_width, float draw_height) {
            bool valid_size = draw_width > 0 && draw_height > 0;

            if (modifier.scale_direction_x != 0 && modifier.scale_x != 1.0 && valid_size) {
                float t = InternalCalcScaleOffset(draw_width, modifier.scale_x, modifier.scale_direction_x);
                TranslateX(t);
            }
            if (modifier.scale_direction_y != 0 && modifier.scale_y != 1.0 && valid_size) {
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
            bool translate = tx != 0 || ty != 0;
            if (modifier.rotate != 0) {
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


            if (modifier.skew_x != 0 || modifier.skew_y != 0)
                Skew(modifier.skew_x, modifier.skew_y);

            if (modifier.scale_x != 1.0 || modifier.scale_y != 1.0) {
                if (modifier.scale_size == true)
                    ScaleSize(draw_x, draw_y, modifier.scale_x, modifier.scale_y);
                else
                    Scale(modifier.scale_x, modifier.scale_y);
            }
        }


        public bool IsIdentity() {
            return Math2D.FloatsAreNearEqual(matrix[0], 1) &&
                Math2D.FloatsAreNearEqual(matrix[1], 0) &&
                Math2D.FloatsAreNearEqual(matrix[2], 0) &&
                Math2D.FloatsAreNearEqual(matrix[3], 0) &&
                Math2D.FloatsAreNearEqual(matrix[4], 0) &&
                Math2D.FloatsAreNearEqual(matrix[5], 1) &&
                Math2D.FloatsAreNearEqual(matrix[6], 0) &&
                Math2D.FloatsAreNearEqual(matrix[7], 0) &&
                Math2D.FloatsAreNearEqual(matrix[8], 0) &&
                Math2D.FloatsAreNearEqual(matrix[9], 0) &&
                Math2D.FloatsAreNearEqual(matrix[10], 1) &&
                Math2D.FloatsAreNearEqual(matrix[11], 0) &&
                Math2D.FloatsAreNearEqual(matrix[12], 0) &&
                Math2D.FloatsAreNearEqual(matrix[13], 0) &&
                Math2D.FloatsAreNearEqual(matrix[14], 0) &&
                Math2D.FloatsAreNearEqual(matrix[15], 1);
        }

        public bool IsEqual(SH4Matrix matrix2) {
            if (this == matrix2) return true;
            for (int i = 0 ; i < SIZE ; i++) {
                if (!Math2D.FloatsAreNearEqual(matrix[i], matrix2.matrix[i])) return false;
            }
            return true;
        }

        public void Clear() {
            this.matrix[0] = 1; this.matrix[1] = 0; this.matrix[2] = 0; this.matrix[3] = 0;
            this.matrix[4] = 0; this.matrix[5] = 1; this.matrix[6] = 0; this.matrix[7] = 0;
            this.matrix[8] = 0; this.matrix[9] = 0; this.matrix[10] = 1; this.matrix[11] = 0;
            this.matrix[12] = 0; this.matrix[13] = 0; this.matrix[14] = 0; this.matrix[15] = 1;
        }

        public void SetTransform(float a, float b, float c, float d, float e, float f, float g, float h, float i, float j, float k, float l, float m, float n, float ñ, float o) {
            this.matrix[0] = a; this.matrix[1] = b; this.matrix[2] = c; this.matrix[3] = d;
            this.matrix[4] = e; this.matrix[5] = f; this.matrix[6] = g; this.matrix[7] = h;
            this.matrix[8] = i; this.matrix[9] = j; this.matrix[10] = k; this.matrix[11] = l;
            this.matrix[12] = m; this.matrix[13] = n; this.matrix[14] = ñ; this.matrix[15] = o;
        }

        public void CopyTo(SH4Matrix dest) {
            // Important: DO NOT USE "System.Buffer.BlockCopy()" ¡does not work! probably is bugged 
            for (int i = 0 ; i < SH4Matrix.SIZE ; i++) dest.matrix[i] = this.matrix[i];
        }

        public void CornerRotate(SH4MatrixCorner matrix_corner, float x, float y, float width, float height) {
            Translate(x, y);
            RotateByRads(matrix_corner.angle);
            Translate(-x, -y);
            Translate(matrix_corner.x * width, matrix_corner.y * height);
        }

        public void ScaleSize(float draw_x, float draw_y, float scale_x, float scale_y) {
            Translate(draw_x - (draw_x * scale_x), draw_y - (draw_y * scale_y));
            Scale(scale_x, scale_y);
        }



        public void InternalRotate(float draw_x, float draw_y, float draw_width, float draw_height, Modifier modifier) {
            if (modifier.rotate == 0) return;

            if (!modifier.rotate_pivot_enabled == true || draw_width < 0 || draw_height < 0) {
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

        public static float InternalCalcScaleOffset(float dimmen, float scale_dimmen, float offset_dimmen) {
            float sign = Math.Sign(scale_dimmen);
            float offset = dimmen * ((float)Math.Abs(scale_dimmen) - 1.0f);
            float translate = offset * sign * offset_dimmen;

            return translate;
        }

    }
}