using System;
using System.Diagnostics;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Font;
using Engine.Game.Gameplay.Helpers;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game.Common;

public delegate bool MenuDrawCallback(PVRContext pvrctx, Menu menu, int index, float item_x, float item_y, float item_width, float item_height);

public class Menu : IAnimate, IDraw {


    private MenuItem[] items;
    private int items_size;
    private FontHolder fontholder;
    private float transition_delay_in;
    private float transition_delay_out;
    private bool is_vertical;
    private Align align;
    private float gap;
    private bool sparse;
    private int static_index;
    private TweenLerp tweenlerp;
    private Drawable drawable;
    private float x;
    private float y;
    private float width;
    private float height;
    private int index_selected;
    private bool item_choosen;
    private bool transition_active;
    private bool transition_out;
    private int render_start;
    private int render_end;
    private float render_distance;
    private float render_distance_end;
    private float render_distance_last;
    private bool per_page;
    private int[] map;
    private int map_size;
    private bool drawcallback_before;
    private MenuDrawCallback drawcallback;


    public Menu(MenuManifest menumanifest, float x, float y, float z, float width, float height) {
        MenuManifest.Parameters @params = menumanifest.parameters;
        Debug.Assert(menumanifest.items.Length == menumanifest.items_size);

        MenuItem[] items = new MenuItem[menumanifest.items_size];
        ModelHolder modelholder = ModelHolder.Init2(@params.font_color, @params.atlas, @params.animlist);

        FontHolder fontholder = null;
        if (@params.font != null) {
            fontholder = new FontHolder(@params.font, @params.font_size, @params.font_glyph_suffix);
            if (fontholder.font == null) throw new Exception("Missing or invalid font: " + @params.font);
            if (fontholder.font_from_atlas) {
                fontholder.font.EnableColorByAddition(@params.font_color_by_addition);
            }
        }

        float[] border = new float[] { 0f, 0f, 0f, 0f };
        Math2D.ColorBytesToFloats(@params.font_border_color, true, border);

        AnimSprite anim_discarded = null;
        AnimSprite anim_idle = null;
        AnimSprite anim_rollback = null;
        AnimSprite anim_selected = null;
        AnimSprite anim_choosen = null;
        AnimSprite anim_in = null;
        AnimSprite anim_out = null;

        if (!String.IsNullOrEmpty(@params.anim_discarded))
            anim_discarded = modelholder.CreateAnimsprite(@params.anim_discarded, false, false);
        if (!String.IsNullOrEmpty(@params.anim_idle))
            anim_idle = modelholder.CreateAnimsprite(@params.anim_idle, false, false);
        if (!String.IsNullOrEmpty(@params.anim_rollback))
            anim_rollback = modelholder.CreateAnimsprite(@params.anim_rollback, false, false);
        if (!String.IsNullOrEmpty(@params.anim_selected))
            anim_selected = modelholder.CreateAnimsprite(@params.anim_selected, false, false);
        if (!String.IsNullOrEmpty(@params.anim_choosen))
            anim_choosen = modelholder.CreateAnimsprite(@params.anim_choosen, false, false);
        if (!String.IsNullOrEmpty(@params.anim_in))
            anim_in = modelholder.CreateAnimsprite(@params.anim_in, false, false);
        if (!String.IsNullOrEmpty(@params.anim_out))
            anim_out = modelholder.CreateAnimsprite(@params.anim_out, false, false);

        for (int i = 0 ; i < menumanifest.items_size ; i++) {
            MenuManifest.Item src_item = menumanifest.items[i];

            items[i] = new MenuItem() {
                name = null,

                anim_self = null,

                anim_choosen = null,
                anim_discarded = null,
                anim_idle = null,
                anim_rollback = null,
                anim_selected = null,
                anim_in = null,
                anim_out = null,

                is_text = false,
                vertex = null,

                center_x = 0,
                center_y = 0,

                cell_dimmen = Single.NaN,
                cell_gap = menumanifest.items[i].placement.gap,
                placement_x = menumanifest.items[i].placement.x,
                placement_y = menumanifest.items[i].placement.y,

                rollback_active = false,
                render_distance = 0f,
                has_scale = false,

                hidden = false
            };

            InternalBuildItem(items[i], src_item, @params, modelholder, fontholder, border);

            if (items[i].anim_choosen == null) items[i].anim_choosen = anim_choosen != null ? anim_choosen.Clone() : null;
            if (items[i].anim_discarded == null) items[i].anim_discarded = anim_discarded != null ? anim_discarded.Clone() : null;
            if (items[i].anim_idle == null) items[i].anim_idle = anim_idle != null ? anim_idle.Clone() : null;
            if (items[i].anim_rollback == null) items[i].anim_rollback = anim_rollback != null ? anim_rollback.Clone() : null;
            if (items[i].anim_selected == null) items[i].anim_selected = anim_selected != null ? anim_selected.Clone() : null;
            if (items[i].anim_in == null) items[i].anim_in = anim_in != null ? anim_in.Clone() : null;
            if (items[i].anim_out == null) items[i].anim_out = anim_out != null ? anim_out.Clone() : null;
        }

        if (anim_discarded != null) anim_discarded.Destroy();
        if (anim_idle != null) anim_idle.Destroy();
        if (anim_rollback != null) anim_rollback.Destroy();
        if (anim_selected != null) anim_selected.Destroy();
        if (anim_choosen != null) anim_choosen.Destroy();
        if (anim_in != null) anim_in.Destroy();
        if (anim_out != null) anim_out.Destroy();

        modelholder.Destroy();

        Align align;
        switch (@params.items_align) {
            case Align.START:
            case Align.CENTER:
            case Align.END:
                align = @params.items_align;
                break;
            default:
                align = Align.START;
                break;
        }


        this.items = items;
        this.items_size = menumanifest.items_size;

        this.fontholder = fontholder;

        this.transition_delay_in = @params.anim_transition_in_delay;
        this.transition_delay_out = @params.anim_transition_out_delay;

        this.is_vertical = @params.is_vertical;
        this.align = align;
        this.gap = @params.items_gap;
        this.sparse = @params.is_sparse;
        this.static_index = @params.static_index;
        this.tweenlerp = null;

        this.drawable = null;
        this.x = x; this.y = y; this.width = width; this.height = height;

        this.index_selected = -1;
        this.item_choosen = false;

        this.transition_active = false;
        this.transition_out = false;

        this.render_start = 0;
        this.render_end = -1;
        this.render_distance = 0;
        this.render_distance_end = 0;
        this.render_distance_last = 0;

        this.per_page = @params.is_per_page;
        this.map = null;
        this.map_size = 0;

        this.drawcallback_before = false;
        this.drawcallback = null;

        if (@params.static_index != 0) {
            this.tweenlerp = new TweenLerp();
            this.tweenlerp.AddLinear(0, 0f, 0f, 100f);
        }

        this.drawable = new Drawable(z, this, this);

        Modifier modifier = this.drawable.GetModifier();
        modifier.x = x;
        modifier.y = y;
        modifier.width = width;
        modifier.height = height;

        if (this.sparse) {
            for (int i = 0 ; i < this.items_size ; i++) {
                this.items[i].center_x = menumanifest.items[i].placement.x;
                this.items[i].center_y = menumanifest.items[i].placement.y;
            }
        } else {
            InternalBuildMap();
        }

        // select the first visible item
        for (int i = 0 ; i < this.items_size ; i++) {
            if (!this.items[i].hidden) {
                this.InternalSetIndexSelected(i);
                break;
            }
        }

    }

