using System;
using Engine.Animation;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Image; 

public class StateSpriteState {
    public Texture texture;
    public AnimSprite animation;
    public int statesprite_id;
    public string state_name;
    public uint vertex_color;
    public AtlasEntry frame_info;
    public CornerRotation corner_rotation;
    public float offset_x;
    public float draw_width;
    public float draw_height;
    public float offset_y;
}

public class StateSprite : IVertex {

    private static int IDS = 0;
    internal static Map<StateSprite> POOL = new Map<StateSprite>();


    private StateSprite() { }


    private Modifier matrix_source;
    private float src_x;
    private float src_y;
    private float[] vertex_color;
    private float[] offsetcolor;
    private Texture texture;
    private float src_width = 0;
    private float src_height = 0;
    private float draw_x;
    private float draw_y;
    private float draw_width;
    private float draw_height;
    private float alpha;
    private float alpha2;
    private bool visible;
    private float z_index;
    private float z_offset;
    private float frame_x;
    private float frame_y;
    private float frame_width;
    private float frame_height;
    private float pivot_x;
    private float pivot_y;
    internal int id;
    private float[] vertex;
    private bool vertex_dirty;
    private AnimSprite animation_external;
    private CornerRotation matrix_corner;
    private SpriteCropInfo crop;
    private bool crop_enabled;
    private bool flip_x;
    private bool flip_y;
    private bool flip_correction;
    private LinkedList<StateSpriteState> state_list;
    private float offset_x;
    private float offset_y;
    private StateSpriteState selected_state;
    private float scale_texture;
    private PVRContextFlag antialiasing;
    private bool atlas_to_draw_size_enabled;
    private PSShader psshader;
    private bool blend_enabled;
    private Blend blend_src_rgb;
    private Blend blend_dst_rgb;
    private Blend blend_src_alpha;
    private Blend blend_dst_alpha;
    private Trail[] trailing_buffer;
    private int trailing_used;
    private int trailing_length;
    private float trailing_alpha;
    private float trailing_delay;
    private float trailing_progress;
    private bool trailing_darken;
    private bool trailing_disabled;
    private float[] trailing_offsetcolor;


