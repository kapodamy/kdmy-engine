#ifndef _funkin_h
#define FUNKIN__funkin_h

#include "game/funkin/playerstats.h"
#include "game/common/weekenumerator.h"


#define FUNKIN_FUNKY "Friday--Night--Funkin"

#define FUNKIN_SCREEN_RESOLUTION_WIDTH 1280
#define FUNKIN_SCREEN_RESOLUTION_HEIGHT 720
#define FUNKIN_BACKGROUND_MUSIC "/assets/common/music/freakyMenu.ogg"

#define FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE 24.0f


#define FUNKIN_COMBO_STREAK_VISIBLE_AFTER 10

#define FUNKIN_HEALTH_BASE 1.0


//
// Note:
//      values are expressed in percent, and indicates the minimum precission
//      required to apply the desired ranking
//
#define FUNKIN_RANKING_SHIT 0.00f
#define FUNKIN_RANKING_BAD 0.25f
#define FUNKIN_RANKING_GOOD 0.50f
#define FUNKIN_RANKING_SICK 0.75f

//
// Notes:
//      - amount of points to add/remove for the desires accuracy
//      - penality means pressing a key without the corresponding arrow
//      - values taken from "https://fridaynightfunkin.fandom.com/wiki/Friday_Night_Funkin'#Score"
//      - in some mods "penality" and "miss" values can be different
//
#define FUNKIN_SCORE_PENALITY -10
#define FUNKIN_SCORE_MISS 0
#define FUNKIN_SCORE_SHIT 50
#define FUNKIN_SCORE_BAD 100
#define FUNKIN_SCORE_GOOD 200
#define FUNKIN_SCORE_SICK 350

//
// Notes:
//      - this indicates how much health the player will loose or gain
//      - expressed in percent (100 health units or 100% or 1.0)
//      - Funkin and Kade engine sourcecode are illegible so i have to "reinvent the wheel"
//
#define FUNKIN_HEALTH_DIFF_OVER_PENALITY -0.01
#define FUNKIN_HEALTH_DIFF_OVER_MISS -0.2
#define FUNKIN_HEALTH_DIFF_OVER_SHIT -0.01
#define FUNKIN_HEALTH_DIFF_OVER_BAD 0.00
#define FUNKIN_HEALTH_DIFF_OVER_GOOD 0.02
#define FUNKIN_HEALTH_DIFF_OVER_SICK 0.04
#define FUNKIN_HEALTH_DIFF_OVER_SUSTAIN 0.01 // applies to hold (gain), release (loose) and recover (gain)


#define FUNKIN_DIFFICULT_EASY "EASY"
#define FUNKIN_DIFFICULT_NORMAL "NORMAL"
#define FUNKIN_DIFFICULT_HARD "HARD"




#define FUNKIN_WEEKS_FOLDER "/assets/weeks/"
#define FUNKIN_WEEK_ABOUT_FILE "about.json"
#define FUNKIN_WEEK_GREETINGS_FILE "weekGreetings.txt"
#define FUNKIN_WEEK_FOLDER_SEPARATOR "/"
#define FUNKIN_WEEK_HOST_FILENAME "host"
#define FUNKIN_WEEK_TITLE_FILENAME "title.png"

// #define FUNKIN_WEEK_ABOUT_FOLDER "/about/"
// #define FUNKIN_WEEK_CUSTOM_FOLDER "custom"
#define FUNKIN_WEEK_GAMEPLAY_MANIFEST "gameplay.json"
#define FUNKIN_WEEK_SONGS_FOLDER "songs/"
#define FUNKIN_WEEK_CHARTS_FOLDER "charts/"

// Chart scroll velocity (do not confuse with the song's chart speed).
// In Funkin, Kade Engine and forks this velocity is 450px/s (pixel per seconds) using
// a render resolution of 1280x720. But here is expressed in pixel per milliseconds for convenience.
#define FUNKIN_CHART_SCROLL_VELOCITY (450.0 / 1000.0)

// Minimal sustain duration for a note, if the duration does not reach
// the threshold, the sustain hit check will be ignored.
// This value is a ratio based on the arrow height
#define FUNKIN_NOTE_MIN_SUSTAIN_THRESHOLD 1.10


#define FUNKIN_COMMON_NOTES_SPLASH "/assets/common/image/week-round/noteSplashes.xml"
#define FUNKIN_COMMON_NOTES "/assets/common/image/week-round/notes.xml"
#define FUNKIN_COMMON_STRUM_BACKGROUND "/assets/common/image/week-round/strumBackground.xml" // STUB

#define FUNKIN_GIRLFRIEND_COMBOBREAK "sad"
#define FUNKIN_PLAYER_DIES "dies"         // the player has lost
#define FUNKIN_PLAYER_SCARED "scared"     // the other players reacts
#define FUNKIN_OPPONENT_VICTORY "victory" // the opponents "laughs"
#define FUNKIN_PLAYER_RETRY "retry"
#define FUNKIN_PLAYER_GIVEUP "giveup"

#define FUNKIN_MARKER_DURATION (1000.0f / 6.0f)

#define FUNKIN_LOADING_SCREEN_TEXTURE "/assets/common/image/funkin/funkay.png"

#define FUNKIN_NO_ACCURACY "N/A"


extern WeekArray weeks_array;


const char* funkin_get_letter_rank(PlayerStats playerstats);
const char* funkin_get_wife3_accuracy(PlayerStats playerstats);

#endif
