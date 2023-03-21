using Engine.Animation;
using System;
using Engine.Externals.LuaInterop;
using Engine.Image;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsModelHolder {

        public const string MODELHOLDER = "ModelHolder";

        static int script_modelholder_init(LuaState L) {
            string src = L.luaL_checkstring(2);

            ModelHolder ret = ModelHolder.Init(src);

            return L.CreateAllocatedUserdata(MODELHOLDER, ret);
        }

        static int script_modelholder_init2(LuaState L) {
            uint vertex_color_rgb8 = (uint)L.luaL_checkinteger(2);
            string atlas_src = L.luaL_optstring(3, null);
            string animlist_src = L.luaL_optstring(4, null);

            ModelHolder ret = ModelHolder.Init2(vertex_color_rgb8, atlas_src, animlist_src);

            return L.CreateAllocatedUserdata(MODELHOLDER, ret);
        }

        static int script_modelholder_destroy(LuaState L) {
            ModelHolder modelholder = L.ReadUserdata<ModelHolder>(MODELHOLDER);

            if (L.IsUserdataAllocated(MODELHOLDER))
                modelholder.Destroy();
            else
                Console.WriteLine("script_modelholder_destroy() object was not allocated by lua");

            return 0;
        }

        static int script_modelholder_is_invalid(LuaState L) {
            ModelHolder modelholder = L.ReadUserdata<ModelHolder>(MODELHOLDER);

            bool ret = modelholder.IsInvalid();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_modelholder_has_animlist(LuaState L) {
            ModelHolder modelholder = L.ReadUserdata<ModelHolder>(MODELHOLDER);

            bool ret = modelholder.HasAnimlist();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_modelholder_create_animsprite(LuaState L) {
            ModelHolder modelholder = L.ReadNullableUserdata<ModelHolder>(2, MODELHOLDER);
            string animation_name = L.luaL_optstring(3, null);
            bool fallback_ = L.lua_toboolean(4);
            bool no_return_null = L.lua_toboolean(5);

            AnimSprite ret = modelholder.CreateAnimsprite(animation_name, fallback_, no_return_null);

            return L.CreateAllocatedUserdata(ExportsAnimSprite.ANIMSPRITE, ret);
        }

        static int script_modelholder_get_atlas(LuaState L) {
            ModelHolder modelholder = L.ReadUserdata<ModelHolder>(MODELHOLDER);

            Atlas ret = modelholder.GetAtlas();

            return ExportsAtlas.script_atlas_new(L, ret);
        }

        static int script_modelholder_get_vertex_color(LuaState L) {
            ModelHolder modelholder = L.ReadUserdata<ModelHolder>(MODELHOLDER);

            uint ret = modelholder.GetVertexColor();

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_modelholder_get_animlist(LuaState L) {
            ModelHolder modelholder = L.ReadUserdata<ModelHolder>(MODELHOLDER);

            AnimList ret = modelholder.GetAnimlist();

            return ExportsAnimList.script_animlist_new(L, ret);
        }

        static int script_modelholder_get_atlas_entry(LuaState L) {
            ModelHolder modelholder = L.ReadUserdata<ModelHolder>(MODELHOLDER);
            string atlas_entry_name = L.luaL_optstring(2, null);

            AtlasEntry ret = modelholder.GetAtlasEntry(atlas_entry_name, false);

            return ExportsAtlas.script_atlas_push_atlas_entry(L, ret);
        }

        static int script_modelholder_get_atlas_entry2(LuaState L) {
            ModelHolder modelholder = L.ReadUserdata<ModelHolder>(MODELHOLDER);
            string atlas_entry_name = L.luaL_optstring(2, null);

            AtlasEntry ret = modelholder.GetAtlasEntry2(atlas_entry_name, false);

            return ExportsAtlas.script_atlas_push_atlas_entry(L, ret);
        }

        static int script_modelholder_get_texture_resolution(LuaState L) {
            ModelHolder modelholder = L.ReadUserdata<ModelHolder>(MODELHOLDER);

            int resolution_width, resolution_height;
            modelholder.GetTextureResolution(out resolution_width, out resolution_height);

            L.lua_pushinteger(resolution_width);
            L.lua_pushinteger(resolution_height);
            return 2;
        }

        static int script_modelholder_utils_is_known_extension(LuaState L) {
            string filename = L.luaL_checkstring(2);

            bool ret = ModelHolder.UtilsIsKnownExtension(filename);

            L.lua_pushboolean(ret);
            return 1;
        }




        private static readonly LuaTableFunction[] MODELHOLDER_FUNCTIONS = {
            new LuaTableFunction("init", script_modelholder_init),
            new LuaTableFunction("init2", script_modelholder_init2),
            new LuaTableFunction("destroy", script_modelholder_destroy),
            new LuaTableFunction("is_invalid", script_modelholder_is_invalid),
            new LuaTableFunction("has_animlist", script_modelholder_has_animlist),
            new LuaTableFunction("create_animsprite", script_modelholder_create_animsprite),
            new LuaTableFunction("get_atlas", script_modelholder_get_atlas),
            new LuaTableFunction("get_vertex_color", script_modelholder_get_vertex_color),
            new LuaTableFunction("get_animlist", script_modelholder_get_animlist),
            new LuaTableFunction("get_atlas_entry", script_modelholder_get_atlas_entry),
            new LuaTableFunction("get_atlas_entry2", script_modelholder_get_atlas_entry2),
            new LuaTableFunction("get_texture_resolution", script_modelholder_get_texture_resolution),
            new LuaTableFunction("utils_is_known_extension", script_modelholder_utils_is_known_extension),
            new LuaTableFunction(null, null)
        };

        static int script_modelholder_new(LuaState L, ModelHolder modelholder) {
            return L.CreateUserdata(MODELHOLDER, modelholder);
        }

        static int script_modelholder_gc(LuaState L) {
            return L.DestroyUserdata(MODELHOLDER);
        }

        static int script_modelholder_tostring(LuaState L) {
            return L.ToString_userdata(MODELHOLDER);
        }

        private static readonly LuaCallback delegate_gc = script_modelholder_gc;
        private static readonly LuaCallback delegate_tostring = script_modelholder_tostring;

        public static void script_modelholder_register(ManagedLuaState L) {
            L.RegisterMetaTable(MODELHOLDER, delegate_gc, delegate_tostring, MODELHOLDER_FUNCTIONS);
        }

    }
}
