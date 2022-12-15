using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Image;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;

namespace Engine.Game {

    public class Countdown : IDraw, IAnimate {

        private const string READY_CONFIRM = "ready-confirm";
        private const string THREE = "three";// in the funkin only the sounds "2", "1", "go!" have sprites
        private const string READY = "ready";
        private const string SET = "set";
        private const string GO = "go";
        private const string DEFAULT_ANIMATION = "countdown";


        private bool valid_model;
        private bool valid_state;
        private StateSprite statesprite;
        private float beat_duration;
        private float height;
        private TweenKeyframe default_animation;
        private bool default_animate;
        private float animation_speed;
        private bool static_ready;
        private bool ready_only;
        private double progress;
        private double timer;
        private SoundPlayer sound_three;
        private SoundPlayer sound_two;
        private SoundPlayer sound_one;
        private SoundPlayer sound_go;
        private Drawable drawable;


        public Countdown(ModelHolder modelholder, float height) {
            FS.FolderStackPush();
            FS.SetWorkingFolder("/assets/common/sound/", false);


            this.valid_model = !modelholder.IsInvalid();
            this.valid_state = false;

            this.statesprite = StateSprite.InitFromTexture(null);
            this.beat_duration = Math2D.BeatsPerMinuteToBeatPerMilliseconds(100);

            this.height = height;
            this.default_animation = null;
            this.default_animate = false;
            this.animation_speed = 1.0f;

            this.static_ready = false;
            this.ready_only = false;
            this.progress = 4;
            this.timer = 0.0;

            this.sound_three = SoundPlayer.Init("intro3.ogg");
            this.sound_two = SoundPlayer.Init("intro2.ogg");
            this.sound_one = SoundPlayer.Init("intro1.ogg");
            this.sound_go = SoundPlayer.Init("introGo.ogg");

            this.drawable = null;


            this.drawable = new Drawable(200f, this, this);

            FS.FolderStackPop();

            if (!this.valid_model) return;

            this.statesprite.SetVisible(false);
            this.statesprite.StateAdd(
                modelholder, Countdown.READY_CONFIRM, Countdown.READY_CONFIRM
            );
            this.statesprite.StateAdd(
                modelholder, Countdown.THREE, Countdown.THREE
            );
            this.statesprite.StateAdd(
                modelholder, Countdown.READY, Countdown.READY
            );
            this.statesprite.StateAdd(
                 modelholder, Countdown.SET, Countdown.SET
            );
            this.statesprite.StateAdd(
                 modelholder, Countdown.GO, Countdown.GO
            );

        }

        public void Destroy() {
            Luascript.DropShared(this);

            this.statesprite.Destroy();
            if (this.default_animation != null) this.default_animation.Destroy();
            this.sound_three.Destroy();
            this.sound_two.Destroy();
            this.sound_one.Destroy();
            this.sound_go.Destroy();
            this.drawable.Destroy();
            //free(this);
        }


        public void SetLayoutViewport(float width, float height) {
            if (!this.valid_model) return;
            float draw_width = 0, draw_height = 0;

            LinkedList<StateSpriteState> state_list = this.statesprite.StateList();


            foreach (StateSpriteState state in state_list) {
                ImgUtils.GetStateSpriteOriginalSize(state, ref draw_width, ref draw_height);
                ImgUtils.CalcSize(draw_width, draw_height, -1, this.height, out draw_width, out draw_height);
                state.draw_width = draw_width;
                state.draw_height = draw_height;
                state.offset_x = state.draw_width / -2.0f;
                state.offset_y = state.draw_height / -2.0f;
            }

            this.statesprite.SetDrawLocation(width / 2, height / 2);
        }

        public void SetBpm(float bpm) {
            this.beat_duration = Math2D.BeatsPerMinuteToBeatPerMilliseconds(bpm);
            this.animation_speed = 1000 / this.beat_duration;
        }

        public void SetDefaultAnimation(AnimList animlist) {
            if (animlist == null) return;
            AnimListItem animlist_item = animlist.GetAnimation(Countdown.DEFAULT_ANIMATION);
            if (animlist_item == null) return;

            this.default_animation = TweenKeyframe.Init2(animlist_item);
        }

        public void SetDefaultAnimation2(TweenKeyframe tweenkeyframe) {
            if (this.default_animation != null) this.default_animation.Destroy();
            this.default_animation = tweenkeyframe != null ? tweenkeyframe.Clone() : null;
        }


        public Drawable GetDrawable() {
            return this.drawable;
        }


        public bool Ready() {
            if (!this.valid_model) return false;

            this.progress = 0;
            this.ready_only = true;
            this.static_ready = false;
            this.valid_state = this.statesprite.StateToggle(Countdown.READY_CONFIRM);

            // restore statesprite alpha
            this.statesprite.SetAlpha(1.0f);

            if (!this.valid_state) {
                this.static_ready = true;
                this.valid_state = this.statesprite.StateToggle(Countdown.READY);
            }

            if (this.valid_state) {
                this.statesprite.AnimationRestart();
                this.statesprite.Animate(0);
            }


            return this.valid_state;
        }

        public bool Start() {
            if (this.sound_three != null) this.sound_three.Replay();
            if (!this.valid_model) return false;
            InternalToggle(Countdown.THREE);
            this.ready_only = false;
            this.static_ready = false;
            this.progress = 0.0;
            this.timer = 0.0;

            return this.valid_state;
        }

        public bool HasEnded() {
            return this.progress > 3 && !this.ready_only;
        }


        public int Animate(float elapsed) {
            if (this.static_ready) return 1;
            if (this.ready_only) return this.statesprite.Animate(elapsed);
            if (this.progress > 4) return 1;

            int completed;

            if (this.default_animate) {
                double percent = this.timer / this.beat_duration;
                if (percent > 1.0) percent = 1.0;

                this.default_animation.AnimatePercent(percent);
                this.default_animation.VertexSetProperties(
                     this.statesprite
                );
                completed = percent >= 1.0 ? 1 : 0;
            } else {
                completed = this.statesprite.Animate(elapsed * this.animation_speed);
            }

            if (this.timer >= this.beat_duration) {
                this.timer -= this.beat_duration;// this keeps the timer in sync
                this.progress++;

                switch (this.progress) {
                    case 1:
                        if (this.sound_three != null) this.sound_three.Stop();
                        if (this.sound_two != null) this.sound_two.Replay();
                        InternalToggle(Countdown.READY);
                        break;
                    case 2:
                        if (this.sound_two != null) this.sound_two.Stop();
                        if (this.sound_one != null) this.sound_one.Replay();
                        InternalToggle(Countdown.SET);
                        break;
                    case 3:
                        if (this.sound_one != null) this.sound_one.Stop();
                        if (this.sound_go != null) this.sound_go.Replay();
                        InternalToggle(Countdown.GO);
                        break;
                }

                this.statesprite.AnimationRestart();
            }

            this.timer += elapsed;

            return completed;
        }

        public void Draw(PVRContext pvrctx) {
            if (!this.valid_model || !this.valid_state) return;
            if (!this.ready_only && this.progress > 3) return;

            this.statesprite.Draw(pvrctx);
        }



        private void InternalToggle(string state_name) {
            this.valid_state = this.statesprite.StateToggle(state_name);
            if (!this.valid_state) return;

            if (this.statesprite.StateGet().animation != null) {
                this.default_animate = false;
            } else if (this.default_animation != null) {
                this.default_animate = true;
            } else {
                this.default_animate = false;
            }
        }

    }
}
