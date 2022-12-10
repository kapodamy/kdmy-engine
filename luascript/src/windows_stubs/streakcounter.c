#include "commons.h"
#include "streakcounter.h"

static Drawable_t drawable = {};

void streakcounter_reset(StreakCounter streakcounter) {
    print_stub("streakcounter_reset", "streakcounter=%p", streakcounter);
}
void streakcounter_hide_combo_sprite(StreakCounter streakcounter, bool hide) {
    print_stub("streakcounter_hide_combo_sprite", "streakcounter=%p hide=(bool)%i", streakcounter, hide);
}
void streakcounter_set_combo_draw_location(StreakCounter streakcounter, float x, float y) {
    print_stub("streakcounter_set_combo_draw_location", "streakcounter=%p x=%f y=%f", streakcounter, x, y);
}
int32_t streakcounter_state_add(StreakCounter streakcounter, ModelHolder combo_modelholder, ModelHolder number_modelholder, const char* state_name) {
    print_stub("streakcounter_state_add", "streakcounter=%p combo_modelholder=%p number_modelholder=%p state_name=%s", streakcounter, combo_modelholder, number_modelholder, state_name);
    return 0;
}
bool streakcounter_state_toggle(StreakCounter streakcounter, const char* state_name) {
    print_stub("streakcounter_state_toggle", "streakcounter=%p state_name=%s", streakcounter, state_name);
    return 0;
}
void streakcounter_set_alpha(StreakCounter streakcounter, float alpha) {
    print_stub("streakcounter_set_alpha", "streakcounter=%p alpha=%f", streakcounter, alpha);
}
Drawable streakcounter_get_drawable(StreakCounter streakcounter) {
    print_stub("streakcounter_get_drawable", "streakcounter=%p", streakcounter);
    return &drawable;
}
void streakcounter_animation_set(StreakCounter streakcounter, AnimSprite animsprite) {
    print_stub("streakcounter_animation_set", "streakcounter=%p animsprite=%p", streakcounter, animsprite);
}
void streakcounter_animation_restart(StreakCounter streakcounter) {
    print_stub("streakcounter_animation_restart", "streakcounter=%p", streakcounter);
}
void streakcounter_animation_end(StreakCounter streakcounter) {
    print_stub("streakcounter_animation_end", "streakcounter=%p", streakcounter);
}
