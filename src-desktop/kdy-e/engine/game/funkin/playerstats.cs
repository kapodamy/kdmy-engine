using System;
using Engine.Game.Common;
using Engine.Utils;

namespace Engine.Game {

    public enum Ranking : int {
        NONE = 0,
        SICK,
        GOOD,
        BAD,
        SHIT,
        MISS,
        PENALITY
    }


    public class PlayerStats {

        private int ranking_sick;
        private int ranking_good;
        private int ranking_bad;
        private int ranking_shit;
        private long score;
        private double accuracy;
        private double health;
        private double health_max;
        private bool can_recover;
        private int iterations;
        private int hits;
        private int miss;
        private int penalties;
        private int streak;
        private int breaks;
        private int highest_streak;
        private int nps_current;
        private int nps_highest;
        internal Ranking last_ranking;
        private double last_diff;
        private double accuracy_accumulator;
        private double accuracy_counter;
        private double accuracy_counter_last;
        private bool penalize_on_empty_strum;
        private int deads_by_fault;
        private double last_accuracy;

        public PlayerStats() {

            this.ranking_sick = 0;
            this.ranking_good = 0;
            this.ranking_bad = 0;
            this.ranking_shit = 0;

            this.score = 0;
            this.accuracy = 0;

            this.health = 0.5;
            this.health_max = Funkin.HEALTH_BASE;
            this.can_recover = true;

            this.iterations = 0;// amount of hit/miss/penalties added

            this.hits = 0;// notes hits
            this.miss = 0;// notes misses
            this.penalties = 0;// on empty strum or pressing the wrong button

            this.streak = 0;// combo streak
            this.breaks = 0;// combo breaks count
            this.highest_streak = 0;// highest combo streak

            this.nps_current = 0;// notes per second
            this.nps_highest = 0;// notes per seconds (highest value)

            this.last_ranking = Ranking.NONE;// last ranking added and/or calculated
            this.last_diff = Single.NaN;// last note hit diference (signed value in milliseconds)
            this.last_accuracy = Double.NaN;// last note hit accuracy

            // internal use only
            this.accuracy_accumulator = 0;
            this.accuracy_counter = 0;
            this.accuracy_counter_last = 0;

            this.penalize_on_empty_strum = true;


            this.deads_by_fault = 0;// fault registered with negative health

        }

        public void Destroy() {
            //free(playerstats);
        }


        public Ranking AddHit(float multiplier, float base_note_duration, double hit_time_difference) {
            double hit_accuracy = 1.0 - Math.Abs(hit_time_difference / base_note_duration);
            double health_gain;

            this.last_diff = hit_time_difference;
            this.streak++;
            this.hits++;
            this.nps_current++;
            this.iterations++;

            if (this.nps_current > this.nps_highest)
                this.nps_highest = this.nps_current;

            if (hit_accuracy >= Funkin.RANKING_SICK) {
                this.ranking_sick++;
                this.score += Funkin.SCORE_SICK;
                health_gain = Funkin.HEALTH_DIFF_OVER_SICK;
                this.last_ranking = Ranking.SICK;
            } else if (hit_accuracy >= Funkin.RANKING_GOOD) {
                this.ranking_good++;
                this.score += Funkin.SCORE_GOOD;
                health_gain = Funkin.HEALTH_DIFF_OVER_GOOD;
                this.last_ranking = Ranking.GOOD;
            } else if (hit_accuracy >= Funkin.RANKING_BAD) {
                this.ranking_bad++;
                this.score += Funkin.SCORE_BAD;
                health_gain = Funkin.HEALTH_DIFF_OVER_BAD;
                this.last_ranking = Ranking.BAD;
            } else if (hit_accuracy >= Funkin.RANKING_SHIT) {
                this.ranking_shit++;
                this.score += Funkin.SCORE_SHIT;
                health_gain = Funkin.HEALTH_DIFF_OVER_SHIT;
                this.last_ranking = Ranking.SHIT;
            } else {
                // this never should happen (hit_accuracy < zero)
                return this.last_ranking;
            }

            health_gain *= multiplier;
            if (this.can_recover || (health_gain > 0.0 && !this.can_recover)) {
                double health = health_gain + this.health;
                if (this.health < 0.0 && multiplier < 0) this.deads_by_fault++;
                this.health = Math.Min(health, this.health_max);
                if (this.health > 0.0 && this.deads_by_fault > 0) this.deads_by_fault = 0;
            }

            this.accuracy_accumulator += hit_accuracy;
            this.accuracy_counter++;
            return this.last_ranking;
        }

