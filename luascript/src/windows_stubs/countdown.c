#include "commons.h"
#include "countdown.h"

void countdown_set_default_animation2(Countdown countdown, TweenKeyframe tweenkeyframe) {
    print_stub("countdown_set_default_animation2", "countdown=%p tweenkeyframe=%p", countdown, tweenkeyframe);
}
void countdown_set_bpm(Countdown countdown, float bpm) {
    print_stub("countdown_set_bpm", "countdown=%p bpm=%f", countdown, bpm);
}
Drawable countdown_get_drawable(Countdown countdown) {
    print_stub("countdown_get_drawable", "countdown=%p", countdown);
    return NULL;
}
bool countdown_ready(Countdown countdown) {
    print_stub("countdown_ready", "countdown=%p", countdown);
    return 0;
}
bool countdown_start(Countdown countdown) {
    print_stub("countdown_start", "countdown=%p", countdown);
    return 0;
}
bool countdown_has_ended(Countdown countdown) {
    print_stub("countdown_has_ended", "countdown=%p", countdown);
    return 0;
}
