using System;
using Engine.Font;
using Engine.Image;
using Engine.Utils;

namespace Engine.Animation {

    public class AnimSprite {

        private static Map<AnimSprite> POOL = new Map<AnimSprite>();
        private static int IDS = 0;

        private AnimSprite() { }


        private int id;
        internal string name;
        private AtlasEntry[] frames;
        private int frame_count;
        private double frame_time;
        private int length;
        private int loop;
        private int loop_progress;
        private bool has_looped;
        private bool disable_loop;
        private double progress;
        private int current_index;
        private int current_offset;
        private float delay;
        private double delay_progress;
        private bool delay_active;
        private bool is_empty;
        private AnimList.AlternateEntry[] alternate_set;
        private int alternate_size;
        private bool alternate_per_loop;
        private int alternate_index;
        private MacroExecutor macroexecutor;
        private TweenLerp tweenlerp;


        public static AnimSprite InitFromAtlas(float frame_rate, int loop, Atlas atlas, string prefix, bool has_number_suffix) {
            LinkedList<AtlasEntry> frames = new LinkedList<AtlasEntry>();

            AnimList.ReadEntriesToFramesArray(frames, prefix, has_number_suffix, atlas, 0, -1);
            int frame_count = frames.Count();

            //
            // Unintended behavior, the caller should build the static animation
            //
            /*if (frame_count < 1) {
                AtlasEntry atlas_entry = null;

                if (has_number_suffix) {
                    // attempt to obtain one frame without number suffix
                    AtlasEntry atlas_entry = atlas.GetEntry(prefix);
                    if (atlas_entry) {
                        frames.AddItem(atlas_entry);
                        frame_count++;
                    }
                }

                if (atlas_entry == null) {
                    frames.Destroy();
                    return null;
                }
            }*/

            if (frame_count < 1) {
                frames.Destroy();
                return null;
            }

            AnimSprite animsprite = AnimSprite.InternalInit(prefix, loop, frame_rate);

            animsprite.frame_count = frame_count;
            animsprite.frames = frames.ToSolidArray();
            frames.Destroy();

            return animsprite;
        }

        public static AnimSprite InitFromAtlasEntry(AtlasEntry atlas_entry, bool loop_indefinitely, float frame_rate) {
            int loop = loop_indefinitely ? 0 : 1;
            AnimSprite animsprite = AnimSprite.InternalInit(atlas_entry.name, loop, frame_rate);
            animsprite.frame_count = 1;
            animsprite.frames = new AtlasEntry[1];
            animsprite.frames[0] = atlas_entry.Clone();
            return animsprite;
        }

        public static AnimSprite InitFromAnimlist(AnimList animlist, string animation_name) {
            for (int i = 0 ; i < animlist.entries_count ; i++) {
                if (animlist.entries[i].name == animation_name) {
                    return AnimSprite.Init(animlist.entries[i]);
                }
            }
            return null;
        }

        public static AnimSprite InitFromMacroexecutor(string name, int loop, MacroExecutor macroexecutor) {
            AnimSprite animsprite = AnimSprite.InternalInit(name, loop, 0);
            animsprite.macroexecutor = macroexecutor;
            return animsprite;
        }

        public static AnimSprite InitAsEmpty(string name) {
            AnimSprite animsprite = AnimSprite.InternalInit(name, 0, 0);
            animsprite.is_empty = true;
            return animsprite;
        }

