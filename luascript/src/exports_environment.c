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
#endif

static int script_environment_get_language(lua_State* L) {

#ifdef JAVASCRIPT
    char* language = kdmyEngine_get_language();
    lua_pushstring(L, language);
    free(language);
#else
    lua_pushstring(L, "English");
#endif

    return 1;
}

static int script_environment_get_username(lua_State* L) {

#ifdef JAVASCRIPT
    char* useragent = kdmyEngine_get_useragent();
    lua_pushstring(L, useragent);
    free(useragent);
#else
    lua_pushstring(L, "Cosme Fulanito");
#endif

    return 1;
}

static int script_environment_get_cmdargs(lua_State* L) {

#ifdef JAVASCRIPT
    char* locationquery = kdmyEngine_get_locationquery();
    lua_pushstring(L, locationquery);
    free(locationquery);
#else
    lua_pushstring(L, "");
#endif

    return 1;
}

static int script_environment_exit(lua_State* L) {
    int exit_code = (int)luaL_checkinteger(L, 1);
    exit(exit_code);
    return 0;
}


static const luaL_Reg ENVIRONMENT_FUNCTIONS[] = {
    { "get_language", script_environment_get_language },
    { "get_username", script_environment_get_username },
    { "get_cmdargs", script_environment_get_cmdargs },
    { "exit", script_environment_exit },
};

void script_environment_register(lua_State* L) {
    luascript_register(L, ENVIRONMENT, NULL, NULL, ENVIRONMENT_FUNCTIONS);
}

