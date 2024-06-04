using System;
using System.Diagnostics;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Game.Common;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game;

public class StreakCounter : IDraw, IAnimate {

    private const string PREFIX_COMBO = "combo";
    private const string UI_COMBO_ANIM = "streak_text";// picked from UI animlist
    private const int STREAK_BUFFER_SIZE = 5;


    private Drawable drawable;
    private Modifier modifier;
    private int last_streak;
    private float number_height;
    private float number_gap;
    private bool ignore_combo_location;
    private float combo_width;
    private float combo_height;
    private StateSprite combo_sprite;
    private AnimSprite combo_animation;
    private bool combo_enabled;
    private NumberItem[] numbers_items;
    private int numbers_id;


    public StreakCounter(LayoutPlaceholder placeholder, float combo_height, float number_gap, float delay) {
        //
        // Notes:
        //      * the width is optional (should no be present)
        //      * alignments are ignored
        //
        float number_height = 0.0f;
        float reference_width = 0.0f;
        if (placeholder != null) {
            if (placeholder.height > 0.0f) number_height = placeholder.height;
            if (placeholder.width > 0.0f) reference_width = placeholder.width;
        }


        this.drawable = null;
        this.modifier = null;

        this.last_streak = -1;

        this.number_height = number_height; this.number_gap = number_gap;

        this.ignore_combo_location = true;

        this.combo_width = -1.0f;
        this.combo_height = combo_height;
        this.combo_sprite = StateSprite.InitFromTexture(null);
        this.combo_animation = null;
        this.combo_enabled = combo_height > 0.0f;

        this.numbers_items = new NumberItem[StreakCounter.STREAK_BUFFER_SIZE];
        this.numbers_id = 0;


        this.combo_sprite.SetVisible(false);

        this.drawable = new Drawable(0f, this, this);

        if (placeholder != null) {
            placeholder.vertex = this.drawable;
            this.drawable.HelperUpdateFromPlaceholder(placeholder);
        }

        this.modifier = this.drawable.GetModifier();
        this.modifier.x += reference_width / -2f;
        this.modifier.y += number_height / -2f;
        this.modifier.height = Math.Max(combo_height, number_height);

        for (int i = 0 ; i < StreakCounter.STREAK_BUFFER_SIZE ; i++) {
            this.numbers_items[i] = new NumberItem { id = -1, digits = new StreakCounterDigits(delay) };
        }


    }

    public void Destroy() {
        Luascript.DropShared(this);

        this.drawable.Destroy();

        for (int i = 0 ; i < StreakCounter.STREAK_BUFFER_SIZE ; i++)
            this.numbers_items[i].digits.Destroy();

        this.combo_animation.Destroy();
        this.combo_sprite.Destroy();

        //free(this);
    }


    public bool PeekStreak(PlayerStats playerstats) {
        int value = playerstats.GetComboStreak();

        if (this.last_streak == value) return false;

        if (value >= Funkin.COMBO_STREAK_VISIBLE_AFTER) {
            NumberItem unused_item = InternalPickItem(
                this.numbers_items, StreakCounter.STREAK_BUFFER_SIZE, this.numbers_id++
            );

            // center the number sprites in the draw location (y axis is already centered)
            value = Math2D.Clamp(value, 0, StreakCounterDigits.STREAKCOUNTER_MAX_VALUE);
            float draw_width = unused_item.digits.Measure(value);
            float x = this.modifier.x;
            float y = this.modifier.y;

            if (this.combo_enabled && this.ignore_combo_location) {
                x -= ((this.combo_width + this.number_gap * 2f) + draw_width) / 2f;
                this.combo_sprite.SetDrawLocation(x, y);
                x += this.combo_width;
            } else {
                x += draw_width / -2f;
            }

            // set the draw location
            unused_item.digits.SetDrawLocation(x, y, this.number_gap);

            Array.Sort(
                this.numbers_items, 0, StreakCounter.STREAK_BUFFER_SIZE,
                InternalSort
            );

            if (this.combo_enabled) {
                this.combo_sprite.AnimationRestart();
                if (this.combo_animation != null) this.combo_animation.Restart();
            }
        }

        bool streak_loose;
        if (this.last_streak >= Funkin.COMBO_STREAK_VISIBLE_AFTER)
            streak_loose = value < this.last_streak;
        else
            streak_loose = false;

        this.last_streak = value;
        return streak_loose;
    }

    public void Reset() {
        this.last_streak = -1;
    }

    public void HideComboSprite(bool hide) {
        bool combo_enabled = !hide;
        if (combo_enabled && this.combo_sprite.StateList().Count() < 1) {
            Logger.Warn("streakcounter_hide_combo_sprite() failed, combo sprite does not have states to show");
            return;
        }
        this.combo_enabled = combo_enabled;
    }

    public void SetComboDrawLocation(float x, float y) {
        this.ignore_combo_location = false;
        this.combo_sprite.SetDrawLocation(x, y);
    }


