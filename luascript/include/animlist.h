#ifndef __animlist_h
#define __animlist_h

#define ANIM_MACRO_INTERPOLATOR_EASE 0
#define ANIM_MACRO_INTERPOLATOR_EASE_IN 1
#define ANIM_MACRO_INTERPOLATOR_EASE_OUT 2
#define ANIM_MACRO_INTERPOLATOR_EASE_IN_OUT 3
#define ANIM_MACRO_INTERPOLATOR_LINEAR 4
#define ANIM_MACRO_INTERPOLATOR_STEPS 5

typedef int AnimInterpolator;

typedef struct {
    int dummy;
} AnimList_t;
typedef AnimList_t* AnimList;

typedef struct {
    char* name;
} AnimListItem_t;
typedef AnimListItem_t* AnimListItem;

AnimList animlist_init(const char* src);
void animlist_destroy(AnimList* animlist);
AnimListItem animlist_get_animation(AnimList animlist, const char* animation_name);
bool animlist_is_item_macro_animation(AnimListItem animlist_item);
bool animlist_is_item_frame_animation(AnimListItem animlist_item);

#endif

