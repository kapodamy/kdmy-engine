#include "commons.h"
#include "rankingcounter.h"

int32_t rankingcounter_add_state(RankingCounter rankingcounter, ModelHolder modelholder, const char* state_name) {
    print_stub("rankingcounter_add_state", "rankingcounter=%p modelholder=%p state_name=%s", rankingcounter, modelholder, state_name);
    return 0;
}
void rankingcounter_toggle_state(RankingCounter rankingcounter, const char* state_name) {
    print_stub("rankingcounter_toggle_state", "rankingcounter=%p state_name=%s", rankingcounter, state_name);
}
void rankingcounter_reset(RankingCounter rankingcounter) {
    print_stub("rankingcounter_reset", "rankingcounter=%p", rankingcounter);
}
void rankingcounter_hide_accuracy(RankingCounter rankingcounter, bool hide) {
    print_stub("rankingcounter_hide_accuracy", "rankingcounter=%p hide=(bool)%i", rankingcounter, hide);
}
void rankingcounter_use_percent_instead(RankingCounter rankingcounter, bool use_accuracy_percenter) {
    print_stub("rankingcounter_use_percent_instead", "rankingcounter=%p use_accuracy_percenter=(bool)%i", rankingcounter, use_accuracy_percenter);
}
void rankingcounter_set_default_ranking_animation2(RankingCounter rankingcounter, AnimSprite animsprite) {
    print_stub("rankingcounter_set_default_ranking_animation2", "rankingcounter=%p animsprite=%p", rankingcounter, animsprite);
}
void rankingcounter_set_default_ranking_text_animation2(RankingCounter rankingcounter, AnimSprite animsprite) {
    print_stub("rankingcounter_set_default_ranking_text_animation2", "rankingcounter=%p animsprite=%p", rankingcounter, animsprite);
}
void rankingcounter_set_alpha(RankingCounter rankingcounter, float alpha) {
    print_stub("rankingcounter_set_alpha", "rankingcounter=%p alpha=%f", rankingcounter, alpha);
}
void rankingcounter_animation_set(RankingCounter rankingcounter, AnimSprite animsprite) {
    print_stub("rankingcounter_animation_set", "rankingcounter=%p animsprite=%p", rankingcounter, animsprite);
}
void rankingcounter_animation_restart(RankingCounter rankingcounter) {
    print_stub("rankingcounter_animation_restart", "rankingcounter=%p", rankingcounter);
}
void rankingcounter_animation_end(RankingCounter rankingcounter) {
    print_stub("rankingcounter_animation_end", "rankingcounter=%p", rankingcounter);
}
