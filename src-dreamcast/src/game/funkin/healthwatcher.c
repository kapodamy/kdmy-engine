#include "game/funkin/healthwatcher.h"

#include "arraylist.h"
#include "externals/luascript.h"
#include "game/funkin/healthbar.h"
#include "game/funkin/playerstats.h"
#include "malloc_utils.h"


typedef struct {
    PlayerStats playerstats;
    bool can_die;
} CharacterInfo;

struct HealthWatcher_s {
    ArrayList /*<CharacterInfo>*/ players;
    ArrayList /*<CharacterInfo>*/ opponents;
};


static bool healthwatcher_internal_add(ArrayList arraylist1, ArrayList arraylist2, PlayerStats playerstats, bool can_recover, bool can_die);
static CharacterInfo* healthwatcher_internal_get_character(HealthWatcher healthwatcher, PlayerStats playerstats);


HealthWatcher healthwatcher_init() {
    HealthWatcher healthwatcher = malloc_chk(sizeof(struct HealthWatcher_s));
    malloc_assert(healthwatcher, HealthWatcher);

    *healthwatcher = (struct HealthWatcher_s){
        .players = arraylist_init2(sizeof(CharacterInfo), 1),
        .opponents = arraylist_init2(sizeof(CharacterInfo), 1),
    };

    return healthwatcher;
}

void healthwatcher_destroy(HealthWatcher* healthwatcher_ptr) {
    if (!healthwatcher_ptr || !*healthwatcher_ptr) return;

    HealthWatcher healthwatcher = *healthwatcher_ptr;

    luascript_drop_shared(healthwatcher);

    arraylist_destroy(&healthwatcher->players);
    arraylist_destroy(&healthwatcher->opponents);

    free_chk(healthwatcher);
    healthwatcher_ptr = NULL;
}


bool healthwatcher_add_opponent(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover, bool can_die) {
    bool success = healthwatcher_internal_add(
        healthwatcher->opponents, healthwatcher->players, playerstats, can_recover, can_die
    );
    if (success) {
        playerstats_set_health(playerstats, 0.0);
    }
    return success;
}

bool healthwatcher_add_player(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover, bool can_die) {
    bool success = healthwatcher_internal_add(
        healthwatcher->players, healthwatcher->opponents, playerstats, can_recover, can_die
    );
    if (success) {
        playerstats_set_health(playerstats, playerstats_get_maximum_health(playerstats) / 2.0);
    }
    return success;
}


int32_t healthwatcher_has_deads(HealthWatcher healthwatcher, bool in_players_or_opponents) {
    ArrayList arraylist = in_players_or_opponents ? healthwatcher->players : healthwatcher->opponents;
    int32_t deads = 0;

    foreach (CharacterInfo*, character, ARRAYLIST_ITERATOR, arraylist) {
        if (playerstats_is_dead(character->playerstats)) deads++;
    }

    return deads;
}


bool healthwatcher_enable_dead(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_die) {
    CharacterInfo* character = healthwatcher_internal_get_character(healthwatcher, playerstats);
    if (!character) return false;

    character->can_die = can_die;
    return true;
}

bool healthwatcher_enable_recover(HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover) {
    CharacterInfo* character = healthwatcher_internal_get_character(healthwatcher, playerstats);
    if (!character) return false;
    playerstats_enable_health_recover(playerstats, can_recover);
    return true;
}

void healthwatcher_clear(HealthWatcher healthwatcher) {
    arraylist_clear(healthwatcher->opponents);
    arraylist_clear(healthwatcher->players);
}

