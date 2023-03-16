using System;
using Engine.Externals.LuaScriptInterop;
using Engine.Utils;

namespace Engine.Game {

    public class Conductor {

        private Character character;
        private ArrayList<Mapping> mappings;
        private bool disable;
        private Strum last_penality_strum;
        private Strum last_sustain_strum;
        private MissNoteFX missnotefx;
        private bool has_misses;
        private bool has_hits;
        private int play_calls;



        public Conductor() {

            this.character = null;
            this.disable = false;
            this.mappings = new ArrayList<Mapping>(4);
            this.last_penality_strum = null;
            this.last_sustain_strum = null;
            this.missnotefx = null;
            this.has_misses = false;
            this.has_hits = false;
            this.play_calls = 0;
        }

        public void Destroy() {
            Luascript.DropShared(this);

            ClearMapping();
            this.mappings.Destroy(false);
            //free(this);
        }


        public void PollReset() {
            foreach (Mapping map in this.mappings) {
                map.last_change_count = -1;
            }
            this.last_penality_strum = null;
            this.last_sustain_strum = null;
            this.has_misses = false;
            this.has_hits = false;
        }

        public void SetCharacter(Character character) {
            if (this.character != character) this.last_sustain_strum = this.last_penality_strum = null;
            this.character = character;

            if (character != null)
                this.play_calls = character.GetPlayCalls() - 1;
        }

        public void UseStrumLine(Strum strum) {
            if (strum == null) return;

            // check if is already in use
            foreach (Mapping map in this.mappings) {
                if (map.strum == strum) {
                    map.is_disabled = false;
                    return;
                }
            }

            this.mappings.Add(new Mapping() {
                strum = strum,
                last_change_count = -1,
                directions = new ArrayList<MapDirection>(4),
                is_disabled = false
            });
        }

        public void UseStrums(Strums strums) {
            if (strums == null) return;

            int size = strums.GetLinesCount();
            for (int i = 0 ; i < size ; i++) {
                Strum strum = strums.GetStrumLine(i);
                UseStrumLine(strum);
            }
        }

        public bool DisableStrumLine(Strum strum, bool should_disable) {
            if (strum == null) return false;
            foreach (Mapping map in this.mappings) {
                if (map.strum == strum) {
                    map.is_disabled = !!should_disable;
                    return true;
                }
            }
            return false;
        }

        public bool RemoveStrum(Strum strum) {
            Mapping[] array = this.mappings.PeekArray();
            int size = this.mappings.Size();
            for (int i = 0 ; i < size ; i++) {
                if (array[i].strum == strum) {
                    InternalDisposedMappedStrum(array[i]);
                    this.mappings.RemoveAt(i);
                    return true;
                }
            }
            return false;
        }

        public void ClearMapping() {
            foreach (Mapping map in this.mappings) {
                InternalDisposedMappedStrum(map);
            }
            this.mappings.Clear();
            this.last_sustain_strum = this.last_penality_strum = null;
        }


        public void MapStrumToPlayerSingAdd(Strum strum, string sing_direction_name) {
            IntenalAddMapping(strum, sing_direction_name, false);
        }

        public void MapStrumToPlayerExtraAdd(Strum strum, string extra_animation_name) {
            IntenalAddMapping(strum, extra_animation_name, true);
        }

        public void MapStrumToPlayerSingRemove(Strum strum, string sing_direction_name) {
            IntenalRemoveMapping(strum, sing_direction_name, false);
        }

        public void MapStrumToPlayerExtraRemove(Strum strum, string extra_animation_name) {
            IntenalRemoveMapping(strum, extra_animation_name, true);
        }


