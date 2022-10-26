using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using LuaNativeMethods;

namespace Engine.Externals.LuaInterop {

    public sealed class ManagedLuaState : IDisposable {

        internal const int LUA_REGISTRY_TABLE_KEY = 1;

        public const int LUA_OK = 0;
        public const int LUA_YIELD = 1;
        public const int LUA_ERRRUN = 2;
        public const int LUA_ERRSYNTAX = 3;
        public const int LUA_ERRMEM = 4;
        public const int LUA_ERRERR = 5;

        internal object context;
        internal unsafe lua_State* L;
        internal UserdataList userdata_list;
        internal ReferenceList allocated_objects;
        private GCHandle self;
        private string last_pushed_function_name;

        private unsafe ManagedLuaState(lua_State* L, object context) {
            this.L = L;
            this.context = context;
            this.self = GCHandle.Alloc(this, GCHandleType.Normal);
            this.userdata_list = new UserdataList(L);
            this.allocated_objects = new ReferenceList();
            this.last_pushed_function_name = null;

            LUA.luaL_openlibs(L);

            LUA.lua_pushlightuserdata(L, (void*)LUA_REGISTRY_TABLE_KEY);
            LUA.lua_pushlightuserdata(L, (void*)GCHandle.ToIntPtr(this.self));
            LUA.lua_settable(L, LUA.REGISTRYINDEX);
        }

        public static ManagedLuaState Init(object context) {
            unsafe {
                lua_State* L = LUA.luaL_newstate();
                if (L == null) return null;
                return new ManagedLuaState(L, context);
            }
        }

        public void Dispose() {
            unsafe {
                LUA.lua_close(this.L);
                this.L = null;
            }

            if (this.self.IsAllocated) this.self.Free();
            this.context = null;
            this.userdata_list.Dispose();
            this.allocated_objects.Dispose();
        }

        internal unsafe static ManagedLuaState SelfFrom(void* luascript) {
            if (luascript == null) {
                throw new NullReferenceException("Can not recover the object, NULL pointer provided");
            }

            GCHandle self = GCHandle.FromIntPtr((IntPtr)luascript);
            object target = self.Target;

            Debug.Assert(target != null, "self.Target returned null");

            return (ManagedLuaState)target;
        }


        public LuaState LuaStateHandle {
            get
            {
                if (!this.self.IsAllocated) throw new InvalidOperationException("Object disposed");
                return LuaState.FromManagedLuaState(this);
            }
        }


        public void RegisterMetaTable(string name, LuaCallback gc, LuaCallback tostring, LuaTableFunction[] fns) {
            unsafe {
                luaL_Reg[] OBJECT_METAMETHODS = new luaL_Reg[] {
                    new luaL_Reg() {name="__gc", func=gc},
                    new luaL_Reg() {name="__tostring", func=tostring},
                    new luaL_Reg() {name=null, func=null}
                };

                luaL_Reg[] reg_fns = new luaL_Reg[fns.Length];
                for (int i = 0 ; i < fns.Length ; i++) {
                    reg_fns[i] = new luaL_Reg() { name = fns[i].name, func = fns[i].func };
                }

                LUA.lua_newtable(this.L);
                LUA.luaL_setfuncs(this.L, reg_fns, 0);
                LUA.lua_pushvalue(this.L, -1);
                LUA.lua_setglobal(this.L, name);

                LUA.luaL_newmetatable(this.L, name);

                LUA.luaL_setfuncs(this.L, OBJECT_METAMETHODS, 0);

                LUA.lua_pushliteral(this.L, "__index");
                LUA.lua_pushvalue(this.L, -3);
                LUA.lua_rawset(this.L, -3);

                LUA.lua_pushliteral(this.L, "__metatable");
                LUA.lua_pushvalue(this.L, -3);
                LUA.lua_rawset(this.L, -3);

                LUA.lua_pop(this.L, 1);
            }
        }

        public void RegisterGlobalFunctions(LuaTableFunction[] staticfunctions) {
            for (int i = 0 ; i < staticfunctions.Length ; i++) {
                if (staticfunctions[i].name == null || staticfunctions[i].func == null) break;

                unsafe {
                    LUA.lua_pushcfunction(this.L, staticfunctions[i].func);
                    LUA.lua_setglobal(this.L, staticfunctions[i].name);
                }
            }
        }

