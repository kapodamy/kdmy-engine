using System;
using System.Collections.Generic;
using System.Diagnostics;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Font;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game;

public class RankingCounter {

    private const string PREFIX_SHIT = "shit";
    private const string PREFIX_BAD = "bad";
    private const string PREFIX_GOOD = "good";
    private const string PREFIX_SICK = "sick";
    private const string PREFIX_MISS = "miss";
    private const string PREFIX_PENALITY = "penality";

    private const uint TEXT_COLOR_SHIT = 0xFF0000;// red
    private const uint TEXT_COLOR_BAD = 0xFF0000;// red
    private const uint TEXT_COLOR_GOOD = 0x00FF00;// green
    private const uint TEXT_COLOR_SICK = 0x00FFFF;// cyan
    private const uint TEXT_COLOR_MISS = 0x151B54;// midnight blue
    private const uint TEXT_COLOR_PENALITY = 0x000000;// black (not implemented)

    private const int RANKING_BUFFER_SIZE = 5;
    private const string FORMAT_TIME = "$2dms";// prints 12.345 as "12.34ms"
    private const string FORMAT_PERCENT = "$0d%";// prints 99.7899 as "99%"
    private const string UI_RANKING_ANIM = "ranking";// picked from UI animlist
    private const string UI_RANKING_TEXT_ANIM = "ranking_text";// picked from UI animlist

    private const string TEXT_MISS = "MISS";

    private const string INTERNAL_STATE_NAME = "_____rankingcounter-state______";



    private int last_iterations;
    private TextSprite textsprite;
    private bool show_accuracy;
    private bool enable_accuracy;
    private bool enable_accuracy_percent;
    private string selected_state;
    private AnimSprite drawable_animation;
    private RankingItem[] ranking_items;
    private int ranking_id;
    private float ranking_height;
    private Drawable drawable_accuracy;
    private Drawable drawable_rank;
    private float correction_x;
    private float correction_y;


    public RankingCounter(LayoutPlaceholder plchldr_rank, LayoutPlaceholder plchldr_accuracy, FontHolder fnthldr) {
        //
        // Notes:
        //      * the width is optional (should no be present)
        //      * alignments are ignored
        //      * the font size is calculated for the screen to avoid huge font characters in VRAM
        //
        float ranking_height = 0.0f;
        if (plchldr_rank != null && plchldr_rank.height > 0.0f) ranking_height = plchldr_rank.height;

        float font_size = 20.0f;
        if (plchldr_accuracy != null) {
            if (plchldr_accuracy.height > 0.0f) {
                font_size = plchldr_accuracy.height;
            } else {
                // dismiss
                plchldr_accuracy = null;
            }
        }


        this.last_iterations = 0;

        this.textsprite = TextSprite.Init2(fnthldr, font_size, 0x000000);
        this.show_accuracy = false;
        this.enable_accuracy = true;
        this.enable_accuracy_percent = false;

        this.selected_state = RankingCounter.INTERNAL_STATE_NAME;
        this.drawable_animation = null;

        this.ranking_items = new RankingItem[RankingCounter.RANKING_BUFFER_SIZE];

        this.ranking_id = 0;
        this.ranking_height = ranking_height;

        this.drawable_accuracy = null;
        this.drawable_rank = null;


        this.drawable_rank = new Drawable(
            -1, this.Draw1, this.Animate1
        );
        this.drawable_accuracy = new Drawable(
            -1, this.Draw2, this.Animate2
        );

        if (plchldr_rank != null) {
            this.drawable_rank.HelperUpdateFromPlaceholder(plchldr_rank);
            plchldr_rank.vertex = this.drawable_rank;

            float x = plchldr_rank.x;
            if (plchldr_rank.width > 0.0f) x -= plchldr_rank.width / 2.0f;

            for (int i = 0 ; i < RankingCounter.RANKING_BUFFER_SIZE ; i++) {
                StateSprite statesprite = StateSprite.InitFromTexture(null);
                statesprite.SetDrawLocation(x, plchldr_rank.y);
                statesprite.SetVisible(false);

                this.ranking_items[i] = new RankingItem() { statesprite = statesprite, animsprite = null, id = -1 };
            }
        } else {
            for (int i = 0 ; i < RankingCounter.RANKING_BUFFER_SIZE ; i++) {
                StateSprite statesprite = StateSprite.InitFromTexture(null);
                this.ranking_items[i] = new RankingItem() { statesprite = statesprite, animsprite = null, id = -1 };
            }
        }

        if (plchldr_accuracy != null) {
            this.drawable_accuracy.HelperUpdateFromPlaceholder(plchldr_accuracy);
            plchldr_accuracy.vertex = this.drawable_accuracy;

            float x = plchldr_accuracy.x;
            if (plchldr_accuracy.width > 0.0f) x -= plchldr_accuracy.width / 2.0f;

            this.textsprite.SetDrawLocation(
             x, plchldr_accuracy.y
        );
            this.correction_x = x;
            this.correction_y = plchldr_accuracy.y;

            this.textsprite.SetAlign(Align.CENTER, Align.CENTER);

            // center the accuracy text on the draw location
            this.textsprite.SetMaxDrawSize(0.0f, 0.0f);
        }


    }

