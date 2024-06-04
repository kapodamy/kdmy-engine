"use strict";

function healthwatcher_init() {
    return {
        players: arraylist_init2(1),
        opponents: arraylist_init2(1),
    };
}

function healthwatcher_destroy(healthwatcher) {
    luascript_drop_shared(healthwatcher);

    arraylist_destroy(healthwatcher.players, false);
    arraylist_destroy(healthwatcher.opponents, false);
    healthwatcher = undefined;
}


function healthwatcher_add_opponent(healthwatcher, playerstats, can_recover, can_die) {
    let success = healthwatcher_internal_add(
        healthwatcher.opponents, healthwatcher.players, playerstats, can_recover, can_die
    );
    if (success) {
        playerstats_set_health(playerstats, 0.0);
    }
    return success;
}

function healthwatcher_add_player(healthwatcher, playerstats, can_recover, can_die) {
    let success = healthwatcher_internal_add(
        healthwatcher.players, healthwatcher.opponents, playerstats, can_recover, can_die
    );
    if (success) {
        playerstats_set_health(playerstats, playerstats_get_maximum_health(playerstats) / 2.0);
    }
    return success;
}


function healthwatcher_has_deads(healthwatcher, in_players_or_opponents) {
    let arraylist = in_players_or_opponents ? healthwatcher.players : healthwatcher.opponents;
    let deads = 0;

    for (let character of arraylist_iterate4(arraylist)) {
        if (playerstats_is_dead(character.playerstats)) deads++;
    }

    return deads;
}


function healthwatcher_enable_dead(healthwatcher, playerstats, can_die) {
    let character = healthwatcher_internal_get_character(healthwatcher, playerstats);
    if (!character) return false;

    character.can_die = can_die;
    return true;
}

function healthwatcher_enable_recover(healthwatcher, playerstats, can_recover) {
    let character = healthwatcher_internal_get_character(healthwatcher, playerstats);
    if (!character) return false;
    playerstats_enable_health_recover(playerstats, can_recover);
    return true;
}

function healthwatcher_clear(healthwatcher) {
    arraylist_clear(healthwatcher.opponents);
    arraylist_clear(healthwatcher.players);
}

function healthwatcher_balance(healthwatcher, healthbar) {
    let opponents_total = 0.0;
    let accumulated = 0.0;
    let maximum = 0.0;
    let players_count = arraylist_size(healthwatcher.players);

    for (let character of arraylist_iterate4(healthwatcher.opponents)) {
        let health = playerstats_get_health(character.playerstats);
        if (health > 0.0) opponents_total += health;

        if (character.can_die && health < 0.0)
            playerstats_kill(character.playerstats);
        else if (players_count > 0)
            playerstats_set_health(character.playerstats, 0.0);

        if (players_count < 1) maximum += playerstats_get_maximum_health(character.playerstats);
    }

    if (players_count > 0) {
        let amount = (-opponents_total) / players_count;

        for (let character of arraylist_iterate4(healthwatcher.players)) {
            playerstats_add_health(character.playerstats, amount, character.can_die);

            if (character.can_die)
                playerstats_kill_if_negative_health(character.playerstats);
            else
                playerstats_raise(character.playerstats, false);

            accumulated += playerstats_get_health(character.playerstats);
            maximum += playerstats_get_maximum_health(character.playerstats);
        }
    }

    if (healthbar) {
        let opponents_recover = opponents_total > 0;

        // Note: originally was "if (players_count < 0)" change if something breaks
        if (players_count < 1) opponents_recover = !opponents_recover;

        healthbar_set_health_position(healthbar, maximum, accumulated, opponents_recover);
    }
}

function healthwatcher_reset_opponents(healthwatcher) {
    for (let character of arraylist_iterate4(healthwatcher.opponents)) {
        playerstats_set_health(character.playerstats, 0.0);
    }
}




function healthwatcher_internal_add(arraylist1, arraylist2, playerstats, can_recover, can_die) {
    for (let character of arraylist_iterate4(arraylist1)) {
        if (character.playerstats == playerstats) return false;
    }
    for (let character of arraylist_iterate4(arraylist2)) {
        if (character.playerstats == playerstats) return false;
    }
    arraylist_add(arraylist1, { playerstats, can_die });
    playerstats_enable_health_recover(playerstats, can_recover);
    return true;
}

function healthwatcher_internal_get_character(healthwatcher, playerstats) {
    for (let character of arraylist_iterate4(healthwatcher.players)) {
        if (character.playerstats == playerstats) return character;
    }
    for (let character of arraylist_iterate4(healthwatcher.opponents)) {
        if (character.playerstats == playerstats) return character;
    }
    return null;
}

/*function healthwatcher_internal_calculate_totals(arraylist, totals) {
    for (let character of arraylist_iterate4(arraylist)) {
        totals.maximum += playerstats_get_maximum_health(character.playerstats);

        if (playerstats_is_dead(character.playerstats)) {
            if (!character.can_die) playerstats_raise(character.playerstats, false);
        } else {
            totals.accumulated += playerstats_get_health(character.playerstats);
        }
    }
}*/

/*function healthwatcher_internal_calculate_balance(arraylist, difference) {
    let size = arraylist_size(arraylist);
    if (size < 1) return;

    let amount = difference / size;
    amount = -amount;

    for (let character of arraylist_iterate4(arraylist)) {
        let maximum = playerstats_get_maximum_health(character.playerstats);
        let health = playerstats_get_health(character.playerstats) + amount;

        if (health < 0.0) {
            if (character.can_die) {
                playerstats_kill(character.playerstats);
            } else {
                health = 0.0;
            }
        } else if (health > maximum) {
            health = maximum;
        }

        playerstats_set_health(character.playerstats, health);
    }
}
*/
