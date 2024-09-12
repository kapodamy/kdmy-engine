#include "animsprite.h"
#include "cloneutils.h"
#include "externals/luascript.h"
#include "float64.h"
#include "linkedlist.h"
#include "logger.h"
#include "macroexecutor.h"
#include "malloc_utils.h"
#include "map.h"
#include "stringutils.h"
#include "tweenlerp.h"
#include "vertexprops.h"


struct AnimSprite_s {
    int32_t id;
    char* name;
    AtlasEntry* frames;
    int32_t frame_count;
    int32_t loop_from_index;
    float64 frame_time;
    int32_t loop;
    int32_t loop_progress;
    bool has_looped;
    bool disable_loop;
    float64 progress;
    int32_t current_index;
    int32_t current_offset;
    float delay;
    float64 delay_progress;
    bool delay_active;
    bool is_empty;
    AnimListAlternateEntry* alternate_set;
    int32_t alternate_size;
    bool alternate_per_loop;
    int32_t alternate_index;
    bool allow_override_size;
    MacroExecutor macroexecutor;
    TweenLerp tweenlerp;
};


static Map ANIMSPRITE_POOL = NULL;
static int32_t ANIMSPRITE_IDS = 0;


static void animsprite_internal_apply_frame(AnimSprite animsprite, Sprite sprite, int32_t index);
static void animsprite_internal_apply_frame2(AnimSprite animsprite, StateSprite statesprite, int32_t index);
static void animsprite_internal_alternate_choose(AnimSprite animsprite, bool loop);
static AnimSprite animsprite_internal_init(const char* name, int32_t loop, float frame_rate);


void __attribute__((constructor)) __ctor_animsprite() {
    ANIMSPRITE_POOL = map_init();
}


AnimSprite animsprite_init_from_atlas(float frame_rate, int32_t loop, Atlas atlas, const char* prefix, bool has_number_suffix) {
    LinkedList frames = linkedlist_init();

    animlist_read_entries_to_frames_array(frames, prefix, has_number_suffix, atlas, 0, -1);
    int32_t frame_count = linkedlist_count(frames);

    //
    // Unintended behavior, the caller should build the static animation
    //
    /*if (frame_count < 1) {
        const AtlasEntry* atlas_entry = NULL;

        if (has_number_suffix) {
            // attempt to obtain one frame without number suffix
            atlas_entry = atlas_get_entry(atlas, prefix);
            if (atlas_entry) {
                linkedlist_add_item(frames, atlas_entry);
                frame_count++;
            }
        }
    }*/

    if (frame_count < 1) {
        linkedlist_destroy(&frames);
        return NULL;
    }

    AnimSprite animsprite = animsprite_internal_init(prefix, loop, frame_rate);

    animsprite->frame_count = frame_count;
    animsprite->frames = linkedlist_to_solid_array(frames, sizeof(AtlasEntry));
    linkedlist_destroy(&frames);

    return animsprite;
}

AnimSprite animsprite_init_from_atlas_entry(const AtlasEntry* atlas_entry, bool loop_indefinitely, float frame_rate) {
    int32_t loop = loop_indefinitely ? 0 : 1;
    AnimSprite animsprite = animsprite_internal_init(atlas_entry->name, loop, frame_rate);
    animsprite->frame_count = 1;
    animsprite->frames = malloc_for_array(AtlasEntry, 1);
    *animsprite->frames = *atlas_entry;
    return animsprite;
}

AnimSprite animsprite_init_from_animlist(AnimList animlist, const char* animation_name) {
    for (int32_t i = 0; i < animlist->entries_count; i++) {
        if (string_equals(animlist->entries[i].name, animation_name)) {
            return animsprite_init(&animlist->entries[i]);
        }
    }
    return NULL;
}

AnimSprite animsprite_init_from_tweenlerp(const char* name, int32_t loop, TweenLerp tweenlerp) {
    if (!tweenlerp) return NULL;

    AnimSprite animsprite = animsprite_internal_init(name, loop, 0.0f);
    animsprite->tweenlerp = tweenlerp_clone(tweenlerp);
    return animsprite;
}

AnimSprite animsprite_init_as_empty(const char* name) {
    AnimSprite animsprite = animsprite_internal_init(name, 0, 0.0f);
    animsprite->is_empty = true;
    return animsprite;
}

