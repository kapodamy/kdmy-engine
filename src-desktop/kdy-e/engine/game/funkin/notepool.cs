using System;


namespace Engine.Game {

    public class NoteAttribute {
        public float hurt_ratio;
        public float heal_ratio;
        public bool ignore_hit;
        public bool ignore_miss;
        public bool can_kill_on_hit;
        public string custom_sick_effect_name;
        public bool is_special;
    }

    public class NotePool {

        public readonly int size;
        public readonly Note[] drawables;
        public readonly ModelHolder[] models_custom_sick_effect;
        public readonly NoteAttribute[] attributes;

        public NotePool(DistributionNote[] notes, int notes_size, float dimmen, float invdimmen, ScrollDirection scroll_direction) {
            DistributionNote[] DEFAULT_NOTES = Strums.DEFAULT_DISTRIBUTION.notes;
            int DEFAULT_NOTES_SIZE = Strums.DEFAULT_DISTRIBUTION.notes_size;


            this.size = notes_size;
            this.drawables = new Note[notes_size];
            this.models_custom_sick_effect = new ModelHolder[notes_size];
            this.attributes = new NoteAttribute[notes_size];


            ModelHolder[] modelholders = new ModelHolder[notes_size];

            for (int i = 0 ; i < this.size ; i++) {
                DistributionNote note_definition = notes[i];

                this.attributes[i] = new NoteAttribute() {
                    hurt_ratio = 1.0f,
                    heal_ratio = 1.0f,
                    ignore_hit = !!note_definition.ignore_hit,
                    ignore_miss = !!note_definition.ignore_miss,
                    can_kill_on_hit = note_definition.can_kill_on_hit,
                    custom_sick_effect_name = note_definition.sick_effect_state_name,
                    is_special = note_definition.is_special
                };

                if (!Single.IsNaN(note_definition.damage_ratio))
                    this.attributes[i].hurt_ratio = note_definition.damage_ratio;

                if (!Single.IsNaN(note_definition.heal_ratio))
                    this.attributes[i].heal_ratio = note_definition.heal_ratio;


                this.models_custom_sick_effect[i] = NotePool.InternalLoad(
                    note_definition.custom_sick_effect_model_src
                );

                if (note_definition.model_src != null && note_definition.model_src.Length < 1) {
                    modelholders[i] = null;// invisible note
                } else if (note_definition.model_src == null && i < DEFAULT_NOTES_SIZE) {
                    modelholders[i] = ModelHolder.Init(DEFAULT_NOTES[i].model_src);
                } else {
                    modelholders[i] = ModelHolder.Init(note_definition.model_src);
                }

                if (modelholders[i] == null) {
                    // this note will be invisible
                    this.drawables[i] = null;
                    continue;
                }

                this.drawables[i] = new Note(note_definition.name, dimmen, invdimmen);
                this.drawables[i].SetScollDirection(scroll_direction);
                this.drawables[i].StateAdd(modelholders[i], null);
                this.drawables[i].StateToggle(null);
            }

            for (int i = 0 ; i < notes_size ; i++) modelholders[i].Destroy();
            //free(modelholders);


        }

        public void Destroy() {
            for (int i = 0 ; i < this.size ; i++) {
                if (this.drawables[i] != null)
                    this.drawables[i].Destroy();
                if (this.models_custom_sick_effect[i] != null)
                    this.models_custom_sick_effect[i].Destroy();
            }

            //free(this.drawable);
            //free(this.models);
            //free(this.models_custom_sick_effect);
            //free(this.attributes);
            //free(this);
        }


        public void ChangeScrollDirection(ScrollDirection scroll_direction) {
            for (int i = 0 ; i < this.size ; i++) {
                if (this.drawables[i] != null)
                    this.drawables[i].SetScollDirection(scroll_direction);

            }
        }

        public void ChangeAlphaAlone(float alpha) {
            for (int i = 0 ; i < this.size ; i++) {
                if (this.drawables[i] != null)
                    this.drawables[i].SetAlphaAlone(alpha);
            }
        }

        public void ChangeAlphaSustain(float alpha) {
            for (int i = 0 ; i < this.size ; i++) {
                if (this.drawables[i] != null)
                    this.drawables[i].SetAlphaSustain(alpha);
            }
        }

        public void ChangeAlpha(float alpha) {
            for (int i = 0 ; i < this.size ; i++) {
                if (this.drawables[i] != null)
                    this.drawables[i].SetAlpha(alpha);
            }
        }


        public int AddState(ModelHolder modelholder, string state_name) {
            int success = 0;
            for (int i = 0 ; i < this.size ; i++) {
                if (this.drawables[i] != null)
                    success += this.drawables[i].StateAdd(modelholder, state_name) ? 1 : 0;
            }
            return success;
        }



        private static ModelHolder InternalLoad(string model_src) {
            if (String.IsNullOrEmpty(model_src)) return null;
            return ModelHolder.Init(model_src);
        }

    }

}
