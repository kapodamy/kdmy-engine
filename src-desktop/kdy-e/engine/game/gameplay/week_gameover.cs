using System;
using Engine.Animation;
using Engine.Font;
using Engine.Game.Common;
using Engine.Game.Helpers;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;

namespace Engine.Game.Gameplay {

    public class WeekGameOver : IDraw, IAnimate {

        public const string JUDGEMENT =
            "sicks  $i\ngood   $i\nbads   $i\nshits  $i\n\n" +
            "misses      $i\npenalties   $i\ndifficult  $s\n\n\n" +
            "Lost at $s ($2d% completed)";

        public const string HELP_RETRY = "retry";
        public const string HELP_DIFFICULT = "change difficult";
        public const string HELP_GIVEUP = "giveup";

        public const string DURATION_DIE = "gameover_duration_die";
        public const string DURATION_RETRY = "gameover_duration_retry";
        public const string DURATION_GIVEUP = "gameover_duration_giveup";

        public const float LAYOUT_Z_INDEX = 200.0f;
        public const string LAYOUT_DREAMCAST = "/assets/common/image/week-round/gameover~dreamcast.xml";
        public const string LAYOUT_WIDESCREEN = "/assets/common/image/week-round/gameover.xml";
        public const string LAYOUT_VERSION = "/assets/common/image/week-round/gameover_version.txt";

        public const GamepadButtons BUTTONS = GamepadButtons.B | GamepadButtons.START | GamepadButtons.T_LR | GamepadButtons.BACK;
        public const GamepadButtons BUTTONS2 = WeekGameOver.BUTTONS | GamepadButtons.AD_LEFT | GamepadButtons.AD_RIGHT;
        public const GamepadButtons BUTTONS_LEFT = GamepadButtons.AD_LEFT | GamepadButtons.TRIGGER_LEFT;
        public const GamepadButtons BUTTONS_SELECTOR = GamepadButtons.AD_LEFT | GamepadButtons.AD_RIGHT | GamepadButtons.T_LR;


        private Layout layout;
        private WeekSelectorHelpText help_retry;
        private WeekSelectorHelpText help_difficult;
        private WeekSelectorHelpText help_giveup;
        private WeekSelectorDifficult selector;
        private Drawable drawable;
        private bool disabled;
        private WeekInfo weekinfo;
        private string difficult;
        private WeekSelectorWeekTitle choosen_difficult;
        private int group_id_help;
        private float default_die_duration;
        private float default_retry_duration;
        private float default_giveup_duration;


        public WeekGameOver() {
            string src = PVRContext.global_context.IsWidescreen() ? WeekGameOver.LAYOUT_WIDESCREEN : WeekGameOver.LAYOUT_DREAMCAST;
            ModelHolder button_icons = ModelHolder.Init(WeekSelector.BUTTONS_MODEL);
            ModelHolder ui_icons = ModelHolder.Init(WeekSelector.UI_ICONS_MODEL);
            AnimList animlist = AnimList.Init(WeekSelector.UI_ANIMS);
            Layout layout = Layout.Init(src);

            if (layout == null) throw new Exception("missing or invalid layout for gameover screen");

            WeekSelectorHelpText help_retry = new WeekSelectorHelpText(
                button_icons, layout, 3, false, WeekSelector.BUTTON_START, WeekGameOver.HELP_RETRY, null
            );
            WeekSelectorHelpText help_difficult = new WeekSelectorHelpText(
               button_icons, layout, 2, false, WeekSelector.BUTTON_LT_RT, WeekGameOver.HELP_DIFFICULT, null
           );
            WeekSelectorHelpText help_giveup = new WeekSelectorHelpText(
                button_icons, layout, 1, false, WeekSelector.BUTTON_B, WeekGameOver.HELP_GIVEUP, null
            );
            button_icons.Destroy();

            WeekSelectorDifficult selector = new WeekSelectorDifficult(animlist, ui_icons, layout);
            selector.SetVisible(false);
            ui_icons.Destroy();


            this.layout = layout; this.help_retry = help_retry; this.help_difficult = help_difficult; this.help_giveup = help_giveup; this.selector = selector;
            this.drawable = null;
            this.disabled = true;
            this.weekinfo = null;
            this.difficult = null;
            this.choosen_difficult = new WeekSelectorWeekTitle(layout);
            this.group_id_help = this.layout.ExternalCreateGroup(null, 0);

            this.default_die_duration = layout.GetAttachedValueAsFloat(
                 WeekGameOver.DURATION_DIE, 3000.0f
            );
            this.default_retry_duration = layout.GetAttachedValueAsFloat(
                 WeekGameOver.DURATION_RETRY, 2000.0f
            );
            this.default_giveup_duration = layout.GetAttachedValueAsFloat(
                WeekGameOver.DURATION_GIVEUP, 500.0f
            );

            this.drawable = new Drawable(
                WeekGameOver.LAYOUT_Z_INDEX, this, this
            );

            layout.ExternalVertexCreateEntries(3);
            layout.ExternalVertexSetEntry(
                0, PVRContextVertex.DRAWABLE, help_retry.GetDrawable(), this.group_id_help
            );
            layout.ExternalVertexSetEntry(
                1, PVRContextVertex.DRAWABLE, help_difficult.GetDrawable(), this.group_id_help
            );
            layout.ExternalVertexSetEntry(
                2, PVRContextVertex.DRAWABLE, help_giveup.GetDrawable(), this.group_id_help
            );
        }