    public void Draw(PVRContext pvrctx) {
        if (this.draw_width < 1f || this.draw_height < 1f) return;
        if (this.alpha <= 0f) return;

        float[] sprite_vertex = this.vertex;
        float render_alpha = this.alpha * this.alpha2;

        // apply offsets from the current state
        float draw_x = this.draw_x + this.offset_x;
        float draw_y = this.draw_y + this.offset_y;

        if (this.vertex_dirty) {
            float draw_width = this.draw_width;
            float draw_height = this.draw_height;

            // flip vertex (if required)
            if (this.flip_x) {
                if (this.flip_correction) draw_x += draw_width;
                draw_width = -draw_width;
            }
            if (this.flip_y) {
                if (this.flip_correction) draw_y += draw_height;
                draw_height = -draw_height;
            }

            if (this.texture != null) {
                float frame_width, frame_height;
                float crop_x, crop_y, crop_width, crop_height;
                float ratio_width, ratio_height;

                // complex frame size redimension
                if (this.frame_width > 0f) {
                    ratio_width = draw_width / this.frame_width;
                    frame_width = this.src_width * ratio_width;
                } else {
                    ratio_width = draw_width / this.src_width;
                    frame_width = draw_width;
                }
                if (this.frame_height > 0) {
                    ratio_height = draw_height / this.frame_height;
                    frame_height = this.src_height * ratio_height;
                } else {
                    ratio_height = draw_height / this.src_height;
                    frame_height = draw_height;
                }

                // calculate cropping (if required)
                if (this.crop_enabled) {
                    if (this.crop.x >= frame_width || this.crop.y >= frame_height)
                        return;

                    crop_x = this.crop.x;
                    crop_y = this.crop.y;
                    crop_width = crop_height = 0;

                    if (this.crop.width != -1 && this.crop.width < frame_width)
                        crop_width = frame_width - this.crop.width;

                    if (this.crop.height != -1 && this.crop.height < frame_height)
                        crop_height = frame_height - this.crop.height;
                } else {
                    crop_x = crop_y = crop_width = crop_height = 0;
                }

                // draw location & size
                sprite_vertex[4] = draw_x - ((this.pivot_x + this.frame_x) * ratio_width);
                sprite_vertex[5] = draw_y - ((this.pivot_y + this.frame_y) * ratio_height);
                sprite_vertex[6] = frame_width - crop_width - crop_x;
                sprite_vertex[7] = frame_height - crop_height - crop_y;

                if (this.crop_enabled) {
                    crop_x /= ratio_width;
                    crop_y /= ratio_height;
                    crop_width /= ratio_width;
                    crop_height /= ratio_height;
                }

                // source location & size
                sprite_vertex[0] = this.src_x + crop_x;
                sprite_vertex[1] = this.src_y + crop_y;
                sprite_vertex[2] = this.src_width - crop_width - crop_x;
                sprite_vertex[3] = this.src_height - crop_height - crop_y;

            } else {
                sprite_vertex[4] = draw_x;
                sprite_vertex[5] = draw_y;
                sprite_vertex[6] = draw_width;
                sprite_vertex[7] = draw_height;

                if (this.scale_texture != 1.0f) {
                    sprite_vertex[6] *= this.scale_texture;
                    sprite_vertex[7] *= this.scale_texture;
                }

                if (this.crop_enabled) {
                    float crop_width = this.crop.width;
                    float crop_height = this.crop.height;

                    sprite_vertex[4] += this.crop.x;
                    sprite_vertex[5] += this.crop.y;
                    sprite_vertex[6] -= this.crop.x;
                    sprite_vertex[7] -= this.crop.y;

                    if (crop_width != -1 && crop_width < sprite_vertex[6])
                        sprite_vertex[6] = crop_width;
                    if (crop_height != -1 && crop_height < sprite_vertex[7])
                        sprite_vertex[7] = crop_height;
                }
            }

            // cache the calculated vertex
            this.vertex_dirty = false;
        }

        pvrctx.Save();
        if (this.psshader != null) pvrctx.AddShader(this.psshader);

        pvrctx.SetVertexBlend(
            this.blend_enabled,
            this.blend_src_rgb,
            this.blend_dst_rgb,
            this.blend_src_alpha,
            this.blend_dst_alpha
        );

        // apply self modifier
        pvrctx.ApplyModifier2(
            this.matrix_source,
            draw_x, draw_y, this.draw_width, this.draw_height
        );

        if (this.antialiasing != PVRFlag.DEFAULT) {
            pvrctx.SetVertexAntialiasing(this.antialiasing);
        }

        // do corner rotation (if required)
        if (this.matrix_corner.angle != 0) {
            pvrctx.CurrentMatrix.CornerRotate(
                this.matrix_corner,
                draw_x, draw_y, this.draw_width, this.draw_height
            );
            //pvrctx.Flush();
        }

        // draw sprites trail if necessary
        if (!this.trailing_disabled && this.trailing_used > 0) {
            float[] trailing_offsetcolor = {
                this.trailing_offsetcolor[0], this.trailing_offsetcolor[1], this.trailing_offsetcolor[2], 1f
            };
            if (this.offsetcolor[3] >= 0) {
                trailing_offsetcolor[0] *= this.offsetcolor[0];
                trailing_offsetcolor[1] *= this.offsetcolor[1];
                trailing_offsetcolor[2] *= this.offsetcolor[2];
                trailing_offsetcolor[3] *= this.offsetcolor[3];
            }

            pvrctx.SetVertexOffsetColor(trailing_offsetcolor);
            pvrctx.SetVertexTexturedDarken(this.trailing_darken);

            for (int i = 0 ; i < this.trailing_used ; i++) {
                Trail trail = this.trailing_buffer[i];
                pvrctx.SetVertexAlpha(trail.alpha * render_alpha);

                if (this.texture != null) {
                    pvrctx.DrawTexture(
                        this.texture,
                        trail.sx, trail.sy, trail.sw, trail.sh,
                        trail.dx, trail.dy, trail.dw, trail.dh
                    );
                } else {
                    pvrctx.DrawSolidColor(
                        this.vertex_color,
                        trail.dx, trail.dy, trail.dw, trail.dh
                    );
                }
            }

            // restore previous values
            pvrctx.SetVertexTexturedDarken(false);
        }

        pvrctx.SetVertexAlpha(render_alpha);
        pvrctx.SetVertexOffsetColor(this.offsetcolor);

        // draw the vertex
        if (this.texture != null) {

            // upload texture to the PVR VRAM, if was not loaded yet
            this.texture.UploadToPVR();

            // textured vertex
            pvrctx.DrawTexture(
                this.texture,
                sprite_vertex[0], sprite_vertex[1], // source location
                sprite_vertex[2], sprite_vertex[3], // source size

                sprite_vertex[4], sprite_vertex[5], // draw location
                sprite_vertex[6], sprite_vertex[7] // draw size
            );
        } else {
            // un-textured vertex
            pvrctx.DrawSolidColor(
                this.vertex_color,
                sprite_vertex[4], sprite_vertex[5],// draw location
                sprite_vertex[6], sprite_vertex[7]// draw size
            );
        }

        pvrctx.Restore();
    }


