using Engine.Font;
using Engine.Platform;

namespace Engine.Game.Gameplay {

    public class WeekResult {

        private const string WEEK_TEXT = "~judgements~\nSicks   $i\nGoods  $i\nBads   $i\n" +
                                        "\n~failures~\nShits      $i\nMissses    $i\nPenalties  $i" +
                                        "\n\n~streak~\nCombo breaks   $i\nHighest combo  $i";
        private const string WEEK_TEXT2 = "~week resume~\n" +
                                           "Total attempts $i\nTotal score    $l\nAverage accuracy $3d%";

        private const string TRACK_TEXT = "~judgements~\nSicks  $i\nGoods  $i\nBads   $i\n" +
                                            "\n~failures~\nShits      $i\nMissses    $i\nPenalties  $i" +
                                            "\n\n~streak~\nCombo breaks   $i\nHighest combo  $i\nHighest nps    $i";
        private const string TRACK_TEXT2 = "~track resume~\nAttempts $i\nScore    $l\nAvg. accuracy $3d%";

        public const string LAYOUT_WIDESCREEN = "/assets/common/image/week-round/results.xml";
        public const string LAYOUT_DREAMCAST = "/assets/common/image/week-round/results~dreamcast.xml";


        private StatsTrack stats_track;
        private StatsAccumulated accumulated_stats_week;


        public WeekResult() {

            this.stats_track = new StatsTrack() {
                sick = 0,
                good = 0,
                bads = 0,
                shits = 0,
                miss = 0,
                penalties = 0,
                score = 0,
                accuracy = 0,
                notesperseconds = 0,
                combobreaks = 0,
                higheststreak = 0,
            };
            this.accumulated_stats_week = new StatsAccumulated() {
                sick = 0,
                good = 0,
                bads = 0,
                shits = 0,
                miss = 0,
                penalties = 0,
                score = 0,
                accuracy = 0,
                notesperseconds = 0,
                combobreaks = 0,
                higheststreak = 0,
            };

        }

        public void Destroy() {
            //free(this);
        }

        public void AddStats(RoundContext roundcontext) {
            for (int i = 0 ; i < roundcontext.players_size ; i++) {
                if (roundcontext.players[i].type != CharacterType.PLAYER) continue;
                if (roundcontext.players[i].is_opponent) continue;
                PlayerStats playerstats = roundcontext.players[i].playerstats;

                this.stats_track.sick = playerstats.GetSicks();
                this.stats_track.good = playerstats.GetGoods();
                this.stats_track.bads = playerstats.GetBads();
                this.stats_track.shits = playerstats.GetShits();
                this.stats_track.miss = playerstats.GetMisses();
                this.stats_track.penalties = playerstats.GetPenalties();
                this.stats_track.score = playerstats.GetScore();
                this.stats_track.accuracy = playerstats.GetAccuracy();
                this.stats_track.notesperseconds = playerstats.GetNotesPerSecondsHighest();
                this.stats_track.combobreaks = playerstats.GetComboBreaks();

                int higheststreak = playerstats.GetHighestComboStreak();
                if (higheststreak > this.stats_track.higheststreak) {
                    this.stats_track.higheststreak = higheststreak;
                }
            }

            this.accumulated_stats_week.sick += this.stats_track.sick;
            this.accumulated_stats_week.good += this.stats_track.good;
            this.accumulated_stats_week.bads += this.stats_track.bads;
            this.accumulated_stats_week.shits += this.stats_track.shits;
            this.accumulated_stats_week.miss += this.stats_track.miss;
            this.accumulated_stats_week.penalties += this.stats_track.penalties;
            this.accumulated_stats_week.score += this.stats_track.score;
            this.accumulated_stats_week.accuracy += this.stats_track.accuracy;
            this.accumulated_stats_week.notesperseconds += this.stats_track.notesperseconds;
            this.accumulated_stats_week.combobreaks += this.stats_track.combobreaks;

            if (this.stats_track.higheststreak > this.accumulated_stats_week.higheststreak) {
                this.accumulated_stats_week.higheststreak = this.stats_track.higheststreak;
            }

        }