void healthwatcher_balance(HealthWatcher healthwatcher, Healthbar healthbar) {
    float64 opponents_total = 0.0;
    float64 accumulated = 0.0;
    float64 maximum = 0.0;
    int32_t players_count = arraylist_size(healthwatcher->players);

    foreach (CharacterInfo*, character, ARRAYLIST_ITERATOR, healthwatcher->opponents) {
        float64 health = playerstats_get_health(character->playerstats);
        if (health > 0.0) opponents_total += health;

        if (character->can_die && health < 0.0)
            playerstats_kill(character->playerstats);
        else if (players_count > 0.0)
            playerstats_set_health(character->playerstats, 0.0);

        if (players_count < 1) maximum += playerstats_get_maximum_health(character->playerstats);
    }

    if (players_count > 0) {
        float64 amount = (-opponents_total) / players_count;

        foreach (CharacterInfo*, character, ARRAYLIST_ITERATOR, healthwatcher->players) {
            if (amount != 0.0)
                playerstats_add_health(character->playerstats, amount, character->can_die);

            if (character->can_die)
                playerstats_kill_if_negative_health(character->playerstats);
            else
                playerstats_raise(character->playerstats, false);

            accumulated += playerstats_get_health(character->playerstats);
            maximum += playerstats_get_maximum_health(character->playerstats);
        }
    }

    if (healthbar) {
        bool opponents_recover = opponents_total > 0;

        // Note: originally was "if (players_count < 0)" change if something breaks
        if (players_count < 1) opponents_recover = !opponents_recover;

        healthbar_set_health_position(healthbar, (float)maximum, (float)accumulated, opponents_recover);
    }
}

void healthwatcher_reset_opponents(HealthWatcher healthwatcher) {
    foreach (CharacterInfo*, character, ARRAYLIST_ITERATOR, healthwatcher->opponents) {
        playerstats_set_health(character->playerstats, 0.0);
    }
}




static bool healthwatcher_internal_add(ArrayList arraylist1, ArrayList arraylist2, PlayerStats playerstats, bool can_recover, bool can_die) {
    foreach (CharacterInfo*, character, ARRAYLIST_ITERATOR, arraylist1) {
        if (character->playerstats == playerstats) return false;
    }
    foreach (CharacterInfo*, character, ARRAYLIST_ITERATOR, arraylist2) {
        if (character->playerstats == playerstats) return false;
    }
    arraylist_add(arraylist1, &(CharacterInfo){.playerstats = playerstats, .can_die = can_die});
    playerstats_enable_health_recover(playerstats, can_recover);
    return true;
}

static CharacterInfo* healthwatcher_internal_get_character(HealthWatcher healthwatcher, PlayerStats playerstats) {
    foreach (CharacterInfo*, character, ARRAYLIST_ITERATOR, healthwatcher->players) {
        if (character->playerstats == playerstats) return character;
    }
    foreach (CharacterInfo*, character, ARRAYLIST_ITERATOR, healthwatcher->opponents) {
        if (character->playerstats == playerstats) return character;
    }
    return NULL;
}

/*static void healthwatcher_internal_calculate_totals(ArrayList arraylist, float64* totals_maximum, float64* totals_accumulated) {
    foreach (CharacterInfo*, character, ARRAYLIST_ITERATOR, arraylist) {
        *totals_maximum += playerstats_get_maximum_health(character->playerstats);

        if (playerstats_is_dead(character->playerstats)) {
            if (!character->can_die) playerstats_raise(character->playerstats, false);
        } else {
            *totals_accumulated += playerstats_get_health(character->playerstats);
        }
    }
}*/

/*static void healthwatcher_internal_calculate_balance(ArrayList arraylist, float64 difference) {
    int32_t size = arraylist_size(arraylist);
    if (size < 1) return;

    float64 amount = difference / size;
    amount = -amount;

    foreach (CharacterInfo*, character, ARRAYLIST_ITERATOR, arraylist) {
        float64 maximum = playerstats_get_maximum_health(character->playerstats);
        float64 health = playerstats_get_health(character->playerstats) + amount;

        if (health < 0.0) {
            if (character->can_die) {
                playerstats_kill(character->playerstats);
            } else {
                health = 0.0;
            }
        } else if (health > maximum) {
            health = maximum;
        }

        playerstats_set_health(character->playerstats, health);
    }
}*/
