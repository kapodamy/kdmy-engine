using Engine.Font;
using Engine.Platform;

namespace Engine.Game.Gameplay;


public struct WeekResult_Stats {
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

public class WeekResult {

    private const string WEEK_TEXT = "~judgements~\nSicks   $i\nGoods  $i\nBads   $i\n" +
                                    "\n~failures~\nShits      $i\nMissses    $i\nPenalties  $i" +
                                    "\n\n~streak~\nCombo breaks   $i\nHighest combo  $i";
    private const string WEEK_TEXT2 = "~week resume~\n" +
                                       "Total attempts $i\nTotal score    $l\nAverage accuracy $3d%";

    private const string SONG_TEXT = "~judgements~\nSicks  $i\nGoods  $i\nBads   $i\n" +
                                        "\n~failures~\nShits      $i\nMissses    $i\nPenalties  $i" +
                                        "\n\n~streak~\nCombo breaks   $i\nHighest combo  $i\nHighest nps    $i";
    private const string SONG_TEXT2 = "~song resume~\nAttempts $i\nScore    $l\nAvg. accuracy $3d%";

    private const string LAYOUT_WIDESCREEN = "/assets/common/image/week-round/results.xml";
    private const string LAYOUT_DREAMCAST = "/assets/common/image/week-round/results~dreamcast.xml";
    private const string MODDING_SCRIPT = "/assets/common/data/scripts/weekresult.lua";

    private StatsSong stats_song;
    private StatsAccumulated accumulated_stats_week;
    private Layout active_layout;


    public WeekResult() {

        this.stats_song = new StatsSong() {
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
        // Note: do not dispose the "active_layout" field

        //free(this);
    }

    public void AddStats(RoundContext roundcontext) {
        for (int i = 0 ; i < roundcontext.players_size ; i++) {
            if (roundcontext.players[i].type != CharacterType.PLAYER) continue;
            if (roundcontext.players[i].is_opponent) continue;
            PlayerStats playerstats = roundcontext.players[i].playerstats;

            this.stats_song.sick = playerstats.GetSicks();
            this.stats_song.good = playerstats.GetGoods();
            this.stats_song.bads = playerstats.GetBads();
            this.stats_song.shits = playerstats.GetShits();
            this.stats_song.miss = playerstats.GetMisses();
            this.stats_song.penalties = playerstats.GetPenalties();
            this.stats_song.score = playerstats.GetScore();
            this.stats_song.accuracy = playerstats.GetAccuracy();
            this.stats_song.notesperseconds = playerstats.GetNotesPerSecondsHighest();
            this.stats_song.combobreaks = playerstats.GetComboBreaks();

            int higheststreak = playerstats.GetHighestComboStreak();
            if (higheststreak > this.stats_song.higheststreak) {
                this.stats_song.higheststreak = higheststreak;
            }
        }

        this.accumulated_stats_week.sick += this.stats_song.sick;
        this.accumulated_stats_week.good += this.stats_song.good;
        this.accumulated_stats_week.bads += this.stats_song.bads;
        this.accumulated_stats_week.shits += this.stats_song.shits;
        this.accumulated_stats_week.miss += this.stats_song.miss;
        this.accumulated_stats_week.penalties += this.stats_song.penalties;
        this.accumulated_stats_week.score += this.stats_song.score;
        this.accumulated_stats_week.accuracy += this.stats_song.accuracy;
        this.accumulated_stats_week.notesperseconds += this.stats_song.notesperseconds;
        this.accumulated_stats_week.combobreaks += this.stats_song.combobreaks;

        if (this.stats_song.higheststreak > this.accumulated_stats_week.higheststreak) {
            this.accumulated_stats_week.higheststreak = this.stats_song.higheststreak;
        }

    }

    public void HelperShowSummary(RoundContext roundcontext, int attempts, int songs_count, bool freeplay, bool reject) {
        string src = PVRContext.global_context.IsWidescreen() ? LAYOUT_WIDESCREEN : LAYOUT_DREAMCAST;
        Layout layout = Layout.Init(src);
        if (layout == null) return;

        this.active_layout = layout;

        if (roundcontext.script != null) {
            roundcontext.script.NotifyBeforeresults();
            Week.Halt(roundcontext, true);
        }

        Modding modding = new Modding(layout, WeekResult.MODDING_SCRIPT);
        modding.has_exit = false;
        modding.has_halt = false;
        modding.native_menu = null;
        modding.HelperNotifyInit(Modding.NATIVE_MENU_SCREEN);

        if (reject) {
            layout.TriggerAny("week_not_cleared");
            modding.HelperNotifyEvent("week_not_cleared");
        } else if (freeplay) {
            layout.TriggerAny("song_cleared");
            modding.HelperNotifyEvent("song_cleared");
        } else {
            layout.TriggerAny("week_cleared");
            modding.HelperNotifyEvent("week_cleared");
        }

        TextSprite textsprite1 = layout.GetTextsprite("stats");
        TextSprite textsprite2 = layout.GetTextsprite("stats2");
        float transition_delay = layout.GetAttachedValueAsFloat("transition_delay", 0f);

        if (songs_count < 0) {
            // show the stats for the current (completed) song
            StatsSong accumulated_stats = this.stats_song;

            if (textsprite1 != null)
                textsprite1.SetTextFormated(WeekResult.SONG_TEXT,
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
                textsprite2.SetTextFormated(WeekResult.SONG_TEXT2,
                    // week resume
                    attempts,
                    accumulated_stats.score,
                    accumulated_stats.accuracy
                );
        } else {
            StatsAccumulated accumulated_stats = this.accumulated_stats_week;
            double average_accuracy = accumulated_stats.accuracy / songs_count;

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

            if (layout != null) {
                roundcontext.layout.Animate(elapsed);
                roundcontext.layout.Draw(PVRContext.global_context);
            }

            layout.Animate(elapsed);
            layout.Draw(PVRContext.global_context);

            ModdingHelperResult res = modding.HelperHandleCustomMenu(controller, elapsed);
            if (res != ModdingHelperResult.CONTINUE || modding.has_exit) break;
            if (modding.has_halt) continue;

            if (roundcontext.script != null && roundcontext.scriptcontext.halt_flag) {
                controller.ClearButtons();
                continue;
            }

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

        modding.HelperNotifyExit2();
        modding.has_funkinsave_changes = false;// ignore funkinsave changes
        modding.Destroy();

        this.active_layout = null;

        controller.Destroy();
        layout.Destroy();
    }

    public Layout GetLayout() {
        return this.active_layout;
    }

    public void GetAccumulatedStats(ref WeekResult_Stats stats) {
        stats.sick = this.accumulated_stats_week.sick;
        stats.good = this.accumulated_stats_week.good;
        stats.bads = this.accumulated_stats_week.bads;
        stats.shits = this.accumulated_stats_week.shits;
        stats.miss = this.accumulated_stats_week.miss;
        stats.penalties = this.accumulated_stats_week.penalties;
        stats.score = this.accumulated_stats_week.score;
        stats.accuracy = this.accumulated_stats_week.accuracy;
        stats.notesperseconds = this.accumulated_stats_week.notesperseconds;
        stats.combobreaks = this.accumulated_stats_week.combobreaks;
        stats.higheststreak = this.accumulated_stats_week.higheststreak;
    }


    private class StatsSong {
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

