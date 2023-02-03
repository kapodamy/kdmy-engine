#include "engine_string.h"
#include "luascript_internal.h"
#include <string.h>


static void* luascript_key_ptr = &luascript_get_instance;


static int get_lua_reference(Luascript luascript, void* obj) {
    const LuascriptObject* shared_array = luascript->shared_array;
    const size_t shared_size = luascript->shared_size;

    for (size_t i = 0; i < shared_size; i++) {
        if (shared_array[i].obj_ptr == obj) {
            return shared_array[i].lua_ref;
        }
    }

    return LUA_NOREF;
}

static LuascriptObject* add_lua_reference(Luascript luascript, void* obj, int ref, bool allocated) {
    size_t index;

    // find and empty slot
    const LuascriptObject* shared_array = luascript->shared_array;
    const size_t shared_size = luascript->shared_size;

    for (size_t i = 0; i < shared_size; i++) {
        if (shared_array[i].obj_ptr == NULL) {
            index = i;
            goto L_store_and_return;
        }
    }

    // no empty slots, grow the array and add it
    index = luascript->shared_size;
    luascript->shared_size += SHARED_ARRAY_CHUNK_SIZE;

    size_t new_size = luascript->shared_size * sizeof(struct LuascriptObject_t);
    luascript->shared_array = realloc(luascript->shared_array, new_size);
    assert(luascript->shared_array /* reallocation failed*/);

    for (size_t i = 0; i < SHARED_ARRAY_CHUNK_SIZE; i++) {
        LuascriptObject* entry = &luascript->shared_array[i + index];
        entry->lua_ref = LUA_NOREF;
        entry->obj_ptr = NULL;
        entry->was_allocated_by_lua = false;
    }

L_store_and_return:
    luascript->shared_array[index].obj_ptr = obj;
    luascript->shared_array[index].lua_ref = ref;
    luascript->shared_array[index].was_allocated_by_lua = allocated;

    return &luascript->shared_array[index];
}

static int remove_lua_reference(Luascript luascript, void* obj) {
    LuascriptObject* shared_array = luascript->shared_array;
    const size_t shared_size = luascript->shared_size;

    for (size_t i = 0; i < shared_size; i++) {
        if (shared_array[i].obj_ptr == obj) {
            int ref = shared_array[i].lua_ref;
            shared_array[i].obj_ptr = NULL;
            shared_array[i].lua_ref = LUA_REFNIL;
            shared_array[i].was_allocated_by_lua = false;
            return ref;
        }
    }

    return LUA_NOREF;
}

static LuascriptObject* read_luascript_object(lua_State* L, int ud, const char* tname) {
    LuascriptObject* udata = luaL_checkudata(L, ud, tname);

    if (!udata || !udata->obj_ptr || udata->lua_ref == LUA_REFNIL || udata->lua_ref == LUA_NOREF) {
        return NULL;
    }

    return udata;
}

static void* read_userdata(lua_State* L, int ud, const char* tname) {
    LuascriptObject* udata = read_luascript_object(L, ud, tname);

    if (!udata) {
        luaL_error(L, "%s object was destroyed.", tname);
        return NULL;
    }

    return udata->obj_ptr;
}

/*static int is_userdata_equals(lua_State* L) {
    bool equals;

    int type_a = lua_type(L, 1);
    int type_b = lua_type(L, 2);

    if (type_a != LUA_TUSERDATA || type_b != LUA_TUSERDATA) {
        equals = false;
    } else {
        void* a = lua_touserdata(L, 1);
        void* b = lua_touserdata(L, 2);

        equals = (a == NULL && b == NULL) || (a != b);
    }

    lua_pushboolean(L, equals);

    return 1;
}*/



