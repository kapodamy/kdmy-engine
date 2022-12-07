#include "commons.h"
#include "animlist.h"

static AnimList_t animlist = {};
static AnimListItem_t animlist_item = {};

AnimList animlist_init(const char* src) {
    print_stub("animlist_init", "src=%s", src);
    return &animlist;
}
void animlist_destroy(AnimList* animlist) {
    print_stub("animlist_destroy", "animlist=%p", animlist);
}
AnimListItem animlist_get_animation(AnimList animlist, const char* animation_name) {
    print_stub("animlist_get_animation", "animlist=%p animation_name=%s", animlist, animation_name);
    return &animlist_item;
}
bool animlist_is_item_macro_animation(AnimListItem animlist_item) {
    print_stub("animlist_is_item_macro_animation", "animlist_item=%p", animlist_item);
    return 0;
}
bool animlist_is_item_frame_animation(AnimListItem animlist_item) {
    print_stub("animlist_is_item_frame_animation", "animlist_item=%p", animlist_item);
    return 0;
}
