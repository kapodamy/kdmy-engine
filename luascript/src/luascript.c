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


        {"MouseButton_BUTTON1", 0},
        {"MouseButton_BUTTON2", 1},
        {"MouseButton_BUTTON3", 2},
        {"MouseButton_BUTTON4", 3},
        {"MouseButton_BUTTON5", 4},
        {"MouseButton_BUTTON6", 5},
        {"MouseButton_BUTTON7", 6},
        {"MouseButton_BUTTON8", 7},

        {"ModKeys_Shift", 0x0001},
        {"ModKeys_Control", 0x0002},
        {"ModKeys_Alt", 0x0004},
        {"ModKeys_Super", 0x0008},
        {"ModKeys_CapsLock", 0x0010},
        {"ModKeys_NumLock", 0x0020},

        {"Keys_Unknown", -1},
        {"Keys_Space", 32},
        {"Keys_Apostrophe", 39},
        {"Keys_Comma", 44},
        {"Keys_Minus", 45},
        {"Keys_Period", 46},
        {"Keys_Slash", 47},
        {"Keys_Numpad0", 48},
        {"Keys_Numpad1", 49},
        {"Keys_Numpad2", 50},
        {"Keys_Numpad3", 51},
        {"Keys_Numpad4", 52},
        {"Keys_Numpad5", 53},
        {"Keys_Numpad6", 54},
        {"Keys_Numpad7", 55},
        {"Keys_Numpad8", 56},
        {"Keys_Numpad9", 57},
        {"Keys_Semicolon", 59},
        {"Keys_Equal", 61},
        {"Keys_A", 65},
        {"Keys_B", 66},
        {"Keys_C", 67},
        {"Keys_D", 68},
        {"Keys_E", 69},
        {"Keys_F", 70},
        {"Keys_G", 71},
        {"Keys_H", 72},
        {"Keys_I", 73},
        {"Keys_J", 74},
        {"Keys_K", 75},
        {"Keys_L", 76},
        {"Keys_M", 77},
        {"Keys_N", 78},
        {"Keys_O", 79},
        {"Keys_P", 80},
        {"Keys_Q", 81},
        {"Keys_R", 82},
        {"Keys_S", 83},
        {"Keys_T", 84},
        {"Keys_U", 85},
        {"Keys_V", 86},
        {"Keys_W", 87},
        {"Keys_X", 88},
        {"Keys_Y", 89},
        {"Keys_Z", 90},
        {"Keys_LeftBracket", 91},
        {"Keys_Backslash", 92},
        {"Keys_RightBracket", 93},
        {"Keys_GraveAccent", 96},
        {"Keys_World_1", 161},
        {"Keys_World_2", 162},
        {"Keys_Escape", 256},
        {"Keys_Enter", 257},
        {"Keys_Tab", 258},
        {"Keys_Backspace", 259},
        {"Keys_Insert", 260},
        {"Keys_Delete", 261},
        {"Keys_Right", 262},
        {"Keys_Left", 263},
        {"Keys_Down", 264},
        {"Keys_Up", 265},
        {"Keys_PageUp", 266},
        {"Keys_PageDown", 267},
        {"Keys_Home", 268},
        {"Keys_End", 269},
        {"Keys_CapsLock", 280},
        {"Keys_ScrollLock", 281},
        {"Keys_NumLock", 282},
        {"Keys_PrintScreen", 283},
        {"Keys_Pause", 284},
        {"Keys_F1", 290},
        {"Keys_F2", 291},
        {"Keys_F3", 292},
        {"Keys_F4", 293},
        {"Keys_F5", 294},
        {"Keys_F6", 295},
        {"Keys_F7", 296},
        {"Keys_F8", 297},
        {"Keys_F9", 298},
        {"Keys_F10", 299},
        {"Keys_F11", 300},
        {"Keys_F12", 301},
        {"Keys_F13", 302},
        {"Keys_F14", 303},
        {"Keys_F15", 304},
        {"Keys_F16", 305},
        {"Keys_F17", 306},
        {"Keys_F18", 307},
        {"Keys_F19", 308},
        {"Keys_F20", 309},
        {"Keys_F21", 310},
        {"Keys_F22", 311},
        {"Keys_F23", 312},
        {"Keys_F24", 313},
        {"Keys_F25", 314},
        {"Keys_KP_0", 320},
        {"Keys_KP_1", 321},
        {"Keys_KP_2", 322},
        {"Keys_KP_3", 323},
        {"Keys_KP_4", 324},
        {"Keys_KP_5", 325},
        {"Keys_KP_6", 326},
        {"Keys_KP_7", 327},
        {"Keys_KP_8", 328},
        {"Keys_KP_9", 329},
        {"Keys_KP_Decimal", 330},
        {"Keys_KP_Divide", 331},
        {"Keys_KP_Multiply", 332},
        {"Keys_KP_Subtract", 333},
        {"Keys_KP_Add", 334},
        {"Keys_KP_Enter", 335},
        {"Keys_KP_Equal", 336},
        {"Keys_LeftShift", 340},
        {"Keys_LeftControl", 341},
        {"Keys_LeftAlt", 342},
        {"Keys_LeftSuper", 343},
        {"Keys_RightShift", 344},
        {"Keys_RightControl", 345},
        {"Keys_RightAlt", 346},
        {"Keys_RightSuper", 347},
        {"Keys_Menu", 348},

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
    script_conductor_register(L);
    script_layout_register(L);
    script_layoutplaceholder_register(L);
    script_messagebox_register(L);
    script_modifier_register(L);
    script_soundplayer_register(L);
    script_videoplayer_register(L);
    script_sprite_register(L);
    script_textsprite_register(L);
    script_psshader_register(L);
    script_tweenlerp_register(L);
    script_tweenkeyframe_register(L);
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
        script_menumanifest_register(L);
        script_menu_register(L);
        script_modding_register(L);
    }

    script_math2d_register(L);
    script_timer_register(L);
    script_fs_register(L);
    script_script_register(L);
    script_enginesettings_register(L);
    script_environment_register(L);

    // set engine settings