    public static StateSprite InitFromVertexColor(uint rgb8_color) {
        StateSprite statesprite = InitFromTexture(null);
        statesprite.SetVertexColorRGB8(rgb8_color);
        return statesprite;
    }

    public static StateSprite InitFromTexture(Texture texture) {
        StateSprite statesprite = new StateSprite();

        statesprite.matrix_source = new Modifier();
        statesprite.matrix_source.Clear();

        statesprite.src_x = 0;
        statesprite.src_y = 0;

        statesprite.vertex_color = new float[] { 1.0f, 1.0f, 1.0f };

        statesprite.offsetcolor = new float[4];
        PVRContext.HelperClearOffsetColor(statesprite.offsetcolor);

        statesprite.texture = texture;
        if (texture != null) {
            statesprite.src_width = statesprite.texture.original_width;
            statesprite.src_height = statesprite.texture.original_height;
        } else {
            statesprite.src_width = 0;
            statesprite.src_height = 0;
        }

        statesprite.draw_x = 100;
        statesprite.draw_y = 100;
        statesprite.draw_width = -1;
        statesprite.draw_height = -1;

        statesprite.alpha = 1.0f;
        statesprite.alpha2 = 1.0f;
        statesprite.visible = true;
        statesprite.z_index = 1;
        statesprite.z_offset = 0;

        statesprite.frame_x = 0;
        statesprite.frame_y = 0;
        statesprite.frame_width = 0;
        statesprite.frame_height = 0;
        statesprite.pivot_x = 0;
        statesprite.pivot_y = 0;

        statesprite.id = StateSprite.IDS++;
        StateSprite.POOL.Set(statesprite.id, statesprite);

        statesprite.vertex = new float[8];
        statesprite.vertex_dirty = true;

        statesprite.animation_external = null;

        statesprite.matrix_corner = new CornerRotation();

        statesprite.crop = new SpriteCropInfo();
        statesprite.crop_enabled = false;

        statesprite.flip_x = false;
        statesprite.flip_y = false;
        statesprite.flip_correction = true;

        statesprite.state_list = new LinkedList<StateSpriteState>();
        statesprite.offset_x = 0;
        statesprite.offset_y = 0;
        statesprite.selected_state = null;

        statesprite.scale_texture = 1.0f;
        statesprite.antialiasing = PVRFlag.DEFAULT;
        statesprite.atlas_to_draw_size_enabled = false;

        statesprite.psshader = null;

        statesprite.blend_enabled = true;
        statesprite.blend_src_rgb = Blend.DEFAULT;
        statesprite.blend_dst_rgb = Blend.DEFAULT;
        statesprite.blend_src_alpha = Blend.DEFAULT;
        statesprite.blend_dst_alpha = Blend.DEFAULT;

        statesprite.trailing_buffer = new Trail[10];
        statesprite.trailing_used = 0;
        statesprite.trailing_length = 10;
        statesprite.trailing_alpha = 0.9f;
        statesprite.trailing_delay = 0;
        statesprite.trailing_darken = true;
        statesprite.trailing_disabled = true;
        statesprite.trailing_progress = 0f;
        statesprite.trailing_offsetcolor = new float[] { 1f, 1f, 1f };

        return statesprite;
    }

    public bool DestroyTextureIfStateless() {
        if (this.texture == null) return false;

        // if the texture belongs to any state, return
        // it will fail in case of duplicated refereces (is a caller problem)
        foreach (StateSpriteState state in this.state_list) {
            if (state.texture != null && state.texture == this.texture) return false;
        }

        this.texture.Destroy();
        this.texture = null;
        return true;
    }

    public void Destroy() {
        foreach (StateSpriteState state in this.state_list) {
            if (state.animation != null) state.animation.Destroy();
            if (state.texture != null) state.texture.Destroy();
            //free(state.state_name);
            //free(state.frame_info);
            //free(state);
        }

        this.state_list.Destroy2(null/*free*/);

        StateSprite.POOL.Delete(this.id);
        //free(this.trailing_buffer);
        //free(this);
    }


    public Texture SetTexture(Texture texture, bool update_offset_source_size) {
        Texture old_texture = this.texture;
        this.texture = texture;

        if (update_offset_source_size && texture != null) {
            this.src_x = 0f;
            this.src_y = 0f;
            this.src_width = this.texture.original_width;
            this.src_height = this.texture.original_height;
            this.vertex_dirty = true;
        }

        return old_texture;
    }

