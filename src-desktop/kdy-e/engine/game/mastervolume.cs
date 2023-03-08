using CsharpWrapper;
using Engine.Externals;
using Engine.Font;
using Engine.Game.Common;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;
using KallistiOS;

namespace Engine.Game {

    public class MasterVolume {
        private const float PADDING = 6;
        private const float BAR_GAP = 4;
        private const float BAR_WIDTH = 8;
        private const float BAR_HEIGHT = 24;
        private const int LEVELS = 10;
        private const float LABEL_HEIGHT = 16;

        private const string LABEL_STRING = "VOLUME";
        private const float HIDE_DELAY = 1000f;
        private const float PRESS_DELAY = 100f;
        private const int STEPS = 100 / LEVELS;

        private const float LABEL_Y_OFFSET = BAR_HEIGHT + (PADDING * 2f);
        private const float BACK_WIDTH = (PADDING * 2f) + (LEVELS * BAR_WIDTH) + (BAR_GAP * (LEVELS - 1));
        private const float BACK_HEIGHT = (PADDING * 3f) + LABEL_HEIGHT + BAR_HEIGHT;
        private const float BACK_OFFSET_X = (Funkin.SCREEN_RESOLUTION_WIDTH - BACK_WIDTH) / 2f;
        private const float BACK_OFFSET_Y = 0f;

        private static readonly float[] BACK_COLOR = { 0f, 0f, 0f, 0.5f };
        private static readonly float[] LABEL_COLOR = { 1f, 1f, 1f, 1f };
        private static readonly float[] BAR_COLOR1 = { 1f, 1f, 1f, 1f };
        private static readonly float[] BAR_COLOR2 = { 0.5f, 0.5f, 0.5f, 1f };

        public static float label_x_offset;
        public static FontType label_font;
        public static bool is_visible;
        public static bool full_alpha;
        public static double hide_timestamp;
        //public static double antibounce_timestamp;
        public static SoundPlayer beep;

        public static void Init() {
            MasterVolume.label_font = FontType.Init("/assets/common/font/vcr.ttf");

            float width = MasterVolume.label_font.Measure(
                MasterVolume.LABEL_HEIGHT, MasterVolume.LABEL_STRING, 0, MasterVolume.LABEL_STRING.Length
            );

            MasterVolume.is_visible = false;
            MasterVolume.label_x_offset = (Funkin.SCREEN_RESOLUTION_WIDTH - width) / 2f;
            MasterVolume.beep = SoundPlayer.Init("/assets/common/sound/volume_beep.ogg");
            MasterVolume.hide_timestamp = 0.0;
            //MasterVolume.antibounce_timestamp = 0.0;


            AICA.sndbridge_set_master_volume(EngineSettings.master_volume / 100f);

            // check if necessary warn the player if muted
            if (EngineSettings.master_volume < 1) {
                MasterVolume.is_visible = true;
                MasterVolume.hide_timestamp = timer.ms_gettime64() + (MasterVolume.HIDE_DELAY * 3f);
            }
        }

        public static void VolumeStep(bool up_or_down) {
            double now = timer.ms_gettime64();

            //if (now < MasterVolume.antibounce_timestamp) return;
            //MasterVolume.antibounce_timestamp += MasterVolume.PRESS_DELAY;

            int new_volume = EngineSettings.master_volume;

            if (up_or_down)
                new_volume += MasterVolume.STEPS;
            else
                new_volume -= MasterVolume.STEPS;

            new_volume = Math2D.Clamp(new_volume, 0, 100);
            if (new_volume != EngineSettings.master_volume) {
                AICA.sndbridge_set_master_volume(new_volume / 100f);
                if (MasterVolume.beep != null) MasterVolume.beep.Replay();
            }

            EngineSettings.master_volume = new_volume;
            MasterVolume.full_alpha = MasterVolume.is_visible;
            MasterVolume.is_visible = true;
            MasterVolume.hide_timestamp = now + MasterVolume.HIDE_DELAY;
        }

        public static void Draw(PVRContext pvrctx) {
            if (!MasterVolume.is_visible) return;

            pvrctx.Save();

            float x = MasterVolume.BACK_OFFSET_X + MasterVolume.PADDING;
            float y = MasterVolume.BACK_OFFSET_Y + MasterVolume.PADDING;
            float scale_x = pvrctx.ScreenWidth / (float)Funkin.SCREEN_RESOLUTION_WIDTH;
            float scale_y = pvrctx.ScreenHeight / (float)Funkin.SCREEN_RESOLUTION_HEIGHT;
            SH4Matrix matrix = pvrctx.CurrentMatrix;

            //
            // low-level rendering
            //

            // adjust volume control to viewport
            matrix.Scale(scale_x, scale_y);

            // render background
            pvrctx.SetVertexAlpha(MasterVolume.full_alpha ? 1f : MasterVolume.BACK_COLOR[3]);
            pvrctx.DrawSolidColor(
                MasterVolume.BACK_COLOR,
                MasterVolume.BACK_OFFSET_X, MasterVolume.BACK_OFFSET_Y,
                MasterVolume.BACK_WIDTH, MasterVolume.BACK_HEIGHT
            );

            // render bars
            int master_volume = EngineSettings.master_volume;
            for (int i = 0 ; i < MasterVolume.LEVELS ; i++) {
                int volume = MasterVolume.STEPS * i;
                float[] color = volume < master_volume ? MasterVolume.BAR_COLOR1 : MasterVolume.BAR_COLOR2;

                volume += MasterVolume.STEPS;
                if (volume > 100) volume = 100;
                float bar_height = MasterVolume.BAR_HEIGHT * (volume / 100f);

                pvrctx.SetVertexAlpha(color[3]);
                pvrctx.DrawSolidColor(
                    color,
                    x, y + (MasterVolume.BAR_HEIGHT - bar_height),
                    MasterVolume.BAR_WIDTH, bar_height
                );

                x += MasterVolume.BAR_WIDTH + MasterVolume.BAR_GAP;
            }

            // render text
            if (MasterVolume.label_font == null) goto L_check_hide_timestamp;
            float[] label_color = MasterVolume.LABEL_COLOR;
            MasterVolume.label_font.EnableBorder(false);
            MasterVolume.label_font.EnableColorByDifference(false);
            MasterVolume.label_font.SetAlpha(label_color[3]);
            MasterVolume.label_font.SetColor(label_color[0], label_color[1], label_color[2]);
            MasterVolume.label_font.DrawText(
                pvrctx,
                MasterVolume.LABEL_HEIGHT,
                MasterVolume.label_x_offset, MasterVolume.LABEL_Y_OFFSET,
                0, MasterVolume.LABEL_STRING.Length, MasterVolume.LABEL_STRING
            );


// check if time to hide the volume control
L_check_hide_timestamp:
            double now = timer.ms_gettime64();
            if (now >= MasterVolume.hide_timestamp) {
                // hide volume control
                MasterVolume.is_visible = false;

                // flush engine settings to ini file ONLY IF the settings menu is not active
                if (!SettingsMenu.is_running) {
                    EngineSettings.ini.SetInt(EngineSettings.INI_MISC_SECTION, "master_volume", master_volume);
                    EngineSettings.ini.Flush();
                }
            }

            pvrctx.Restore();
        }

    }
}
