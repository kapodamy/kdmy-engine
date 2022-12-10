#include "commons.h"
#include "healthwatcher.h"

bool healthwatcher_add_opponent(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover, bool can_die) {
    print_stub("healthwatcher_add_opponent", "healthwatcher=%p playerstats=%p can_recover=(bool)%i can_die=(bool)%i", healthwatcher, playerstats, can_recover, can_die);
    return 0;
}
bool healthwatcher_add_player(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover, bool can_die) {
    print_stub("healthwatcher_add_player", "healthwatcher=%p playerstats=%p can_recover=(bool)%i can_die=(bool)%i", healthwatcher, playerstats, can_recover, can_die);
    return 0;
}
int32_t healthwatcher_has_deads(HealthWatcher healthwatcher, bool in_players_or_opponents) {
    print_stub("healthwatcher_has_deads", "healthwatcher=%p in_players_or_opponents=(bool)%i", healthwatcher, in_players_or_opponents);
    return 0;
}
bool healthwatcher_enable_dead(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_die) {
    print_stub("healthwatcher_enable_dead", "healthwatcher=%p playerstats=%p can_die=(bool)%i", healthwatcher, playerstats, can_die);
    return 0;
}
bool healthwatcher_enable_recover(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover) {
    print_stub("healthwatcher_enable_recover", "healthwatcher=%p playerstats=%p can_recover=(bool)%i", healthwatcher, playerstats, can_recover);
    return 0;
}
void healthwatcher_clear(HealthWatcher healthwatcher) {
    print_stub("healthwatcher_clear", "healthwatcher=%p", healthwatcher);
}
void healthwatcher_balance(HealthWatcher healthwatcher, HealthBar healthbar) {
    print_stub("healthwatcher_balance", "healthwatcher=%p healthbar=%p", healthwatcher, healthbar);
}
void healthwatcher_reset_opponents(HealthWatcher healthwatcher) {
    print_stub("healthwatcher_reset_opponents", "healthwatcher=%p", healthwatcher);
}
