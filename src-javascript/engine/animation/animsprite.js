"use strict";

const FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE = 24;

const ANIMSPRITE_POOL = new Map();
var ANIMSPRITE_IDS = 0;

function animsprite_init_from_atlas(frame_rate, loop, atlas, prefix, has_number_suffix) {
    let frames = linkedlist_init();

    animlist_read_entries_to_frames_array(frames, prefix, has_number_suffix, atlas, 0, -1);
    let frame_count = linkedlist_count(frames);

    //
    // Unintended behavior, the caller should build the static animation
    //
    /*if (frame_count < 1) {
        let atlas_entry = null;

        if (has_number_suffix) {
            // attempt to obtain one frame without number suffix
            let atlas_entry = atlas_get_entry(atlas, prefix);
            if (atlas_entry) {
                linkedlist_add_item(frames, atlas_entry);
                frame_count++;
            }
        }

        if (!atlas_entry) {
            linkedlist_destroy(frames);
            return null;
        }
    }*/

    if (frame_count < 1) {
        linkedlist_destroy(frames);
        return null;
    }

    let animsprite = animsprite_internal_init(prefix, loop, frame_rate);

    animsprite.frame_count = frame_count;
    animsprite.frames = linkedlist_to_solid_array(frames);
    linkedlist_destroy(frames);

    return animsprite;
}

function animsprite_init_from_atlas_entry(atlas_entry, loop_indefinitely, frame_rate) {
    let loop = loop_indefinitely ? 0 : 1;
    let animsprite = animsprite_internal_init(atlas_entry.name, loop, frame_rate);
    animsprite.frame_count = 1;
    animsprite.frames = new Array(1);
    animsprite.frames[0] = clone_struct(atlas_entry, 1);
    return animsprite;
}

function animsprite_init_from_animlist(animlist, animation_name) {
    for (let i = 0; i < animlist.entries_count; i++) {
        if (animlist.entries[i].name == animation_name) {
            return animsprite_init(animlist.entries[i]);
        }
    }
    return null;
}

function animprite_init_from_macroexecutor(name, loop, macroexecutor) {
    let animsprite = animsprite_internal_init(name, loop, 0);
    animsprite.macroexecutor = macroexecutor;
    return animsprite;
}

function animsprite_init_as_empty(name) {
    let animsprite = animsprite_internal_init(name, 0, 0);
    animsprite.is_empty = 1;
    return animsprite;
}

function animsprite_init(animlist_item) {
	let animsprite;

    if (animlist_item.is_tweenlerp) {
        animsprite = animsprite_internal_init(animlist_item.name, animlist_item.loop, -1);
        animsprite.tweenlerp = tweenlerp_init2(animlist_item);
        return animsprite;
    }

    if (animlist_item.alternate_set_size > 0) {
        let frame_count = 0;
        for (let i = 0; i < animlist_item.alternate_set_size; i++)
            frame_count += animlist_item.alternate_set[i].length;

        if (frame_count > animlist_item.frame_count)
            throw new Error("Invalid animlist_item.alternate_set");
    }

    animsprite = animsprite_internal_init(
        animlist_item.name, animlist_item.loop, animlist_item.frame_rate
    );

    animsprite.alternate_per_loop = animlist_item.alternate_per_loop;
    animsprite.alternate_size = animlist_item.alternate_set_size;
    animsprite.alternate_set = clone_array(
        animlist_item.alternate_set,
        animlist_item.alternate_set_size
    );
    if (animlist_item.alternate_no_random) animsprite.alternate_index = 0;

    animsprite.frame_count = animlist_item.frame_count;
    animsprite.frames = clone_array(animlist_item.frames, animlist_item.frame_count);

    if (animlist_item.instructions_count > 0) {
        let instructions = clone_array(animlist_item.instructions, animlist_item.instructions_count);
		for (let i=0 ; i< animlist_item.instructions_count ; i++) {
			instructions[i].values = clone_array(instructions[i].values, instructions[i].values_size);
		}

        animsprite.macroexecutor = macroexecutor_init(
            instructions, animlist_item.instructions_count, animsprite.frames, animlist_item.frame_count
        );

        macroexecutor_set_restart_in_frame(
            animsprite.macroexecutor,
            animlist_item.frame_restart_index, animlist_item.frame_allow_size_change
        );
    }

    return animsprite;
}