int luascript_create_userdata(Luascript luascript, void* obj, const char* metatable_name, bool allocated) {
    lua_State* L = luascript->L;

    if (!obj) {
        lua_pushnil(L);
        return 1;
    }

    int ref = get_lua_reference(luascript, obj);
    if (ref != LUA_NOREF) {
        // recover userdata back into stack
        lua_pushinteger(L, ref);
        lua_gettable(L, LUA_REGISTRYINDEX);
    } else {
        LuascriptObject* udata = (LuascriptObject*)lua_newuserdata(L, sizeof(LuascriptObject));
        assert(udata /* can not create userdata*/);

        luaL_getmetatable(L, metatable_name);
        lua_setmetatable(L, -2);

        lua_pushvalue(L, -1);
        ref = luaL_ref(L, LUA_REGISTRYINDEX);

        udata->lua_ref = ref;
        udata->obj_ptr = obj;
        udata->was_allocated_by_lua = allocated;

        add_lua_reference(luascript, obj, ref, allocated);
    }

    return 1;
}

void luascript_remove_userdata(Luascript luascript, void* obj) {
    assert(obj);

    lua_State* L = luascript->L;

    // aquire lua reference and nullify the object
    int ref = remove_lua_reference(luascript, obj);
    if (ref == LUA_NOREF) return;

    // lua_pushinteger(L, ref);
    // lua_pushnil(L);
    // lua_settable(L, LUA_REGISTRYINDEX);
    // return;

    // remove from lua registry
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
}


void* luascript_read_userdata(lua_State* L, const char* check_metatable_name) {
    if (lua_isnil(L, 1)) {
        luaL_error(L, "%s was null (nil in lua).", check_metatable_name);
        return NULL;
    }

    return read_userdata(L, 1, check_metatable_name);
}

void* luascript_read_nullable_userdata(lua_State* L, int idx, const char* check_metatable_name) {
    if (lua_isnil(L, idx))
        return NULL;
    else
        return read_userdata(L, idx, check_metatable_name);
}


Luascript luascript_get_instance(lua_State* L) {
    lua_pushlightuserdata(L, luascript_key_ptr);
    lua_gettable(L, LUA_REGISTRYINDEX);

    Luascript luascript = lua_touserdata(L, -1);
    assert(luascript);
    lua_pop(L, 1);

    return luascript;
}

void luascript_set_instance(Luascript luascript) {
    lua_State* L = luascript->L;

    lua_pushlightuserdata(L, luascript_key_ptr);
    lua_pushlightuserdata(L, luascript);
    lua_settable(L, LUA_REGISTRYINDEX);
}


int luascript_userdata_new(lua_State* L, const char* check_metatable_name, void* obj) {
    return luascript_create_userdata(luascript_get_instance(L), obj, check_metatable_name, false);
}

int luascript_userdata_allocnew(lua_State* L, const char* check_metatable_name, void* obj) {
    return luascript_create_userdata(luascript_get_instance(L), obj, check_metatable_name, true);
}

int luascript_userdata_tostring(lua_State* L, const char* check_metatable_name) {
    void* udata = luascript_read_userdata(L, check_metatable_name);
    lua_pushfstring(L, "{%s %p}", check_metatable_name, udata);
    return 1;
}

int luascript_userdata_gc(lua_State* L, const char* check_metatable_name) {
    void* udata = luascript_read_userdata(L, check_metatable_name);
    luascript_remove_userdata(luascript_get_instance(L), udata);
    return 0;
}

int luascript_userdata_destroy(lua_State* L, const char* check_metatable_name, Destructor destructor) {
    LuascriptObject* udata = read_luascript_object(L, 1, check_metatable_name);

    if (!udata) return 0;

    void* obj_ptr = udata->obj_ptr;
    bool was_allocated_by_lua = udata->was_allocated_by_lua;

    luascript_remove_userdata(luascript_get_instance(L), obj_ptr);
    udata->obj_ptr = NULL;
    udata->lua_ref = LUA_NOREF;
    udata->was_allocated_by_lua = false;

    if (was_allocated_by_lua) {
        assert(destructor);
        destructor(&obj_ptr);
    }

    return 0;
}

