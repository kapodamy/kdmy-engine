using System;
using Engine.Animation;
using Engine.Externals.LuaScriptInterop;
using Engine.Game.Common;
using Engine.Game.Gameplay;
using Engine.Game.Gameplay.Helpers;
using Engine.Image;
using Engine.Platform;
using Engine.Utils;

//
// it took over four months to create this, the most expensive part of the project
//
namespace Engine.Game {
    public enum StrumPressState : int {
        NONE = 0,
        HIT,
        HIT_SUSTAIN,
        PENALTY_NOTE,
        PENALTY_HIT,
        MISS,
        INVALID
    }

    public class Strum {

        public enum NoteState {
            CLEAR,
            MISS,
            PENDING,
            HOLD,
            RELEASE
        };

        public class StrumNote {
            public double timestamp;
            public int id;
            public double duration;
            public bool alt_anim;
            public NoteState state;
            public double hit_diff;
            public double release_time;
            public GamepadButtons release_button;
            public int previous_quarter;
            public double custom_data;
            public bool hit_on_penality;

            public StrumNote(ChartNote chart_note, int[] map, int map_size) {
                int id = 0;
                for (int i = 0 ; i < map_size ; i++) {
                    if (map[i] == (int)chart_note.direction) {
                        id = i;
                        break;
                    }
                }

                // base note info
                this.id = id;
                this.timestamp = chart_note.timestamp;
                this.duration = chart_note.duration;
                this.alt_anim = chart_note.alt_anim;
                this.custom_data = chart_note.data;
                this.hit_on_penality = false;

                this.state = NoteState.PENDING;
                this.hit_diff = Double.NaN;
                this.release_time = -1.0;

                this.release_button = GamepadButtons.NOTHING;

                this.previous_quarter = 0;
            }

            public bool IsSustain { get => this.duration > 0; }

            public double EndTimestamp { get => this.timestamp + this.duration; }

            public static readonly Comparer SortCallback = new Comparer();


            public class Comparer : System.Collections.Generic.IComparer<StrumNote> {
                public int Compare(StrumNote note1, StrumNote note2) {
                    return note1.timestamp.CompareTo(note2.timestamp);
                }
            }
        }

        private enum StrumMarkerState {
            INVALID,
            NOTHING,
            CONFIRM,
            PRESS
        }

        private enum StrumExtraState {
            NONE,
            WAIT,
            COMPLETED
        }


        private const string MARKER_SUFFIX_CONFIRM = "confirm";
        private const string MARKER_SUFFIX_MARKER = "marker";
        private const string MARKER_SUFFIX_HIT = "press";
        private const string SICK_EFFECT_SUFFIX = "splash";
        private const string BACKGROUND_SUFFIX = "background";

        public enum StrumScriptTarget {
            MARKER,
            SICK_EFFECT,
            BACKGROUND,
            STRUM_LINE,
            NOTE,
            ALL
        }

        public enum StrumScriptOn {
            HIT_DOWN,
            HIT_UP,
            MISS,
            PENALITY,
            IDLE,
            ALL
        }

        /** Default sprite size of the sick effect in relation to the marker sprite size */
        private const float DEFAULT_SICK_EFFECT_RATIO = 2f;

        /** Number of notes to draw after the marker */
        public const float DRAW_PAST_NOTES = 1.1f;

        /** Marker used in auto-scroll */
        private const StrumMarkerState AUTO_SCROLL_MARKER_STATE = StrumMarkerState.CONFIRM;

        /** Increases the marker duration 5% */
        public const float ADDITIONAL_MARKER_DURATION_RATIO = 1.05f;



        private int strum_id;
        private string strum_name;
        private float sick_effect_ratio;
        private StateSprite sprite_marker_nothing;
        private StateSprite sprite_marker_confirm;
        private StateSprite sprite_marker_press;
        private StateSprite sprite_sick_effect;
        private StateSprite sprite_background;
        internal StrumNote[] chart_notes;
        private int chart_notes_size;
        private Note[] drawable_notes;
        internal NoteAttribute[] attribute_notes;
        internal int[] chart_notes_id_map;
        private int chart_notes_id_map_size;
        private double scroll_velocity_base;
        private double scroll_velocity;
        private bool scroll_is_vertical;
        private bool scroll_is_inverse;
        private double scroll_window;
        private double last_song_timestamp;
        private int notes_peek_index;
        private int notes_render_index;
        private int press_state_changes;
        private StrumPressState press_state;
        private bool press_state_use_alt_anim;
        private StrumMarkerState marker_state;
        private bool marker_state_changed;
        private StrumMarkerState marker_sick_state;
        private string marker_sick_state_name;
        private string selected_sick_effect_state;
        private float dimmen_length;
        private float dimmen_opposite;
        private float dimmen_marker;
        private float marker_duration;
        private float marker_duration_quarter;
        private float minimum_sustain_duration;
        private float marker_duration_multiplier;
        private double key_test_limit;
        private Modifier modifier;
        private bool enable_sick_effect;
        private bool enable_sick_effect_draw;
        private bool enable_background;
        private bool keep_aspect_ratio_background;
        private bool markers_scale_keep;
        private float markers_scale;
        private string selected_notes_state;
        private AttachedAnimations animation_strum_line;
        private AttachedAnimations animation_note;
        private AttachedAnimations animation_marker;
        private AttachedAnimations animation_sick_effect;
        private AttachedAnimations animation_background;
        private bool extra_animations_have_penalties;
        private bool extra_animations_have_misses;
        private double auto_scroll_elapsed;
        private ArrayList<StrumNote> sustain_queue;
        private Drawable drawable;
        private double draw_offset_milliseconds;
        public int player_id;
        private float inverse_offset;
        private bool use_fukin_marker_duration;
        private TweenKeyframe tweenkeyframe_note;
        private bool use_beat_synced_idle_and_continous;
        private BeatWatcher beatwatcher;

        public Strum(int id, string name, float x, float y, float marker_dimmen, float invdimmen, float length_dimmen, bool keep_markers_scale) {


            this.strum_id = id;
            this.strum_name = name;

            this.sick_effect_ratio = Strum.DEFAULT_SICK_EFFECT_RATIO;

            this.sprite_marker_nothing = StateSprite.InitFromTexture(null);
            this.sprite_marker_confirm = StateSprite.InitFromTexture(null);
            this.sprite_marker_press = StateSprite.InitFromTexture(null);
            this.sprite_sick_effect = StateSprite.InitFromTexture(null);
            this.sprite_background = StateSprite.InitFromTexture(null);

            this.chart_notes = null;
            this.chart_notes_size = 0;

            this.drawable_notes = null;
            this.attribute_notes = null;
            this.chart_notes_id_map = null;
            this.chart_notes_id_map_size = 0;

            this.scroll_velocity_base = Funkin.CHART_SCROLL_VELOCITY;
            this.scroll_velocity = 0;

            this.scroll_is_vertical = true;
            this.scroll_is_inverse = false;
            this.scroll_window = 0.0;

            this.last_song_timestamp = 0.0;
            this.notes_peek_index = 0;
            this.notes_render_index = 0;

            this.press_state_changes = -1;
            this.press_state = StrumPressState.NONE;
            this.press_state_use_alt_anim = false;

            this.marker_state = StrumMarkerState.NOTHING;
            this.marker_state_changed = false;
            this.marker_sick_state = StrumMarkerState.NOTHING;

            this.marker_sick_state_name = null;
            this.selected_sick_effect_state = null;

            this.dimmen_length = length_dimmen;
            this.dimmen_opposite = invdimmen;

            this.dimmen_marker = marker_dimmen;
            this.marker_duration = 0f;
            this.marker_duration_quarter = 0f;
            this.minimum_sustain_duration = 0f;
            this.marker_duration_multiplier = 1.0f;

            this.key_test_limit = 0.0;

            this.modifier = new Modifier();

            this.enable_sick_effect = true;
            this.enable_sick_effect_draw = true;
            this.enable_background = false;

            this.keep_aspect_ratio_background = true;

            this.markers_scale_keep = keep_markers_scale;
            this.markers_scale = -1.0f;

            this.selected_notes_state = null;

            this.animation_strum_line = new AttachedAnimations() { };
            this.animation_note = new AttachedAnimations() { };
            this.animation_marker = new AttachedAnimations() { };
            this.animation_sick_effect = new AttachedAnimations() { };
            this.animation_background = new AttachedAnimations() { };

            this.extra_animations_have_penalties = false;
            this.extra_animations_have_misses = false;

            this.auto_scroll_elapsed = -1.0;
            this.sustain_queue = new ArrayList<StrumNote>();

            this.drawable = new Drawable(-1, (IDraw)null, (IAnimate)null);
            this.draw_offset_milliseconds = 0.0;
            this.player_id = -1;
            this.inverse_offset = length_dimmen - marker_dimmen;
            this.use_fukin_marker_duration = true;
            this.tweenkeyframe_note = null;
            this.use_beat_synced_idle_and_continous = true;
            this.beatwatcher = new BeatWatcher();


            // set sprites location and modifier location
            UpdateDrawLocation(x, y);

            // set sprites size
            this.sprite_marker_nothing.SetDrawSize(marker_dimmen, marker_dimmen);
            this.sprite_marker_confirm.SetDrawSize(marker_dimmen, marker_dimmen);
            this.sprite_marker_press.SetDrawSize(marker_dimmen, marker_dimmen);
            this.sprite_sick_effect.SetDrawSize(marker_dimmen, marker_dimmen);
            this.sprite_background.SetDrawSize(marker_dimmen, length_dimmen);


            SetScrollSpeed(1.0f);
            SetScrollDirection(ScrollDirection.UPSCROLL);
            SetBpm(100);

        }

