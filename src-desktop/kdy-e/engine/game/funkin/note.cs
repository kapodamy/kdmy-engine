using System;
using Engine.Animation;
using Engine.Game.Common;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;


namespace Engine.Game {

    public enum ScrollDirection {
        UPSCROLL, DOWNSCROLL, LEFTSCROLL, RIGHTSCROLL
    }

    public class Note {

        private const string SUFFIX_ALONE = "alone";
        private const string SUFFIX_HOLD = "hold";
        private const string SUFFIX_TAIL = "tail";
        private const string ALTERNATE_MINE = "mine";

        //private const bool SUSTAIN_BODY_HEIGHT_USE_INTEGERS = false;
        private const float SUSTAIN_BODY_OVELAPING_PIXELS = 0.5f;
        public const int STRUCT_ELEMENTS_COUNT = 4;// [timestamp, note_id, duration, "reserved"]
        private const string INTERNAL_STATE_NAME = "____note-state___";


        private StateSprite sprite_alone;
        private StateSprite sprite_hold;
        private StateSprite sprite_tail;
        private float current_dimmen;
        private float dimmen;
        private float invdimmen;
        private float dimmen_alone;
        private float dimmen_alone_half;
        private float height_hold;
        private float height_tail;
        private float offset_hold;
        private float offset_tail;
        private ScrollDirection direction;
        private Modifier modifier_sustain;
        private string name;
        private float alpha_sustain;
        private float alpha_alone;
        private LinkedList<NoteState> internal_states;
        private string current_state_name;
        private bool has_part_alone;
        private bool has_part_hold;
        private bool has_part_tail;



        public Note(string name, float dimmen, float invdimmen) {

            this.sprite_alone = StateSprite.InitFromTexture(null);
            this.sprite_hold = StateSprite.InitFromTexture(null);
            this.sprite_tail = StateSprite.InitFromTexture(null);

            this.current_dimmen = dimmen;
            this.dimmen = dimmen;
            this.invdimmen = invdimmen;

            // assign some default values
            this.dimmen_alone = dimmen;
            this.dimmen_alone_half = dimmen / 2f;

            this.height_hold = 44;
            this.height_tail = 64;

            this.offset_hold = 15;
            this.offset_tail = 15;

            this.direction = ScrollDirection.UPSCROLL;

            this.modifier_sustain = new Modifier() { };

            this.name = name;

            this.alpha_sustain = 1.0f;
            this.alpha_alone = 1.0f;

            this.internal_states = new LinkedList<NoteState>();
            this.current_state_name = INTERNAL_STATE_NAME;

            this.has_part_alone = false;
            this.has_part_hold = false;
            this.has_part_tail = false;


            this.modifier_sustain.Clear();

            this.sprite_alone.SetVisible(false);
            this.sprite_hold.SetVisible(false);
            this.sprite_tail.SetVisible(false);


        }

        public void Destroy() {
            this.sprite_alone.Destroy();
            this.sprite_hold.Destroy();
            this.sprite_tail.Destroy();

            this.internal_states.Destroy2(Note.InternalDestroyInternalState);

            // note: do not dispose "this.current_state_name" is part of "this.internal_states"

            this.name = null;
            //free(note);
        }