bool luascript_userdata_is_allocated(lua_State* L, const char* check_metatable_name) {
    LuascriptObject* udata = read_luascript_object(L, 1, check_metatable_name);

    if (!udata) return false;

    return udata->was_allocated_by_lua;
}


void luascript_register(lua_State* lua, const char* name, const lua_CFunction gc, const lua_CFunction tostring, const luaL_Reg fns[]) {
    luaL_Reg OBJECT_METAMETHODS[] = {
        {"__gc", gc},
        {"__tostring", tostring},
        //{"__eq", is_userdata_equals},
        {NULL, NULL}};

    lua_newtable(lua);
    luaL_setfuncs(lua, fns, 0);
    lua_pushvalue(lua, -1);
    lua_setglobal(lua, name);

    luaL_newmetatable(lua, name);

    luaL_setfuncs(lua, OBJECT_METAMETHODS, 0);

    lua_pushliteral(lua, "__index");
    lua_pushvalue(lua, -3);
    lua_rawset(lua, -3);

    lua_pushliteral(lua, "__metatable");
    lua_pushvalue(lua, -3);
    lua_rawset(lua, -3);

    lua_pop(lua, 1);
}


Align luascript_parse_align(lua_State* L, const char* align) {
    Align value = luascript_parse_align2(align);
    if (value != ALIGN_INVALID) return value;

    return luaL_error(L, "invalid align: %s", align);
}

Align luascript_parse_align2(const char* align) {
    if (!align || align[0] == '\0' || string_equals(align, "NONE"))
        return ALIGN_NONE;
    if (string_equals(align, "START"))
        return ALIGN_START;
    else if (string_equals(align, "CENTER"))
        return ALIGN_CENTER;
    else if (string_equals(align, "END"))
        return ALIGN_END;
    else if (string_equals(align, "BOTH"))
        return ALIGN_BOTH;

    return ALIGN_INVALID;
}

int luascript_parse_pvrflag(lua_State* L, const char* pvrflag) {
    if (!pvrflag || pvrflag[0] == '\0' || string_equals(pvrflag, "DEFAULT"))
        return PVR_FLAG_DEFAULT;
    else if (string_equals(pvrflag, "ENABLE"))
        return PVR_FLAG_ENABLE;
    else if (string_equals(pvrflag, "DISABLE"))
        return PVR_FLAG_DISABLE;

    return luaL_error(L, "invalid pvrflag: %s", pvrflag);
}

int luascript_parse_forcecase(lua_State* L, const char* forcecase) {
    if (!forcecase || forcecase[0] == '\0' || string_equals(forcecase, "none"))
        return TEXTSPRITE_FORCECASE_NONE;
    else if (string_equals(forcecase, "upper"))
        return TEXTSPRITE_FORCECASE_UPPER;
    else if (string_equals(forcecase, "lower"))
        return TEXTSPRITE_FORCECASE_LOWER;

    return luaL_error(L, "invalid forcecase: %s", forcecase);
}

int luascript_parse_wordbreak(lua_State* L, const char* wordbreak) {
    if (!wordbreak || wordbreak[0] == '\0' || string_equals(wordbreak, "none"))
        return FONT_WORDBREAK_NONE;
    else if (string_equals(wordbreak, "loose"))
        return FONT_WORDBREAK_LOOSE;
    else if (string_equals(wordbreak, "break"))
        return FONT_WORDBREAK_BREAK;

    return luaL_error(L, "invalid wordbreak: %s", wordbreak);
}