        public void Destroy() {
            Luascript.DropShared(this);

            this.sprite_marker_nothing.Destroy();
            this.sprite_marker_confirm.Destroy();
            this.sprite_marker_press.Destroy();
            this.sprite_sick_effect.Destroy();
            this.sprite_background.Destroy();

            //free(this.strum_name);

            //if (this.chart_notes != null) free(this.chart_notes);
            //if (this.drawable_notes != null) free(this.drawable_notes);
            //if (this.attribute_notes != null) free(this.attribute_notes);
            //if (this.chart_notes_id_map != null) free(this.chart_notes_id_map);

            //if (this.selected_notes_state != null) free(this.selected_notes_state);
            //if (this.selected_sick_effect_state != null) free(this.selected_sick_effect_state);

            Strum.InternalExtraDestroyAnimation(this.animation_marker);
            Strum.InternalExtraDestroyAnimation(this.animation_sick_effect);
            Strum.InternalExtraDestroyAnimation(this.animation_background);
            Strum.InternalExtraDestroyAnimation(this.animation_strum_line);
            Strum.InternalExtraDestroyAnimation(this.animation_note);

            if (this.tweenkeyframe_note != null) this.tweenkeyframe_note.Destroy();

            this.sustain_queue.Destroy(false);

            // Note: do not release "this.marker_sick_state_name"

            this.drawable.Destroy();
            //free(this);
        }

        public int SetNotes(Chart chart, DistributionStrum[] strumsdefs, int strumsdefs_size, int player_id, NotePool notepool) {
            if (this.chart_notes != null) {
                // free() previous used resources
                //free(this.chart_notes_id_map);
                //free(this.chart_notes);
                this.chart_notes_size = 0;
                this.key_test_limit = Double.NegativeInfinity;
            }

            InternalResetScrolling();// obligatory

            // validate ids
            int strum_id = this.strum_id;
            if (strum_id < 0 || strum_id >= strumsdefs_size) return -1;
            if (player_id < 0 || player_id >= chart.entries_size) return 0;

            int[] notes_ids = strumsdefs[strum_id].notes_ids;
            int note_ids_size = strumsdefs[strum_id].notes_ids_size;

            ChartNote[] player_notes = chart.entries[player_id].notes;
            int player_notes_size = chart.entries[player_id].notes_size;

            // step 1: count the notes required for this strum
            int count = 0;
            for (int i = 0 ; i < player_notes_size ; i++) {
                int note_direction = (int)player_notes[i].direction;
                for (int j = 0 ; j < note_ids_size ; j++) {
                    if (note_direction == notes_ids[j]) {
                        count++;
                        break;
                    }
                }
            }

            // step 2: map all note IDs
            this.chart_notes_id_map = new int[note_ids_size];
            this.chart_notes_id_map_size = note_ids_size;

            for (int i = 0 ; i < note_ids_size ; i++)
                this.chart_notes_id_map[i] = notes_ids[i];

            // step 3: grab notes from the chart
            this.chart_notes = new StrumNote[count];
            this.chart_notes_size = count;

            int k = 0;
            for (int i = 0 ; i < player_notes_size ; i++) {
                bool ignore = true;
                for (int j = 0 ; j < note_ids_size ; j++) {
                    if (player_notes[i].direction == notes_ids[j]) {
                        ignore = false;
                        break;
                    }
                }
                if (ignore) continue;

                this.chart_notes[k++] = new StrumNote(player_notes[i], notes_ids, note_ids_size);
            }

            if (count > 0) {
                // Important: sort the notes by timestamp
                Array.Sort(this.chart_notes, 0, this.chart_notes_size, StrumNote.SortCallback);

                // calculate the key test time limit
                this.key_test_limit = Math.Max(this.chart_notes[0].timestamp - this.marker_duration, 0.0);

                // remove duplicated notes (filtered by timestamp and id)
                int j = 0;
                double last_timestamp = Double.NaN;
                int last_id = -1;
                for (int i = 0 ; i < this.chart_notes_size ; i++) {
                    int id = this.chart_notes[i].id;
                    double timestamp = this.chart_notes[i].timestamp;
                    if (timestamp == last_timestamp && id == last_id) {
                        Console.Error.WriteLine($"duplicated note found: ts={timestamp} id={id}");
                    } else {
                        last_timestamp = timestamp;
                        last_id = id;
                        this.chart_notes[j++] = this.chart_notes[i];
                    }
                }
                if (j != this.chart_notes_size) {
                    // trim array
                    this.chart_notes_size = j;
                    Array.Resize(ref this.chart_notes, this.chart_notes_size);
                }

            } else {
                this.key_test_limit = -Double.PositiveInfinity;
            }


            if (this.strum_name != null)
                InternalSetNoteDrawables(notepool);


            return count;
        }

        public void ForceRebuild(float marker_dimmen, float invdimmen, float length_dimmen, bool keep_markers_scale) {
            //
            // Note: the caller MUST change later the scroll direction in order to take effect
            //
            Console.Error.WriteLine("[WARN] strum_force_rebuild() was called, this never should happen");

            this.dimmen_length = length_dimmen;
            this.dimmen_opposite = invdimmen;
            this.dimmen_marker = marker_dimmen;
            this.markers_scale_keep = keep_markers_scale;

            // set sprites size
            this.sprite_marker_nothing.SetDrawSize(marker_dimmen, marker_dimmen);
            this.sprite_marker_confirm.SetDrawSize(marker_dimmen, marker_dimmen);
            this.sprite_marker_press.SetDrawSize(marker_dimmen, marker_dimmen);
            this.sprite_sick_effect.SetDrawSize(marker_dimmen, marker_dimmen);
            this.sprite_background.SetDrawSize(marker_dimmen, length_dimmen);
        }

        public void UpdateDrawLocation(float x, float y) {
            this.sprite_marker_nothing.SetDrawLocation(x, y);
            this.sprite_marker_confirm.SetDrawLocation(x, y);
            this.sprite_marker_press.SetDrawLocation(x, y);
            this.sprite_sick_effect.SetDrawLocation(x, y);
            this.sprite_background.SetDrawLocation(x, y);

            this.modifier.x = x;
            this.modifier.y = y;
        }

        public void SetScrollSpeed(double speed) {
            this.scroll_velocity = this.scroll_velocity_base * speed;

            // Use half of the scolling speed if the screen aspect ratio is 4:3 (dreamcast)
            if (!PVRContext.global_context.IsWidescreen()) this.scroll_velocity *= 0.5;

            InternalCalcMarkerDuration(this.scroll_velocity);
            InternalCalcScrollWindow();
        }

        public void SetScrollDirection(ScrollDirection direction) {
            this.scroll_is_vertical = direction == ScrollDirection.UPSCROLL || direction == ScrollDirection.DOWNSCROLL;
            this.scroll_is_inverse = direction == ScrollDirection.DOWNSCROLL || direction == ScrollDirection.RIGHTSCROLL;

            if (this.scroll_is_vertical) {
                this.modifier.width = this.dimmen_marker;
                this.modifier.height = this.dimmen_length;
            } else {
                this.modifier.width = this.dimmen_length;
                this.modifier.height = this.dimmen_marker;
            }

            InternalCalcScrollWindow();
            InternalStatesRecalculate();
        }

        public void SetMarkerDurationMultiplier(float multipler) {
            this.marker_duration_multiplier = multipler > 0f ? multipler : 1.0f;

            double velocity = this.scroll_velocity;

            InternalCalcMarkerDuration(velocity);
        }

        public void Reset(double scroll_speed, string state_name) {
            InternalResetScrolling();

            this.drawable.SetAlpha(1.0f);
            this.drawable.SetOffsetColorToDefault();

            for (int i = 0 ; i < this.chart_notes_size ; i++) {
                this.chart_notes[i].state = NoteState.PENDING;
                this.chart_notes[i].hit_diff = Double.NaN;
                this.chart_notes[i].release_time = -1.0;
                this.chart_notes[i].release_button = 0x00;
                this.chart_notes[i].previous_quarter = 0;
                this.chart_notes[i].hit_on_penality = false;
            }

            SetScrollSpeed(scroll_speed);
            StateToggle(state_name);
        }

