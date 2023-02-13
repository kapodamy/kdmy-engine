using Engine.Font;
using Engine.Game.Common;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Game {

    public class IntroScreen {

        private const float DELAY = 600;
        private const float TEXT_DURATION = 0;
        private const string TEXT_SPARSE = "--";
        private const string LAYOUT = "/assets/common/image/intro-screen/layout.xml";
        private const string LAYOUT_DREAMCAST = "/assets/common/image/intro-screen/layout~dreamcast.xml";
        private const string MODDING_SCRIPT = "/assets/common/data/scripts/introscreen.lua";
        private const GamepadButtons SKIP_BUTTONS = GamepadButtons.START | GamepadButtons.A;

        public static void Main() {
            Layout layout = Layout.Init(PVRContext.global_context.IsWidescreen() ? LAYOUT : LAYOUT_DREAMCAST);
            if (layout == null) return;

            Modding modding = new Modding(layout, IntroScreen.MODDING_SCRIPT);
            Gamepad maple_pad = new Gamepad(-1);

            modding.HelperNotifyInit(Modding.NATIVE_MENU_SCREEN);

            // read delay/durations from layout
            float delay = layout.GetAttachedValueAsFloat("delay", IntroScreen.DELAY);
            double custom_duration = layout.GetAttachedValueAsFloat(
                "custom_duration", IntroScreen.TEXT_DURATION
            );

            if (custom_duration > 0.0) {
                modding.HelperNotifyEvent("custom-intro");

                // custom intro detected wait the requeted time
                double progress = 0.0;
                while (progress < custom_duration) {
                    float elapsed = PVRContext.global_context.WaitReady();
                    PVRContext.global_context.Reset();

                    ModdingHelperResult res = modding.HelperHandleCustomMenu(maple_pad, elapsed);
                    if (modding.has_exit || res != ModdingHelperResult.CONTINUE) break;

                    layout.Animate(elapsed);
                    layout.Draw(PVRContext.global_context);
                    if (modding.has_halt) continue;

                    progress += elapsed;
                    if (maple_pad.HasPressed(IntroScreen.SKIP_BUTTONS).Bool()) break;
                }

                modding.HelperNotifyExit2();

                if (modding != null) modding.Destroy();
                layout.Destroy();
                maple_pad.Destroy();
                return;
            }

            if (layout.GetTextsprite("greetings") == null) {
                layout.Destroy();
                maple_pad.Destroy();
                return;
            }

            string self_text = FS.ReadText("/assets/engineText.txt");
            string intro_text = IntroScreen.ReadIntroText("/assets/common/data/introText.txt");
            string week_greetings = null;
            string funky = (string)layout.GetAttachedValue("funky", AttachedValueType.STRING, Funkin.FUNKY);

            // 25% chance of displaying choosen week greetings
            if (Math2D.RandomFloat() <= 0.25f) week_greetings = IntroScreen.ReadWeekGretings();

            // if there an camera animation called "camera_animation", use it
            layout.TriggerCamera("camera_animation");

            float engine_duration = delay * 4;
            float greetings_duration = delay * 4;
            float funkin_duration = delay * 5;

            // start this game
            if (GameMain.background_menu_music != null) GameMain.background_menu_music.Play();

            layout.TriggerAny("intro-engine");
            modding.HelperNotifyEvent("intro-engine");
            IntroScreen.DrawSparseText(self_text, delay, engine_duration, modding, maple_pad);

            if (week_greetings != null) {
                layout.TriggerAny("intro-week-grettings");
                modding.HelperNotifyEvent("intro-week-grettings");
                IntroScreen.DrawSparseText(week_greetings, delay, greetings_duration, modding, maple_pad);
            } else {
                layout.TriggerAny("intro-greetings");
                modding.HelperNotifyEvent("intro-greetings");
                IntroScreen.DrawSparseText(intro_text, delay, greetings_duration, modding, maple_pad);
            }

            layout.TriggerAny("intro-funkin");
            modding.HelperNotifyEvent("intro-funkin");
            IntroScreen.DrawSparseText(funky, delay, funkin_duration, modding, maple_pad);

            modding.HelperNotifyExit2();

            // dispose resources used
            //if (funky != Funkin.FUNKY) free(funky);
            //free(self_text);
            //free(intro_text);
            //free(week_greetings);
            modding.Destroy();
            layout.Destroy();
            maple_pad.Destroy();
        }

        private static void DrawSparseText(string text, float delay, float duration, Modding modding, Gamepad maple_pad) {
            Layout layout = modding.layout;
            TextSprite textsprite = layout.GetTextsprite("greetings");
            if (textsprite == null || text == null || modding.has_exit) return;

            string text_buffer = "";
            int text_length = text.Length;
            int lines = text.OccurrencesOfKDY(IntroScreen.TEXT_SPARSE) + 1;
            double paragraph_duration = duration / (double)lines;
            double progress = paragraph_duration - delay;
            int sparse_length = IntroScreen.TEXT_SPARSE.Length;
            int last_index = 0;

            while (true) {
                if (progress >= paragraph_duration) {
                    if (last_index >= text_length) break;
                    if (last_index > 0) text_buffer += "\n";

                    int index = text.IndexOf(IntroScreen.TEXT_SPARSE, last_index);
                    if (index < 0) index = text_length;

                    text_buffer += text.SubstringKDY(last_index, index);
                    last_index = index + sparse_length;

                    textsprite.SetTextIntern(true, text_buffer);
                    progress -= paragraph_duration;
                }

                float elapsed = PVRContext.global_context.WaitReady();
                PVRContext.global_context.Reset();

                ModdingHelperResult res = modding.HelperHandleCustomMenu(maple_pad, elapsed);
                if (modding.has_exit || res != ModdingHelperResult.CONTINUE) break;

                layout.Animate(elapsed);
                layout.Draw(PVRContext.global_context);
                if (modding.has_halt) continue;

                progress += elapsed;
                if (maple_pad.HasPressed(IntroScreen.SKIP_BUTTONS) != GamepadButtons.NOTHING) break;
            }

            //free(text_buffer);
        }

        private static string ReadIntroText(string path) {
            string lines = FS.ReadText(path);
            int line_count = 0;

            if (lines == null) return null;

            // count lines in the file
            int index = 0;
            while (true) {
                line_count++;
                index = lines.IndexOf("\n", index);
                if (index < 0) break;
                index++;
            }

            // choose a random line
            int random_index = Math2D.RandomInt(1, line_count);
            line_count = 0;

            // read the choosen line
            index = 0;
            while (line_count < random_index) {
                line_count++;
                index = lines.IndexOf('\n', index) + 1;
            }

            int next_index = lines.IndexOf('\n', index++);
            if (next_index < 0) next_index = lines.Length;

            return lines.SubstringKDY(index - 1, next_index);
        }

        public static string ReadWeekGretings() {
            if (GameMain.custom_style_from_week == null) return null;
            string greetings = WeekEnumerator.GetGreetings(GameMain.custom_style_from_week);
            if (greetings == null) return null;
            string text = ReadIntroText(greetings);
            //free(greetings);
            return text;
        }

    }

}
