#ifndef _healthwatcher_h
#define _healthwatcher_h

#include <stdbool.h>
#include <stdint.h>

#include "playerstats_types.h"

//
//  forward reference
//

typedef struct Healthbar_s* Healthbar;


typedef struct HealthWatcher_s* HealthWatcher;


HealthWatcher healthwatcher_init();
void healthwatcher_destroy(HealthWatcher* healthwatcher);

bool healthwatcher_add_opponent(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover, bool can_die);
bool healthwatcher_add_player(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover, bool can_die);

int32_t healthwatcher_has_deads(HealthWatcher healthwatcher, bool in_players_or_opponents);

bool healthwatcher_enable_dead(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_die);
bool healthwatcher_enable_recover(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover);
void healthwatcher_clear(HealthWatcher healthwatcher);
void healthwatcher_balance(HealthWatcher healthwatcher, Healthbar healthbar);
void healthwatcher_reset_opponents(HealthWatcher healthwatcher);

#endif