        public int Scroll(double song_timestamp, DDRKeysFIFO ddrkeys_fifo, PlayerStats playerstats, WeekScript weekscript) {

            StrumNote[] chart_notes = this.chart_notes;
            int chart_notes_size = this.chart_notes_size;
            float marker_duration = this.marker_duration;

            StrumMarkerState marker_state = StrumMarkerState.INVALID;
            int keys_processed = 0;
            int notes_peek_index = this.notes_peek_index;
            double song_offset_timestamp = song_timestamp + marker_duration;
            StrumPressState press_state = StrumPressState.INVALID;
            bool press_state_use_alt_anim = false;
            int notes_ahead = 0;
            int notes_cleared = 0;


            if (this.auto_scroll_elapsed > 0.0) {
                // cancel ScrollAuto() state keep
                this.auto_scroll_elapsed = -1.0;
                this.marker_state = StrumMarkerState.NOTHING;
                this.marker_state_changed = true;
            }

            // step 1: enter in "key testing" mode on the early seconds of the song or at the end
            if (song_timestamp < this.key_test_limit || notes_peek_index >= chart_notes_size) {
                for (int i = 0 ; i < ddrkeys_fifo.available ; i++) {
                    DDRKey ddr_key = ddrkeys_fifo.queue[i];
                    if (ddr_key.discard || ddr_key.strum_id != this.strum_id) continue;
                    if (ddr_key.in_song_timestamp > song_timestamp) continue;

                    keys_processed++;
                    ddr_key.discard = true;
                    marker_state = ddr_key.holding ? StrumMarkerState.CONFIRM : StrumMarkerState.NOTHING;
                }

                if (keys_processed > 0) InternalUpdatePressState(StrumPressState.NONE);

                InternalCheckSustainQueue(song_timestamp, playerstats);
                this.last_song_timestamp = song_timestamp;
                if (marker_state != StrumMarkerState.INVALID) {
                    this.marker_state = marker_state;
                    this.marker_state_changed = true;
                }
                return keys_processed;
            }

            // step 2: count all notes near the song position (under normal conditions is always 1)
            for (int i = notes_peek_index ; i < chart_notes_size ; i++) {
                if (chart_notes[i].timestamp > song_offset_timestamp) break;
                notes_ahead++;
            }

            // step 3: do key checking (very extensive)
            for (int i = 0 ; i < ddrkeys_fifo.available && notes_cleared < notes_ahead ; i++) {
                NoteAttribute note_attributes;
                DDRKey ddr_key = ddrkeys_fifo.queue[i];
                double key_timestamp = ddr_key.in_song_timestamp;
                if (ddr_key.discard || ddr_key.strum_id != this.strum_id) continue;

                // phase 1: check key event against every note ahead
                double lowest_diff = Double.PositiveInfinity;
                StrumNote note = null;
                int hit_index = -1;
                int index_ahead = 0;


                if (ddr_key.holding) {

                    // find a candidate for this key event
                    for (int j = notes_peek_index ; j < chart_notes_size ; j++) {
                        if (index_ahead++ > notes_ahead) break;

                        // check if is trying to recover a sustain note
                        if (chart_notes[j].state == NoteState.RELEASE) {
                            double start = chart_notes[j].timestamp;
                            double end = start + chart_notes[j].duration;
                            if (key_timestamp < start || key_timestamp > end) continue;

                            // the timestamp is within the bounds of the note
                            note = chart_notes[j];
                            hit_index = j;

                            // ignore recover if the next note is closer
                            j++;
                            if (j < chart_notes_size && chart_notes[j].state == NoteState.PENDING) {
                                start = chart_notes[j].timestamp - (marker_duration * 0.75);
                                if (start <= key_timestamp) {
                                    note = chart_notes[j];
                                    hit_index = j;
                                }
                            }

                            break;
                        }

                        // check if the note is on pending state and near the key timestamp
                        if (chart_notes[j].state != NoteState.PENDING) continue;
                        if (!InternalOnNoteBounds(j, key_timestamp)) continue;

                        // check how far are from the key, if near, remember the note
                        double diff = Math.Abs(chart_notes[j].timestamp - key_timestamp);
                        if (diff < lowest_diff) {
                            lowest_diff = diff;
                            note = chart_notes[j];
                            hit_index = j;
                        }
                    }

                    if (note == null) {
                        if (ddr_key.in_song_timestamp > song_timestamp) {
                            //Console.Error.WriteLine("[WARN] [hold] in future ts_k=" + key_timestamp+" ts_s=" + song_timestamp);
                        }

                        // maybe the key event is a penalty hit/press or future event (do not discard)
                        continue;
                    }

                    if (note.state == NoteState.RELEASE) {
                        //Console.Error.WriteLine("[INFO] "[hold] sustain recover!  ts_k=" + key_timestamp + " ts_n=" + note.timestamp);

                        if (Double.IsNaN(note.hit_diff)) {
                            // never pressed compute hit
                            note_attributes = this.attribute_notes[note.id];
                            note.hit_diff = note.timestamp - key_timestamp;
                            if (!note_attributes.ignore_hit) {
                                playerstats.AddSustainDelayedHit(note_attributes.heal_ratio, note.hit_diff);
                            }
                            if (note_attributes.can_kill_on_hit) playerstats.KillIfNegativeHealth();
                        }

                        // recover the sustain note
                        note.release_time = -1;
                        note.state = NoteState.HOLD;
                        note.release_button = ddr_key.button;

                        press_state = StrumPressState.HIT;
                        press_state_use_alt_anim = note.alt_anim;
                        goto L_discard_key_event;
                    }

                    // process pending note
                    note_attributes = this.attribute_notes[note.id];
                    bool sick_effect_ready = this.marker_sick_state == StrumMarkerState.NOTHING;
                    lowest_diff = note.timestamp - key_timestamp;// hit difference (for stats)

                    // calculate rank
                    Ranking rank = Ranking.NONE;
                    if (!note_attributes.ignore_hit) {
                        rank = playerstats.AddHit(
                             note_attributes.heal_ratio, marker_duration, lowest_diff
                        );
                    }

                    if (note_attributes.can_kill_on_hit) playerstats.KillIfNegativeHealth();
                    if (weekscript != null) weekscript.NotifyNoteHit(this, hit_index, playerstats);

                    //Console.Error.WriteLine($"[INFO] [hold] note hit!  ts={key_timestamp} diff={lowest_diff} rank={rank}");

                    // check if necessary display the sick effect
                    if (rank == Ranking.SICK && sick_effect_ready) {
                        this.marker_sick_state = StrumMarkerState.PRESS;

                        // if the note has a custom sick effect choose from the state list
                        if (!String.IsNullOrEmpty(note_attributes.custom_sick_effect_name))
                            this.marker_sick_state_name = note_attributes.custom_sick_effect_name;
                        else
                            this.marker_sick_state_name = this.selected_sick_effect_state;
                    }

                    if (note.duration >= this.minimum_sustain_duration) {
                        // add note to the sustain queue to check the health loss/gain later
                        this.sustain_queue.Add(note);
                        press_state = StrumPressState.HIT_SUSTAIN;
                    } else {
                        press_state = StrumPressState.HIT;
                    }

                    press_state_use_alt_anim = note.alt_anim;
                    note.state = NoteState.HOLD;
                    note.release_button = ddr_key.button;
                    note.release_time = note.EndTimestamp;// use end timestamp to avoid compute as miss
                    note.hit_diff = lowest_diff;
                } else {
                    //
                    // find what note is being released. Note: this method will fail
                    // if there multiple key inputs which under normal circumstances
                    // never should happen
                    //
                    for (int j = notes_peek_index ; j < chart_notes_size ; j++) {
                        if (chart_notes[j].state != NoteState.HOLD) continue;// ignore non-hold notes
                        if (chart_notes[j].release_button != ddr_key.button) continue;
                        if (index_ahead++ > notes_ahead) break;

                        note = chart_notes[j];
                        //Console.Error.WriteLine($"[LOG] [release] note found!  ts_k={key_timestamp} ts_n={note.timestamp}");
                        break;
                    }

                    press_state = StrumPressState.NONE;

                    if (note == null) {
                        if (key_timestamp <= song_timestamp) {
                            //Console.Error.WriteLine($"[LOG] (`[release] empty strum at {key_timestamp}");
                        }

                        //
                        // The key was pressed for too long or the penality hit/press was released.
                        // Note: InternalCheckSustainQueue() function handles sustain notes
                        // pressed longer than required.
                        //
                        goto L_discard_key_event;
                    }

                    note.release_button = GamepadButtons.NOTHING;

                    // clear if the note is not sustain
                    if (note.duration < this.minimum_sustain_duration) {
                        //Console.Error.WriteLine($"[INFO] [release] clear non-sustain ts={note.timestamp}");
                        note.state = NoteState.CLEAR;
                        note.release_time = Double.PositiveInfinity;
                        notes_cleared++;

                        goto L_discard_key_event;
                    }

                    // check if the sustain note was released early
                    double end_timestamp = note.EndTimestamp;

                    if (key_timestamp < end_timestamp) {
                        //Console.Error.WriteLine($"[LOG] [release] early! left duration {end_timestamp - key_timestamp}");

                        // early release
                        note.state = NoteState.RELEASE;
                        note.release_time = key_timestamp;
                    } else {
                        //Console.Error.WriteLine($"[LOG] [release] sustain clear! remain was {end_timestamp - note.release_time}");
                        this.sustain_queue.Remove(note);
                        note.state = NoteState.CLEAR;
                        note.release_time = Double.PositiveInfinity;
                        notes_cleared++;
                    }
                }

// phase 2: discard the key event
L_discard_key_event:
                ddr_key.discard = true;
                keys_processed++;

                // phase 3: update the marker state (press/confirm/nothing)
                if (ddr_key.holding)
                    marker_state = note != null ? StrumMarkerState.PRESS : StrumMarkerState.CONFIRM;
                else
                    marker_state = StrumMarkerState.NOTHING;
            }

            // step 4: check for missed notes or early released sustains
            song_offset_timestamp = song_timestamp - marker_duration;
            for (int i = notes_peek_index, j = 0 ; i < chart_notes_size && j < notes_ahead ; i++, j++) {
                StrumNote note = chart_notes[i];
                bool compute_miss = !this.attribute_notes[note.id].ignore_miss;
                double end_timestamp = note.timestamp + Math.Max(note.duration, marker_duration);

                switch (note.state) {
                    case NoteState.CLEAR:
                    case NoteState.HOLD:
                    case NoteState.MISS:
                        continue;
                }

                // check if the note is non-sustain
                if (note.duration < this.minimum_sustain_duration) {
                    if (song_offset_timestamp >= end_timestamp) {
                        //Console.Error.WriteLine($"[INFO] [missed] non-sustain note ts={note.timestamp}");
                        note.state = NoteState.MISS;
                        note.release_time = Double.NegativeInfinity;

                        if (compute_miss) {
                            playerstats.AddMiss(this.attribute_notes[note.id].hurt_ratio);
                            if (weekscript != null) weekscript.NotifyNoteLoss(this, i, playerstats, false);
                            this.extra_animations_have_misses = true;
                            press_state = StrumPressState.MISS;
                        }
                    }
                    continue;
                }


                //
                // On sustain notes check how many note-quarters are lost.
                // The InternalCheckSustainQueue() function calculates it is loosing.
                //

                if (compute_miss) {
                    if (note.state == NoteState.PENDING && weekscript != null)
                        weekscript.NotifyNoteLoss(this, i, playerstats, false);
                    if (note.state != NoteState.RELEASE)
                        this.extra_animations_have_misses = true;
                }

                if (song_timestamp > end_timestamp) {
                    //Console.Error.WriteLine($"[INFO] [missed] sustain note ts={note.timestamp} ts_end={end_timestamp}");
                    this.sustain_queue.Remove(note);

                    note.state = NoteState.MISS;
                    note.release_time = Double.NegativeInfinity;
                    note.release_button = GamepadButtons.NOTHING;
                    continue;
                }

                // consider as missed if the worst possible ranking can not be assigned
                double miss_timestamp = note.timestamp + marker_duration;
                if (song_timestamp >= miss_timestamp) {
                    //Console.Error.WriteLine($"[INFO] [miss] sustain loosing ts_n={note.timestamp} ts_s={song_timestamp}");
                    if (note.state == NoteState.PENDING) {
                        this.sustain_queue.Add(note);
                        note.state = NoteState.RELEASE;
                    }
                }
            }

            // step 5: move notes_peek_index
            for (int i = 0 ; i < notes_ahead && notes_peek_index < chart_notes_size ; i++) {
                NoteState note_state = chart_notes[notes_peek_index].state;
                if (note_state != NoteState.CLEAR && note_state != NoteState.MISS) break;
                notes_peek_index++;
            }

            // step 6: check for released keys of pentalties presses
            for (int i = 0 ; i < ddrkeys_fifo.available ; i++) {
                DDRKey ddr_key = ddrkeys_fifo.queue[i];
                double key_timestamp = ddr_key.in_song_timestamp;

                if (ddr_key.discard || ddr_key.holding || ddr_key.strum_id != this.strum_id) continue;
                if (key_timestamp > song_timestamp) continue;

                if (marker_state == StrumMarkerState.INVALID) marker_state = StrumMarkerState.NOTHING;
                if (press_state == StrumPressState.INVALID) press_state = StrumPressState.NONE;

                ddr_key.discard = true;
                keys_processed++;
            }

            // before leave, check the sustain queue
            InternalCheckSustainQueue(song_timestamp, playerstats);

            this.last_song_timestamp = song_timestamp;
            this.notes_peek_index = notes_peek_index;
            if (marker_state != StrumMarkerState.INVALID) {
                this.marker_state = marker_state;
                this.marker_state_changed = true;
            }
            InternalUpdatePressState(press_state);

            if (press_state == StrumPressState.HIT || press_state == StrumPressState.HIT_SUSTAIN) {
                this.press_state_use_alt_anim = press_state_use_alt_anim;
            }

            return keys_processed;
        }

