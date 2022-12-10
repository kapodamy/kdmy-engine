#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(void, songprogressbar_set_songplayer, (SongProgressbar songprogressbar, SongPlayer songplayer), {
    songprogressbar_set_songplayer(kdmyEngine_obtain(songprogressbar), kdmyEngine_obtain(songplayer));
});
EM_JS_PRFX(void, songprogressbar_set_duration, (SongProgressbar songprogressbar, double duration), {
    songprogressbar_set_duration(kdmyEngine_obtain(songprogressbar), duration);
});
EM_JS_PRFX(Drawable, songprogressbar_get_drawable, (SongProgressbar songprogressbar), {
    let ret = songprogressbar_get_drawable(kdmyEngine_obtain(songprogressbar));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, songprogressbar_set_visible, (SongProgressbar songprogressbar, bool visible), {
    songprogressbar_set_visible(kdmyEngine_obtain(songprogressbar), visible);
});
EM_JS_PRFX(void, songprogressbar_set_background_color, (SongProgressbar songprogressbar, float r, float g, float b, float a), {
    songprogressbar_set_background_color(kdmyEngine_obtain(songprogressbar), r, g, b, a);
});
EM_JS_PRFX(void, songprogressbar_set_bar_back_color, (SongProgressbar songprogressbar, float r, float g, float b, float a), {
    songprogressbar_set_bar_back_color(kdmyEngine_obtain(songprogressbar), r, g, b, a);
});
EM_JS_PRFX(void, songprogressbar_set_bar_progress_color, (SongProgressbar songprogressbar, float r, float g, float b, float a), {
    songprogressbar_set_bar_progress_color(kdmyEngine_obtain(songprogressbar), r, g, b, a);
});
EM_JS_PRFX(void, songprogressbar_set_text_color, (SongProgressbar songprogressbar, float r, float g, float b, float a), {
    songprogressbar_set_text_color(kdmyEngine_obtain(songprogressbar), r, g, b, a);
});
EM_JS_PRFX(void, songprogressbar_hide_time, (SongProgressbar songprogressbar, bool hidden), {
    songprogressbar_hide_time(kdmyEngine_obtain(songprogressbar), hidden);
});
EM_JS_PRFX(void, songprogressbar_show_elapsed, (SongProgressbar songprogressbar, bool elapsed_or_remain_time), {
    songprogressbar_show_elapsed(kdmyEngine_obtain(songprogressbar), elapsed_or_remain_time);
});
EM_JS_PRFX(void, songprogressbar_manual_update_enable, (SongProgressbar songprogressbar, bool enabled), {
    songprogressbar_manual_update_enable(kdmyEngine_obtain(songprogressbar), enabled);
});
EM_JS_PRFX(void, songprogressbar_manual_set_text, (SongProgressbar songprogressbar, const char* text), {
    songprogressbar_manual_set_text(kdmyEngine_obtain(songprogressbar), kdmyEngine_ptrToString(text));
});
EM_JS_PRFX(double, songprogressbar_manual_set_position, (SongProgressbar songprogressbar, double elapsed, double duration, bool should_update_time_text), {
    let ret = songprogressbar_manual_set_position(kdmyEngine_obtain(songprogressbar), elapsed, duration, should_update_time_text);
    return ret;
});
EM_JS_PRFX(void, songprogressbar_animation_set, (SongProgressbar songprogressbar, AnimSprite animsprite), {
    songprogressbar_animation_set(kdmyEngine_obtain(songprogressbar), kdmyEngine_obtain(animsprite));
});
EM_JS_PRFX(void, songprogressbar_animation_restart, (SongProgressbar songprogressbar), {
    songprogressbar_animation_restart(kdmyEngine_obtain(songprogressbar));
});
EM_JS_PRFX(void, songprogressbar_animation_end, (SongProgressbar songprogressbar), {
    songprogressbar_animation_end(kdmyEngine_obtain(songprogressbar));
});
#endif


static int script_songprogressbar_set_songplayer(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    SongPlayer songplayer = luascript_read_nullable_userdata(L, 2, SONGPLAYER);

    songprogressbar_set_songplayer(songprogressbar, songplayer);

    return 0;
}

static int script_songprogressbar_set_duration(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    double duration = (double)luaL_checknumber(L, 2);

    songprogressbar_set_duration(songprogressbar, duration);

    return 0;
}

static int script_songprogressbar_get_drawable(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);

    Drawable ret = songprogressbar_get_drawable(songprogressbar);

    return script_drawable_new(L, ret);
}

static int script_songprogressbar_set_visible(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    bool visible = (bool)lua_toboolean(L, 2);

    songprogressbar_set_visible(songprogressbar, visible);

    return 0;
}