    public void Destroy() {
        Luascript.DropShared(this);

        if (this.fontholder != null) this.fontholder.Destroy();

        for (int i = 0 ; i < this.items_size ; i++) {
            if (this.items[i].is_text) {
                this.items[i].vertex.Destroy();
            } else {
                ((StateSprite)this.items[i].vertex).DestroyTextureIfStateless();// important step
                this.items[i].vertex.Destroy();
            }

            //free(this.items[i].name);

            if (this.items[i].anim_self != null) this.items[i].anim_self.Destroy();
            if (this.items[i].anim_discarded != null) this.items[i].anim_discarded.Destroy();
            if (this.items[i].anim_idle != null) this.items[i].anim_idle.Destroy();
            if (this.items[i].anim_rollback != null) this.items[i].anim_rollback.Destroy();
            if (this.items[i].anim_selected != null) this.items[i].anim_selected.Destroy();
            if (this.items[i].anim_choosen != null) this.items[i].anim_choosen.Destroy();
            if (this.items[i].anim_in != null) this.items[i].anim_in.Destroy();
            if (this.items[i].anim_out != null) this.items[i].anim_out.Destroy();
        }

        if (this.tweenlerp != null) this.tweenlerp.Destroy();

        this.drawable.Destroy();

        //free(this.items);
        //free(this.map);
        //free();
    }