        public void Destroy() {
            this.layout.Destroy();
            this.help_retry.Destroy();
            this.help_difficult.Destroy();
            this.help_giveup.Destroy();
            this.selector.Destroy();
            this.drawable.Destroy();
            this.choosen_difficult.Destroy();
            //free(this);
        }

        public static int ReadVersion() {
            Grapheme garapheme = new Grapheme() { code = 0, size = 0 };
            string version = FS.ReadText(WeekGameOver.LAYOUT_VERSION);
            StringUtils.GetCharacterCodepoint(version, 0, garapheme);
            //free(version);
            return garapheme.code;
        }


        public void Hide() {
            this.disabled = true;
            this.weekinfo = null;
            this.difficult = null;
            this.selector.Load(null, null);
            this.drawable.SetVisible(false);
        }

        public void Display(double timestamp, double duration, PlayerStats playerstats, WeekInfo weekinfo, string difficult) {
            double percent = Math.Min((timestamp / duration) * 100.0, 100.00);

            this.layout.SetGroupVisibilityById(this.group_id_help, true);

            this.selector.SetVisible(false);
            this.layout.TriggerAny(null);// restart any animation and/or camera
            this.drawable.SetVisible(true);

            TextSprite title = this.layout.GetTextsprite("title");
            if (title != null) {
                string display_name = !String.IsNullOrEmpty(weekinfo.display_name) ? weekinfo.display_name : weekinfo.name;
                title.SetTextIntern(true, display_name);
            }

            TextSprite jugement = this.layout.GetTextsprite("judgement");
            if (jugement != null) {
                string time = Math2D.TimestampToString(timestamp);
                jugement.SetTextFormated(
                    WeekGameOver.JUDGEMENT,

                    playerstats.GetSicks(),
                    playerstats.GetGoods(),
                    playerstats.GetBads(),
                    playerstats.GetShits(),

                    playerstats.GetMisses(),
                    playerstats.GetPenalties(),
                    difficult,

                    time,
                    percent
                );
                //free(time);
            }

            this.weekinfo = weekinfo;
            this.difficult = difficult;
            this.disabled = true;
        }

        public void DisplaySelector() {
            this.selector.Load(
                this.weekinfo, this.difficult
        );
            this.selector.Relayout(false);// obligatory
            this.selector.SetVisible(true);
            this.layout.TriggerAny("selector_show");
        }

        public void DisplayChoosenDifficult() {
            this.layout.TriggerAny("changed_difficult");
            this.choosen_difficult.MoveDifficult(this.selector);
        }

        public Drawable GetDrawable() {
            return this.drawable;
        }

        public string GetDifficult() {
            return this.difficult;
        }

