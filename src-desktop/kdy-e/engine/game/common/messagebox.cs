using System;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Font;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game.Common;

public class MessageBox : IDraw, IAnimate {

    public const string LAYOUT = "/assets/common/image/messagebox.xml";

    private Sprite image;
    private Layout layout;
    private Atlas atlas;
    private Drawable drawable;
    private bool hide_animation;
    private bool small;
    private bool full_title;

    public MessageBox() {
        Layout layout = Layout.Init(MessageBox.LAYOUT);
        if (layout == null) throw new Exception("can not load " + MessageBox.LAYOUT);

        Atlas atlas = null;
        string atlas_src = (string)layout.GetAttachedValue("btn-atlas", AttachedValueType.STRING, null);
        if (atlas_src != null) atlas = Atlas.Init(atlas_src);


        this.image = Sprite.Init(null);
        this.layout = layout; this.atlas = atlas;
        this.drawable = null;
        this.hide_animation = false;
        this.small = false;
        this.full_title = false;


        this.drawable = new Drawable(-1, this, this);
        this.drawable.SetVisible(false);
        this.layout.ExternalVertexCreateEntries(1);

        this.ShowButtonsIcons(false);

    }

    public void Destroy() {
        Luascript.DropShared(this);
        this.layout.Destroy();
        if (this.atlas != null) this.atlas.Destroy();
        this.drawable.Destroy();
        this.image.DestroyFull();
    }


    public void SetButtonsText(string left_text, string right_text) {
        this.layout.SetGroupVisibility("btn1", false);
        this.layout.SetGroupVisibility("btn2", true);

        this.layout.TriggerAny(
            this.small ? "rest-fntsize-small" : "rest-fntsize-normal"
        );

        InternalSetText("btn1text", left_text);
        InternalSetText("btn2text", right_text);

        this.layout.SetGroupVisibility("btn1g", left_text != null);
        this.layout.SetGroupVisibility("btn2g", right_text != null);
    }

    public void SetButtonSingle(string center_text) {
        this.layout.SetGroupVisibility("btn1", true);
        this.layout.SetGroupVisibility("btn2", false);

        this.layout.TriggerAny(
             this.small ? "rest-fntsize-small" : "rest-fntsize-normal"
        );

        InternalSetText("btntext", center_text);
    }

    public void SetButtonsIcons(string left_icon_name, string right_icon_name) {
        ShowButtonsIcons(true);
        InternalSetIcon("btn2icn1", left_icon_name);
        InternalSetIcon("btn2icn2", right_icon_name);
        this.layout.TriggerAny("btn-resize");
    }

    public void SetButtonSingleIcon(string center_icon_name) {
        ShowButtonsIcons(true);
        InternalSetIcon("btn1icn", center_icon_name);
        this.layout.TriggerAny("btn-resize");
    }

    public void SetTitle(string text) {
        TextSprite textsprite = this.layout.GetTextsprite("title");
        if (textsprite != null) textsprite.SetTextIntern(false, text);
    }

    public void SetImageBackgroundColor(uint color_rgb8) {
        Sprite sprite = this.layout.GetSprite("imgbg");
        if (sprite != null) sprite.SetVertexColorRGB8(color_rgb8);
    }

    public void SetImageBackgroundColorDefault() {
        this.layout.TriggerAny("def-imgcolor");
    }

    public void SetMessage(string text) {
        TextSprite textsprite = this.layout.GetTextsprite("message");
        if (textsprite != null) textsprite.SetTextIntern(false, text);
    }

    public void SetMessageFormated(string format, params object[] values) {
        TextSprite textsprite = this.layout.GetTextsprite("message");
        if (textsprite != null) textsprite.SetTextFormated2(format, values);
    }

    public void HideImageBackground(bool hide) {
        this.layout.TriggerAny(hide ? "no-imgbg" : "with-imgbg");
    }

    public void HideImage(bool hide) {
        this.layout.TriggerAny(hide ? "no-img" : "with-img");
        this.layout.SetGroupVisibility("imgg", !hide);
    }

    public void HideButtons() {
        this.layout.SetGroupVisibility("btn1", false);
        this.layout.SetGroupVisibility("btn2", false);
        this.layout.TriggerAny(
            this.small ? "rest-fntsize-small" : "rest-fntsize-normal"
        );
    }

    public void ShowButtonsIcons(bool show) {
        this.layout.TriggerAny(show ? "btnicn-show" : "btnicn-hide");

        if (!show) {
            InternalSetIcon("btn2icn1", null);
            InternalSetIcon("btn2icn2", null);
            InternalSetIcon("btn1icn", null);
        }
    }

    public void UseSmallSize(bool small_or_normal) {
        if (this.small == small_or_normal) return;
        if (small_or_normal) HideImage(true);
        this.layout.TriggerAny(small_or_normal ? "size-small" : "size-normal");
        this.small = small_or_normal;
        UseFullTitle(this.full_title);
    }

    public void UseFullTitle(bool enable) {
        this.full_title = !enable;
        if (enable)
            this.layout.TriggerAny(this.small ? "full-title-small" : "full-title-big");
        else
            this.layout.TriggerAny("no-full-title");
    }