        public void SetScollDirection(ScrollDirection direction) {
            switch (direction) {
                case ScrollDirection.LEFTSCROLL:
                    this.modifier_sustain.rotate = -90;
                    break;
                case ScrollDirection.RIGHTSCROLL:
                    this.modifier_sustain.rotate = 90;
                    break;
                case ScrollDirection.DOWNSCROLL:
                    this.modifier_sustain.rotate = 180;
                    break;
                case ScrollDirection.UPSCROLL:
                    this.modifier_sustain.rotate = 0;
                    break;
                default:
                    return;
            }

            switch (direction) {
                case ScrollDirection.LEFTSCROLL:
                case ScrollDirection.RIGHTSCROLL:
                    this.current_dimmen = this.invdimmen;
                    break;
                case ScrollDirection.DOWNSCROLL:
                case ScrollDirection.UPSCROLL:
                    this.current_dimmen = this.dimmen;
                    break;
            }

            this.direction = direction;

            if (direction != ScrollDirection.UPSCROLL) {
                this.modifier_sustain.rotate *= Math2D.DEG_TO_RAD;
                this.modifier_sustain.rotate_pivot_enabled = true;
                this.modifier_sustain.rotate_pivot_u = 0f;
                this.modifier_sustain.rotate_pivot_v = 0f;
                this.modifier_sustain.width = 0f;
                this.modifier_sustain.height = 0f;
                this.modifier_sustain.translate_rotation = true;
            }

            // recalculate all sprite states
            foreach (NoteState state_note in this.internal_states) {
                StateSpriteState state_alone = InternalGetStatespriteState(this.sprite_alone, state_note.name);
                StateSpriteState state_hold = InternalGetStatespriteState(this.sprite_hold, state_note.name);
                StateSpriteState state_tail = InternalGetStatespriteState(this.sprite_tail, state_note.name);
                InternalResizeSprites(state_note, state_alone, state_hold, state_tail);
            }

            // reload the current sprite state
            foreach (NoteState note_state in this.internal_states) {
                if (note_state.name == this.current_state_name) {
                    this.height_hold = note_state.height_hold;
                    this.offset_hold = note_state.offset_hold;
                    this.height_tail = note_state.height_tail;
                    this.offset_tail = note_state.offset_tail;
                    break;
                }
            }
        }

        public void SetAlpha(float alpha) {
            SetAlphaAlone(alpha);
            SetAlphaSustain(alpha);
        }

        public void SetAlphaAlone(float alpha) {
            this.alpha_alone = alpha;
        }

        public void SetAlphaSustain(float alpha) {
            this.alpha_sustain = alpha;
        }

        public int Animate(float elapsed) {
            int res = 0;
            if (this.sprite_alone != null) res += this.sprite_alone.Animate(elapsed);
            if (this.sprite_hold != null) res += this.sprite_hold.Animate(elapsed);
            if (this.sprite_tail != null) res += this.sprite_tail.Animate(elapsed);
            return res;
        }

        public string GetName() {
            return this.name;// in C return the string pointer
        }