        public static AnimSprite Init(AnimListItem animlist_item) {
            AnimSprite animsprite;

            if (animlist_item.is_tweenlerp) {
                animsprite = AnimSprite.InternalInit(animlist_item.name, animlist_item.loop, -1);
                animsprite.tweenlerp = TweenLerp.Init2(animlist_item);
                return animsprite;
            }

            if (animlist_item.alternate_set_size > 0) {
                int frame_count = 0;
                for (int i = 0 ; i < animlist_item.alternate_set_size ; i++)
                    frame_count += animlist_item.alternate_set[i].length;

                if (frame_count > animlist_item.frame_count)
                    throw new Exception("Invalid animlist_item.alternate_set");
            }

            animsprite = AnimSprite.InternalInit(
               animlist_item.name, animlist_item.loop, animlist_item.frame_rate
           );

            animsprite.alternate_per_loop = animlist_item.alternate_per_loop;
            animsprite.alternate_size = animlist_item.alternate_set_size;
            animsprite.alternate_set = CloneUtils.CloneArray(
                animlist_item.alternate_set,
                animlist_item.alternate_set_size
            );
            if (animlist_item.alternate_no_random) animsprite.alternate_index = 0;

            animsprite.frame_count = animlist_item.frame_count;
            animsprite.frames = CloneUtils.CloneArray(animlist_item.frames, animlist_item.frame_count);

            if (animlist_item.instructions_count > 0) {
                MacroExecutorInstruction[] instructions = CloneUtils.CloneArray(animlist_item.instructions, animlist_item.instructions_count);
                for (int i = 0 ; i < animlist_item.instructions_count ; i++) {
                    instructions[i].values = CloneUtils.CloneArray(instructions[i].values, instructions[i].values_size);
                }

                animsprite.macroexecutor = new MacroExecutor(
                    instructions, animlist_item.instructions_count, animsprite.frames, animlist_item.frame_count
                );

                animsprite.macroexecutor.SetRestartInFrame(
            animlist_item.frame_restart_index, animlist_item.frame_allow_size_change
        );
            }

            return animsprite;
        }

        public void Destroy() {
            //if (this == null) return;

            //if (this.frames != null) {
            //    for (int i=0 ; i<this.frame_count ; i++) {
            //        free(this.frames[i].name);
            //    }
            //    free(this.frames);
            //}

            //if (this.alternate_set != null)
            //    free(this.alternate_set);

            if (this.macroexecutor != null)
                this.macroexecutor.Destroy();

            if (this.tweenlerp != null)
                this.tweenlerp.Destroy();

            AnimSprite.POOL.Delete(this.id);

            //free(animsprite);
        }

        public AnimSprite Clone() {
            //if (this == null) return null;

            AnimSprite copy = new AnimSprite() {
                id = AnimSprite.IDS++,
                name = this.name,

                frames = this.frames,
                frame_count = this.frame_count,

                frame_time = this.frame_time,
                length = this.length,

                loop = this.loop,
                loop_progress = this.loop_progress,
                has_looped = this.has_looped,
                disable_loop = this.disable_loop,

                progress = this.progress,

                current_index = this.current_index,
                current_offset = this.current_offset,

                delay = this.delay,
                delay_progress = this.delay_progress,
                delay_active = this.delay_active,

                is_empty = this.is_empty,

                alternate_set = this.alternate_set,
                alternate_size = this.alternate_size,
                alternate_per_loop = this.alternate_per_loop,
                alternate_index = this.alternate_index,

                macroexecutor = this.macroexecutor,
                tweenlerp = this.tweenlerp,
            };
            //if (copy == null) return null;

            copy.id = AnimSprite.IDS++;
            AnimSprite.POOL.Set(copy.id, copy);

            copy.alternate_set = CloneUtils.CloneArray(this.alternate_set, this.alternate_size);
            copy.frames = CloneUtils.CloneArray(this.frames, this.frame_count);

            if (copy.macroexecutor != null) {
                copy.macroexecutor = this.macroexecutor.Clone(false);
                copy.macroexecutor.frames = copy.frames;
                copy.macroexecutor.frame_count = copy.frame_count;
            }

            if (copy.tweenlerp != null) {
                copy.tweenlerp = this.tweenlerp.Clone();
            }

            return copy;
        }

        public void SetLoop(int loop) {
            this.loop = loop;
        }

        public void Restart() {
            if (this.is_empty) return;

            this.loop_progress = 0;
            this.has_looped = false;
            this.disable_loop = false;
            this.delay_progress = 0.0;
            this.delay_active = this.delay > 0;

            if (this.macroexecutor != null) {
                this.macroexecutor.Restart();
                return;
            }

            if (this.tweenlerp != null) {
                this.tweenlerp.Restart();
                return;
            }

            this.progress = 0;
            this.current_index = 0;

            InternalAlternateChoose(false);
        }

