using System;
using Engine.Utils;
using static Engine.Platform.FSFolderEnumerator;

namespace Engine.Image {

    public static class ImgUtils {

        public static void CalcSize(float orig_width, float orig_height, float max_width, float max_height, out float result_width, out float result_height) {
            float width = orig_width, height = orig_height;

            if (max_width < 0f && max_height < 0f) {
                width = orig_width;
                height = orig_height;
            } else if (orig_width == 0f && orig_height == 0f) {
                if (max_width > 0f) width = max_width;
                if (max_height > 0f) height = max_height;
            } else if (max_width == 0f || max_height == 0f) {
                width = height = 0f;
            } else {
                if (max_width > 0f && max_height > 0f) {
                    float scale_x = max_width / orig_width;
                    float scale_y = max_height / orig_height;

                    if (scale_x > scale_y)
                        max_width = Single.NegativeInfinity;
                    else
                        max_height = Single.NegativeInfinity;
                }

                if (max_height > 0f) {
                    height = max_height;
                    width = (orig_width * max_height) / orig_height;
                }

                if (max_width > 0f) {
                    height = (orig_height * max_width) / orig_width;
                    width = max_width;
                }
            }

            result_width = width;
            result_height = height;
        }

        public static void CalcSize2(Texture texture, AtlasEntry atlas_entry, float max_width, float max_height, out float resized_width, out float resized_height) {
            float orig_width = max_width, orig_height = max_height;

            if (atlas_entry != null) {
                orig_width = atlas_entry.frame_width > 0f ? atlas_entry.frame_width : atlas_entry.width;
                orig_height = atlas_entry.frame_height > 0f ? atlas_entry.frame_height : atlas_entry.height;
            } else if (texture != null) {
                orig_width = texture.original_width;
                orig_height = texture.original_height;
            } else {
                resized_width = max_width;
                resized_height = max_height;
            }

            ImgUtils.CalcSize(orig_width, orig_height, max_width, max_height, out resized_width, out resized_height);
        }

        public static void CalcCenteredLocation(float x, float y, float width, float height, float ref_width, float ref_height, out float result_x, out float result_y) {
            if (ref_width >= 0f) x += ((ref_width - width) / 2f);
            if (ref_height >= 0f) y += ((ref_height - height) / 2f);

            result_x = x;
            result_y = y;
        }

        public static void GetStateSpriteOriginalSize(StateSpriteState statesprite_state, ref float output_width, ref float output_height) {
            if (statesprite_state.frame_info != null) {
                if (statesprite_state.frame_info.frame_width > 0f)
                    output_width = statesprite_state.frame_info.frame_width;
                else
                    output_width = statesprite_state.frame_info.width;

                if (statesprite_state.frame_info.frame_height > 0f)
                    output_height = statesprite_state.frame_info.frame_height;
                else
                    output_height = statesprite_state.frame_info.height;
            } else if (statesprite_state.texture != null) {
                statesprite_state.texture.GetOriginalDimmensions(out output_width, out output_height);
            }
        }

        public static void CalcRectangle(float x, float y, float max_width, float max_height, float src_width, float src_height, Align align_hrzntl, Align align_vrtcl, float[] d_size, float[] d_location) {
            float draw_width = src_width, draw_height = src_height;
            ImgUtils.CalcSize(src_width, src_height, max_width, max_height, out draw_width, out draw_height);

            if (max_width > 0f && (align_hrzntl == Align.CENTER || align_hrzntl == Align.END)) {
                float offset = (max_width - draw_width);
                if (align_hrzntl == Align.CENTER) offset /= 2.0f;
                x += offset;
            }
            if (max_height > 0f && (align_vrtcl == Align.CENTER || align_vrtcl == Align.END)) {
                float offset = (max_height - draw_height);
                if (align_hrzntl == Align.CENTER) offset /= 2.0f;
                y += offset;
            }

            if (d_size != null) {
                d_size[0] = draw_width;
                d_size[1] = draw_height;
            }

            if (d_location != null) {
                d_location[0] = x;
                d_location[1] = y;
            }
        }

        public static void CalcRectangleInSprite(float x, float y, float max_width, float max_height, Align align_hrzntl, Align align_vrtcl, Sprite sprite) {
            float[] draw_size = { 0f, 0f };
            float[] draw_location = { -1f, 1f };

            sprite.GetSourceSize(out draw_size[0], out draw_size[1]);
            ImgUtils.CalcRectangle(
                x, y, max_width, max_height,
                draw_size[0], draw_size[1], align_hrzntl, align_vrtcl, draw_size, draw_location
            );

            sprite.SetDrawSize(draw_size[0], draw_size[1]);
            sprite.SetDrawLocation(draw_location[0], draw_location[1]);
        }

        public static void CalcRectangleInStateSpriteState(float off_x, float off_y, float max_width, float max_height, Align align_hrzntl, Align align_vrtcl, StateSpriteState statesprite_state) {
            float[] draw_size = { 0f, 0f };
            float[] draw_location = { 0f, 0f };

            if (statesprite_state == null) return;

            ImgUtils.GetStateSpriteOriginalSize(statesprite_state, ref draw_size[0], ref draw_size[1]);
            ImgUtils.CalcRectangle(
                off_x, off_y, max_width, max_height,
                draw_size[0], draw_size[1], align_hrzntl, align_vrtcl, draw_size, draw_location
            );

            statesprite_state.draw_width = draw_size[0];
            statesprite_state.draw_height = draw_size[1];
            statesprite_state.offset_x = draw_location[0];
            statesprite_state.offset_y = draw_location[1];
        }

        public static void CalcResizeSprite(Sprite sprite, float max_width, float max_height, bool cover, bool center) {
            float draw_width, draw_height;
            float location_x, location_y;
            if (cover) {
                sprite.GetSourceSize(out draw_width, out draw_height);
                float ratio_width = draw_width / max_width;
                float ratio_height = draw_height / max_height;

                // resize the longest dimension of the sprite
                if (ratio_width > ratio_height)
                    sprite.ResizeDrawSize(-1, max_height, out draw_width, out draw_height);
                else
                    sprite.ResizeDrawSize(max_width, -1, out draw_width, out draw_height);
            } else {
                sprite.ResizeDrawSize(max_width, max_height, out draw_width, out draw_height);
            }
            if (center) {
                sprite.GetDrawLocation(out location_x, out location_y);
                if (max_width >= 0) location_x -= (draw_width - max_width) / 2.0f;
                if (max_height >= 0) location_y -= (draw_height - max_height) / 2.0f;
                sprite.SetDrawLocation(location_x, location_y);
            }
        }
    }

}
