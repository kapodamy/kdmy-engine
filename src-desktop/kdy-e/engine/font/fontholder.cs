using Engine.Utils;

namespace Engine.Font;

public class FontHolder {
    public IFont font;
    public bool font_from_atlas;
    public bool font_color_by_addition;


    public FontHolder(string src, string glyph_suffix, bool color_by_addition) {
        IFont font_instance;
        bool is_altas_type = src.LowercaseEndsWithKDY(".xml");

        if (is_altas_type)
            font_instance = FontGlyph.Init(src, glyph_suffix, true);
        else
            font_instance = FontType.Init(src);


        this.font = font_instance;
        this.font_from_atlas = is_altas_type;
        this.font_color_by_addition = color_by_addition;
    }

    public FontHolder(IFont font_instance, bool is_altas_type, bool color_by_addition) {
        this.font = font_instance;
        this.font_from_atlas = is_altas_type;
        this.font_color_by_addition = color_by_addition;
    }

    public void Destroy() {
        if (this.font_from_atlas)
            this.font.Destroy();
        else
            this.font.Destroy();

        //free(fontholder);
    }

}