AnimSprite animsprite_init(const AnimListItem* animlist_item) {
    AnimSprite animsprite;

    if (animlist_item->is_tweenkeyframe) {
        // unsupported, use <AnimationMacro/> instead
        return NULL;
    }

    animsprite = animsprite_internal_init(
        animlist_item->name, animlist_item->loop, animlist_item->frame_rate
    );

    if (animlist_item->instructions_count > 0) {
        animsprite->macroexecutor = macroexecutor_init(animlist_item);
        macroexecutor_set_restart_in_frame(
            animsprite->macroexecutor,
            animlist_item->frame_restart_index, animlist_item->frame_allow_size_change
        );

        return animsprite;
    }

    //
    // Initialize frame animation
    //
    if (animlist_item->alternate_set_size > 0) {
        int32_t frame_count = 0;
        for (int32_t i = 0; i < animlist_item->alternate_set_size; i++)
            frame_count += animlist_item->alternate_set[i].length;

        if (frame_count > animlist_item->frame_count) {
            logger_error("Invalid animlist_item->alternate_set");
            assert(frame_count <= animlist_item->frame_count);
        }
    }

    animsprite->alternate_per_loop = animlist_item->alternate_per_loop;
    animsprite->alternate_size = animlist_item->alternate_set_size;
    animsprite->alternate_set = CLONE_STRUCT_ARRAY(AnimListAlternateEntry, animlist_item->alternate_set, animlist_item->alternate_set_size);
    if (animlist_item->alternate_no_random) animsprite->alternate_index = 0;

    animsprite->frame_count = animlist_item->frame_count;
    animsprite->frames = CLONE_STRUCT_ARRAY(AtlasEntry, animlist_item->frames, animlist_item->frame_count);
    animsprite->loop_from_index = animlist_item->loop_from_index;

    return animsprite;
}

void animsprite_destroy(AnimSprite* animsprite_ptr) {
    AnimSprite animsprite = *animsprite_ptr;
    if (!animsprite) return;

    free_chk(animsprite->name);
    free_chk(animsprite->frames);
    free_chk(animsprite->alternate_set);

    if (animsprite->macroexecutor)
        macroexecutor_destroy(&animsprite->macroexecutor);

    if (animsprite->tweenlerp)
        tweenlerp_destroy(&animsprite->tweenlerp);

    map_delete(ANIMSPRITE_POOL, animsprite->id);

    luascript_drop_shared(animsprite);

    free_chk(animsprite);
    *animsprite_ptr = NULL;
}

AnimSprite animsprite_clone(AnimSprite animsprite) {
    AnimSprite copy = CLONE_STRUCT(struct AnimSprite_s, AnimSprite, animsprite);

    copy->id = ANIMSPRITE_IDS++;
    map_add(ANIMSPRITE_POOL, copy->id, copy);

    copy->name = string_duplicate(animsprite->name);

    animsprite->alternate_set = CLONE_STRUCT_ARRAY(AnimListAlternateEntry, animsprite->alternate_set, animsprite->alternate_size);
    animsprite->frames = CLONE_STRUCT_ARRAY(AtlasEntry, animsprite->frames, animsprite->frame_count);

    if (copy->macroexecutor)
        copy->macroexecutor = macroexecutor_clone(animsprite->macroexecutor);

    if (copy->tweenlerp)
        copy->tweenlerp = tweenlerp_clone(animsprite->tweenlerp);

    return copy;
}

void animsprite_set_loop(AnimSprite animsprite, int32_t loop) {
    animsprite->loop = loop;
}

void animsprite_restart(AnimSprite animsprite) {
    if (animsprite->is_empty) return;

    animsprite->loop_progress = 0;
    animsprite->has_looped = false;
    animsprite->disable_loop = false;
    animsprite->delay_progress = 0.0;
    animsprite->delay_active = animsprite->delay > 0.0f;

    if (animsprite->macroexecutor) {
        macroexecutor_restart(animsprite->macroexecutor);
        return;
    }

    if (animsprite->tweenlerp) {
        tweenlerp_restart(animsprite->tweenlerp);
        return;
    }

    animsprite->progress = 0.0;
    animsprite->current_index = 0;

    animsprite_internal_alternate_choose(animsprite, false);
}

