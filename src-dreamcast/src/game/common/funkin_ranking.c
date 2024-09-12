#include <stdlib.h>

#include "float64.h"
#include "game/common/funkin.h"


////////////////////////////////////
// Imported from Kade Engine v1.6.1
/////////////////////////////////////

static const float64 FUNKIN_WIFE3_ACCURACIES[] = {
    99.9935, 99.980, 99.970, 99.955, 99.90, 99.80, 99.70, 99, 96.50, 93, 90, 85,
    80, 70, 60, 60
};
static const char* FUNKIN_WIFE3_RANKS[] = {
    "AAAAA", "AAAA:", "AAAA.", "AAAA", "AAA:", "AAA.", "AAA", "AA:", "AA.", "AA", "A:", "A.",
    "A", "B", "C", "D"
};
static const size_t FUNKIN_WIFE3_LENGTH = sizeof(FUNKIN_WIFE3_ACCURACIES) / sizeof(float64);

static const char* FUNKIN_RANK_MARVELOUS_FULL_COMBO = "(MFC)";
static const char* FUNKIN_RANK_GOOD_FULL_COMBO = "(GFC)";
static const char* FUNKIN_RANK_FULL_COMBO = "(FC)";
static const char* FUNKIN_RANK_SINGLE_DIGIT_COMBO_BREAKS = "(SDCB)";
static const char* FUNKIN_CLEAR = "(Clear)";


const char* funkin_get_letter_rank(PlayerStats playerstats) {
    int32_t miss_count = playerstats_get_misses(playerstats);
    int32_t shit_count = playerstats_get_shits(playerstats);
    int32_t bad_count = playerstats_get_bads(playerstats);
    int32_t good_count = playerstats_get_goods(playerstats);
    // int32_t sicks = playerstats_get_sicks(playerstats);

    bool has_no_misses_shits_bads = miss_count == 0 && shit_count == 0 && bad_count == 0;

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

const char* funkin_get_wife3_accuracy(PlayerStats playerstats) {
    // elegant way to calc wife3 accuracy
    float64 accuracy = playerstats_get_accuracy(playerstats);
    size_t last_rank = FUNKIN_WIFE3_LENGTH - 1;

    for (size_t i = 0; i < last_rank; i++) {
        if (accuracy >= FUNKIN_WIFE3_ACCURACIES[i]) return FUNKIN_WIFE3_RANKS[i];
    }

    // accuracy < 60
    return FUNKIN_WIFE3_RANKS[last_rank];
}
