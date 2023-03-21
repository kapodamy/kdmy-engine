
using System;
using Engine.Externals.LuaScriptInterop;
using Engine.Platform;
using KallistiOS;

namespace Engine.Game.Gameplay {

    public enum ScriptNote : uint {
        PENALITY = 0,
        MISS = 1,
        SHIT = 2,
        BAD = 3,
        GOOD = 4,
        SICK = 5
    }

    public class WeekScript {

        private Luascript luascript;


        public static WeekScript Init(string src, object context, bool is_week) {
            string lua_sourcecode = FS.ReadText(src);
            if (lua_sourcecode == null) {
                string path = FS.GetFullPathAndOverride(src);
                Console.Error.WriteLine("[ERROR] weekscript_init() missing file '" + path + "'");
                return null;
            }

            // init luascript
            string filename = FS.GetFilenameWithoutExtension(src);
            Luascript luascript = Luascript.Init(lua_sourcecode, filename, context, is_week);
            //free(filename);

            if (luascript == null) {
                Console.Error.WriteLine($"[ERROR] weekscript_init() call to luascript_init() failed lua={src}");
                return null;
            }

            LuascriptPlatform.AddLuascript(luascript);
            return new WeekScript() { luascript = luascript };
        }

        public void Destroy() {
            //if (!this) return;
            LuascriptPlatform.RemoveLuascript(this.luascript);
            LuascriptPlatform.RestoreWindowTitleIfChanged();
            this.luascript.Destroy();
        }


        public void NotifyWeekinit(int freeplay_song_index) {
            this.luascript.notify_weekinit(freeplay_song_index);
        }

        public void NotifyBeforeready(bool from_retry) {
            this.luascript.notify_beforeready(from_retry);
        }

        public void NotifyReady() {
            this.luascript.notify_ready();
        }


        public void NotifyAftercountdown() {
            this.luascript.notify_aftercountdown();
        }

        public void NotifyFrame(float elapsed) {
            long timestamp = (long)timer.ms_gettime64();
            this.luascript.notify_timer_run(timestamp);
            this.luascript.notify_frame(elapsed);
        }

        public void NotifyTimerSong(double song_timestamp) {
            long timestamp = (long)timer.ms_gettime64();
            this.luascript.notify_timersong_run(song_timestamp);
        }

        public void NotifyNoteHit(Strum strum, int strum_note_index, PlayerStats playerstats) {
            /** @type {StrumNote} */
            Strum.StrumNote strum_note = strum.chart_notes[strum_note_index];
            double timestamp = strum_note.timestamp;
            double duration = strum_note.duration;
            int note_id = strum.chart_notes_id_map[strum_note.id];
            bool special = strum.attribute_notes[strum_note.id].is_special;
            double data = strum_note.custom_data;
            int player_id = strum.player_id;

            ScriptNote state;
            switch (playerstats.last_ranking) {
                case Ranking.SICK:
                    state = ScriptNote.SICK;
                    break;
                case Ranking.GOOD:
                    state = ScriptNote.GOOD;
                    break;
                case Ranking.BAD:
                    state = ScriptNote.BAD;
                    break;
                case Ranking.SHIT:
                    state = ScriptNote.SHIT;
                    break;
                default:
                    return;
            }

            InternalNotifyNote(timestamp, note_id, duration, data, special, player_id, state);
        }

        public void NotifyNoteLoss(Strum strum, int strum_note_idx, PlayerStats plyrstts, bool is_penalty) {
            /** @type {StrumNote} */
            Strum.StrumNote strum_note = strum.chart_notes[strum_note_idx];
            bool ignore_miss = strum.attribute_notes[strum_note.id].ignore_miss;

            if (ignore_miss) return;

            double timestamp = strum_note.timestamp;
            double duration = strum_note.duration;
            int note_id = strum.chart_notes_id_map[strum_note.id];
            double data = strum_note.custom_data;
            bool special = strum.attribute_notes[strum_note.id].is_special;
            int player_id = strum.player_id;
            ScriptNote state = is_penalty ? ScriptNote.PENALITY : ScriptNote.MISS;

            InternalNotifyNote(timestamp, note_id, duration, data, special, player_id, state);
        }

        public void NotifyUnknownNote(int player_id, double timestamp, int direction, double duration, double data) {
            this.luascript.notify_unknownnote(player_id, timestamp, direction, duration, data);
        }

        public void NotifyRoundend(bool loose) {
            this.luascript.notify_roundend(loose);
        }

        public void NotifyWeekend(bool giveup) {
            this.luascript.notify_weekend(giveup);
        }

        public void NotifyGameoverloop() {
            this.luascript.notify_gameoverloop();
        }

        public void NotifyGameoverdecision(bool retry_or_giveup, string changed_difficult) {
            this.luascript.notify_gameoverdecision(retry_or_giveup, changed_difficult);
        }

        public void NotifyGameoverended() {
            this.luascript.notify_gameoverended();
        }

        public void NotifyPause(bool pause_or_resume) {
            this.luascript.notify_pause(pause_or_resume);
        }

        public void NotifyWeekleave() {
            this.luascript.notify_weekleave();
        }

        public void NotifyAfterresults(int total_attempts, int songs_count, bool reject_completed) {
            this.luascript.notify_afterresults(total_attempts, songs_count, reject_completed);
        }

        public void NotifyScriptchange() {
            this.luascript.notify_scriptchange();
        }

        public void NotifyPauseOptionChoosen(int option_index) {
            this.luascript.notify_pause_optionchoosen(option_index);
        }

        public void NotifyPauseMenuvisible(bool shown_or_hidden) {
            this.luascript.notify_pause_menuvisible(shown_or_hidden);
        }

        public void NotifyBeat(int beat_count, float since) {
            this.luascript.notify_beat(beat_count, since);
        }

        public void NotifyQuarter(int quarter_count, float since) {
            this.luascript.notify_quarter(quarter_count, since);
        }

        public void NotifyButtons(int player_id, GamepadButtons buttons) {
            this.luascript.notify_buttons(player_id, (uint)buttons);
        }

        public void NotifyAfterStrumScroll() {
            this.luascript.notify_after_strum_scroll();
        }

        public void NotifyDialogueBuiltinOpen(string dialog_src) {
            this.luascript.notify_dialogue_builtin_open(dialog_src);
        }


        public Luascript GetLuaScript() {
            return this.luascript;
        }


        private void InternalNotifyNote(double timestamp, int id, double duration, double data, bool special, int player_id, ScriptNote state) {
            this.luascript.notify_note(timestamp, id, duration, data, special, player_id, (uint)state);
        }
    }

}