    public Drawable GetDrawable() {
        return this.drawable;
    }

    public void TrasitionIn() {
        this.transition_out = false;
        InternalTransition();
    }

    public void TrasitionOut() {
        this.transition_out = true;
        InternalTransition();
    }


    public bool SelectItem(string name) {
        for (int i = 0 ; i < this.items_size ; i++) {
            if (this.items[i].name == name) {
                if (this.items[i].hidden) return false;
                InternalSetIndexSelected(i);
                return true;
            };
        }
        InternalSetIndexSelected(-1);
        return false;
    }

    public void SelectIndex(int index) {
        if (index >= 0 && index < this.items_size && this.items[index].hidden) return;
        InternalSetIndexSelected(index);
    }

    public bool SelectVertical(int offset) {
        if (this.sparse) return InternalSetSelectedSparse(0, offset);
        if (!this.is_vertical) return false;

        return InternalScroll(offset);
    }

    public bool SelectHorizontal(int offset) {
        if (this.sparse) return InternalSetSelectedSparse(offset, 0);
        if (this.is_vertical) return false;

        return InternalScroll(offset);
    }

    public void ToggleChoosen(bool enable) {
        this.item_choosen = enable;
        for (int i = 0 ; i < this.items_size ; i++) {
            if (this.items[i].anim_choosen != null) this.items[i].anim_choosen.Restart();
            if (this.items[i].anim_discarded != null) this.items[i].anim_discarded.Restart();

            if (this.index_selected == i) {
                this.items[i].rollback_active = false;

                if (this.items[i].anim_rollback != null) {
                    this.items[i].anim_rollback.Restart();
                    this.items[i].anim_rollback.ForceEnd();

                    if (this.items[i].is_text) {
                        this.items[i].anim_rollback.UpdateTextsprite((TextSprite)this.items[i].vertex, true);
                    } else {
                        this.items[i].anim_rollback.UpdateStatesprite((StateSprite)this.items[i].vertex, true);
                    }
                }
            }
        }
    }

    public int GetSelectedIndex() {
        return this.index_selected;
    }

    public int GetItemsCount() {
        return this.items_size;
    }

    public bool SetItemText(int index, string text) {
        if (index < 0 || index >= this.items_size) return false;
        if (!this.items[index].is_text) return false;
        ((TextSprite)this.items[index].vertex).SetTextIntern(false, text);
        return true;
    }

    public bool SetItemVisibility(int index, bool visible) {
        if (index < 0 || index >= this.items_size) return false;

        this.items[index].hidden = !visible;

        if (this.index_selected == index) {
            for (index = this.index_selected ; index > 0 ; index--) {
                if (!this.items[this.index_selected].hidden) break;
            }
            InternalSetIndexSelected(this.index_selected);
        }

        InternalBuildMap();
        return true;
    }

    public bool HasValidSelection() {
        return this.index_selected >= 0 && this.index_selected < this.items_size;
    }