        public void Draw(PVRContext pvrctx, double scroll_velocity, double x, double y, double duration_ms, bool only_body) {
            //
            // Note:
            //      Scroll velocity should be according to the UI layout viewport (or the screen resolution)
            //      and the song chart speed should be already applied
            //
            double length = Math.Abs(duration_ms * scroll_velocity);// note length in pixels
            double sustain_length = length - this.dimmen_alone;

            // save PVR context and apply the modifier to alone, hold and tail sprites
            pvrctx.Save();

            float dimmen_alone_half = this.has_part_alone ? this.dimmen_alone_half : 0.0f;

            // draw the sustain body (hold & tail)
            if (sustain_length > 0.0 && this.has_part_hold && this.has_part_tail) {
                if (this.direction != ScrollDirection.UPSCROLL) {
                    switch (this.direction) {
                        case ScrollDirection.DOWNSCROLL:
                            y -= sustain_length;
                            break;
                        case ScrollDirection.RIGHTSCROLL:
                            x -= sustain_length;
                            break;
                    }

                    this.modifier_sustain.x = (float)x;
                    this.modifier_sustain.y = (float)y;
                }

                double draw_x = x + this.offset_hold;
                double draw_y = y;
                double tail_correction;
                float height_hold = this.height_hold;
                double height_tail = this.height_tail;

                switch (this.direction) {
                    case ScrollDirection.RIGHTSCROLL:
                    case ScrollDirection.DOWNSCROLL:
                        draw_y -= sustain_length + dimmen_alone_half;
                        tail_correction = 0.0;
                        break;
                    default:
                        draw_y += dimmen_alone_half;
                        tail_correction = length;
                        break;
                }

                // crop sprite_tail if necessary
                double hold_length = sustain_length + dimmen_alone_half;
                if (hold_length < height_tail) {
                    // crop the top part of the sprite
                    double offset = height_tail - hold_length;
                    height_tail = hold_length;
                    this.sprite_tail.CropEnable(true);
                    this.sprite_tail.Crop(0f, (float)offset, -1f, -1f);
                }

                // reserve space for the sprite_tail
                hold_length -= height_tail;
                // Rendering workaround to avoid gaps
                hold_length += Note.SUSTAIN_BODY_OVELAPING_PIXELS;
                height_hold -= Note.SUSTAIN_BODY_OVELAPING_PIXELS;

                pvrctx.Save();
                pvrctx.ApplyModifier(this.modifier_sustain);
                pvrctx.global_alpha = this.alpha_sustain;

                while (hold_length > height_hold) {
                    this.sprite_hold.SetDrawLocation((float)draw_x, (float)draw_y);
                    this.sprite_hold.Draw(pvrctx);
                    draw_y += height_hold;
                    hold_length -= height_hold;
                }

                if (hold_length > 0.0) {
                    // crop the bottom part of sprite_hold and draw it
                    this.sprite_hold.CropEnable(true);
                    this.sprite_hold.Crop(0f, 0f, -1f, (float)hold_length);

                    this.sprite_hold.SetDrawLocation((float)draw_x, (float)draw_y);
                    this.sprite_hold.Draw(pvrctx);
                }

                // draw sprite_tail
                draw_x = x + this.offset_tail;
                draw_y = y + tail_correction - height_tail;
                this.sprite_tail.SetDrawLocation((float)draw_x, (float)draw_y);
                this.sprite_tail.Draw(pvrctx);

                // disable crop sustain body (hold & tail)
                this.sprite_hold.CropEnable(false);
                this.sprite_tail.CropEnable(false);

                pvrctx.Restore();
            }

            // draw sprite_alone
            if (!only_body && this.has_part_alone) {
                if (sustain_length > 0.0) {
                    if (this.direction == ScrollDirection.DOWNSCROLL)
                        y += sustain_length;
                    else if (this.direction == ScrollDirection.RIGHTSCROLL)
                        x += sustain_length;
                }

                pvrctx.global_alpha = this.alpha_alone;
                this.sprite_alone.SetDrawLocation((float)x, (float)y);
                this.sprite_alone.Draw(pvrctx);
            }

            pvrctx.Restore();
        }


        public bool StateAdd(ModelHolder modelholder, string state_name) {
            //
            // build every part of the note
            // all sprites are hidden to avoid getting drawn by the PVR graphics backend
            // we draw those sprites manually in Draw()
            //

            StateSpriteState state_alone = Note.InternalLoadPart(
                this.sprite_alone, this.name, Note.SUFFIX_ALONE, modelholder, state_name
            );
            StateSpriteState state_hold = Note.InternalLoadPart(
                this.sprite_hold, this.name, Note.SUFFIX_HOLD, modelholder, state_name
            );
            StateSpriteState state_tail = Note.InternalLoadPart(
                this.sprite_tail, this.name, Note.SUFFIX_TAIL, modelholder, state_name
            );

            if (state_alone == null && state_hold == null && state_tail == null) return false;

            NoteState state_note = new NoteState() { name = null, height_hold = 0f, offset_hold = 0f, height_tail = 0f, offset_tail = 0f };
            if (state_name != null) state_note.name = state_name;

            this.internal_states.AddItem(state_note);
            InternalResizeSprites(state_note, state_alone, state_hold, state_tail);

            return true;
        }

