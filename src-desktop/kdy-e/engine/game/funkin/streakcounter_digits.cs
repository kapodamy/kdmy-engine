using Engine.Animation;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game;

public class StreakCounterDigits : IAnimate, IDraw {

    internal const string STREAKCOUNTER_UI_STREAK_ANIM = "streak_number";// picked from UI animlist
    private static readonly char[] STREAKCOUNTER_NUMBERS = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    private const int STREAKCOUNTER_DIGITS = 4;// maximum visible digits
    internal const int STREAKCOUNTER_MAX_VALUE = 9999;// ¡¡this must match the STREAKCOUNTER_DIGITS!!


    private sbyte[] buffer;
    private StateSprite[] digits;
    private AnimSprite[] animations;
    private bool has_animations;
    private float delay;
    private AnimSprite animation_pair;
    private Drawable drawable;
    private string selected_state;


    public StreakCounterDigits(float delay) {

        this.buffer = new sbyte[StreakCounterDigits.STREAKCOUNTER_DIGITS];
        this.digits = new StateSprite[StreakCounterDigits.STREAKCOUNTER_DIGITS];
        this.animations = new AnimSprite[StreakCounterDigits.STREAKCOUNTER_DIGITS];
        this.has_animations = false;

        this.delay = delay;

        this.animation_pair = null;
        this.drawable = null;

        // fake drawable for pair animations
        this.drawable = new Drawable(-1, (IDraw)null, (IAnimate)null);
        this.drawable.SetVisible(false);

        for (int i = 0 ; i < STREAKCOUNTER_DIGITS ; i++) {
            this.buffer[i] = -1;
            this.animations[i] = null;
            this.digits[i] = StateSprite.InitFromTexture(null);
            this.digits[i].SetVisible(false);
        }

    }

    public void Destroy() {
        for (int i = 0 ; i < STREAKCOUNTER_DIGITS ; i++) {
            this.digits[i].Destroy();
            if (this.animations[i] != null) this.animations[i].Destroy();
        }
        if (this.animation_pair != null) this.animation_pair.Destroy();
        this.drawable.Destroy();
        //if (this.selected_state != INTERNAL_STATE_NAME) free(this.selected_state);
        //free(this);
    }


    public int StateAdd(float max_hght, ModelHolder mdlhldr, string state_name) {
        float temp_width = 0, temp_height = 0;
        int success = 0;

        for (int i = 0 ; i < STREAKCOUNTER_NUMBERS.Length ; i++) {
            string number = STREAKCOUNTER_NUMBERS[i].ToString();
            string animation_name = StringUtils.ConcatForStateName(number, state_name);

            for (int j = 0 ; j < STREAKCOUNTER_DIGITS ; j++) {
                StateSpriteState statesprite_state = this.digits[j].StateAdd(
                     mdlhldr, animation_name, animation_name
                );

                if (statesprite_state == null) continue;

                ImgUtils.GetStateSpriteOriginalSize(statesprite_state, ref temp_width, ref temp_height);
                ImgUtils.CalcSize(temp_width, temp_height, -1, max_hght, out temp_width, out temp_height);

                statesprite_state.draw_width = temp_width;
                statesprite_state.draw_height = temp_height;
                statesprite_state.offset_x = 0.0f;
                statesprite_state.offset_y = 0.0f;
            }

            //free(animation_name);
        }

        return success;
    }

    public void StateToggle(string state_name) {
        //if (this.selected_state != INTERNAL_STATE_NAME) free(this.selected_state);
        this.selected_state = state_name;
    }

    public void AnimationRestart() {
        for (int i = 0 ; i < STREAKCOUNTER_DIGITS ; i++) {
            this.digits[i].AnimationRestart();
            if (this.has_animations)
                this.animations[i].Restart();
        }
        if (this.animation_pair != null)
            this.animation_pair.Restart();
    }

    public void AnimationEnd() {
        for (int i = 0 ; i < STREAKCOUNTER_DIGITS ; i++) {
            this.digits[i].AnimationEnd();
            if (this.has_animations) {
                this.animations[i].ForceEnd();
                this.animations[i].UpdateStatesprite(this.digits[i], true);
            }
        }
        if (this.animation_pair != null) {
            this.animation_pair.ForceEnd();
            this.animation_pair.UpdateDrawable(this.drawable, true);
        }
    }


