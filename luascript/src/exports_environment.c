#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(char*, kdmyEngine_get_language, (), {
    const code = window.navigator.language;
    const lang = new Intl.DisplayNames([code], {type: 'language'});
    const name = lang.of(code);
    return kdmyEngine_stringToPtr(name);
});
EM_JS_PRFX(char*, kdmyEngine_get_useragent, (), {
    return kdmyEngine_stringToPtr(navigator.userAgent);
});
EM_JS_PRFX(char*, kdmyEngine_get_locationquery, (), {
    let query = location.search;
    if (query.length > 0 && query[0] == '?') query = query.substring(1);

    let name = location.pathname;
    let idx = name.lastIndexOf('/');
    if (idx >= 0) name = name.substring(idx + 1);

    let str = name + " ";
    for (let part of query.split("&")) {
        let idx = part.indexOf("=");
            
        if (idx < 0) {
            str += decodeURIComponent(part) + " ";
            continue;
        }

        let key = part.substring(0, idx);
        let value = part.substring(idx + 1);
        
        if (value.includes(" ") && value[0] != '"' && value[value.length-1] != '"') {
            value = '"' + value + '"';
        }

        str += "-" + decodeURIComponent(key) + " " + value + " ";
    }

    return kdmyEngine_stringToPtr(str);
});
EM_JS_PRFX(void, kdmyEngine_require_window_attention, (), {
    alert(document.title + "\n Environment:require_window_attention()");
});
EM_JS_PRFX(void, kdmyEngine_change_window_title, (const char* title, bool from_modding_context), {
    luascriptplatform.ChangeWindowTitle(kdmyEngine_ptrToString(title), from_modding_context);
});
EM_JS_PRFX(void, kdmyEngine_open_link, (const char* url), {
    let target_url = kdmyEngine_ptrToString(url);
    if (!target_url) return;
    window.open(target_url, "_blank", "noopener,noreferrer");
});
#else
static const char* language = "English";
static const char* username = "Cosme Fulanito";
static const char* cmdargs = "";
#endif


static int script_environment_get_language(lua_State* L) {

#ifdef JAVASCRIPT
    char* language = kdmyEngine_get_language();
    lua_pushstring(L, language);
    free(language);
#else
    lua_pushstring(L, language);
#endif

    return 1;
}

static int script_environment_get_username(lua_State* L) {

#ifdef JAVASCRIPT
    char* useragent = kdmyEngine_get_useragent();
    lua_pushstring(L, useragent);
    free(useragent);
#else
    lua_pushstring(L, username);
#endif

    return 1;
}

static int script_environment_get_cmdargs(lua_State* L) {

#ifdef JAVASCRIPT
    char* locationquery = kdmyEngine_get_locationquery();
    lua_pushstring(L, locationquery);
    free(locationquery);
#else
    lua_pushstring(L, cmdargs);
#endif

    return 1;
}

static int script_environment_exit(lua_State* L) {
    int exit_code = (int)luaL_checkinteger(L, 2);
    exit(exit_code);
    return 0;
}

static int script_environment_change_window_title(lua_State* L) {

#ifdef JAVASCRIPT
    Luascript luascript = luascript_get_instance(L);
    const char* title = luaL_optstring(L, 2, NULL);
    kdmyEngine_change_window_title(title, !luascript->is_week);
#else
    (void)L;
#endif

    return 0;
}

static int script_environment_require_window_attention(lua_State* L) {
    (void)L;

#ifdef JAVASCRIPT
    kdmyEngine_require_window_attention();
#endif

    return 0;
}

static int script_environment_open_www_link(lua_State* L) {
    const char* url = luaL_checkstring(L, 2);

#ifdef JAVASCRIPT
    kdmyEngine_open_link(url);
#endif

    return 0;
}




static const luaL_Reg ENVIRONMENT_FUNCTIONS[] = {
    { "get_language", script_environment_get_language },
    { "get_username", script_environment_get_username },
    { "get_cmdargs", script_environment_get_cmdargs },
    { "exit", script_environment_exit },
    { "change_window_title", script_environment_change_window_title },
    { "require_window_attention", script_environment_require_window_attention },
    { "open_www_link", script_environment_open_www_link },
    { NULL, NULL }
};

static int script_environment_gc(lua_State* L) {
    // nothing to do
    (void)L;
    return 0;
}

static int script_environment_tostring(lua_State* L) {
#ifdef JAVASCRIPT
    char* language = kdmyEngine_get_language();
    char* username = kdmyEngine_get_useragent();
    char* cmdargs = kdmyEngine_get_locationquery();
#endif

    lua_pushfstring(
        L,
        "{language=\"%s\" username=\"%s\" cmdargs=\"%s\"}",
        language,
        username,
        cmdargs
    );

#ifdef JAVASCRIPT
    free(language);
    free(username);
    free(cmdargs);
#endif

    return 0;
}


void script_environment_register(lua_State* L) {
    luascript_register(L, ENVIRONMENT, script_environment_gc, script_environment_tostring, ENVIRONMENT_FUNCTIONS);
}

