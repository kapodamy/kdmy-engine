using System;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Image;

internal struct SpriteCropInfo {
    public float x;
    public float y;
    public float width;
    public float height;
}

public class Sprite : IVertex {
    private static int IDS = 0;
    private static Map<Sprite> POOL = new Map<Sprite>();

    private Sprite() { }

    private Modifier matrix_source;
    private bool flip_x;
    private bool flip_y;
    private bool flip_correction;
    private CornerRotation matrix_corner;
    private float src_x;
    private float src_y;
    private float[] vertex_color;
    private float[] offsetcolor;
    private Texture texture;
    private float src_width;
    private float src_height;
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
    private int id;
    private LinkedList<AnimSprite> animation_list;
    private AnimSprite animation_selected;
    private AnimSprite animation_external;

    private float[] vertex;
    private bool vertex_dirty;

    private SpriteCropInfo crop;
    private bool crop_enabled;

    private PVRFlag antialiasing;
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

        float draw_x = this.draw_x;
        float draw_y = this.draw_y;

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
                if (this.frame_width > 0) {
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

        if (this.antialiasing != PVRFlag.DEFAULT) {
            pvrctx.SetVertexAntialiasing(this.antialiasing);
        }

        // apply transformation matrix
        MatrixCalculate(pvrctx);

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

    public static Sprite InitFromRGB8(uint solid_rgb8_color) {
        float[] percent_color = { 0.0f, 0.0f, 0.0f };

        Math2D.ColorBytesToFloats(solid_rgb8_color, false, percent_color);

        Sprite sprite = Sprite.Init(null);
        sprite.SetVertexColor(percent_color[0], percent_color[1], percent_color[2]);

        return sprite;
    }

    public static Sprite Init(Texture src_texture) {
        Sprite sprite = new Sprite();

        sprite.matrix_source = new Modifier();
        sprite.matrix_source.Clear();

        sprite.flip_x = false;
        sprite.flip_y = false;
        sprite.flip_correction = true;
        sprite.matrix_corner = new CornerRotation();

        sprite.src_x = 0;
        sprite.src_y = 0;

        sprite.vertex_color = new float[] { 1.0f, 1.0f, 1.0f };

        sprite.offsetcolor = new float[4];
        PVRContext.HelperClearOffsetColor(sprite.offsetcolor);

        sprite.texture = src_texture;
        if (src_texture != null) {
            sprite.src_width = sprite.texture.original_width;
            sprite.src_height = sprite.texture.original_height;
        } else {
            sprite.src_width = 0;
            sprite.src_height = 0;
        }

        sprite.draw_x = 0;
        sprite.draw_y = 0;
        sprite.draw_width = -1;
        sprite.draw_height = -1;

        sprite.alpha = 1.0f;
        sprite.alpha2 = 1.0f;
        sprite.visible = true;
        sprite.z_index = 1;
        sprite.z_offset = 0;

        sprite.frame_x = 0;
        sprite.frame_y = 0;
        sprite.frame_width = 0;
        sprite.frame_height = 0;

        sprite.pivot_x = 0;
        sprite.pivot_y = 0;

        sprite.id = Sprite.IDS++;
        Sprite.POOL.Set(sprite.id, sprite);

        sprite.animation_list = new LinkedList<AnimSprite>();
        sprite.animation_selected = null;
        sprite.animation_external = null;

        sprite.vertex = new float[8];
        sprite.vertex_dirty = true;

        sprite.crop = new SpriteCropInfo();
        sprite.crop_enabled = false;

        sprite.antialiasing = PVRFlag.DEFAULT;

        sprite.psshader = null;
        sprite.blend_enabled = true;// obligatory
        sprite.blend_src_rgb = Blend.DEFAULT;
        sprite.blend_dst_rgb = Blend.DEFAULT;
        sprite.blend_src_alpha = Blend.DEFAULT;
        sprite.blend_dst_alpha = Blend.DEFAULT;

        sprite.trailing_buffer = new Trail[10];
        sprite.trailing_used = 0;
        sprite.trailing_length = 10;
        sprite.trailing_alpha = 0.9f;
        sprite.trailing_delay = 0;
        sprite.trailing_darken = true;
        sprite.trailing_disabled = true;
        sprite.trailing_progress = 0f;
        sprite.trailing_offsetcolor = new float[] { 1f, 1f, 1f };

        return sprite;
    }

    public void Destroy() {
        this.animation_list.Destroy();

        this.animation_list = null;
        this.animation_external = null;
        this.texture = null;

        Sprite.POOL.Delete(this.id);
        Luascript.DropShared(this.matrix_source);
        Luascript.DropShared(this);
        //free(this.trailing_buffer);
        //free(this);
    }

    public void DestroyAllAnimations() {
        foreach (AnimSprite animsprite in this.animation_list)
            animsprite.Destroy();

        this.animation_list.Clear(null);
        this.animation_selected = null;

        if (this.animation_external != null) {
            this.animation_external.Destroy();
            this.animation_external = null;
        }
    }

    public void DestroyTexture() {
        if (this.texture != null) {
            this.texture.Destroy();
            this.texture = null;
        }
    }

    public void DestroyFull() {
        //if (!this) return;
        DestroyTexture();
        DestroyAllAnimations();
        Destroy();
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

    public Modifier MatrixGetModifier() {
        return this.matrix_source;
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

    public void SetOffsetSource(float x, float y, float width, float height) {
        this.src_x = x;
        this.src_y = y;
        this.src_width = width;
        this.src_height = height;

        this.vertex_dirty = true;

        if (this.draw_width < 0) this.draw_width = width;
        if (this.draw_height < 0) this.draw_height = height;
    }

    public void SetOffsetFrame(float x, float y, float width, float height) {
        this.frame_x = x;
        this.frame_y = y;
        this.frame_width = width;
        this.frame_height = height;

        this.vertex_dirty = true;

        if (this.draw_width < 0) this.draw_width = width;
        if (this.draw_height < 0) this.draw_height = height;
    }

    public void SetOffsetPivot(float x, float y) {
        this.pivot_x = x;
        this.pivot_y = y;

        this.vertex_dirty = true;
    }

    public void FlipRenderedTexture(bool? flip_x, bool? flip_y) {
        if (flip_x != null) this.flip_x = flip_x == true;
        if (flip_y != null) this.flip_y = flip_y == true;
        this.vertex_dirty = true;
    }

    public void FlipRenderedTextureEnableCorrection(bool enabled) {
        this.flip_correction = enabled;
    }

    public void MatrixReset() {
        this.matrix_source.Clear();
        this.flip_x = false;
        this.flip_y = false;
        this.matrix_corner.Clear();
    }

    public void SetDrawLocation(float x, float y) {
        this.draw_x = x;
        this.draw_y = y;
        this.vertex_dirty = true;
    }

    public void SetDrawSize(float width, float height) {
        if (!Single.IsNaN(width)) this.draw_width = width;
        if (!Single.IsNaN(height)) this.draw_height = height;
        this.vertex_dirty = true;

        if (this.texture == null) {
            if (!Single.IsNaN(width)) this.src_width = width;
            if (!Single.IsNaN(height)) this.src_height = height;
        }
    }

    public void SetDrawSizeFromSourceSize() {
        this.draw_width = this.src_width;
        this.draw_height = this.src_height;
        this.vertex_dirty = true;
    }

    public void SetAlpha(float alpha) {
        this.alpha = alpha;
    }

    public float GetAlpha() {
        return this.alpha * this.alpha2;
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

    public float GetZIndex() {
        return this.z_index + this.z_offset;
    }

    public void AnimationAdd(AnimSprite animsprite) {
        if (animsprite == null) return;

        // ¿should duplicate first?
        this.animation_list.AddItem(animsprite);
    }

    public void AnimationRemove(string animation_name) {
        if (animation_name == null) {
            this.animation_list.Clear(null);
            return;
        }

        int i = 0;
        foreach (AnimSprite animsprite in this.animation_list) {
            if (animsprite.GetName() == animation_name) {
                this.animation_list.RemoveItemAt(i);
                return;
            }
            i++;
        }
    }

    public void AnimationRestart() {
        if (this.animation_selected == null) return;
        this.animation_selected.Restart();
        this.animation_selected.UpdateSprite(this, true);
    }

    public bool AnimationPlay(string animation_name) {
        //this.animation_selected = null;
        if (animation_name == null) return false;

        AnimSprite animation = AnimationGetAttached(animation_name);

        if (animation == null) {
            Logger.Log($"sprite_animation_play() animation no added: {animation_name}");
            return true;
        }

        this.animation_selected = animation;
        return true;
    }

    public void AnimationPlayByIndex(int index) {
        //this.animation_selected = null;

        if (index < 0 || index >= this.animation_list.Count()) {
            Logger.Log("sprite_animation_play_by_index() rejected, index was out-of-bounds");
            return;
        }

        this.animation_selected = this.animation_list.GetByIndex(index);
    }

    public bool AnimationPlayByAnimsprite(AnimSprite animsprite, bool only_if_attached) {
        if (animsprite == null) {
            if (!only_if_attached)
                this.animation_selected = null;

            return false;
        }

        if (!only_if_attached) {
            //
            // Warning: 
            //         using this way, the animation won't be disposed
            //         calling sprite_destroy_all_animations() or sprite_destroy_full()
            //
            this.animation_selected = animsprite;
            return true;
        }

        foreach (AnimSprite attached_animsprite in this.animation_list) {
            if (attached_animsprite == animsprite) {
                this.animation_selected = attached_animsprite;
                return true;
            }
        }

        return false;
    }

    public void AnimationPlayFirst() {
        // if there no animations on the list the "animation_selected" will be null.
        // This means no animation selected
        this.animation_selected = this.animation_list.GetFirstItem();
    }

    public void Animation_Stop() {
        if (this.animation_selected != null) this.animation_selected.Stop();
    }

    public void AnimationEnd() {
        if (this.animation_selected != null) {
            this.animation_selected.ForceEnd();
            this.animation_selected.UpdateSprite(this, true);
        }
    }

    public int Animate(float elapsed) {
        if (Single.IsNaN(elapsed)) throw new ArgumentException("elapsed", "Invalid elapsed value:" + elapsed);

        int result = 1;

        if (this.animation_selected != null) {
            result = this.animation_selected.Animate(elapsed);
            this.animation_selected.UpdateSprite(this, true);
        }

        if (this.animation_external != null) {
            result = this.animation_external.Animate(elapsed);
            this.animation_external.UpdateSprite(this, false);
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

    public AnimSprite AnimationGetAttached(string animation_name) {
        if (this.animation_list.Count() < 1) {
            Logger.Log($"sprite_animation_get_attached() no animation attached, can not play: {animation_name}");
            return null;
        }

        foreach (AnimSprite animsprite in this.animation_list) {
            if (animation_name == null || animsprite.GetName() == animation_name)
                return animsprite;
        }

        return null;
    }


    public void GetSourceSize(out float width, out float height) {
        width = this.frame_width > 0 ? this.frame_width : this.src_width;
        height = this.frame_height > 0 ? this.frame_height : this.src_height;
    }

    public void GetDrawSize(out float width, out float height) {
        width = this.draw_width >= 0 ? this.draw_width : this.src_width;
        height = this.draw_height >= 0 ? this.draw_height : this.src_height;
    }

    public void GetDrawLocation(out float x, out float y) {
        x = this.draw_x;
        y = this.draw_y;
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
                SetVertexColor(value, -1, -1);
                break;
            case VertexProps.SPRITE_PROP_VERTEX_COLOR_G:
                SetVertexColor(-1, value, -1);
                break;
            case VertexProps.SPRITE_PROP_VERTEX_COLOR_B:
                SetVertexColor(-1, -1, value);
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
                if (this.animation_selected != null)
                    this.animation_selected.SetLoop((int)value);
                break;
            case VertexProps.SPRITE_PROP_ROTATE_PIVOT_ENABLED:
                MatrixRotatePivotEnable(value >= 1.0f);
                break;
            case VertexProps.SPRITE_PROP_ROTATE_PIVOT_U:
                MatrixRotatePivot(value, Single.NaN);
                break;
            case VertexProps.SPRITE_PROP_ROTATE_PIVOT_V:
                MatrixRotatePivot(Single.NaN, value);
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
            case VertexProps.SPRITE_PROP_SCALE_TRANSLATION:
                this.matrix_source.scale_translation = value >= 1.0f;
                break;
            case VertexProps.SPRITE_PROP_Z_OFFSET:
                this.z_offset = value;
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
            case VertexProps.SPRITE_PROP_ANTIALIASING:
                this.antialiasing = (PVRFlag)(int)Math.Truncate(value);
                break;
            case VertexProps.SPRITE_PROP_ALPHA2:
                this.alpha2 = value;
                break;
        }
    }


    public void SetVertexColorRGB8(uint rbg8_color) {
        Math2D.ColorBytesToFloats(rbg8_color, false, this.vertex_color);
    }

    public void SetVertexColorRGBA8(uint rbga8_color) {
        SetVertexColorRGB8(rbga8_color >> 8);
        this.alpha = (rbga8_color & 0xFF) / 255f;
    }

    public void SetVertexColor(float r, float g, float b) {
        if (r >= 0) this.vertex_color[0] = r;
        if (g >= 0) this.vertex_color[1] = g;
        if (b >= 0) this.vertex_color[2] = b;
    }

    public uint GetVertexColorRGB8() {
        return Math2D.ColorFloatsToBytes(this.vertex_color, false);
    }


    public void SetOffsetColorRGBA8(uint rgba8_color) {
        Math2D.ColorBytesToFloats(rgba8_color, true, this.offsetcolor);
    }

    public void SetOffsetColor(float r, float g, float b, float a) {
        if (r >= 0) this.offsetcolor[0] = r;
        if (g >= 0) this.offsetcolor[1] = g;
        if (b >= 0) this.offsetcolor[2] = b;
        if (a >= 0) this.offsetcolor[3] = a;
    }


    public void MatrixCalculate(PVRContext pvrctx) {
        SIMDMatrix matrix = pvrctx.CurrentMatrix;

        // apply self modifier
        matrix.ApplyModifier2(
            this.matrix_source, this.draw_x, this.draw_y, this.draw_width, this.draw_height
        );

        // do corner rotation (if required)
        if (this.matrix_corner.angle != 0) {
            matrix.CornerRotate(
                this.matrix_corner,
                this.draw_x, this.draw_y, this.draw_width, this.draw_height
            );
        }

        //pvrctx.Flush();
    }


    public AnimSprite ExternalAnimationSet(AnimSprite animsprite) {
        AnimSprite old_animation = this.animation_external;
        this.animation_external = animsprite;
        return old_animation;
    }

    public void ExternalAnimationRestart() {
        if (this.animation_external == null) return;
        this.animation_external.Restart();
        this.animation_external.UpdateSprite(this, false);
    }

    public void ExternalAnimationEnd() {
        if (this.animation_external != null)
            this.animation_external.ForceEnd2(this);
    }


    public void ResizeDrawSize(float max_width, float max_height, out float applied_width, out float applied_height) {
        float orig_width = 0f, orig_height = 0f;
        GetSourceSize(out orig_width, out orig_height);

        float width = Single.NaN, height = Single.NaN;

        if (max_width < 1f && max_height < 1f) {
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
        float draw_width = 0f, draw_height = 0f;
        GetDrawSize(out draw_width, out draw_height);

        if (ref_width >= 0f) x += ((ref_width - draw_width) / 2f);
        if (ref_height >= 0f) y += ((ref_height - draw_height) / 2f);

        this.draw_x = x;
        this.draw_y = y;
        this.vertex_dirty = true;

        applied_x = this.draw_x;
        applied_y = this.draw_y;
    }


    public bool IsTextured() {
        return this.texture != null;
    }

    public bool IsVisible() {
        return this.visible;
    }


    public void MatrixRotatePivotEnable(bool enable) {
        this.matrix_source.rotate_pivot_enabled = enable;
    }

    public void MatrixRotatePivot(float u, float v) {
        if (!Single.IsNaN(u)) this.matrix_source.rotate_pivot_u = u;
        if (!Single.IsNaN(v)) this.matrix_source.rotate_pivot_v = v;
    }

    public void MatrixScaleOffset(float direction_x, float direction_y) {
        if (!Single.IsNaN(direction_x)) this.matrix_source.scale_direction_x = direction_x;
        if (!Single.IsNaN(direction_y)) this.matrix_source.scale_direction_y = direction_y;
    }

    public void MatrixTranslateRotation(bool enable_translate_rotation) {
        this.matrix_source.translate_rotation = enable_translate_rotation;
    }

    public void MatrixScaleSize(bool enable_scale_size) {
        this.matrix_source.scale_size = enable_scale_size;
    }

    public void MatrixCornerRotation(Corner corner) {
        this.matrix_corner.HelperCalculateCornerRotation(corner);
    }

    public void SetDrawX(float value) {
        this.draw_x = value;
        this.vertex_dirty = true;
    }

    public void SetDrawY(float value) {
        this.draw_y = value;
        this.vertex_dirty = true;
    }


    public bool Crop(float dx, float dy, float dwidth, float dheight) {
        if (!Single.IsNaN(dx)) this.crop.x = dx;
        if (!Single.IsNaN(dy)) this.crop.y = dy;
        if (!Single.IsNaN(dwidth)) this.crop.width = dwidth;
        if (!Single.IsNaN(dheight)) this.crop.height = dheight;

        bool invalid = this.crop.x < 0f && this.crop.y < 0f && this.crop.width == 0f && this.crop.height == 0f;

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


    public void FillAtlasEntry(AtlasEntry target_atlas_entry) {
        target_atlas_entry.name = null;
        target_atlas_entry.x = this.src_x;
        target_atlas_entry.y = this.src_y;
        target_atlas_entry.width = this.src_width;
        target_atlas_entry.height = this.src_height;
        target_atlas_entry.frame_x = this.frame_x;
        target_atlas_entry.frame_y = this.frame_y;
        target_atlas_entry.frame_width = this.frame_width;
        target_atlas_entry.frame_height = this.frame_height;
        target_atlas_entry.pivot_x = this.pivot_x;
        target_atlas_entry.pivot_y = this.pivot_y;
    }

    public Texture GetTexture() {
        return this.texture;
    }

    public void SetAntialiasing(PVRFlag antialiasing) {
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
        if (this.trailing_disabled) this.trailing_used = 0;
    }

    public void TrailingSetParams(int length, float trail_delay, float trail_alpha, bool? darken_colors) {
        if (length > 0) {
            EngineUtils.ResizeArray(ref this.trailing_buffer, length);
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