    public void Destroy() {
        Luascript.DropShared(this);

        // destroy the attached animation of textsprite
        AnimSprite old_animation = this.textsprite.AnimationSet(null);
        if (old_animation != null) old_animation.Destroy();

        this.textsprite.Destroy();
        this.drawable_rank.Destroy();
        this.drawable_accuracy.Destroy();
        if (this.drawable_animation != null) this.drawable_animation.Destroy();

        for (int i = 0 ; i < RankingCounter.RANKING_BUFFER_SIZE ; i++) {
            if (this.ranking_items[i].animsprite != null) this.ranking_items[i].animsprite.Destroy();
            if (this.ranking_items[i].statesprite != null)
                this.ranking_items[i].statesprite.Destroy();
        }

        //if (this.selected_state != RankingCounter.INTERNAL_STATE_NAME)
        //    free(this.selected_state);

        //free(this);
    }


    public int AddState(ModelHolder modelholder, string state_name) {
        float max_height = this.ranking_height;

        int success = 0;
        for (int i = 0 ; i < RankingCounter.RANKING_BUFFER_SIZE ; i++) {
            StateSprite statesprite = this.ranking_items[i].statesprite;

            success += InternalAddState(
                statesprite, max_height, modelholder, RankingCounter.PREFIX_SHIT, state_name
            );
            success += InternalAddState(
                statesprite, max_height, modelholder, RankingCounter.PREFIX_BAD, state_name
            );
            success += InternalAddState(
                statesprite, max_height, modelholder, RankingCounter.PREFIX_GOOD, state_name
            );
            success += InternalAddState(
                statesprite, max_height, modelholder, RankingCounter.PREFIX_SICK, state_name
            );
            success += InternalAddState(
                statesprite, max_height, modelholder, RankingCounter.PREFIX_MISS, state_name
            );
            success += InternalAddState(
                statesprite, max_height, modelholder, RankingCounter.PREFIX_PENALITY, state_name
            );
        }

        return success / RankingCounter.RANKING_BUFFER_SIZE;
    }

    public void ToggleState(string state_name) {
        //if (this.selected_state != RankingCounter.INTERNAL_STATE_NAME)
        //    free(this.selected_state);

        this.selected_state = state_name;
    }


    public void PeekRanking(PlayerStats playerstats) {
        double value; string format; string ranking; uint color;

        int interations = playerstats.GetIterations();
        if (interations == this.last_iterations) return;
        this.last_iterations = interations;

        Ranking rank = playerstats.GetLastRanking();

        switch (rank) {
            case Ranking.NONE:
                return;
            case Ranking.SICK:
                ranking = RankingCounter.PREFIX_SICK;
                color = RankingCounter.TEXT_COLOR_SICK;
                break;
            case Ranking.GOOD:
                ranking = RankingCounter.PREFIX_GOOD;
                color = RankingCounter.TEXT_COLOR_GOOD;
                break;
            case Ranking.BAD:
                ranking = RankingCounter.PREFIX_BAD;
                color = RankingCounter.TEXT_COLOR_BAD;
                break;
            case Ranking.SHIT:
                ranking = RankingCounter.PREFIX_SHIT;
                color = RankingCounter.TEXT_COLOR_SHIT;
                break;
            case Ranking.MISS:
                ranking = RankingCounter.PREFIX_MISS;
                color = RankingCounter.TEXT_COLOR_MISS;
                break;
            case Ranking.PENALITY:
                ranking = RankingCounter.PREFIX_PENALITY;
                color = RankingCounter.TEXT_COLOR_PENALITY;
                break;
            default:
                return;
        }

        // find an unused item
        RankingItem item = RankingCounter.InternalPickItem(
            this.ranking_items, this.ranking_id++
        );

        // toggle state for this item
        string state_name = StringUtils.ConcatForStateName(ranking, this.selected_state);
        if (item.statesprite.StateToggle(state_name)) {
            item.statesprite.AnimationRestart();
            if (item.animsprite != null) item.animsprite.Restart();

            // sort visible items (old items first)
            EngineUtils.Sort(
                this.ranking_items, 0,
                RankingCounter.RANKING_BUFFER_SIZE,
                RankingCounter.InternalSort
            );
        } else {
            // no state for this item, hide it
            item.id = -1;
        }
        //free(state_name);

        if (!this.enable_accuracy || rank == Ranking.PENALITY) return;

        if (this.enable_accuracy_percent) {
            value = playerstats.GetLastAccuracy();
            format = RankingCounter.FORMAT_PERCENT;
        } else {
            value = playerstats.GetLastDifference();
            format = RankingCounter.FORMAT_TIME;
        }

        if (Double.IsNaN(value)) return;

        this.show_accuracy = true;
        this.textsprite.AnimationRestart();
        this.textsprite.SetColorRGBA8(color);

        if (rank == Ranking.MISS)
            this.textsprite.SetTextIntern(true, RankingCounter.TEXT_MISS);
        else
            this.textsprite.SetTextFormated(format, value);
    }