int32_t animsprite_animate(AnimSprite animsprite, float elapsed) {
    if (math2d_is_float_NaN(elapsed)) {
        logger_error("invalid elapsed argument");
        assert(!math2d_is_float_NaN(elapsed));
    }
    if (animsprite->is_empty) return 0;
    if (animsprite->loop > 0 && animsprite->loop_progress >= animsprite->loop) return 1;
    if (animsprite->loop_progress == MATH2D_MAX_INT32) return 1;

    if (animsprite->delay_active && animsprite->delay > 0.0f) {
        animsprite->delay_progress += elapsed;
        if (animsprite->delay_progress < animsprite->delay) return 0;

        elapsed = (float)(animsprite->delay_progress - animsprite->delay);
        animsprite->delay_active = false;
        animsprite->delay_progress = 0.0;
    }

    animsprite->progress += elapsed;

    if (animsprite->macroexecutor || animsprite->tweenlerp) {
        bool completed;

        if (animsprite->macroexecutor)
            completed = macroexecutor_animate(animsprite->macroexecutor, elapsed);
        else
            completed = tweenlerp_animate(animsprite->tweenlerp, elapsed);

        if (completed) {
            animsprite->delay_active = animsprite->delay > 0.0f;
            animsprite->delay_progress = 0.0;
            animsprite->has_looped = true;

            if (animsprite->disable_loop) {
                animsprite->loop_progress = MATH2D_MAX_INT32;
                return 1;
            }

            if (animsprite->loop > 0) {
                animsprite->loop_progress++;
                if (animsprite->loop_progress >= animsprite->loop) return 0;
            }

            if (animsprite->macroexecutor)
                macroexecutor_restart(animsprite->macroexecutor);
            else
                tweenlerp_restart(animsprite->tweenlerp);
        }
        return 0;
    }

    float64 new_index = animsprite->progress / animsprite->frame_time;
    animsprite->current_index = (int32_t)new_index;

    if (animsprite->current_index >= animsprite->frame_count) {
        animsprite->has_looped = true;
        if (animsprite->disable_loop) {
            animsprite->loop_progress = MATH2D_MAX_INT32;
            return 1;
        }
        if (animsprite->loop > 0) {
            animsprite->loop_progress++;
            if (animsprite->loop_progress >= animsprite->loop) return 1;
        }
        animsprite_internal_alternate_choose(animsprite, true);
        animsprite->delay_active = animsprite->delay > 0.0f;
        animsprite->current_index = animsprite->loop_from_index;
        animsprite->progress = animsprite->loop_from_index * animsprite->frame_time;
    }

    return 0;
}

const char* animsprite_get_name(AnimSprite animsprite) {
    return animsprite->name;
}


bool animsprite_is_completed(AnimSprite animsprite) {
    if (animsprite->is_empty) return true;
    if (animsprite->loop < 1) return false;
    if (animsprite->loop_progress >= animsprite->loop) return true;

    if (animsprite->macroexecutor)
        return macroexecutor_is_completed(animsprite->macroexecutor);
    else if (animsprite->tweenlerp)
        return tweenlerp_is_completed(animsprite->tweenlerp);
    else
        return animsprite->current_index >= animsprite->frame_count;
}

bool animsprite_is_frame_animation(AnimSprite animsprite) {
    return animsprite && !animsprite->macroexecutor && !animsprite->tweenlerp;
}

bool animsprite_has_looped(AnimSprite animsprite) {
    bool has_looped = animsprite->has_looped;
    if (has_looped) animsprite->has_looped = false;
    return has_looped;
}

void animsprite_disable_loop(AnimSprite animsprite) {
    animsprite->disable_loop = true;
}

void animsprite_stop(AnimSprite animsprite) {
    animsprite->loop_progress = MATH2D_MAX_INT32;
}


void animsprite_force_end(AnimSprite animsprite) {
    if (animsprite->is_empty) return;

    if (animsprite->macroexecutor)
        macroexecutor_force_end(animsprite->macroexecutor, NULL);
    else if (animsprite->tweenlerp)
        tweenlerp_restart(animsprite->tweenlerp);
    else
        animsprite->current_index = animsprite->frame_count - 1;

    animsprite->delay_active = false;

    if (animsprite->loop != 0) animsprite->loop_progress++;
}