        public int StateToggle(string state_name) {
            if (this.current_state_name == state_name) return 4;

            int res = 0;

            foreach (NoteState note_state in this.internal_states) {
                if (note_state.name == state_name) {
                    this.height_hold = note_state.height_hold;
                    this.offset_hold = note_state.offset_hold;
                    this.height_tail = note_state.height_tail;
                    this.offset_tail = note_state.offset_tail;
                    this.current_state_name = note_state.name;
                    res = 1;
                    break;
                }
            }

            if (res < 1) return 0;

            this.has_part_alone = this.sprite_alone.StateToggle(state_name); if (this.has_part_alone) res++;
            this.has_part_hold = this.sprite_hold.StateToggle(state_name); if (this.has_part_hold) res++;
            this.has_part_tail = this.sprite_tail.StateToggle(state_name); if (this.has_part_tail) res++;

            return res;
        }

        public StateSprite PeekAloneStateSprite() {
            return this.sprite_alone;
        }


        private static StateSpriteState InternalLoadPart(StateSprite part, string name, string suffix, ModelHolder modelholder, string state_name) {
            Atlas atlas = modelholder.GetAtlas();
            AnimList animlist = modelholder.GetAnimlist();
            Texture texture = modelholder.GetTexture(false);
            uint rgb8_color = modelholder.GetVertexColor();

            string animation_name = "";
            if (!String.IsNullOrEmpty(name)) animation_name = name;
            if (!String.IsNullOrEmpty(suffix)) animation_name += " " + suffix;
            if (!String.IsNullOrEmpty(state_name)) animation_name += " " + state_name;


            //
            // Attempt to load the arrow part (texture, frame & animation)
            //
            AnimSprite animsprite;


            // read the animation from the animlist (if necessary)
            if (animlist != null) {
                AnimListItem animlist_item = animlist.GetAnimation(animation_name);
                if (animlist_item != null) {
                    animsprite = AnimSprite.Init(animlist_item);
                    goto L_build_state;
                }
            }

            // fallback, build the animation using atlas frames directly
            float framerate = atlas.GetGlyphFPS();
            if (framerate <= 0.0) framerate = Funkin.DEFAULT_ANIMATIONS_FRAMERATE;

            animsprite = AnimSprite.InitFromAtlas(framerate, 0, atlas, animation_name, true);

            if (animsprite != null)
                goto L_build_state;

            // the arrow is not animated, build an static animation
            AtlasEntry atlas_entry = atlas.GetEntry(animation_name);
            if (atlas_entry != null) {
                animsprite = AnimSprite.InitFromAtlasEntry(atlas_entry, true, framerate);
                goto L_build_state;
            }

            /*if (!String.IsNullOrEmpty(state_name)) {
                //free(animation_name);

                // try again but without the state name, can fail if the texture supplied is custom
                // with a hold duration (alone + hold + tail)
                StateSpriteState statesprite_state = Note.InternalCreateArrowPart(
                    part, name, suffix, atlas, animlist, texture, rgb8_color, null
                );

                // set the state name (null was used, before)
                statesprite_state.state_name = state_name;

                return statesprite_state;
            }*/

            // this part of the arrow is incomplete
            animsprite = null;



L_build_state:
            if (animsprite == null) {
                //free(animation_name);
                return null;
            }

            atlas_entry = atlas.GetEntryWithNumberSuffix(animation_name);
            if (atlas_entry == null) atlas_entry = atlas.GetEntry(animation_name);

            //free(animation_name);

            StateSpriteState state = part.StateAdd2(texture, animsprite, atlas_entry, rgb8_color, state_name);
            if (state == null) animsprite.Destroy();

            return state;
        }

