#ifndef __healthwatcher_h
#define __healthwatcher_h

#include "healthbar.h"
#include "playerstats.h"


typedef struct {
    int dummy;
} HealthWatcher_t;
typedef HealthWatcher_t* HealthWatcher;


bool healthwatcher_add_opponent(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover, bool can_die);
bool healthwatcher_add_player(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover, bool can_die);
int32_t healthwatcher_has_deads(HealthWatcher healthwatcher, bool in_players_or_opponents);
bool healthwatcher_enable_dead(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_die);
bool healthwatcher_enable_recover(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover);
void healthwatcher_clear(HealthWatcher healthwatcher);
void healthwatcher_balance(HealthWatcher healthwatcher, HealthBar healthbar);
void healthwatcher_reset_opponents(HealthWatcher healthwatcher);

#endif
