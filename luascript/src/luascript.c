#include "engine_string.h"
#include "engine_version.h"
#include "gamepad.h"
#include "luascript_internal.h"

typedef struct {
    const char* variable;
    const char* value;
} luaL_Reg_String;

typedef struct {
    const char* variable;
    uint32_t value;
} luaL_Reg_Integer;


static Linkedlist luascript_instances;

static void __attribute__((constructor)) ctor() {
    luascript_instances = linkedlist_init();
}



static void luascript_register_objects(lua_State* L, bool is_week) {

    const luaL_Reg_String GLOBALS_STRINGS[] = {
        {"ENGINE_NAME", ENGINE_NAME},
        {"ENGINE_VERSION", ENGINE_VERSION},
        {NULL, NULL}
    };
    const luaL_Reg_Integer GLOBALS_INTEGERS[] = {
        {"GAMEPAD_A", GAMEPAD_A},
        {"GAMEPAD_B", GAMEPAD_B},
        {"GAMEPAD_X", GAMEPAD_X},
        {"GAMEPAD_Y", GAMEPAD_Y},

        {"GAMEPAD_DPAD_UP", GAMEPAD_DPAD_UP},
        {"GAMEPAD_DPAD_DOWN", GAMEPAD_DPAD_DOWN},
        {"GAMEPAD_DPAD_RIGHT", GAMEPAD_DPAD_RIGHT},
        {"GAMEPAD_DPAD_LEFT", GAMEPAD_DPAD_LEFT},

        {"GAMEPAD_START", GAMEPAD_START},
        {"GAMEPAD_SELECT", GAMEPAD_SELECT},

        {"GAMEPAD_TRIGGER_LEFT", GAMEPAD_TRIGGER_LEFT},
        {"GAMEPAD_TRIGGER_RIGHT", GAMEPAD_TRIGGER_RIGHT},
        {"GAMEPAD_BUMPER_LEFT", GAMEPAD_BUMPER_LEFT},
        {"GAMEPAD_BUMPER_RIGHT", GAMEPAD_BUMPER_RIGHT},

        {"GAMEPAD_APAD_UP", GAMEPAD_APAD_UP},
        {"GAMEPAD_APAD_DOWN", GAMEPAD_APAD_DOWN},
        {"GAMEPAD_APAD_RIGHT", GAMEPAD_APAD_RIGHT},
        {"GAMEPAD_APAD_LEFT", GAMEPAD_APAD_LEFT},

        {"GAMEPAD_DPAD2_UP", GAMEPAD_DPAD2_UP},
        {"GAMEPAD_DPAD2_DOWN", GAMEPAD_DPAD2_DOWN},
        {"GAMEPAD_DPAD2_RIGHT", GAMEPAD_DPAD2_RIGHT},
        {"GAMEPAD_DPAD2_LEFT", GAMEPAD_DPAD2_LEFT},

        {"GAMEPAD_DPAD3_UP", GAMEPAD_DPAD3_UP},
        {"GAMEPAD_DPAD3_DOWN", GAMEPAD_DPAD3_DOWN},
        {"GAMEPAD_DPAD3_RIGHT", GAMEPAD_DPAD3_RIGHT},
        {"GAMEPAD_DPAD3_LEFT", GAMEPAD_DPAD3_LEFT},

        {"GAMEPAD_DPAD4_UP", GAMEPAD_DPAD4_UP},
        {"GAMEPAD_DPAD4_DOWN", GAMEPAD_DPAD4_DOWN},
        {"GAMEPAD_DPAD4_RIGHT", GAMEPAD_DPAD4_RIGHT},
        {"GAMEPAD_DPAD4_LEFT", GAMEPAD_DPAD4_LEFT},

        {"GAMEPAD_BACK", GAMEPAD_BACK},
        {NULL, 0}
    };

    for (size_t i = 0;; i++) {
        if (GLOBALS_STRINGS[i].variable == NULL) break;
        lua_pushstring(L, GLOBALS_STRINGS[i].value);
        lua_setglobal(L, GLOBALS_STRINGS[i].variable);
    }

    for (size_t i = 0;; i++) {
        if (GLOBALS_INTEGERS[i].variable == NULL) break;
        lua_pushinteger(L, (lua_Integer)GLOBALS_INTEGERS[i].value);
        lua_setglobal(L, GLOBALS_INTEGERS[i].variable);
    }

    // register all objects (metatables) and functions
    script_camera_register(L);
    script_character_register(L);
    script_layout_register(L);
    script_layoutplaceholder_register(L);
    script_messagebox_register(L);
    script_modifier_register(L);
    script_soundplayer_register(L);
    script_sprite_register(L);
    script_textsprite_register(L);
    script_psshader_register(L);
    script_tweenlerp_register(L);
    script_atlas_register(L);
    script_animlist_register(L);
    script_animlistitem_register(L);
    script_animsprite_register(L);
    script_modelholder_register(L);

    if (is_week) {
        script_songplayer_register(L);
        script_dialogue_register(L);
        script_week_register(L);
    } else {
        script_modding_register(L);
    }

    script_math2d_register(L);
    script_timer_register(L);
    script_fs_register(L);
}