#ifdef JAVASCRIPT
    script_enginesettings_new(L, kdmy_read_window_object("SETTINGS"));
    lua_setglobal(L, "Settings");
#else
    script_enginesettings_new(L, &SETTINGS);
    lua_setglobal(L, "Settings");
#endif
}

static void luascript_register_sandbox(lua_State* L) {
    int result = luaL_dostring(L, "debug.debug = nil\n"
                                  "debug.getfenv = getfenv\n"
                                  "debug.getregistry = nil\n"
                                  "debug = nil\n"
                                  "dofile = nil\n"
                                  "io = nil\n"
                                  //"load = nil\n"
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
                                  //"require = nil\n"
                                  "newproxy = nil\n"
    );
    assert(result == 0 /*luascript_register_sandbox() failed*/);
}

static void luascript_print_warning(void *ud, const char *msg, int tocont) {
    (void)ud;
    (void)tocont;

    if (tocont)
        printf("%s", msg);
    else
        printf("%s\n", msg);
}


Luascript luascript_init(const char* lua_sourcecode, const char* filename, void* context, bool is_week) {
    lua_State* L = luaL_newstate();

    if (L == NULL) {
        fprintf(stderr, "luascript_init() cannot create lua state, not enough memory\n");
        return NULL;
    }

    luaL_openlibs(L);
    
    // allow lua scripts to emit warnings
    lua_setwarnf(L, luascript_print_warning, NULL);

    Luascript luascript = malloc(sizeof(struct Luascript_t));
    luascript->L = L;
    luascript->shared_size = SHARED_ARRAY_CHUNK_SIZE;
    luascript->shared_array = malloc(sizeof(struct LuascriptObject_t) * SHARED_ARRAY_CHUNK_SIZE);
    luascript->context = context;
    luascript->is_week = is_week;

    luascript_set_instance(luascript);
    luascript_register_objects(L, is_week);
    luascript_register_sandbox(L);

    // fake the filename
    int filename_index = lua_gettop(L) + 1;
    lua_pushfstring(L, "@%s", filename);

    // parse the lua sourcode and give a false filename
    int status = luaL_loadbufferx(L, lua_sourcecode, strlen(lua_sourcecode), lua_tostring(L, -1), NULL);
    lua_remove(L, filename_index);

    if (!status) status = luascript_pcallk(L, 0, LUA_MULTRET);

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

bool luascript_eval(Luascript luascript, const char* eval_string) {
    if (!eval_string || !eval_string[0]) return LUA_OK;

    int result = luaL_loadstring(luascript->L, eval_string);
    
#ifdef JAVASCRIPT
    free((char*)eval_string);
#endif

    if (result != LUA_OK) return false;

    int ret = luascript_pcallk(luascript->L, 0, LUA_MULTRET);

    return ret == LUA_OK;
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


