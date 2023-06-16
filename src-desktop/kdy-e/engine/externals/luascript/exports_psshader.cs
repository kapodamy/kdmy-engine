using System;
using Engine.Externals.LuaInterop;
using Engine.Platform;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsPSShader {

    public const string PSSHADER = "PSShader";

    private static readonly double[] buffer = new double[16];



    static int script_psshader_init(LuaState L) {
        string vertex_sourcecode = L.luaL_optstring(2, null);
        string fragment_sourcecode = L.luaL_optstring(3, null);

        PSShader psshader = PSShader.Init(vertex_sourcecode, fragment_sourcecode);

        return L.CreateAllocatedUserdata(PSSHADER, psshader);
    }

    static int script_psshader_destroy(LuaState L) {
        PSShader psshader = L.ReadUserdata<PSShader>(PSSHADER);

        if (L.IsUserdataAllocated(PSSHADER))
            psshader.Destroy();
        else
            Console.WriteLine("script_psshader_destroy() object was not allocated by lua");

        return 0;
    }

    static int script_psshader_set_uniform_any(LuaState L) {
        PSShader psshader = L.ReadUserdata<PSShader>(PSSHADER);

        string name = L.luaL_checkstring(2);
        int values_count = L.lua_gettop() - 2;

        Array.Clear(buffer, 0, buffer.Length);
        for (int i = 0 ; i < values_count ; i++) {
            if (i > 16) break;
            buffer[i] = L.luaL_checknumber(i + 3);
        }

        int ret = psshader.SetUniformAny(name, buffer);
        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_psshader_set_uniform1f(LuaState L) {
        PSShader psshader = L.ReadUserdata<PSShader>(PSSHADER);

        string name = L.luaL_checkstring(2);
        float value = (float)L.luaL_checknumber(3);

        bool ret = psshader.SetUniform1F(name, value);

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_psshader_set_uniform1i(LuaState L) {
        PSShader psshader = L.ReadUserdata<PSShader>(PSSHADER);

        string name = L.luaL_checkstring(2);
        int value = (int)L.luaL_checkinteger(3);

        bool ret = psshader.SetUniform1I(name, value);

        L.lua_pushboolean(ret);
        return 1;
    }




    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////

    private static readonly LuaTableFunction[] PSSHADER_FUNCTIONS = {
        new LuaTableFunction("init", script_psshader_init),
        new LuaTableFunction("destroy", script_psshader_destroy),
        new LuaTableFunction("set_uniform_any", script_psshader_set_uniform_any),
        new LuaTableFunction("set_uniform1f", script_psshader_set_uniform1f),
        new LuaTableFunction("set_uniform1i", script_psshader_set_uniform1i),
        new LuaTableFunction(null, null)
    };


    public static int script_psshader_new(LuaState L, PSShader psshader) {
        return L.CreateUserdata<PSShader>(PSSHADER, psshader);
    }

    static int script_psshader_gc(LuaState L) {
        return L.DestroyUserdata(PSSHADER);
    }

    static int script_psshader_tostring(LuaState L) {
        return L.ToString_userdata(PSSHADER);
    }

    private static readonly LuaCallback delegate_gc = script_psshader_gc;
    private static readonly LuaCallback delegate_tostring = script_psshader_tostring;

    public static void script_psshader_register(ManagedLuaState L) {
        L.RegisterMetaTable(PSSHADER, delegate_gc, delegate_tostring, PSSHADER_FUNCTIONS);
    }

}