        private void InternalResizeSprites(NoteState state_note, StateSpriteState state_alone, StateSpriteState state_hold, StateSpriteState state_tail) {
            //
            // confuse part, set the desired width and height of the alone sprite
            //
            float ref_width, ref_height, ref_invdimmen;
            bool inverse = false;

            switch (this.direction) {
                case ScrollDirection.UPSCROLL:
                case ScrollDirection.DOWNSCROLL:
                    ref_width = this.invdimmen;
                    ref_height = this.dimmen;
                    ref_invdimmen = this.invdimmen;
                    break;
                case ScrollDirection.LEFTSCROLL:
                case ScrollDirection.RIGHTSCROLL:
                    ref_width = this.dimmen;
                    ref_height = this.invdimmen;
                    ref_invdimmen = this.invdimmen;
                    break;
                default:
                    return;
            }

            if (this.direction == ScrollDirection.LEFTSCROLL || this.direction == ScrollDirection.DOWNSCROLL) inverse = true;

            float alone_size_width = ref_width, alone_size_height = ref_height;

            // obtain the alone sprite original dimmensions
            if (state_alone != null)
                ImgUtils.GetStateSpriteOriginalSize(state_alone, ref alone_size_width, ref alone_size_height);
            else
                Console.Error.WriteLine("[WARN] note_internal_resize_sprites() warning state_alone was NULL");

            // resize the sustain body sprites using the same scale as alone sprite
            float scale_factor;
            if (this.direction == ScrollDirection.UPSCROLL || this.direction == ScrollDirection.DOWNSCROLL) {
                scale_factor = ref_height / alone_size_width;
            } else {
                scale_factor = ref_width / alone_size_height;
            }


            //
            // resize sprites
            //

            if (state_alone != null) {
                float draw_width = ref_width, draw_height = ref_height;
                ImgUtils.CalcSize(alone_size_width, alone_size_height, ref_width, ref_height, out draw_width, out draw_height);
                state_alone.draw_width = draw_width;
                state_alone.draw_height = draw_height;

                state_alone.offset_x = (ref_width - state_alone.draw_width) / 2f;
                state_alone.offset_y = (ref_height - state_alone.draw_height) / 2f;
            }

            if (state_hold != null) {
                Note.InternalResizeSustainPart(state_hold, scale_factor);
                state_note.height_hold = state_hold.draw_height;
                state_note.offset_hold = (ref_invdimmen - state_hold.draw_width) / 2.0f;
                if (inverse)
                    state_note.offset_hold = (state_note.offset_hold + state_hold.draw_width) * -1f;
            } else {
                state_note.height_hold = 0f;
                state_note.offset_hold = 0f;
            }

            if (state_tail != null) {
                Note.InternalResizeSustainPart(state_tail, scale_factor);
                state_note.height_tail = state_tail.draw_height;
                state_note.offset_tail = (ref_invdimmen - state_tail.draw_width) / 2.0f;
                if (inverse)
                    state_note.offset_tail = (state_note.offset_tail + state_tail.draw_width) * -1f;
            } else {
                state_note.height_tail = 0f;
                state_note.offset_tail = 0f;
            }
        }

        private static void InternalResizeSustainPart(StateSpriteState statesprite_state, float scale) {
            float applied_draw_width = 0f, applied_draw_height = 0f;
            // obtain the size of this part of the note
            ImgUtils.GetStateSpriteOriginalSize(statesprite_state, ref applied_draw_width, ref applied_draw_height);

            // keep at the same scale as sprite_alone
            statesprite_state.draw_width = applied_draw_width * scale;
            statesprite_state.draw_height = applied_draw_height * scale;
        }

        private static void InternalDestroyInternalState(NoteState internal_state) {
            //if (internal_state.name) free(internal_state.name);
            //free(internal_state);
        }

        private static StateSpriteState InternalGetStatespriteState(StateSprite statesprite, string state_name) {
            LinkedList<StateSpriteState> states = statesprite.StateList();
            foreach (StateSpriteState statesprite_state in states) {
                if (statesprite_state.state_name == state_name)
                    return statesprite_state;
            }
            return null;
        }


        private class NoteState {
            public string name;
            public float height_hold;
            public float offset_hold;
            public float height_tail;
            public float offset_tail;
        }

    }

}
