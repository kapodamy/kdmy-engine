#include "layout.h"
#include "modding.h"
#include <stdlib.h>
#include "commons.h"

static Layout_t stub_layout;

Layout modding_get_layout(ModdingContext moddingcontext) {
    print_stub("modding_get_layout", "moddingcontext=%p", moddingcontext);
    return &stub_layout;
}
void modding_exit(ModdingContext moddingcontext) {
    print_stub("modding_exit", "moddingcontext=%p", moddingcontext);
}

