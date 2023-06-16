using System.Diagnostics;
using Engine.Externals.LuaInterop;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsScript {

    internal static void script_script_register(ManagedLuaState lua) {

        const string script_sourcecode =
            "--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]\n" +
            "function importScript(lua_script_filename)\n" +
            "    local str = fs_readfile(lua_script_filename)\n" +
            "    if str == nil then\n" +
            "        return false\n" +
            "    end\n" +
            "    local fn, err = load(str, lua_script_filename, 't')\n" +
            "    if fn then\n" +
            "        fn()\n" +
            "        return true\n" +
            "    end\n" +
            "    print(err)\n" +
            "    return false\n" +
            "end\n" +
            "function evalScript(lua_script_filename)\n" +
            "    local str = fs_readfile(lua_script_filename)\n" +
            "    if str == nil then\n" +
            "        return false\n" +
            "    end\n" +
            "    local fn, err = load(str, lua_script_filename, 't')\n" +
            "    if fn then\n" +
            "        return fn()\n" +
            "    end\n" +
            "    print(err)\n" +
            "    return false\n" +
            "end;\n";

        bool result = lua.EvaluateString(script_sourcecode);
        Debug.Assert(result, "register_script() failed");
    }

}
