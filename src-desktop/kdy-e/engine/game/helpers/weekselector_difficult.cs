using System;
using System.Diagnostics;
using Engine.Animation;
using Engine.Game.Common;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game.Helpers;

public class WeekSelectorDifficult : IDraw, IAnimate {

    private const string UI_NO_WARNS = "ui_difficult_no_warns";
    private const string UI_WARNS = "ui_difficult_warns";


    private AnimSprite animsprite;
    private int index;
    private Item[] list;
    private int list_size;
    private StateSprite sprite_commons;
    private StateSprite sprite_customs;
    private Sprite icon_locked;
    private Sprite icon_left;
    private Sprite icon_right;
    private LayoutPlaceholder placeholder1;
    private LayoutPlaceholder placeholder2;
    private bool has_left;
    private bool has_right;
    private bool is_common;
    private bool is_locked;
    private Drawable drawable;

    public WeekSelectorDifficult(AnimList animlist, ModelHolder modelholder, Layout layout) {

        this.animsprite = AnimSprite.InitFromAnimlist(animlist, WeekSelector.ARROW_SPRITE_NAME);

        this.index = -1;
        this.list = null;
        this.list_size = 0;

        this.sprite_commons = StateSprite.InitFromTexture(null);
        this.sprite_customs = StateSprite.InitFromTexture(null);

        this.icon_locked = modelholder.CreateSprite(WeekSelector.LOCKED, true);
        this.icon_left = modelholder.CreateSprite(WeekSelector.ARROW_SPRITE_NAME, true);
        this.icon_right = modelholder.CreateSprite(WeekSelector.ARROW_SPRITE_NAME, true);

        this.placeholder1 = layout.GetPlaceholder(WeekSelectorDifficult.UI_NO_WARNS);
        this.placeholder2 = layout.GetPlaceholder(WeekSelectorDifficult.UI_WARNS);

        this.has_left = true;
        this.has_right = true;
        this.is_common = false;
        this.is_locked = false;

        this.drawable = null;


        this.icon_right.FlipRenderedTexture(true, false);

        this.drawable = new Drawable(
            -1, this, this
        );
        SetVisible(true);

        this.sprite_commons.SetVisible(false);
        this.sprite_customs.SetVisible(false);

        this.icon_locked.SetVisible(false);
        this.icon_left.SetVisible(false);
        this.icon_right.SetVisible(false);

        // prebuild the list
        this.sprite_commons.StateAdd(
            modelholder, Funkin.DIFFICULT_EASY, Funkin.DIFFICULT_EASY
        );
        this.sprite_commons.StateAdd(
             modelholder, Funkin.DIFFICULT_NORMAL, Funkin.DIFFICULT_NORMAL
        );
        this.sprite_commons.StateAdd(
             modelholder, Funkin.DIFFICULT_HARD, Funkin.DIFFICULT_HARD
        );

    }

    public void Destroy() {
        this.animsprite.Destroy();

        this.drawable.Destroy();

        this.sprite_commons.Destroy();
        this.sprite_customs.Destroy();

        this.icon_locked.DestroyFull();
        this.icon_left.DestroyFull();
        this.icon_right.DestroyFull();

        // do not disposed the names on the list
        //free(this.list);
        //free(this);
    }


    public void Draw(PVRContext pvrctx) {
        pvrctx.Save();

        if (this.is_locked) this.icon_locked.Draw(pvrctx);
        this.icon_left.Draw(pvrctx);
        this.icon_right.Draw(pvrctx);

        if (this.is_locked)
            pvrctx.SetGlobalAlpha(WeekSelector.PREVIEW_DISABLED_ALPHA);

        if (this.is_common)
            this.sprite_commons.Draw(pvrctx);
        else
            this.sprite_customs.Draw(pvrctx);

        if (this.is_locked) {
            pvrctx.SetGlobalAlpha(1.0f);
            this.icon_locked.Draw(pvrctx);
        }

        pvrctx.Restore();
    }

    public int Animate(float elapsed) {
        if (this.animsprite != null) this.animsprite.Animate(elapsed);

        this.icon_left.Animate(elapsed);
        this.icon_right.Animate(elapsed);

        this.sprite_commons.Animate(elapsed);
        this.sprite_customs.Animate(elapsed);

        if (this.has_left)
            this.animsprite.UpdateSprite(this.icon_left, false);
        if (this.has_right)
            this.animsprite.UpdateSprite(this.icon_right, true);

        return 0;
    }


    public bool Select(int new_index) {
        if (new_index < 0 || new_index >= this.list_size) return false;

        this.is_common = this.list[new_index].is_common;
        this.is_locked = this.list[new_index].is_locked;
        this.index = new_index;
        this.has_left = new_index > 0;
        this.has_right = new_index < (this.list_size - 1);

        (this.is_common ? this.sprite_commons : this.sprite_customs).StateToggle(
            this.list[new_index].name
        );
        this.icon_left.SetAlpha(
             this.has_left ? 1.0f : WeekSelector.ARROW_DISABLED_ALPHA
        );
        this.icon_right.SetAlpha(
             this.has_right ? 1.0f : WeekSelector.ARROW_DISABLED_ALPHA
        );

        return true;
    }

    public void SelectDefault() {
        for (int i = 0 ; i < this.list_size ; i++) {
            if (this.list[i].name == Funkin.DIFFICULT_NORMAL) {
                Select(i);
                return;
            }
        }
        Select(0);
    }

    public bool Scroll(int offset) {
        return Select(this.index + offset);
    }

