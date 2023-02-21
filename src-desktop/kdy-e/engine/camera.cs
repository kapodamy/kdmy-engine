using System;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Platform;
using Engine.Utils;

namespace Engine {

    public class Camera : IAnimate, ISetProperty {

        public const AnimInterpolator DEFAULT_INTERPOLATOR = AnimInterpolator.LINEAR;
        public const int PX = 0;
        public const int PY = 1;
        public const int PZ = 2;
        public const int OX = 3;
        public const int OY = 4;
        public const int OZ = 5;


        private TweenLerp tweenlerp;
        private Modifier modifier;
        private double progress;
        private double duration;
        //private bool scene_zoom;
        private double beat_duration;
        private bool internal_modifier;
        private bool transition_completed;
        private bool force_update;
        private bool has_transition_offset;
        private float parallax_x;
        private float parallax_y;
        private float parallax_z;
        private float offset_x;
        private float offset_y;
        private float offset_z;
        private Layout parent_layout;
        private AnimSprite animation;
        private bool enable_offset_zoom;
        private float half_viewport_width;
        private float half_viewport_height;


        public Camera(Modifier modifier, float viewport_width, float viewport_height) {
            this.tweenlerp = null;
            this.modifier = modifier;
            this.progress = 600.0;
            this.duration = 600.0f;
            this.beat_duration = 600f;
            this.internal_modifier = modifier == null;
            this.transition_completed = true;
            this.force_update = false;
            this.has_transition_offset = false;
            this.parallax_x = 0.0f; this.parallax_y = 0.0f; this.parallax_z = 1.0f;
            this.offset_x = 0.0f; this.offset_y = 0.0f; this.offset_z = 1.0f;
            this.parent_layout = null;
            this.animation = null;
            this.enable_offset_zoom = true;
            this.half_viewport_width = viewport_width / 2f;
            this.half_viewport_height = viewport_height / 2f;

            if (this.internal_modifier) {
                // no modifier provided used the internal one
                this.modifier = new Modifier();
                this.modifier.Clear();
                this.modifier.width = viewport_width;
                this.modifier.width = viewport_height;
                this.modifier.scale_direction_x = -0.5f;
                this.modifier.scale_direction_y = -0.5f;
            }

            this.SetInterpolatorType(Camera.DEFAULT_INTERPOLATOR);
        }

        public void Destroy() {
            this.tweenlerp.Destroy();

            if (this.internal_modifier) {
                Luascript.DropShared(this.modifier);
                //free(this.modifier);
            }

            if (this.animation != null) this.animation.Destroy();

            Luascript.DropShared(this);
            //free(camera);
        }


        public void SetBpm(float beats_per_minute) {
            this.beat_duration = Math2D.BeatsPerMinuteToBeatPerMilliseconds(beats_per_minute);
        }

        public void SetInterpolatorType(AnimInterpolator type) {
            float px, py, pz, ox, oy, oz;

            if (this.tweenlerp != null) {
                px = this.tweenlerp.PeekValueByIndex(Camera.PX);
                py = this.tweenlerp.PeekValueByIndex(Camera.PY);
                pz = this.tweenlerp.PeekValueByIndex(Camera.PZ);
                ox = this.tweenlerp.PeekValueByIndex(Camera.OX);
                oy = this.tweenlerp.PeekValueByIndex(Camera.OY);
                oz = this.tweenlerp.PeekValueByIndex(Camera.OZ);
                this.tweenlerp.Destroy();
            } else {
                px = this.parallax_x;
                py = this.parallax_y;
                pz = this.parallax_z;
                ox = this.offset_x;
                oy = this.offset_y;
                oz = this.offset_z;
            }

            this.tweenlerp = new TweenLerp();

            this.tweenlerp.AddInterpolator(-1, px, px, 0, type);
            this.tweenlerp.AddInterpolator(-1, py, py, 0, type);
            this.tweenlerp.AddInterpolator(-1, pz, pz, 0, type);
            this.tweenlerp.AddInterpolator(-1, ox, ox, 0, type);
            this.tweenlerp.AddInterpolator(-1, oy, oy, 0, type);
            this.tweenlerp.AddInterpolator(-1, oz, oz, 0, type);
            this.tweenlerp.End();

            this.progress = this.duration;
            this.transition_completed = true;
        }

