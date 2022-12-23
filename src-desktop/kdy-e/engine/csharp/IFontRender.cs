using Engine.Platform;

namespace Engine.Font {
    public interface IFontRender : IAnimate {
        void Destroy();
        float DrawText(PVRContext pvrctx, float height, float x, float y, int text_index, int text_size, string text);
        void EnableBorder(bool enable);
        void EnableColorByDifference(bool enable);
        void MeansureChar(int codepoint, float height, FontLineInfo lineinfo);
        float Measure(float height, string text, int text_index, int text_size);
        void SetAlpha(float alpha);
        void SetBorder(bool enable, float size, float[] rgba);
        void SetBorderOffset(float x, float y);
        void SetBorderColor(float r, float g, float b, float a);
        void SetBorderColorRGBA8(uint rbga8_color);
        void SetBorderSize(float size);
        void SetColor(float r, float g, float b);
        void SetLinesSeparation(float height);
        void SetRGB8Color(uint rbg8_color);
    }
}