        public int Animate(float elapsed) {
            if (Single.IsNaN(elapsed)) throw new NaNArgumentError("invalid elapsed argument");
            if (this.is_empty) return 0;
            if (Single.IsNaN(this.loop_progress)) return 1;
            if (this.loop > 0 && this.loop_progress >= this.loop) return 1;

            if (this.delay_active && this.delay > 0) {
                this.delay_progress += elapsed;
                if (this.delay_progress < this.delay) return 0;

                elapsed = (float)(this.delay_progress - this.delay);
                this.delay_active = false;
                this.delay_progress = 0.0;
            }

            this.progress += elapsed;

            if (this.macroexecutor != null || this.tweenlerp != null) {
                int completed;

                if (this.macroexecutor != null)
                    completed = this.macroexecutor.Animate(elapsed);
                else
                    completed = this.tweenlerp.Animate(elapsed);

                if (completed > 0) {
                    this.delay_active = this.delay > 0;
                    this.delay_progress = 0;
                    this.has_looped = true;

                    if (this.disable_loop) {
                        this.loop_progress = Int32.MaxValue;
                        return 1;
                    }

                    if (this.loop > 0) {
                        this.loop_progress++;
                        if (this.loop_progress >= this.loop) return 0;
                    }

                    if (this.macroexecutor != null) this.macroexecutor.Restart();
                    else this.tweenlerp.Restart();
                }
                return 0;
            }

            double new_index = this.progress / this.frame_time;
            this.current_index = (int)new_index;

            if (this.current_index >= this.frame_count) {
                this.has_looped = true;
                if (this.disable_loop) {
                    this.loop_progress = Int32.MaxValue;
                    return 1;
                }
                if (this.loop > 0) {
                    this.loop_progress++;
                    if (this.loop_progress >= this.loop) return 1;
                }
                InternalAlternateChoose(true);
                this.delay_active = this.delay > 0;
                this.current_index = 0;
                this.progress = 0;
            }

            return 0;
        }

        public string GetName() {
            return this.name;
        }


        public bool IsCompleted() {
            if (this.is_empty) return true;
            if (this.loop < 1) return false;
            if (this.loop_progress >= this.loop) return true;

            if (this.macroexecutor != null)
                return this.macroexecutor.IsCompleted();
            else if (this.tweenlerp != null)
                return this.tweenlerp.IsCompleted();
            else
                return this.current_index >= this.frame_count;
        }

        public bool IsFrameAnimation() {
            return/* this != null &&*/ this.macroexecutor == null && this.tweenlerp == null;
        }

        public bool HasLooped() {
            bool has_looped = this.has_looped;
            if (has_looped) this.has_looped = false;
            return has_looped;
        }

        public void DisableLoop() {
            this.disable_loop = true;
        }

        public void Stop() {
            this.loop_progress = Int32.MaxValue;
        }


        public void ForceEnd() {
            if (this.is_empty) return;

            if (this.macroexecutor != null)
                this.macroexecutor.ForceEnd(null);
            else if (this.tweenlerp != null)
                this.tweenlerp.Restart();
            else
                this.current_index = this.frame_count - 1;

            this.delay_active = false;

            if (this.loop != 0) this.loop_progress++;
        }

        public void ForceEnd2(Sprite sprite) {
            if (this.is_empty) return;
            ForceEnd();

            if (sprite == null) return;

            UpdateSprite(sprite, false);
        }

        public void ForceEnd3(StateSprite statesprite) {
            if (this.is_empty) return;
            ForceEnd();

            if (statesprite == null) return;

            UpdateStatesprite(statesprite, false);
        }

        public void SetDelay(float delay_milliseconds) {
            this.delay = delay_milliseconds;
            this.delay_progress = 0.0;// ¿should clear the delay progress?
            this.delay_active = true;
        }

        public void UpdateSprite(Sprite sprite, bool stack_changes) {
            if (this.is_empty) return;

            if (this.macroexecutor != null)
                this.macroexecutor.StateApply(sprite, !stack_changes);
            else if (this.tweenlerp != null)
                this.tweenlerp.VertexSetProperties(sprite);
            else
                InternalApplyFrame(sprite, this.current_index);
        }

        public void UpdateStatesprite(StateSprite statesprite, bool stack_changes) {
            if (this.is_empty) return;

            if (this.macroexecutor != null)
                this.macroexecutor.StateApply2(statesprite, !stack_changes);
            else if (this.tweenlerp != null)
                this.tweenlerp.VertexSetProperties(statesprite);
            else
                InternalApplyFrame2(statesprite, this.current_index);
        }

        public void UpdateTextsprite(TextSprite textsprite, bool stack_changes) {
            if (this.is_empty) return;

            if (this.tweenlerp != null)
                this.tweenlerp.VertexSetProperties(textsprite);
            else if (this.macroexecutor != null)
                this.macroexecutor.StateApply4(textsprite, !stack_changes);
        }