    public void Reset() {
        this.show_accuracy = false;
        this.last_iterations = 0;

        this.drawable_accuracy.SetAntialiasing(PVRFlag.DEFAULT);
        this.drawable_rank.SetAntialiasing(PVRFlag.DEFAULT);

        SetOffsetcolorToDefault();

        this.drawable_accuracy.GetModifier().Clear();
        this.drawable_rank.GetModifier().Clear();

        // hide all visible ranking items
        for (int i = 0 ; i < RankingCounter.RANKING_BUFFER_SIZE ; i++)
            this.ranking_items[i].id = -1;
    }

    public void HideAccuracy(bool hide) {
        this.enable_accuracy = hide;
    }

    public void UsePercentInstead(bool use_accuracy_percenter) {
        this.enable_accuracy_percent = use_accuracy_percenter;
    }

    public void SetDefaultRankingAnimation(AnimList animlist) {
        if (animlist == null) return;
        AnimListItem animlist_item = animlist.GetAnimation(RankingCounter.UI_RANKING_ANIM);

        if (animlist_item == null) return;
        AnimSprite animsprite = AnimSprite.Init(animlist_item);

        SetDefaultRankingAnimation2(animsprite);
        animsprite.Destroy();
    }

    public void SetDefaultRankingAnimation2(AnimSprite animsprite) {
        for (int i = 0 ; i < RankingCounter.RANKING_BUFFER_SIZE ; i++) {
            if (this.ranking_items[i].animsprite != null) this.ranking_items[i].animsprite.Destroy();
            this.ranking_items[i].animsprite = animsprite != null ? animsprite.Clone() : null;
        }
    }


    public void SetDefaultRankingTextAnimation(AnimList animlist) {
        if (animlist == null) return;
        AnimListItem animlist_item = animlist.GetAnimation(RankingCounter.UI_RANKING_TEXT_ANIM);

        if (animlist_item == null) return;
        AnimSprite animsprite = AnimSprite.Init(animlist_item);

        AnimSprite old_animation = this.textsprite.AnimationSet(animsprite);
        if (old_animation != null) old_animation.Destroy();
    }

    public void SetDefaultRankingTextAnimation2(AnimSprite animsprite) {
        AnimSprite old_animation = this.textsprite.AnimationSet(animsprite != null ? animsprite.Clone() : null);
        if (old_animation != null) old_animation.Destroy();
    }



    public void SetAlpha(float alpha) {
        this.drawable_accuracy.SetAlpha(alpha);
        this.drawable_rank.SetAlpha(alpha);
    }

    public void SetOffsetcolor(float r, float g, float b, float a) {
        this.drawable_accuracy.SetOffsetColor(r, g, b, a);
        this.drawable_rank.SetOffsetColor(r, g, b, a);
    }

    public void SetOffsetcolorToDefault() {
        this.drawable_accuracy.SetOffsetColorToDefault();
        this.drawable_rank.SetOffsetColorToDefault();
    }


    public void AnimationSet(AnimSprite animsprite) {
        this.drawable_animation.Destroy();
        this.drawable_animation = animsprite.Clone();
    }

    public void AnimationRestart() {
        for (int i = 0 ; i < RankingCounter.RANKING_BUFFER_SIZE ; i++) {
            if (this.ranking_items[i].animsprite != null)
                this.ranking_items[i].animsprite.Restart();
            this.ranking_items[i].statesprite.AnimationRestart();
        }

        this.textsprite.AnimationRestart();

        if (this.drawable_animation != null)
            this.drawable_animation.Restart();
    }