    public int Animate(float elapsed) {
        for (int i = 0 ; i < this.items_size ; i++) {
            InternalAnimate(i, this.items[i].anim_self, elapsed);

            if (this.index_selected == i) {
                AnimSprite anim = this.item_choosen ? this.items[i].anim_choosen : this.items[i].anim_selected;
                InternalAnimate(i, anim, elapsed);
            } else {
                if (this.item_choosen) {
                    InternalAnimate(i, this.items[i].anim_discarded, elapsed);
                } else if (this.items[i].rollback_active) {
                    if (InternalAnimate(i, this.items[i].anim_rollback, elapsed) > 0) {
                        this.items[i].rollback_active = false;
                    }
                } else {
                    InternalAnimate(i, this.items[i].anim_idle, elapsed);
                }
            }

            if (this.transition_active) {
                if (this.transition_out)
                    InternalAnimate(i, this.items[i].anim_out, elapsed);
                else
                    InternalAnimate(i, this.items[i].anim_in, elapsed);
            }

            InternalCalcItemBounds(i);
        }

        if (this.fontholder != null && this.fontholder.font_from_atlas)
            this.fontholder.font.Animate(elapsed);

        if (this.tweenlerp != null) this.tweenlerp.Animate(elapsed);

        return 0;
    }

    public void Draw(PVRContext pvrctx) {
        pvrctx.Save();

        if (this.sparse) {
            for (int i = 0 ; i < this.items_size ; i++) {
                if (this.items[i].hidden) continue;

                if (this.drawcallback_before && this.InternalDrawCallback(pvrctx, i)) continue;

                if (this.items[i].is_text)
                    this.items[i].vertex.Draw(pvrctx);
                else
                    this.items[i].vertex.Draw(pvrctx);

                if (!this.drawcallback_before && this.InternalDrawCallback(pvrctx, i)) break;
            }
        } else {
            float render_distance;

            if (this.tweenlerp != null) {
                if (this.render_distance_last != this.render_distance) {
                    if (this.tweenlerp.IsCompleted()) this.tweenlerp.Restart();
                    this.tweenlerp.ChangeBoundsByIndex(
                        0, this.render_distance_last, this.render_distance
                    );
                    this.render_distance_last = this.render_distance;
                }
                render_distance = this.tweenlerp.PeekValueByIndex(0);
            } else {
                render_distance = this.render_distance;
            }

            if (this.is_vertical)
                pvrctx.CurrentMatrix.TranslateY(render_distance);
            else
                pvrctx.CurrentMatrix.TranslateX(render_distance);

            //pvrctx.Flush();

            for (int i = 0 ; i < this.items_size ; i++) {
                if (this.items[i].hidden) continue;

                // ignore items outside of the visible space
                if (i < this.render_start || i > this.render_end) continue;

                if (this.drawcallback_before && this.InternalDrawCallback(pvrctx, i)) continue;

                if (this.items[i].is_text)
                    this.items[i].vertex.Draw(pvrctx);
                else
                    this.items[i].vertex.Draw(pvrctx);

                if (!this.drawcallback_before && this.InternalDrawCallback(pvrctx, i)) break;
            }
        }
        pvrctx.Restore();
    }


    public bool GetItemRect(int index, out float x, out float y, out float width, out float height) {
        if (index < 0 || index >= this.items_size) {
            x = y = width = height = Single.NaN;
            return false;
        }

        MenuItem item = this.items[index];
        if (item.is_text) {
            item.vertex.GetDrawLocation(out x, out y);
            item.vertex.GetDrawSize(out width, out height);
        } else {
            item.vertex.GetDrawLocation(out x, out y);
            item.vertex.GetDrawSize(out width, out height);
        }

        if (this.is_vertical)
            y += this.render_distance;
        else
            x += this.render_distance;

        return true;
    }

    public bool GetSelectedItemRect(out float x, out float y, out float width, out float height) {
        return GetItemRect(this.index_selected, out x, out y, out width, out height);
    }


    public string GetSelectedItemName() {
        if (this.index_selected < 0 || this.index_selected >= this.items_size) return null;

        return this.items[this.index_selected].name;
    }

    public void SetTextForceCase(int none_or_lowercase_or_uppercase) {
        for (int i = 0 ; i < this.items_size ; i++) {
            if (this.items[i].is_text) {
                ((TextSprite)this.items[i].vertex).ForceCase(none_or_lowercase_or_uppercase);
            }
        }
    }

    public void SetDrawCallback(bool before_or_after, MenuDrawCallback callback) {
        this.drawcallback_before = before_or_after;
        this.drawcallback = callback;
    }

    public bool HasItem(string name) {
        for (int i = 0 ; i < this.items_size ; i++) {
            if (this.items[i].name == name) {
                return true;
            }
        }
        return false;
    }

