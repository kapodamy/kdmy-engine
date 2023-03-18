using System;
using System.Diagnostics;
using Engine.Animation;
using Engine.Font;
using Engine.Game.Common;
using Engine.Game.Gameplay.Helpers;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game.Helpers {

    public class WeekSelectorMdlSelect : IDraw, IAnimate {

        internal const string HEY = "choosen";
        internal const string IDLE = "idle";

        private const string UI_BF_LABEL = "ui_character_selector_player";
        private const string UI_GF_LABEL = "ui_character_selector_girlfriend";
        private const string UI_BF_PREVIEW = "ui_character_player";
        private const string UI_GF_PREVIEW = "ui_character_girlfriend";

        public const string MODELS_BF = "/assets/common/data/boyfriend_models.json";
        public const string MODELS_GF = "/assets/common/data/girlfriend_models.json";



        private AnimSprite animsprite;
        private int index;
        private CharacterInfo[] list;
        private int list_size;
        private StateSprite preview;
        private Sprite icon_locked;
        private Sprite icon_up;
        private Sprite icon_down;
        private bool has_up;
        private bool has_down;
        private bool is_locked;
        private bool is_enabled;
        private bool is_boyfriend;
        private bool hey_playing;
        private volatile int load_thread_id;
        private Drawable drawable;
        private Drawable drawable_character;
        private LayoutPlaceholder placeholder_character;
        private TextSprite label;
        private BeatWatcher beatwatcher;
        private TexturePool texturepool;


        public WeekSelectorMdlSelect(AnimList animlist, ModelHolder modelholder, Layout layout, TexturePool texturepool, bool is_boyfriend) {
            float draw_width, draw_height;

            string name;

            this.animsprite = AnimSprite.InitFromAnimlist(animlist, WeekSelector.ARROW_SPRITE_NAME);

            this.index = -1;
            this.list = null;
            this.list_size = 0;

            this.preview = StateSprite.InitFromTexture(null);

            this.icon_locked = modelholder.CreateSprite(WeekSelector.LOCKED, true);
            this.icon_up = modelholder.CreateSprite(WeekSelector.ARROW_SPRITE_NAME, true);
            this.icon_down = modelholder.CreateSprite(WeekSelector.ARROW_SPRITE_NAME, true);

            this.has_up = true;
            this.has_down = true;
            this.is_locked = false;
            this.is_enabled = false;
            this.is_boyfriend = is_boyfriend;
            this.hey_playing = false;

            this.load_thread_id = 0;

            this.drawable = null;
            this.drawable_character = null;
            this.placeholder_character = null;
            this.label = null;

            this.beatwatcher = new BeatWatcher() { };

            this.texturepool = texturepool;


            this.beatwatcher.Reset(true, 100f);

            this.drawable = new Drawable(
                -1f, this, this
            );
            this.drawable_character = new Drawable(
                -1f, this.preview, this.preview
            );

            this.icon_locked.SetVisible(false);
            this.icon_up.SetVisible(false);
            this.icon_down.SetVisible(false);

            name = is_boyfriend ? WeekSelectorMdlSelect.UI_BF_PREVIEW : WeekSelectorMdlSelect.UI_GF_PREVIEW;
            LayoutPlaceholder placeholder_character = layout.GetPlaceholder(name);
            Debug.Assert(placeholder_character != null, "missing :" + name);

            int index = 0;

            string models = is_boyfriend ? WeekSelectorMdlSelect.MODELS_BF : WeekSelectorMdlSelect.MODELS_GF;
            JSONParser models_array = JSONParser.LoadFrom(models);
            int models_array_size = JSONParser.ReadArrayLength(models_array);

            // count all character models
            for (int i = 0 ; i < models_array_size ; i++) {
                JSONToken json_obj = JSONParser.ReadArrayItemObject(models_array, i);
                string unlock_directive = JSONParser.ReadString(json_obj, "unlockDirectiveName", null);
                bool hide_if_locked = JSONParser.ReadBoolean(json_obj, "hideIfLocked", false);
                bool is_locked = !FunkinSave.ContainsUnlockDirective(unlock_directive);
                //free(unlock_directive);

                if (is_locked && hide_if_locked) continue;
                index++;
            }
            for (int i = 0 ; i < Funkin.weeks_array.size ; i++) {
                WeekInfo.UnlockableCharacter[] array; int array_size;

                if (is_boyfriend) {
                    array = Funkin.weeks_array.array[i].unlockables.boyfriend_models;
                    array_size = Funkin.weeks_array.array[i].unlockables.boyfriend_models_size;
                } else {
                    array = Funkin.weeks_array.array[i].unlockables.girlfriend_models;
                    array_size = Funkin.weeks_array.array[i].unlockables.girlfriend_models_size;
                }

                for (int j = 0 ; j < array_size ; j++) {
                    bool is_locked = !FunkinSave.ContainsUnlockDirective(array[j].unlock_directive);
                    if (is_locked && array[j].hide_if_locked) continue;
                    index++;
                }
            }

            // build list
            this.list_size = index;
            this.list = new CharacterInfo[this.list_size];
            index = 0;
            for (int i = 0 ; i < models_array_size ; i++) {
                JSONToken json_obj = JSONParser.ReadArrayItemObject(models_array, i);
                string unlock_directive = JSONParser.ReadString(json_obj, "unlockDirectiveName", null);
                bool hide_if_locked = JSONParser.ReadBoolean(json_obj, "hideIfLocked", false);
                bool is_locked = !FunkinSave.ContainsUnlockDirective(unlock_directive);

                if (is_locked && hide_if_locked) {
                    //free(unlock_directive);
                    continue;
                }

                // get path of the manifest file
                string manifest = JSONParser.ReadString(json_obj, "manifest", null);
                if (String.IsNullOrEmpty(manifest)) {
                    throw new Exception("WeekSelectorMdlSelect.init() invalid / missing 'manifest' in " + models);
                }
                string manifest_path = FS.BuildPath2(models, manifest);
                //free(manifest);

                this.list[index] = new CharacterInfo() {
                    name = JSONParser.ReadString(json_obj, "name", null),
                    is_locked = is_locked,
                    imported = true,
                    week_selector_left_facing = false,
                    week_selector_enable_beat = false,
                    week_selector_model = null,
                    week_selector_idle_anim_name = null,
                    week_selector_choosen_anim_name = null,
                    manifest = manifest_path
                };

                WeekSelectorMdlSelect.HelperImportFromManifest(
                    manifest_path, this.list[index]
                );
                index++;

            }
            JSONParser.Destroy(models_array);
            for (int i = 0 ; i < Funkin.weeks_array.size ; i++) {
                WeekInfo.UnlockableCharacter[] array; int array_size;

                if (is_boyfriend) {
                    array = Funkin.weeks_array.array[i].unlockables.boyfriend_models;
                    array_size = Funkin.weeks_array.array[i].unlockables.boyfriend_models_size;
                } else {
                    array = Funkin.weeks_array.array[i].unlockables.girlfriend_models;
                    array_size = Funkin.weeks_array.array[i].unlockables.girlfriend_models_size;
                }

                for (int j = 0 ; j < array_size ; j++) {
                    bool is_locked = !FunkinSave.ContainsUnlockDirective(array[j].unlock_directive);
                    if (is_locked && array[j].hide_if_locked) continue;

                    this.list[index] = new CharacterInfo() {
                        name = array[j].name,
                        is_locked = is_locked,
                        imported = false,
                        week_selector_left_facing = false,
                        week_selector_enable_beat = false,
                        week_selector_model = null,
                        week_selector_idle_anim_name = null,
                        week_selector_choosen_anim_name = null,
                        manifest = array[j].manifest
                    };

                    WeekSelectorMdlSelect.HelperImportFromManifest(
                       array[j].manifest, this.list[index]
                     );
                    index++;
                }
            }

            float arrows_height = (float)((double)layout.GetAttachedValue(
                 "ui_character_selector_arrowsHeight", AttachedValueType.FLOAT, 0.0
            ));
            float font_size = (float)((double)layout.GetAttachedValue(
                "ui_character_selector_fontSize", AttachedValueType.FLOAT, 18.0
            ));
            string font_name = (string)layout.GetAttachedValue(
                "ui_character_selector_fontName", AttachedValueType.STRING, null
            );
            uint font_color = (uint)layout.GetAttachedValue(
                "ui_character_selector_fontColor", AttachedValueType.HEX, 0x00FFFF
            );

            name = is_boyfriend ? WeekSelectorMdlSelect.UI_BF_LABEL : WeekSelectorMdlSelect.UI_GF_LABEL;
            LayoutPlaceholder placeholder = layout.GetPlaceholder(name);
            Debug.Assert(placeholder != null, "missing :" + name);

            // resize&rotate&locate arrows
            WeekSelectorMdlSelect.InternalPlaceArrow(this.icon_up, arrows_height, placeholder, false);
            WeekSelectorMdlSelect.InternalPlaceArrow(this.icon_down, arrows_height, placeholder, true);

            this.icon_locked.ResizeDrawSize(
                placeholder.width, placeholder.height - arrows_height * 1.5f, out draw_width, out draw_height
            );
            this.icon_locked.CenterDrawLocation(
                placeholder.x, placeholder.y,
                placeholder.width, placeholder.height,
                out _, out _
            );

            this.label = TextSprite.Init2(
                layout.GetAttachedFont(font_name), font_size, font_color
            );
            this.label.SetDrawLocation(placeholder.x, placeholder.y);
            this.label.SetMaxDrawSize(placeholder.width, placeholder.height);
            this.label.SetAlign(Align.CENTER, Align.CENTER);
            this.label.SetVisible(false);

            this.placeholder_character = placeholder_character;
            placeholder_character.vertex = this.drawable_character;
            placeholder.vertex = this.drawable;
            this.drawable_character.HelperUpdateFromPlaceholder(
                 placeholder_character
            );
            this.drawable.HelperUpdateFromPlaceholder(placeholder);

            this.drawable_character.SetVisible(false);
            this.preview.SetDrawLocation(placeholder_character.x, placeholder_character.y);
            this.preview.SetDrawSize(
                 placeholder_character.width, placeholder_character.height
            );
            this.preview.StateAdd2(null, null, null, 0XFFFFFF, null);

            Select(0);

        }

        public void Destroy() {
            this.animsprite.Destroy();

            this.drawable.Destroy();
            this.drawable_character.Destroy();

            this.preview.Destroy();
            this.label.Destroy();

            this.icon_locked.DestroyFull();
            this.icon_up.DestroyFull();
            this.icon_down.DestroyFull();

            //for (int i = 0 ; i < this.list_size ; i++) {
            //    if (this.list[i].imported) {
            //        free(this.list[i].name);
            //        free(this.list[i].manifest);
            //    }
            //    free(this.list[i].week_selector_model);
            //    free(this.list[i].week_selector_idle_anim_name);
            //    free(this.list[i].week_selector_choosen_anim_name);
            //}

            //free(this.list);
            //free(this);
        }


        public void Draw(PVRContext pvrctx) {
            pvrctx.Save();

            if (this.is_locked) this.icon_locked.Draw(pvrctx);

            if (this.is_enabled) {
                this.icon_up.Draw(pvrctx);
                this.icon_down.Draw(pvrctx);
            }

            if (this.is_locked)
                pvrctx.SetGlobalAlpha(WeekSelector.PREVIEW_DISABLED_ALPHA);

            this.label.Draw(pvrctx);

            pvrctx.Restore();
        }

        public int Animate(float elapsed) {
            if (this.animsprite != null) this.animsprite.Animate(elapsed);

            this.icon_up.Animate(elapsed);
            this.icon_down.Animate(elapsed);

            if (!this.hey_playing && this.beatwatcher.Poll()) {
                elapsed = this.beatwatcher.since;
                if (this.preview.AnimationCompleted()) {
                    this.preview.AnimationRestart();
                }
            }

            this.preview.Animate(elapsed);

            if (this.has_up)
                this.animsprite.UpdateSprite(this.icon_up, false);
            if (this.has_down)
                this.animsprite.UpdateSprite(this.icon_down, true);

            return 0;
        }


        public string GetManifest() {
            if (this.index < 0 || this.index >= this.list_size) return null;
            CharacterInfo mdl = this.list[this.index];
            return mdl.is_locked ? null : mdl.manifest;
        }

        public bool IsSelectedLocked() {
            if (this.index < 0 || this.index >= this.list_size) return true;
            return this.list[this.index].is_locked;
        }

        public void ToggleChoosen() {
            this.hey_playing = this.preview.StateToggle(WeekSelectorMdlSelect.HEY);
        }

        public void EnableArrows(bool enabled) {
            this.is_enabled = enabled;
        }

        public void SelectDefault() {
            if (this.index == 0) return;
            Select(0);
        }

        public bool Select(int new_index) {
            if (new_index < 0 || new_index >= this.list_size) return false;
            if (new_index == this.index) return true;

            this.is_locked = this.list[new_index].is_locked;
            this.index = new_index;
            this.has_up = new_index > 0;
            this.has_down = new_index < (this.list_size - 1);
            this.load_thread_id++;

            this.preview.StateToggle(null);
            this.preview.StateRemove(WeekSelectorMdlSelect.IDLE);
            this.preview.StateRemove(WeekSelectorMdlSelect.HEY);
            this.drawable_character.SetVisible(false);
            this.label.SetTextIntern(true, this.list[new_index].name);

            this.icon_up.SetAlpha(
                this.has_up ? 1.0f : WeekSelector.ARROW_DISABLED_ALPHA
            );
            this.icon_down.SetAlpha(
                 this.has_down ? 1.0f : WeekSelector.ARROW_DISABLED_ALPHA
            );

            GameMain.THDHelperSpawn(WeekSelectorMdlSelect.InternalLoadAsync, this);

            return true;
        }

        public bool Scroll(int offset) {
            return Select(this.index + offset);
        }

        public Drawable GetDrawable() {
            return this.drawable;
        }

        public void SetBeats(float bpm) {
            this.beatwatcher.ChangeBpm(bpm);
        }


        private static void InternalPlaceArrow(Sprite sprite, float arrow_size, LayoutPlaceholder placeholder, bool end) {
            float draw_width, draw_height;

            sprite.ResizeDrawSize(arrow_size, -1, out draw_width, out draw_height);

            // note: the sprite is rotated (corner rotation)
            float x = placeholder.x + ((placeholder.width - draw_height) / 2.0f);
            float y = placeholder.y;
            if (end) y += placeholder.height - draw_width;

            sprite.SetDrawLocation(x, y);
            sprite.MatrixCornerRotation(end ? Corner.TOPRIGHT : Corner.BOTTOMLEFT);
        }

        private static object InternalLoadAsync(object arg) {
            WeekSelectorMdlSelect mdlselect = arg as WeekSelectorMdlSelect;

            // remember the statesprite id to check if "weekselector_mdlselect" was disposed
            int id = mdlselect.preview.id;
            int load_thread_id = mdlselect.load_thread_id;
            CharacterInfo character_info = mdlselect.list[mdlselect.index];
            bool week_selector_left_facing = character_info.week_selector_left_facing;
            bool week_selector_enable_beat = character_info.week_selector_enable_beat;
            bool flip_x = false;

            if (mdlselect.is_boyfriend && !week_selector_left_facing) flip_x = true;
            //else if (!mdlselect.is_boyfriend && week_selector_left_facing) flip_x = true;

            ModelHolder modelholder = ModelHolder.Init(character_info.week_selector_model);

            if (!StateSprite.POOL.Has(id) || load_thread_id != mdlselect.load_thread_id) {
                modelholder.Destroy();
                return null;
            }

            if (modelholder == null) {
                Console.Error.WriteLine("[ERROR] weekselector_mdlselect_internal_load_async() modelholder_init failed");
                return null;
            }

            WeekSelectorMdlSelect.HelperImport(
             mdlselect.preview, modelholder,
             mdlselect.placeholder_character,
             week_selector_enable_beat,
             character_info.week_selector_idle_anim_name ?? WeekSelectorMdlSelect.IDLE,
             WeekSelectorMdlSelect.IDLE
         );
            WeekSelectorMdlSelect.HelperImport(
                 mdlselect.preview, modelholder,
                 mdlselect.placeholder_character,
                 false,
                 character_info.week_selector_choosen_anim_name ?? WeekSelectorMdlSelect.HEY,
                 WeekSelectorMdlSelect.HEY
             );
            modelholder.Destroy();

            mdlselect.texturepool.Add(modelholder.GetTexture(false));

            mdlselect.preview.StateToggle(WeekSelectorMdlSelect.IDLE);
            mdlselect.preview.Animate(mdlselect.beatwatcher.RemainingUntilNext());
            mdlselect.preview.FlipTexture(flip_x, false);
            mdlselect.drawable_character.SetVisible(true);

            return null;
        }

        public static void HelperImport(StateSprite statesprite, ModelHolder mdlhldr, LayoutPlaceholder placeholder, bool enable_beat, string anim_name, string name) {
            if (String.IsNullOrEmpty(anim_name)) return;
            StateSpriteState statesprite_state = statesprite.StateAdd(mdlhldr, anim_name, name);
            if (statesprite_state == null) return;

            ImgUtils.CalcRectangleInStateSpriteState(
                0.0f,
                0.0f,
                placeholder.width,
                placeholder.height,
                placeholder.align_horizontal,
                placeholder.align_vertical,
                statesprite_state
            );

            if (statesprite_state.animation == null) return;

            if (enable_beat || name == WeekSelectorMdlSelect.HEY) {
                statesprite_state.animation.SetLoop(1);
            }
        }

        private static void HelperImportFromManifest(string src, CharacterInfo list_item) {
            CharacterManifest charactermanifest = new CharacterManifest(src, false);
            list_item.week_selector_left_facing = charactermanifest.week_selector_left_facing;
            list_item.week_selector_enable_beat = charactermanifest.week_selector_enable_beat;
            list_item.week_selector_model = charactermanifest.week_selector_model;
            list_item.week_selector_idle_anim_name = charactermanifest.week_selector_idle_anim_name;
            list_item.week_selector_choosen_anim_name = charactermanifest.week_selector_choosen_anim_name;
            charactermanifest.Destroy();
        }


        private class CharacterInfo {
            public string name;
            public bool is_locked;
            public bool imported;
            public bool week_selector_left_facing;
            public bool week_selector_enable_beat;
            public string week_selector_model;
            public string week_selector_idle_anim_name;
            public string week_selector_choosen_anim_name;
            public string manifest;
        };

    }

}