static void luascript_register_sandbox(lua_State* L) {
    int result = luaL_dostring(L, "debug.debug = nil\n"
                                  "debug.getfenv = getfenv\n"
                                  "debug.getregistry = nil\n"
                                  "debug = nil\n"
                                  "dofile = nil\n"
                                  "io = nil\n"
                                  "load = nil\n"
                                  "loadfile = nil\n"
                                  "dofile = nil\n"
                                  "os.execute = nil\n"
                                  "os.getenv = nil\n"
                                  "os.remove = nil\n"
                                  "os.tmpname = nil\n"
                                  "os.setlocale = nil\n"
                                  "os.rename = nil\n"
                                  //"os.exit = nil\n"
                                  //"loadstring = nil\n"
                                  "package.loaded.io = nil\n"
                                  "package.loaded.package = nil\n"
                                  "package.cpath = nil\n"
                                  "package.loaded = nil\n"
                                  "package.loaders= nil\n"
                                  "package.loadlib= nil\n"
                                  "package.path= nil\n"
                                  "package.preload= nil\n"
                                  "package.seeall= nil\n"
                                  "package.searchpath= nil\n"
                                  "package.searchers= nil\n"
                                  "package = nil\n"
                                  "require = nil\n"
                                  "newproxy = nil\n"
    );
    assert(result == 0 /*luascript_register_sandbox() failed*/);
}


Luascript luascript_init(const char* lua_sourcecode, const char* filename, void* context, bool is_week) {
    lua_State* L = luaL_newstate();

    if (L == NULL) {
        fprintf(stderr, "luascript_init() cannot create lua state, not enough memory\n");
        return NULL;
    }

    luaL_openlibs(L);

    Luascript luascript = malloc(sizeof(struct Luascript_t));
    luascript->L = L;
    luascript->shared_size = SHARED_ARRAY_CHUNK_SIZE;
    luascript->shared_array = malloc(sizeof(struct LuascriptObject_t) * SHARED_ARRAY_CHUNK_SIZE);
    luascript->context = context;

    luascript_set_instance(luascript);
    luascript_register_objects(L, is_week);
    luascript_register_sandbox(L);

    // fake the filename
    int filename_index = lua_gettop(L) + 1;
    lua_pushfstring(L, "@%s", filename);

    // parse the lua sourcode and give a false filename
    int status = luaL_loadbufferx(L, lua_sourcecode, strlen(lua_sourcecode), lua_tostring(L, -1), NULL);
    lua_remove(L, filename_index);

    if (!status) status = lua_pcall(L, 0, LUA_MULTRET, 0);

    if (status != LUA_OK) {
        const char* error_message = lua_tostring(L, -1);
        fprintf(stderr, "luascript_init() luaL_loadfile() failed: %s\n", error_message);

        luascript_destroy(&luascript);
        return NULL;
    }

    linkedlist_add_item(luascript_instances, luascript);

    return luascript;
}

void luascript_destroy(Luascript* luascript) {
    if (*luascript == NULL) return;

    Luascript obj = *luascript;

    linkedlist_remove_item(luascript_instances, obj);
    lua_close(obj->L);
    free(obj->shared_array);

    free(obj);
    *luascript = NULL;
}

int luascript_eval(Luascript luascript, const char* eval_string) {
    if (!eval_string || !eval_string[0]) return LUA_OK;
    return luaL_dostring(luascript->L, eval_string) == LUA_OK;
}

void luascript_drop_shared(void* obj_ptr) {
    ITERATE_LINKEDLIST(luascript_instances, Luascript, tmp, luascript, {
        luascript_remove_userdata(luascript, obj_ptr);
    });
}


