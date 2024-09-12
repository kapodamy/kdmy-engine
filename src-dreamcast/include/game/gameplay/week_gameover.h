#ifndef _week_gameover_h
#define _week_gameover_h

#include "week_types.h"


#define WEEK_GAMEOVER_NOMUSIC 0
#define WEEK_GAMEOVER_NOSFXDIE 1
#define WEEK_GAMEOVER_NOSFXRETRY 2
#define WEEK_GAMEOVER_ANIMDURATIONDIE 3
#define WEEK_GAMEOVER_ANIMDURATIONRETRY 4
#define WEEK_GAMEOVER_ANIMDURATIONGIVEUP 5
#define WEEK_GAMEOVER_ANIMDURATIONBEFORE 6
#define WEEK_GAMEOVER_ANIMDURATIONBEFOREFORCEEND 7
#define WEEK_GAMEOVER_SETMUSIC 8
#define WEEK_GAMEOVER_SETSFXDIE 9
#define WEEK_GAMEOVER_SETSFXRETRY 10


typedef struct WeekGameOver_s* WeekGameOver;


WeekGameOver week_gameover_init();
void week_gameover_destroy(WeekGameOver* weekgameover_ptr);
uint32_t week_gameover_read_version();

void week_gameover_hide(WeekGameOver weekgameover);
void week_gameover_display(WeekGameOver weekgameover, float64 timestamp, float64 duration, PlayerStats playerstats, WeekInfo* weekinfo, const char* difficult);
void week_gameover_display_selector(WeekGameOver weekgameover, bool load, bool visible);
void week_gameover_display_choosen_difficult(WeekGameOver weekgameover);
Drawable week_gameover_get_drawable(WeekGameOver weekgameover);
const char* week_gameover_get_difficult(WeekGameOver weekgameover);
int32_t week_gameover_helper_ask_to_player(WeekGameOver weekgameover, RoundContext roundcontext);

void week_gameover_set_option(WeekGameOver weekgameover, int32_t option, float nro, const char* str);
Layout week_gameover_get_layout(WeekGameOver weekgameover);

#endif
