using System;
using System.Text;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Game.Common;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

namespace Engine;

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
            if (!StringUtils.EqualsIgnoreCase(instance.instance_src, full_path)) continue;

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
        string fallback_texture_path = null;
        uint vertex_color = 0xFFFFFF;
        bool from_manifest = full_path.LowercaseEndsWithKDY(".json");

        if (from_manifest) {
            if (!FS.FileExists(full_path)) {
                FS.FolderStackPop();
                return null;
            }
            JSONToken json = JSONParser.LoadFrom(full_path);
            manifest_texture = JSONParser.ReadString(json, "texture", null);
            manifest_atlas = JSONParser.ReadString(json, "atlas", null);
            manifest_animlist = JSONParser.ReadString(json, "animlist", null);
            vertex_color = JSONParser.ReadHex(json, "vertexColor", vertex_color);
            JSONParser.Destroy(json);
        } else {
            string temp = full_path;
            bool from_atlas = Atlas.UtilsIsKnownExtension(full_path);

            if (from_atlas) temp = FS.GetFilenameWithoutExtension(full_path);
            string atlas_filename = from_atlas ? full_path : StringUtils.Concat(temp, ".xml");
            string animlist_filename = StringUtils.Concat(temp, "_anims.xml");
            string fallback_texture_filename = StringUtils.Concat(temp, ".png");
            //if (from_atlas) free(temp);

            manifest_atlas = FS.BuildPath2(full_path, atlas_filename);
            manifest_animlist = FS.BuildPath2(full_path, animlist_filename);
            fallback_texture_path = FS.BuildPath2(full_path, fallback_texture_filename);

            //free(atlas_filename);
            //free(animlist_filename);
            //free(fallback_texture_filename);
        }

        ModelHolder modelholder = new ModelHolder() {
            atlas = ModelHolder.STUB_ATLAS,
            animlist = ModelHolder.STUB_ANIMLIST,
            texture = null,
            vertex_color_rgb8 = vertex_color,

            id = ModelHolder.IDS++,
            instance_references = 1,
            instance_src = full_path
        };

        if (StringUtils.IsNotEmpty(manifest_atlas) && FS.FileExists(manifest_atlas)) {
            Atlas atlas = Atlas.Init(manifest_atlas);
            if (atlas != null) modelholder.atlas = atlas;
        }

        if (StringUtils.IsNotEmpty(manifest_animlist) && FS.FileExists(manifest_animlist)) {
            AnimList animlist = AnimList.Init(manifest_animlist);
            if (animlist != null) modelholder.animlist = animlist;
        }


        if (modelholder.atlas != null && modelholder.atlas != ModelHolder.STUB_ATLAS && StringUtils.IsEmpty(manifest_texture)) {
            string altas_texture = modelholder.atlas.GetTexturePath();
            if (StringUtils.IsNotEmpty(altas_texture) && FS.FileExists(altas_texture)) {
                modelholder.texture = Texture.Init(altas_texture);
                if (modelholder.texture == null) {
                    Logger.Error($"modelholder_init() atlas texture not found: atlas={manifest_atlas} texture={altas_texture}");
                }
            }
        }

        if (modelholder.texture == null) {
            string texture_src = manifest_texture ?? fallback_texture_path;
            if (FS.FileExists(texture_src)) {
                modelholder.texture = Texture.Init(texture_src);
            } else {
                Logger.Error($"modelholder_init() missing manifest texture: {manifest_texture}");
                Logger.Error($"modelholder_init() missing manifest texture: {texture_src}");
            }
        }

        //free(fallback_texture_path);
        //free(manifest_atlas);
        //free(manifest_texture);
        //free(manifest_animlist);

        FS.FolderStackPop();

        bool no_atlas = modelholder.atlas == ModelHolder.STUB_ATLAS;
        bool no_animlist = modelholder.animlist == ModelHolder.STUB_ANIMLIST;
        if (!from_manifest && modelholder.texture == null && no_atlas && no_animlist) {
            Logger.Error($"modelholder_init() failed, missing resources of {src}");
            modelholder.Destroy();
            return null;
        }

        ModelHolder.POOL.Set(modelholder.id, modelholder);
        return modelholder;
    }

    public static ModelHolder Init2(uint vertex_color_rgb8, string atlas_src, string animlist_src) {
        string full_atlas_src = FS.GetFullPathAndOverride(atlas_src);
        string full_animlist_src = FS.GetFullPathAndOverride(animlist_src);

        StringBuilder stringbuilder = new StringBuilder(128);
        stringbuilder.AddFormatKDY("$s|$s|0x$I", full_atlas_src, full_animlist_src, vertex_color_rgb8);
        string fake_src = stringbuilder.ToString();

        //free(full_atlas_src);
        //free(full_animlist_src);

        // find an instance of this
        foreach (ModelHolder instance in ModelHolder.POOL) {
            if (!StringUtils.EqualsIgnoreCase(instance.instance_src, fake_src)) continue;

            instance.instance_references++;
            //free(fake_src);
            return instance;
        }

        ModelHolder modelholder = new ModelHolder() {
            atlas = ModelHolder.STUB_ATLAS,
            animlist = ModelHolder.STUB_ANIMLIST,
            texture = null,
            vertex_color_rgb8 = vertex_color_rgb8,

            id = ModelHolder.IDS++,
            instance_references = 1,
            instance_src = fake_src
        };
        ModelHolder.POOL.Set(modelholder.id, modelholder);

        if (StringUtils.IsNotEmpty(animlist_src) && FS.FileExists(animlist_src)) {
            modelholder.animlist = AnimList.Init(animlist_src);
            if (modelholder.animlist == null) modelholder.animlist = ModelHolder.STUB_ANIMLIST;
        }

        string atlas_texture = null;

        if (StringUtils.IsNotEmpty(atlas_src) && FS.FileExists(atlas_src)) {
            modelholder.atlas = Atlas.Init(atlas_src);

            if (modelholder.atlas != null)
                atlas_texture = modelholder.atlas.GetTexturePath();
            else
                modelholder.atlas = ModelHolder.STUB_ATLAS;
        }

        if (StringUtils.IsNotEmpty(atlas_texture) && FS.FileExists(atlas_texture)) {
            modelholder.texture = Texture.Init(atlas_texture);
        } else if (StringUtils.IsNotEmpty(atlas_src)) {
            // try use atlas name instead
            FS.FolderStackPush();
            FS.SetWorkingFolder(atlas_src, true);

            string temp = FS.GetFilenameWithoutExtension(atlas_src);
            string texture_path = temp + ".png";

            if (FS.FileExists(texture_path)) {
                modelholder.texture = Texture.Init(texture_path);
            }

            if (modelholder.texture != null) {
                Logger.Warn($"modelholder_init2() expected {atlas_texture}, found {texture_path}");
            } else {
                Logger.Error($"modelholder_init2() missing texture file: {atlas_texture} in {atlas_src}");
            }

            //free(temp);
            //free(texture_path);
            FS.FolderStackPop();
        }

        return modelholder;
    }

    public static ModelHolder Init3(uint vertex_color_rgb8, Texture texture, Atlas atlas, AnimList animlist) {
        ModelHolder modelholder = new ModelHolder() {
            atlas = atlas ?? ModelHolder.STUB_ATLAS,
            animlist = animlist ?? ModelHolder.STUB_ANIMLIST,
            texture = texture,
            vertex_color_rgb8 = vertex_color_rgb8,

            id = ModelHolder.IDS++,
            instance_references = 1,
            instance_src = null
        };

        ModelHolder.POOL.Set(modelholder.id, modelholder);
        return modelholder;
    }

    public void Destroy() {
        //if (this == null) return;

        // before continue, check if there another instances
        this.instance_references--;
        if (this.instance_references > 0) return;

        if (this.instance_src != null) {
            if (this.atlas != ModelHolder.STUB_ATLAS)
                this.atlas.Destroy();

            if (this.animlist != ModelHolder.STUB_ANIMLIST)
                this.animlist.Destroy();

            if (this.texture != null)
                this.texture.Destroy();
        }

        ModelHolder.POOL.Delete(this.id);
        Luascript.DropShared(this);
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
            AtlasEntry atlas_entry = GetAtlasEntry2(atlas_entry_name);
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

    public AtlasEntry GetAtlasEntry(string atlas_entry_name) {
        if (this.atlas == ModelHolder.STUB_ATLAS) return null;

        return this.atlas.GetEntry(atlas_entry_name);
    }

    /**
     * Retrieves an atlas entry with the specified name. If the entry is not found, retrieves the first
     * entry with number suffix (example "dance left000"), 
     * @param {Object} modelholder The modelholder instance
     * @param {string} atlas_entry_name Entry name and/or entry name prefix
     * @returns {object} the atlas entry or NULL if not found
     */
    public AtlasEntry GetAtlasEntry2(string atlas_entry_name) {
        if (this.atlas == ModelHolder.STUB_ATLAS) return null;

        AtlasEntry atlas_entry = this.atlas.GetEntry(atlas_entry_name);

        if (atlas_entry == null)
            atlas_entry = this.atlas.GetEntryWithNumberSuffix(atlas_entry_name);

        return atlas_entry;
    }

    public bool GetTextureResolution(out int resolution_width, out int resolution_height) {
        if (this.atlas == ModelHolder.STUB_ATLAS) {
            resolution_width = Funkin.SCREEN_RESOLUTION_WIDTH;
            resolution_height = Funkin.SCREEN_RESOLUTION_HEIGHT;
            return false;
        }
        return this.atlas.GetTextureResolution(out resolution_width, out resolution_height);
    }

    public static bool UtilsIsKnownExtension(string filename) {
        if (StringUtils.IsEmpty(filename)) return false;
        return Atlas.UtilsIsKnownExtension(filename) || filename.LowercaseEndsWithKDY(".json");
    }
}