        public void AddSustain(int quarters, bool is_released) {
            if (Single.IsNaN(quarters)) throw new NaNArgumentError("quarters is NaN");

            if (is_released) {
                this.last_ranking = Ranking.MISS;
                quarters = -quarters;
                if (this.last_ranking != Ranking.MISS) this.iterations++;
            } else {
                switch (this.last_ranking) {
                    case Ranking.MISS:
                    case Ranking.PENALITY:
                        this.last_ranking = Ranking.GOOD;
                        this.iterations++;
                        break;
                }
                if (!this.can_recover && quarters > 0.0) return;

            }

            double health = this.health + (quarters * Funkin.HEALTH_DIFF_OVER_SUSTAIN);
            this.health = Math.Min(health, this.health_max);

            if (this.health < 0.0)
                this.deads_by_fault++;
            else if (this.health > 0.0 && this.deads_by_fault > 0)
                this.deads_by_fault = 0;

            this.score += (int)(quarters * Funkin.SCORE_MISS);
        }

        public Ranking AddSustainDelayedHit(float multiplier, double hit_time_difference) {
            this.last_accuracy = 0.0;
            this.last_ranking = Ranking.SHIT;
            this.last_diff = hit_time_difference;
            this.hits++;
            this.streak++;
            this.nps_current++;
            this.iterations++;

            double health_gain = Funkin.HEALTH_DIFF_OVER_SHIT * multiplier;
            if (this.can_recover || (health_gain > 0.0 && !this.can_recover)) {
                double health = health_gain + this.health;
                if (this.health < 0.0 && multiplier < 0) this.deads_by_fault++;
                this.health = Math.Min(health, this.health_max);
                if (this.health > 0.0 && this.deads_by_fault > 0) this.deads_by_fault = 0;
            }

            return this.last_ranking;
        }

        public void AddPenality(bool on_empty_strum) {
            if (!this.penalize_on_empty_strum && on_empty_strum) return;

            double health = Funkin.HEALTH_DIFF_OVER_PENALITY;
            if (!on_empty_strum) health *= 2;
            this.health += health;
            if (this.health < 0.0) this.deads_by_fault++;

            this.score += Funkin.SCORE_PENALITY;
            this.last_ranking = Ranking.PENALITY;
            this.iterations++;
            this.penalties++;

            if (this.streak > 0) {
                if (this.streak > this.highest_streak) {
                    this.highest_streak = this.streak;
                }
                this.streak = 0;
                this.breaks++;
            }

        }

        public void AddMiss(float multiplier) {
            //
            // FIXME: ¿should decrease the accumulated accuracy? ¿with what criteria? 
            //

            this.health += Funkin.HEALTH_DIFF_OVER_MISS * multiplier;

            if (this.health < 0.0 && multiplier > 0)
                this.deads_by_fault++;
            else if (this.health > 0.0 && multiplier < 0.0 && this.deads_by_fault > 0)
                this.deads_by_fault = 0;

            this.miss++;
            this.score += Funkin.SCORE_MISS;
            this.last_ranking = Ranking.MISS;
            this.iterations++;
        }