        public void HelperShowSummary(RoundContext roundcontext, int attempts, int tracks_count, bool is_cleared) {
            string src = PVRContext.global_context.IsWidescreen() ? LAYOUT_WIDESCREEN : LAYOUT_DREAMCAST;
            Layout layout = Layout.Init(src);
            if (layout == null) return;

            if (is_cleared)
                layout.TriggerAny("week_not_cleared");
            else
                layout.TriggerAny("week_cleared");

            TextSprite textsprite1 = layout.GetTextsprite("stats");
            TextSprite textsprite2 = layout.GetTextsprite("stats2");
            float transition_delay = layout.GetAttachedValueAsFloat("transition_delay", 0f);

            if (tracks_count < 0) {
                // show the stats for the current (completed) track
                StatsTrack accumulated_stats = this.stats_track;

                if (textsprite1 != null)
                    textsprite1.SetTextFormated(WeekResult.TRACK_TEXT,
                        // judgements
                        accumulated_stats.sick,
                        accumulated_stats.good,
                        accumulated_stats.bads,

                        // streak
                        accumulated_stats.shits,
                        accumulated_stats.miss,
                        accumulated_stats.penalties,

                        accumulated_stats.combobreaks,
                        accumulated_stats.higheststreak,
                        accumulated_stats.notesperseconds
                    );
                if (textsprite2 != null)
                    textsprite2.SetTextFormated(WeekResult.TRACK_TEXT2,
                        // week resume
                        attempts,
                        accumulated_stats.score,
                        accumulated_stats.accuracy
                    );
            } else {
                StatsAccumulated accumulated_stats = this.accumulated_stats_week;
                double average_accuracy = accumulated_stats.accuracy / tracks_count;

                if (textsprite1 != null)
                    textsprite1.SetTextFormated(WeekResult.WEEK_TEXT,
                        // judgements
                        accumulated_stats.sick,
                        accumulated_stats.good,
                        accumulated_stats.bads,

                        // streak
                        accumulated_stats.shits,
                        accumulated_stats.miss,
                        accumulated_stats.penalties,

                        accumulated_stats.combobreaks,
                        accumulated_stats.higheststreak
                    );
                if (textsprite2 != null)
                    textsprite2.SetTextFormated(WeekResult.WEEK_TEXT2,
                        // week resume
                        attempts,
                        accumulated_stats.score,
                        average_accuracy
                    );
            }

            bool transition = false;
            Gamepad controller = new Gamepad(-1);
            controller.ClearButtons();

            while (true) {
                float elapsed = PVRContext.global_context.WaitReady();
                PVRContext.global_context.Reset();

                if (roundcontext.script != null) roundcontext.script.NotifyFrame(elapsed);

                roundcontext.layout.Animate(elapsed);
                roundcontext.layout.Draw(PVRContext.global_context);

                layout.Animate(elapsed);
                layout.Draw(PVRContext.global_context);

                if (transition) {
                    if (transition_delay > 0) {
                        transition_delay -= elapsed;
                        if (transition_delay < 0) layout.TriggerAny("transition");
                        continue;
                    }
                    if (layout.AnimationIsCompleted("transition_effect") > 0) break;
                } else if (controller.GetPressed() != GamepadButtons.NOTHING) {
                    transition = true;
                    layout.SetGroupVisibility("allstats", false);
                }
            }

            layout.Destroy();
        }


        private class StatsTrack {
            public int sick;
            public int good;
            public int bads;
            public int shits;
            public int miss;
            public int penalties;
            public long score;
            public double accuracy;
            public int notesperseconds;
            public int combobreaks;
            public int higheststreak;
        }

        private class StatsAccumulated {
            public int sick;
            public int good;
            public int bads;
            public int shits;
            public int miss;
            public int penalties;
            public long score;
            public double accuracy;
            public int notesperseconds;
            public int combobreaks;
            public int higheststreak;
        }
    }

}

