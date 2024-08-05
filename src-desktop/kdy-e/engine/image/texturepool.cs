using Engine.Utils;

namespace Engine.Image;

public class TexturePool {

    private readonly int max_bytes;
    private int used_bytes;
    private LinkedList<Texture> list;

    public TexturePool(int max_size) {
        this.max_bytes = max_size;
        this.used_bytes = 0;
        this.list = new LinkedList<Texture>();
    }

    public void Destroy() {
        foreach (Texture tex in this.list) {
            tex.Destroy();
        }
        this.list.Destroy();
        //free(texpool);
    }

    public void Add(Texture texture) {
        if (texture == null || texture.src_filename == null) return;// illegal operation

        // check if already present
        foreach (Texture tex in this.list) {
            if (tex == texture) return;
        }

        // check if can afford this texture
        int used_bytes = this.used_bytes + texture.GetSize();

        while (used_bytes > this.max_bytes) {
            // release textures at the start of the list
            Texture tex = this.list.GetFirstItem();
            if (tex == null) break;// empty list

            used_bytes -= tex.GetSize();
            tex.Destroy();

            if (used_bytes < this.max_bytes) break;
        }

        // adquire
        this.used_bytes = used_bytes;
        texture.ShareReference();
        this.list.AddItem(texture);
    }

}
