using System;
using Engine.Animation;
using Engine.Game.Common;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine {

    public class ModelHolder {

        private static Map<ModelHolder> POOL = new Map<ModelHolder>();
        private static int IDS = 0;

        internal static Atlas STUB_ATLAS = new Atlas() { size = 0, glyph_fps = 0, texture_filename = null };
        internal static AnimList STUB_ANIMLIST = new AnimList() { entries_count = 0 };

        private ModelHolder() { }


        private string instance_src;
        private int instance_references;
        private Atlas atlas;
        private AnimList animlist;
        private Texture texture;
        internal uint vertex_color_rgb8;
        private int id;


        public static ModelHolder Init(string src) {
            string full_path = FS.GetFullPathAndOverride(src);

            // find an instance of this 
            foreach (ModelHolder instance in ModelHolder.POOL) {
                if (instance.instance_src != full_path) continue;

                instance.instance_references++;
                //free(full_path);
                return instance;
            }

            FS.FolderStackPush();
            FS.SetWorkingFolder(src, true);

            // load the model manifest (if was specified)
            string manifest_texture = null;
            string manifest_atlas = null;
            string manifest_animlist = null;
            uint vertex_color = 0xFFFFFF;
            bool from_manifest = src.LowercaseEndsWithKDY(".json");

            if (from_manifest) {
                if (!FS.FileExists(src)) {
                    FS.FolderStackPop();
                    return null;
                }
                JSONParser json = JSONParser.LoadFrom(src);
                manifest_texture = JSONParser.ReadString(json, "texture", null);
                manifest_atlas = JSONParser.ReadString(json, "atlas", null);
                manifest_animlist = JSONParser.ReadString(json, "animlist", null);
                vertex_color = JSONParser.ReadHex(json, "vertexColor", vertex_color);
                JSONParser.Destroy(json);
            } else {
                string temp = src;
                bool from_atlas = Atlas.UtilsIsKnownExtension(src);

                if (from_atlas) temp = FS.GetFilenameWithoutExtension(src);
                manifest_texture = StringUtils.Concat(temp, ".png");
                manifest_atlas = from_atlas ? src : StringUtils.Concat(temp, ".xml");
                manifest_animlist = StringUtils.Concat(temp, "_anims.xml");
                //if (from_atlas) free(temp);
            }

            // JS only
            if (manifest_texture != null) manifest_texture = FS.BuildPath2(full_path, manifest_texture);
            if (manifest_atlas != null) manifest_atlas = FS.BuildPath2(full_path, manifest_atlas);
            if (manifest_animlist != null) manifest_animlist = FS.BuildPath2(full_path, manifest_animlist);

            ModelHolder modelholder = new ModelHolder() {
                atlas = ModelHolder.STUB_ATLAS,
                animlist = ModelHolder.STUB_ANIMLIST,
                texture = null,
                vertex_color_rgb8 = vertex_color,

                id = ModelHolder.IDS++,
                instance_references = 1,
                instance_src = full_path
            };

            if (!String.IsNullOrEmpty(manifest_atlas) && FS.FileExists(manifest_atlas)) {
                Atlas atlas = Atlas.Init(manifest_atlas);
                if (atlas != null) modelholder.atlas = atlas;
            }

            if (!String.IsNullOrEmpty(manifest_animlist) && FS.FileExists(manifest_animlist)) {
                AnimList animlist = AnimList.Init(manifest_animlist);
                if (animlist != null) modelholder.animlist = animlist;
            }

            if (!String.IsNullOrEmpty(manifest_texture) && FS.FileExists(manifest_texture)) {
                modelholder.texture = Texture.Init(manifest_texture);
            } else {
                string altas_texture = modelholder.atlas.GetTexturePath();
                if (!String.IsNullOrEmpty(altas_texture)) {
                    if ((FS.FileExists(altas_texture))) {
                        modelholder.texture = Texture.Init(altas_texture);
                    } else {
                        Console.Error.WriteLine(
                            "modelholder_init() missing texture '" +
                            altas_texture +
                            "' of atlas '" +
                            manifest_atlas +
                            "'"
                        );
                    }
                }
            }

            //free(manifest_atlas);
            //free(manifest_texture);
            //free(manifest_animlist);

            FS.FolderStackPop();

            bool no_atlas = modelholder.atlas == ModelHolder.STUB_ATLAS;
            bool no_animlist = modelholder.animlist == ModelHolder.STUB_ANIMLIST;
            if (!from_manifest && modelholder.texture == null && no_atlas && no_animlist) {
                Console.Error.WriteLine("modelholder_init() failed, missing resources of " + src);
                modelholder.Destroy();
                return null;
            }

            ModelHolder.POOL.Set(modelholder.id, modelholder);
            return modelholder;
        }

        public static ModelHolder Init2(uint vertex_color_rgb8, string atlas_src, string animlist_src) {
            ModelHolder modelholder = new ModelHolder() {
                atlas = ModelHolder.STUB_ATLAS,
                animlist = ModelHolder.STUB_ANIMLIST,
                texture = null,
                vertex_color_rgb8 = vertex_color_rgb8,

                id = ModelHolder.IDS++,
                instance_references = 1,
                instance_src = null
            };
            ModelHolder.POOL.Set(modelholder.id, modelholder);

            if (!String.IsNullOrEmpty(animlist_src) && FS.FileExists(animlist_src)) {
                modelholder.animlist = AnimList.Init(animlist_src);
                if (modelholder.animlist == null) modelholder.animlist = ModelHolder.STUB_ANIMLIST;
            }

            if (!String.IsNullOrEmpty(atlas_src) && FS.FileExists(atlas_src)) {
                modelholder.atlas = Atlas.Init(atlas_src);
                if (modelholder.atlas == null) modelholder.atlas = ModelHolder.STUB_ATLAS;

                if (modelholder.atlas != null) {
                    string atlas_texture = modelholder.atlas.GetTexturePath();
                    if (!String.IsNullOrEmpty(atlas_texture)) {
                        if (FS.FileExists(atlas_texture)) {
                            modelholder.texture = Texture.Init(atlas_texture);
                        } else {
                            Console.Error.Write("missing texture file: " + atlas_texture + " in " + atlas_src);
                        }
                    }
                }
            }

            return modelholder;
        }


        public void Destroy() {
            //if (this == null) return;

            // before continue, check if there another instances
            this.instance_references--;
            if (this.instance_references > 0) return;

            if (this.atlas != ModelHolder.STUB_ATLAS)
                this.atlas.Destroy();

            if (this.animlist != ModelHolder.STUB_ANIMLIST)
                this.animlist.Destroy();

            if (this.texture != null)
                this.texture.Destroy();

            ModelHolder.POOL.Delete(this.id);
            //free(this.instance_src);
            //free;
        }


        public bool IsInvalid() {
            //if (this == null) return true;
            return this.atlas == ModelHolder.STUB_ATLAS &&
                this.animlist == ModelHolder.STUB_ANIMLIST &&
                this.texture == null;
        }

        public bool HasAnimlist() {
            return this.animlist != ModelHolder.STUB_ANIMLIST;
        }

        public Atlas GetAtlas() {
            return this.atlas;
        }

        public Texture GetTexture(bool increment_reference) {
            if (this.texture == null)
                return null;
            else if (increment_reference)
                return this.texture.ShareReference();
            else
                return this.texture;
        }

        public uint GetVertexColor() {
            return this.vertex_color_rgb8;
        }

        public AnimList GetAnimlist() {
            return this.animlist;
        }

        public Sprite CreateSprite(string atlas_entry_name, bool with_animation) {
            Sprite sprite;
            if (this.texture != null) {
                sprite = Sprite.Init(this.texture.ShareReference());
                AtlasEntry atlas_entry = GetAtlasEntry2(atlas_entry_name, false);
                if (atlas_entry != null) Atlas.ApplyFromEntry(sprite, atlas_entry, true);
            } else {
                sprite = Sprite.InitFromRGB8(this.vertex_color_rgb8);
            }

            if (with_animation) {
                AnimSprite animsprite = CreateAnimsprite(atlas_entry_name, false, false);
                if (animsprite != null) sprite.AnimationAdd(animsprite);
            }

            return sprite;
        }

        public AnimSprite CreateAnimsprite(string animation_name, bool fallback_static, bool no_return_null) {
            if (this.animlist != ModelHolder.STUB_ANIMLIST) {
                AnimListItem animlist_item = this.animlist.GetAnimation(animation_name);
                if (animlist_item != null) return AnimSprite.Init(animlist_item);
            }

            bool has_atlas = this.atlas != ModelHolder.STUB_ATLAS;


            if (/*this.animlist == ModelHolder.STUB_ANIMLIST && */has_atlas) {
                /*if (
                    !fallback_static &&
                    !this.atlas.GetEntryWithNumberSuffix( animation_name)
                ) {
                    goto L_return;
                }*/

                // try build an animation using atlas frames (loops indefinitely at 24 FPS)
                AnimSprite animsprite = AnimSprite.InitFromAtlas(
                    Funkin.DEFAULT_ANIMATIONS_FRAMERATE, 0, this.atlas, animation_name, true
                );
                if (animsprite != null) return animsprite;

                if (fallback_static) {
                    // Try build a static animation (single atlas frame)
                    AtlasEntry atlas_entry = this.atlas.GetEntry(animation_name);
                    if (atlas_entry != null) {
                        return AnimSprite.InitFromAtlasEntry(
                            atlas_entry, false, Funkin.DEFAULT_ANIMATIONS_FRAMERATE
                        );
                    }
                }
            }

            //L_return:
            if (no_return_null)
                return AnimSprite.InitAsEmpty(animation_name);
            else
                return null;
        }

        public AtlasEntry GetAtlasEntry(string atlas_entry_name, bool return_copy) {
            if (this.atlas == ModelHolder.STUB_ATLAS) return null;

            if (return_copy)
                return this.atlas.GetEntryCopy(atlas_entry_name);
            else
                return this.atlas.GetEntry(atlas_entry_name);
        }

        /**
         * Retrieves an atlas entry with the specified name. If the entry is not found, retrieves the first
         * entry with number suffix (example "dance left000"), 
         * @param {Object} modelholder The modelholder instance
         * @param {string} atlas_entry_name Entry name and/or entry name prefix
         * @param {bool} return_copy 1 to clone entry (must be free() later), otherwise, 0 to obtain an reference
         * @returns {object} the atlas entry or NULL if not found
         */
        public AtlasEntry GetAtlasEntry2(string atlas_entry_name, bool return_copy) {
            if (this.atlas == ModelHolder.STUB_ATLAS) return null;

            AtlasEntry atlas_entry = this.atlas.GetEntry(atlas_entry_name);

            if (atlas_entry == null)
                atlas_entry = this.atlas.GetEntryWithNumberSuffix(atlas_entry_name);

            if (atlas_entry != null && return_copy) atlas_entry = atlas_entry.Clone();

            return atlas_entry;
        }

        public bool GetTextureResolution(out int resolution_width, out int resolution_height) {
            if (this.atlas == ModelHolder.STUB_ATLAS) {
                resolution_width = Funkin.SCREEN_RESOLUTION_WIDTH;
                resolution_height = Funkin.SCREEN_RESOLUTION_HEIGHT;
                return false;
            }
            this.atlas.GetTextureResolution(out resolution_width, out resolution_height); return true;
        }

    }

}