        public void ScrollAuto(double song_timestamp, PlayerStats playerstats, WeekScript weekscript) {
            StrumNote[] chart_notes = this.chart_notes;
            NoteAttribute[] attribute_notes = this.attribute_notes;
            int chart_notes_size = this.chart_notes_size;
            float marker_duration = this.marker_duration;
            float press_time = this.marker_duration * 2f;

            int notes_peek_index = this.notes_peek_index;
            int notes_cleared = 0;

            StrumPressState press_state = StrumPressState.INVALID;
            bool press_state_use_alt_anim = false;

            // clear all notes ahead, but keep in hold the sustain ones
            for (; notes_peek_index < chart_notes_size ; notes_peek_index++) {
                StrumNote note = chart_notes[notes_peek_index];
                bool is_sustain = note.duration >= this.minimum_sustain_duration;
                double end_timestamp = note.EndTimestamp;

                if (note.timestamp > song_timestamp) break;

                if (note.state == NoteState.PENDING || note.state == NoteState.RELEASE) {

                    if (note.state == NoteState.PENDING) {
                        if (attribute_notes[note.id].can_kill_on_hit) {
                            if (!attribute_notes[note.id].ignore_miss)
                                playerstats.AddMiss(attribute_notes[note.id].hurt_ratio);
                        } else if (!attribute_notes[note.id].ignore_hit) {
                            float heal_ratio = attribute_notes[note.id].heal_ratio;
                            note.hit_diff = 0.0;
                            playerstats.AddHit(heal_ratio, marker_duration, note.hit_diff);

                            // keep the marker in press state for a while
                            this.auto_scroll_elapsed = note.duration < 1 ? press_time : note.duration;
                            this.marker_state = Strum.AUTO_SCROLL_MARKER_STATE;
                            this.marker_state_changed = true;
                            press_state = is_sustain ? StrumPressState.HIT_SUSTAIN : StrumPressState.HIT;
                            press_state_use_alt_anim = note.alt_anim;
                        }

                        if (weekscript != null) {
                            if (attribute_notes[note.id].can_kill_on_hit)
                                weekscript.NotifyNoteLoss(this, notes_peek_index, playerstats, false);
                            else
                                weekscript.NotifyNoteHit(this, notes_peek_index, playerstats);
                        }

                        if (is_sustain) this.sustain_queue.Add(note);
                        else notes_cleared++;
                    }

                    note.release_button = GamepadButtons.NOTHING;

                    if (attribute_notes[note.id].can_kill_on_hit) {
                        note.state = is_sustain ? NoteState.RELEASE : NoteState.MISS;
                    } else {
                        note.state = is_sustain ? NoteState.HOLD : NoteState.CLEAR;
                        note.release_time = is_sustain ? end_timestamp : Double.PositiveInfinity;
                    }
                } else if (note.state == NoteState.HOLD) {
                    if (song_timestamp >= end_timestamp) {
                        note.state = NoteState.CLEAR;
                        note.release_time = Double.PositiveInfinity;
                        note.release_button = GamepadButtons.NOTHING;
                        notes_cleared++;

                        press_state = StrumPressState.NONE;
                        if (is_sustain) this.sustain_queue.Remove(note);
                    }
                } else if (note.state == NoteState.RELEASE && song_timestamp >= end_timestamp) {
                    note.state = NoteState.MISS;
                    note.release_time = Double.NegativeInfinity;
                    note.release_button = GamepadButtons.NOTHING;
                    notes_cleared++;

                    press_state = StrumPressState.NONE;
                    if (is_sustain) this.sustain_queue.Remove(note);
                }
            }

            InternalUpdatePressState(press_state);
            InternalCheckSustainQueue(song_timestamp, playerstats);
            this.last_song_timestamp = song_timestamp;

            if (press_state == StrumPressState.HIT || press_state == StrumPressState.HIT_SUSTAIN) {
                this.press_state_use_alt_anim = press_state_use_alt_anim;
            }

            int count = notes_peek_index - this.notes_peek_index;
            if (notes_cleared < count) return;

            this.notes_peek_index = notes_peek_index;

        }

        public void ForceKeyRelease() {
            this.marker_state = StrumMarkerState.NOTHING;
            this.marker_state_changed = true;
            InternalUpdatePressState(StrumPressState.NONE);

            for (int i = this.notes_peek_index ; i < this.chart_notes_size ; i++) {
                StrumNote note = this.chart_notes[i];
                if (note.state == NoteState.HOLD) {
                    note.state = NoteState.RELEASE;
                    note.release_time = Math.Max(this.last_song_timestamp, note.timestamp);
                }
            }

            Animate(0f);
        }

        public int FindPenaltiesNoteHit(double song_timestamp, DDRKeysFIFO ddrkeys_fifo, PlayerStats playerstats, WeekScript weekscript) {
            if (ddrkeys_fifo.available < 0) return 0;

            StrumNote[] chart_notes = this.chart_notes;
            int chart_notes_size = this.chart_notes_size;
            NoteAttribute[] attribute_notes = this.attribute_notes;

            int keys_processed = 0;

            //
            // How this function works:
            //
            //  * Pick key events from neighbor strums and check if hits notes on this this. To accomplish
            //    this Scroll() function must be called before doing this.
            //
            //  * Ignore key events of "invisible" strums which are used in mechanics like "press space"
            //
            //  * Do not highlight the strum (STRUM_MARKER_*) for foreign key events
            //
            //  * Only pending notes are checked, sustain notes and early released sustains are ignored.
            //
            //  * ignore notes with less of 50% of accuracy
            //

            for (int i = Math.Min(this.notes_peek_index, chart_notes_size - 1) ; i < chart_notes_size ; i++) {
                StrumNote note = chart_notes[i];
                if (note.state != NoteState.PENDING) continue;
                if (note.duration >= this.minimum_sustain_duration) continue;

                for (int j = 0 ; j < ddrkeys_fifo.available ; j++) {
                    DDRKey ddr_key = ddrkeys_fifo.queue[j];
                    double key_timestamp = ddr_key.in_song_timestamp;

                    if (ddr_key.discard || ddr_key.strum_invisible || !ddr_key.holding) continue;
                    if (ddr_key.strum_id == this.strum_id) continue;
                    if (ddr_key.in_song_timestamp > song_timestamp) continue;

                    if (InternalOnNoteBounds(i, key_timestamp)) {
                        double diff = key_timestamp / this.chart_notes[i].timestamp;
                        if (diff <= 0.50 || diff >= 1.50) continue;

                        //Console.Error.WriteLine("[LOG] [penality] hit on pending note ts=" + note.timestamp);
                        this.extra_animations_have_penalties = true;
                        InternalUpdatePressState(StrumPressState.PENALTY_HIT);

                        if (!attribute_notes[note.id].ignore_miss) {
                            note.state = NoteState.MISS;
                            note.hit_on_penality = true;
                            note.release_button = GamepadButtons.NOTHING;
                            note.release_time = Double.NegativeInfinity;
                            playerstats.AddPenality(false);
                        }
                        if (weekscript != null) weekscript.NotifyNoteLoss(this, i, playerstats, true);
                    } else {
                        // ignore key event
                        continue;
                    }

                    ddr_key.discard = true;
                    keys_processed++;
                }
            }

            return keys_processed;
        }

        public int FindPenaltiesEmptyHit(double song_timestamp, DDRKeysFIFO ddrkeys_fifo, PlayerStats playerstats) {
            if (ddrkeys_fifo.available < 1) return 0;

            int keys_processed = 0;

            //
            // How this function works:
            //
            //  * Check if the player is pressing a button on a strum empty.
            //
            //  * FindPenaltiesNoteHit() function must be called before doing this.
            //

            for (int i = 0 ; i < ddrkeys_fifo.available ; i++) {
                DDRKey ddr_key = ddrkeys_fifo.queue[i];
                double key_timestamp = ddr_key.in_song_timestamp;

                if (ddr_key.discard || ddr_key.strum_id != this.strum_id) continue;
                if (key_timestamp > song_timestamp) continue;// maybe is a future penality

                if (ddr_key.holding) {
                    //Console.Error.WriteLine("[LOG] [penality] key hold on empty strum ts=" + key_timestamp);
                    playerstats.AddPenality(true);
                    InternalUpdatePressState(StrumPressState.PENALTY_NOTE);
                    this.marker_state = StrumMarkerState.CONFIRM;
                    this.extra_animations_have_penalties = true;
                } else {
                    //Console.Error.WriteLine("[LOG] [penality] key release on empty strum ts=" + key_timestamp);
                    this.marker_state = StrumMarkerState.NOTHING;
                    InternalUpdatePressState(StrumPressState.NONE);
                }

                this.marker_state_changed = true;
                ddr_key.discard = true;
                keys_processed++;
            }

            return keys_processed;
        }


        public int GetPressStateChanges() {
            return this.press_state_changes;
        }

        public StrumPressState GetPressState() {
            return this.press_state;
        }

