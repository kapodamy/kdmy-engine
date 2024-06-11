using Engine.Platform;

namespace Engine.Font;


public struct FontParams {
    public float height;
    public float paragraph_space;
    public bool color_by_addition;

    /// <summary>RGBA color</summary>
    public float[] tint_color;

    public bool border_enable;
    public float border_size;
    
    /// <summary>RGBA color</summary>
    public float[] border_color;

    public float border_offset_x;
    public float border_offset_y;

    public FontParams() {
        this.tint_color = new float[4];
        this.border_color = new float[4];
    }

}

public struct FontLineInfo {
    public float space_width;
    public float last_char_width;
    public int line_char_count;
    public int previous_codepoint;
}

public interface IFont : IAnimate {
    void Destroy();
    float DrawText(PVRContext pvrctx, ref FontParams @params, float x, float y, int text_index, int text_length, string text);
    void MeasureChar(int codepoint, float height, ref FontLineInfo lineinfo);
    float Measure(ref FontParams @params, string text, int text_index, int text_length);
    void MapCodepoints(int text_index, int text_length, string text);
}
