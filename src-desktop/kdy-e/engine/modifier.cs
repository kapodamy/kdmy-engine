using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Engine.Utils;

namespace Engine {

    //
    // High-Level matrix manipulation
    //
    public class Modifier : ISetProperty {
        public float translate_x;
        public float translate_y;

        public float rotate;

        public float skew_x;
        public float skew_y;

        public float scale_x;
        public float scale_y;

        public float scale_direction_x;
        public float scale_direction_y;

        public bool? rotate_pivot_enabled;
        public float rotate_pivot_u;
        public float rotate_pivot_v;

        public bool? translate_rotation;
        public bool? scale_size;
        public bool? scale_translation;

        public float x;
        public float y;
        public float width;
        public float height;

        public Modifier() {
            Clear();
        }


        public void Clear() {
            this.translate_x = 0.0f;
            this.translate_y = 0.0f;

            this.rotate = 0.0f;

            this.skew_x = 0.0f;
            this.skew_y = 0.0f;

            this.scale_x = 1.0f;
            this.scale_y = 1.0f;

            this.scale_direction_x = 0.0f;
            this.scale_direction_y = 0.0f;

            this.rotate_pivot_enabled = false;
            this.rotate_pivot_u = 0.5f;
            this.rotate_pivot_v = 0.5f;

            this.translate_rotation = false;
            this.scale_size = false;
            this.scale_translation = false;

            this.x = 0.0f;
            this.y = 0.0f;
            this.width = -1.0f;
            this.height = -1.0f;
        }

        public void Invalidate() {
            this.translate_x = Single.NaN;
            this.translate_y = Single.NaN;

            this.rotate = Single.NaN;

            this.skew_x = Single.NaN;
            this.skew_y = Single.NaN;

            this.scale_x = Single.NaN;
            this.scale_y = Single.NaN;

            this.scale_direction_x = Single.NaN;
            this.scale_direction_y = Single.NaN;

            this.rotate_pivot_enabled = null;
            this.rotate_pivot_u = Single.NaN;
            this.rotate_pivot_v = Single.NaN;

            this.translate_rotation = null;
            this.scale_size = null;
            this.scale_translation = null;

            this.x = Single.NaN;
            this.y = Single.NaN;
            this.width = Single.NaN;
            this.height = Single.NaN;
        }


        public void SetProperty(int property, float value) {
            switch (property) {
                case VertexProps.SPRITE_PROP_ROTATE:
                    this.rotate = value * Math2D.DEG_TO_RAD;
                    break;
                case VertexProps.SPRITE_PROP_SCALE_X:
                    this.scale_x = value;
                    break;
                case VertexProps.SPRITE_PROP_SCALE_Y:
                    this.scale_y = value;
                    break;
                case VertexProps.SPRITE_PROP_TRANSLATE_X:
                    this.translate_x = value;
                    break;
                case VertexProps.SPRITE_PROP_TRANSLATE_Y:
                    this.translate_y = value;
                    break;
                case VertexProps.SPRITE_PROP_SKEW_X:
                    this.skew_x = value;
                    break;
                case VertexProps.SPRITE_PROP_SKEW_Y:
                    this.skew_y = value;
                    break;
                case VertexProps.SPRITE_PROP_ROTATE_PIVOT_ENABLE:
                    this.rotate_pivot_enabled = value >= 1.0;
                    break;
                case VertexProps.SPRITE_PROP_ROTATE_PIVOT_U:
                    this.rotate_pivot_u = value;
                    break;
                case VertexProps.SPRITE_PROP_ROTATE_PIVOT_V:
                    this.rotate_pivot_v = value;
                    break;
                case VertexProps.SPRITE_PROP_SCALE_DIRECTION_X:
                    this.scale_direction_x = value;
                    break;
                case VertexProps.SPRITE_PROP_SCALE_DIRECTION_Y:
                    this.scale_direction_y = value;
                    break;
                case VertexProps.SPRITE_PROP_TRANSLATE_ROTATION:
                    this.translate_rotation = value >= 1.0;
                    break;
                case VertexProps.SPRITE_PROP_SCALE_SIZE:
                    this.scale_size = value >= 1.0;
                    break;
                case VertexProps.SPRITE_PROP_SCALE_TRANSLATION:
                    this.scale_translation = value >= 1.0;
                    break;
            }
        }

        public void CopyTo(Modifier dest_modifier) {
            if (!Single.IsNaN(this.translate_x)) dest_modifier.translate_x = this.translate_x;
            if (!Single.IsNaN(this.translate_y)) dest_modifier.translate_y = this.translate_y;

            if (!Single.IsNaN(this.rotate)) dest_modifier.rotate = this.rotate;

            if (!Single.IsNaN(this.skew_x)) dest_modifier.skew_x = this.skew_x;
            if (!Single.IsNaN(this.skew_y)) dest_modifier.skew_y = this.skew_y;

            if (!Single.IsNaN(this.scale_x)) dest_modifier.scale_x = this.scale_x;
            if (!Single.IsNaN(this.scale_y)) dest_modifier.scale_y = this.scale_y;

            if (!Single.IsNaN(this.scale_direction_x)) dest_modifier.scale_direction_x = this.scale_direction_x;
            if (!Single.IsNaN(this.scale_direction_y)) dest_modifier.scale_direction_y = this.scale_direction_y;

            if (this.rotate_pivot_enabled != null) dest_modifier.rotate_pivot_enabled = this.rotate_pivot_enabled;
            if (!Single.IsNaN(this.rotate_pivot_u)) dest_modifier.rotate_pivot_u = this.rotate_pivot_u;
            if (!Single.IsNaN(this.rotate_pivot_v)) dest_modifier.rotate_pivot_v = this.rotate_pivot_v;

            if (this.translate_rotation != null) dest_modifier.translate_rotation = this.translate_rotation;
            if (this.scale_size != null) dest_modifier.scale_size = this.scale_size;
            if (this.scale_translation != null) dest_modifier.scale_translation = this.scale_translation;

            if (!Single.IsNaN(this.x)) dest_modifier.x = this.x;
            if (!Single.IsNaN(this.y)) dest_modifier.y = this.y;
            if (!Single.IsNaN(this.width)) dest_modifier.width = this.width;
            if (!Single.IsNaN(this.height)) dest_modifier.height = this.height;
        }

    };

}