        public bool GetPressStateUseAltAnim() {
            return this.press_state_use_alt_anim;
        }

        public string GetName() {
            return this.strum_name;
        }

        public float GetMarkerDuration() {
            return this.marker_duration;
        }

        public void UseFunkinMakerDuration(bool enable) {
            this.use_fukin_marker_duration = !!enable;
            InternalCalcMarkerDuration(this.scroll_velocity);
        }

        public void SetPlayerID(int id) {
            this.player_id = id;
        }


        public int Animate(float elapsed) {
            if (String.IsNullOrEmpty(this.strum_name)) return 1;

            if (this.beatwatcher.Poll() && this.use_beat_synced_idle_and_continous) {
                this.sprite_marker_nothing.AnimationRestart();
                InternalRestartExtraContinous();
            }

            int res = 0;

            #region marker animation

            // if ScrollAuto() function was used, keep the marker in press state for a while
            if (this.auto_scroll_elapsed > 0) {
                if (this.marker_state != Strum.AUTO_SCROLL_MARKER_STATE) {
                    this.auto_scroll_elapsed = -1;
                } else {
                    this.auto_scroll_elapsed -= elapsed;
                    if (this.auto_scroll_elapsed <= 0) {
                        this.auto_scroll_elapsed = -1;
                        this.marker_state = StrumMarkerState.NOTHING;
                        this.marker_state_changed = true;
                        InternalUpdatePressState(StrumPressState.NONE);
                    }
                }
            }

            StateSprite current_marker;
            switch (this.marker_state) {
                case StrumMarkerState.CONFIRM:
                    current_marker = this.sprite_marker_confirm;
                    break;
                case StrumMarkerState.PRESS:
                    current_marker = this.sprite_marker_press;
                    break;
                case StrumMarkerState.NOTHING:
                    current_marker = this.sprite_marker_nothing;
                    break;
                default:
                    return 1;
            }

            // restart the marker animation (if was changed)
            if (this.marker_state_changed) {
                this.marker_state_changed = false;
                current_marker.AnimationRestart();
            }

            res += current_marker.Animate(elapsed);
            #endregion

            // background animation
            if (this.enable_background)
                res += this.sprite_background.Animate(elapsed);

            #region splash effect animation
            if (this.enable_sick_effect) {
                // check if is necessary restart the sick effect animation
                bool visible = true;
                switch (this.marker_sick_state) {
                    case StrumMarkerState.NOTHING:
                        visible = false;
                        break;
                    case StrumMarkerState.PRESS:
                        // change the state if the note has a custom sick effect
                        this.sprite_sick_effect.StateToggle(this.marker_sick_state_name);

                        // play the splash effect animation again
                        this.sprite_sick_effect.AnimationRestart();
                        this.marker_sick_state = StrumMarkerState.CONFIRM;
                        break;
                }

                // hide if ended
                if (visible && (res += this.sprite_sick_effect.Animate(elapsed)) > 0)
                    this.marker_sick_state = StrumMarkerState.NOTHING;
            }
            #endregion

            #region compute & animate extra animations
            StrumScriptOn current_event;
            if (this.extra_animations_have_penalties) {
                current_event = StrumScriptOn.PENALITY;
                this.extra_animations_have_misses = this.extra_animations_have_penalties = false;
            } else if (this.extra_animations_have_misses) {
                current_event = StrumScriptOn.MISS;
                this.extra_animations_have_penalties = this.extra_animations_have_misses = false;
            } else if (this.marker_state == StrumMarkerState.PRESS) {
                current_event = StrumScriptOn.HIT_DOWN;
            } else {
                current_event = StrumScriptOn.IDLE;
            }

            res += InternalExtraAnimate(StrumScriptTarget.STRUM_LINE, current_event, false, elapsed);
            res += InternalExtraAnimate(StrumScriptTarget.NOTE, current_event, false, elapsed);
            res += InternalExtraAnimate(StrumScriptTarget.MARKER, current_event, false, elapsed);
            res += InternalExtraAnimate(StrumScriptTarget.SICK_EFFECT, current_event, false, elapsed);
            res += InternalExtraAnimate(StrumScriptTarget.BACKGROUND, current_event, false, elapsed);
            #endregion

            #region execute continuous extra animations
            InternalExtraAnimateSprite(
                StrumScriptTarget.STRUM_LINE, this.animation_strum_line.continuous.action
            );
            InternalExtraAnimateSprite(
                StrumScriptTarget.NOTE, this.animation_note.continuous.action
            );
            InternalExtraAnimateSprite(
                StrumScriptTarget.MARKER, this.animation_marker.continuous.action
            );
            InternalExtraAnimateSprite(
                StrumScriptTarget.SICK_EFFECT, this.animation_sick_effect.continuous.action
            );
            InternalExtraAnimateSprite(
                StrumScriptTarget.BACKGROUND, this.animation_background.continuous.action
            );
            #endregion

            return res;
        }

        public void Draw(PVRContext pvrctx) {
            if (String.IsNullOrEmpty(this.strum_name)) return;

            pvrctx.Save();
            this.drawable.HelperApplyInContext(pvrctx);

            // draw the background
            if (this.enable_background) {
                this.sprite_background.Draw(pvrctx);
            }

            // draw the strum marker
            if (this.marker_state == StrumMarkerState.NOTHING) {
                this.sprite_marker_nothing.Draw(pvrctx);
            }

            // draw the note sick hit effect
            if (this.enable_sick_effect && this.marker_sick_state == StrumMarkerState.CONFIRM) {
                this.sprite_sick_effect.Draw(pvrctx);
            }

            // check if there notes to draw
            if (this.notes_render_index >= this.chart_notes_size) {
                // nothing to draw, update the marker (if required) and return
                switch (this.marker_state) {
                    case StrumMarkerState.CONFIRM:
                        this.sprite_marker_confirm.Draw(pvrctx);
                        break;
                    case StrumMarkerState.PRESS:
                        this.sprite_marker_press.Draw(pvrctx);
                        break;
                }

                pvrctx.Restore();
                return;
            }

            // toggle state on the notes (because note objects can be shared with other strum lines)
            for (int i = 0 ; i < this.chart_notes_id_map_size ; i++) {
                if (this.drawable_notes[i] != null)
                    this.drawable_notes[i].StateToggle(this.selected_notes_state);
            }


            //
            // setup everything to start drawing notes
            //

            double song_timestamp = this.last_song_timestamp + this.draw_offset_milliseconds;
            int notes_render_index = this.notes_render_index;
            double draw_x = this.modifier.x;
            double draw_y = this.modifier.y;
            double song_window_past = song_timestamp - (this.marker_duration * Strum.DRAW_PAST_NOTES);
            double song_window_future = song_timestamp + this.scroll_window;

            if (this.scroll_is_inverse) {
                if (this.scroll_is_vertical)
                    draw_y += this.inverse_offset;
                else
                    draw_x += this.inverse_offset;
            }

            for (int i = notes_render_index ; i < this.chart_notes_size ; i++) {
                StrumNote note = this.chart_notes[i];

                if (note.EndTimestamp <= song_window_past) {
                    notes_render_index++;
                    continue;
                } else if (note.timestamp >= song_window_future) {
                    // future note, stop here
                    break;
                }

                // do not draw cleared notes or invisible ones
                if (note.state == NoteState.CLEAR) continue;
                if (note.state == NoteState.HOLD && note.duration < 1) continue;
                if (note.state == NoteState.MISS && note.hit_on_penality) continue;
                if (this.drawable_notes[note.id] == null) continue;

                double note_duration = note.duration;
                double note_timestamp = note.timestamp;
                bool body_only = false;

                // on sustain notes only draw the remaining duration
                if (note.duration > 0.0) {
                    if (note.state == NoteState.HOLD) {
                        body_only = true;
                        note_duration -= song_timestamp - note_timestamp;
                        note_timestamp = song_timestamp;
                    } else if (note.release_time > 0.0) {
                        // early release
                        body_only = true;
                        note_duration -= note.release_time - note_timestamp;
                        note_timestamp = note.release_time;
                    }

                    if (note.state != NoteState.HOLD && note_timestamp <= song_window_past) {
                        // past note, show part of the sustain body 
                        body_only = true;
                        double past_note_timestamp = song_timestamp - this.marker_duration;
                        note_duration -= past_note_timestamp - note_timestamp;
                        note_timestamp = past_note_timestamp;

                        // ignore short sustain duration
                        if (body_only && note_duration < this.marker_duration) continue;
                    }

                    if (note_duration <= 0.0) continue;
                }

                double scroll_offset;
                double x = draw_x;
                double y = draw_y;
                Note drawable_note = this.drawable_notes[note.id];

                if (this.scroll_is_inverse) {
                    scroll_offset = song_timestamp - note_timestamp;
                } else {
                    scroll_offset = note_timestamp - song_timestamp;
                }

                scroll_offset *= this.scroll_velocity;

                if (this.tweenkeyframe_note != null) {
                    double percent = Math.Abs(scroll_offset / this.scroll_window);
                    this.tweenkeyframe_note.AnimatePercent(percent);
                    this.tweenkeyframe_note.VertexSetProperties(
                        drawable_note.PeekAloneStateSprite()
                    );
                }

                if (this.scroll_is_vertical)
                    y += scroll_offset;
                else
                    x += scroll_offset;


                drawable_note.Draw(pvrctx, this.scroll_velocity, x, y, note_duration, body_only);

                if (this.tweenkeyframe_note != null) {
                    //
                    // after the note is drawn, "attempt" to restore the original values
                    // by running again the TweenKeyframe at 0%.
                    //
                    this.tweenkeyframe_note.AnimatePercent(0.0);
                    this.tweenkeyframe_note.VertexSetProperties(
                        drawable_note.PeekAloneStateSprite()
                    );
                }
            }

            switch (this.marker_state) {
                case StrumMarkerState.CONFIRM:
                    this.sprite_marker_confirm.Draw(pvrctx);
                    break;
                case StrumMarkerState.PRESS:
                    this.sprite_marker_press.Draw(pvrctx);
                    break;
            }

            this.notes_render_index = notes_render_index;
            pvrctx.Restore();
        }


        public void EnableBackground(bool enable) {
            this.enable_background = enable;
        }