    public void SetOffsetSource(float x, float y, float width, float height) {
        this.src_x = x;
        this.src_y = y;
        this.src_width = width;
        this.src_height = height;

        if (this.atlas_to_draw_size_enabled) {
            this.draw_width = width * this.scale_texture;
            this.draw_height = height * this.scale_texture;
        } else {
            if (this.draw_width < 0) this.draw_width = width * this.scale_texture;
            if (this.draw_height < 0) this.draw_height = height * this.scale_texture;
        }

        this.vertex_dirty = true;
    }

    public void SetOffsetFrame(float x, float y, float width, float height) {
        this.frame_x = x;
        this.frame_y = y;
        this.frame_width = width;
        this.frame_height = height;

        this.vertex_dirty = true;

        if (this.atlas_to_draw_size_enabled) {
            if (width > 0) this.draw_width = width * this.scale_texture;
            if (height > 0) this.draw_height = height * this.scale_texture;
        } else {
            if (this.draw_width < 0 && width > 0)
                this.draw_width = width * this.scale_texture;
            if (this.draw_height < 0 && height > 0)
                this.draw_height = height * this.scale_texture;
        }
    }

    public void SetOffsetPivot(float x, float y) {
        this.pivot_x = x;
        this.pivot_y = y;
        this.vertex_dirty = true;
    }


    public void SetDrawLocation(float x, float y) {
        this.draw_x = x;
        this.draw_y = y;
        this.vertex_dirty = true;
    }

    public void SetDrawSize(float width, float height) {
        this.draw_width = width;
        this.draw_height = height;
        this.vertex_dirty = true;
    }

    public void SetAlpha(float alpha) {
        this.alpha = alpha;
    }

    public void SetVisible(bool visible) {
        this.visible = visible;
    }

    public void SetZIndex(float index) {
        this.z_index = index;
    }

    public void SetZOffset(float offset) {
        this.z_offset = offset;
    }

    public void SetProperty(int property_id, float value) {
        switch (property_id) {
            case VertexProps.SPRITE_PROP_X:
                this.draw_x = value;
                this.vertex_dirty = true;
                break;
            case VertexProps.SPRITE_PROP_Y:
                this.draw_y = value;
                this.vertex_dirty = true;
                break;
            case VertexProps.SPRITE_PROP_WIDTH:
                this.draw_width = value;
                this.vertex_dirty = true;
                break;
            case VertexProps.SPRITE_PROP_HEIGHT:
                this.draw_height = value;
                this.vertex_dirty = true;
                break;
            case VertexProps.SPRITE_PROP_ROTATE:
                MatrixRotate(value);
                break;
            case VertexProps.SPRITE_PROP_SCALE_X:
                MatrixScale(value, Single.NaN);
                break;
            case VertexProps.SPRITE_PROP_SCALE_Y:
                MatrixScale(Single.NaN, value);
                break;
            case VertexProps.SPRITE_PROP_SKEW_X:
                MatrixSkew(value, Single.NaN);
                break;
            case VertexProps.SPRITE_PROP_SKEW_Y:
                MatrixSkew(Single.NaN, value);
                break;
            case VertexProps.SPRITE_PROP_TRANSLATE_X:
                MatrixTranslate(value, Single.NaN);
                break;
            case VertexProps.SPRITE_PROP_TRANSLATE_Y:
                MatrixTranslate(Single.NaN, value);
                break;
            case VertexProps.SPRITE_PROP_ALPHA:
                this.alpha = Math2D.Clamp(value, 0.0f, 1.0f);
                break;
            case VertexProps.SPRITE_PROP_Z:
                this.z_index = value;
                break;
            case VertexProps.SPRITE_PROP_VERTEX_COLOR_R:
                this.vertex_color[0] = value;
                break;
            case VertexProps.SPRITE_PROP_VERTEX_COLOR_G:
                this.vertex_color[1] = value;
                break;
            case VertexProps.SPRITE_PROP_VERTEX_COLOR_B:
                this.vertex_color[2] = value;
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
                Logger.Warn("statesprite_set_property() SPRITE_PROP_ANIMATIONLOOP is not supported");
                break;
            case VertexProps.SPRITE_PROP_ROTATE_PIVOT_ENABLED:
                MatrixRotatePivotEnable(value >= 1.0f);
                break;
            case VertexProps.SPRITE_PROP_ROTATE_PIVOT_U:
                this.matrix_source.rotate_pivot_u = value;
                break;
            case VertexProps.SPRITE_PROP_ROTATE_PIVOT_V:
                this.matrix_source.rotate_pivot_u = value;
                break;
            case VertexProps.SPRITE_PROP_SCALE_DIRECTION_X:
                this.matrix_source.scale_direction_x = value;
                break;
            case VertexProps.SPRITE_PROP_SCALE_DIRECTION_Y:
                this.matrix_source.scale_direction_y = value;
                break;
            case VertexProps.SPRITE_PROP_TRANSLATE_ROTATION:
                this.matrix_source.translate_rotation = value >= 1.0f;
                break;
            case VertexProps.SPRITE_PROP_SCALE_SIZE:
                this.matrix_source.scale_size = value >= 1.0f;
                break;
            case VertexProps.SPRITE_PROP_Z_OFFSET:
                this.z_offset = value;
                break;
            case VertexProps.SPRITE_PROP_ANTIALIASING:
                this.antialiasing = (PVRFlag)(int)Math.Truncate(value);
                break;
            case VertexProps.SPRITE_PROP_FLIP_X:
                this.flip_x = value >= 1.0f;
                this.vertex_dirty = true;
                break;
            case VertexProps.SPRITE_PROP_FLIP_Y:
                this.flip_y = value >= 1.0f;
                this.vertex_dirty = true;
                break;
            case VertexProps.SPRITE_PROP_FLIP_CORRECTION:
                this.flip_correction = value >= 1.0f;
                this.vertex_dirty = true;
                break;
            case VertexProps.SPRITE_PROP_ALPHA2:
                this.alpha2 = value;
                break;
        }
    }