        public void RegisterGlobalFunction(string name, LuaCallback func) {
            unsafe {
                LUA.lua_pushcfunction(this.L, func);
                LUA.lua_setglobal(this.L, name);
            }
        }

        public void RegisterStringConstants(LuaStringConstant[] constants) {
            for (int i = 0 ; i < constants.Length ; i++) {
                if (constants[i].variable == null) break;
                unsafe {
                    LUA.lua_pushstring(this.L, constants[i].value);
                    LUA.lua_setglobal(this.L, constants[i].variable);
                }
            }
        }

        public void RegisterIntegerConstants(LuaIntegerConstant[] constants) {
            for (int i = 0 ; i < constants.Length ; i++) {
                if (constants[i].variable == null) break;
                unsafe {
                    LUA.lua_pushinteger(this.L, constants[i].value);
                    LUA.lua_setglobal(this.L, constants[i].variable);
                }
            }

        }

        public void RegisterConstantString(string variable, string value) {
            unsafe {
                LUA.lua_pushstring(this.L, value);
                LUA.lua_setglobal(this.L, variable);
            }
        }

        public void RegisterConstantInteger(string variable, long value) {
            unsafe {
                LUA.lua_pushinteger(this.L, value);
                LUA.lua_setglobal(this.L, variable);
            }
        }

        public void RegisterStructMetaTable(string name, LuaCallback gc, LuaCallback tostring, LuaCallback index, LuaCallback newindex) {
            unsafe {
                lua_State* lua = this.L;

                LUA.luaL_newmetatable(lua, name);

                LUA.lua_pushcfunction(lua, gc);
                LUA.lua_setfield(lua, -2, "__gc");

                LUA.lua_pushcfunction(lua, tostring);
                LUA.lua_setfield(lua, -2, "__tostring");

                LUA.lua_pushcfunction(lua, index);
                LUA.lua_setfield(lua, -2, "__index");

                LUA.lua_pushcfunction(lua, newindex);
                LUA.lua_setfield(lua, -2, "__newindex");

                LUA.lua_pop(lua, 1);
            }
        }

        public int EvaluateString(string lua_sourcecode) {
            unsafe {
                return LUA.luaL_dostring(this.L, lua_sourcecode);
            }
        }

        public int EvaluateString(string lua_sourcecode, string fake_filename) {
            unsafe {
                //int filename_index = LUA.lua_gettop(L) + 1;
                //LUA.lua_pushstring(L, $"@{filename}");

                // parse the lua sourcode and give a false filename
                int status = LUA.luaL_loadbufferx(L, lua_sourcecode, $"@{fake_filename}");
                //LUA.lua_remove(L, filename_index);

                if (status == 0) status = LUA.lua_pcall(L, 0, LUA.MULTRET, 0);

                return status;
            }
        }

        public bool PushGlobalFunction(string function_name) {
            unsafe {
                if (LUA.lua_getglobal(this.L, function_name) != LUA.TFUNCTION) {
                    LUA.lua_pop(this.L, 1);
                    return true;
                }
                this.last_pushed_function_name = function_name;
            }
            return false;
        }

        public int CallPushedGlobalFunction(int arguments_count) {
            unsafe {
                string fn_name = this.last_pushed_function_name;
                this.last_pushed_function_name = null;

                if (LUA.lua_pcallk(this.L, arguments_count, 0, 0, 0, null) == 0) {
                    return 0;
                }

                string error_message = LUA.lua_tostring(L, -1);
                Console.Error.WriteLine("lua_imported_fn() call to '{0}' failed.\n{1}\n", fn_name, error_message);
                //Console.Error.Flush();
                LUA.lua_pop(this.L, 1);
            }

            return 1;
        }

        public static string GetVersion() {
            unsafe {
                string version = null;
                lua_State* L = LUA.luaL_newstate();

                LUA.luaL_openlibs(L);

                int ret = LUA.lua_getglobal(L, "_VERSION");
                if (ret == LUA.TSTRING) {
                    version = LUA.lua_tostring(L, 1);
                }

                LUA.lua_pop(L, 1);
                LUA.lua_close(L);

                return version;
            }
        }

    }

}