    public int IndexOfItem(string name) {
        for (int i = 0 ; i < this.items_size ; i++) {
            if (this.items[i].name == name) return i;
        }
        return -1;
    }


    private void InternalBuildItem(MenuItem item, MenuManifest.Item src_item, MenuManifest.Parameters @params, ModelHolder modelholder, FontHolder fontholder, float[] border) {
        bool custom_modelholder = false;
        item.is_text = !String.IsNullOrEmpty(src_item.text);
        item.name = src_item.name;
        item.hidden = src_item.hidden;

        float dimmen = item.is_text ? @params.font_size : @params.items_dimmen;
        if (src_item.placement.dimmen > 0) dimmen = src_item.placement.dimmen;

        if (item.is_text) {
            uint font_color = @params.font_color;
            if (src_item.has_font_color) font_color = src_item.font_color;

            // important: do not intern the text
            item.vertex = TextSprite.Init2(fontholder, dimmen, font_color);
            ((TextSprite)item.vertex).SetTextIntern(false, src_item.text);
            if (@params.font_border_size > 0) {
                ((TextSprite)item.vertex).BorderEnable(true);
                ((TextSprite)item.vertex).BorderSetColor(
                    border[0], border[1], border[2], border[3]
                );
                ((TextSprite)item.vertex).BorderSetSize(@params.font_border_size);
            }
        } else {
            if (src_item.model != null) {
                custom_modelholder = true;
                modelholder = ModelHolder.Init(src_item.model);
            }
            StateSprite statesprite = StateSprite.InitFromTexture(modelholder.GetTexture(true));
            statesprite.SetVertexColorRGB8(modelholder.GetVertexColor());
            if (modelholder.IsInvalid()) statesprite.SetAlpha(0);

            item.vertex = statesprite;

            float scale = src_item.texture_scale > 0 ? src_item.texture_scale : @params.texture_scale;
            if (scale > 0) {
                item.has_scale = true;
                ((StateSprite)item.vertex).ChangeDrawSizeInAtlasApply(true, scale);
            }
        }

        item.anim_self = InternalLoadAnim(
            modelholder, src_item.name, null, null
        );
        item.anim_selected = InternalLoadAnim(
            modelholder, src_item.anim_selected, src_item.name, @params.suffix_selected
        );
        item.anim_choosen = InternalLoadAnim(
            modelholder, src_item.anim_choosen, src_item.name, @params.suffix_choosen
        );
        item.anim_discarded = InternalLoadAnim(
            modelholder, src_item.anim_discarded, src_item.name, @params.suffix_discarded
        );
        item.anim_idle = InternalLoadAnim(
            modelholder, src_item.anim_idle, src_item.name, @params.suffix_idle
        );
        item.anim_rollback = InternalLoadAnim(
            modelholder, src_item.anim_rollback, src_item.name, @params.suffix_rollback
        );
        item.anim_in = InternalLoadAnim(
            modelholder, src_item.anim_in, src_item.name, @params.suffix_in
        );
        item.anim_out = InternalLoadAnim(
            modelholder, src_item.anim_out, src_item.name, @params.suffix_out
        );

        if (custom_modelholder) modelholder.Destroy();

        if (item.anim_self != null) {
            if (item.is_text)
                item.anim_self.UpdateTextsprite((TextSprite)item.vertex, false);
            else
                item.anim_self.UpdateStatesprite((StateSprite)item.vertex, false);
        }

        if (!@params.is_vertical && item.is_text && @params.enable_horizontal_text_correction) {
            ((TextSprite)item.vertex).GetDrawSize(out dimmen, out _);
        }

        item.cell_dimmen = dimmen;
        item.cell_gap = !Single.IsNaN(src_item.placement.gap) ? src_item.placement.gap : 0;
    }

    private AnimSprite InternalLoadAnim(ModelHolder modelholder, string absolute_name, string prefix, string suffix) {
        string temp;
        if (!String.IsNullOrEmpty(absolute_name))
            temp = absolute_name;
        else if (!String.IsNullOrEmpty(prefix) && !String.IsNullOrEmpty(suffix))
            temp = StringUtils.ConcatForStateName(prefix, suffix);
        else
            return null;

        if (temp == null) return null;

        AnimSprite animsprite = modelholder.CreateAnimsprite(temp, true, String.IsNullOrEmpty(prefix) && String.IsNullOrEmpty(suffix));
        //if (!String.IsNullOrEmpty(absolute_name)) free(temp);

        return animsprite;
    }