    public void AnimationEnd() {
        for (int i = 0 ; i < RankingCounter.RANKING_BUFFER_SIZE ; i++)
            this.ranking_items[i].id = -1;

        this.textsprite.AnimationEnd();

        if (this.drawable_animation != null) {
            this.drawable_animation.ForceEnd();
            this.drawable_animation.UpdateDrawable(this.drawable_accuracy, false);
            this.drawable_animation.UpdateDrawable(this.drawable_rank, true);
        }
    }


    public int Animate1(float elapsed) {
        int total = RankingCounter.RANKING_BUFFER_SIZE + 1;

        for (int i = 0 ; i < RankingCounter.RANKING_BUFFER_SIZE ; i++) {
            RankingItem item = this.ranking_items[i];
            if (item.id < 0) {
                total--;
                continue;
            }

            int completed = item.statesprite.Animate(elapsed);

            if (item.animsprite != null) {
                completed = item.animsprite.Animate(elapsed);
                item.animsprite.UpdateStatesprite(item.statesprite, true);
            }

            if (completed > 0) {
                item.id = -1;
                total--;
            }
        }

        if (this.drawable_animation != null) {
            if (this.drawable_animation.Animate(elapsed) > 0) total--;
            this.drawable_animation.UpdateDrawable(this.drawable_accuracy, false);
            this.drawable_animation.UpdateDrawable(this.drawable_rank, true);
        }

        return total;
    }

    public int Animate2(float elapsed) {
        if (this.enable_accuracy && this.show_accuracy) {
            int completed = this.textsprite.Animate(elapsed);
            if (completed > 0) this.show_accuracy = false;
            return completed;
        }

        return 1;
    }

    public void Draw1(PVRContext pvrctx) {
        pvrctx.Save();
        this.drawable_rank.HelperApplyInContext(pvrctx);

        for (int i = 0 ; i < RankingCounter.RANKING_BUFFER_SIZE ; i++) {
            if (this.ranking_items[i].id < 0) continue;
            this.ranking_items[i].statesprite.Draw(pvrctx);
        }

        pvrctx.Restore();
    }

    public void Draw2(PVRContext pvrctx) {
        if (!this.enable_accuracy || !this.show_accuracy) return;
        pvrctx.Save();
        this.drawable_accuracy.HelperApplyInContext(pvrctx);

        this.textsprite.Draw(pvrctx);
        pvrctx.Restore();
    }



    private int InternalAddState(StateSprite statesprite, float max_height, ModelHolder modelholder, string prefix, string state_name) {
        string animation_name = StringUtils.ConcatForStateName(prefix, state_name);

        Texture texture = modelholder.GetTexture(false);
        uint vertex_color = modelholder.GetVertexColor();
        AnimSprite animsprite = modelholder.CreateAnimsprite(animation_name, false, false);
        AtlasEntry atlas_entry = modelholder.GetAtlasEntry2(animation_name);

        StateSpriteState state = statesprite.StateAdd2(
            texture, animsprite, atlas_entry, vertex_color, animation_name
        );
        //free(animation_name);

        if (state == null) {
            if (animsprite != null) animsprite.Destroy();
            return 0;
        }

        float temp_width = 0, temp_height = 0;
        ImgUtils.GetStateSpriteOriginalSize(state, ref temp_width, ref temp_height);
        ImgUtils.CalcSize(temp_width, temp_height, -1, max_height, out temp_width, out temp_height);

        state.draw_width = temp_width;
        state.draw_height = temp_height;

        // center the sprite on the draw location
        state.offset_x = state.draw_width / -2;
        state.offset_y = state.draw_height / -2;

        return 1;
    }

    private static RankingItem InternalPickItem(RankingItem[] array, int new_id) {
        for (int i = 0 ; i < RankingCounter.RANKING_BUFFER_SIZE ; i++) {
            if (array[i].id < 0) {
                array[i].id = new_id;
                return array[i];
            }
        }

        int oldest_id = Math2D.MAX_INT32;
        RankingItem oldest_item = null;

        for (int i = 0 ; i < RankingCounter.RANKING_BUFFER_SIZE ; i++) {
            if (array[i].id < oldest_id) {
                oldest_id = array[i].id;
                oldest_item = array[i];
            }
        }

        Debug.Assert(oldest_item != null);
        oldest_item.id = new_id;
        return oldest_item;
    }

    private static int InternalSort(RankingItem item1, RankingItem item2) {
        return item1.id - item2.id;
    }


    private class RankingItem {
        public StateSprite statesprite;
        public AnimSprite animsprite;
        public int id;
    }

}
