using System;
using System.Threading;
using Engine.Externals;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Image;

public class Texture {
    private static int TEXTURE_IDS = 0;
    private static Map<Texture> TEXTURE_POOL = new Map<Texture>();
    private static bool texture_defer_enabled = true;

    private int size;
    internal WebGLTexture data_vram;
    private ImageData data_ram;

    internal int width;
    internal int height;
    internal int original_width;
    internal int original_height;

    private int id;
    internal string src_filename;
    private int references;
    private int cache_references;
    internal bool has_mipmaps;
    internal WebGLTexture[] data_vram_crhoma_planes;


    public static Texture Init(string src) {
        return Texture.InitDeferred(src, texture_defer_enabled);
    }

    public static Texture InitDeferred(string src, bool deffered) {
        Texture loaded_texture = Texture.GetFromGlobalPool(src);
        if (loaded_texture != null) {
            // This texture is already in use, bump the reference count and reuse it
            loaded_texture.references++;
            return loaded_texture;
        }

        ImageData data_ram = FS.ReadTexture(src);
        if (data_ram == null) return null;

        Texture texture = InitFromRAW(
            data_ram, data_ram.size, false, data_ram.pow2_width, data_ram.pow2_height, -1, -1
        );

        // remember the filename to avoid duplicated textures
        texture.src_filename = FS.GetFullPathAndOverride(src);

        // original size if was readed from an standard image file (png, jpg, bmp)
        texture.original_width = data_ram.orig_width;
        texture.original_height = data_ram.orig_height;

        if (Thread.CurrentThread.Name == "MainThread" && !deffered) texture.UploadToPVR();

        return texture;
    }

    public static Texture InitFromRAW(object ptr, int size, bool in_vram, int width, int height, int orig_width, int orig_height) {
        Texture texture = new Texture();
        texture.size = size;
        texture.data_vram = WebGLTexture.Null;
        texture.data_ram = null;

        texture.width = width;
        texture.height = height;
        texture.original_width = orig_width;
        texture.original_height = orig_height;

        texture.id = TEXTURE_IDS++;
        texture.src_filename = null;
        texture.references = 1;
        texture.cache_references = 0;
        texture.has_mipmaps = false;
        texture.data_vram_crhoma_planes = null;

        if (in_vram)
            texture.data_vram = (WebGLTexture)ptr;
        else
            texture.data_ram = (ImageData)ptr;

        if (!in_vram && texture.data_ram.pixelDataBuffer is DDSPixelDataBuffer) {
            DDSPixelDataBuffer dds = (DDSPixelDataBuffer)texture.data_ram.pixelDataBuffer;
            texture.has_mipmaps = dds.Mipmaps.Length > 0;
        }

        TEXTURE_POOL.Set(texture.id, texture);

        return texture;
    }


    public Texture ShareReference() {
        this.references++;
        return this;
    }

    public void Cache(bool adquire) {
        if (adquire)
            this.cache_references++;
        else if (this.cache_references > 0)
            this.cache_references--;

        //
        // This functionality is only available in the Dreamcast
        // OpenGL and WebGL has lot of limitations
        //
    }

    public void UploadToPVR() {
        if (!this.data_vram.IsNull) return;

        if (this.data_ram == null) {
            // null data loaded
            return;
        }

        this.data_vram = PVRContext.global_context.webopengl.CreateTexture(this.width, this.height, this.data_ram);

        if (this.data_vram.IsNull) {
            Logger.Error($"texture_upload_to_pvr() failed, id={this.id} src={this.src_filename}");
        }

        if (this.cache_references > 0) return;

        this.data_ram.Dispose();
        this.data_ram = null;
    }

    public int GetSize() {
        return this.size;
    }

    public void GetOriginalDimmensions(out float width, out float height) {
        width = this.original_width;
        height = this.original_height;
    }

    public void GetDimmensions(out float width, out float height) {
        width = this.width;
        height = this.height;

    }


    public void DestroyForce() {
        this.references = 0;
        this.cache_references = 0;
        Destroy();
    }

    public bool Destroy() {
        this.references--;

        // check if the texture is shared somewhere
        if (this.references > 0) {
            return false;
        }

        // JS only
        if (this.data_ram != null) {
            this.data_ram.Dispose();
        }
        if (!this.data_vram.IsNull) {
            PVRContext.global_context.webopengl.DestroyTexture(this.data_vram);
        }
        if (this.data_vram_crhoma_planes != null) {
            PVRContext.global_context.webopengl.DestroyTexture(this.data_vram_crhoma_planes[0]);
            PVRContext.global_context.webopengl.DestroyTexture(this.data_vram_crhoma_planes[1]);
        }

        this.data_ram = null;
        this.data_vram = WebGLTexture.Null;
        //free(this.src_filename);
        TEXTURE_POOL.Delete(this.id);
        return true;
    }


    public static Texture GetFromGlobalPool(string src) {
        Texture texture = null;
        if (src != null) {
            string filename = FS.GetFullPathAndOverride(src);
            foreach (Texture loaded_texture in TEXTURE_POOL) {
                if (loaded_texture.src_filename == filename) {
                    texture = loaded_texture;
                    break;
                }
            }
            //free(filename);
        }
        return texture;
    }

    public static void DisableDefering(bool disable) {
        Texture.texture_defer_enabled = disable;
    }

}