    public void AnimationSet(AnimListItem animlist_item) {
        this.has_animations = animlist_item != null;
        for (int i = 0 ; i < STREAKCOUNTER_DIGITS ; i++) {
            if (this.animations[i] != null) this.animations[i].Destroy();

            if (animlist_item == null) {
                this.animations[i] = null;
                continue;
            }

            this.animations[i] = AnimSprite.Init(animlist_item);
            float delay = this.delay * Math2D.RandomInt(0, STREAKCOUNTER_DIGITS);
            this.animations[i].SetDelay(delay);
        }
    }

    public void AnimationPairSet(AnimSprite animsprite_to_clone) {
        if (this.animation_pair != null) this.animation_pair.Destroy();
        this.animation_pair = animsprite_to_clone.Clone();
    }


    public int Animate(float elapsed) {
        int completed = 1;

        for (int i = 0 ; i < STREAKCOUNTER_DIGITS ; i++) {
            if (this.buffer[i] < 0) continue;

            this.digits[i].Animate(elapsed);

            if (!this.has_animations) continue;
            if (this.animations[i].Animate(elapsed) > 0) continue;

            completed = 0;
            this.animations[i].UpdateStatesprite(
            this.digits[i], true
        );
        }

        if (this.animation_pair == null) return completed;

        if (this.animation_pair.Animate(elapsed) < 1) completed = 0;
        this.animation_pair.UpdateDrawable(this.drawable, true);

        return completed;
    }

    public float Meansure(int value) {
        float draw_width, draw_height;

        //value = Math2D.Clamp(value, 0, StreakCounterDigits.STREAKCOUNTER_MAX_VALUE);

        for (int i = 0 ; i < StreakCounterDigits.STREAKCOUNTER_DIGITS ; i++)
            this.buffer[i] = -1;

        int index = StreakCounterDigits.STREAKCOUNTER_DIGITS - 1;
        while (value > 0) {
            this.buffer[index--] = (sbyte)(value % 10);
            value = value / 10;
        }

        // add leading zero
        if (index > 0) this.buffer[index - 1] = 0;

        float meansured_width = 0;

        for (int i = 0 ; i < StreakCounterDigits.STREAKCOUNTER_DIGITS ; i++) {
            sbyte digit = this.buffer[i];
            if (digit < 0) continue;

            if (digit >= StreakCounterDigits.STREAKCOUNTER_NUMBERS.Length) {
                // this never should happen
                this.buffer[i] = -1;
                continue;
            }

            string number = StreakCounterDigits.STREAKCOUNTER_NUMBERS[digit].ToString();

            StateSprite statesprite = this.digits[i];
            string state_name = StringUtils.ConcatForStateName(
                number, this.selected_state
            );

            if (statesprite.StateToggle(state_name)) {
                statesprite.AnimationRestart();

                if (this.animations[i] != null) {
                    this.animations[i].Restart();
                    this.animations[i].UpdateStatesprite(statesprite, true);
                }

                statesprite.GetDrawSize(out draw_width, out draw_height);
                meansured_width += draw_width;
            } else {
                this.buffer[i] = -1;
            }

            //free(state_name);
        }

        return meansured_width;
    }

    public void SetDrawLocation(float x, float y, float gap) {
        float draw_width, draw_height;
        Modifier modifier = this.drawable.GetModifier();
        float max_width = 0;
        float max_height = 0;

        for (int i = 0 ; i < StreakCounterDigits.STREAKCOUNTER_DIGITS ; i++) {
            if (this.buffer[i] < 0) continue;

            this.digits[i].GetDrawSize(out draw_width, out draw_height);
            this.digits[i].SetDrawLocation(x, y);
            x += gap + draw_width;

            if (draw_width > max_width) max_width = draw_width;
            if (draw_height > max_height) max_height = draw_height;
        }

        modifier.x = x;
        modifier.y = y;
        modifier.width = max_width;
        modifier.width = max_height;
    }

    public void Draw(PVRContext pvrctx) {
        pvrctx.Save();
        this.drawable.HelperApplyInContext(pvrctx);

        for (int i = 0 ; i < STREAKCOUNTER_DIGITS ; i++) {
            if (this.buffer[i] < 0) continue;
            this.digits[i].Draw(pvrctx);
        }

        pvrctx.Restore();
    }

}
