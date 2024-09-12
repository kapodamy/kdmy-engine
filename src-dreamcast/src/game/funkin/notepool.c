#include "game/funkin/notepool.h"

#include "game/funkin/note.h"
#include "game/funkin/strums.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "modelholder.h"
#include "stringutils.h"


static ModelHolder notepool_internal_load(const char* model_src);
static void notepool_internal_alpha_set(NotePool notepool, void (*callback)(Note, float), float alpha);


NotePool notepool_init(DistributionNote* notes, int32_t notes_size, float dimmen, float invdimmen, ScrollDirection scroll_direction) {
    const DistributionNote* DEFAULT_NOTES = STRUMS_DEFAULT_DISTRIBUTION.notes;
    const int32_t DEFAULT_NOTES_SIZE = STRUMS_DEFAULT_DISTRIBUTION.notes_size;

    NotePool notepool = malloc_chk(sizeof(struct NotePool_s));
    malloc_assert(notepool, NotePool);

    *notepool = (struct NotePool_s){
        .size = notes_size,
        .drawables = malloc_for_array(Note, notes_size),
        .models_custom_sick_effect = malloc_for_array(ModelHolder, notes_size),
        .attributes = malloc_for_array(NoteAttribute, notes_size)
    };

    ModelHolder* modelholders = malloc_for_array(ModelHolder, notes_size);

    for (int32_t i = 0; i < notepool->size; i++) {
        DistributionNote* note_definition = &notes[i];

        notepool->attributes[i] = (NoteAttribute){
            .hurt_ratio = 1.0f,
            .heal_ratio = 1.0f,
            .ignore_hit = note_definition->ignore_hit,
            .ignore_miss = note_definition->ignore_miss,
            .can_kill_on_hit = note_definition->can_kill_on_hit,
            .custom_sick_effect_name = string_duplicate(note_definition->sick_effect_state_name)
        };

        if (math2d_is_float_Inf(note_definition->damage_ratio))
            notepool->attributes[i].hurt_ratio = note_definition->damage_ratio;

        if (math2d_is_float_Inf(note_definition->heal_ratio))
            notepool->attributes[i].heal_ratio = note_definition->heal_ratio;


        notepool->models_custom_sick_effect[i] = notepool_internal_load(
            note_definition->custom_sick_effect_model_src
        );

        if (string_is_empty(note_definition->model_src)) {
            modelholders[i] = NULL; // invisible note
        } else if (note_definition->model_src == NULL && i < DEFAULT_NOTES_SIZE) {
            modelholders[i] = modelholder_init(DEFAULT_NOTES[i].model_src);
        } else {
            modelholders[i] = modelholder_init(note_definition->model_src);
        }

        if (!modelholders[i]) {
            // this note will be invisible
            notepool->drawables[i] = NULL;
            continue;
        }

        notepool->drawables[i] = note_init(note_definition->name, dimmen, invdimmen);
        note_set_scoll_direction(notepool->drawables[i], scroll_direction);
        note_state_add(notepool->drawables[i], modelholders[i], NULL);
        note_state_toggle(notepool->drawables[i], NULL);
    }

    for (int32_t i = 0; i < notes_size; i++) modelholder_destroy(&modelholders[i]);
    free_chk(modelholders);

    return notepool;
}

void notepool_destroy(NotePool* notepool_ptr) {
    if (!notepool_ptr || !*notepool_ptr) return;

    NotePool notepool = *notepool_ptr;

    for (int32_t i = 0; i < notepool->size; i++) {
        if (notepool->drawables[i])
            note_destroy(&notepool->drawables[i]);
        if (notepool->models_custom_sick_effect[i])
            modelholder_destroy(&notepool->models_custom_sick_effect[i]);
    }

    free_chk(notepool->drawables);
    free_chk(notepool->models_custom_sick_effect);
    free_chk(notepool->attributes);

    free_chk(notepool);
    *notepool_ptr = NULL;
}


void notepool_change_scroll_direction(NotePool notepool, ScrollDirection scroll_direction) {
    for (int32_t i = 0; i < notepool->size; i++) {
        if (notepool->drawables[i])
            note_set_scoll_direction(notepool->drawables[i], scroll_direction);
    }
}

void notepool_change_alpha_alone(NotePool notepool, float alpha) {
    notepool_internal_alpha_set(notepool, note_set_alpha_alone, alpha);
}

void notepool_change_alpha_sustain(NotePool notepool, float alpha) {
    notepool_internal_alpha_set(notepool, note_set_alpha_sustain, alpha);
}

void notepool_change_alpha(NotePool notepool, float alpha) {
    notepool_internal_alpha_set(notepool, note_set_alpha, alpha);
}


int32_t notepool_add_state(NotePool notepool, ModelHolder modelholder, const char* state_name) {
    int32_t success = 0;
    for (int32_t i = 0; i < notepool->size; i++) {
        if (notepool->drawables[i])
            success += note_state_add(notepool->drawables[i], modelholder, state_name);
    }
    return success;
}


static ModelHolder notepool_internal_load(const char* model_src) {
    if (!model_src) return NULL;
    return modelholder_init(model_src);
}

static void notepool_internal_alpha_set(NotePool notepool, void (*callback)(Note, float), float alpha) {
    for (int32_t i = 0; i < notepool->size; i++) {
        if (notepool->drawables[i])
            callback(notepool->drawables[i], alpha);
    }
}