    public void SetOffsetColor(float r, float g, float b, float a) {
        if (r >= 0) this.offsetcolor[0] = r;
        if (g >= 0) this.offsetcolor[1] = g;
        if (b >= 0) this.offsetcolor[2] = b;
        if (a >= 0) this.offsetcolor[3] = a;
    }

    public void SetVertexColor(float r, float g, float b) {
        if (r >= 0) this.vertex_color[0] = r;
        if (g >= 0) this.vertex_color[1] = g;
        if (b >= 0) this.vertex_color[2] = b;
    }

    public void SetVertexColorRGB8(uint rbg8_color) {
        Math2D.ColorBytesToFloats(rbg8_color, false, this.vertex_color);
    }


    public AnimSprite AnimationSet(AnimSprite animsprite) {
        AnimSprite old_animation = this.animation_external;
        this.animation_external = animsprite;
        return old_animation;
    }

    public void AnimationRestart() {
        if (this.animation_external != null) {
            this.animation_external.Restart();
            this.animation_external.UpdateStatesprite(this, true);
        }
    }

    public void AnimationEnd() {
        if (this.animation_external != null)
            this.animation_external.ForceEnd3(this);
    }

    public bool AnimationCompleted() {
        return this.animation_external == null || this.animation_external.IsCompleted();
    }

    public int Animate(float elapsed) {
        if (Single.IsNaN(elapsed)) throw new ArgumentException("elapsed", "Invalid elapsed value:" + elapsed);

        int result = 1;

        if (this.animation_external != null) {
            result = this.animation_external.Animate(elapsed);
            this.animation_external.UpdateStatesprite(this, true);
        }

        if (this.trailing_disabled) return result;

        // check delay for next trail
        bool wait = this.trailing_progress < this.trailing_delay;
        this.trailing_progress += elapsed;

        if (wait) return result;
        this.trailing_progress -= this.trailing_delay;

        // compute trailing using the cached vertex
        bool insert_vertex = true;
        if (this.trailing_used > 0) {
            // check if the last vertex equals to the current vertex
            ref Trail trail = ref this.trailing_buffer[0];
            bool source = trail.sx == this.vertex[0] && trail.sy == this.vertex[1] && trail.sw == this.vertex[2] && trail.sh == this.vertex[3];
            bool draw = trail.dx == this.vertex[4] && trail.dy == this.vertex[5] && trail.dw == this.vertex[6] && trail.dh == this.vertex[7];
            bool color = this.vertex_color[0] == trail.r && this.vertex_color[1] == trail.g && this.vertex_color[2] == trail.b;
            insert_vertex = !source || !draw || !color;

            if (insert_vertex) {
                // do right shift (this should be optimized to shift only used trails)
                for (int i = this.trailing_length - 1, j = this.trailing_length - 2 ; j >= 0 ; j--, i--) {
                    this.trailing_buffer[i] = this.trailing_buffer[j];
                }
            } else {
                this.trailing_used--;
            }
        }

        if (insert_vertex) {
            ref Trail trail = ref this.trailing_buffer[0];
            // add new trail
            trail.sx = this.vertex[0];
            trail.sy = this.vertex[1];
            trail.sw = this.vertex[2];
            trail.sh = this.vertex[3];
            trail.dx = this.vertex[4];
            trail.dy = this.vertex[5];
            trail.dw = this.vertex[6];
            trail.dh = this.vertex[7];
            trail.r = this.vertex_color[0];
            trail.g = this.vertex_color[1];
            trail.b = this.vertex_color[2];
            if (this.trailing_used < this.trailing_length) this.trailing_used++;
        }

        // update alpha of each trail
        for (int i = 0 ; i < this.trailing_used ; i++) {
            this.trailing_buffer[i].alpha = (1f - (i / this.trailing_used)) * this.trailing_alpha;
        }

        return result;
    }


