namespace Engine {
    public interface IVertex: ISetProperty, IDraw, IAnimate {

        void GetDrawSize(out float draw_width, out float draw_height);

        void GetDrawLocation(out float draw_x, out float draw_y);

        float GetZIndex();

        bool IsVisible();
        void Destroy();
    }
}
