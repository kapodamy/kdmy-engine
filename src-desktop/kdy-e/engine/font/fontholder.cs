using Engine.Utils;

namespace Engine.Font {

    public class FontHolder {

        public static float DEFAULT_SIZE_PIXELS = 8;// in pixels

        public IFontRender font;
        public bool font_from_atlas;
        public float font_size;


        public FontHolder(string src, float default_size_px, string glyph_suffix) {
            IFontRender font_instance;
            bool is_altas_type = src.LowercaseEndsWithKDY(".xml");

            if (is_altas_type)
                font_instance = FontGlyph.Init(src, glyph_suffix, true);
            else
                font_instance = FontType.Init(src);


            this.font = font_instance;
            this.font_from_atlas = is_altas_type;
            this.font_size = default_size_px > 0 ? default_size_px : FontHolder.DEFAULT_SIZE_PIXELS;
        }

        public FontHolder(IFontRender font_instance, bool is_altas_type, float default_size_px) {
            this.font = font_instance;
            this.font_from_atlas = is_altas_type;
            this.font_size = default_size_px > 0 ? default_size_px : FontHolder.DEFAULT_SIZE_PIXELS;
        }

        public void Destroy() {
            if (this.font_from_atlas)
                this.font.Destroy();
            else
                this.font.Destroy();

            //free(fontholder);
        }

    }

}