    public float GetZIndex() {
        return this.z_index + this.z_offset;
    }

    public Modifier GetModifier() {
        return this.matrix_source;
    }

    public void GetDrawSize(out float draw_width, out float draw_height) {
        draw_width = this.draw_width >= 0 ? this.draw_width : this.src_width;
        draw_height = this.draw_height >= 0 ? this.draw_height : this.src_height;
    }

    public void GetDrawLocation(out float draw_x, out float draw_y) {
        draw_x = this.draw_x;
        draw_y = this.draw_y;
    }

    public float GetAlpha() {
        return this.alpha * this.alpha2;
    }

    public void GetOffsetSourceSize(out float width, out float height) {
        width = this.frame_width > 0 ? this.frame_width : this.src_width;
        height = this.frame_height > 0 ? this.frame_height : this.src_height;
    }

    public bool IsVisible() {
        return this.visible;
    }


    public void MatrixReset() {
        this.matrix_source.Clear();
        this.flip_x = this.flip_y = false;
        this.matrix_corner.Clear();
    }

    public void MatrixScale(float scale_x, float scale_y) {
        if (!Single.IsNaN(scale_x)) this.matrix_source.scale_x = scale_x;
        if (!Single.IsNaN(scale_y)) this.matrix_source.scale_y = scale_y;
    }

    public void MatrixRotate(float degrees_angle) {
        this.matrix_source.rotate = degrees_angle * Math2D.DEG_TO_RAD;
    }

    public void MatrixSkew(float skew_x, float skew_y) {
        if (!Single.IsNaN(skew_x)) this.matrix_source.skew_x = skew_x;
        if (!Single.IsNaN(skew_y)) this.matrix_source.skew_y = skew_y;
    }

    public void MatrixTranslate(float translate_x, float translate_y) {
        if (!Single.IsNaN(translate_x)) this.matrix_source.translate_x = translate_x;
        if (!Single.IsNaN(translate_y)) this.matrix_source.translate_y = translate_y;
    }

    public void MatrixRotatePivotEnable(bool enable) {
        this.matrix_source.rotate_pivot_enabled = enable;
    }

    public void MatrixRotatePivot(float u, float v) {
        if (!Single.IsNaN(u)) this.matrix_source.rotate_pivot_u = u;
        if (!Single.IsNaN(v)) this.matrix_source.rotate_pivot_v = v;
    }

    public void matrixScaleOffset(float direction_x, float direction_y) {
        if (!Single.IsNaN(direction_x)) this.matrix_source.scale_direction_x = direction_x;
        if (!Single.IsNaN(direction_y)) this.matrix_source.scale_direction_y = direction_y;
    }

    public void MatrixTranslateRotation(bool enable_translate_rotation) {
        this.matrix_source.translate_rotation = enable_translate_rotation;
    }

    public void MatrixScaleSize(bool enable_scale_size) {
        this.matrix_source.scale_size = enable_scale_size;
    }


    public StateSpriteState StateAdd(ModelHolder modelholder, string animation_name, string state_name) {
        if (modelholder == null) return null;
        AtlasEntry atlas_entry = modelholder.GetAtlasEntry2(animation_name, false);
        AnimSprite animsprite = modelholder.CreateAnimsprite(animation_name, false, false);
        Texture texture = modelholder.GetTexture(false);
        uint vertex_color = modelholder.GetVertexColor();

        if (atlas_entry == null && animsprite != null) {
            atlas_entry = animsprite.HelperGetFirstFrameAtlasEntry();
        }

        StateSpriteState statesprite_state = StateAdd2(
             texture, animsprite, atlas_entry, vertex_color, state_name
        );

        if (statesprite_state == null && animsprite != null) animsprite.Destroy();

        return statesprite_state;
    }

