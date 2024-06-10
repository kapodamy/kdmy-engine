"use strict";

async function notepool_init(notes, notes_size, dimmen, invdimmen, scroll_direction) {
    const DEFAULT_NOTES = STRUMS_DEFAULT_DISTRIBUTION.notes;
    const DEFAULT_NOTES_SIZE = STRUMS_DEFAULT_DISTRIBUTION.notes_size;

    let notepool = {
        size: notes_size,
        drawables: new Array(notes_size),
        models_custom_sick_effect: new Array(notes_size),
        attributes: new Array(notes_size)
    };

    let modelholders = new Array(notes_size);

    for (let i = 0; i < notepool.size; i++) {
        let note_definition = notes[i];

        notepool.attributes[i] = {
            hurt_ratio: 1.0,
            heal_ratio: 1.0,
            ignore_hit: !!note_definition.ignore_hit,
            ignore_miss: !!note_definition.ignore_miss,
            can_kill_on_hit: !!note_definition.can_kill_on_hit,
            custom_sick_effect_name: strdup(note_definition.sick_effect_state_name)
        };

        if (!Number.isNaN(note_definition.damage_ratio))
            notepool.attributes[i].hurt_ratio = note_definition.damage_ratio;

        if (!Number.isNaN(note_definition.heal_ratio))
            notepool.attributes[i].heal_ratio = note_definition.heal_ratio;


        notepool.models_custom_sick_effect[i] = await notepool_internal_load(
            note_definition.custom_sick_effect_model_src
        );

        if (note_definition.model_src != null && note_definition.model_src.length < 1) {
            modelholders[i] = null;// invisible note
        } else if (note_definition.model_src == null && i < DEFAULT_NOTES_SIZE) {
            modelholders[i] = await modelholder_init(DEFAULT_NOTES[i].model_src);
        } else {
            modelholders[i] = await modelholder_init(note_definition.model_src);
        }

        if (!modelholders[i]) {
            // this note will be invisible
            notepool.drawables[i] = null;
            continue;
        }

        notepool.drawables[i] = note_init(note_definition.name, dimmen, invdimmen);
        note_set_scoll_direction(notepool.drawables[i], scroll_direction);
        note_state_add(notepool.drawables[i], modelholders[i], null);
        note_state_toggle(notepool.drawables[i], null);
    }

    for (let i = 0; i < notes_size; i++) modelholder_destroy(modelholders[i]);
    modelholders = undefined;

    return notepool;
}

function notepool_destroy(notepool) {
    for (let i = 0; i < notepool.size; i++) {
        if (notepool.drawables[i])
            note_destroy(notepool.drawables[i]);
        if (notepool.models_custom_sick_effect[i])
            modelholder_destroy(notepool.models_custom_sick_effect[i]);
    }

    notepool.drawables = undefined;
    notepool.models_custom_sick_effect = undefined;
    notepool.attributes = undefined;
    notepool = undefined;
}


function notepool_change_scroll_direction(notepool, scroll_direction) {
    for (let i = 0; i < notepool.size; i++) {
        if (notepool.drawables[i])
            note_set_scoll_direction(notepool.drawables[i], scroll_direction);

    }
}

function notepool_change_alpha_alone(notepool, alpha) {
    notepool_internal_alpha_set(notepool, note_set_alpha_alone, alpha);
}

function notepool_change_alpha_sustain(notepool, alpha) {
    notepool_internal_alpha_set(notepool, note_set_alpha_sustain, alpha);
}

function notepool_change_alpha(notepool, alpha) {
    notepool_internal_alpha_set(notepool, note_set_alpha, alpha);
}


function notepool_add_state(notepool, modelholder, state_name) {
    let success = 0;
    for (let i = 0; i < notepool.size; i++) {
        if (notepool.drawables[i])
            success += note_state_add(notepool.drawables[i], modelholder, state_name);
    }
    return success;
}


/** @async */
function notepool_internal_load(model_src) {
    if (!model_src) return null;
    return modelholder_init(model_src);
}

function notepool_internal_alpha_set(notepool, callback, alpha) {
    for (let i = 0; i < notepool.size; i++) {
        if (notepool.drawables[i])
            callback(notepool.drawables[i], alpha);
    }
}