        public void EnableSickEffect(bool enable) {
            this.enable_sick_effect = enable;
            this.sprite_sick_effect.SetVisible(
                 this.enable_sick_effect_draw && this.enable_sick_effect
            );
        }

        public void StateAdd(ModelHolder mdlhldr_mrkr, ModelHolder mdlhldr_sck_ffct, ModelHolder mdlhldr_bckgrnd, string state_name) {
            StateSpriteState state;

            // marker nothing
            this.markers_scale = -1.0f;
            state = InternalStateAdd(
                this.sprite_marker_nothing, mdlhldr_mrkr, this.strum_name,
                Strum.MARKER_SUFFIX_MARKER, state_name
            );
            if (state != null) InternalCalcStateMarker(state);

            // marker confirm
            state = InternalStateAdd(
                this.sprite_marker_confirm, mdlhldr_mrkr, this.strum_name,
                Strum.MARKER_SUFFIX_CONFIRM, state_name
            );
            if (state != null) InternalCalcStateMarker(state);

            // marker press
            state = InternalStateAdd(
                this.sprite_marker_press, mdlhldr_mrkr, this.strum_name,
                Strum.MARKER_SUFFIX_HIT, state_name
            );
            if (state != null) InternalCalcStateMarker(state);

            // sick effect state
            InternalLoadSickEffectState(
                 mdlhldr_sck_ffct,
                 state_name
             );

            // background state
            state = InternalStateAdd(
                this.sprite_background, mdlhldr_bckgrnd, this.strum_name,
                Strum.BACKGROUND_SUFFIX, state_name
            );
            if (state != null) InternalCalcStateBackground(state);

        }

        public int StateToggle(string state_name) {
            int success = 0;
            success += StateToggleMarker(state_name);
            success += StateToggleSickEffect(state_name) ? 1 : 0;
            success += StateToggleBackground(state_name) ? 1 : 0;
            success += StateToggleNotes(state_name);
            return success;
        }

        public int StateToggleNotes(string state_name) {
            //if (this.selected_notes_state) free(this.selected_notes_state);
            this.selected_notes_state = state_name;

            return 1;
        }

        public bool StateToggleSickEffect(string state_name) {
            //if (this.selected_sick_effect_state) free(this.selected_sick_effect_state);
            this.selected_sick_effect_state = state_name;

            return this.sprite_sick_effect.StateToggle(state_name);
        }

        public int StateToggleMarker(string state_name) {
            int res = 0;

            // toggle marker sub-states (yeah state of states)
            res += this.sprite_marker_nothing.StateToggle(state_name) ? 1 : 0;
            res += this.sprite_marker_confirm.StateToggle(state_name) ? 1 : 0;
            res += this.sprite_marker_press.StateToggle(state_name) ? 1 : 0;

            return res;
        }

        public bool StateToggleBackground(string state_name) {
            return this.sprite_background.StateToggle(state_name);
        }


        public float SetAlphaBackground(float alpha) {
            float old_alpha = this.sprite_background.GetAlpha();
            this.sprite_background.SetAlpha(alpha);
            return old_alpha;
        }

        public float SetAlphaSickEffect(float alpha) {
            float old_alpha = this.sprite_sick_effect.GetAlpha();
            this.sprite_sick_effect.SetAlpha(alpha);
            return old_alpha;
        }

        public void SetKeepAspectRatioBackground(bool enable) {
            this.keep_aspect_ratio_background = enable;
            Strum.InternalCalcStates(this.sprite_background, InternalCalcStateBackground);
        }

        public StateSprite DrawSickEffectApart(bool enable) {
            this.enable_sick_effect_draw = enable;
            this.sprite_sick_effect.SetVisible(false);
            return this.sprite_sick_effect;
        }


        public void SetExtraAnimation(StrumScriptTarget strum_script_target, StrumScriptOn strum_script_on, bool undo, AnimSprite animsprite) {
            if (strum_script_target == StrumScriptTarget.ALL) {
                SetExtraAnimation(StrumScriptTarget.MARKER, strum_script_on, undo, animsprite);
                SetExtraAnimation(StrumScriptTarget.SICK_EFFECT, strum_script_on, undo, animsprite);
                SetExtraAnimation(StrumScriptTarget.BACKGROUND, strum_script_on, undo, animsprite);
                SetExtraAnimation(StrumScriptTarget.STRUM_LINE, strum_script_on, undo, animsprite);
                SetExtraAnimation(StrumScriptTarget.NOTE, strum_script_on, undo, animsprite);
                return;
            }

            AttachedAnimations extra_animations = InternalExtraGetHolder(strum_script_target);
            if (extra_animations == null) {
                Console.Error.WriteLine("[WARN] strum_set_extra_animation() unknown strum_script_target" + strum_script_target);
                return;
            }

            AttachedAnimations.Action action_undo_anims;

            switch (strum_script_on) {
                case StrumScriptOn.HIT_UP:
                    action_undo_anims = extra_animations.hit_up;
                    break;
                case StrumScriptOn.HIT_DOWN:
                    action_undo_anims = extra_animations.hit_down;
                    break;
                case StrumScriptOn.MISS:
                    action_undo_anims = extra_animations.miss;
                    break;
                case StrumScriptOn.PENALITY:
                    action_undo_anims = extra_animations.penality;
                    break;
                case StrumScriptOn.IDLE:
                    action_undo_anims = extra_animations.idle;
                    break;
                case StrumScriptOn.ALL:
                    SetExtraAnimation(strum_script_target, StrumScriptOn.HIT_UP, undo, animsprite);
                    SetExtraAnimation(strum_script_target, StrumScriptOn.HIT_DOWN, undo, animsprite);
                    SetExtraAnimation(strum_script_target, StrumScriptOn.MISS, undo, animsprite);
                    SetExtraAnimation(strum_script_target, StrumScriptOn.IDLE, undo, animsprite);
                    SetExtraAnimation(strum_script_target, StrumScriptOn.PENALITY, undo, animsprite);
                    return;
                default:
                    Console.Error.WriteLine("[WARN] strum_set_extra_animation() unknown strum_script_on = " + strum_script_on);
                    return;
            }

            AnimSprite anim = undo ? action_undo_anims.undo : action_undo_anims.action;

            if (anim != null)
                anim.Destroy();

            // obligatory, make a copy of animsprite
            anim = animsprite == null ? null : animsprite.Clone();
        }

        public void SetExtraAnimationContinuous(StrumScriptTarget strum_script_target, AnimSprite animsprite) {
            AttachedAnimations attached_animations;

            switch (strum_script_target) {
                case StrumScriptTarget.MARKER:
                    attached_animations = this.animation_marker;
                    break;
                case StrumScriptTarget.SICK_EFFECT:
                    attached_animations = this.animation_sick_effect;
                    break;
                case StrumScriptTarget.BACKGROUND:
                    attached_animations = this.animation_background;
                    break;
                case StrumScriptTarget.STRUM_LINE:
                    attached_animations = this.animation_strum_line;
                    break;
                case StrumScriptTarget.NOTE:
                    attached_animations = this.animation_note;
                    break;
                case StrumScriptTarget.ALL:
                    SetExtraAnimationContinuous(StrumScriptTarget.MARKER, animsprite);
                    SetExtraAnimationContinuous(StrumScriptTarget.SICK_EFFECT, animsprite);
                    SetExtraAnimationContinuous(StrumScriptTarget.BACKGROUND, animsprite);
                    SetExtraAnimationContinuous(StrumScriptTarget.STRUM_LINE, animsprite);
                    SetExtraAnimationContinuous(StrumScriptTarget.NOTE, animsprite);
                    return;
                default:
                    Console.Error.WriteLine("[WARN]" +
                        "strum_set_extra_animation_continuous() unknown strum_script_target=" +
                        strum_script_target
                    );
                    return;
            }

            // dispose previous animation
            if (attached_animations.continuous.action != null)
                attached_animations.continuous.action.Destroy();

            // obligatory, make a copy of animsprite
            attached_animations.continuous.action = animsprite == null ? null : animsprite.Clone();
        }


        public void SetNoteTweenkeyframe(TweenKeyframe tweenkeyframe) {
            if (this.tweenkeyframe_note != null) this.tweenkeyframe_note.Destroy();
            this.tweenkeyframe_note = tweenkeyframe != null ? tweenkeyframe.Clone() : null;
        }

        public void SetSickeffectSizeRatio(float size_ratio) {
            this.sick_effect_ratio = size_ratio;
            Strum.InternalCalcStates(this.sprite_sick_effect, InternalCalcStateSickEffect);
        }


        public void SetOffsetColor(float r, float g, float b, float a) {
            this.drawable.SetOffsetColor(r, g, b, a);
        }

        public void SetAlpha(float alpha) {
            this.drawable.SetAlpha(alpha);
        }

        public void SetVisible(bool visible) {
            this.drawable.SetVisible(visible);
        }

        public void SetDrawOffset(double offset_milliseconds) {
            this.draw_offset_milliseconds = offset_milliseconds;
        }

        public void SetBpm(float beats_per_minute) {
            this.beatwatcher.Reset(true, beats_per_minute);
        }

        public void DisableBeatSyncedIdleAndContinous(bool disabled) {
            this.use_beat_synced_idle_and_continous = !disabled;
        }


        public Modifier GetModifier() {
            return this.drawable.GetModifier();
        }

        public Drawable GetDrawable() {
            return this.drawable;
        }

        public double GetDuration() {
            double max_duration = 0;
            for (int i = 0 ; i < this.chart_notes_size ; i++) {
                double end_timestamp = this.chart_notes[i].EndTimestamp;
                if (end_timestamp > max_duration) max_duration = end_timestamp;
            }
            return max_duration;
        }

        public void AnimationRestart() {
            Strum.InternalExtraBatch(this.animation_strum_line, true);
            Strum.InternalExtraBatch(this.animation_note, true);
            Strum.InternalExtraBatch(this.animation_marker, true);
            Strum.InternalExtraBatch(this.animation_sick_effect, true);
            Strum.InternalExtraBatch(this.animation_background, true);

        }