        public int MapAutomatically(bool should_map_extras) {
            if (this.character == null) {
                Console.Error.WriteLine("[ERROR] conductor_map_automatically() failed, no character set");
                return 0;
            }

            Mapping[] mappings = this.mappings.PeekArray();
            int size = this.mappings.Size();

            if (size < 1) {
                Console.Error.WriteLine("[ERROR] conductor_map_automatically() failed, no strums are set");
                return 0;
            }

            int count = 0;
            for (int i = 0 ; i < size ; i++) {
                string strum_name = mappings[i].strum.GetName();

                if (this.character.HasDirection(strum_name, false)) {
                    // strum_name --> sing_direction
                    MapStrumToPlayerSingAdd(mappings[i].strum, strum_name);
                    count++;
                    continue;
                } else if (should_map_extras && this.character.HasDirection(strum_name, true)) {
                    // strum_mame --> extra_animation
                    MapStrumToPlayerExtraAdd(mappings[i].strum, strum_name);
                    count++;
                    break;
                }
            }

            return count;
        }

        public void SetMissnotefx(MissNoteFX missnotefx) {
            this.missnotefx = missnotefx;
        }

        public void Poll() {
            if (this.disable || this.character == null) return;

            Mapping[] array = this.mappings.PeekArray();
            int size = this.mappings.Size();
            int success = 0;

            this.has_hits = false;
            this.has_misses = false;

            for (int i = 0 ; i < size ; i++) {
                if (array[i].is_disabled) continue;

                int press_changes = array[i].strum.GetPressStateChanges();
                StrumPressState press_state = array[i].strum.GetPressState();
                bool press_state_use_alt_anim = array[i].strum.GetPressStateUseAltAnim();

                // check if the strums was updated
                if (press_changes == array[i].last_change_count) continue;
                array[i].last_change_count = press_changes;

                if (press_state_use_alt_anim) this.character.UseAlternateSingAnimations(true);

                switch (press_state) {
                    case StrumPressState.MISS:
                        this.has_misses = true;
                        goto case StrumPressState.NONE;
                    case StrumPressState.NONE:
                        if (this.last_penality_strum == array[i].strum) {
                            // stop the penality animation in the next beat
                            success += InternalExecuteMiss(this.character, array[i].directions, false);
                            this.last_penality_strum = null;
                        } else if (this.last_sustain_strum == null || this.last_sustain_strum == array[i].strum) {
                            this.last_sustain_strum = null;
                            if (!this.character.IsIdleActive()) {
                                //
                                // only play the idle animation if the last action
                                // was done by the conductor itself
                                //
                                if (this.play_calls == this.character.GetPlayCalls()) {
                                    this.character.PlayIdle();
                                    this.play_calls = this.character.GetPlayCalls();
                                }
                            }
                        }
                        break;
                    case StrumPressState.HIT:
                        //this.current_duration = array[i].strum.GetMarkerDuration();
                        this.last_sustain_strum = null;
                        this.last_penality_strum = null;
                        this.has_misses = false;
                        this.has_hits = true;
                        success += InternalExecuteSing(this.character, array[i].directions, false);
                        break;
                    case StrumPressState.HIT_SUSTAIN:
                        this.last_sustain_strum = array[i].strum;
                        this.last_penality_strum = null;
                        this.has_misses = false;
                        this.has_hits = true;
                        success += InternalExecuteSing(this.character, array[i].directions, true);
                        break;
                    case StrumPressState.PENALTY_NOTE:
                        // button press on empty strum
                        this.has_misses = true;
                        this.last_sustain_strum = null;
                        this.last_penality_strum = array[i].strum;
                        success += InternalExecuteMiss(this.character, array[i].directions, true);
                        if (this.missnotefx != null) this.missnotefx.PlayEffect();
                        break;
                    case StrumPressState.PENALTY_HIT:
                        // wrong note button
                        this.last_sustain_strum = null;
                        this.last_penality_strum = array[i].strum;
                        success += InternalExecuteMiss(this.character, array[i].directions, false);
                        if (this.missnotefx != null) this.missnotefx.PlayEffect();
                        break;
                    default:
                        break;
                }

                if (press_state_use_alt_anim) this.character.UseAlternateSingAnimations(false);
            }

            if (success > 0) {
                // remember the amount of played animations if latter is necessary play idle
                this.play_calls = this.character.GetPlayCalls();
            }

        }

