using Engine.Animation;
using Engine.Game.Common;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game.Helpers {
    public class FreeplaySongIcons {

        private SongIcon[] icons;
        private int icons_size;

        public FreeplaySongIcons(ArrayList<FreeplayMenu.MappedSong> song_map, float max_dimmen, float font_size) {
            this.icons_size = song_map.Size();
            this.icons = new SongIcon[this.icons_size];


            string last_model = null;
            bool last_model_exists = false;
            float base_offset_y = (font_size - max_dimmen) / 2f;

            for (int i = 0 ; i < this.icons_size ; i++) {
                FreeplayMenu.MappedSong track = song_map.Get(i);
                WeekInfo weekinfo = Funkin.weeks_array.array[track.week_index];

                string icon_name = weekinfo.songs[track.song_index].freeplay_host_icon_name;
                string model_src = weekinfo.songs[track.song_index].freeplay_host_icon_model;

                if (icon_name == null) icon_name = HealthBar.ICON_PREFIX_NEUTRAL;
                if (model_src == null) model_src = weekinfo.songs_default_freeplay_host_icon_model;

                if (model_src != last_model) {
                    last_model = model_src;
                    last_model_exists = FS.FileExists(model_src);
                }
                if (!last_model_exists) model_src = null;

                Sprite sprite = null;
                float offset_x = 0;
                float offset_y = 0;

                if (ModelHolder.UtilsIsKnownExtension(model_src)) {
                    ModelHolder modelholder = ModelHolder.Init(model_src);
                    if (modelholder == null) goto L_continue;

                    Texture texture = modelholder.GetTexture(true);
                    AnimSprite anim = modelholder.CreateAnimsprite(icon_name, false, false);
                    uint color_rgb8 = modelholder.GetVertexColor();

                    if (texture != null)
                        sprite = Sprite.Init(texture);
                    else
                        sprite = Sprite.InitFromRGB8(color_rgb8);

                    if (anim != null) {
                        sprite.ExternalAnimationSet(anim);
                    } else {
                        AtlasEntry atlas_entry = modelholder.GetAtlasEntry(icon_name, false);
                        if (atlas_entry != null) Atlas.ApplyFromEntry(sprite, atlas_entry, true);
                    }

                    modelholder.Destroy();
                } else if (model_src != null) {
                    Texture texture = Texture.Init(model_src);
                    sprite = Sprite.Init(texture);
                }
L_continue:
                if (sprite != null) {
                    float draw_width, draw_height;
                    sprite.ResizeDrawSize(max_dimmen, max_dimmen, out draw_width, out draw_height);

                    offset_x = (max_dimmen - draw_width) / 2f;
                    offset_y = (max_dimmen - draw_height) / 2f;
                    offset_y += base_offset_y;
                }

                this.icons[i] = new SongIcon() {
                    sprite = sprite,
                    offset_x = offset_x,
                    offset_y = offset_y
                };
            }
        }

        public void Destroy() {
            for (int i = 0 ; i < this.icons_size ; i++) {
                if (this.icons[i].sprite != null) this.icons[i].sprite.DestroyFull();
            }
            //free(this.icons);
            //free(this);
        }

        public bool DrawItemIcon(PVRContext pvrctx, Menu menu, int idx, float x, float y, float w, float h) {
            SongIcon icon = this.icons[idx];
            if (icon.sprite == null) return true;

            icon.sprite.SetDrawLocation(x + w, y + icon.offset_y);
            icon.sprite.Draw(pvrctx);

            return true;
        }


        private struct SongIcon {
            public Sprite sprite;
            public float offset_x;
            public float offset_y;
        }

    }

}
