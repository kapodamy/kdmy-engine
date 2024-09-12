#ifndef _animlist_h
#define _animlist_h

#include <stdbool.h>
#include <stdint.h>

#include "atlas.h"
#include "linkedlist.h"
#include "macroexecutor_types.h"
#include "vertexprops_types.h"


typedef struct {
    float at;
    int32_t id;
    float value;
    AnimInterpolator interpolator;
    Align steps_dir;
    int32_t steps_count;
} AnimListTweenKeyframeEntry;

typedef struct {
    int32_t index;
    int32_t length;
} AnimListAlternateEntry;

typedef struct AnimListItem_s {
    char* name;
    bool is_tweenkeyframe;
    AnimListTweenKeyframeEntry* tweenkeyframe_entries;
    int32_t tweenkeyframe_entries_count;
    int32_t loop;
    float frame_rate;
    bool alternate_per_loop;
    bool alternate_no_random;
    int32_t instructions_count;
    MacroExecutorInstruction* instructions;
    int32_t frame_count;
    AtlasEntry* frames;
    AnimListAlternateEntry* alternate_set;
    int32_t alternate_set_size;
    int32_t frames_count;
    int32_t frame_restart_index;
    bool frame_allow_size_change;
    int32_t loop_from_index;
} AnimListItem;

typedef struct AnimList_s {
    char* src;
    int32_t id;
    int32_t references;
    int32_t entries_count;
    AnimListItem* entries;
}* AnimList;

AnimList animlist_init(const char* src);
void animlist_destroy(AnimList* animlist);

const AnimListItem* animlist_get_animation(AnimList animlist, const char* animation_name);
bool animlist_is_item_macro_animation(const AnimListItem* animlist_item);
bool animlist_is_item_frame_animation(const AnimListItem* animlist_item);
bool animlist_is_item_tweenkeyframe_animation(const AnimListItem* animlist_item);

void animlist_read_entries_to_frames_array(LinkedList frame_list, const char* name, bool has_number_suffix, Atlas atlas, int32_t start, int32_t end);

#endif
