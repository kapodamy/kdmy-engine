using System;
using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Image;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsAnimSprite {

        public const string ANIMSPRITE = "AnimSprite";


        static int script_animsprite_init_from_atlas(LuaState L) {
            float frame_rate = (float)L.luaL_checknumber(1);
            int loop = (int)L.luaL_checkinteger(2);
            Atlas atlas = L.ReadNullableUserdata<Atlas>(3, ExportsAtlas.ATLAS);
            string prefix = L.luaL_optstring(4, null);
            bool has_number_suffix = L.lua_toboolean(5);

            AnimSprite ret = AnimSprite.InitFromAtlas(frame_rate, loop, atlas, prefix, has_number_suffix);

            return L.CreateAllocatedUserdata(ANIMSPRITE, ret);
        }

        static int script_animsprite_init_from_animlist(LuaState L) {
            AnimList animlist = L.ReadUserdata<AnimList>(ExportsAnimList.ANIMLIST);
            string animation_name = L.luaL_optstring(2, null);

            AnimSprite ret = AnimSprite.InitFromAnimlist(animlist, animation_name);

            return L.CreateAllocatedUserdata(ANIMSPRITE, ret);
        }

        static int script_animsprite_init_from_tweenlerp(LuaState L) {
            string name = L.luaL_optstring(1, null);
            int loop = (int)L.luaL_checkinteger(2);
            TweenLerp tweenlerp = L.ReadNullableUserdata<TweenLerp>(3, ExportsTweenLerp.TWEENLERP);

            AnimSprite ret = AnimSprite.InitFromTweenLerp(name, loop, tweenlerp);

            return L.CreateAllocatedUserdata(ANIMSPRITE, ret);
        }

        static int script_animsprite_init_as_empty(LuaState L) {
            string name = L.luaL_checkstring(1);

            AnimSprite ret = AnimSprite.InitAsEmpty(name);

            return L.CreateAllocatedUserdata(ANIMSPRITE, ret);
        }

        static int script_animsprite_init(LuaState L) {
            AnimListItem animlist_item = L.ReadNullableUserdata<AnimListItem>(1, ExportsAnimListItem.ANIMLISTITEM);

            AnimSprite ret = AnimSprite.Init(animlist_item);

            return L.CreateAllocatedUserdata(ANIMSPRITE, ret);
        }

        static int script_animsprite_destroy(LuaState L) {
            AnimSprite animsprite = L.ReadUserdata<AnimSprite>(ANIMSPRITE);

            if (L.IsUserdataAllocated(ANIMSPRITE))
                animsprite.Destroy();
            else
                Console.WriteLine("script_animsprite_destroy() object was not allocated by lua");

            return 0;
        }

        static int script_animsprite_set_loop(LuaState L) {
            AnimSprite animsprite = L.ReadUserdata<AnimSprite>(ANIMSPRITE);
            int loop = (int)L.luaL_checkinteger(2);

            animsprite.SetLoop(loop);

            return 0;
        }

        static int script_animsprite_get_name(LuaState L) {
            AnimSprite animsprite = L.ReadUserdata<AnimSprite>(ANIMSPRITE);

            string ret = animsprite.GetName();

            L.lua_pushstring(ret);
            return 1;
        }

        static int script_animsprite_is_frame_animation(LuaState L) {
            AnimSprite animsprite = L.ReadUserdata<AnimSprite>(ANIMSPRITE);

            bool ret = animsprite.IsFrameAnimation();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_animsprite_set_delay(LuaState L) {
            AnimSprite animsprite = L.ReadUserdata<AnimSprite>(ANIMSPRITE);
            float delay_milliseconds = (float)L.luaL_checknumber(2);

            animsprite.SetDelay(delay_milliseconds);

            return 0;
        }




        static readonly LuaTableFunction[] ANIMSPRITE_FUNCTIONS = {
            new LuaTableFunction("init_from_atlas", script_animsprite_init_from_atlas),
            new LuaTableFunction("init_from_animlist", script_animsprite_init_from_animlist),
            new LuaTableFunction("init_from_tweenlerp", script_animsprite_init_from_tweenlerp),
            new LuaTableFunction("init_as_empty", script_animsprite_init_as_empty),
            new LuaTableFunction("init", script_animsprite_init),
            new LuaTableFunction("destroy", script_animsprite_destroy),
            new LuaTableFunction("set_loop", script_animsprite_set_loop),
            new LuaTableFunction("get_name", script_animsprite_get_name),
            new LuaTableFunction("is_frame_animation", script_animsprite_is_frame_animation),
            new LuaTableFunction("set_delay", script_animsprite_set_delay),
            new LuaTableFunction(null, null)
        };


        static int script_animsprite_new(LuaState L, AnimSprite animsprite) {
            return L.CreateUserdata(ANIMSPRITE, animsprite);
        }

        static int script_animsprite_gc(LuaState L) {
            return L.DestroyUserdata(ANIMSPRITE);
        }

        static int script_animsprite_tostring(LuaState L) {
            return L.ToString_userdata(ANIMSPRITE);
        }

        private static readonly LuaCallback delegate_gc = script_animsprite_gc;
        private static readonly LuaCallback delegate_tostring = script_animsprite_tostring;

        public static void script_animsprite_register(ManagedLuaState L) {
            L.RegisterMetaTable(ANIMSPRITE, delegate_gc, delegate_tostring, ANIMSPRITE_FUNCTIONS);
        }


    }
}
