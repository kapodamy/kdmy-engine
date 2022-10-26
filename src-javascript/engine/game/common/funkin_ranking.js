"use strict";

////////////////////////////////////
// Imported from Kade Engine v1.6.1
/////////////////////////////////////

const FUNKIN_NO_ACCURACY = "N/A";
const FUNKIN_WIFE3_ACCURACIES = [
    99.9935, 99.980, 99.970, 99.955, 99.90, 99.80, 99.70, 99, 96.50, 93, 90, 85,
    80, 70, 60, 60
];
const FUNKIN_WIFE3_RANKS = [
    "AAAAA", "AAAA:", "AAAA.", "AAAA", "AAA:", "AAA.", "AAA", "AA:", "AA.", "AA", "A:", "A.",
    "A", "B", "C", "D"
];

const FUNKIN_RANK_MARVELOUS_FULL_COMBO = "(MFC)";
const FUNKIN_RANK_GOOD_FULL_COMBO = "(GFC)";
const FUNKIN_RANK_FULL_COMBO = "(FC)";
const FUNKIN_RANK_SINGLE_DIGIT_COMBO_BREAKS = "(SDCB)";
const FUNKIN_CLEAR = "(Clear)";


function funkin_get_letter_rank(playerstats) {
    let miss_count = playerstats_get_misses(playerstats);
    let shit_count = playerstats_get_shits(playerstats);
    let bad_count = playerstats_get_bads(playerstats);
    let good_count = playerstats_get_goods(playerstats);
    //let sicks = playerstats_get_sicks(playerstats);

    let has_no_misses_shits_bads = miss_count == 0 && shit_count == 0 && bad_count == 0;

    if (has_no_misses_shits_bads && good_count == 0)
        return FUNKIN_RANK_MARVELOUS_FULL_COMBO;
    else if (has_no_misses_shits_bads && good_count >= 1)
        return FUNKIN_RANK_GOOD_FULL_COMBO;
    else if (miss_count == 0)
        return FUNKIN_RANK_FULL_COMBO;
    else if (miss_count < 10)
        return FUNKIN_RANK_SINGLE_DIGIT_COMBO_BREAKS;
    else
        return FUNKIN_CLEAR;
}

function funking_get_wife3_accuracy(playerstats) {
    // elegant way to calc wife3 accuracy
    const accuracy = playerstats_get_accuracy(playerstats);
    let last_rank = FUNKIN_WIFE3_ACCURACIES.length - 1;

    for (let i = 0; i < last_rank; i++) {
        if (accuracy >= FUNKIN_WIFE3_ACCURACIES[i]) return FUNKIN_WIFE3_RANKS[i];
    }

    // accuracy < 60
    return FUNKIN_WIFE3_RANKS[last_rank];
}

