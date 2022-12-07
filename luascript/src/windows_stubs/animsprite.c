#include "commons.h"
#include "animsprite.h"

static AnimSprite_t animsprite = {};

AnimSprite animsprite_init_from_atlas(float frame_rate, int32_t loop, Atlas atlas, const char* prefix, bool has_number_suffix) {
    print_stub("animsprite_init_from_atlas", "frame_rate=%f loop=%i atlas=%p prefix=%s has_number_suffix=(bool)%i", frame_rate, loop, atlas, prefix, has_number_suffix);
    return &animsprite;
}
AnimSprite animsprite_init_from_animlist(AnimList animlist, const char* animation_name) {
    print_stub("animsprite_init_from_animlist", "animlist=%p animation_name=%s", animlist, animation_name);
    return &animsprite;
}
AnimSprite animsprite_init_as_empty(const char* name) {
    print_stub("animsprite_init_as_empty", "name=%s", name);
    return &animsprite;
}
AnimSprite animsprite_init(AnimListItem animlist_item) {
    print_stub("animsprite_init", "animlist_item=%p", animlist_item);
    return &animsprite;
}
void animsprite_destroy(AnimSprite* animsprite) {
    print_stub("animsprite_destroy", "animsprite=%p", animsprite);
}
void animsprite_set_loop(AnimSprite animsprite, int32_t loop) {
    print_stub("animsprite_set_loop", "animsprite=%p loop=%i", animsprite, loop);
}
void animsprite_restart(AnimSprite animsprite) {
    print_stub("animsprite_restart", "animsprite=%p", animsprite);
}
const char* animsprite_get_name(AnimSprite animsprite) {
    print_stub("animsprite_get_name", "animsprite=%p", animsprite);
    return "123abc";
}
bool animsprite_is_frame_animation(AnimSprite animsprite) {
    print_stub("animsprite_is_frame_animation", "animsprite=%p", animsprite);
    return 0;
}
void animsprite_set_delay(AnimSprite animsprite, float delay_milliseconds) {
    print_stub("animsprite_set_delay", "animsprite=%p delay_milliseconds=%f", animsprite, delay_milliseconds);
}
