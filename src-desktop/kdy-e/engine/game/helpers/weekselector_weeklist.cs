using System;
using Engine.Animation;
using Engine.Game.Common;
using Engine.Game.Gameplay.Helpers;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game.Helpers {

    public class WeekSelectorWeekList : IDraw, IAnimate {

        private const int VISIBLE_SIZE = 4;
        private const float TWEEN_DURATION = 120;


        private Item[] list_visible;
        private int index;
        private float x;
        private float y;
        private float width;
        private float row_height;
        private double progress;
        private bool do_reverse;
        private Drawable drawable_host;
        private Drawable drawable_list;
        private StateSprite host_statesprite;
        private volatile bool host_loading;
        private LayoutPlaceholder host_placeholder;
        private volatile int host_load_id;
        private TexturePool texturepool;
        private AnimSprite anim_selected;
        private bool week_choosen;
        private bool hey_playing;
        private BeatWatcher beatwatcher;


        public WeekSelectorWeekList(AnimList animlist, ModelHolder modelholder, Layout layout, TexturePool texturepool) {
            LayoutPlaceholder placeholder = layout.GetPlaceholder("ui_weeklist");
            if (placeholder == null) throw new Exception("missing placeholder 'ui_weeklist'");

            LayoutPlaceholder placeholder_host = layout.GetPlaceholder("ui_character_opponent");
            if (placeholder_host == null) throw new Exception("missing placeholder 'ui_weeklist'");

            AnimSprite anim_selected = AnimSprite.InitFromAnimlist(animlist, "week_title_selected");


            this.list_visible = new Item[WeekSelectorWeekList.VISIBLE_SIZE];

            this.index = 0;

            this.x = placeholder.x;
            this.y = placeholder.y;
            this.width = placeholder.width;
            this.row_height = placeholder.height / 4.0f;

            this.progress = WeekSelectorWeekList.TWEEN_DURATION;
            this.do_reverse = false;

            this.drawable_host = null;
            this.drawable_list = null;

            this.host_statesprite = StateSprite.InitFromTexture(null);
            this.host_loading = false;
            this.host_placeholder = placeholder_host;
            this.host_load_id = 0;

            this.texturepool = texturepool;
            this.anim_selected = anim_selected;
            this.week_choosen = false;
            this.hey_playing = false;

            this.beatwatcher = new BeatWatcher() { };

            this.beatwatcher.Reset(true, 100f);

            for (int i = 0 ; i < WeekSelectorWeekList.VISIBLE_SIZE ; i++) {
                this.list_visible[i] = new Item() {
                    icon_locked = modelholder.CreateSprite(WeekSelector.LOCKED, true),
                    sprite_title = Sprite.Init(null),
                    tweenlerp_locked = WeekSelectorWeekList.InternalCreateTween(),
                    tweenlerp_title = WeekSelectorWeekList.InternalCreateTween(),
                    is_locked = true
                };
                this.list_visible[i].icon_locked.MatrixScaleSize(true);
                this.list_visible[i].sprite_title.MatrixScaleSize(true);
            }

            InternalPrepareLockedTweens();

            this.drawable_list = new Drawable(
                placeholder.z, this, this
            );
            placeholder.vertex = this.drawable_list;

            this.drawable_host = new Drawable(
                placeholder_host.z, InternalHostDraw, null
            );
            placeholder_host.vertex = this.drawable_host;
            this.host_statesprite.SetDrawLocation(placeholder_host.x, placeholder_host.y);

            string last_played_week = FunkinSave.GetLastPlayedWeek();
            int index = 0;
            for (int i = 0 ; i < Funkin.weeks_array.size ; i++) {
                if (Funkin.weeks_array.array[i].name == last_played_week) {
                    index = i;
                    break;
                }
            }

            Select(index);

            // obligatory step
            this.progress = WeekSelectorWeekList.TWEEN_DURATION;

            float percent = index > 0 ? 0f : 1f;
            for (int i = 0 ; i < WeekSelectorWeekList.VISIBLE_SIZE ; i++) {
                Item visible_item = this.list_visible[i];
                visible_item.tweenlerp_locked.AnimatePercent(percent);
                visible_item.tweenlerp_title.AnimatePercent(percent);

                visible_item.tweenlerp_locked.VertexSetProperties(visible_item.icon_locked);
                if (visible_item.sprite_title.IsTextured()) {
                    visible_item.tweenlerp_title.VertexSetProperties(visible_item.sprite_title);
                }
            }

        }

        public void Destroy() {
            for (int i = 0 ; i < WeekSelectorWeekList.VISIBLE_SIZE ; i++) {
                this.list_visible[i].icon_locked.DestroyFull();
                this.list_visible[i].sprite_title.DestroyFull();
                this.list_visible[i].tweenlerp_title.Destroy();
                this.list_visible[i].tweenlerp_locked.Destroy();
            }
            //free(this.list_visible);
            this.host_statesprite.Destroy();
            if (this.anim_selected != null) this.anim_selected.Destroy();
            this.drawable_host.Destroy();
            this.drawable_list.Destroy();
            //free(this);
        }


        public int Animate(float elapsed) {
            float since_beat = elapsed;

            if (!this.hey_playing && this.beatwatcher.Poll()) {
                since_beat = this.beatwatcher.since;
                if (this.host_statesprite.AnimationCompleted()) {
                    this.host_statesprite.AnimationRestart();
                }
            }

            if (!this.host_loading) this.host_statesprite.Animate(since_beat);

            for (int i = 0 ; i < WeekSelectorWeekList.VISIBLE_SIZE ; i++) {
                Item visible_item = this.list_visible[i];

                visible_item.icon_locked.Animate(elapsed);

                if (visible_item.sprite_title.IsTextured())
                    visible_item.sprite_title.Animate(elapsed);
            }

            if (this.week_choosen && this.anim_selected != null) {
                Item visible_item = this.list_visible[this.do_reverse ? 2 : 1];
                this.anim_selected.UpdateSprite(visible_item.sprite_title, true);
                this.anim_selected.Animate(elapsed);
            }

            if (this.progress >= WeekSelectorWeekList.TWEEN_DURATION) return 1;

            double percent = Math.Min(1.0, this.progress / WeekSelectorWeekList.TWEEN_DURATION);
            this.progress += elapsed;
            if (this.do_reverse) percent = 1.0 - percent;

            for (int i = 0 ; i < WeekSelectorWeekList.VISIBLE_SIZE ; i++) {
                Item visible_item = this.list_visible[i];

                visible_item.tweenlerp_locked.AnimatePercent(percent);
                visible_item.tweenlerp_title.AnimatePercent(percent);

                visible_item.tweenlerp_locked.VertexSetProperties(
                    visible_item.icon_locked
                );
                if (visible_item.sprite_title.IsTextured()) {
                    visible_item.tweenlerp_title.VertexSetProperties(
                        visible_item.sprite_title
                    );
                }
            }

            return 0;
        }

        public void Draw(PVRContext pvrctx) {
            pvrctx.Save();

            pvrctx.CurrentMatrix.Translate(this.x, this.y);
            //pvrctx.Flush();

            if (this.week_choosen) {
                // draw only the choosen week
                Item visible_item = this.list_visible[this.do_reverse ? 2 : 1];
                if (visible_item.sprite_title.IsTextured()) {
                    visible_item.sprite_title.Draw(pvrctx);
                }
            } else {
                for (int i = 0 ; i < WeekSelectorWeekList.VISIBLE_SIZE ; i++) {
                    Item visible_item = this.list_visible[i];
                    if (!visible_item.sprite_title.IsTextured()) continue;

                    visible_item.sprite_title.Draw(pvrctx);
                    if (visible_item.is_locked) visible_item.icon_locked.Draw(pvrctx);
                }
            }

            pvrctx.Restore();
        }


        public void ToggleChoosen() {
            this.week_choosen = true;
            if (this.host_loading) return;
            if (this.host_statesprite.StateToggle(WeekSelectorMdlSelect.HEY)) {
                this.hey_playing = true;
            }
        }

        public WeekInfo GetSelected() {
            if (this.index < 0 || this.index >= Funkin.weeks_array.size) return null;
            return Funkin.weeks_array.array[this.index];
        }

        public Sprite PeekTitleSprite() {
            return this.list_visible[this.do_reverse ? 2 : 1].sprite_title;
        }

        public bool Scroll(int offset) {
            return Select(this.index + offset);
        }

        public bool Select(int index) {
            if (index < 0 || index >= Funkin.weeks_array.size) return false;

            this.do_reverse = this.index < index;
            this.index = index;

            // center the selected week in the visible list
            index -= this.do_reverse ? 2 : 1;

            for (int i = 0 ; i < WeekSelectorWeekList.VISIBLE_SIZE ; i++, index++) {
                if (index < 0 || index >= Funkin.weeks_array.size) {
                    this.list_visible[i].is_locked = true;
                    this.list_visible[i].sprite_title.DestroyTexture();
                    continue;
                }

                WeekInfo weekinfo = Funkin.weeks_array.array[index];
                this.list_visible[i].is_locked = !FunkinSave.ContainsUnlockDirective(
                    weekinfo.unlock_directive
                );

                string title_src = WeekEnumerator.GetTitleTexture(weekinfo);
                Texture texture = Texture.Init(title_src);
                //free(title_src);

                // add title texture to the texturepool and dispose the previous one
                this.texturepool.Add(texture);
                this.list_visible[i].sprite_title.DestroyTexture();
                this.list_visible[i].sprite_title.SetTexture(texture, true);
                this.list_visible[i].sprite_title.SetDrawSizeFromSourceSize();
            }

            this.host_load_id++;
            this.host_loading = true;
            GameMain.THDHelperSpawn(InternalLoadHostAsync, this);

            this.progress = 0.0;
            InternalPrepareTitleTweens();

            return true;
        }

        public int GetSelectedIndex() {
            return this.index;
        }

        public void SetBeats(float beats_per_minute) {
            this.beatwatcher.ChangeBpm(beats_per_minute);
        }



        private static TweenLerp InternalCreateTween() {
            const float duration = WeekSelectorWeekList.TWEEN_DURATION;
            TweenLerp tweenlerp = TweenLerp.Init();
            tweenlerp.AddLinear(VertexProps.SPRITE_PROP_SCALE_X, Single.NaN, Single.NaN, duration);
            tweenlerp.AddLinear(VertexProps.SPRITE_PROP_SCALE_Y, Single.NaN, Single.NaN, duration);
            tweenlerp.AddLinear(VertexProps.SPRITE_PROP_TRANSLATE_X, Single.NaN, Single.NaN, duration);
            tweenlerp.AddLinear(VertexProps.SPRITE_PROP_TRANSLATE_Y, Single.NaN, Single.NaN, duration);
            tweenlerp.AddLinear(VertexProps.SPRITE_PROP_ALPHA, Single.NaN, Single.NaN, duration);
            return tweenlerp;
        }

        private void InternalPrepareLockedTweens() {
            float[] row1 = { 0.0f, 0.0f, 0.0f, 0.0f };
            float[] row2 = { 0.0f, 0.0f, 0.0f, 0.0f };

            TweenLerp tweenlerp;
            float row_height = this.row_height;
            float x1, x2, y1, y2;

            float height3 = row_height * 3.0f;
            float height4 = row_height * 4.0f;

            // all locked icons have the same draw size
            InternalCalcRowSizes(
                this.list_visible[0].icon_locked, row1, row2
            );

            // void1 -> row1
            x1 = this.width / 2.0f;
            x2 = (this.width - row1[2]) / 2.0f;
            y1 = -row_height;
            y2 = (row_height - row1[3]) / 2.0f;
            tweenlerp = this.list_visible[0].tweenlerp_locked;
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_X, 0.0f, row1[0]);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_Y, 0.0f, row1[1]);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_X, x1, x2);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_Y, y1, y2);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_ALPHA, 0.0f, 0.7f);

            // row1 -> row2
            x1 = (this.width - row1[2]) / 2.0f;
            x2 = (this.width - row2[2]) / 2.0f;
            y1 = (row_height - row1[3]) / 2.0f;
            y2 = ((this.row_height * 2.0f) - row2[3]) / 2.0f;
            tweenlerp = this.list_visible[1].tweenlerp_locked;
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_X, row1[0], row2[0]);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_Y, row1[1], row2[1]);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_X, x1, x2);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_Y, y1, y2 + row_height);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_ALPHA, 0.7f, 1.0f);

            // row2 -> row3
            x1 = (this.width - row2[2]) / 2.0f;
            x2 = (this.width - row1[2]) / 2.0f;
            y1 = ((this.row_height * 2.0f) - row2[3]) / 2.0f;
            y2 = (row_height - row1[3]) / 2.0f;
            tweenlerp = this.list_visible[2].tweenlerp_locked;
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_X, row2[0], row1[0]);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_Y, row2[1], row1[1]);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_X, x1, x2);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_Y, y1 + row_height, y2 + height3);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_ALPHA, 1.0f, 0.7f);

            // row3 -> void2
            x1 = (this.width - row1[2]) / 2.0f;
            x2 = this.width / 2.0f;
            y1 = (row_height - row1[3]) / 2.0f;
            y2 = row_height;
            tweenlerp = this.list_visible[3].tweenlerp_locked;
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_X, row1[0], 0.0f);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_Y, row1[1], 0.0f);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_X, x1, x2);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_Y, y1 + height3, y2 + height4);
            tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_ALPHA, 0.7f, 0.0f);
        }

        private void InternalPrepareTitleTweens() {
            float[] row1 = { 0.0f, 0.0f, 0.0f, 0.0f };
            float[] row2 = { 0.0f, 0.0f, 0.0f, 0.0f };

            TweenLerp tweenlerp;
            float height1 = this.row_height * 1.0f;
            float height3 = this.row_height * 3.0f;
            float height4 = this.row_height * 4.0f;

            // void1 -> row1
            if (this.list_visible[0].sprite_title != null) {
                tweenlerp = this.list_visible[0].tweenlerp_title;
                InternalCalcRowSizes(
            this.list_visible[0].sprite_title, row1, row2
        );
                float x1 = this.width / 2.0f;
                float x2 = (this.width - row1[2]) / 2.0f;
                float y1 = -height1;
                float y2 = (height1 - row1[3]) / 2.0f;
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_X, 0.0f, row1[0]);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_Y, 0, row1[1]);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_X, x1, x2);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_Y, y1, y2);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_ALPHA, 0.0f, 0.3f);
            }

            // row1 -> row2
            if (this.list_visible[1].sprite_title != null) {
                tweenlerp = this.list_visible[1].tweenlerp_title;
                InternalCalcRowSizes(
            this.list_visible[1].sprite_title, row1, row2
        );
                float x1 = (this.width - row1[2]) / 2.0f;
                float x2 = (this.width - row2[2]) / 2.0f;
                float y1 = (height1 - row1[3]) / 2.0f;
                float y2 = ((this.row_height * 2.0f) - row2[3]) / 2.0f;
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_X, row1[0], row2[0]);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_Y, row1[1], row2[1]);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_X, x1, x2);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_Y, y1, y2 + height1);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_ALPHA, 0.3f, 1.0f);
            }

            // row2 -> row3
            if (this.list_visible[2].sprite_title != null) {
                tweenlerp = this.list_visible[2].tweenlerp_title;
                InternalCalcRowSizes(
            this.list_visible[2].sprite_title, row1, row2
        );
                float x1 = (this.width - row2[2]) / 2.0f;
                float x2 = (this.width - row1[2]) / 2.0f;
                float y1 = ((this.row_height * 2.0f) - row2[3]) / 2.0f;
                float y2 = (height1 - row1[3]) / 2.0f;
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_X, row2[0], row1[0]);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_Y, row2[1], row1[1]);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_X, x1, x2);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_Y, y1 + height1, y2 + height3);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_ALPHA, 1.0f, 0.3f);
            }

            // row3 -> void2
            if (this.list_visible[3].sprite_title != null) {
                tweenlerp = this.list_visible[3].tweenlerp_title;
                InternalCalcRowSizes(
            this.list_visible[3].sprite_title, row1, row2
        );
                float x1 = (this.width - row1[2]) / 2.0f;
                float x2 = this.width / 2.0f;
                float y1 = (height1 - row1[3]) / 2.0f;
                float y2 = height4 + height1;
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_X, row1[0], 0.0f);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_SCALE_Y, row1[1], 0.0f);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_X, x1, x2);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_TRANSLATE_Y, y1 + height3, y2);
                tweenlerp.ChangeBoundsById(VertexProps.SPRITE_PROP_ALPHA, 0.3f, 0.0f);
            }

        }

        private void InternalCalcRowSizes(Sprite vertex, float[] row1, float[] row2) {
            float draw_width = 0.0f, draw_height = 0.0f;

            float height1 = this.row_height * 1.0f;
            float height2 = this.row_height * 2.0f;
            float width = this.width;

            vertex.GetDrawSize(out draw_width, out draw_height);

            ImgUtils.CalcSize(draw_width, draw_height, width, height1, out row1[0], out row1[1]);
            ImgUtils.CalcSize(draw_width, draw_height, width, height2, out row2[0], out row2[1]);

            row1[0] /= draw_width;// scale x
            row1[1] /= draw_height;// scale y
            row1[2] = draw_width * row1[0];// width 1
            row1[3] = draw_height * row1[1];// height 1

            row2[0] /= draw_width;// scale x
            row2[1] /= draw_height;// scale y
            row2[2] = draw_width * row2[0];// width 2
            row2[3] = draw_height * row2[1];// height 2
        }

        private void InternalHostDraw(PVRContext pvrctx) {
            if (this.host_loading) return;
            pvrctx.Save();
            this.host_statesprite.Draw(pvrctx);
            pvrctx.Restore();
        }

        private static object InternalLoadHostAsync(object arg) {
            WeekSelectorWeekList weeklist = arg as WeekSelectorWeekList;

            int host_load_id = weeklist.host_load_id;
            int host_statesprite_id = weeklist.host_statesprite.id;
            WeekInfo weekinfo = Funkin.weeks_array.array[weeklist.index];
            bool host_flip, host_beat; ModelHolder modelholder;

            if (!String.IsNullOrEmpty(weekinfo.week_host_manifest)) {
                CharacterManifest charactermanifest = new CharacterManifest(weekinfo.week_host_manifest, false);
                host_flip = charactermanifest.left_facing;// face to the right
                host_beat = charactermanifest.week_selector_enable_beat;

                modelholder = ModelHolder.Init(charactermanifest.model_week_selector);
                charactermanifest.Destroy();
            } else {
                host_flip = weekinfo.host_flip_sprite;
                host_beat = weekinfo.host_enable_beat;

                string week_host_model = WeekEnumerator.GetHostModel(weekinfo);
                modelholder = ModelHolder.Init(week_host_model);
                //free(week_host_model);
            }

            if (!StateSprite.POOL.Has(host_statesprite_id)) {
                // weeklist was disposed
                if (modelholder != null) modelholder.Destroy();
                return null;
            }

            if (modelholder == null) {
                Console.Error.WriteLine("[ERROR] weekselector_weeklist_internal_load_host_async() modelholder_init failed");
                if (host_load_id == weeklist.host_load_id) {
                    weeklist.host_statesprite.StateRemove(WeekSelectorMdlSelect.HEY);
                    weeklist.host_statesprite.StateRemove(WeekSelectorMdlSelect.IDLE);
                }
                weeklist.host_statesprite.SetTexture(null, false);
                weeklist.host_loading = false;
                return null;
            }

            Texture texture = modelholder.GetTexture(false);
            if (texture != null) weeklist.texturepool.Add(texture);

            if (host_load_id == weeklist.host_load_id) {
                weeklist.host_statesprite.StateRemove(WeekSelectorMdlSelect.HEY);
                weeklist.host_statesprite.StateRemove(WeekSelectorMdlSelect.IDLE);

                WeekSelectorMdlSelect.HelperImport(
                    weeklist.host_statesprite,
                    modelholder,
                    weeklist.host_placeholder,
                    host_beat,
                    WeekSelectorMdlSelect.IDLE
                );
                WeekSelectorMdlSelect.HelperImport(
                    weeklist.host_statesprite,
                    modelholder,
                    weeklist.host_placeholder,
                    false,
                    WeekSelectorMdlSelect.HEY
                );

                if (weeklist.week_choosen)
                    weeklist.host_statesprite.StateToggle(WeekSelectorMdlSelect.HEY);
                else
                    weeklist.host_statesprite.StateToggle(WeekSelectorMdlSelect.IDLE);

                weeklist.host_statesprite.FlipTexture(host_flip, false);
                weeklist.host_statesprite.StateApply(null);
                weeklist.host_statesprite.Animate(weeklist.beatwatcher.RemainingUntilNext());
                weeklist.host_loading = false;
            }

            modelholder.Destroy();

            return null;
        }


        private class Item {
            public Sprite icon_locked;
            public Sprite sprite_title;
            public TweenLerp tweenlerp_locked;
            public TweenLerp tweenlerp_title;
            public bool is_locked;
        }

    }

}