function animsprite_destroy(animsprite) {
	if (animsprite.frames != null) {
        for (let i=0 ; i<animsprite.frame_count ; i++) {
            animsprite.frames[i].name = undefined;
        }
        animsprite.frames = undefined;
    }

    if (animsprite.alternate_set)
        animsprite.alternate_set = undefined;

    if (animsprite.macroexecutor)
        macroexecutor_destroy(animsprite.macroexecutor);

    if (animsprite.tweenlerp)
        tweenlerp_destroy(animsprite.tweenlerp);

    ANIMSPRITE_POOL.delete(animsprite.id);

    animsprite = undefined;
}

function animsprite_clone(animsprite) {
    let copy = clone_struct(animsprite);
    if (!copy) return null;
	
	copy.id = ANIMSPRITE_IDS++;
	ANIMSPRITE_POOL.set(copy.id, copy);	

    copy.alternate_set = clone_array(animsprite.alternate_set, animsprite.alternate_size);
    copy.frames = clone_array(animsprite.frames, animsprite.frame_count);

    if (copy.macroexecutor) {
		copy.macroexecutor = macroexecutor_clone(animsprite.macroexecutor, 0);
        copy.macroexecutor.frames = copy.frames;
        copy.macroexecutor.frame_count = copy.frame_count;
    }

	if (copy.tweenlerp) {
		copy.tweenlerp = tweenlerp_clone(animsprite.tweenlerp);
	}

    return copy;
}

function animsprite_set_loop(animsprite, loop) {
    animsprite.loop = loop;
}

function animsprite_restart(animsprite) {
    if (animsprite.is_empty) return;

    animsprite.loop_progress = 0;
    animsprite.has_looped = 0;
    animsprite.disable_loop = 0;
    animsprite.delay_progress = 0.0;
    animsprite.delay_active = animsprite.delay > 0;

    if (animsprite.macroexecutor) {
        macroexecutor_restart(animsprite.macroexecutor);
        return;
    }

    if (animsprite.tweenlerp) {
        tweenlerp_restart(animsprite.tweenlerp);
        return;
    }

    animsprite.progress = 0;
    animsprite.current_index = 0;

    animsprite_internal_alternate_choose(animsprite, 0);
}

function animsprite_animate(animsprite, elapsed) {
    if (!Number.isFinite(elapsed)) throw new Error("invalid elapsed argument");
    if (animsprite.is_empty) return 0;
    if (!Number.isFinite(animsprite.loop_progress)) return 1;
    if (animsprite.loop > 0 && animsprite.loop_progress >= animsprite.loop) return 1;

    if (animsprite.delay_active && animsprite.delay > 0) {
        animsprite.delay_progress += elapsed;
        if (animsprite.delay_progress < animsprite.delay) return 0;

        elapsed = animsprite.delay_progress - animsprite.delay;
        animsprite.delay_active = 0;
        animsprite.delay_progress = 0.0;
    }

    animsprite.progress += elapsed;

    if (animsprite.macroexecutor || animsprite.tweenlerp) {
        let completed;

        if (animsprite.macroexecutor)
            completed = macroexecutor_animate(animsprite.macroexecutor, elapsed);
        else
            completed = tweenlerp_animate(animsprite.tweenlerp, elapsed);

        if (completed) {
            animsprite.delay_active = animsprite.delay > 0;
            animsprite.delay_progress = 0;
            animsprite.has_looped = 1;

            if (animsprite.disable_loop) {
                animsprite.loop_progress = Infinity;
                return 1;
            }

            if (animsprite.loop > 0) {
                animsprite.loop_progress++;
                if (animsprite.loop_progress >= animsprite.loop) return 0;
            }

            if (animsprite.macroexecutor) macroexecutor_restart(animsprite.macroexecutor);
            else tweenlerp_restart(animsprite.tweenlerp);
        }
        return 0;
    }

    let new_index = animsprite.progress / animsprite.frame_time;
    animsprite.current_index = Math.trunc(new_index);

    if (animsprite.current_index >= animsprite.frame_count) {
        animsprite.has_looped = 1;
        if (animsprite.disable_loop) {
            animsprite.loop_progress = Infinity;
            return 1;
        }
        if (animsprite.loop > 0) {
            animsprite.loop_progress++;
            if (animsprite.loop_progress >= animsprite.loop) return 1;
        }
        animsprite_internal_alternate_choose(animsprite, 1);
        animsprite.delay_active = animsprite.delay > 0;
        animsprite.current_index = 0;
        animsprite.progress = 0;
    }

    return 0;
}