int luascript_parse_interpolator(lua_State* L, const char* interpolator) {
    if (string_equals(interpolator, "ease"))
        return ANIM_MACRO_INTERPOLATOR_EASE;
    else if (string_equals(interpolator, "ease-in"))
        return ANIM_MACRO_INTERPOLATOR_EASE_IN;
    else if (string_equals(interpolator, "ease-out"))
        return ANIM_MACRO_INTERPOLATOR_EASE_OUT;
    else if (string_equals(interpolator, "ease-in-out"))
        return ANIM_MACRO_INTERPOLATOR_EASE_IN_OUT;
    else if (string_equals(interpolator, "linear"))
        return ANIM_MACRO_INTERPOLATOR_LINEAR;
    else if (string_equals(interpolator, "steps"))
        return ANIM_MACRO_INTERPOLATOR_STEPS;

    return luaL_error(L, "invalid interpolator: %s", interpolator);
}

Blend luascript_parse_blend(lua_State* L, const char* blend) {
    if (!blend || blend[0] == '\0' || string_equals("BLEND_DEFAULT", blend))
        return BLEND_DEFAULT;
    else if (string_equals("BLEND_ZERO", blend))
        return BLEND_ZERO;
    else if (string_equals("BLEND_ONE", blend))
        return BLEND_ONE;
    else if (string_equals("BLEND_SRC_COLOR", blend))
        return BLEND_SRC_COLOR;
    else if (string_equals("BLEND_ONE_MINUS_SRC_COLOR", blend))
        return BLEND_ONE_MINUS_SRC_COLOR;
    else if (string_equals("BLEND_DST_COLOR", blend))
        return BLEND_DST_COLOR;
    else if (string_equals("BLEND_ONE_MINUS_DST_COLOR", blend))
        return BLEND_ONE_MINUS_DST_COLOR;
    else if (string_equals("BLEND_SRC_ALPHA", blend))
        return BLEND_SRC_ALPHA;
    else if (string_equals("BLEND_ONE_MINUS_SRC_ALPHA", blend))
        return BLEND_ONE_MINUS_SRC_ALPHA;
    else if (string_equals("BLEND_DST_ALPHA", blend))
        return BLEND_DST_ALPHA;
    else if (string_equals("BLEND_ONE_MINUS_DST_ALPHA", blend))
        return BLEND_ONE_MINUS_DST_ALPHA;
    else if (string_equals("BLEND_CONSTANT_COLOR", blend))
        return BLEND_CONSTANT_COLOR;
    else if (string_equals("BLEND_ONE_MINUS_CONSTANT_COLOR", blend))
        return BLEND_ONE_MINUS_CONSTANT_COLOR;
    else if (string_equals("BLEND_CONSTANT_ALPHA", blend))
        return BLEND_CONSTANT_ALPHA;
    else if (string_equals("BLEND_ONE_MINUS_CONSTANT_ALPHA", blend))
        return BLEND_ONE_MINUS_CONSTANT_ALPHA;
    else if (string_equals("BLEND_SRC_ALPHA_SATURATE", blend))
        return BLEND_SRC_ALPHA_SATURATE;

    return luaL_error(L, "invalid blend: %s", blend);
}

StrumScriptTarget luascript_parse_strumscripttarget(lua_State* L, const char* strumscripttarget) {
    if (string_equals(strumscripttarget, "marker"))
        return STRUM_SCRIPT_TARGET_MARKER;
    if (string_equals(strumscripttarget, "sick_effect"))
        return STRUM_SCRIPT_TARGET_SICK_EFFECT;
    if (string_equals(strumscripttarget, "background"))
        return STRUM_SCRIPT_TARGET_BACKGROUND;
    if (string_equals(strumscripttarget, "strum_line"))
        return STRUM_SCRIPT_TARGET_STRUM_LINE;
    if (string_equals(strumscripttarget, "note"))
        return STRUM_SCRIPT_TARGET_NOTE;
    if (string_equals(strumscripttarget, "all"))
        return STRUM_SCRIPT_TARGET_ALL;

    return luaL_error(L, "invalid strumscripttarget: %s", strumscripttarget);
}