        public void SetTransitionDuration(bool expresed_in_beats, float value) {
            if (expresed_in_beats)
                this.duration = value * this.beat_duration;
            else
                this.duration = value;
        }

        public void SetAbsoluteZoom(float z) {
            if (!Single.IsNaN(z)) InternalTweenlerpAbsolute(Camera.PZ, z);
            this.force_update = true;
            End();
        }

        public void SetAbsolutePosition(float x, float y) {
            if (!Single.IsNaN(x)) InternalTweenlerpAbsolute(Camera.PX, x);
            if (!Single.IsNaN(y)) InternalTweenlerpAbsolute(Camera.PY, y);
            this.force_update = true;
            End();
        }

        public void SetAbsolute(float x, float y, float z) {
            SetAbsolutePosition(x, y);
            SetAbsoluteZoom(z);
        }


        public void SetOffset(float x, float y, float z) {
            if (!Single.IsNaN(x)) {
                InternalTweenlerpAbsolute(Camera.OX, x);
                this.offset_x = x;
            }
            if (!Single.IsNaN(y)) {
                InternalTweenlerpAbsolute(Camera.OY, y);
                this.offset_y = y;
            }
            if (!Single.IsNaN(z)) {
                InternalTweenlerpAbsolute(Camera.OZ, z);
                this.offset_z = z;
            }

            this.has_transition_offset = false;
        }

        public void GetOffset(out float x, out float y, out float z) {
            x = this.offset_x;
            y = this.offset_y;
            z = this.offset_z;
        }


        public void ReplaceModifier(Modifier new_modifier) {
            if (new_modifier == null) throw new ArgumentNullException("new_modifier", "Invalid modifier");

            if (this.internal_modifier) {
                //free(this.modifier);
                this.internal_modifier = false;
            }

            this.modifier = new_modifier;
        }

        public Modifier GetModifier() {
            return this.modifier;
        }


        public void Move(float end_x, float end_y, float end_z) {
            InternalDropAnimation();

            this.tweenlerp.ChangeBoundsByIndex(Camera.PX, Single.NaN, end_x);
            this.tweenlerp.ChangeBoundsByIndex(Camera.PY, Single.NaN, end_y);
            this.tweenlerp.ChangeBoundsByIndex(Camera.PZ, Single.NaN, end_z);

            End();
        }

        public void MoveOffset(float end_x, float end_y, float end_z) {
            InternalDropAnimation();

            this.tweenlerp.ChangeBoundsByIndex(Camera.OX, Single.NaN, end_x);
            this.tweenlerp.ChangeBoundsByIndex(Camera.OY, Single.NaN, end_y);
            this.tweenlerp.ChangeBoundsByIndex(Camera.OZ, Single.NaN, end_z);

            this.has_transition_offset = true;
            End();
        }

        public void Slide(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z) {
            InternalDropAnimation();

            this.tweenlerp.ChangeBoundsByIndex(Camera.PX, start_x, end_x);
            this.tweenlerp.ChangeBoundsByIndex(Camera.PY, start_y, end_y);
            this.tweenlerp.ChangeBoundsByIndex(Camera.PZ, start_z, end_z);

            Repeat();
        }

        public void SlideOffset(float start_x, float start_y, float start_z, float end_x, float end_y, float end_z) {
            InternalDropAnimation();

            this.tweenlerp.ChangeBoundsByIndex(Camera.OX, start_x, end_x);
            this.tweenlerp.ChangeBoundsByIndex(Camera.OY, start_y, end_y);
            this.tweenlerp.ChangeBoundsByIndex(Camera.OZ, start_z, end_z);

            this.has_transition_offset = true;
            Repeat();
        }

        public void SlideX(float start, float end) {
            InternalDropAnimation();

            this.tweenlerp.ChangeBoundsByIndex(Camera.PX, start, end);
            this.tweenlerp.OverrideStartWithEndByIndex(Camera.PY);
            this.tweenlerp.OverrideStartWithEndByIndex(Camera.PZ);
            Repeat();
        }

        public void SlideXOffset(float start, float end) {
            InternalDropAnimation();

            this.tweenlerp.ChangeBoundsByIndex(Camera.OX, start, end);
            this.tweenlerp.OverrideStartWithEndByIndex(Camera.OY);
            this.tweenlerp.OverrideStartWithEndByIndex(Camera.OZ);
            this.has_transition_offset = true;
            Repeat();
        }