void animsprite_force_end2(AnimSprite animsprite, Sprite sprite) {
    if (animsprite->is_empty) return;
    animsprite_force_end(animsprite);

    if (!sprite) return;

    animsprite_update_sprite(animsprite, sprite, false);
}

void animsprite_force_end3(AnimSprite animsprite, StateSprite statesprite) {
    if (animsprite->is_empty) return;
    animsprite_force_end(animsprite);

    if (!statesprite) return;

    animsprite_update_statesprite(animsprite, statesprite, false);
}

void animsprite_set_delay(AnimSprite animsprite, float delay_milliseconds) {
    animsprite->delay = delay_milliseconds;
    animsprite->delay_progress = 0.0; // ¿should clear the delay progress?
    animsprite->delay_active = true;
}

void animsprite_update_sprite(AnimSprite animsprite, Sprite sprite, bool stack_changes) {
    if (animsprite->is_empty) return;

    if (animsprite->macroexecutor)
        macroexecutor_state_apply(animsprite->macroexecutor, sprite, !stack_changes);
    else if (animsprite->tweenlerp)
        tweenlerp_vertex_set_properties(animsprite->tweenlerp, sprite, (PropertySetter)sprite_set_property);
    else
        animsprite_internal_apply_frame(animsprite, sprite, animsprite->current_index);
}

void animsprite_update_statesprite(AnimSprite animsprite, StateSprite statesprite, bool stack_changes) {
    if (animsprite->is_empty) return;

    if (animsprite->macroexecutor)
        macroexecutor_state_apply2(animsprite->macroexecutor, statesprite, !stack_changes);
    else if (animsprite->tweenlerp)
        tweenlerp_vertex_set_properties(animsprite->tweenlerp, statesprite, (PropertySetter)statesprite_set_property);
    else
        animsprite_internal_apply_frame2(animsprite, statesprite, animsprite->current_index);
}

void animsprite_update_textsprite(AnimSprite animsprite, TextSprite textsprite, bool stack_changes) {
    if (animsprite->is_empty) return;

    if (animsprite->tweenlerp)
        tweenlerp_vertex_set_properties(animsprite->tweenlerp, textsprite, (PropertySetter)textsprite_set_property);
    else if (animsprite->macroexecutor)
        macroexecutor_state_apply4(animsprite->macroexecutor, textsprite, !stack_changes);
}

void animsprite_update_modifier(AnimSprite animsprite, Modifier* modifier, bool stack_changes) {
    if (animsprite->macroexecutor)
        macroexecutor_state_to_modifier(animsprite->macroexecutor, modifier, !stack_changes);
    if (animsprite->tweenlerp)
        tweenlerp_vertex_set_properties(
            animsprite->tweenlerp,
            modifier,
            (PropertySetter)pvr_context_helper_set_modifier_property
        );
}

void animsprite_update_drawable(AnimSprite animsprite, Drawable drawable, bool stack_changes) {
    if (animsprite->macroexecutor)
        macroexecutor_state_apply3(animsprite->macroexecutor, drawable, !stack_changes);
    else if (animsprite->tweenlerp)
        tweenlerp_vertex_set_properties(animsprite->tweenlerp, drawable, (PropertySetter)drawable_set_property);
}

void animsprite_update_using_callback(AnimSprite animsprite, void* private_data, PropertySetter setter_callback, bool stack_changes) {
    if (animsprite->macroexecutor) {
        macroexecutor_state_apply5(
            animsprite->macroexecutor, private_data, setter_callback, !stack_changes
        );
    } else if (animsprite->tweenlerp) {
        tweenlerp_vertex_set_properties(animsprite->tweenlerp, private_data, setter_callback);
    } else {
        setter_callback(
            private_data, SPRITE_PROP_FRAMEINDEX, animsprite->current_index + animsprite->current_offset
        );
    }
}

bool animsprite_rollback(AnimSprite animsprite, float elapsed) {
    if (animsprite->progress <= 0.0) return true; // completed

    if (animsprite->macroexecutor) {
        // imposible rollback a macroexecutor animation
        animsprite->progress = 0.0;
        return true;
    } else if (animsprite->tweenlerp) {
        // tweenlerp animation
        tweenlerp_animate_timestamp(animsprite->tweenlerp, animsprite->progress);
    } else {
        // frame animation
        animsprite->current_index = (int32_t)(animsprite->progress / animsprite->frame_time);
    }

    animsprite->progress -= elapsed;
    return false;
}