    private void InternalCalcItemBounds(int index) {
        float draw_width, draw_height;
        MenuItem menu_item = this.items[index];
        float offset_x, offset_y;

        if (menu_item.is_text) {
            this.items[index].vertex.GetDrawSize(out draw_width, out draw_height);
        } else if (menu_item.has_scale) {
            menu_item.vertex.GetDrawSize(out draw_width, out draw_height);
        } else {
            float max_width, max_height;
            if (this.is_vertical) {
                max_width = -1f;
                max_height = menu_item.cell_dimmen;
            } else {
                max_width = menu_item.cell_dimmen;
                max_height = -1f;
            }
            ((StateSprite)menu_item.vertex).ResizeDrawSize(max_width, max_height, out draw_width, out draw_height);
        }

        if (this.sparse) {
            offset_x = (draw_width / -2f) + this.x;
            offset_y = (draw_height / -2f) + this.y;
        } else if (this.align == Align.START) {
            offset_x = 0f;
            offset_y = 0f;
        } else {
            if (this.is_vertical) {
                offset_x = this.width - draw_width;
                if (this.align == Align.CENTER) offset_x /= 2f;
                offset_y = 0f;
            } else {
                offset_y = this.height - draw_height;
                if (this.align == Align.CENTER) offset_y /= 2f;
                offset_x = 0f;
            }
        }

        offset_x += this.items[index].center_x;
        offset_y += this.items[index].center_y;

        if (this.items[index].is_text)
            ((TextSprite)this.items[index].vertex).SetDrawLocation(offset_x, offset_y);
        else
            ((StateSprite)this.items[index].vertex).SetDrawLocation(offset_x, offset_y);
    }

    private int InternalAnimate(int index, AnimSprite anim, float elapsed) {
        if (anim == null) return 1;

        int completed = anim.Animate(elapsed);

        if (this.items[index].is_text)
            anim.UpdateTextsprite((TextSprite)this.items[index].vertex, true);
        else
            anim.UpdateStatesprite((StateSprite)this.items[index].vertex, true);

        return completed;
    }