        public bool HasMisses() {
            return this.has_misses;
        }

        public bool HasHits() {
            return this.has_hits;
        }

        public void Disable(bool disable) {
            this.disable = disable;
            this.last_sustain_strum = this.last_penality_strum = null;
        }


        public void PlayIdle() {
            if (this.character == null) return;
            this.last_sustain_strum = this.last_penality_strum = null;

            if (this.character.PlayIdle() > 0)
                this.play_calls = this.character.GetPlayCalls();
        }

        public void PlayHey() {
            if (this.character == null) return;
            this.last_sustain_strum = this.last_penality_strum = null;

            if (this.character.PlayHey())
                this.play_calls = this.character.GetPlayCalls();
        }

        public Character GetCharacter() {
            return this.character;
        }


        private void InternalDisposedMappedStrum(Mapping mapped_strum) {
            //foreach (MapDirection mapped_sing in mapped_strum.directions) {
            //    free(mapped_sing.name);
            //    free(mapped_sing);
            //}
            mapped_strum.directions.Destroy(false);
            mapped_strum.strum = null;
            //free(mapped_strum);
        }

        public void IntenalAddMapping(Strum strum, string name, bool is_extra) {
            Mapping mapped_strum = null;
            foreach (Mapping map in this.mappings) {
                if (map.strum == strum) {
                    mapped_strum = map;
                    break;
                }
            }

            if (mapped_strum == null) {
                mapped_strum = new Mapping() {
                    directions = new ArrayList<MapDirection>(4),
                    is_disabled = false,
                    last_change_count = -1,
                    strum = strum
                };
                this.mappings.Add(mapped_strum);
            }

            // check if already is added
            foreach (MapDirection item in mapped_strum.directions) {
                if (item.name == name && item.is_extra == is_extra) {
                    return;
                }
            }

            //name = strdup(name);
            mapped_strum.directions.Add(new MapDirection() { name = name, is_extra = is_extra });
        }

        public void IntenalRemoveMapping(Strum strum, string name, bool is_extra) {
            foreach (Mapping mapped_strum in this.mappings) {
                if (mapped_strum.strum != strum) continue;

                MapDirection[] directions = mapped_strum.directions.PeekArray();
                int size = mapped_strum.directions.Size();

                for (int i = 0 ; i < size ; i++) {
                    if (directions[i].name == name && directions[i].is_extra == is_extra) {
                        //free(directions[i].name);
                        //free(directions[i]);
                        mapped_strum.directions.RemoveAt(i);

                        return;
                    }
                }
            }
        }

        private static int InternalExecuteSing(Character character, ArrayList<MapDirection> mapping, bool is_sustain) {
            int done = 0;
            foreach (MapDirection direction in mapping) {
                if (direction.is_extra) {
                    if (character.PlayExtra(direction.name, is_sustain)) done++;
                } else {
                    if (character.PlaySing(direction.name, is_sustain)) done++;
                }
            }
            return done;
        }

        private static int InternalExecuteMiss(Character character, ArrayList<MapDirection> mapping, bool keep_in_hold) {
            int done = 0;
            foreach (MapDirection direction in mapping) {
                if (direction.is_extra) {
                    if (character.PlayExtra(direction.name, keep_in_hold)) done++;
                } else {
                    if (character.PlayMiss(direction.name, keep_in_hold) == 1) done++;
                }
            }
            return done;
        }


        private class MapDirection {
            public string name;
            public bool is_extra;
        }
        private class Mapping {
            public Strum strum;
            public ArrayList<MapDirection> directions;
            public bool is_disabled;
            public int last_change_count;
        }
    }

}