    public int StateAdd(ModelHolder combo_modelholder, ModelHolder number_modelholder, string state_name) {
        float temp_width = 0f, temp_height = 0f;

        int success = 0;

        for (int i = 0 ; i < StreakCounter.STREAK_BUFFER_SIZE ; i++) {
            success += this.numbers_items[i].digits.StateAdd(
                this.number_height,
                number_modelholder,
                state_name
            );
        }

        if (combo_modelholder == null) return success;

        string animation_name = StringUtils.ConcatForStateName(StreakCounter.PREFIX_COMBO, state_name);
        StateSpriteState statesprite_state = this.combo_sprite.StateAdd(
            combo_modelholder, animation_name, state_name
        );
        //free(animation_name);


        if (statesprite_state != null) {
            ImgUtils.GetStateSpriteOriginalSize(statesprite_state, ref temp_width, ref temp_height);
            ImgUtils.CalcSize(temp_width, temp_height, -1, this.combo_height, out temp_width, out temp_height);
            statesprite_state.draw_width = temp_width;
            statesprite_state.draw_height = temp_height;
            statesprite_state.offset_x = 0;
            statesprite_state.offset_y = temp_width / -2f;
            this.combo_width = temp_width;
            success++;
        }

        return success;
    }

    public bool StateToggle(string state_name) {
        for (int i = 0 ; i < StreakCounter.STREAK_BUFFER_SIZE ; i++)
            this.numbers_items[i].digits.StateToggle(state_name);

        if (this.combo_sprite.StateToggle(state_name)) {
            this.combo_width = this.combo_sprite.StateGet().draw_width;
            return true;
        }

        return false;
    }


    public void SetAlpha(float alpha) {
        this.drawable.SetAlpha(alpha);
    }

    public void SetOffsetcolor(float r, float g, float b, float a) {
        this.drawable.SetOffsetColor(r, g, b, a);
    }

    public void SetOffsetcolorToDefault() {
        this.drawable.SetOffsetColorToDefault();
    }

    public Modifier GetModifier() {
        return this.drawable.GetModifier();
    }

    public Drawable GetDrawable() {
        return this.drawable;
    }

    public void SetNumberAnimation(AnimList animlist) {
        if (animlist == null) return;

        AnimListItem animlist_item = animlist.GetAnimation(StreakCounterDigits.STREAKCOUNTER_UI_STREAK_ANIM);
        if (animlist_item == null) return;

        for (int i = 0 ; i < StreakCounter.STREAK_BUFFER_SIZE ; i++)
            this.numbers_items[i].digits.AnimationSet(animlist_item);
    }

    public void SetComboAnimation(AnimList animlist) {
        if (animlist == null) return;
        AnimListItem animlist_item = animlist.GetAnimation(StreakCounter.UI_COMBO_ANIM);

        if (animlist_item == null) return;
        this.combo_animation = AnimSprite.Init(animlist_item);
    }


    public void AnimationSet(AnimSprite animsprite) {
        for (int i = 0 ; i < StreakCounter.STREAK_BUFFER_SIZE ; i++)
            this.numbers_items[i].digits.AnimationPairSet(animsprite);
    }

    public void AnimationRestart() {
        for (int i = 0 ; i < StreakCounter.STREAK_BUFFER_SIZE ; i++)
            this.numbers_items[i].digits.AnimationRestart();


        this.combo_sprite.AnimationRestart();
        if (this.combo_animation != null) this.combo_animation.Restart();
    }

    public void AnimationEnd() {
        for (int i = 0 ; i < StreakCounter.STREAK_BUFFER_SIZE ; i++)
            this.numbers_items[i].digits.AnimationEnd();

        this.combo_sprite.AnimationEnd();
        if (this.combo_animation != null)
            this.combo_animation.ForceEnd3(this.combo_sprite);
    }


    public int Animate(float elapsed) {
        int res = 0;

        for (int i = 0 ; i < StreakCounter.STREAK_BUFFER_SIZE ; i++) {
            if (this.numbers_items[i].id < 0) continue;
            if (this.numbers_items[i].digits.Animate(elapsed) > 0) {
                res++;
                this.numbers_items[i].id = -1;// animation completed, hide
            }
        }

        res += this.combo_sprite.Animate(elapsed);
        if (this.combo_animation != null) {
            res += this.combo_animation.Animate(elapsed);
            this.combo_animation.UpdateStatesprite(this.combo_sprite, true);
        }

        return res;
    }

    public void Draw(PVRContext pvrctx) {
        if (this.last_streak < Funkin.COMBO_STREAK_VISIBLE_AFTER) return;

        pvrctx.Save();
        this.drawable.HelperApplyInContext(pvrctx);

        bool draw_combo = this.combo_enabled;

        for (int i = 0 ; i < StreakCounter.STREAK_BUFFER_SIZE ; i++) {
            if (this.numbers_items[i].id < 0) continue;

            if (draw_combo) {
                draw_combo = false;
                this.combo_sprite.Draw(pvrctx);
            }

            this.numbers_items[i].digits.Draw(pvrctx);
        }

        pvrctx.Restore();
    }



    private static NumberItem InternalPickItem(NumberItem[] array, int size, int new_id) {
        for (int i = 0 ; i < size ; i++) {
            if (array[i].id < 0) {
                array[i].id = new_id;
                return array[i];
            }
        }

        int oldest_id = Math2D.MAX_INT32;
        NumberItem oldest_item = null;

        for (int i = 0 ; i < size ; i++) {
            if (array[i].id < oldest_id) {
                oldest_id = array[i].id;
                oldest_item = array[i];
            }
        }

        Debug.Assert(oldest_item != null);
        oldest_item.id = new_id;
        return oldest_item;
    }

    private static Comparer InternalSort = new Comparer();


    private class NumberItem {
        public int id;
        public StreakCounterDigits digits;
    }
    private class Comparer : System.Collections.Generic.IComparer<NumberItem> {
        public int Compare(NumberItem item1, NumberItem item2) {
            return item1.id - item2.id;
        }

    }

}