        public void AnimationEnd() {
            Strum.InternalExtraBatch(this.animation_strum_line, false);
            Strum.InternalExtraBatch(this.animation_note, false);
            Strum.InternalExtraBatch(this.animation_marker, false);
            Strum.InternalExtraBatch(this.animation_sick_effect, false);
            Strum.InternalExtraBatch(this.animation_background, false);
        }



        private void InternalCalcScrollWindow() {
            this.scroll_window = Math.Abs(this.dimmen_length / this.scroll_velocity);
        }

        private bool InternalOnNoteBounds(int note_index, double test_timestamp) {
            StrumNote strum_note = this.chart_notes[note_index];

            double note_timestamp = strum_note.timestamp;
            double note_duration = InternalGetNoteDuration(note_index);

            double start = note_timestamp - this.marker_duration;
            double end = note_timestamp + note_duration;

            // increase end timestamp for non-sustain notes
            if (strum_note.duration < 1) end += this.marker_duration;

            return test_timestamp >= start && test_timestamp <= end;
        }

        private double InternalGetNoteDuration(int note_index) {
            double note_duration = this.chart_notes[note_index].duration;

            //
            // Note:
            //      ¿is the sustain duration visible at the current chart speed and scroll velocity?
            //      if not, the sustain length in pixels is negligible (duration too short).
            //
            //      Sustain notes should have a minimal duration according to the
            //      scroll speed, also the sustain note duration must reach the threshold.
            //

            return note_duration < this.minimum_sustain_duration ? this.marker_duration : note_duration;
        }


        private static void InternalCalcStateDimmen(StateSpriteState state, bool scroll_is_vertical, float dimmen, float invdimmen) {
            float draw_width, draw_height;
            float width, height;

            if (scroll_is_vertical) {
                width = invdimmen;
                height = dimmen;
            } else {
                width = dimmen;
                height = invdimmen;
            }

            ImgUtils.CalcSize2(state.texture, state.frame_info, width, height, out draw_width, out draw_height);

            state.draw_width = draw_width;
            state.draw_height = draw_height;
        }

        private static StateSpriteState InternalStateAdd(StateSprite statesprite, ModelHolder modelholder, string strum_name, string target, string state_name) {
            if (statesprite.StateHas(state_name)) return null;
            if (modelholder == null) return null;

            string animation_name = strum_name + " " + target;

            if (state_name != null) animation_name = animation_name + " " + state_name;

            StateSpriteState state = statesprite.StateAdd(modelholder, animation_name, state_name);
            //free(animation_name);

            // reset the draw location offsets beacuase is picked from last state applied
            if (state != null) {
                state.offset_x = 0;
                state.offset_y = 0;
            }

            return state;
        }

        private static void InternalCalcStates(StateSprite statesprite, CalcCallback calc_callback) {
            LinkedList<StateSpriteState> list = statesprite.StateList();
            foreach (StateSpriteState state in list) {
                // reset the state offsets before continue
                state.offset_x = state.offset_y = 0;
                calc_callback(state);
            }
            statesprite.StateApply(null);
        }

        private void InternalCalcStateMarker(StateSpriteState state) {
            float original_width = 0, original_height = 0;
            float width, height;

            if (this.scroll_is_vertical) {
                width = this.dimmen_opposite;
                height = this.dimmen_marker;
                if (this.scroll_is_inverse) state.offset_y += this.inverse_offset;
            } else {
                width = this.dimmen_marker;
                height = this.dimmen_opposite;
                if (this.scroll_is_inverse) state.offset_x += this.inverse_offset;
            }

            if (this.markers_scale_keep) {
                ImgUtils.GetStateSpriteOriginalSize(state, ref original_width, ref original_height);
            }

            if (this.markers_scale < 0.0 || !this.markers_scale_keep) {
                Strum.InternalCalcStateDimmen(
                     state, this.scroll_is_vertical, this.dimmen_marker, this.dimmen_opposite
                 );
                this.markers_scale = state.draw_width / original_width;
            } else {
                //
                // resize sprite_marker_confirm and sprite_marker_press with the same
                // scale as sprite_marker_nothing
                //
                state.draw_width = original_width * this.markers_scale;
                state.draw_height = original_height * this.markers_scale;
            }


            // center marker inside of the marker bounds
            state.offset_x += (width - state.draw_width) / 2f;
            state.offset_y += (height - state.draw_height) / 2f;
        }

        private void InternalCalcStateSickEffect(StateSpriteState state) {
            float dimmen_sick_effect = this.dimmen_marker * this.sick_effect_ratio;

            float width, height;
            float inverse = this.dimmen_length - dimmen_sick_effect;

            if (this.scroll_is_vertical) {
                width = this.dimmen_opposite;
                height = this.dimmen_marker;
                if (this.scroll_is_inverse) state.offset_y += inverse + height;
            } else {
                width = dimmen_sick_effect;
                height = this.dimmen_marker;
                if (this.scroll_is_inverse) state.offset_x += inverse + width;
            }

            Strum.InternalCalcStateDimmen(
                 state, this.scroll_is_vertical, dimmen_sick_effect, this.dimmen_opposite * this.sick_effect_ratio
             );

            // center the splash with the marker
            state.offset_x += (width - state.draw_width) / 2;
            state.offset_y += (height - state.draw_height) / 2;
        }

        private void InternalCalcStateBackground(StateSpriteState state) {
            float temp_width, temp_height;
            Corner corner;
            float width = this.dimmen_opposite;
            float height = this.dimmen_length;

            if (this.scroll_is_vertical) {
                corner = this.scroll_is_inverse ? Corner.BOTTOMRIGHT : Corner.TOPLEFT;
            } else {
                corner = this.scroll_is_inverse ? Corner.BOTTOMLEFT : Corner.TOPRIGHT;
            }

            state.corner_rotation.HelperCalculateCornerRotation(corner);

            if (!this.keep_aspect_ratio_background) {
                state.draw_width = width;
                state.draw_height = height;
                return;
            }

            ImgUtils.CalcSize2(state.texture, state.frame_info, width, height, out temp_width, out temp_height);
            state.draw_width = temp_width;
            state.draw_height = temp_height;

            float offset_x = (this.dimmen_opposite - state.draw_width) / 2f;
            float offset_y = this.dimmen_length - state.draw_height;

            switch (corner) {
                case Corner.TOPLEFT:// upscroll
                    state.offset_x = offset_x;
                    break;
                case Corner.BOTTOMRIGHT:// downscroll
                    state.offset_x = offset_x;
                    state.offset_y = offset_y;
                    break;
                case Corner.TOPRIGHT:// leftscroll
                    state.offset_y = offset_x;
                    break;
                case Corner.BOTTOMLEFT:// rightscroll
                    state.offset_x = offset_y;
                    break;
            }

        }

        private void InternalStatesRecalculate() {
            this.markers_scale = -1.0f;
            Strum.InternalCalcStates(this.sprite_marker_nothing, InternalCalcStateMarker);
            Strum.InternalCalcStates(this.sprite_marker_confirm, InternalCalcStateMarker);
            Strum.InternalCalcStates(this.sprite_marker_press, InternalCalcStateMarker);
            Strum.InternalCalcStates(this.sprite_sick_effect, InternalCalcStateSickEffect);
            Strum.InternalCalcStates(this.sprite_background, InternalCalcStateBackground);
        }



        private static void InternalExtraDestroyAnimation(AttachedAnimations holder) {
            if (holder.hit_up.action != null) holder.hit_up.action.Destroy();
            if (holder.hit_down.action != null) holder.hit_down.action.Destroy();
            if (holder.miss.action != null) holder.miss.action.Destroy();
            if (holder.penality.action != null) holder.penality.action.Destroy();
            if (holder.idle.action != null) holder.idle.action.Destroy();
            if (holder.continuous.action != null) holder.continuous.action.Destroy();

            if (holder.hit_up.undo != null) holder.hit_up.undo.Destroy();
            if (holder.hit_down.undo != null) holder.hit_down.undo.Destroy();
            if (holder.miss.undo != null) holder.miss.undo.Destroy();
            if (holder.penality.undo != null) holder.penality.undo.Destroy();
            if (holder.idle.undo != null) holder.idle.undo.Destroy();
            //if (holder.continuous.undo != null) holder.continuous.undo.Destroy();// never used
        }

        private int InternalExtraAnimate(StrumScriptTarget target, StrumScriptOn @event, bool undo, float elapsed) {
            AttachedAnimations holder = InternalExtraGetHolder(target);
            AttachedAnimations.Action subholder;

            if (undo) {
                if (@event == StrumScriptOn.ALL) return 1;

                // execute "undo" animation of the previous event
                subholder = InternalExtraGetSubholder(holder, @event);

                if (subholder.undo != null) {
                    subholder.undo.Restart();
                    subholder.undo.Animate(elapsed);
                    InternalExtraAnimateSprite(target, subholder.undo);
                }
                return 1;
            }


            //
            // painful check:
            //      - differentiate between idle and keyup events
            //      - also wait for miss/penality/keyup animation end on idle events
            //
            if (@event == StrumScriptOn.IDLE) {
                switch (holder.state) {
                    case StrumExtraState.WAIT:
                        @event = holder.last_event;
                        break;
                    case StrumExtraState.NONE:
                        if (holder.last_event == StrumScriptOn.HIT_DOWN) {
                            holder.state = StrumExtraState.WAIT;
                            @event = StrumScriptOn.HIT_UP;
                        }
                        break;
                }
            } else {
                switch (@event) {
                    case StrumScriptOn.MISS:
                    case StrumScriptOn.PENALITY:
                        holder.state = StrumExtraState.WAIT;
                        break;
                    default:
                        holder.state = StrumExtraState.NONE;
                        break;
                }
            }

            bool changed = @event != holder.last_event;
            subholder = InternalExtraGetSubholder(holder, @event);

            if (changed)
                InternalExtraAnimate(target, holder.last_event, true, float.PositiveInfinity);

            holder.last_event = @event;
            int completed = 1;

            if (subholder.action != null) {
                if (changed)
                    subholder.action.Restart();

                completed = subholder.action.Animate(elapsed);
                InternalExtraAnimateSprite(target, subholder.action);
            }

            if (completed > 0) {
                switch (@event) {
                    case StrumScriptOn.HIT_UP:
                    case StrumScriptOn.MISS:
                    case StrumScriptOn.PENALITY:
                        holder.state = StrumExtraState.COMPLETED;
                        break;
                    default:
                        holder.state = StrumExtraState.NONE;
                        break;
                }
            }

            return completed;
        }