function animsprite_get_name(animsprite) {
    return animsprite.name;
}


function animsprite_is_completed(animsprite) {
    if (animsprite.is_empty) return 1;
    if (animsprite.loop < 1) return 0;
    if (animsprite.loop_progress >= animsprite.loop) return 1;

    if (animsprite.macroexecutor)
        return macroexecutor_is_completed(animsprite.macroexecutor);
    else if (animsprite.tweenlerp)
        return tweenlerp_is_completed(animsprite.tweenlerp);
    else
        return animsprite.current_index >= animsprite.frame_count;
}

function animsprite_is_frame_animation(animsprite) {
    return animsprite && !animsprite.macroexecutor && !animsprite.tweenlerp;
}

function animsprite_has_looped(animsprite) {
    let has_looped = animsprite.has_looped;
    if (has_looped) animsprite.has_looped = 0;
    return has_looped;
}

function animsprite_disable_loop(animsprite) {
    animsprite.disable_loop = 1;
}

function animsprite_stop(animsprite) {
    animsprite.loop_progress = Infinity;
}


function animsprite_force_end(animsprite) {
    if (animsprite.is_empty) return;

    if (animsprite.macroexecutor)
        macroexecutor_force_end(animsprite.macroexecutor, null);
    else if (animsprite.tweenlerp)
        tweenlerp_restart(animsprite.tweenlerp);
    else
        animsprite.current_index = animsprite.frame_count - 1;

    animsprite.delay_active = 0;

    if (animsprite.loop != 0) animsprite.loop_progress++;
}

function animsprite_force_end2(animsprite, sprite) {
    if (animsprite.is_empty) return;
    animsprite_force_end(animsprite);

    if (!sprite) return;

    animsprite_update_sprite(animsprite, sprite, 0);
}

function animsprite_force_end3(animsprite, statesprite) {
    if (animsprite.is_empty) return;
    animsprite_force_end(animsprite);

    if (!statesprite) return;

    animsprite_update_statesprite(animsprite, statesprite, 0);
}

function animsprite_set_delay(animsprite, delay_milliseconds) {
    animsprite.delay = delay_milliseconds;
    animsprite.delay_progress = 0.0;// ¿should clear the delay progress?
    animsprite.delay_active = 1;
}

function animsprite_update_sprite(animsprite, sprite, stack_changes) {
    if (animsprite.is_empty) return;

    if (animsprite.macroexecutor)
        macroexecutor_state_apply(animsprite.macroexecutor, sprite, !stack_changes);
    else if (animsprite.tweenlerp)
        tweenlerp_vertex_set_properties(animsprite.tweenlerp, sprite, sprite_set_property);
    else
        animsprite_internal_apply_frame(animsprite, sprite, animsprite.current_index);
}

function animsprite_update_statesprite(animsprite, statesprite, stack_changes) {
    if (animsprite.is_empty) return;

    if (animsprite.macroexecutor)
        macroexecutor_state_apply2(animsprite.macroexecutor, statesprite, !stack_changes);
    else if (animsprite.tweenlerp)
        tweenlerp_vertex_set_properties(animsprite.tweenlerp, statesprite, statesprite_set_property);
    else
        animsprite_internal_apply_frame2(animsprite, statesprite, animsprite.current_index);
}

function animsprite_update_textsprite(animsprite, textsprite, stack_changes) {
    if (animsprite.is_empty) return;

    if (animsprite.tweenlerp)
        tweenlerp_vertex_set_properties(animsprite.tweenlerp, textsprite, textsprite_set_property);
    else if (animsprite.macroexecutor)
        macroexecutor_state_apply4(animsprite.macroexecutor, textsprite, !stack_changes);
}

function animsprite_update_modifier(animsprite, modifier, stack_changes) {
    if (animsprite.macroexecutor)
        macroexecutor_state_to_modifier(animsprite.macroexecutor, modifier, !stack_changes);
    if (animsprite.tweenlerp)
        tweenlerp_vertex_set_properties(
            animsprite.tweenlerp,
            modifier,
            pvrctx_helper_set_modifier_property
        );
}

