using System;
using Engine.Externals.LuaInterop;
using Engine.Image;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsAtlas {

    public const string ATLAS = "Atlas";

    public static int script_atlas_push_atlas_entry(LuaState L, AtlasEntry entry) {
        if (entry == null) {
            L.lua_pushnil();
            return 1;
        }

        LuaTableBuilder table = new LuaTableBuilder(11);
        table.AddString("name", entry.name);
        table.AddNumber("x", entry.x);
        table.AddNumber("y", entry.y);
        table.AddNumber("width", entry.width);
        table.AddNumber("height", entry.height);
        table.AddNumber("frameX", entry.frame_x);
        table.AddNumber("frameY", entry.frame_y);
        table.AddNumber("frameWidth", entry.frame_width);
        table.AddNumber("frameHeight", entry.frame_height);
        table.AddNumber("pivotX", entry.pivot_x);
        table.AddNumber("pivotY", entry.pivot_y);

        table.PushTable(L);
        return 1;
    }


    static int script_atlas_init(LuaState L) {
        string src = L.luaL_checkstring(2);

        Atlas ret = Atlas.Init(src);
        return L.CreateAllocatedUserdata(ATLAS, ret);
    }

    static int script_atlas_destroy(LuaState L) {
        Atlas atlas = L.ReadUserdata<Atlas>(ATLAS);

        if (L.IsUserdataAllocated(ATLAS))
            atlas.Destroy();
        else
            Logger.Warn("script_atlas_destroy() object was not allocated by lua");

        return 0;
    }

    static int script_atlas_get_index_of(LuaState L) {
        Atlas atlas = L.ReadUserdata<Atlas>(ATLAS);
        string name = L.luaL_optstring(2, null);

        int ret = atlas.GetIndexOf(name);

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_atlas_get_entry(LuaState L) {
        Atlas atlas = L.ReadUserdata<Atlas>(ATLAS);
        string name = L.luaL_optstring(2, null);

        AtlasEntry ret = atlas.GetEntry(name);

        return script_atlas_push_atlas_entry(L, ret);
    }

    static int script_atlas_get_entry_with_number_suffix(LuaState L) {
        Atlas atlas = L.ReadUserdata<Atlas>(ATLAS);
        string name_prefix = L.luaL_optstring(2, null);

        AtlasEntry ret = atlas.GetEntryWithNumberSuffix(name_prefix);

        return script_atlas_push_atlas_entry(L, ret);
    }

    static int script_atlas_get_glyph_fps(LuaState L) {
        Atlas atlas = L.ReadUserdata<Atlas>(ATLAS);

        float ret = atlas.GetGlyphFPS();

        L.lua_pushnumber(ret);
        return 1;
    }

    static int script_atlas_get_texture_resolution(LuaState L) {
        Atlas atlas = L.ReadUserdata<Atlas>(ATLAS);
        int resolution_width, resolution_height;

        atlas.GetTextureResolution(out resolution_width, out resolution_height);

        L.lua_pushinteger(resolution_width);
        L.lua_pushinteger(resolution_height);
        return 2;
    }

    static int script_atlas_utils_is_known_extension(LuaState L) {
        string src = L.luaL_checkstring(1);

        bool ret = Atlas.UtilsIsKnownExtension(src);

        L.lua_pushboolean(ret);
        return 1;
    }




    static readonly LuaTableFunction[] ATLAS_FUNCTIONS = {
        new LuaTableFunction("init", script_atlas_init),
        new LuaTableFunction("destroy", script_atlas_destroy),
        new LuaTableFunction("get_index_of", script_atlas_get_index_of),
        new LuaTableFunction("get_entry", script_atlas_get_entry),
        new LuaTableFunction("get_entry_with_number_suffix", script_atlas_get_entry_with_number_suffix),
        new LuaTableFunction("get_glyph_fps", script_atlas_get_glyph_fps),
        new LuaTableFunction("get_texture_resolution", script_atlas_get_texture_resolution),
        new LuaTableFunction("utils_is_known_extension", script_atlas_utils_is_known_extension),
        new LuaTableFunction(null, null)
    };

    public static int script_atlas_new(LuaState L, Atlas atlas) {
        return L.CreateUserdata(ATLAS, atlas);
    }

    static int script_atlas_gc(LuaState L) {
        return L.DestroyUserdata(ATLAS);
    }

    static int script_atlas_tostring(LuaState L) {
        return L.ToString_userdata(ATLAS);
    }

    private static readonly LuaCallback delegate_gc = script_atlas_gc;
    private static readonly LuaCallback delegate_tostring = script_atlas_tostring;

    public static void script_atlas_register(ManagedLuaState L) {
        L.RegisterMetaTable(ATLAS, delegate_gc, delegate_tostring, ATLAS_FUNCTIONS);
    }
}