        private void InternalExtraAnimateSprite(StrumScriptTarget target, AnimSprite animsprite) {
            if (animsprite == null) return;

            switch (target) {
                case StrumScriptTarget.STRUM_LINE:
                    animsprite.UpdateDrawable(this.drawable, true);
                    break;
                case StrumScriptTarget.NOTE:
                    int last_index = this.chart_notes_id_map_size - 1;
                    for (int i = 0 ; i < this.chart_notes_id_map_size ; i++) {
                        if (this.drawable_notes[i] != null) {
                            StateSprite statesprite = this.drawable_notes[i].PeekAloneStateSprite();
                            animsprite.UpdateStatesprite(statesprite, i == last_index);
                        }
                    }
                    break;
                case StrumScriptTarget.MARKER:
                    animsprite.UpdateStatesprite(this.sprite_marker_confirm, false);
                    animsprite.UpdateStatesprite(this.sprite_marker_nothing, false);
                    animsprite.UpdateStatesprite(this.sprite_marker_press, true);
                    break;
                case StrumScriptTarget.SICK_EFFECT:
                    animsprite.UpdateStatesprite(this.sprite_sick_effect, true);
                    break;
                case StrumScriptTarget.BACKGROUND:
                    animsprite.UpdateStatesprite(this.sprite_background, false);
                    break;
            }
        }

        private AttachedAnimations InternalExtraGetHolder(StrumScriptTarget target) {
            switch (target) {
                case StrumScriptTarget.STRUM_LINE:
                    return this.animation_strum_line;
                case StrumScriptTarget.NOTE:
                    return this.animation_note;
                case StrumScriptTarget.MARKER:
                    return this.animation_marker;
                case StrumScriptTarget.SICK_EFFECT:
                    return this.animation_sick_effect;
                case StrumScriptTarget.BACKGROUND:
                    return this.animation_background;
                default:
                    return null;
            }
        }

        private AttachedAnimations.Action InternalExtraGetSubholder(AttachedAnimations holder, StrumScriptOn @event) {
            switch (@event) {
                case StrumScriptOn.HIT_UP:
                    return holder.hit_up;
                case StrumScriptOn.HIT_DOWN:
                    return holder.hit_down;
                case StrumScriptOn.MISS:
                    return holder.miss;
                case StrumScriptOn.PENALITY:
                    return holder.penality;
                case StrumScriptOn.IDLE:
                    return holder.idle;
                default:
                    return null;
            }
        }


        private void InternalCheckSustainQueue(double song_timestamp, PlayerStats playerstats) {
            int size = this.sustain_queue.Size();
            StrumNote[] array = this.sustain_queue.PeekArray();

            for (int i = 0 ; i < size ; i++) {
                StrumNote note = array[i];
                double end_timestamp = note.EndTimestamp;
                NoteAttribute note_attributes = this.attribute_notes[note.id];
                bool is_released = note.state == NoteState.RELEASE;// ¿was early released?
                int quarter = (int)(
                    (note.duration - (end_timestamp - song_timestamp)) / this.marker_duration_quarter
                );

                if (quarter <= 0 || quarter <= note.previous_quarter) continue;

                if (song_timestamp > end_timestamp) {
                    // the sustain note is still in hold (the player keeps holding the button)
                    sustain_queue.RemoveAt(i);
                    i--;
                    size--;
                    continue;
                }

                if (is_released && !note_attributes.ignore_miss || !is_released && !note_attributes.ignore_hit) {
                    int quarters = quarter - note.previous_quarter;
                    playerstats.AddSustain(quarters, is_released);

                    // Mark press state as missed after the 4 first quarters 
                    if (is_released && quarter > 4) {
                        double total_quarters = (note.duration / this.marker_duration_quarter) - 2;
                        // also ignore last 2 quarters
                        if (quarter < total_quarters) InternalUpdatePressState(StrumPressState.MISS);
                    }

                    //Console.Error.WriteLine($"[LOG] [sustain] ts={note.timestamp} release={is_released} quarters={quarters}");
                }

                note.previous_quarter = quarter;
            }
        }

        private void InternalLoadSickEffectState(ModelHolder modelholder, string state_name) {
            StateSpriteState state = InternalStateAdd(
                this.sprite_sick_effect, modelholder, this.strum_name,
                Strum.SICK_EFFECT_SUFFIX, state_name
            );
            if (state != null) InternalCalcStateSickEffect(state);
        }

        private void InternalSetNoteDrawables(NotePool notepool) {
            // free() previous arrays
            //if (this.drawable_notes) free(this.drawable_notes);
            //if (this.attribute_notes) free(this.attribute_notes);

            this.drawable_notes = new Note[this.chart_notes_id_map_size];
            this.attribute_notes = new NoteAttribute[this.chart_notes_id_map_size];

            for (int i = 0 ; i < this.chart_notes_id_map_size ; i++) {
                int id = this.chart_notes_id_map[i];

                if (id < 0 || id >= notepool.size) {
                    Console.Error.WriteLine("[ERROR] Invalid note id found in the chart: " + id);
                    this.drawable_notes[i] = null;
                    this.attribute_notes[i] = null;
                    continue;
                }

                this.drawable_notes[i] = notepool.drawables[id];
                this.attribute_notes[i] = notepool.attributes[id];

                if (notepool.models_custom_sick_effect[id] != null && !String.IsNullOrEmpty(notepool.attributes[id].custom_sick_effect_name)) {
                    InternalLoadSickEffectState(
                         notepool.models_custom_sick_effect[id],
                         notepool.attributes[id].custom_sick_effect_name
                     );
                }
            }
        }

        private static void InternalExtraBatch(AttachedAnimations holder, bool restart_or_end) {
            if (holder.hit_up.action != null)
                if (restart_or_end) holder.hit_up.action.Restart(); else holder.hit_up.action.ForceEnd();
            if (holder.hit_down.action != null)
                if (restart_or_end) holder.hit_down.action.Restart(); else holder.hit_down.action.ForceEnd();
            if (holder.miss.action != null)
                if (restart_or_end) holder.miss.action.Restart(); else holder.miss.action.ForceEnd();
            if (holder.penality.action != null)
                if (restart_or_end) holder.penality.action.Restart(); else holder.penality.action.ForceEnd();
            if (holder.idle.action != null)
                if (restart_or_end) holder.idle.action.Restart(); else holder.idle.action.ForceEnd();
        }

        private void InternalUpdatePressState(StrumPressState press_state) {
            if (press_state != StrumPressState.INVALID) {
                //if (press_state == this.press_state) return;
                this.press_state = press_state;
                this.press_state_use_alt_anim = false;
                this.press_state_changes++;
            }
        }

        private void InternalResetScrolling() {
            this.notes_peek_index = 0;
            this.notes_render_index = 0;
            this.last_song_timestamp = 0;
            this.marker_state = StrumMarkerState.NOTHING;
            this.marker_state_changed = true;
            this.marker_sick_state = StrumMarkerState.NOTHING;
            this.marker_sick_state_name = null;
            this.extra_animations_have_penalties = false;
            this.extra_animations_have_misses = false;
            this.auto_scroll_elapsed = -1;

            this.press_state_changes = -1;
            this.press_state = StrumPressState.NONE;

            this.animation_background.last_event = StrumScriptOn.ALL;
            this.animation_marker.last_event = StrumScriptOn.ALL;
            this.animation_sick_effect.last_event = StrumScriptOn.ALL;
            this.animation_strum_line.last_event = StrumScriptOn.ALL;
            this.animation_note.last_event = StrumScriptOn.ALL;

            this.animation_background.state = StrumExtraState.NONE;
            this.animation_marker.state = StrumExtraState.NONE;
            this.animation_sick_effect.state = StrumExtraState.NONE;
            this.animation_strum_line.state = StrumExtraState.NONE;
            this.animation_note.state = StrumExtraState.NONE;
        }

        private void InternalCalcMarkerDuration(double velocity) {
            if (this.use_fukin_marker_duration)
                this.marker_duration = Funkin.MARKER_DURATION;
            else
                this.marker_duration = (float)Math.Abs(this.dimmen_marker / velocity) * Strum.ADDITIONAL_MARKER_DURATION_RATIO;

            this.marker_duration *= this.marker_duration_multiplier;

            this.minimum_sustain_duration = (float)(this.marker_duration * Funkin.NOTE_MIN_SUSTAIN_THRESHOLD);
            this.marker_duration_quarter = this.marker_duration / 4.0f;

            if (this.chart_notes_size > 0)
                this.key_test_limit = Math.Max(this.chart_notes[0].timestamp - this.marker_duration, 0);
            else
                this.key_test_limit = double.NegativeInfinity;
        }

        private void InternalRestartExtraContinous() {
            if (this.animation_strum_line.continuous.action != null)
                this.animation_strum_line.continuous.action.Restart();
            if (this.animation_note.continuous.action != null)
                this.animation_note.continuous.action.Restart();
            if (this.animation_marker.continuous.action != null)
                this.animation_marker.continuous.action.Restart();
            if (this.animation_sick_effect.continuous.action != null)
                this.animation_sick_effect.continuous.action.Restart();
            if (this.animation_background.continuous.action != null)
                this.animation_background.continuous.action.Restart();
        }


        private delegate void CalcCallback(StateSpriteState state);

        private class AttachedAnimations {
            public class Action { public AnimSprite action; public AnimSprite undo; }

            public Action hit_up;
            public Action hit_down;
            public Action penality;
            public Action miss;
            public Action idle;
            public Action continuous;
            public StrumScriptOn last_event;
            public StrumExtraState state;

            public AttachedAnimations() {
                this.hit_up = new Action() { action = null, undo = null };
                this.hit_down = new Action() { action = null, undo = null };
                this.penality = new Action() { action = null, undo = null };
                this.miss = new Action() { action = null, undo = null };
                this.idle = new Action() { action = null, undo = null };
                this.continuous = new Action() { action = null };
                this.last_event = StrumScriptOn.ALL;
                this.state = StrumExtraState.NONE;
            }

        }

    }

}
