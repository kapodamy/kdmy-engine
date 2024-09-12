#include "luascript_internal.h"

void script_script_register(lua_State* L) {

	const char* script_sourcecode =
        "--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]\n"
        "function importScript(lua_script_filename)\n"
        "    local str = fs_readfile(lua_script_filename)\n"
        "    if str == nil then\n"
        "        return false\n"
        "    end\n"
        "    local fn, err = load(str, lua_script_filename, 't')\n"
        "    if fn then\n"
        "        fn()\n"
        "        return true\n"
        "    end\n"
        "    print(err)\n"
        "    return false\n"
        "end\n"
        "function evalScript(lua_script_filename)\n"
        "    local str = fs_readfile(lua_script_filename)\n"
        "    if str == nil then\n"
        "        return false\n"
        "    end\n"
        "    local fn, err = load(str, lua_script_filename, 't')\n"
        "    if fn then\n"
        "        return fn()\n"
        "    end\n"
        "    print(err)\n"
        "    return false\n"
        "end;\n";

	int result = luaL_dostring(L, script_sourcecode);
	assert(result == 0/*register_script() failed*/);
}

