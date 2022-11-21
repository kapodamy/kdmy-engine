#include "commons.h"
#include "dialogue.h"
#include <stdbool.h>

bool dialogue_apply_state(Dialogue dialogue, const char* state_name) {
    print_stub("dialogue_apply_state", "dialogue=%p state_name=%s", dialogue, state_name);
    return 0;
}
bool dialogue_is_completed(Dialogue dialogue) {
    print_stub("dialogue_is_completed", "dialogue=%p", dialogue);
    return 0;
}
bool dialogue_is_hidden(Dialogue dialogue) {
    print_stub("dialogue_is_hidden", "dialogue=%p", dialogue);
    return 0;
}
bool dialogue_show_dialog(Dialogue dialogue, const char* dialog_src) {
    print_stub("dialogue_show_dialog", "dialogue=%p dialog_src=%s", dialogue, dialog_src);
    return 0;
}
void dialogue_close(Dialogue dialogue) {
    print_stub("dialogue_close", "dialogue=%p", dialogue);
}
void dialogue_hide(Dialogue dialogue, bool hidden) {
    print_stub("dialogue_hide", "dialogue=%p hidden=(bool)%i", dialogue, hidden);
}
Modifier dialogue_get_modifier(Dialogue dialogue) {
    print_stub("dialogue_get_modifier", "dialogue=%p", dialogue);
    return NULL;
}
void dialogue_set_offsetcolor(Dialogue dialogue, float r, float g, float b, float a) {
    print_stub("dialogue_set_offsetcolor", "dialogue=%p r=%f g=%f b=%f a=%f", dialogue, r, g, b, a);
}
void dialogue_set_alpha(Dialogue dialogue, float alpha) {
    print_stub("dialogue_set_alpha", "dialogue=%p alpha=%f", dialogue, alpha);
}
void dialogue_set_antialiasing(Dialogue dialogue, PVRFLAG antialiasing) {
    print_stub("dialogue_set_antialiasing", "dialogue=%p antialiasing=%p", dialogue, antialiasing);
}