        public void SlideY(float start, float end) {
            InternalDropAnimation();

            this.tweenlerp.OverrideStartWithEndByIndex(Camera.PX);
            this.tweenlerp.ChangeBoundsByIndex(Camera.PY, start, end);
            this.tweenlerp.OverrideStartWithEndByIndex(Camera.PZ);
            Repeat();
        }

        public void SlideYOffset(float start, float end) {
            InternalDropAnimation();

            this.tweenlerp.OverrideStartWithEndByIndex(Camera.OX);
            this.tweenlerp.ChangeBoundsByIndex(Camera.OY, start, end);
            this.tweenlerp.OverrideStartWithEndByIndex(Camera.OZ);
            this.has_transition_offset = true;
            Repeat();
        }

        public void SlideZ(float start, float end) {
            InternalDropAnimation();

            this.tweenlerp.OverrideStartWithEndByIndex(Camera.PX);
            this.tweenlerp.OverrideStartWithEndByIndex(Camera.PY);
            this.tweenlerp.ChangeBoundsByIndex(Camera.PZ, start, end);
            Repeat();
        }

        public void SlideZOffset(float start, float end) {
            InternalDropAnimation();

            this.tweenlerp.OverrideStartWithEndByIndex(Camera.OX);
            this.tweenlerp.OverrideStartWithEndByIndex(Camera.OY);
            this.tweenlerp.ChangeBoundsByIndex(Camera.OZ, start, end);
            Repeat();
        }

        public void SlideTo(float x, float y, float z) {
            InternalDropAnimation();

            float start_x = this.tweenlerp.PeekValueByIndex(Camera.PX);
            float start_y = this.tweenlerp.PeekValueByIndex(Camera.PY);
            float start_z = this.tweenlerp.PeekValueByIndex(Camera.PZ);

            if (Single.IsNaN(x)) x = start_x;
            if (Single.IsNaN(y)) y = start_y;
            if (Single.IsNaN(z)) z = start_z;

            this.tweenlerp.ChangeBoundsByIndex(Camera.PX, start_x, x);
            this.tweenlerp.ChangeBoundsByIndex(Camera.PY, start_y, y);
            this.tweenlerp.ChangeBoundsByIndex(Camera.PZ, start_z, z);

            Repeat();
        }

        public void SlideToOffset(float x, float y, float z) {
            InternalDropAnimation();

            float start_x = this.tweenlerp.PeekValueByIndex(Camera.OX);
            float start_y = this.tweenlerp.PeekValueByIndex(Camera.OY);
            float start_z = this.tweenlerp.PeekValueByIndex(Camera.OZ);

            if (Single.IsNaN(x)) x = start_x;
            if (Single.IsNaN(y)) y = start_y;
            if (Single.IsNaN(z)) z = start_z;

            this.tweenlerp.ChangeBoundsByIndex(Camera.OX, start_x, x);
            this.tweenlerp.ChangeBoundsByIndex(Camera.OY, start_y, y);
            this.tweenlerp.ChangeBoundsByIndex(Camera.OZ, start_z, z);

            this.has_transition_offset = true;

            Repeat();
        }

