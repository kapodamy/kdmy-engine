using System;
using Engine.Externals.LuaScriptInterop;
using Engine.Platform;
using KallistiOS;

namespace Engine.Game.Gameplay;

public class WeekScript {

    private Luascript luascript;


    public static WeekScript Init(string src, object context, bool is_week) {
        string lua_sourcecode = FS.ReadText(src);
        if (lua_sourcecode == null) {
            string path = FS.GetFullPathAndOverride(src);
            Logger.Error($"weekscript_init() missing file '{path}'");
            //free(path);
            return null;
        }

        string lua_filename = FS.GetFilenameWithoutExtension(src);
        string tmp_full_path = FS.GetFullPath(src);
        string working_folder = FS.GetParentFolder(tmp_full_path);

        //free(tmp_full_path);
    
        // init luascript
        Luascript luascript = Luascript.Init(lua_sourcecode, lua_filename, working_folder, context, is_week);

        // Note: the "working_folder" is deallocated by luascript_destroy() function
        //free(lua_sourcecode);
        //free(lua_filename);

        if (luascript == null) {
            Logger.Error($"weekscript_init() call to luascript_init() failed lua={src}");
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

    public void NotifyBeforeresults() {
        this.luascript.notify_beforeresults();
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

    public void NotifyNote(double timestamp, int id, double duration, double data, bool special, int player_id, Ranking ranking) {
        this.luascript.notify_note(timestamp, id, duration, data, special, player_id, (uint)ranking);
    }


    public Luascript GetLuaScript() {
        return this.luascript;
    }

}
