using System;
using Engine.Externals.LuaInterop;
using Engine.Platform;

public static class ExportsPSShader {

    public const string PSSHADER = "PSShader";

    private static readonly double[] buffer = new double[16];



    static int script_psshader_init(LuaState L) {
        string vertex_sourcecode = L.luaL_optstring(1, null);
        string fragment_sourcecode = L.luaL_optstring(2, null);

        PSShader psshader = PSShader.Init(vertex_sourcecode, fragment_sourcecode);

        if (psshader == null) {
            L.lua_pushnil();
            return 1;
        }

        psshader.was_allocated_by_lua = true;
        return L.CreateUserdata<PSShader>(PSSHADER, psshader);
    }

    static int script_psshader_destroy(LuaState L) {
        PSShader psshader = L.ReadUserdata<PSShader>(PSSHADER);

        if (!psshader.was_allocated_by_lua) {
            return L.luaL_error("this object was not allocated by lua");
        }

        return L.NullifyUserdata(PSSHADER);
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
        new LuaTableFunction() { name = "destroy", func = script_psshader_destroy },
        new LuaTableFunction() { name = "set_uniform_any", func = script_psshader_set_uniform_any },
        new LuaTableFunction() { name = "set_uniform1f", func = script_psshader_set_uniform1f },
        new LuaTableFunction() { name = "set_uniform1i", func = script_psshader_set_uniform1i },
        new LuaTableFunction() { name = null, func = null }
    };


    public static int script_psshader_new(LuaState L, PSShader psshader) {
        return L.CreateUserdata<PSShader>(PSSHADER, psshader);
    }

    static int script_psshader_gc(LuaState L) {
        PSShader psshader = L.ReadUserdata<PSShader>(PSSHADER);

        if (psshader.was_allocated_by_lua) {
            L.NullifyUserdata(PSSHADER);
            psshader.Destroy();
        }

        return 0;
    }

    static int script_psshader_tostring(LuaState L) {
        PSShader psshader = L.ReadUserdata<PSShader>(PSSHADER);
        L.lua_pushstring("[PSShader]");
        return 1;
    }

    private static readonly LuaCallback delegate_psshader_init = script_psshader_init;

    public static void register_psshader(ManagedLuaState L) {
        // register constructor
        L.RegisterGlobalFunction("engine_create_shader", delegate_psshader_init);

        L.RegisterMetaTable(
            PSSHADER,
            script_psshader_gc,
            script_psshader_tostring,
            PSSHADER_FUNCTIONS
        );
    }

}


