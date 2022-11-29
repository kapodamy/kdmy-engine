using System;
using Engine.Platform;
using Engine.Utils;

namespace Engine {

    public delegate void DelegateDraw(PVRContext pvrctx);
    public delegate int DelegateAnimate(float elapsed);

    public class Drawable : IVertex {

        private static Map<Drawable> POOL = new Map<Drawable>();
        private static int IDS = 0;


        private int id;
        private float alpha;
        private float[] offsetcolor;
        private Modifier modifier;
        private float z_index;
        private float z_offset;
        private DelegateDraw callback_draw;
        private DelegateAnimate callback_animate;
        private bool visible;
        private PVRContextFlag antialiasing;
        private PSShader psshader;
        private bool blend_enabled;
        private Blend blend_src_rgb;
        private Blend blend_dst_rgb;
        private Blend blend_src_alpha;
        private Blend blend_dst_alpha;

        public Drawable(float z, IDraw draw_impl, IAnimate animate_impl) :
            this(z, default(DelegateDraw), default(DelegateAnimate)) {
            if (draw_impl == null)
                this.callback_draw = null;
            else
                this.callback_draw = draw_impl.Draw;

            if (animate_impl == null)
                this.callback_animate = null;
            else
                this.callback_animate = animate_impl.Animate;
        }

        public Drawable(float z, DelegateDraw callback_draw, DelegateAnimate callback_animate) {
            this.id = Drawable.IDS++;

            this.alpha = 1.0f;
            this.offsetcolor = new float[4];
            this.modifier = new Modifier();

            this.z_index = z;
            this.z_offset = 0;

            this.callback_draw = callback_draw; this.callback_animate = callback_animate;

            this.visible = true;

            this.antialiasing = PVRContextFlag.DEFAULT;
            this.psshader = null;

            this.blend_enabled = true;
            this.blend_src_rgb = Blend.DEFAULT;
            this.blend_dst_rgb = Blend.DEFAULT;
            this.blend_src_alpha = Blend.DEFAULT;
            this.blend_dst_alpha = Blend.DEFAULT;

            PVRContext.HelperClearOffsetColor(this.offsetcolor);
            this.modifier.Clear();

            Drawable.POOL.Set(this.id, this);
        }

        public void Destroy() {
            Drawable.POOL.Delete(this.id);

            this.callback_draw = null;
            this.callback_animate = null;


            //free(drawable);
        }


        public void SetZIndex(float z_index) {
            this.z_index = z_index;
        }

        public float GetZIndex() {
            return this.z_index + this.z_offset;
        }

        public void SetZOffset(float offset) {
            this.z_offset = offset;
        }


        public void SetAlpha(float alpha) {
            this.alpha = alpha;
        }

        public float GetAlpha() {
            return this.alpha;
        }

        public void SetOffsetColor(float r, float g, float b, float a) {
            if (!Single.IsNaN(r)) this.offsetcolor[0] = r;
            if (!Single.IsNaN(g)) this.offsetcolor[1] = g;
            if (!Single.IsNaN(b)) this.offsetcolor[2] = b;
            if (!Single.IsNaN(a)) this.offsetcolor[3] = a;
        }

        public void SetOffsetColorToDefault() {
            PVRContext.HelperClearOffsetColor(this.offsetcolor);
        }

        public void SetVisible(bool visible) {
            this.visible = visible;
        }

        public Modifier GetModifier() {
            return this.modifier;
        }


        public void Draw(PVRContext pvrctx) {
            this.callback_draw(pvrctx);
        }

        public int Animate(float elapsed) {
            if (this.callback_animate != null)
                return this.callback_animate(elapsed);
            else
                return 0;
        }