    public StateSpriteState StateAdd2(Texture texture, AnimSprite animsprt, AtlasEntry atlas_entry, uint rgb8_color, string state_name) {
        if (texture != null && atlas_entry == null && animsprt == null) return null;

        float corner_x = this.matrix_corner.x;
        float corner_y = this.matrix_corner.y;
        float corner_angle = this.matrix_corner.angle;

        if (texture == null && atlas_entry != null) {
            Logger.Warn("statesprite_state_add2() atlas_entry specified but the texture was null");
            atlas_entry = null;
        }

        if (texture != null) texture = texture.ShareReference();

        StateSpriteState state = new StateSpriteState() {
            statesprite_id = this.id,
            state_name = state_name,
            texture = texture,
            animation = animsprt,
            vertex_color = rgb8_color,
            frame_info = null,
            corner_rotation = new CornerRotation() { x = corner_x, y = corner_y, angle = corner_angle },
            offset_x = this.offset_x,
            offset_y = this.offset_y,
            draw_width = this.draw_width,
            draw_height = this.draw_height
        };

        if (atlas_entry != null) {
            state.frame_info = atlas_entry.Clone();
        }

        this.state_list.AddItem(state);

        return state;
    }

    public bool StateToggle(string state_name) {
        if (this.selected_state != null && this.selected_state.state_name == state_name)
            return true;

        StateSpriteState state = null;

        foreach (StateSpriteState item in this.state_list) {
            if (item.state_name == state_name) {
                state = item;
                break;
            }
        }

        if (state == null) return false;

        StateApply(state);

        return true;
    }

    public LinkedList<StateSpriteState> StateList() {
        return this.state_list;
    }

    public string StateName() {
        if (this.selected_state != null) return this.selected_state.state_name;
        return null;
    }

    public bool StateHas(string state_name) {
        foreach (StateSpriteState state in this.state_list) {
            if (state.state_name == state_name) return true;
        }
        return false;
    }

    public StateSpriteState StateGet() {
        return this.selected_state;
    }

    public bool StateApply(StateSpriteState statesprite_state) {
        if (statesprite_state == null && this.selected_state == null) return false;

        if (statesprite_state != null && statesprite_state.statesprite_id != this.id) {
            Logger.Error("statesprite_state_apply() attempt to apply a foreign state");
            return false;
        }

        StateSpriteState state = statesprite_state != null ? statesprite_state : this.selected_state;

        if (state.frame_info != null) {
            SetOffsetSource(
                state.frame_info.x, state.frame_info.y,
                state.frame_info.width, state.frame_info.height
            );
            SetOffsetFrame(
                state.frame_info.frame_x, state.frame_info.frame_y,
                state.frame_info.frame_width, state.frame_info.frame_height
            );
            SetOffsetPivot(
                state.frame_info.pivot_x,
                state.frame_info.pivot_y
            );
        } else {
            SetOffsetPivot(
                0f,
                0f
            );
        }

        this.texture = state.texture;
        this.animation_external = state.animation;
        SetVertexColorRGB8(state.vertex_color);
        this.offset_x = state.offset_x;
        this.offset_y = state.offset_y;
        this.draw_width = state.draw_width;
        this.draw_height = state.draw_height;
        this.matrix_corner.x = state.corner_rotation.x;
        this.matrix_corner.y = state.corner_rotation.y;
        this.matrix_corner.angle = state.corner_rotation.angle;

        this.selected_state = state;
        this.vertex_dirty = true;
        return true;
    }

    public void StateGetOffsets(out float offset_x, out float offset_y) {
        offset_x = this.offset_x;
        offset_y = this.offset_y;
    }

    public void StateSetOffsets(float offset_x, float offset_y, bool update_state) {
        this.offset_x = offset_x;
        this.offset_y = offset_y;

        if (update_state && this.selected_state != null) {
            this.selected_state.offset_x = offset_x;
            this.selected_state.offset_y = offset_y;
        }

        this.vertex_dirty = true;
    }

    public void StateRemove(string state_name) {
        int i = 0;

        foreach (StateSpriteState state in this.state_list) {
            if (state.state_name == state_name) {
                if (this.animation_external == state.animation) this.animation_external = null;
                if (state.texture != null) state.texture.Destroy();
                if (state.animation != null) state.animation.Destroy();
                if (state == this.selected_state) this.selected_state = null;

                this.state_list.RemoveItemAt(i);

                //free(state.state_name);
                //free(state.frame_info);
                //free(state);
                return;
            }
            i++;
        }
    }