        public bool FromLayout(Layout layout, string camera_name) {
            if (layout == null) {
                if (this.parent_layout == null) return false;
                layout = this.parent_layout;
            }

            CameraPlaceholder camera_placeholder = layout.GetCameraPlaceholder(camera_name);
            if (camera_placeholder == null) return false;

            if (camera_placeholder.enable_offset_zoom) {
                this.enable_offset_zoom = camera_placeholder.enable_offset_zoom;
            }

            if (camera_placeholder.animation != null) {
                this.SetAnimation(camera_placeholder.animation);
                return true;
            }

            if (!camera_placeholder.is_empty || camera_placeholder.has_duration) {
                SetTransitionDuration(camera_placeholder.duration_in_beats, camera_placeholder.duration);
            }

            if (camera_placeholder.is_empty) {
                return false;
            }
            InternalDropAnimation();

            if (camera_placeholder.has_parallax_offset_only) {
                SetOffset(
                    camera_placeholder.offset_x, camera_placeholder.offset_y, camera_placeholder.offset_z
                );
            } else if (camera_placeholder.move_offset_only) {
                MoveOffset(
                    camera_placeholder.to_offset_x, camera_placeholder.to_offset_y, camera_placeholder.to_offset_z
                );
                this.force_update = true;
            } else if (camera_placeholder.has_offset_from) {
                SlideOffset(
                    camera_placeholder.from_offset_x, camera_placeholder.from_offset_y, camera_placeholder.from_offset_z,
                    camera_placeholder.to_offset_x, camera_placeholder.to_offset_y, camera_placeholder.to_offset_z
                );
            } else if (camera_placeholder.has_offset_to) {
                SlideToOffset(
                     camera_placeholder.to_offset_x, camera_placeholder.to_offset_y, camera_placeholder.to_offset_z
                 );
            }

            if (camera_placeholder.move_only) {
                Move(
                    camera_placeholder.to_x, camera_placeholder.to_y, camera_placeholder.to_z
                );
                SetTransitionDuration(
                    camera_placeholder.duration_in_beats, camera_placeholder.duration
                );
                this.force_update = true;
            } else {
                if (camera_placeholder.has_from) {
                    Slide(
                        camera_placeholder.from_x, camera_placeholder.from_y, camera_placeholder.from_z,
                        camera_placeholder.to_x, camera_placeholder.to_y, camera_placeholder.to_z
                    );
                } else {
                    SlideTo(
                        camera_placeholder.to_x, camera_placeholder.to_y, camera_placeholder.to_z
                    );
                }
                SetTransitionDuration(
                    camera_placeholder.duration_in_beats, camera_placeholder.duration
                );
            }

            return true;
        }

        public void ToOrigin(bool should_slide) {
            InternalDropAnimation();

            if (should_slide) {
                SlideTo(0f, 0f, 1f);
                Repeat();
            } else {
                Move(0f, 0f, 1f);
            }
        }

        public void ToOriginOffset(bool should_slide) {
            InternalDropAnimation();

            if (should_slide) {
                SlideToOffset(0f, 0f, 1f);
                Repeat();
            } else {
                MoveOffset(0f, 0f, 1f);
            }
            this.has_transition_offset = true;
        }

        public Layout GetParentLayout() {
            return this.parent_layout;
        }

        public void SetParentLayout(Layout layout) {
            this.parent_layout = layout;
        }

        public void SetAnimation(AnimSprite animsprite) {
            if (this.animation != null) this.animation.Destroy();
            this.animation = animsprite != null ? animsprite.Clone() : null;
            this.transition_completed = true;
        }


        public void Repeat() {
            if (this.animation != null) {
                this.animation.Restart();
                this.animation.UpdateUsingCallback(this, true);
                return;
            }

            this.progress = 0.0;
            this.transition_completed = false;
            this.tweenlerp.Restart();
        }

        public void Stop() {
            this.progress = this.duration;
            this.transition_completed = true;

            if (this.animation != null) {
                this.animation.Stop();
                return;
            }

            this.parallax_x = this.tweenlerp.PeekValueByIndex(Camera.PX);
            this.parallax_y = this.tweenlerp.PeekValueByIndex(Camera.PY);
            this.parallax_z = this.tweenlerp.PeekValueByIndex(Camera.PZ);

            if (this.has_transition_offset) {
                this.offset_x = this.tweenlerp.PeekValueByIndex(Camera.OX);
                this.offset_y = this.tweenlerp.PeekValueByIndex(Camera.OY);
                this.offset_z = this.tweenlerp.PeekValueByIndex(Camera.OZ);
            }
        }

        public void End() {
            if (this.animation != null) {
                this.animation.ForceEnd();
                this.animation.UpdateUsingCallback(this, true);
                return;
            }

            this.tweenlerp.End();

            this.progress = this.duration;
            this.transition_completed = true;
            this.has_transition_offset = false;

            this.parallax_x = this.tweenlerp.PeekValueByIndex(Camera.PX);
            this.parallax_y = this.tweenlerp.PeekValueByIndex(Camera.PY);
            this.parallax_z = this.tweenlerp.PeekValueByIndex(Camera.PZ);

            if (this.has_transition_offset) {
                this.offset_x = this.tweenlerp.PeekValueByIndex(Camera.OX);
                this.offset_y = this.tweenlerp.PeekValueByIndex(Camera.OY);
                this.offset_z = this.tweenlerp.PeekValueByIndex(Camera.OZ);
            }
        }