static int script_songprogressbar_set_background_color(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    float r = (float)luaL_checknumber(L, 2);
    float g = (float)luaL_checknumber(L, 3);
    float b = (float)luaL_checknumber(L, 4);
    float a = (float)luaL_checknumber(L, 5);

    songprogressbar_set_background_color(songprogressbar, r, g, b, a);

    return 0;
}

static int script_songprogressbar_set_bar_back_color(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    float r = (float)luaL_checknumber(L, 2);
    float g = (float)luaL_checknumber(L, 3);
    float b = (float)luaL_checknumber(L, 4);
    float a = (float)luaL_checknumber(L, 5);

    songprogressbar_set_bar_back_color(songprogressbar, r, g, b, a);

    return 0;
}

static int script_songprogressbar_set_bar_progress_color(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    float r = (float)luaL_checknumber(L, 2);
    float g = (float)luaL_checknumber(L, 3);
    float b = (float)luaL_checknumber(L, 4);
    float a = (float)luaL_checknumber(L, 5);

    songprogressbar_set_bar_progress_color(songprogressbar, r, g, b, a);

    return 0;
}

static int script_songprogressbar_set_text_color(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    float r = (float)luaL_checknumber(L, 2);
    float g = (float)luaL_checknumber(L, 3);
    float b = (float)luaL_checknumber(L, 4);
    float a = (float)luaL_checknumber(L, 5);

    songprogressbar_set_text_color(songprogressbar, r, g, b, a);

    return 0;
}

static int script_songprogressbar_hide_time(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    bool hidden = (bool)lua_toboolean(L, 2);

    songprogressbar_hide_time(songprogressbar, hidden);

    return 0;
}

static int script_songprogressbar_show_elapsed(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    bool elapsed_or_remain_time = (bool)lua_toboolean(L, 2);

    songprogressbar_show_elapsed(songprogressbar, elapsed_or_remain_time);

    return 0;
}

static int script_songprogressbar_manual_update_enable(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    bool enabled = (bool)lua_toboolean(L, 2);

    songprogressbar_manual_update_enable(songprogressbar, enabled);

    return 0;
}

static int script_songprogressbar_manual_set_text(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    const char* text = luaL_optstring(L, 2, NULL);

    songprogressbar_manual_set_text(songprogressbar, text);

    return 0;
}

static int script_songprogressbar_manual_set_position(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    double elapsed = (double)luaL_checknumber(L, 2);
    double duration = (double)luaL_checknumber(L, 3);
    bool should_update_time_text = (bool)lua_toboolean(L, 4);

    double ret = songprogressbar_manual_set_position(songprogressbar, elapsed, duration, should_update_time_text);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_songprogressbar_animation_set(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    songprogressbar_animation_set(songprogressbar, animsprite);

    return 0;
}

static int script_songprogressbar_animation_restart(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);

    songprogressbar_animation_restart(songprogressbar);

    return 0;
}

static int script_songprogressbar_animation_end(lua_State* L) {
    SongProgressbar songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);

    songprogressbar_animation_end(songprogressbar);

    return 0;
}




static const luaL_Reg SONGPROGRESSBAR_FUNCTIONS[] = {
    { "set_songplayer", script_songprogressbar_set_songplayer },
    { "set_duration", script_songprogressbar_set_duration },
    { "get_drawable", script_songprogressbar_get_drawable },
    { "set_visible", script_songprogressbar_set_visible },
    { "set_background_color", script_songprogressbar_set_background_color },
    { "set_bar_back_color", script_songprogressbar_set_bar_back_color },
    { "set_bar_progress_color", script_songprogressbar_set_bar_progress_color },
    { "set_text_color", script_songprogressbar_set_text_color },
    { "hide_time", script_songprogressbar_hide_time },
    { "show_elapsed", script_songprogressbar_show_elapsed },
    { "manual_update_enable", script_songprogressbar_manual_update_enable },
    { "manual_set_text", script_songprogressbar_manual_set_text },
    { "manual_set_position", script_songprogressbar_manual_set_position },
    { "animation_set", script_songprogressbar_animation_set },
    { "animation_restart", script_songprogressbar_animation_restart },
    { "animation_end", script_songprogressbar_animation_end },
    { NULL, NULL }
};

int script_songprogressbar_new(lua_State* L, SongProgressbar songprogressbar) {
    return luascript_userdata_new(L, SONGPROGRESSBAR, songprogressbar);
}

static int script_songprogressbar_gc(lua_State* L) {
    return luascript_userdata_gc(L, SONGPROGRESSBAR);
}

static int script_songprogressbar_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, SONGPROGRESSBAR);
}

void script_songprogressbar_register(lua_State* L) {
    luascript_register(L, SONGPROGRESSBAR, script_songprogressbar_gc, script_songprogressbar_tostring, SONGPROGRESSBAR_FUNCTIONS);
}