    private void InternalSetIndexSelected(int new_index) {
        int old_index = this.index_selected;

        if (old_index >= 0 && old_index < this.items_size) {
            this.items[old_index].rollback_active = this.items[old_index].anim_rollback != null;

            if (this.items[old_index].anim_rollback != null) {
                this.items[old_index].anim_rollback.Restart();
                if (this.item_choosen) {
                    this.items[old_index].anim_rollback.ForceEnd();
                    if (this.items[old_index].is_text) {
                        this.items[old_index].anim_rollback.UpdateTextsprite(
                           (TextSprite)this.items[old_index].vertex, true
                        );
                    } else {
                        this.items[old_index].anim_rollback.UpdateStatesprite(
                            (StateSprite)this.items[old_index].vertex, true
                        );
                    }
                    this.items[old_index].rollback_active = false;
                }
            }
            if (this.items[old_index].anim_idle != null)
                this.items[old_index].anim_idle.Restart();
        }

        if (new_index >= 0 && new_index < this.items_size) {
            this.items[new_index].rollback_active = false;

            if (this.items[new_index].anim_self != null)
                this.items[new_index].anim_self.Restart();

            if (this.items[new_index].anim_rollback != null) {
                this.items[new_index].anim_rollback.Restart();
                this.items[new_index].anim_rollback.ForceEnd();

                if (this.items[new_index].is_text) {
                    this.items[new_index].anim_rollback.UpdateTextsprite(
                  (TextSprite)this.items[new_index].vertex, true
              );
                } else {
                    this.items[new_index].anim_rollback.UpdateStatesprite(
               (StateSprite)this.items[new_index].vertex, true
            );
                }
            }

            if (this.items[new_index].anim_selected != null)
                this.items[new_index].anim_selected.Restart();
        }

        this.index_selected = new_index;
        if (new_index < 0 || new_index >= this.items_size) return;
        if (this.sparse) return;


        // calculate the visible page of the list
        if (this.map == null) {
            bool has_static_index = this.static_index != 0;

            if (has_static_index) {
                int lower_index = new_index - this.static_index;
                if (lower_index >= this.items_size) lower_index = this.items_size - 1;

                while (lower_index > 0 && this.items[lower_index].hidden) lower_index--;

                if (lower_index < 0) {
                    if (old_index < 0) old_index = 0;
                    has_static_index = false;
                    goto L_stop_checking_static_index;
                }

                this.render_start = lower_index;
                this.render_end = this.items_size;
                this.render_distance = this.items[lower_index].render_distance;

            }

L_stop_checking_static_index:
            if (!has_static_index) {
                if (this.render_end > 0 && (new_index - old_index) == 1) {
                    this.render_start++;
                    this.render_end++;
                    this.render_distance = -this.items[this.render_start].render_distance;
                    return;
                }

                this.render_start = new_index;
                this.render_end = this.items_size;
                this.render_distance = this.items[new_index].render_distance;
            }

            float stop = this.render_distance + (this.is_vertical ? this.height : this.width);
            for (int i = this.items_size - 1 ; i >= 0 ; i--) {
                if (this.items[i].render_distance < stop) {
                    if (stop < this.render_distance_end) this.render_end = i - 1;
                    break;
                }
            }
            this.render_distance = -this.render_distance;
            return;
        }

        if (new_index < this.render_start && new_index > this.render_end) return;

        this.render_start = -1;
        this.render_end = this.items_size;
        this.render_distance = 0;

        for (int i = 0 ; i < this.map_size ; i++) {
            if (this.map[i] > this.index_selected) {
                this.render_end = this.map[i] - 1;
                i--;
                if (i >= 0) {
                    this.render_start = this.map[i];
                    this.render_distance = this.items[this.map[i]].render_distance;
                }
                break;
            }
        }

        if (this.render_start < 0 && this.map_size > 0) {
            this.render_start = this.map[this.map_size - 1];
            this.render_distance = this.items[this.render_start].render_distance;
        }

        this.render_distance = -this.render_distance;
    }

    private void InternalTransition() {
        AnimSprite anim;
        float transition_delay = this.transition_out ? this.transition_delay_out : this.transition_delay_in;

        bool reverse = transition_delay < 0f;
        if (reverse) transition_delay = Math.Abs(transition_delay);

        float nonvisible_first_delay = transition_delay * this.render_start;
        float nonvisible_last_delay = transition_delay * this.render_end;

        this.transition_active = true;

        for (int i = 0 ; i < this.items_size ; i++) {
            if (this.transition_out)
                anim = this.items[i].anim_out;
            else
                anim = this.items[i].anim_in;

            if (anim == null) continue;
            anim.Restart();

            float delay;

            if (i < this.render_start) {
                delay = reverse ? nonvisible_last_delay : nonvisible_first_delay;
            } else if (i > this.render_end) {
                delay = reverse ? nonvisible_last_delay : nonvisible_first_delay;
            } else {
                if (reverse)
                    delay = transition_delay * (this.render_end - i);
                else
                    delay = transition_delay * (i - this.render_start);
            }

            anim.SetDelay(delay);
        }

    }

    private bool InternalSetSelectedSparse(float offset_x, float offset_y) {
        if (this.index_selected < 0 || this.index_selected >= this.items_size) {
            SelectIndex(0);
            return true;
        }

        int short_index = -1;
        float short_distance = Single.PositiveInfinity;

        float center_x = this.items[this.index_selected].center_x;
        float center_y = this.items[this.index_selected].center_y;

        for (int i = 0 ; i < this.items_size ; i++) {
            if (this.index_selected == i) continue;
            if (this.items[i].hidden) continue;

            if (offset_x != 0) {
                if (offset_x > 0) {
                    if (this.items[i].center_x < center_x) continue;
                } else {
                    if (this.items[i].center_x > center_x) continue;
                }
            }
            if (offset_y != 0) {
                if (offset_y > 0) {
                    if (this.items[i].center_y < center_y) continue;
                } else {
                    if (this.items[i].center_y > center_y) continue;
                }
            }

            float distance = Math2D.PointsDistance(
                this.items[i].center_x, this.items[i].center_y, center_x, center_y
            );

            if (distance < short_distance) {
                short_distance = distance;
                short_index = i;
            }
        }

        if (short_index < 0) return false;

        InternalSetIndexSelected(short_index);
        return true;
    }