        public int Animate(float elapsed) {
            if (this.animation != null) {
                int completed = this.animation.Animate(elapsed);
                if (completed < 1) this.animation.UpdateUsingCallback(this, true);
                return completed;
            }

            if (this.transition_completed) {
                if (this.force_update) {
                    this.force_update = false;
                    return 0;
                }

                this.has_transition_offset = false;
                return 1;
            }

            if (this.progress >= this.duration) {
                End();
                return 0;// required to apply the end values
            }

            double percent = this.progress / this.duration;
            this.progress += elapsed;

            this.tweenlerp.AnimatePercent(percent);
            return 0;
        }

        public void Apply(PVRContext pvrctx) {
            if (this.transition_completed || this.animation != null) {
                this.modifier.translate_x = this.parallax_x;
                this.modifier.translate_y = this.parallax_y;
                this.modifier.scale_x = this.parallax_z;
                this.modifier.scale_y = this.parallax_z;
            } else {
                float x = this.tweenlerp.PeekValueByIndex(Camera.PX);
                float y = this.tweenlerp.PeekValueByIndex(Camera.PY);
                float z = this.tweenlerp.PeekValueByIndex(Camera.PZ);

                if (!Single.IsNaN(x)) this.modifier.translate_x = x;
                if (!Single.IsNaN(y)) this.modifier.translate_y = y;
                if (!Single.IsNaN(z)) this.modifier.scale_x = this.modifier.scale_y = z;
            }

            if (pvrctx != null) {
                SH4Matrix matrix = pvrctx.CurrentMatrix;
                ApplyOffset(matrix);
                matrix.ApplyModifier(this.modifier);
                //pvrctx.Flush();
            }

        }

        public void ApplyOffset(SH4Matrix destination_matrix) {
            if (this.animation == null && this.has_transition_offset) {
                float x = this.tweenlerp.PeekValueByIndex(Camera.OX);
                float y = this.tweenlerp.PeekValueByIndex(Camera.OY);
                float z = this.tweenlerp.PeekValueByIndex(Camera.OZ);

                if (!Single.IsNaN(x)) this.offset_x = x;
                if (!Single.IsNaN(y)) this.offset_y = y;
                if (!Single.IsNaN(z)) this.offset_z = z;
            }

            destination_matrix.Translate(this.offset_x, this.offset_y);

            if (this.enable_offset_zoom && this.offset_z != 1f) {
                destination_matrix.ScaleSize(
                    this.half_viewport_width, this.half_viewport_height,
                    this.offset_z, this.offset_z
                );
            } else {
                destination_matrix.Scale(this.offset_z, this.offset_z);
            }
        }

        public bool IsCompleted() {
            if (this.animation != null) return this.animation.IsCompleted();
            if (this.transition_completed && this.force_update) return false;
            return this.progress >= this.duration;
        }

        public void DisableOffsetZoom(bool disabled) {
            this.enable_offset_zoom = !disabled;
        }

        public void SetProperty(int id, float value) {
            switch (id) {
                case VertexProps.SPRITE_PROP_X:
                    this.parallax_x = value;
                    break;
                case VertexProps.SPRITE_PROP_Y:
                    this.parallax_y = value;
                    break;
                case VertexProps.SPRITE_PROP_Z:
                    this.parallax_z = value;
                    break;
                case VertexProps.CAMERA_PROP_OFFSET_X:
                    this.offset_x = value;
                    break;
                case VertexProps.CAMERA_PROP_OFFSET_Y:
                    this.offset_y = value;
                    break;
                case VertexProps.CAMERA_PROP_OFFSET_Z:
                    this.offset_z = value;
                    break;
                case VertexProps.CAMERA_PROP_OFFSET_ZOOM:
                    this.enable_offset_zoom = value >= 1f;
                    break;
            }
        }


        public void DebugLogInfo() {
            Console.WriteLine("[LOG] camera offset: x=" + this.offset_x + " y=" + this.offset_y + " z=" + this.offset_z);
            Console.WriteLine("[LOG] camera position: x=" + this.modifier.translate_x + " y=" + this.modifier.translate_y + " z=" + this.modifier.scale_x);
        }

        public void InternalTweenlerpAbsolute(int index, float value) {
            this.tweenlerp.ChangeBoundsByIndex(index, value, value);
        }

        private void InternalDropAnimation() {
            if (this.animation != null) {
                this.animation.Destroy();
                this.animation = null;
            }
        }

    }

}
