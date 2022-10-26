
namespace Engine.Game.Common {

    public static class Funkin {
        
        /**
         * Contains all weeks definitions found
         */
        public static WeekArray weeks_array;

        public const string FUNKY = "Friday--Night--Funkin";

        public const int SCREEN_RESOLUTION_WIDTH = 1280;
        public const int SCREEN_RESOLUTION_HEIGHT = 720;
        public const string BACKGROUND_MUSIC = "/assets/common/music/freakyMenu.ogg";

        public const float DEFAULT_ANIMATIONS_FRAMERATE = 24;


        public const int COMBO_STREAK_VISIBLE_AFTER = 10;

        public const double HEALTH_BASE = 1.0;


        //
        // Note:
        //      values are expressed in percent, and indicates the minimum precission
        //      required to apply the desired ranking
        //
        public const float RANKING_SHIT = 0.00f;
        public const float RANKING_BAD = 0.25f;
        public const float RANKING_GOOD = 0.50f;
        public const float RANKING_SICK = 0.75f;

        //
        // Notes:
        //      - amount of points to add/remove for the desires accuracy
        //      - penality means pressing a key without the corresponding arrow 
        //      - values taken from "https://fridaynightfunkin.fandom.com/wiki/Friday_Night_Funkin'#Score"
        //      - in some mods "penality" and "miss" values can be different
        //
        public const int SCORE_PENALITY = -10;
        public const int SCORE_MISS = 0;
        public const int SCORE_SHIT = 50;
        public const int SCORE_BAD = 100;
        public const int SCORE_GOOD = 200;
        public const int SCORE_SICK = 350;

        //
        // Notes:
        //      - this indicates how much health the player will loose or gain
        //      - expressed in percent (100 health units or 100% or 1.0)
        //      - Funkin and Kade engine sourcecode are illegible so i have to "reinvent the wheel"
        //
        public const double HEALTH_DIFF_OVER_PENALITY = -0.01;
        public const double HEALTH_DIFF_OVER_MISS = -0.2;
        public const double HEALTH_DIFF_OVER_SHIT = -0.01;
        public const double HEALTH_DIFF_OVER_BAD = 0.00;
        public const double HEALTH_DIFF_OVER_GOOD = 0.02;
        public const double HEALTH_DIFF_OVER_SICK = 0.04;
        public const double HEALTH_DIFF_OVER_SUSTAIN = 0.01;// applies to hold (gain), release (loose) and recover (gain)


        public const string DIFFICULT_EASY = "EASY";
        public const string DIFFICULT_NORMAL = "NORMAL";
        public const string DIFFICULT_HARD = "HARD";




        public const string WEEKS_FOLDER = "/assets/weeks/";
        public const string WEEK_ABOUT_FILE = "about.json";
        public const string WEEK_GREETINGS_FILE = "weekGreetings.txt";
        public const string WEEK_FOLDER_SEPARATOR = "/";
        public const string WEEK_HOST_FILENAME = "host";
        public const string WEEK_TITLE_FILENAME = "title.png";

        //public const string WEEK_ABOUT_FOLDER = "/about/";
        //public const string WEEK_CUSTOM_FOLDER = "custom";
        public const string WEEK_GAMEPLAY_MANIFEST = "gameplay.json";
        public const string WEEK_SONGS_FOLDER = "songs/";
        public const string WEEK_CHARTS_FOLDER = "charts/";

        /// <summary>
        /// Chart scroll velocity (do not confuse with the song's chart speed).
        /// In Funkin, Kade Engine and forks this velocity is 450px/s (pixel per seconds) using
        /// a render resolution of 1280x720. But here is expressed in pixel per milliseconds for convenience.
        /// </summary>
        public const double CHART_SCROLL_VELOCITY = 450.0 / 1000.0;

        /// <summary>
        /// Minimal sustain duration for a note, if the duration does not reach
        /// the threshold, the sustain hit check will be ignored.
        /// This value is a ratio based on the arrow height
        /// </summary>
        public const double NOTE_MIN_SUSTAIN_THRESHOLD = 1.10;


        public const string COMMON_NOTES_SPLASH = "/assets/common/image/week-round/noteSplashes.xml";
        public const string COMMON_NOTES = "/assets/common/image/week-round/notes.xml";
        public const string COMMON_STRUM_BACKGROUND = "/assets/common/image/week-round/strumBackground.xml";// STUB

        public const string GIRLFRIEND_COMBOBREAK = "sad";
        public const string PLAYER_DIES = "dies";// the player has lost
        public const string PLAYER_SCARED = "scared";// the other players reacts
        public const string OPPONENT_VICTORY = "victory";// the opponents "laughs"
        public const string PLAYER_RETRY = "retry";
        public const string PLAYER_GIVEUP = "giveup";

        public const float MARKER_DURATION = 1000f / 6f;


        ////////////////////////////////////
        // Imported from Kade Engine v1.6.1
        /////////////////////////////////////

        public static readonly string NO_ACCURACY = "N/A";
        public static readonly double[] WIFE3_ACCURACIES = new double[] {
            99.9935, 99.980, 99.970, 99.955, 99.90, 99.80, 99.70, 99, 96.50, 93, 90, 85,
            80, 70, 60, 60
        };
        public static readonly string[] WIFE3_RANKS = new string[] {
            "AAAAA", "AAAA:", "AAAA.", "AAAA", "AAA:", "AAA.", "AAA", "AA:", "AA.", "AA", "A:", "A.",
            "A", "B", "C", "D"
        };

        public const string RANK_MARVELOUS_FULL_COMBO = "(MFC)";
        public const string RANK_GOOD_FULL_COMBO = "(GFC)";
        public const string RANK_FULL_COMBO = "(FC)";
        public const string RANK_SINGLE_DIGIT_COMBO_BREAKS = "(SDCB)";
        public const string CLEAR = "(Clear)";


        public static string GetLetterRank(PlayerStats playerstats) {
            int miss_count = playerstats.GetMisses();
            int shit_count = playerstats.GetShits();
            int bad_count = playerstats.GetBads();
            int good_count = playerstats.GetGoods();
            //int sicks = playerstats.GetSicks();

            bool has_no_misses_shits_bads = miss_count == 0 && shit_count == 0 && bad_count == 0;

            if (has_no_misses_shits_bads && good_count == 0)
                return Funkin.RANK_MARVELOUS_FULL_COMBO;
            else if (has_no_misses_shits_bads && good_count >= 1)
                return Funkin.RANK_GOOD_FULL_COMBO;
            else if (miss_count == 0)
                return Funkin.RANK_FULL_COMBO;
            else if (miss_count < 10)
                return Funkin.RANK_SINGLE_DIGIT_COMBO_BREAKS;
            else
                return Funkin.CLEAR;
        }

        public static string GetWife3Accuracy(PlayerStats playerstats) {
            // elegant way to calc wife3 accuracy
            double accuracy = playerstats.GetAccuracy();
            int last_rank = Funkin.WIFE3_ACCURACIES.Length - 1;

            for (int i = 0 ; i < last_rank ; i++) {
                if (accuracy >= Funkin.WIFE3_ACCURACIES[i]) return Funkin.WIFE3_RANKS[i];
            }

            // accuracy < 60
            return Funkin.WIFE3_RANKS[last_rank];
        }

    }

}