        public int HelperAskToPlayer(RoundContext roundcontext) {
            GamepadButtons pressed = GamepadButtons.NOTHING;
            Layout layout = roundcontext.layout ?? roundcontext.ui_layout;

            // match antialiasing with the stage layout
            this.layout.DisableAntialiasing(layout.IsAntialiasingDisabled());
            this.layout.SetLayoutAntialiasing(layout.GetLayoutAntialiasing());

            Gamepad controller = new Gamepad(-1);
            controller.SetButtonsDelay(WeekSelector.BUTTON_DELAY);
            controller.ClearButtons();

            // ¿which player is dead?    
            int dead_player_index = roundcontext.players_size < 2 ? 0 : 1;

            if (roundcontext.girlfriend != null) {
                roundcontext.girlfriend.PlayExtra("cry", false);
            }

            if (roundcontext.layout.TriggerAny("camera_gameover") < 1) {
                for (int i = 0 ; i < roundcontext.players_size ; i++) {
                    if (roundcontext.players[i].playerstats == null) continue;
                    if (roundcontext.players[i].is_opponent) {
                        roundcontext.players[i].character.PlayExtra(Funkin.OPPONENT_VICTORY, false);
                        continue;
                    }

                    if (roundcontext.players[i].playerstats.IsDead()) {
                        dead_player_index = i;
                        roundcontext.players[i].character.PlayExtra(Funkin.PLAYER_DIES, false);

                        string target = roundcontext.players[i].is_opponent ? Week.ROUND_CAMERA_OPONNENT : Week.ROUND_CAMERA_PLAYER;
                        Week.CameraFocusGuess(roundcontext, target, i);
                    } else {
                        roundcontext.players[i].character.PlayExtra(Funkin.PLAYER_SCARED, false);
                    }
                }
            }

            // durations
            float die_animation_duration = layout.GetAttachedValueAsFloat(
                WeekGameOver.DURATION_DIE, this.default_die_duration
            );
            float retry_animation_duration = layout.GetAttachedValueAsFloat(
                 WeekGameOver.DURATION_RETRY, this.default_retry_duration
            );
            float giveup_animation_duration = layout.GetAttachedValueAsFloat(
                 WeekGameOver.DURATION_GIVEUP, this.default_giveup_duration
            );

            SoundPlayer sfx_die = SoundPlayer.Init("/assets/common/sound/loss_sfx.ogg");
            SoundPlayer music_bg = SoundPlayer.Init("/assets/common/music/gameOver.ogg");
            SoundPlayer sfx_end = SoundPlayer.Init("/assets/common/sound/gameOverEnd.ogg");

            if (sfx_die != null) sfx_die.Replay();

            // try draw only the dead player
            string character_name = Week.InternalConcatSuffix(Week.ROUND_CHARACTER_PREFIX, dead_player_index);
            roundcontext.layout.SetSingleItemToDraw(character_name);
            //free(character_name);

            // trigger layout (normally shows the player only with a black background)
            bool stage_has_gameover = (bool)layout.GetAttachedValue(
                "gameover_with_stage", AttachedValueType.BOOLEAN, false
            );
            if (stage_has_gameover) layout.TriggerAny("gameover");

            int decision;
            bool selector_unloaded = true;
            GamepadButtons selector_buttons = GamepadButtons.T_LR;
            GamepadButtons ui_buttons = WeekGameOver.BUTTONS;
            double total = 0;

            while (true) {
                float elapsed = PVRContext.global_context.WaitReady();

                if (roundcontext.script != null) roundcontext.script.NotifyFrame(elapsed);

                // animate & draw first to minimize suttering
                PVRContext.global_context.Reset();
                layout.Animate(elapsed);

                Draw2(roundcontext, PVRContext.global_context);

                GamepadButtons buttons = controller.HasPressedDelayed(ui_buttons);

                if ((buttons & (GamepadButtons.B | GamepadButtons.BACK)) != GamepadButtons.NOTHING) {
                    // giveup
                    decision = 1;
                    break;
                } else if ((buttons & GamepadButtons.START) != GamepadButtons.NOTHING) {
                    // retry
                    decision = 2;
                    this.disabled = false;
                    if (selector_unloaded) break;

                    // the string returned is constant
                    string difficult = this.selector.GetSelected();
                    if (difficult == null) break;// is locked or not selected

                    if (difficult != this.difficult) {
                        this.difficult = difficult;
                        DisplayChoosenDifficult();
                    }

                    decision = 2;
                    break;
                } else if ((buttons & selector_buttons) != GamepadButtons.NOTHING) {
                    // change difficult
                    if (selector_unloaded) {
                        // show the selector and wait until the custom difficulties are readed from disk
                        controller.ClearButtons();
                        DisplaySelector();
                        selector_unloaded = false;
                        selector_buttons = WeekGameOver.BUTTONS_SELECTOR;
                        ui_buttons = WeekGameOver.BUTTONS2;
                    } else {
                        // switch difficult
                        int offset = ((buttons & WeekGameOver.BUTTONS_LEFT) != GamepadButtons.NOTHING) ? -1 : 1;
                        this.selector.Scroll(offset);
                    }
                }

                total += elapsed;
                if (this.disabled && total > die_animation_duration) {
                    this.disabled = false;
                    this.layout.SetGroupVisibilityById(this.group_id_help, true);
                    if (music_bg != null) {
                        music_bg.Replay();
                        music_bg.LoopEnable(true);
                    }
                    if (sfx_die != null) sfx_die.Stop();
                }
            }

            if (roundcontext.players_size > 0) {
                string anim = decision == 2 ? Funkin.PLAYER_RETRY : Funkin.PLAYER_GIVEUP;
                if (roundcontext.players[dead_player_index].character.HasDirection(anim, true)) {
                    roundcontext.players[dead_player_index].character.PlayExtra(anim, false);
                }
            }

            if (sfx_die != null) sfx_die.Stop();
            this.layout.SetGroupVisibilityById(this.group_id_help, false);

            if (decision == 2) {
                this.layout.TriggerAny("hide_stats");
                if (music_bg != null) music_bg.Stop();
                if (sfx_end != null) sfx_end.Replay();
            }

            total = decision == 2 ? retry_animation_duration : giveup_animation_duration;
            bool trigger_transition = true;
            while (total > 0) {
                float elapsed = PVRContext.global_context.WaitReady();

                for (int i = 0 ; i < roundcontext.players_size ; i++) {
                    Gamepad controller_player = roundcontext.players[i].controller;
                    if (controller_player != null && controller_player.GetManagedPresses(true, ref pressed)) {
                        if (roundcontext.script != null) roundcontext.script.NotifyButtons(i, pressed);
                    }
                }

                if (roundcontext.script != null) roundcontext.script.NotifyFrame(elapsed);

                // animate & draw first to minimize suttering
                PVRContext.global_context.Reset();
                layout.Animate(elapsed);

                Draw2(roundcontext, PVRContext.global_context);

                if (controller.HasPressedDelayed(GamepadButtons.START | GamepadButtons.A | GamepadButtons.X) != GamepadButtons.NOTHING) {
                    this.layout.TriggerAny("transition_force_end");
                    break;
                }

                if (trigger_transition && total <= 500.0) {
                    trigger_transition = false;
                    this.layout.TriggerAny("transition");
                    if (decision == 1) music_bg.Fade(false, (float)total);
                } else if (trigger_transition) {
                    total -= elapsed;
                } else if (this.layout.AnimationIsCompleted("transition_effect") > 0) {
                    break;
                }
            }

            if (sfx_die != null) sfx_die.Destroy();
            if (sfx_end != null) sfx_end.Destroy();
            if (music_bg != null) music_bg.Destroy();
            controller.Destroy();
            roundcontext.layout.SetSingleItemToDraw(null);

            return decision;
        }

        public int Animate(float elapsed) {
            if (this.disabled) return 1;
            return this.layout.Animate(elapsed);
        }

        public void Draw(PVRContext pvrctx) {
            if (this.disabled) return;
            this.layout.Draw(pvrctx);
        }

        public void Draw2(RoundContext roundcontext, PVRContext pvrctx) {
            roundcontext.screen_background.Draw(pvrctx);
            (roundcontext.layout ?? roundcontext.ui_layout).Draw(pvrctx);
            if (this.disabled) return;
            this.layout.Draw(pvrctx);
        }

    }

}