    public void ResizeDrawSize(float max_width, float max_height, out float applied_width, out float applied_height) {
        float orig_width = this.frame_width > 0f ? this.frame_width : this.src_width;
        float orig_height = this.frame_height > 0f ? this.frame_height : this.src_height;

        float width = Single.NaN, height = Single.NaN;

        if (max_width < 1 && max_height < 1f) {
            width = orig_width;
            height = orig_height;
        } else if (orig_width == 0f && orig_height == 0f) {
            width = max_width < 0f ? max_height : max_width;
            height = max_height < 0f ? max_width : max_height;
        } else if (max_width == 0f || max_height == 0f) {
            width = height = 0f;
        } else {
            if (max_width > 0f && max_height > 0f) {
                float scale_x = max_width / orig_width;
                float scale_y = max_height / orig_height;

                if (scale_x > scale_y)
                    max_width = Single.NegativeInfinity;
                else
                    max_height = Single.NegativeInfinity;
            }

            if (max_height > 0f) {
                height = max_height;
                width = (orig_width * max_height) / orig_height;
            }

            if (max_width > 0f) {
                height = (orig_height * max_width) / orig_width;
                width = max_width;
            }
        }

        this.draw_width = width;
        this.draw_height = height;
        this.vertex_dirty = true;

        applied_width = width;
        applied_height = height;
    }

    public void CenterDrawLocation(float x, float y, float ref_width, float ref_height, out float applied_x, out float applied_y) {
        if (ref_width >= 0f) x += ((ref_width - this.draw_width) / 2f);
        if (ref_height >= 0f) y += ((ref_height - this.draw_height) / 2f);

        this.draw_x = x;
        this.draw_y = y;
        this.vertex_dirty = true;

        applied_x = this.draw_x;
        applied_y = this.draw_y;
    }

    public void CornerRotate(Corner corner) {
        this.matrix_corner.HelperCalculateCornerRotation(corner);
    }


    public bool Crop(float dx, float dy, float dwidth, float dheight) {
        if (!Single.IsNaN(dx)) this.crop.x = dx;
        if (!Single.IsNaN(dy)) this.crop.y = dy;
        if (!Single.IsNaN(dwidth)) this.crop.width = dwidth;
        if (!Single.IsNaN(dheight)) this.crop.height = dheight;

        bool invalid = this.crop.x < 0f &&
            this.crop.y < 0f &&
            this.crop.width == 0f &&
            this.crop.height == 0f;

        // disable cropping if the bounds are invalid
        if (this.crop_enabled && invalid) this.crop_enabled = false;

        this.vertex_dirty = true;
        return !invalid;
    }

    public bool IsCropEnabled() {
        return this.crop_enabled;
    }

    public void CropEnable(bool enable) {
        this.crop_enabled = enable;
    }

    public void FlipTexture(bool? flip_x, bool? flip_y) {
        if (flip_x != null) this.flip_x = flip_x == true;
        if (flip_y != null) this.flip_y = flip_y == true;
        this.vertex_dirty = true;
    }

    public void FlipRenderedTextureEnableCorrection(bool enabled) {
        this.flip_correction = enabled;
    }

    public void ChangeDrawSizeInAtlasApply(bool enable, float scale_factor) {
        if (Single.IsNaN(scale_factor)) throw new NaNArgumentError("invalid scale_factor");
        this.scale_texture = scale_factor;
        this.atlas_to_draw_size_enabled = enable;
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

    public void BlendEnable(bool enabled) {
        this.blend_enabled = enabled;
    }

    public void BlendSet(Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha) {
        this.blend_src_rgb = src_rgb;
        this.blend_dst_rgb = dst_rgb;
        this.blend_src_alpha = src_alpha;
        this.blend_dst_alpha = dst_alpha;
    }

    public void TrailingEnabled(bool enabled) {
        this.trailing_disabled = !enabled;
    }

    public void TrailingSetParams(int length, float trail_delay, float trail_alpha, bool? darken_colors) {
        if (length > 0) {
            Array.Resize(ref this.trailing_buffer, length);
            this.trailing_length = length;
            if (this.trailing_used > length) this.trailing_used = length;
        }
        if (darken_colors != null) this.trailing_darken = (bool)darken_colors;
        if (!Single.IsNaN(trail_delay)) this.trailing_delay = trail_delay;
        if (!Single.IsNaN(trail_alpha)) this.trailing_alpha = trail_alpha;

        // force update
        this.trailing_progress = this.trailing_delay;
    }

    public void TrailingSetOffsetcolor(float r, float g, float b) {
        if (!Single.IsNaN(r)) this.trailing_offsetcolor[0] = r;
        if (!Single.IsNaN(g)) this.trailing_offsetcolor[1] = g;
        if (!Single.IsNaN(b)) this.trailing_offsetcolor[2] = b;
    }


    private struct Trail {
        public float sx, sy, sw, sh, dx, dy, dw, dh, alpha, r, g, b;
    }
}