StrumScriptOn luascript_parse_strumscripton(lua_State* L, const char* strumscripton) {
    if (string_equals(strumscripton, "hit_down"))
        return STRUM_SCRIPT_ON_HIT_DOWN;
    if (string_equals(strumscripton, "hit_up"))
        return STRUM_SCRIPT_ON_HIT_UP;
    if (string_equals(strumscripton, "miss"))
        return STRUM_SCRIPT_ON_MISS;
    if (string_equals(strumscripton, "penality"))
        return STRUM_SCRIPT_ON_PENALITY;
    if (string_equals(strumscripton, "idle"))
        return STRUM_SCRIPT_ON_IDLE;
    if (string_equals(strumscripton, "all"))
        return STRUM_SCRIPT_ON_ALL;

    return luaL_error(L, "invalid strumscripton: %s", strumscripton);
}

ScrollDirection luascript_parse_scrolldirection(lua_State* L, const char* scrolldirection) {
    if (string_equals(scrolldirection, "UPSCROLL"))
        return STRUM_UPSCROLL;
    else if (string_equals(scrolldirection, "LEFTSCROLL"))
        return STRUM_LEFTSCROLL;
    else if (string_equals(scrolldirection, "DOWNSCROLL"))
        return STRUM_DOWNSCROLL;
    else if (string_equals(scrolldirection, "RIGHTSCROLL"))
        return STRUM_RIGHTSCROLL;

    return luaL_error(L, "invalid scrolldirection: %s", scrolldirection);
}


const char* luascript_stringify_align(Align align) {
    switch (align) {
        case ALIGN_START:
            return "START";
        case ALIGN_END:
            return "END";
        case ALIGN_CENTER:
            return "CENTER";
        case ALIGN_BOTH:
            return "BOTH";
        case ALIGN_NONE:
        default:
            return "NONE";
    }
}

const char* luascript_stringify_actiontype(CharacterActionType actiontype) {
    switch (actiontype) {
        case CHARACTER_ACTION_TYPE_MISS:
            return "miss";
        case CHARACTER_ACTION_TYPE_EXTRA:
            return "extra";
        case CHARACTER_ACTION_TYPE_IDLE:
            return "idle";
        case CHARACTER_ACTION_TYPE_SING:
            return "sing";
        default:
            return "none";
    }
}


static int luascript_handle_error(lua_State* L) {
    const char* msg = lua_tostring(L, -1);
    luaL_traceback(L, L, msg, 2);
    lua_remove(L, -2);

    // keep the error (message & traceback)  in the stack
    return 1;
}

int luascript_pcallk(lua_State* L, int arguments_count, int results_count) {
    // push error handler
    lua_pushcfunction(L, luascript_handle_error);
    lua_insert(L, 1);

    int result = lua_pcallk(L, arguments_count, results_count, 1, 0, NULL);

    // remove error handler
    lua_remove(L, 1);

    return result;
}

#ifdef JAVASCRIPT
//
// javascript imported functions are always static, warp them
//
EM_JS_PRFX(double, kdmyEngine_read_prop_double, (void* obj_id, const char* field_name), {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj) throw new Error("Uknown object id:" + obj_id);

    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];

    return ret;
});
EM_JS_PRFX(float, kdmyEngine_read_prop_float, (void* obj_id, const char* field_name), {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj) throw new Error("Uknown object id:" + obj_id);

    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];

    return ret;
});
EM_JS_PRFX(char*, kdmyEngine_read_prop_string, (void* obj_id, const char* field_name), {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj) throw new Error("Uknown object id:" + obj_id);

    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];

    return kdmyEngine_stringToPtr(ret);
});
EM_JS_PRFX(int32_t, kdmyEngine_read_prop_integer, (void* obj_id, const char* field_name), {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj) throw new Error("Uknown object id:" + obj_id);

    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];

    return ret;
});
EM_JS_PRFX(void*, kdmyEngine_read_prop_object, (void* obj_id, const char* field_name), {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj) throw new Error("Uknown object id:" + obj_id);

    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];

    return kdmyEngine_obtain((typeof ret === 'object') ? ret : null);
});
EM_JS_PRFX(bool, kdmyEngine_read_prop_boolean, (void* obj_id, const char* field_name), {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj) throw new Error("Uknown object id:" + obj_id);

    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];

    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, kdmyEngine_read_prop_floatboolean, (void* obj_id, const char* field_name), {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj) throw new Error("Uknown object id:" + obj_id);

    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];

    return ret >= 1.0 || ret === true;
});
EM_JS_PRFX(void, kdmyEngine_forget_obtained, (void* obj_id), {
    let ret = kdmyEngine_forget(obj_id);
    if (!ret) throw new Error("Uknown object id:" + obj_id);
});

