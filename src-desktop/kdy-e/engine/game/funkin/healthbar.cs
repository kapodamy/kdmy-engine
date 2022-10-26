using System;
using Engine.Animation;
using Engine.Game.Common;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game {

    public class HealthBar : IDraw, IAnimate {

        private const string LENGTH_NORMAL = "normal";
        private const string LENGTH_EXTRA = "extra";
        private const string LENGTH_LONG = "long";

        public const uint DEFAULT_COLOR_BACKGROUND = 0x000000;// black
        public const uint DEFAULT_COLOR_DAD = 0xFF0000;// red
        public const uint DEFAULT_COLOR_BOYFRIEND = 0x00FF00;// green


        public const int DEFAULT_DIMMEN = 20;// 20px in a 1280x720 screen
        public const int DEFAULT_BORDER_SIZE = 4;// 4px in a 1280x720 screen

        public const int DEFAULT_ICON_SIZE = 80;//80px in a 1280x720 screen

        private const float SCALE_TO_DIMMEN_ICON = 80f / HealthBar.DEFAULT_DIMMEN;
        private const float SCALE_TO_DIMMEN_BORDER = 4f / HealthBar.DEFAULT_DIMMEN;
        private const float SCALE_TO_DIMMEN_BAR = 1.0f;

        private const string ICON_PREFIX_WINNER = "winner";
        private const string ICON_PREFIX_WINNING = "winning";
        private const string ICON_PREFIX_NEUTRAL = "neutral";
        private const string ICON_PREFIX_LOOSING = "loosing";
        private const string ICON_PREFIX_LOOSER = "looser";

        private const string WARNING_ALT_SUFFIX = " alt";
        private const string WARNING_FAST_DRAIN = "fastDrain";
        private const string WARNING_SLOW_DRAIN = "slowDrain";
        private const string WARNING_LOCKED = "locked";
        private const string WARNING_OPPONENT_RECOVER = "opponentRecover";

        public const float CHARACTER_WARNING_PERCENT = 0.25f;// warn if less or equal to 25%
        public const float HEALTH_TRANSITION_RATIO = 8;// the transition duration in BMP/N

        public const float RATIO_SIZE_NORMAL = 600;// 600px in a 1280x720 screen
        public const float RATIO_SIZE_EXTRA = HealthBar.RATIO_SIZE_NORMAL;
        public const float RATIO_SIZE_LONG = 1180;// 1180px in a 1280x70 screen

        private static readonly ModelHolder STUB_MODELHOLDER = ModelHolder.Init2(0x000000, null, null);

        private static readonly float[] LOW_HEALTH_WARN_COLOR = { 1.0f, 0.0f, 0.0f, 0.5f };// rgba: half-transparent red
        private const float LOW_HEALTH_PERCENT = 0.10f;// warn if less or equal to 10%
        private const float LOW_HEALTH_FLASH_RATIO = 8f;// active flash duration in BMP/N
        private const string UI_ICON_BUMP = "healthbar_icon_bump";// picked from UI animlist
        public const string WARNING_MODEL = "/assets/common/image/week-round/healthbar_warns.xml";
        private const string INTERNAL_STATE_NAME = "______healthbar-state_____";



        private float x;
        private float y;
        private float z;
        private float length;
        private float dimmen;
        private float border;
        private float icon_overlap;
        private float warn_height;
        private float lock_height;
        private bool is_vertical;
        private bool enable_overlap;
        private StateSprite sprite_background;
        private StateSprite sprite_bar_opponent;
        private StateSprite sprite_bar_player;
        private StateSprite sprite_icon_opponent;
        private StateSprite sprite_icon_player;
        private bool flip_icon_opponent;
        private bool flip_icon_player;
        private float health_bar_length;
        private bool extra_enabled;
        private float extra_translation;
        private string selected_state_player;
        private string selected_state_opponent;
        private AnimSprite bump_animation_opponent;
        private AnimSprite bump_animation_player;
        private Modifier bump_modifier_opponent;
        private Modifier bump_modifier_player;
        private bool enable_bump;
        private LinkedList<TextureResolution> resolutions_player;
        private LinkedList<TextureResolution> resolutions_opponent;
        private float layout_width;
        private float layout_height;
        private BeatWatcher beatwatcher;
        private bool transition_enabled;
        private TweenLerp tweenlerp;
        private float last_health;
        private string prefix_state_player;
        private string prefix_state_opponent;
        private AnimSprite drawable_animation;
        private StateSprite sprite_warnings;
        private bool enable_warnings;
        private bool enable_flash_warning;
        private float low_health_flash_warning;
        private bool has_warning_locked;
        private bool has_warning_drain;
        private int has_warning_opponent_recover;
        private float warning_locked_position;
        private float warning_drain_x;
        private float warning_drain_y;
        private Modifier modifier;
        private Drawable drawable;
        private bool first_init;


        public HealthBar(float x, float y, float z, float length, float dimmen, float border, float icon_overlap, float warn_height, float lock_height) {

            this.x = x; this.y = y; this.z = z; this.length = length; this.dimmen = dimmen; this.border = border; this.icon_overlap = icon_overlap; this.warn_height = warn_height; this.lock_height = lock_height;

            this.is_vertical = false;
            this.enable_overlap = true;

            this.sprite_background = StateSprite.InitFromVertexColor(HealthBar.DEFAULT_COLOR_BACKGROUND);

            this.sprite_bar_opponent = StateSprite.InitFromVertexColor(HealthBar.DEFAULT_COLOR_DAD);
            this.sprite_bar_player = StateSprite.InitFromVertexColor(HealthBar.DEFAULT_COLOR_BOYFRIEND);

            this.sprite_icon_opponent = StateSprite.InitFromTexture(null);
            this.sprite_icon_player = StateSprite.InitFromTexture(null);

            this.flip_icon_opponent = false;
            this.flip_icon_player = true;

            this.health_bar_length = length - (border * 2);
            this.extra_enabled = false;
            this.extra_translation = 0f;

            // note= in C replace "Symbol" with an unique constantselected_state_background= Symbol;
            this.selected_state_player = INTERNAL_STATE_NAME;
            this.selected_state_opponent = INTERNAL_STATE_NAME;

            this.bump_animation_opponent = null;
            this.bump_animation_player = null;

            this.bump_modifier_opponent = new Modifier() { };
            this.bump_modifier_player = new Modifier() { };
            this.enable_bump = true;

            this.resolutions_player = new LinkedList<TextureResolution>();
            this.resolutions_opponent = new LinkedList<TextureResolution>();

            this.layout_width = Funkin.SCREEN_RESOLUTION_WIDTH;
            this.layout_height = Funkin.SCREEN_RESOLUTION_HEIGHT;

            this.beatwatcher = new BeatWatcher() { };

            this.transition_enabled = true;
            this.tweenlerp = TweenLerp.Init();

            this.last_health = Single.NaN;
            this.prefix_state_player = HealthBar.ICON_PREFIX_NEUTRAL;
            this.prefix_state_opponent = HealthBar.ICON_PREFIX_NEUTRAL;

            this.drawable_animation = null;

            this.sprite_warnings = StateSprite.InitFromTexture(null);
            this.enable_warnings = true;
            this.enable_flash_warning = false;
            this.low_health_flash_warning = 0.0f;

            this.has_warning_locked = false;
            this.has_warning_drain = false;
            this.has_warning_opponent_recover = -1;
            this.warning_locked_position = 0;
            this.warning_drain_x = 0;
            this.warning_drain_y = 0;

            this.modifier = null;
            this.drawable = null;

            this.first_init = true;


            this.beatwatcher.Reset(true, 100f);

            this.bump_modifier_opponent.Clear();
            this.bump_modifier_opponent.x = x;
            this.bump_modifier_opponent.x = y;

            this.bump_modifier_player.Clear();
            this.bump_modifier_player.x = x;
            this.bump_modifier_player.x = y;

            this.sprite_icon_opponent.SetDrawLocation(x, y);
            this.sprite_icon_player.SetDrawLocation(x, y);
            this.sprite_background.SetDrawLocation(x, y);
            this.sprite_bar_opponent.SetDrawLocation(x, y);
            this.sprite_bar_player.SetDrawLocation(x, y);

            // hide from the PVR backend, draw these sprites manually
            this.sprite_icon_opponent.SetVisible(false);
            this.sprite_icon_player.SetVisible(false);
            this.sprite_background.SetVisible(false);
            this.sprite_bar_opponent.SetVisible(false);
            this.sprite_bar_player.SetVisible(false);

            this.sprite_bar_opponent.CropEnable(true);
            this.sprite_bar_player.CropEnable(true);

            this.drawable = new Drawable(z, this, this);

            this.modifier = this.drawable.GetModifier();
            this.modifier.x = x;
            this.modifier.y = y;

            float transition_ms = this.beatwatcher.tick / HealthBar.HEALTH_TRANSITION_RATIO;
            this.tweenlerp.AddEaseOut(-1, 0f, 0f, transition_ms);

        }

        public void Destroy() {
            this.drawable.Destroy();
            if (this.drawable_animation != null) this.drawable_animation.Destroy();

            //if (this.selected_state_player != HealthBar. INTERNAL_STATE_NAME) free(this.selected_state_player);
            //if (this.selected_state_opponent != HealthBar.INTERNAL_STATE_NAME) free(this.selected_state_opponent);

            this.tweenlerp.Destroy();

            if (this.bump_animation_opponent != null) this.bump_animation_opponent.Destroy();
            if (this.bump_animation_player != null) this.bump_animation_player.Destroy();

            this.sprite_background.Destroy();
            this.sprite_bar_opponent.Destroy();
            this.sprite_bar_player.Destroy();
            this.sprite_icon_opponent.Destroy();
            this.sprite_icon_player.Destroy();

            this.sprite_warnings.Destroy();

            // dispose the resolution list, list items and state names

            //foreach (TextureResolution item in this.resolutions_opponent) {
            //    free(item.state_name);
            //    free(item);
            //}
            this.resolutions_opponent.Destroy();

            //foreach (TextureResolution item in this.resolutions_player) {
            //    free(item.state_name);
            //    free(item);
            //}
            this.resolutions_player.Destroy();

            //free(this);
        }


        public void SetLayoutSize(float width, float height) {
            this.layout_width = width;
            this.layout_height = height;
        }

        public void EnableExtraLength(bool extra_enabled) {
            this.extra_enabled = extra_enabled;

            // recalculate everything
            float last_health = this.last_health;
            this.last_health = Single.NaN;
            if (Single.IsNaN(last_health)) return;
            InternalCalcHealthPositions(last_health);
        }

        public void EnableVertical(bool enable_vertical) {
            this.is_vertical = enable_vertical;
            InternalCalcDimmensions();
        }


        public int StateOpponentAdd(ModelHolder icon_mdlhldr, ModelHolder bar_mdlhldr, string state_name) {
            return InternalAddChrctrState(
                this.sprite_icon_opponent, this.sprite_bar_opponent,
                icon_mdlhldr, bar_mdlhldr,
                this.resolutions_opponent,
                state_name
            );
        }

        public int StateOpponentAdd2(ModelHolder icon_mdlhldr, uint bar_color_rbb8, string state_name) {
            HealthBar.STUB_MODELHOLDER.vertex_color_rgb8 = bar_color_rbb8;
            return InternalAddChrctrState(
                this.sprite_icon_opponent, this.sprite_bar_opponent,
                icon_mdlhldr, HealthBar.STUB_MODELHOLDER,
                this.resolutions_opponent,
                state_name
            );
        }

        public int StatePlayerAdd(ModelHolder icon_mdlhldr, ModelHolder bar_mdlhldr, string state_name) {
            return InternalAddChrctrState(
                this.sprite_icon_player, this.sprite_bar_player,
                icon_mdlhldr, bar_mdlhldr,
                this.resolutions_player,
                state_name
            );
        }

        public int StatePlayerAdd2(ModelHolder icon_modelholder, uint bar_color_rgb8, string state_name) {
            HealthBar.STUB_MODELHOLDER.vertex_color_rgb8 = bar_color_rgb8 | 0x00;
            return InternalAddChrctrState(
                this.sprite_icon_player, this.sprite_bar_player,
                icon_modelholder, HealthBar.STUB_MODELHOLDER,
                this.resolutions_player,
                state_name
            );
        }


        public bool StateBackgroundAdd(ModelHolder modelholder, string state_name) {
            StateSpriteState state = this.sprite_background.StateAdd(modelholder, state_name, state_name);
            return state != null;
        }

        public bool StateBackgroundAdd2(uint color_rgb8, AnimSprite animsprite, string state_name) {
            animsprite = animsprite != null ? animsprite.Clone() : null;
            StateSpriteState state = this.sprite_background.StateAdd2(
                null, animsprite, null, color_rgb8, state_name
            );
            if (state == null && animsprite != null) animsprite.Destroy();
            return state != null;
        }

        public bool LoadWarnings(ModelHolder modelholder, bool use_alt_icons) {
            int success = 0;

            success += InternalAddWarning(
                this.sprite_warnings, modelholder, use_alt_icons, this.warn_height,
                HealthBar.WARNING_FAST_DRAIN
            );
            success += InternalAddWarning(
                this.sprite_warnings, modelholder, use_alt_icons, this.warn_height,
                HealthBar.WARNING_SLOW_DRAIN
            );
            success += InternalAddWarning(
                this.sprite_warnings, modelholder, use_alt_icons, this.lock_height,
                HealthBar.WARNING_LOCKED
            );
            success += InternalAddWarning(
                this.sprite_warnings, modelholder, use_alt_icons, this.warn_height,
                HealthBar.WARNING_OPPONENT_RECOVER
            );

            return success >= 4;
        }


        public void SetOpponentBarColor(uint color_rgb8) {
            this.sprite_bar_opponent.SetVertexColorRGB8(color_rgb8);
        }

        public void SetPlayerBarColor(uint color_rgb8) {
            this.sprite_bar_player.SetVertexColorRGB8(color_rgb8);
        }


        public int StateToggle(string state_name) {
            int success = 0;
            success += StateToggleBackground(state_name) ? 1 : 0;
            success += StateTogglePlayer(state_name) ? 1 : 0;
            success += StateToggleOpponent(state_name) ? 1 : 0;

            if (this.first_init) {
                this.first_init = false;
                InternalCalcDimmensions();
            }

            return success;
        }

        public bool StateToggleBackground(string state_name) {
            return this.sprite_background.StateToggle(state_name);
        }

        public bool StateTogglePlayer(string state_name) {
            return InternalToggleChrctrState(
                ref this.selected_state_player, this.prefix_state_player,
                state_name, this.sprite_bar_player, this.sprite_icon_player
            );
        }

        public bool StateToggleOpponent(string state_name) {
            return InternalToggleChrctrState(
                ref this.selected_state_opponent, this.prefix_state_opponent,
                state_name, this.sprite_bar_opponent, this.sprite_icon_opponent
            );
        }


        public void SetBumpAnimation(AnimList animlist) {
            if (animlist == null) return;
            AnimListItem animlist_item = animlist.GetAnimation(HealthBar.UI_ICON_BUMP);

            if (animlist_item == null) return;
            AnimSprite animsprite = AnimSprite.Init(animlist_item);

            if (this.bump_animation_opponent != null) this.bump_animation_opponent.Destroy();
            if (this.bump_animation_player != null) this.bump_animation_player.Destroy();

            SetBumpAnimationOpponent(animsprite);
            SetBumpAnimationPlayer(animsprite);
            animsprite.Destroy();
        }

        public void SetBumpAnimationOpponent(AnimSprite animsprite) {
            if (this.bump_animation_opponent != null) this.bump_animation_opponent.Destroy();
            this.bump_animation_opponent = animsprite.Clone();
        }

        public void SetBumpAnimationPlayer(AnimSprite animsprite) {
            if (this.bump_animation_player != null) this.bump_animation_player.Destroy();
            this.bump_animation_player = animsprite.Clone();
        }

        public void BumpEnable(bool enable_bump) {
            this.enable_bump = enable_bump;
        }


        public void SetBpm(float beats_per_minute) {
            this.beatwatcher.ChangeBpm(beats_per_minute);

            float transition_ms = this.beatwatcher.tick / HealthBar.HEALTH_TRANSITION_RATIO;
            this.tweenlerp.ChangeDurationByIndex(0, transition_ms);
        }


        public void SetOffsetColor(float r, float g, float b, float a) {
            this.drawable.SetOffsetColor(r, g, b, a);
        }

        public void SetAlpha(float alpha) {
            this.drawable.SetAlpha(alpha);
        }

        public void SetVisible(bool visible) {
            this.drawable.SetVisible(visible);
        }

        public Modifier GetModifier() {
            return this.drawable.GetModifier();
        }

        public Drawable GetDrawable() {
            return this.drawable;
        }


        public void AnimationSet(AnimSprite animsprite) {
            if (this.drawable_animation != null) this.drawable_animation.Destroy();
            this.drawable_animation = animsprite != null ? animsprite.Clone() : null;
        }

        public void AnimationRestart() {
            if (this.bump_animation_opponent != null) this.bump_animation_opponent.Restart();
            if (this.bump_animation_player != null) this.bump_animation_player.Restart();
            if (this.drawable_animation != null) this.drawable_animation.Restart();

            this.sprite_background.AnimationRestart();
            this.sprite_bar_opponent.AnimationRestart();
            this.sprite_icon_opponent.AnimationRestart();
            this.sprite_bar_player.AnimationRestart();
            this.sprite_icon_player.AnimationRestart();
        }

        public void AnimationEnd() {
            if (this.bump_animation_opponent != null) this.bump_animation_opponent.ForceEnd();
            if (this.bump_animation_player != null) this.bump_animation_player.ForceEnd();
            if (this.drawable_animation != null) this.drawable_animation.ForceEnd();

            this.sprite_background.AnimationEnd();
            this.sprite_bar_opponent.AnimationEnd();
            this.sprite_icon_opponent.AnimationEnd();
            this.sprite_bar_player.AnimationEnd();
            this.sprite_icon_player.AnimationEnd();
        }


        public int Animate(float elapsed) {
            float since_beat = elapsed;
            bool has_bump_opponent = this.enable_bump && this.bump_animation_opponent != null;
            bool has_bump_player = this.enable_bump && this.bump_animation_opponent != null;
            int res = 0;

            if (this.beatwatcher.Poll()) {
                since_beat = this.beatwatcher.since;

                if (has_bump_opponent && this.bump_animation_opponent.IsCompleted())
                    this.bump_animation_opponent.Restart();

                if (has_bump_player && this.bump_animation_player.IsCompleted())
                    this.bump_animation_player.Restart();

                if (
                    this.enable_warnings &&
                    this.enable_flash_warning &&
                    this.last_health <= HealthBar.LOW_HEALTH_PERCENT
                ) {
                    this.low_health_flash_warning = this.beatwatcher.tick / HealthBar.LOW_HEALTH_FLASH_RATIO;
                }

            } else if (this.low_health_flash_warning > 0) {
                this.low_health_flash_warning -= elapsed;
            }

            if (has_bump_opponent) {
                res += this.bump_animation_opponent.Animate(since_beat);
                this.bump_animation_opponent.UpdateModifier(
            this.bump_modifier_opponent, true
        );
            }

            if (has_bump_player) {
                res += this.bump_animation_player.Animate(since_beat);
                this.bump_animation_player.UpdateModifier(
            this.bump_modifier_player, true
        );
            }

            if (this.drawable_animation != null) {
                res += this.drawable_animation.Animate(elapsed);
                this.drawable_animation.UpdateDrawable(this.drawable, true);
            }

            if (this.transition_enabled) {
                res += this.tweenlerp.Animate(elapsed);
                float health = this.tweenlerp.PeekValue();
                InternalCalcHealthPositions(health);
            }

            res += this.sprite_background.Animate(elapsed);

            res += this.sprite_bar_opponent.Animate(elapsed);
            res += this.sprite_icon_opponent.Animate(elapsed);

            res += this.sprite_bar_player.Animate(elapsed);
            res += this.sprite_icon_player.Animate(elapsed);

            res += this.sprite_warnings.Animate(elapsed);

            return res;
        }

        public void Draw(PVRContext pvrctx) {
            pvrctx.Save();
            this.drawable.HelperApplyInContext(pvrctx);

            bool has_low_warning_flash = this.low_health_flash_warning > 0.0f;
            if (has_low_warning_flash) {
                pvrctx.Save();
                pvrctx.SetGlobalOffsetColor(HealthBar.LOW_HEALTH_WARN_COLOR);
            }

            if (this.extra_enabled) {
                float x, y;
                if (this.is_vertical) {
                    x = 0.0f;
                    y = this.extra_translation;
                } else {
                    x = this.extra_translation;
                    y = 0.0f;
                }
                pvrctx.CurrentMatrix.Translate(x, y);
            }

            this.sprite_background.Draw(pvrctx);
            this.sprite_bar_opponent.Draw(pvrctx);
            this.sprite_bar_player.Draw(pvrctx);

            InternalDrawChrctr(
                pvrctx, this.bump_modifier_player, this.sprite_icon_player
            );
            InternalDrawChrctr(
                pvrctx, this.bump_modifier_opponent, this.sprite_icon_opponent
            );

            if (this.enable_warnings && this.has_warning_drain) {
                this.sprite_warnings.SetDrawLocation(
             this.warning_drain_x, this.warning_drain_y
        );
                this.sprite_warnings.Draw(pvrctx);
            }

            if (this.enable_warnings && this.has_warning_locked) {
                float half_dimmen = this.dimmen / 2.0f;
                float x, y;
                if (this.is_vertical) {
                    x = this.modifier.y + half_dimmen;
                    y = this.warning_locked_position;
                } else {
                    x = this.warning_locked_position;
                    y = this.modifier.y + half_dimmen;
                }
                this.sprite_warnings.SetDrawLocation(x, y);
                this.sprite_warnings.Draw(pvrctx);
            }

            if (this.enable_warnings && this.has_warning_opponent_recover > this.beatwatcher.count) {
                this.sprite_warnings.SetDrawLocation(
                     this.warning_drain_x, this.warning_drain_y
        );
                this.sprite_warnings.Draw(pvrctx);
            }

            if (has_low_warning_flash) pvrctx.Restore();
            pvrctx.Restore();
        }


        public void DisableProgressAnimation(bool disable) {
            this.transition_enabled = !disable;
        }

        public float SetHealthPosition(float max_health, float health, bool opponent_recover) {
            // calculate the health percent
            health = Math2D.InverseLerp(0.0f, max_health, health);

            if (opponent_recover) {
                opponent_recover =
                    this.enable_warnings &&
                    health < this.last_health &&
                    !this.has_warning_drain &&
                    !this.has_warning_locked &&
                    this.sprite_warnings.StateToggle(HealthBar.WARNING_OPPONENT_RECOVER)
                    ;
            }

            this.has_warning_opponent_recover = opponent_recover ? (this.beatwatcher.count + 2) : -1;

            if (!this.transition_enabled || Single.IsNaN(this.last_health)) {
                this.tweenlerp.ChangeBoundsByIndex(0, -1f, health);
                this.tweenlerp.End();
                InternalCalcHealthPositions(health);
                return health;
            }

            bool is_completed = this.tweenlerp.ChangeBoundsByIndex(
                 0, this.last_health, health
            );

            if (is_completed) this.tweenlerp.Restart();

            return health;
        }

        public void SetHealthPosition2(float percent) {
            percent = Math2D.Clamp(percent, 0.0f, 1.0f);
            this.tweenlerp.ChangeBoundsByIndex(0, -1f, percent);
            this.tweenlerp.End();
            InternalCalcHealthPositions(percent);
        }

        public void DisableIconOverlap(bool disable) {
            this.enable_overlap = !disable;
            InternalCalcDimmensions();

            float last_health = this.last_health;
            this.last_health = Single.NaN;
            InternalCalcHealthPositions(last_health);
        }

        public void DisableWarnings(bool disable) {
            this.enable_warnings = !disable;
        }

        public void EnableLowHealthFlashWarning(bool enable) {
            this.enable_flash_warning = !enable;
        }



        public void HideWarnings() {
            this.has_warning_drain = false;
            this.has_warning_locked = false;
            this.low_health_flash_warning = 0.0f;
        }

        public void ShowDrainWarning(bool use_fast_drain) {
            HideWarnings();
            this.has_warning_drain = this.sprite_warnings.StateToggle(
                use_fast_drain ? HealthBar.WARNING_FAST_DRAIN : HealthBar.WARNING_SLOW_DRAIN
            );
        }

        public void ShowLockedWarning() {
            HideWarnings();
            this.has_warning_locked = this.sprite_warnings.StateToggle(
                HealthBar.WARNING_LOCKED
            );
        }



        private void InternalCalcDimmensions() {
            int resolution_bar_width, resolution_bar_height;
            int resolution_icon_width, resolution_icon_height;

            float width, height;

            if (this.is_vertical) {
                width = this.dimmen;
                height = this.length;
            } else {
                width = this.length;
                height = this.dimmen;
            }

            this.modifier.width = width;
            this.modifier.height = height;

            // resize & center background in the screen
            InternalCenterBar(
                width, height, 0,
                -1, -1, this.sprite_background
            );

            // resize & center opponent health bar and icon
            InternalLoadResolutions(
                this.resolutions_opponent, this.selected_state_opponent,
                out resolution_bar_width, out resolution_bar_height, out resolution_icon_width, out resolution_icon_height
            );
            InternalCenterBar(
                width, height, this.border, resolution_bar_width, resolution_bar_height, this.sprite_bar_opponent
            );
            InternalCenterIcon(
                true, resolution_icon_width, resolution_icon_height, this.sprite_icon_opponent
            );

            // resize & center player health bar and icon
            InternalLoadResolutions(
                this.resolutions_player, this.selected_state_player,
                out resolution_bar_width, out resolution_bar_height, out resolution_icon_width, out resolution_icon_height
            );
            InternalCenterBar(
                width, height, this.border, resolution_bar_width, resolution_bar_height, this.sprite_bar_player
            );
            InternalCenterIcon(
                false, resolution_icon_width, resolution_icon_height, this.sprite_icon_player
            );

            // apply again the selected state
            this.sprite_background.StateApply(null);
            this.sprite_bar_opponent.StateApply(null);
            this.sprite_bar_player.StateApply(null);
            this.sprite_icon_opponent.StateApply(null);
            this.sprite_icon_player.StateApply(null);

            // flip icons (if was necessary), the player icon should be always flipped
            InternalIconFlip(
                this.sprite_icon_opponent, this.is_vertical, this.flip_icon_opponent
            );
            InternalIconFlip(
                this.sprite_icon_player, this.is_vertical, this.flip_icon_player
            );
        }

        private void InternalCalcHealthPositions(float player_health) {
            if (player_health == this.last_health) return;

            this.last_health = player_health;

            if (this.extra_enabled) {
                player_health = Math2D.Lerp(0.0f, 2.0f, player_health);
                if (player_health > 1.0f) {
                    float extra_percent = player_health - 1.0f;
                    float extra_length = this.length / 2f;
                    this.extra_translation = Math2D.Lerp(0f, extra_length, extra_percent) * -1.0f;
                    player_health = 1.0f;
                }
            }

            float opponent_health = 1.0f - player_health;

            float health_position_opponent = Math2D.Clamp(
                this.health_bar_length * opponent_health,
                0f,
                this.health_bar_length
            );
            float health_position_player = Math2D.Clamp(
                this.health_bar_length * player_health,
                0f,
                this.health_bar_length
            );


            string player_health_state, opponent_health_state;

            if (player_health >= 1.0f) {
                player_health_state = HealthBar.ICON_PREFIX_WINNER;
                opponent_health_state = HealthBar.ICON_PREFIX_LOOSER;
            } else if (player_health <= HealthBar.CHARACTER_WARNING_PERCENT) {
                player_health_state = HealthBar.ICON_PREFIX_LOOSING;
                opponent_health_state = HealthBar.ICON_PREFIX_WINNING;
            } else if (opponent_health <= HealthBar.CHARACTER_WARNING_PERCENT) {
                player_health_state = HealthBar.ICON_PREFIX_WINNING;
                opponent_health_state = HealthBar.ICON_PREFIX_LOOSING;
            } else if (opponent_health >= 1.0f) {
                player_health_state = HealthBar.ICON_PREFIX_LOOSER;
                opponent_health_state = HealthBar.ICON_PREFIX_WINNER;
            } else {
                player_health_state = HealthBar.ICON_PREFIX_NEUTRAL;
                opponent_health_state = HealthBar.ICON_PREFIX_NEUTRAL;
            }

            // set the character new state
            InternalSetChrctrState(
                opponent_health_state, this.selected_state_opponent,
                this.sprite_bar_opponent, this.sprite_icon_opponent
            );
            InternalSetChrctrState(
                player_health_state, this.selected_state_player,
                this.sprite_bar_player, this.sprite_icon_player
            );

            // calculate the icon & bar positions
            InternalCalcChrctr(
                false,
                health_position_opponent, this.sprite_bar_opponent, this.sprite_icon_opponent
            );
            InternalCalcChrctr(
                true,
                health_position_player, this.sprite_bar_player, this.sprite_icon_player
            );

            // calc locked icon position on the bar
            this.warning_locked_position = health_position_opponent + this.border;
            if (this.is_vertical) this.warning_locked_position += this.modifier.y;
            else this.warning_locked_position += this.modifier.x;

            // calc the top-right corner of the player icon
            float temp_x, temp_y, temp_width, temp_height;
            this.sprite_icon_player.GetDrawLocation(out temp_x, out temp_y);
            this.warning_drain_x = temp_x;
            this.warning_drain_y = temp_y;
            this.sprite_icon_player.GetDrawSize(out temp_width, out temp_height);
            this.warning_drain_x += temp_height;

            if (this.enable_overlap) {
                if (this.is_vertical) this.warning_drain_y -= this.icon_overlap;
                else this.warning_drain_x -= this.icon_overlap;
            }
        }


        private int InternalAddChrctrState(StateSprite icon, StateSprite bar, ModelHolder icn_mdlhldr, ModelHolder hlth_mdlhldr, LinkedList<TextureResolution> rsltn, string state_name) {
            string name;
            int success = 0;

            name = StringUtils.ConcatForStateName(HealthBar.ICON_PREFIX_WINNING, state_name);
            success += InternalAddChrctrState2(icon, bar, icn_mdlhldr, hlth_mdlhldr, name);
            //free(name);

            name = StringUtils.ConcatForStateName(HealthBar.ICON_PREFIX_NEUTRAL, state_name);
            success += InternalAddChrctrState2(icon, bar, icn_mdlhldr, hlth_mdlhldr, name);
            //free(name);

            name = StringUtils.ConcatForStateName(HealthBar.ICON_PREFIX_LOOSING, state_name);
            success += InternalAddChrctrState2(icon, bar, icn_mdlhldr, hlth_mdlhldr, name);
            //free(name);

            if (success > 0)
                InternalSaveResolutions(rsltn, state_name, hlth_mdlhldr, icn_mdlhldr);

            return success;
        }

        private int InternalAddChrctrState2(StateSprite icon, StateSprite bar, ModelHolder icn_mdlhldr, ModelHolder hlth_mdlhldr, string sub_state_name) {
            int success = 0;

            if (icn_mdlhldr != null && icon.StateAdd(icn_mdlhldr, sub_state_name, sub_state_name) != null)
                success++;
            if (hlth_mdlhldr != null && bar.StateAdd(hlth_mdlhldr, sub_state_name, sub_state_name) != null)
                success++;

            return success;
        }


        private void InternalCenterBar(float width, float height, float border, int resolution_width, int resolution_height, StateSprite statesprite) {
            bool has_borders = border > 0.0f;
            float double_border = border * 2.0f;

            foreach (StateSpriteState state in statesprite.StateList()) {
                // if this state does not have texture or there are borders resize as-is
                if (has_borders || state.texture == null) {
                    // there borders in the four sides of the bar
                    state.offset_x = state.offset_y = border;

                    state.draw_width = width - double_border;
                    state.draw_height = height - double_border;
                    continue;
                }

                // textured bar, resize properly
                InternalResizeState(
                    state, resolution_width, resolution_height, HealthBar.DEFAULT_DIMMEN, HealthBar.SCALE_TO_DIMMEN_ICON
                );

                state.offset_x = (width - state.draw_width) / 2.0f;
                state.offset_y = (height - state.draw_height) / 2.0f;
            }
        }

        private void InternalCenterIcon(bool align, int resolution_width, int resolution_height, StateSprite statesprite) {
            float icon_overlap = this.enable_overlap ? this.icon_overlap : 0f;

            foreach (StateSpriteState state in statesprite.StateList()) {

                InternalResizeState(
                    state, resolution_width, resolution_height, HealthBar.DEFAULT_ICON_SIZE, HealthBar.SCALE_TO_DIMMEN_ICON
                );

                if (this.is_vertical) {
                    state.offset_x = (this.dimmen - state.draw_width) / 2.0f;
                    if (align) {
                        state.offset_y = -state.draw_height;
                        state.offset_y += icon_overlap;
                    } else {
                        state.offset_y = -icon_overlap;
                    }
                } else {
                    state.offset_y = (this.dimmen - state.draw_height) / 2.0f;
                    if (align) {
                        state.offset_x = -state.draw_width;
                        state.offset_x += icon_overlap;
                    } else {
                        state.offset_x = -icon_overlap;
                    }
                }
            }
        }

        private void InternalResizeState(StateSpriteState state, int resolution_width, int resolution_height, int def_size, float scale_to_dimmen) {
            float orig_width = def_size, orig_height = def_size;

            ImgUtils.GetStateSpriteOriginalSize(state, ref orig_width, ref orig_height);

            if (resolution_width >= 0 && resolution_height >= 0) {
                // resize using the display resolution
                float scale = this.layout_width / resolution_width;
                state.draw_width = orig_width * scale;
                state.draw_height = orig_height * scale;
            } else {
                // resize using the ratio of 1:N the health bar size
                float icon_dimmen = this.dimmen * scale_to_dimmen;
                float width = -1.0f;
                float height = -1.0f;

                if (this.is_vertical) width = icon_dimmen;
                else height = icon_dimmen;

                ImgUtils.CalcSize(orig_width, orig_height, width, height, out orig_width, out orig_height);
                state.draw_width = orig_width;
                state.draw_height = orig_height;
            }
        }

        private void InternalIconFlip(StateSprite statesprite, bool is_vertical, bool do_flip) {
            bool x, y;
            if (is_vertical) {
                x = false;
                y = do_flip;
            } else {
                x = do_flip;
                y = false;
            }
            statesprite.FlipTexture(x, y);
        }

        private bool InternalToggleChrctrState(ref string slctd_ptr, string prefix, string state_name, StateSprite bar, StateSprite icon) {
            if (state_name == HealthBar.INTERNAL_STATE_NAME) return false;

            //if (slctd_ptr != null) free(slctd_ptr);
            slctd_ptr = state_name;

            return InternalToggleChrctrState2(prefix, state_name, bar, icon) > 0;
        }

        private int InternalToggleChrctrState2(string prefix, string state_name, StateSprite bar, StateSprite icon) {
            int success = 0;
            string sub_state_name = StringUtils.ConcatForStateName(prefix, state_name);

            success += bar.StateToggle(sub_state_name) ? 1 : 0;
            success += icon.StateToggle(sub_state_name) ? 1 : 0;

            //free(sub_state_name);
            return success;
        }

        private void InternalSetChrctrState(string prefix, string state_name, StateSprite bar, StateSprite icon) {
            //
            // confusing part, set the states in this order:
            //      neutral -> winning/loosing -> winner/looser
            //
            // must be done in this way in case there missing icons for non-neutral states
            // 

            // step 1: default to "neutral"
            InternalToggleChrctrState2(HealthBar.ICON_PREFIX_NEUTRAL, state_name, bar, icon);

            if (prefix == HealthBar.ICON_PREFIX_NEUTRAL) return;

            // step 2: default to "winning" or "loosing"
            if (prefix == HealthBar.ICON_PREFIX_WINNER)
                InternalToggleChrctrState2(HealthBar.ICON_PREFIX_WINNING, state_name, bar, icon);
            else if (prefix == HealthBar.ICON_PREFIX_LOOSER)
                InternalToggleChrctrState2(HealthBar.ICON_PREFIX_LOOSING, state_name, bar, icon);

            // step 3: set the state "winner" or "looser"
            InternalToggleChrctrState2(prefix, state_name, bar, icon);
        }

        private void InternalCalcChrctr(bool invert, float position, StateSprite bar, StateSprite icon) {
            float crop_width = -1.0f;
            float crop_height = -1.0f;
            float crop_x = 0.0f;
            float crop_y = 0.0f;

            if (invert) {
                // player bar, invert the position
                position = this.health_bar_length - position;

                if (this.is_vertical)
                    crop_y = position;
                else
                    crop_x = position;
            } else {
                if (this.is_vertical)
                    crop_height = position;
                else
                    crop_width = position;
            }
            bar.Crop(crop_x, crop_y, crop_width, crop_height);


            float x = this.modifier.x;
            float y = this.modifier.y;

            //float half_dimmen = this.dimmen / -2.0f;
            float half_dimmen = 0.0f;// mimics Funkin behavior

            if (this.is_vertical) {
                x += half_dimmen;
                y += position;
            } else {
                x += position;
                y += half_dimmen;
            }
            icon.SetDrawLocation(x, y);
        }

        private void InternalDrawChrctr(PVRContext pvrctx, Modifier bump_modifier, StateSprite icon) {
            pvrctx.Save();

            float draw_width, draw_height;
            float draw_x, draw_y;
            float offset_x, offset_y;

            icon.GetDrawSize(out draw_width, out draw_height);
            icon.GetDrawLocation(out draw_x, out draw_y);
            icon.StateGetOffsets(out offset_x, out offset_y);

            pvrctx.ApplyModifier2(
        bump_modifier,
        draw_x + offset_x,
        draw_y + offset_y,
        draw_width,
        draw_height
    );
            icon.Draw(pvrctx);

            pvrctx.Restore();
        }

        private int InternalAddWarning(StateSprite sprite, ModelHolder modelholder, bool use_alt, float height, string state_name) {
            string anim_name;

            if (use_alt)
                anim_name = StringUtils.Concat(state_name, HealthBar.WARNING_ALT_SUFFIX);
            else
                anim_name = state_name;

            sprite.StateRemove(state_name);
            StateSpriteState state = sprite.StateAdd(modelholder, anim_name, state_name);

            //if (use_alt) free(anim_name);

            if (state == null) return 0;

            float temp_width = 0, temp_height = 0;
            ImgUtils.GetStateSpriteOriginalSize(state, ref temp_width, ref temp_height);
            ImgUtils.CalcSize(temp_width, temp_height, -1, height, out temp_width, out temp_height);

            state.draw_width = temp_width;
            state.draw_height = temp_height;
            state.offset_x = state.draw_width / -2f;
            state.offset_y = state.draw_height / -2f;

            return 1;
        }

        private void InternalSaveResolutions(LinkedList<TextureResolution> linkedlist, string state_name, ModelHolder mdlhldr_bar, ModelHolder mdlhldr_icn) {
            TextureResolution item = new TextureResolution() {
                state_name = state_name
            };

            mdlhldr_icn.GetTextureResolution(out item.resolution_bar_width, out item.resolution_bar_height);
            mdlhldr_bar.GetTextureResolution(out item.resolution_icon_width, out item.resolution_icon_height);

            linkedlist.AddItem(item);
        }

        private void InternalLoadResolutions(LinkedList<TextureResolution> linkedlist, string state_name, out int rsltn_bar_width, out int rsltn_bar_height, out int rsltn_icn_width, out int rsltn_icn_height) {
            foreach (TextureResolution item in linkedlist) {
                if (item.state_name == state_name) {
                    rsltn_bar_width = item.resolution_bar_width;
                    rsltn_bar_height = item.resolution_bar_height;
                    rsltn_icn_width = item.resolution_icon_width;
                    rsltn_icn_height = item.resolution_icon_height;
                    return;
                }
            }

            // this never should happen
            rsltn_bar_width = Funkin.SCREEN_RESOLUTION_WIDTH;
            rsltn_bar_height = Funkin.SCREEN_RESOLUTION_HEIGHT;
            rsltn_icn_width = Funkin.SCREEN_RESOLUTION_WIDTH;
            rsltn_icn_height = Funkin.SCREEN_RESOLUTION_HEIGHT;
        }


        private class TextureResolution {
            public string state_name;
            public int resolution_bar_width, resolution_bar_height;
            public int resolution_icon_width, resolution_icon_height;
        }

    }

}