    public void SetVisible(bool visible) {
        this.drawable.SetVisible(visible);
    }

    public string GetSelected() {
        if (this.index < 0 || this.index >= this.list_size) return null;
        Item difficult = this.list[this.index];
        return difficult.is_locked ? null : difficult.name;
    }

    public StateSprite PeekStateSprite() {
        if (this.index < 0 || this.index >= this.list_size) return null;

        if (this.list[this.index].is_common)
            return this.sprite_commons;
        else
            return this.sprite_customs;
    }

    public void Relayout(bool use_alt_placeholder) {
        LayoutPlaceholder old_placeholder = use_alt_placeholder ? this.placeholder1 : this.placeholder2;
        LayoutPlaceholder placeholder = use_alt_placeholder ? this.placeholder2 : this.placeholder1;
        if (placeholder == null) return;

        if (old_placeholder != null) old_placeholder.vertex = null;
        placeholder.vertex = this.drawable;
        this.drawable.HelperUpdateFromPlaceholder(placeholder);

        // resize&locate left arrow
        ImgUtils.CalcRectangleInSprite(
            placeholder.x, placeholder.y, placeholder.width, placeholder.height,
            Align.START, Align.CENTER, this.icon_left
        );

        // resize&locate right arrow
        ImgUtils.CalcRectangleInSprite(
            placeholder.x, placeholder.y, placeholder.width, placeholder.height,
            Align.END, Align.CENTER, this.icon_right
        );

        // calculte the horizontal space between arrows
        float draw_width, draw_height;
        this.icon_left.GetDrawSize(out draw_width, out draw_height);
        float difficult_width = placeholder.width - (draw_width * 2f);

        // resize&locate commons difficults
        foreach (StateSpriteState state in this.sprite_commons.StateList()) {
            ImgUtils.CalcRectangleInStateSpriteState(
                draw_width, 0f, difficult_width, placeholder.height, Align.CENTER, Align.CENTER, state
            );
        }
        this.sprite_commons.SetDrawLocation(placeholder.x, placeholder.y);
        this.sprite_commons.StateApply(null);

        // resize&locate custom difficults
        foreach (StateSpriteState state in this.sprite_customs.StateList()) {
            ImgUtils.CalcRectangleInStateSpriteState(
                draw_width, 0f, difficult_width, placeholder.height, Align.CENTER, Align.CENTER, state
            );
        }
        this.sprite_customs.SetDrawLocation(placeholder.x, placeholder.y);
        this.sprite_customs.StateApply(null);

        // resize&locate locked icon
        ImgUtils.CalcRectangleInSprite(
            placeholder.x, placeholder.y, placeholder.width, placeholder.height,
            Align.CENTER, Align.CENTER, this.icon_locked
        );
    }

    public bool SelectedIsLocked() {
        if (this.index < 0 || this.index >= this.list_size) return true;
        return this.list[this.index].is_locked;
    }


    public void Load(WeekInfo weekinfo, string default_difficult) {
        if (this.animsprite != null) this.animsprite.Restart();

        // dispose previous list and custom difficults
        //free(this.list);
        this.sprite_customs.Destroy();
        this.sprite_customs = StateSprite.InitFromTexture(null);
        this.sprite_customs.SetVisible(false);

        if (weekinfo == null) return;

        this.list_size = 3;
        if (!String.IsNullOrEmpty(weekinfo.custom_difficults_model)) this.list_size += weekinfo.custom_difficults_size;

        int index = 0;
        this.list = new Item[this.list_size];

        if (weekinfo.has_difficulty_easy)
            this.list[index++] = new Item() { name = Funkin.DIFFICULT_EASY, is_locked = false, is_common = true };
        if (weekinfo.has_difficulty_normal)
            this.list[index++] = new Item() { name = Funkin.DIFFICULT_NORMAL, is_locked = false, is_common = true };
        if (weekinfo.has_difficulty_hard)
            this.list[index++] = new Item() { name = Funkin.DIFFICULT_HARD, is_locked = false, is_common = true };

        if (!String.IsNullOrEmpty(weekinfo.custom_difficults_model)) {
            ModelHolder modelholder = ModelHolder.Init(weekinfo.custom_difficults_model);

            if (modelholder != null) {
                for (int i = 0 ; i < weekinfo.custom_difficults_size ; i++) {
                    WeekInfo.CustomDifficult difficult = weekinfo.custom_difficults[i];
                    this.list[index++] = new Item() {
                        name = difficult.name,
                        is_locked = !FunkinSave.ContainsUnlockDirective(difficult.unlock_directive),
                        is_common = false
                    };
                    this.sprite_customs.StateAdd(
                         modelholder, difficult.name, difficult.name
                    );
                }
                modelholder.Destroy();
            }
        }

        Debug.Assert(index <= this.list_size, "weekselector_difficult_load() list overflow");
        this.list_size = index;// trim the list

        Relayout(false);

        // select default difficult
        string selected = default_difficult == null ? Funkin.DIFFICULT_NORMAL : default_difficult;
        string last_difficult_played = FunkinSave.GetLastPlayedDifficult();// read-only string
        if (String.IsNullOrEmpty(default_difficult) && !String.IsNullOrEmpty(last_difficult_played)) selected = last_difficult_played;

        // select normal difficult
        for (int i = 0 ; i < this.list_size ; i++) {
            if (this.list[i].name == selected) {
                Select(i);
                return;
            }
        }

        // fallback to first one
        Select(0);
    }


    private struct Item {
        public string name;
        public bool is_locked;
        public bool is_common;
    }

}