#if JAVASCRIPT
void luascript_destroy_JS(Luascript luascript) {
    Luascript temp = luascript;
    luascript_destroy(&temp);
}
void luascript_set_engine_globals_JS(const char* name, const char* version) {
    ENGINE_NAME = name;
    ENGINE_VERSION = version;
}
#endif

////
//// Unimplemented and/or exported functions for lua
////
/*

// unimplemented
modelholder_init(src);
modelholder_is_invalid(modelholder);
modelholder_create_animsprite(modelholder, animation_name, fallback_static, no_return_null);// internal alloc
modelholder_get_atlas_entry(modelholder, atlas_entry_name);// managed
animlist_init(src);
animlist_create_animsprite(animlist, animation_name);// managed
atlas_init(src);
atlas_get_entry(atlas, entry_name);
atlas_apply(atlas, sprite, name, override_draw_size);


math2d_random(min, max);
math2d_random_int(min, max);
math2d_lerp(start, end, step);
math2d_inverselerp(start, end, value);
math2d_nearestdown(value, step);
math2d_cubicbezier(offset, point0, point1, point2, point3);
math2d_points_distance(x1, y1, x2, y2);


healthbar_state_opponent_add(healthbar, icon_mdlhldr, bar_mdlhldr, state_name);
healthbar_state_opponent_add2(healthbar, icon_mdlhldr, bar_color_rbb8, state_name);
healthbar_state_player_add(healthbar, icon_mdlhldr, bar_mdlhldr, state_name);
healthbar_state_player_add2(healthbar, icon_modelholder, bar_color_rgb8, state_name);
healthbar_state_background_add(healthbar, modelholder, state_name);
healthbar_state_background_add2(healthbar, color_rgb8, animsprite, state_name);
healthbar_set_opponent_bar_color(healthbar, color_rgb8);
healthbar_set_player_bar_color(healthbar, color_rgb8);
healthbar_state_toggle(healthbar, state_name);
healthbar_state_toggle_background(healthbar, state_name);
healthbar_state_toggle_player(healthbar, state_name);
healthbar_state_toggle_opponent(healthbar, state_name);

healthbar_set_bump_animation(healthbar, animlist);
healthbar_set_bump_animation_opponent(healthbar, animsprite);
healthbar_set_bump_animation_player(healthbar, animsprite);
healthbar_bump_enable(healthbar, enable_bump);
healthbar_set_visible(healthbar, visible);
healthbar_set_alpha(healthbar, alpha);
healthbar_get_modifier(healthbar);
healthbar_animation_set(healthbar, animsprite);
healthbar_disable_progress_animation(healthbar, disable);
healthbar_disable_icon_overlap(healthbar, disable);
healthbar_hide_warnings(healthbar);
healthbar_show_drain_warning(healthbar, use_fast_drain);
healthbar_show_locked_warning(healthbar);


notepool_change_alpha_alone(notepool, alpha);
notepool_change_alpha_sustain(notepool, alpha);
notepool_change_alpha(notepool, alpha);


playerstats_add_hit(playerstats, multiplier, base_note_duration, hit_time_difference);
playerstats_add_sustain(playerstats, quarters, is_released);
playerstats_add_penality(playerstats, on_empty_strum);
playerstats_add_miss(playerstats, multiplier);
playerstats_add_extra_health(playerstats, multiplier);
playerstats_enable_health_recover(playerstats, enable);
playerstats_get_health(playerstats);
playerstats_get_last_ranking(playerstats);
playerstats_get_last_difference(playerstats);
playerstats_get_combo_streak(playerstats);
playerstats_get_combo_breaks(playerstats);
playerstats_get_score(playerstats);
playerstats_get_hits(playerstats);
playerstats_get_misses(playerstats);
playerstats_get_penalties(playerstats);
playerstats_set_health(playerstats, health);
playerstats_add_health(playerstats, health, die_if_negative);
playerstats_raise(playerstats, with_full_health);


rankingcounter_add_state(rankingcounter, modelholder, state_name);
rankingcounter_toggle_state(rankingcounter, state_name);
rankingcounter_reset(rankingcounter);
rankingcounter_hide_accuracy(rankingcounter, hide);
rankingcounter_use_percent_instead(rankingcounter, use_accuracy_percenter);
rankingcounter_set_default_ranking_animation2(rankingcounter, animsprite);
rankingcounter_set_default_ranking_text_animation2(rankingcounter, animsprite);
rankingcounter_set_alpha(rankingcounter, alpha);
rankingcounter_set_offsetcolor(rankingcounter, r, g, b, a);
rankingcounter_set_offsetcolor_to_default(rankingcounter);
rankingcounter_get_modifier(rankingcounter);


roundstats_hide(roundstats, hide);
roundstats_reset(roundstats);
roundstats_get_drawable(roundstats);
roundstats_tweenlerp_set_on_beat(roundstats, tweenlerp, rollback_beats, beat_duration);
roundstats_tweenlerp_set_on_hit(roundstats, tweenlerp, rollback_beats, beat_duration);
roundstats_tweenlerp_set_on_miss(roundstats, tweenlerp, rollback_beats, beat_duration);


streakcounter_hide_combo_sprite(streakcounter, hide);
streakcounter_set_combo_draw_location(streakcounter, x, y);
streakcounter_state_add(streakcounter, combo_modelholder, number_modelholder, state_name);
streakcounter_state_toggle(streakcounter, state_name);
streakcounter_set_alpha(streakcounter, alpha);
streakcounter_set_offsetcolor(streakcounter, r, g, b, a);
streakcounter_set_offsetcolor_to_default(streakcounter);
streakcounter_get_modifier(streakcounter);


strum_set_scroll_speed(strum, speed);
strum_set_scroll_direction(strum, direction);
strum_reset(strum, scroll_speed, state_name);
strum_force_key_release(strum);
strum_get_press_state(strum);
strum_get_name(strum);
strum_get_marker_duration(strum);
strum_set_marker_duration(strum, duration);
strum_enable_background(strum, enable);
strum_enable_sick_effect(strum, enable);
strum_state_add(strum, mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name);
strum_state_toggle(strum, state_name);
strum_state_toggle_notes(strum, state_name);
strum_state_toggle_sick_effect(strum, state_name);
strum_state_toggle_marker(strum, state_name);
strum_state_toggle_background(strum, state_name);
strum_set_alpha_background(strum, alpha);
strum_set_alpha_sick_effect(strum, alpha);
strum_set_keep_aspect_ratio_background(strum, enable);
strum_set_extra_animation(strum, strum_script_target, strum_script_on, undo, animsprite);// managed
strum_set_extra_animation_continuous(strum, strum_script_target, animsprite);// managed
strum_set_notesmaker_tweenlerp(strum, tweenlerp, apply_to_marker_too);// not implemented
strum_set_sickeffect_size_ratio(strum, size_ratio);
strum_set_offsetcolor(strum, r, g, b, a);
strum_set_alpha(strum, alpha);
strum_set_visible(strum, visible);
strum_set_draw_offset(strum, offset_milliseconds);
strum_get_modifier(strum);
strum_animation_restart(strum);
strum_animation_end(strum);



conductor_poll_reset(conductor);
conductor_set_character(conductor, character);
conductor_use_strum_line(conductor, strum);
conductor_use_strums(conductor, strums);
conductor_disable_strum_line(conductor, strum, should_disable);
conductor_remove_strum(conductor, strum);
conductor_clear_mapping(conductor);
conductor_map_strum_to_player_sing_add(conductor, strum, sing_direction_name);
conductor_map_strum_to_player_extra_add(conductor, strum, extra_animation_name);
conductor_map_strum_to_player_sing_remove(conductor, strum_name, sing_direction_name);
conductor_map_strum_to_player_extra_remove(conductor, strum_name, extra_animation_name);
conductor_map_automatically(conductor, should_map_extras);
conductor_disable(conductor, disable);
conductor_play_idle(conductor);
conductor_play_hey(conductor);
conductor_get_character(conductor);


countdown_set_bpm(countdown, bpm);
countdown_ready(countdown);
countdown_start(countdown);
countdown_has_ended(countdown);


imgutils_calc_rectangle(x, y, max_width, max_height, src_width, src_height, align_hrzntl, align_vrtcl);// managed + multiple result
imgutils_calc_centered_location(x, y, width, height, ref_width, ref_height);



drawable_set_antialiasing(drawable, antialiasing);
statesprite_set_antialiasing(drawable, antialiasing);

// engine_feature******
healthbar_enable_extra_length(healthbar, extra_enabled);
healthbar_disable_warnings(healthbar, disable);
playerstats_enable_penality_on_empty_strum(playerstats, enable);

*/
