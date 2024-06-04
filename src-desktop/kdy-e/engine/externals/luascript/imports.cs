
namespace Engine.Externals.LuaScriptInterop;

public partial class Luascript {

    public void notify_weekinit(int freeplay_song_index) {
        const string FUNCTION = "f_weekinit";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushinteger(freeplay_song_index);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_beforeready(bool from_retry) {
        const string FUNCTION = "f_beforeready";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushboolean(from_retry);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_ready() {
        const string FUNCTION = "f_ready";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        lua.CallPushedGlobalFunction(0);
    }

    public void notify_aftercountdown() {
        const string FUNCTION = "f_aftercountdown";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        lua.CallPushedGlobalFunction(0);
    }

    public void notify_frame(float elapsed) {
        const string FUNCTION = "f_frame";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushnumber(elapsed);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_roundend(bool loose) {
        const string FUNCTION = "f_roundend";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushboolean(loose);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_weekend(bool giveup) {
        const string FUNCTION = "f_weekend";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushboolean(giveup);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_gameoverloop() {
        const string FUNCTION = "f_gameoverloop";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        lua.CallPushedGlobalFunction(0);
    }

    public void notify_gameoverdecision(bool retry_or_giveup, string changed_difficult) {
        const string FUNCTION = "f_gameoverdecision";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushboolean(retry_or_giveup);
        L.lua_pushstring(changed_difficult);

        lua.CallPushedGlobalFunction(2);
    }

    public void notify_gameoverended() {
        const string FUNCTION = "f_gameoverended";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        lua.CallPushedGlobalFunction(0);
    }

    public void notify_pause(bool pause_or_resume) {
        const string FUNCTION = "f_pause";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushboolean(pause_or_resume);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_weekleave() {
        const string FUNCTION = "f_weekleave";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        lua.CallPushedGlobalFunction(0);
    }

    public void notify_beforeresults() {
        const string FUNCTION = "f_beforeresults";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        lua.CallPushedGlobalFunction(0);
    }

    public void notify_afterresults(int total_attempts, int songs_count, bool reject_completed) {
        const string FUNCTION = "f_afterresults";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        L.lua_pushinteger(total_attempts);
        L.lua_pushinteger(songs_count);
        L.lua_pushboolean(reject_completed);

        lua.CallPushedGlobalFunction(3);
    }

    public void notify_scriptchange() {
        const string FUNCTION = "f_scriptchange";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        lua.CallPushedGlobalFunction(0);
    }

    public void notify_pause_optionchoosen(int option_index) {
        const string FUNCTION = "f_pause_optionchoosen";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushinteger(option_index);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_pause_menuvisible(bool shown_or_hidden) {
        const string FUNCTION = "f_pause_menuvisible";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushboolean(shown_or_hidden);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_note(double timestamp, int id, double duration, double data, bool special, int player_id, uint state) {
        const string FUNCTION = "f_note";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushnumber(timestamp);
        L.lua_pushinteger(id);
        L.lua_pushnumber(duration);
        L.lua_pushnumber(data);
        L.lua_pushboolean(special);
        L.lua_pushinteger(player_id);
        L.lua_pushinteger(state);

        lua.CallPushedGlobalFunction(7);
    }

    public void notify_buttons(int player_id, uint buttons) {
        const string FUNCTION = "f_buttons";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushinteger(player_id);
        L.lua_pushinteger(buttons);

        lua.CallPushedGlobalFunction(2);
    }

    public void notify_unknownnote(int player_id, double timestamp, int id, double duration, double data) {
        const string FUNCTION = "f_unknownnote";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushinteger(player_id);
        L.lua_pushnumber(timestamp);
        L.lua_pushinteger(id);
        L.lua_pushnumber(duration);
        L.lua_pushnumber(data);

        lua.CallPushedGlobalFunction(5);
    }

    public void notify_beat(int current_beat, float since) {
        const string FUNCTION = "f_beat";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushinteger(current_beat);
        L.lua_pushnumber(since);

        lua.CallPushedGlobalFunction(2);
    }

    public void notify_quarter(int current_quarter, float since) {
        const string FUNCTION = "f_quarter";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushinteger(current_quarter);
        L.lua_pushnumber(since);

        lua.CallPushedGlobalFunction(2);
    }

    public void notify_timer_run(double kos_timestamp) {
        const string FUNCTION = "______kdmyEngine_timer_run";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushnumber(kos_timestamp);
        L.lua_pushboolean(false);

        lua.CallPushedGlobalFunction(2);
    }

    public void notify_timersong_run(double song_timestamp) {
        const string FUNCTION = "______kdmyEngine_timer_run";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushnumber(song_timestamp);
        L.lua_pushboolean(true);

        lua.CallPushedGlobalFunction(2);
    }

    public void notify_dialogue_builtin_open(string dialog_src) {
        const string FUNCTION = "f_dialogue_builtin_open";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushstring(dialog_src);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_dialogue_line_starts(int line_index, string state_name, string text) {
        const string FUNCTION = "f_dialogue_line_starts";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushinteger(line_index);
        L.lua_pushstring(state_name);
        L.lua_pushstring(text);

        lua.CallPushedGlobalFunction(3);
    }

    public void notify_dialogue_line_ends(int line_index, string state_name, string text) {
        const string FUNCTION = "f_dialogue_line_ends";
        if (lua.PushGlobalFunction(FUNCTION)) return;


        L.lua_pushinteger(line_index);
        L.lua_pushstring(state_name);
        L.lua_pushstring(text);

        lua.CallPushedGlobalFunction(3);
    }

    public void notify_after_strum_scroll() {
        const string FUNCTION = "f_after_strum_scroll";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        lua.CallPushedGlobalFunction(0);
    }

    public void notify_modding_menu_option_selected(object menu, int index, string name) {
        const string FUNCTION = "f_modding_menu_option_selected";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        L.CreateUserdata(ExportsMenu.MENU, menu);
        L.lua_pushinteger(index);
        L.lua_pushstring(name);

        lua.CallPushedGlobalFunction(3);
    }

    public bool notify_modding_menu_option_choosen(object menu, int index, string name) {
        const string FUNCTION = "f_modding_menu_option_choosen";
        if (lua.PushGlobalFunction(FUNCTION)) return false;

        L.CreateUserdata(ExportsMenu.MENU, menu);
        L.lua_pushinteger(index);
        L.lua_pushstring(name);

        return lua.CallPushedGlobalFunctionWithReturnBool(3);
    }

    public bool notify_modding_back() {
        const string FUNCTION = "f_modding_back";
        if (lua.PushGlobalFunction(FUNCTION)) return false;

        return lua.CallPushedGlobalFunctionWithReturnBool(0);
    }

    public object notify_modding_exit() {
        const string FUNCTION = "f_modding_exit";
        if (lua.PushGlobalFunction(FUNCTION)) return false;

        return lua.CallPushedGlobalFunctionWithReturn(0);
    }

    public void notify_modding_init(object value) {
        const string FUNCTION = "f_modding_init";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        LuascriptHelpers.PushModdingValue(lua.LuaStateHandle, value);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_modding_event(string event_name) {
        const string FUNCTION = "f_modding_event";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        L.lua_pushstring(event_name);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_modding_handle_custom_option(string option_name) {
        const string FUNCTION = "f_modding_handle_custom_option";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        L.lua_pushstring(option_name);
        lua.CallPushedGlobalFunction(1);
    }

    public void notify_modding_window_focus(bool focused) {
        if (is_week) return;
        const string FUNCTION = "f_modding_window_focus";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        L.lua_pushboolean(focused);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_modding_window_minimized(bool minimized) {
        if (is_week) return;
        const string FUNCTION = "f_modding_window_minimized";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        L.lua_pushboolean(minimized);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_window_size_changed(int screen_width, int screen_height) {
        const string FUNCTION = "f_window_size_changed";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        L.lua_pushinteger(screen_width);
        L.lua_pushinteger(screen_height);

        lua.CallPushedGlobalFunction(2);
    }

    public void notify_input_keyboard(int key, int scancode, bool is_pressed, int mods) {
        const string FUNCTION = "f_input_keyboard";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        L.lua_pushinteger(key);
        L.lua_pushinteger(scancode);
        L.lua_pushboolean(is_pressed);
        L.lua_pushinteger(mods);

        lua.CallPushedGlobalFunction(4);
    }

    public void notify_input_mouse_position(double x, double y) {
        const string FUNCTION = "f_input_mouse_position";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        L.lua_pushnumber(x);
        L.lua_pushnumber(y);

        lua.CallPushedGlobalFunction(2);
    }

    public void notify_input_mouse_enter(bool entering) {
        const string FUNCTION = "f_input_mouse_enter";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        L.lua_pushboolean(entering);

        lua.CallPushedGlobalFunction(1);
    }

    public void notify_input_mouse_button(int button, bool is_pressed, int mods) {
        const string FUNCTION = "f_input_mouse_button";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        L.lua_pushinteger(button);
        L.lua_pushboolean(is_pressed);
        L.lua_pushinteger(mods);

        lua.CallPushedGlobalFunction(3);
    }

    public void notify_input_mouse_scroll(double x, double y) {
        const string FUNCTION = "f_input_mouse_scroll";
        if (lua.PushGlobalFunction(FUNCTION)) return;

        L.lua_pushnumber(x);
        L.lua_pushnumber(y);

        lua.CallPushedGlobalFunction(2);
    }

}