        public void UpdateModifier(Modifier modifier, bool stack_changes) {
            if (this.macroexecutor != null)
                this.macroexecutor.StateToModifier(modifier, !stack_changes);
            if (this.tweenlerp != null)
                this.tweenlerp.VertexSetProperties(
                    modifier
                );
        }

        public void UpdateDrawable(Drawable drawable, bool stack_changes) {
            if (this.macroexecutor != null)
                this.macroexecutor.StateApply3(drawable, !stack_changes);
            else if (this.tweenlerp != null)
                this.tweenlerp.VertexSetProperties(drawable);
        }

        public void UpdateUsingCallback(/*object private_data, */ISetProperty setter_callback, bool stack_changes) {
            if (this.macroexecutor != null) {
                this.macroexecutor.StateApply5(
                     setter_callback, !stack_changes
                );
            } else if (this.tweenlerp != null) {
                this.tweenlerp.VertexSetProperties(setter_callback);
            } else {
                setter_callback.SetProperty(
                    VertexProps.SPRITE_PROP_FRAMEINDEX, this.current_index + this.current_offset
                );
            }
        }

        public MacroExecutor GetMacroexecutor() {
            return this.macroexecutor;
        }


        public bool Rollback(float elapsed) {
            if (this.progress <= 0.0) return true;// completed

            if (this.macroexecutor != null) {
                // imposible rollback a macroexecutor animation
                this.progress = 0.0;
                return true;
            } else if (this.tweenlerp != null) {
                // tweenlerp animation
                this.tweenlerp.AnimateTimestamp(this.progress);
            } else {
                // frame animation
                this.current_index = (int)Math.Truncate(this.progress / this.frame_time);
            }

            this.progress -= elapsed;
            return false;
        }

        public AtlasEntry HelperGetFirstFrameAtlasEntry() {
            if (this.frame_count < 1) return null;
            return this.frames[0];
        }


        private void InternalApplyFrame(Sprite sprite, int index) {
            if (this.frames == null) return;
            if (index < 0 || index >= this.frame_count) return;

            index += this.current_offset;

            AtlasEntry frame = this.frames[index];
            sprite.SetOffsetSource(frame.x, frame.y, frame.width, frame.height);
            sprite.SetOffsetFrame(frame.frame_x, frame.frame_y, frame.frame_width, frame.frame_height);
            sprite.SetOffsetPivot(frame.pivot_x, frame.pivot_y);
        }

        private void InternalApplyFrame2(StateSprite statesprite, int index) {
            if (this.frames == null) return;
            if (index < 0 || index >= this.frame_count) return;

            index += this.current_offset;

            AtlasEntry frame = this.frames[index];
            statesprite.SetOffsetSource(
                frame.x, frame.y, frame.width, frame.height
            );
            statesprite.SetOffsetFrame(
                frame.frame_x, frame.frame_y, frame.frame_width, frame.frame_height
            );
            statesprite.SetOffsetPivot(
                frame.pivot_x, frame.pivot_y
            );
        }

        private void InternalAlternateChoose(bool loop) {
            if (this.alternate_size < 2) return;
            if (loop && !this.alternate_per_loop) return;

            int index;

            if (this.alternate_index < 0) {
                index = Math2D.RandomInt(0, this.alternate_size - 1);
            } else {
                this.alternate_index++;
                if (this.alternate_index >= this.alternate_size) this.alternate_index = 0;
                index = this.alternate_index;
            }


            AnimList.AlternateEntry alternate = this.alternate_set[index];

            this.current_offset = alternate.index;
            this.frame_count = alternate.length;
        }


        private static AnimSprite InternalInit(string name, int loop, float frame_rate) {
            float frame_time = frame_rate > 0 ? (1000.0f / frame_rate) : 0;
            //if (name != null) name = strdup(name);

            AnimSprite animsprite = new AnimSprite() {
                id = AnimSprite.IDS++,
                name = name,

                frames = null,
                frame_count = 0,

                frame_time = frame_time,
                length = 0,

                loop = loop,
                loop_progress = 0,
                has_looped = false,
                disable_loop = false,

                progress = 0,

                current_index = 0,
                current_offset = 0,

                delay = 0,
                delay_progress = 0,
                delay_active = false,

                is_empty = false,

                alternate_set = null,
                alternate_size = 0,
                alternate_per_loop = false,
                alternate_index = -1,

                macroexecutor = null,
                tweenlerp = null,
            };

            AnimSprite.POOL.Set(animsprite.id, animsprite);
            return animsprite;
        }

    }

}