    public void SetImageSprite(Sprite sprite) {
        if (sprite == null) {
            this.layout.ExternalVertexSetEntry(0, PVRContextVertex.SPRITE, sprite, -1);
            return;
        }

        LayoutPlaceholder placeholder = this.layout.GetPlaceholder("img");
        if (placeholder == null) return;

        float[] draw_size = new float[2];
        float[] draw_location = new float[2];

        sprite.GetSourceSize(out draw_size[0], out draw_size[1]);
        ImgUtils.CalcRectangle(
            placeholder.x, placeholder.y, placeholder.width, placeholder.height,
            draw_size[0], draw_size[1],
            Align.CENTER, Align.CENTER,
            draw_size, draw_location
        );

        sprite.SetDrawSize(draw_size[0], draw_size[1]);
        sprite.SetDrawLocation(draw_location[0], draw_location[1]);
        sprite.SetZIndex(placeholder.z);
        sprite.SetVisible(true);
        this.layout.ExternalVertexSetEntry(0, PVRContextVertex.SPRITE, sprite, -1);
    }

    public void SetImageFromTexture(string filename) {
        Texture texture = Texture.Init(filename);

        this.image.SetVisible(texture != null);
        this.image.DestroyTexture();
        this.image.DestroyAllAnimations();

        if (texture == null) {
            this.image.SetTexture(texture, true);
            this.image.SetDrawSizeFromSourceSize();
        } else {
            Console.Error.WriteLine("[WARN] messagebox_set_image_from_texture() can not load: " + filename);
        }

        SetImageSprite(this.image);
    }

    public void SetImageFromAtlas(string filename, string entry_name, bool is_animation) {
        Atlas atlas = null;
        Texture texture = null;

        this.image.DestroyTexture();
        this.image.DestroyAllAnimations();

        atlas = Atlas.Init(filename);
        if (atlas == null) goto L_dipose_and_return;

        texture = Texture.Init(filename);
        if (texture == null) goto L_dipose_and_return;

        this.image.SetTexture(texture, true);

        if (is_animation) {
            AnimSprite animsprite = AnimSprite.InitFromAtlas(
                Funkin.DEFAULT_ANIMATIONS_FRAMERATE, 0, atlas, entry_name, true
            );
            if (animsprite == null) goto L_dipose_and_return;

            this.image.ExternalAnimationSet(animsprite);
            this.image.Animate(0);// brief animate
        } else {
            AtlasEntry atlas_entry = atlas.GetEntry(entry_name);
            if (atlas_entry == null) goto L_dipose_and_return;

            Atlas.ApplyFromEntry(this.image, atlas_entry, true);
        }

        this.image.SetVisible(true);
        SetImageSprite(this.image);
        return;


L_dipose_and_return:
        if (atlas != null) atlas.Destroy();
        if (texture != null) texture.Destroy();

        Console.Error.WriteLine("[WARN]" +
            "messagebox_set_image_from_atlas() can not load the atlas, texture or entry: " + filename
        );

        this.image.SetVisible(false);
        SetImageSprite(this.image);
    }

    public void Hide(bool animated) {
        if (animated) {
            this.layout.TriggerCamera("hide-anim");
            this.layout.Animate(0);// brief animate
            this.hide_animation = true;
        } else {
            this.drawable.SetVisible(false);
        }
    }

    public void Show(bool animated) {
        this.drawable.SetVisible(true);
        this.hide_animation = false;

        if (animated)
            this.layout.TriggerCamera("show-anim");
        else
            this.layout.TriggerCamera("show-static");

        this.layout.Animate(0);// brief animate
    }

    public void SetZIndex(float z_index) {
        this.drawable.SetZIndex(z_index);
    }


    public Drawable GetDrawable() {
        return this.drawable;
    }

    public Modifier GetModifier() {
        return this.drawable.GetModifier();
    }

    public int Animate(float elapsed) {
        if (this.hide_animation && this.layout.CameraIsCompleted()) {
            this.hide_animation = false;
            this.drawable.SetVisible(false);
            return 1;
        }

        return this.layout.Animate(elapsed);
    }

    public void Draw(PVRContext pvrctx) {
        if (!this.drawable.IsVisible()) return;
        this.drawable.HelperApplyInContext(pvrctx);
        this.layout.Draw(pvrctx);
    }



    private void InternalSetText(string name, string text) {
        TextSprite textsprite = this.layout.GetTextsprite(name);
        if (textsprite == null) return;

        textsprite.SetTextIntern(false, text);

        float draw_width, draw_height;
        textsprite.GetDrawSize(out draw_width, out draw_height);

        float width = draw_width;
        textsprite.GetMaxDrawSize(out draw_width, out draw_height);

        if (width <= draw_width) return;

        float font_size = (draw_width * textsprite.GetFontSize()) / width;
        textsprite.SetFontSize(font_size);
    }

    private void InternalSetIcon(string name, string icon_name) {
        Sprite sprite = this.layout.GetSprite(name);
        if (sprite == null) return;

        bool visible = icon_name != null && this.atlas != null;
        if (visible && this.atlas.Apply(sprite, icon_name, true)) visible = false;

        sprite.SetVisible(visible);
    }

}
