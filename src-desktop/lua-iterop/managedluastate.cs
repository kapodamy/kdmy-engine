using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using LuaNativeMethods;

namespace Engine.Externals.LuaInterop {

    public sealed class ManagedLuaState : IDisposable {

        public const int LUA_OK = LUA.OK;


        internal object context;
        internal GCHandle self;

        internal string last_pushed_function_name;
        internal ReferenceList handle_references;

        internal unsafe lua_State* L;
        internal unsafe LuascriptObject* shared_array;
        internal unsafe int shared_size;


        private unsafe ManagedLuaState(lua_State* L, object context) {
            this.L = L;
            this.context = context;
            this.self = GCHandle.Alloc(this, GCHandleType.Normal);
            this.last_pushed_function_name = null;
            this.handle_references = new ReferenceList();
            this.shared_size = LuaInteropHelpers.SHARED_ARRAY_CHUNK_SIZE;
            this.shared_array = (LuascriptObject*)Marshal.AllocHGlobal(sizeof(LuascriptObject) * this.shared_size);

            LUA.luaL_openlibs(L);

            // overwrite print because the stdout can be redirected
            LUA.lua_pushcfunction(this.L, LuaInteropHelpers.delegate_print);
            LUA.lua_setglobal(this.L, "print");

            // allow lua scripts to emit warnings
            LUA.lua_setwarnf(this.L, LuaInteropHelpers.delegate_print_warning, null);

            LuaInteropHelpers.luascript_set_instance(this);
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
            this.handle_references.RevokeAllReferences();

            this.context = null;
            this.handle_references = null;
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

        public void RegisterConstantUserdata<T>(string variable, LuaUserdataNew<T> userdata_new, T obj) {
            unsafe {
                if (userdata_new != null) {
                    if (userdata_new(this.LuaStateHandle, obj) < 1) {
                        return;
                    }
                } else {
                    LUA.lua_pushnil(this.L);
                }

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

                if (newindex != null) {
                    LUA.lua_pushcfunction(lua, newindex);
                    LUA.lua_setfield(lua, -2, "__newindex");
                }

                LUA.lua_pop(lua, 1);
            }
        }

        public bool EvaluateString(string eval_string) {
            unsafe {
                int result = LUA.luaL_loadstring(this.L, eval_string);
                if (result != LUA.OK) return false;

                int ret = LuaInteropHelpers.luascript_pcallk(this.L, 0, LUA.MULTRET);

                return ret == LUA.OK;
            }
        }

        public int EvaluateString(string lua_sourcecode, string fake_filename) {
            unsafe {
                //int filename_index = LUA.lua_gettop(L) + 1;
                //LUA.lua_pushstring(L, $"@{filename}");

                // parse the lua sourcecode and give a false filename
                int status = LUA.luaL_loadbufferx(L, lua_sourcecode, $"@{fake_filename}");
                //LUA.lua_remove(L, filename_index);

                if (status == 0) status = LuaInteropHelpers.luascript_pcallk(L, 0, LUA.MULTRET);

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

                if (LuaInteropHelpers.luascript_pcallk(this.L, arguments_count, 0) == 0) {
                    return 0;
                }

                string error_message = LUA.lua_tostring(L, -1);
                Console.Error.WriteLine("lua_imported_fn() call to '{0}' failed.\n{1}\n", fn_name, error_message);
                //Console.Error.Flush();
                LUA.lua_pop(this.L, 1);
            }

            return 1;
        }

        public object CallPushedGlobalFunctionWithReturn(int arguments_count) {
            unsafe {
                string fn_name = this.last_pushed_function_name;
                this.last_pushed_function_name = null;

                if (LuaInteropHelpers.luascript_pcallk(this.L, arguments_count, 1) == 0) {

                    object value = null;
                    switch (LUA.lua_type(this.L, -1)) {
                        case LUA.TBOOLEAN:
                            value = LUA.lua_toboolean(this.L, -1);
                            break;
                        case LUA.TNUMBER:
                            value = LUA.lua_tonumber(this.L, -1);
                            break;
                        case LUA.TSTRING:
                            value = LUA.lua_tostring(this.L, -1);
                            break;
                        case LUA.TNIL:
                        case LUA.TNONE:
                            break;
                        default:
                            Console.Error.WriteLine("[ERROR] ManagedLuaState::CallPushedGlobalFunctionWithReturn() unknown lua return type");
                            break;
                    }

                    return value;
                }

                string error_message = LUA.lua_tostring(L, -1);
                Console.Error.WriteLine("lua_imported_fn() call to '{0}' failed.\n{1}\n", fn_name, error_message);
                //Console.Error.Flush();
                LUA.lua_pop(this.L, 1);
            }

            return null;
        }

        public void DropSharedObject(object obj) {
            unsafe {
                void* obj_ptr = this.handle_references.GetReference(obj);
                LuaInteropHelpers.luascript_remove_userdata(this, obj_ptr);
            }
        }

        public static string GetVersion() {
            unsafe {
                string version = null;

                lua_State* L = LUA.luaL_newstate();
                if (L == null) return null;
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