function animsprite_update_drawable(animsprite, drawable, stack_changes) {
    if (animsprite.macroexecutor)
        macroexecutor_state_apply3(animsprite.macroexecutor, drawable, !stack_changes);
    else if (animsprite.tweenlerp)
        tweenlerp_vertex_set_properties(animsprite.tweenlerp, drawable, drawable_set_property);
}

function animsprite_update_using_callback(animsprite, private_data, setter_callback, stack_changes) {
    if (animsprite.macroexecutor) {
        macroexecutor_state_apply5(
            animsprite.macroexecutor, private_data, setter_callback, !stack_changes
        );
    } else if (animsprite.tweenlerp) {
        tweenlerp_vertex_set_properties(animsprite.tweenlerp, private_data, setter_callback);
    } else {
        setter_callback(
            private_data, SPRITE_PROP_FRAMEINDEX, animsprite.current_index + animsprite.current_offset
        );
    }
}

function animsprite_get_macroexecutor(animsprite) {
    return animsprite.macroexecutor;
}


function animsprite_rollback(animsprite, elapsed) {
    if (animsprite.progress <= 0) return 1;// completed

    if (animsprite.macroexecutor) {
        // imposible rollback a macroexecutor animation
        animsprite.progress = 0;
        return 1;
    } else if (animsprite.tweenlerp) {
        // tweenlerp animation
        tweenlerp_animate_timestamp(animsprite.tweenlerp, animsprite.progress);
    } else {
        // frame animation
        animsprite.current_index = Math.trunc(animsprite.progress / animsprite.frame_time);
    }

    animsprite.progress -= elapsed;
    return 0;
}

function animsprite_helper_get_first_frame_atlas_entry(animsprite) {
    if (animsprite.frame_count < 1) return null;
    return animsprite.frames[0];
}


function animsprite_internal_apply_frame(animsprite, sprite, index) {
    if (!animsprite.frames) return;
    if (index < 0 || index >= animsprite.frame_count) return;

    index += animsprite.current_offset;

    let frame = animsprite.frames[index];
    sprite_set_offset_source(sprite, frame.x, frame.y, frame.width, frame.height);
    sprite_set_offset_frame(sprite, frame.frame_x, frame.frame_y, frame.frame_width, frame.frame_height);
    sprite_set_offset_pivot(sprite, frame.pivot_x, frame.pivot_y);
}

function animsprite_internal_apply_frame2(animsprite, statesprite, index) {
    if (!animsprite.frames) return;
    if (index < 0 || index >= animsprite.frame_count) return;

    index += animsprite.current_offset;

    let frame = animsprite.frames[index];
    statesprite_set_offset_source(
        statesprite, frame.x, frame.y, frame.width, frame.height
    );
    statesprite_set_offset_frame(
        statesprite, frame.frame_x, frame.frame_y, frame.frame_width, frame.frame_height
    );
    statesprite_set_offset_pivot(
        statesprite, frame.pivot_x, frame.pivot_y
    );
}

function animsprite_internal_alternate_choose(animsprite, loop) {
    if (animsprite.alternate_size < 2) return;
    if (loop && !animsprite.alternate_per_loop) return;

    let index;

    if (animsprite.alternate_index < 0) {
        index = math2d_random_int(0, animsprite.alternate_size - 1);
    } else {
        animsprite.alternate_index++;
        if (animsprite.alternate_index >= animsprite.alternate_size) animsprite.alternate_index = 0;
        index = animsprite.alternate_index;
    }


    let alternate = animsprite.alternate_set[index];

    animsprite.current_offset = alternate.index;
    animsprite.frame_count = alternate.length;
}


function animsprite_internal_init(name, loop, frame_rate) {
    let frame_time = frame_rate > 0 ? (1000.0 / frame_rate) : 0;
    if (name) name = strdup(name);

    let animsprite = {
        id: ANIMSPRITE_IDS++,
        name: name,

        frames: null,
        frame_count: 0,

        frame_time: frame_time,
        length: 0,

        loop: loop,
        loop_progress: 0,
        has_looped: 0,
        disable_loop: 0,

        progress: 0,

        current_index: 0,
        current_offset: 0,

        delay: 0,
        delay_progress: 0,
        delay_active: 0,

        is_empty: 0,

        alternate_set: null,
        alternate_size: 0,
        alternate_per_loop: 0,
        alternate_index: -1,

        macroexecutor: null,
        tweenlerp: null,
    };

    ANIMSPRITE_POOL.set(animsprite.id, animsprite);
    return animsprite;
}

