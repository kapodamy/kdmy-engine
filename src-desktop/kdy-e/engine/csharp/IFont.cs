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

public struct FontCharInfo {
    public float last_char_width;
    public float last_char_height;
    public int line_char_count;
    public uint previous_codepoint;
    public float last_char_width_end;
}

public struct FontLinesInfo {
    public float max_width;
    public float total_height;
}

public interface IFont : IAnimate {
    void Destroy();
    float DrawText(PVRContext pvrctx, ref FontParams @params, float x, float y, int text_index, int text_length, string text);
    void MeasureChar(uint codepoint, float height, ref FontCharInfo charinfo);
    void Measure(ref FontParams @params, string text, int text_index, int text_length, ref FontLinesInfo linesinfo);
    float MeasureLineHeight(float height);
    void MapCodepoints(string text, int text_index, int text_length);
}