        public void Reset() {
            this.ranking_sick = 0;
            this.ranking_good = 0;
            this.ranking_bad = 0;
            this.ranking_shit = 0;
            this.score = 0;
            this.accuracy = 0;
            this.iterations = 0;
            this.hits = 0;
            this.miss = 0;
            this.penalties = 0;
            this.streak = 0;
            this.breaks = 0;
            this.highest_streak = 0;
            this.nps_current = 0;
            this.nps_highest = 0;
            this.accuracy_accumulator = 0;
            this.accuracy_counter = 0;
            this.accuracy_counter_last = 0;
            this.deads_by_fault = 0;

            //
            // Non-zero fields
            //
            this.last_ranking = Ranking.NONE;
            this.last_diff = Single.NaN;
            this.last_accuracy = Double.NaN;

            this.health = Funkin.HEALTH_BASE / 2f;
            this.health_max = Funkin.HEALTH_BASE;
        }

        public void ResetNotesPerSeconds() {
            this.nps_current = 0;
        }

        public void AddExtraHealth(float multiplier) {
            if (Single.IsNaN(multiplier)) throw new NaNArgumentError("multiplier is NaN");

            if (multiplier > 1)
                this.health_max = Funkin.HEALTH_BASE * multiplier;
            else
                this.health_max = Funkin.HEALTH_BASE;

            this.health = Math.Min(this.health, this.health_max);
        }


        public void EnablePenalityOnEmptyStrum(bool enable) {
            this.penalize_on_empty_strum = enable;
        }

        public void EnableHealthRecover(bool enable) {
            this.can_recover = enable;
        }


        public double GetMaximumHealth() {
            return this.health_max;
        }

        public double GetHealth() {
            return this.health;
        }

        public double GetAccuracy() {
            if (this.accuracy_counter_last != this.accuracy_counter) {
                this.accuracy = this.accuracy_accumulator / this.accuracy_counter;
                this.accuracy_counter_last = this.accuracy_counter;

                // for convenience express the result as hundred (100.0000000%)
                this.accuracy *= 100.0;
            }

            return this.accuracy;
        }

        public double GetLastAccuracy() {
            return this.last_accuracy;
        }

        public Ranking GetLastRanking() {
            return this.last_ranking;
        }

        public double GetLastDifference() {
            return this.last_diff;
        }

        public int GetComboStreak() {
            return this.streak;
        }

        public int GetHighestComboStreak() {
            return this.highest_streak;
        }

        public int GetComboBreaks() {
            return this.breaks;
        }

        public int GetNotesPerSeconds() {
            return this.nps_current;
        }

        public int GetNotesPerSecondsHighest() {
            return this.nps_highest;
        }

        public int GetIterations() {
            return this.iterations;
        }

        public long GetScore() {
            return this.score;
        }

        public int GetHits() {
            return this.hits;
        }

        public int GetMisses() {
            return this.miss;
        }

        public int GetPenalties() {
            return this.penalties;
        }

        public int GetShits() {
            return this.ranking_shit;
        }

        public int GetBads() {
            return this.ranking_bad;
        }

        public int GetGoods() {
            return this.ranking_good;
        }

        public int GetSicks() {
            return this.ranking_sick;
        }




        public void SetHealth(double health) {
            if (Double.IsNaN(health)) throw new NaNArgumentError("health");
            this.health = Math.Min(health, this.health_max);
            if (health >= 0.0) this.deads_by_fault = 0;
        }

        public double AddHealth(double health, bool die_if_negative) {
            if (Double.IsNaN(health)) throw new NaNArgumentError("health");

            health += this.health;

            if (!die_if_negative && health < 0.0)
                health = 0.0;
            else if (health > this.health_max)
                health = this.health_max;

            if (die_if_negative && health < 0.0) this.deads_by_fault++;

            return this.health = health;
        }


        public void Raise(bool with_full_health) {
            if (this.health < 0.0) this.health = with_full_health ? this.health_max : 0.0;
            this.deads_by_fault = 0;
        }

        public void Kill() {
            this.health = -1.0;
            this.deads_by_fault++;
        }

        public void KillIfNegativeHealth() {
            if (this.health < 0.0) {
                this.deads_by_fault++;
                this.health = 0.0;
            }
        }

        public bool IsDead() {
            return this.deads_by_fault > 0;
        }

    }

}