EM_JS_PRFX(void*, kdmyEngine_create_object, (), {
    return kdmyEngine_obtain(new Object());
});
EM_JS_PRFX(void*, kdmyEngine_create_array, (size_t size), {
    return kdmyEngine_obtain(new Array(size));
});

EM_JS_PRFX(void*, kdmyEngine_read_window_object, (const char* variable_name), {
    let obj = window[kdmyEngine_ptrToString(variable_name)];
    return obj === undefined ? 0 : kdmyEngine_obtain(obj);
});

EM_JS_PRFX(void, kdmyEngine_write_prop_double, (void* obj_id, const char* field_name, double value), {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj) throw new Error("Uknown object id:" + obj_id);

    let field = kdmyEngine_ptrToString(field_name);
    obj[field] = value;
});
EM_JS_PRFX(void, kdmyEngine_write_prop_float, (void* obj_id, const char* field_name, float value), {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj) throw new Error("Uknown object id:" + obj_id);

    let field = kdmyEngine_ptrToString(field_name);
    obj[field] = value;
});
EM_JS_PRFX(void, kdmyEngine_write_prop_string, (void* obj_id, const char* field_name, char* value), {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj) throw new Error("Uknown object id:" + obj_id);

    let field = kdmyEngine_ptrToString(field_name);
    obj[field] = kdmyEngine_ptrToString(value);
});
EM_JS_PRFX(void, kdmyEngine_write_prop_integer, (void* obj_id, const char* field_name, int32_t value), {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj) throw new Error("Uknown object id:" + obj_id);

    let field = kdmyEngine_ptrToString(field_name);
    obj[field] = value;
});
EM_JS_PRFX(void, kdmyEngine_write_prop_object, (void* obj_id, const char* field_name, void* value), {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj) throw new Error("Uknown object id:" + obj_id);

    let field = kdmyEngine_ptrToString(field_name);
    obj[field] = kdmyEngine_obtain(value);
});
EM_JS_PRFX(void, kdmyEngine_write_prop_boolean, (void* obj_id, const char* field_name, bool value), {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj) throw new Error("Uknown object id:" + obj_id);

    let field = kdmyEngine_ptrToString(field_name);
    obj[field] = value;
});

EM_JS_PRFX(void, kdmyEngine_write_in_array_double, (void* array_id, int32_t index, double value), {
    let array = kdmyEngine_obtain(array_id);
    if (!array) throw new Error("Uknown array id:" + array_id);

    array[index] = value;
});
EM_JS_PRFX(void, kdmyEngine_write_in_array_float, (void* array_id, int32_t index, float value), {
    let array = kdmyEngine_obtain(array_id);
    if (!array) throw new Error("Uknown array id:" + array_id);

    array[index] = value;
});
EM_JS_PRFX(void, kdmyEngine_write_in_array_string, (void* array_id, int32_t index, char* value), {
    let array = kdmyEngine_obtain(array_id);
    if (!array) throw new Error("Uknown array id:" + array_id);

    array[index] = kdmyEngine_ptrToString(value);
});
EM_JS_PRFX(void, kdmyEngine_write_in_array_integer, (void* array_id, int32_t index, int32_t value), {
    let array = kdmyEngine_obtain(array_id);
    if (!array) throw new Error("Uknown array id:" + array_id);

    array[index] = value;
});
EM_JS_PRFX(void, kdmyEngine_write_in_array_object, (void* array_id, int32_t index, void* value), {
    let array = kdmyEngine_obtain(array_id);
    if (!array) throw new Error("Uknown array id:" + array_id);

    array[index] = kdmyEngine_obtain(value);
});
EM_JS_PRFX(void, kdmyEngine_write_in_array_boolean, (void* array_id, int32_t index, bool value), {
    let array = kdmyEngine_obtain(array_id);
    if (!array) throw new Error("Uknown array id:" + array_id);

    array[index] = value;
});