    private void InternalBuildMap() {
        if (this.sparse) return;

        // dispose last map created (if defined)
        this.map = null;
        this.map_size = 0;

        float accumulator = 0;
        for (int i = 0 ; i < this.items_size ; i++) {
            if (this.items[i].hidden) continue;
            this.items[i].render_distance = accumulator;
            accumulator += this.items[i].cell_dimmen + (this.items[i].cell_gap * 2) + this.gap;
        }

        float render_distance = Single.PositiveInfinity;
        for (int i = this.items_size - 1 ; i >= 0 ; i--) {
            if (this.items[i].hidden)
                this.items[i].render_distance = render_distance;
            else
                render_distance = this.items[i].render_distance;
        }

        float dimmen = this.is_vertical ? this.height : this.width;
        float pages = (accumulator - this.gap) / dimmen;

        this.render_distance_end = accumulator;

        if (this.per_page && pages > 0) {
            this.map_size = (int)Math.Ceiling(pages);
            this.map = new int[this.map_size];
        }

        int j = 0;
        float next_distance = -1;
        for (int i = 0 ; i < this.items_size ; i++) {
            if (this.items[i].hidden) continue;

            float draw_x = this.x + this.items[i].placement_x;
            float draw_y = this.y + this.items[i].placement_y;

            if (this.is_vertical) {
                draw_y += this.items[i].render_distance + this.items[i].cell_gap;
            } else {
                draw_x += this.items[i].render_distance + this.items[i].cell_gap;
            }

            if (this.map != null && (this.items[i].render_distance - this.gap) > next_distance) {
                this.map[j++] = Math.Max(0, i - 1);
                next_distance = dimmen * j;
            }

            this.items[i].center_x = draw_x;
            this.items[i].center_y = draw_y;
        }

        if (this.map != null) {
            if (j < this.map_size) this.map[j] = this.items_size - 1;

            if (this.map_size > 1)
                this.render_end = this.map[1] - 1;
            else
                this.render_end = this.items_size;
        }
    }

    private bool InternalScroll(int offset) {
        int new_index = offset + this.index_selected;

        // skip hidden items
        while (new_index >= 0 && new_index < this.items_size && this.items[new_index].hidden) {
            new_index += offset;
        }

        if (new_index < 0 || new_index >= this.items_size) return false;

        InternalSetIndexSelected(new_index);
        return true;
    }

    private bool InternalDrawCallback(PVRContext pvrctx, int index) {
        if (this.drawcallback == null) return false;

        float draw_x, draw_y;
        float draw_width, draw_height;
        MenuItem item = this.items[index];

        if (item.is_text) {
            item.vertex.GetDrawLocation(out draw_x, out draw_y);
            item.vertex.GetDrawSize(out draw_width, out draw_height);
        } else {
            item.vertex.GetDrawLocation(out draw_x, out draw_y);
            item.vertex.GetDrawSize(out draw_width, out draw_height);
        }

        return !this.drawcallback(pvrctx, this, index, draw_x, draw_y, draw_width, draw_height);
    }


    private class MenuItem {
        public string name;
        public AnimSprite anim_self;
        public AnimSprite anim_choosen;
        public AnimSprite anim_discarded;
        public AnimSprite anim_idle;
        public AnimSprite anim_rollback;
        public AnimSprite anim_selected;
        public AnimSprite anim_in;
        public AnimSprite anim_out;
        public bool is_text;
        public IVertex vertex;
        public float center_x;
        public float center_y;
        public float cell_dimmen;
        public float cell_gap;
        public float placement_x;
        public float placement_y;
        public bool rollback_active;
        public float render_distance;
        public bool has_scale;
        public bool hidden;
    }


}