const AtlasEntry* animsprite_helper_get_first_frame_atlas_entry(AnimSprite animsprite) {
    if (animsprite->macroexecutor)
        return macroexecutor_get_frame(animsprite->macroexecutor, 0);
    else if (animsprite->frame_count > 0)
        return &animsprite->frames[0];
    else
        return NULL;
}

void animsprite_allow_override_sprite_size(AnimSprite animsprite, bool enable) {
    animsprite->allow_override_size = enable;
}


static void animsprite_internal_apply_frame(AnimSprite animsprite, Sprite sprite, int32_t index) {
    if (!animsprite->frames) return;
    if (index < 0 || index >= animsprite->frame_count) return;

    index += animsprite->current_offset;

    const AtlasEntry* frame = &animsprite->frames[index];
    sprite_set_offset_source(sprite, frame->x, frame->y, frame->width, frame->height);
    sprite_set_offset_frame(sprite, frame->frame_x, frame->frame_y, frame->frame_width, frame->frame_height);
    sprite_set_offset_pivot(sprite, frame->pivot_x, frame->pivot_y);

    if (animsprite->allow_override_size) {
        sprite_set_draw_size(
            sprite,
            frame->frame_width > 0.0f ? frame->frame_width : frame->width,
            frame->frame_height > 0.0f ? frame->frame_height : frame->height
        );
    }
}

static void animsprite_internal_apply_frame2(AnimSprite animsprite, StateSprite statesprite, int32_t index) {
    if (!animsprite->frames) return;
    if (index < 0 || index >= animsprite->frame_count) return;

    index += animsprite->current_offset;

    const AtlasEntry* frame = &animsprite->frames[index];
    statesprite_set_offset_source(
        statesprite, frame->x, frame->y, frame->width, frame->height
    );
    statesprite_set_offset_frame(
        statesprite, frame->frame_x, frame->frame_y, frame->frame_width, frame->frame_height
    );
    statesprite_set_offset_pivot(
        statesprite, frame->pivot_x, frame->pivot_y
    );

    if (animsprite->allow_override_size) {
        statesprite_set_draw_size(
            statesprite,
            frame->frame_width > 0.0f ? frame->frame_width : frame->width,
            frame->frame_height > 0.0f ? frame->frame_height : frame->height
        );
    }
}

static void animsprite_internal_alternate_choose(AnimSprite animsprite, bool loop) {
    if (animsprite->alternate_size < 2) return;
    if (loop && !animsprite->alternate_per_loop) return;

    int32_t index;

    if (animsprite->alternate_index < 0) {
        index = math2d_random_int(0, animsprite->alternate_size);
    } else {
        animsprite->alternate_index++;
        if (animsprite->alternate_index >= animsprite->alternate_size) animsprite->alternate_index = 0;
        index = animsprite->alternate_index;
    }

    AnimListAlternateEntry* alternate = &animsprite->alternate_set[index];
    animsprite->current_offset = alternate->index;
    animsprite->frame_count = alternate->length;
}


static AnimSprite animsprite_internal_init(const char* name, int32_t loop, float frame_rate) {
    float64 frame_time = frame_rate > 0.0f ? (1000.0 / frame_rate) : 0.0;

    AnimSprite animsprite = malloc_chk(sizeof(struct AnimSprite_s));
    malloc_assert(animsprite, AnimSprite);

    *animsprite = (struct AnimSprite_s){
        .id = ANIMSPRITE_IDS++,
        .name = string_duplicate(name),

        .frames = NULL,
        .frame_count = 0,
        .loop_from_index = 0,

        .frame_time = frame_time,

        .loop = loop,
        .loop_progress = 0,
        .has_looped = false,
        .disable_loop = false,

        .progress = 0.0,

        .current_index = 0,
        .current_offset = 0,

        .delay = 0.0f,
        .delay_progress = 0.0,
        .delay_active = false,

        .is_empty = false,

        .alternate_set = NULL,
        .alternate_size = 0,
        .alternate_per_loop = false,
        .alternate_index = -1,

        .allow_override_size = false,

        .macroexecutor = NULL,
        .tweenlerp = NULL
    };

    map_add(ANIMSPRITE_POOL, animsprite->id, animsprite);
    return animsprite;
}