double kdmy_read_prop_double(void* obj_id, const char* field_name) {
    return kdmyEngine_read_prop_double(obj_id, field_name);
}
float kdmy_read_prop_float(void* obj_id, const char* field_name) {
    return kdmyEngine_read_prop_float(obj_id, field_name);
}
char* kdmy_read_prop_string(void* obj_id, const char* field_name) {
    return kdmyEngine_read_prop_string(obj_id, field_name);
}
int32_t kdmy_read_prop_integer(void* obj_id, const char* field_name) {
    return kdmyEngine_read_prop_integer(obj_id, field_name);
}
void* kdmy_read_prop_object(void* obj_id, const char* field_name) {
    return kdmyEngine_read_prop_object(obj_id, field_name);
}
bool kdmy_read_prop_boolean(void* obj_id, const char* field_name) {
    return kdmyEngine_read_prop_boolean(obj_id, field_name);
}
bool kdmy_read_prop_floatboolean(void* obj_id, const char* field_name) {
    return kdmyEngine_read_prop_floatboolean(obj_id, field_name);
}
void kdmy_forget_obtained(void* obj_id) {
    return kdmyEngine_forget_obtained(obj_id);
}

void* kdmy_create_object() {
    return kdmyEngine_create_object();
}
void* kdmy_create_array(int32_t size) {
    return kdmyEngine_create_array(size);
}

void* kdmy_read_window_object(const char* variable_name) {
    return kdmyEngine_read_window_object(variable_name);
}

void kdmy_write_prop_double(void* obj_id, const char* field_name, double value) {
    return kdmyEngine_write_prop_double(obj_id, field_name, value);
}
void kdmy_write_prop_float(void* obj_id, const char* field_name, float value) {
    return kdmyEngine_write_prop_float(obj_id, field_name, value);
}
void kdmy_write_prop_string(void* obj_id, const char* field_name, char* value) {
    return kdmyEngine_write_prop_string(obj_id, field_name, value);
}
void kdmy_write_prop_integer(void* obj_id, const char* field_name, int32_t value) {
    return kdmyEngine_write_prop_integer(obj_id, field_name, value);
}
void kdmy_write_prop_object(void* obj_id, const char* field_name, void* value) {
    return kdmyEngine_write_prop_object(obj_id, field_name, value);
}
void kdmy_write_prop_boolean(void* obj_id, const char* field_name, bool value) {
    return kdmyEngine_write_prop_boolean(obj_id, field_name, value);
}

void kdmy_write_in_array_double(void* array_id, int32_t index, double value) {
    kdmyEngine_write_in_array_double(array_id, index, value);
}
void kdmy_write_in_array_float(void* array_id, int32_t index, float value) {
    kdmyEngine_write_in_array_float(array_id, index, value);
}
void kdmy_write_in_array_string(void* array_id, int32_t index, char* value) {
    kdmyEngine_write_in_array_string(array_id, index, value);
}
void kdmy_write_in_array_integer(void* array_id, int32_t index, int32_t value) {
    kdmyEngine_write_in_array_integer(array_id, index, value);
}
void kdmy_write_in_array_object(void* array_id, int32_t index, void* value) {
    kdmyEngine_write_in_array_object(array_id, index, value);
}
void kdmy_write_in_array_boolean(void* array_id, int32_t index, bool value) {
    kdmyEngine_write_in_array_boolean(array_id, index, value);
}

#endif
