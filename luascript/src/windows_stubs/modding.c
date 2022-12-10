#include "commons.h"
#include "modding.h"

static Layout_t stub_layout;

void modding_unlockdirective_create(ModdingContext moddingcontext, const char* name, double value) {
    print_stub("modding_unlockdirective_create", "moddingcontext=%p name=%s value=%f", moddingcontext, name, value);
}
bool modding_unlockdirective_has(ModdingContext moddingcontext, const char* name) {
    print_stub("modding_unlockdirective_has", "moddingcontext=%p name=%s", moddingcontext, name);
    return 1;
}
void modding_unlockdirective_remove(ModdingContext moddingcontext, const char* name) {
    print_stub("modding_unlockdirective_remove", "moddingcontext=%p name=%s", moddingcontext, name);
}
double modding_unlockdirective_get(ModdingContext moddingcontext, const char* name) {
    print_stub("modding_unlockdirective_get", "moddingcontext=%p name=%s", moddingcontext, name);
    return 123;
}
void modding_set_ui_visibility(ModdingContext moddingcontext, bool visible) {
    print_stub("modding_set_ui_visibility", "moddingcontext=%p visible=(bool)%i", moddingcontext, visible);
}
Layout modding_get_layout(ModdingContext moddingcontext) {
    print_stub("modding_get_layout", "moddingcontext=%p", moddingcontext);
    return &stub_layout;
}
void modding_set_halt(ModdingContext moddingcontext, bool halt) {
    print_stub("modding_set_halt", "moddingcontext=%p halt=(bool)%i", halt);
}
void modding_exit(ModdingContext moddingcontext) {
    print_stub("modding_exit", "moddingcontext=%p", moddingcontext);
}