        public void SetProperty(int property_id, float value) {
            switch (property_id) {
                case VertexProps.SPRITE_PROP_X:
                    this.modifier.x = value;
                    break;
                case VertexProps.SPRITE_PROP_Y:
                    this.modifier.y = value;
                    break;
                case VertexProps.SPRITE_PROP_WIDTH:
                    this.modifier.width = value;
                    break;
                case VertexProps.SPRITE_PROP_HEIGHT:
                    this.modifier.height = value;
                    break;
                case VertexProps.SPRITE_PROP_ROTATE:
                    this.modifier.rotate = value * Math2D.DEG_TO_RAD;
                    break;
                case VertexProps.SPRITE_PROP_SCALE_X:
                    this.modifier.scale_x = value;
                    break;
                case VertexProps.SPRITE_PROP_SCALE_Y:
                    this.modifier.scale_y = value;
                    break;
                case VertexProps.SPRITE_PROP_SKEW_X:
                    this.modifier.skew_x = value;
                    break;
                case VertexProps.SPRITE_PROP_SKEW_Y:
                    this.modifier.skew_y = value;
                    break;
                case VertexProps.SPRITE_PROP_TRANSLATE_X:
                    this.modifier.translate_x = value;
                    break;
                case VertexProps.SPRITE_PROP_TRANSLATE_Y:
                    this.modifier.translate_y = value;
                    break;
                case VertexProps.SPRITE_PROP_ALPHA:
                    this.alpha = Math2D.Clamp(value, 0f, 1.0f);
                    break;
                case VertexProps.SPRITE_PROP_Z:
                    this.z_index = value;
                    break;
                case VertexProps.SPRITE_PROP_VERTEX_COLOR_R:
                case VertexProps.SPRITE_PROP_VERTEX_COLOR_G:
                case VertexProps.SPRITE_PROP_VERTEX_COLOR_B:
                    break;
                case VertexProps.SPRITE_PROP_VERTEX_COLOR_OFFSET_R:
                    this.offsetcolor[0] = value;
                    break;
                case VertexProps.SPRITE_PROP_VERTEX_COLOR_OFFSET_G:
                    this.offsetcolor[1] = value;
                    break;
                case VertexProps.SPRITE_PROP_VERTEX_COLOR_OFFSET_B:
                    this.offsetcolor[2] = value;
                    break;
                case VertexProps.SPRITE_PROP_VERTEX_COLOR_OFFSET_A:
                    this.offsetcolor[3] = value;
                    break;
                case VertexProps.SPRITE_PROP_ANIMATIONLOOP:
                    break;
                case VertexProps.SPRITE_PROP_ROTATE_PIVOT_ENABLE:
                    this.modifier.rotate_pivot_enabled = value >= 1.0f;
                    break;
                case VertexProps.SPRITE_PROP_ROTATE_PIVOT_U:
                    this.modifier.rotate_pivot_u = value;
                    break;
                case VertexProps.SPRITE_PROP_ROTATE_PIVOT_V:
                    this.modifier.rotate_pivot_v = value;
                    break;
                case VertexProps.SPRITE_PROP_SCALE_DIRECTION_X:
                    this.modifier.scale_direction_x = value;
                    break;
                case VertexProps.SPRITE_PROP_SCALE_DIRECTION_Y:
                    this.modifier.scale_direction_y = value;
                    break;
                case VertexProps.SPRITE_PROP_TRANSLATE_ROTATION:
                    this.modifier.translate_rotation = value >= 1.0f;
                    break;
                case VertexProps.SPRITE_PROP_SCALE_SIZE:
                    this.modifier.scale_size = value >= 1.0f;
                    break;
                case VertexProps.SPRITE_PROP_SCALE_TRANSLATION:
                    this.modifier.scale_translation = value >= 1.0f;
                    break;
                case VertexProps.SPRITE_PROP_Z_OFFSET:
                    this.z_offset = value;
                    break;
                case VertexProps.SPRITE_PROP_ANTIALIASING:
                    this.antialiasing = (PVRContextFlag)((int)Math.Truncate(value));
                    break;
            }
        }

        public void HelperApplyInContext(PVRContext pvrctx) {
            pvrctx.ApplyModifier(this.modifier);
            pvrctx.SetVertexAlpha(this.alpha);
            pvrctx.SetVertexOffsetColor(this.offsetcolor);
            if (this.antialiasing != PVRContextFlag.DEFAULT) {
                pvrctx.SetGlobalAntialiasing(this.antialiasing);
            }
            if (this.psshader != null) pvrctx.AddShader(this.psshader);
            pvrctx.SetVertexBlend(
                this.blend_enabled,
                this.blend_src_rgb,
                this.blend_dst_rgb,
                this.blend_src_alpha,
                this.blend_dst_alpha
            );
        }

        public void HelperUpdateFromPlaceholder(LayoutPlaceholder layout_placeholder) {
            this.modifier.x = layout_placeholder.x;
            this.modifier.y = layout_placeholder.y;
            this.z_index = layout_placeholder.z;
            this.modifier.width = layout_placeholder.width;
            this.modifier.height = layout_placeholder.height;
            this.z_offset = 0;
        }


        public bool IsVisible() {
            return this.visible;
        }

        public void GetDrawLocation(out float draw_x, out float draw_y) {
            draw_x = this.modifier.x;
            draw_y = this.modifier.y;
        }

        public void SetAntialiasing(PVRContextFlag antialiasing) {
            this.antialiasing = antialiasing;
        }

        public void SetShader(PSShader psshader) {
            this.psshader = psshader;
        }

        public PSShader GetShader() {
            return this.psshader;
        }

        public void GetDrawSize(out float draw_width, out float draw_height) {
            throw new InvalidOperationException("drawable_get_draw_size() is not available");
        }

        public void BlendEnable(bool enabled) {
            this.blend_enabled = enabled;
        }

        public void BlendSet(Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha) {
            this.blend_src_rgb = src_rgb;
            this.blend_dst_rgb = dst_rgb;
            this.blend_src_alpha = src_alpha;
            this.blend_dst_alpha = dst_alpha;
        }

    }
}
