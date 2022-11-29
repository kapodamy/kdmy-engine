using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using LuaNativeMethods;

namespace Engine.Externals.LuaInterop {

    public struct LuaState {

        internal unsafe lua_State* L;
        internal static LuaState FromManagedLuaState(ManagedLuaState mls) {
            LuaState instance = new LuaState();
            unsafe { instance.L = mls.L; }
            return instance;
        }


        public T ReadUserdata<T>(string metatable_name) where T : class {
            unsafe {
                Debug.Assert(this.L != null);

                if (LUA.lua_isnil(L, 1)) {
                    // this function should never return
                    LUA.luaL_error(this.L, $"{metatable_name} was null (nil in lua).");
                }

                void** ptr;
                if (metatable_name == null)
                    ptr = (void**)LUA.lua_touserdata(L, 1);
                else
                    ptr = (void**)LUA.luaL_checkudata(L, 1, metatable_name);

                T obj = (T)AdquireObject(ptr);
                if (obj != null) return obj;


                // this function should never return
                LUA.luaL_error(this.L, $"{metatable_name} object was disposed.");
            }

            return null;
        }

        public T ReadUserdataOrNull<T>(int n, string metatable_name) where T : class {
            unsafe {
                if (LUA.lua_isnil(L, n)) return null;

                void** ptr;
                if (metatable_name == null)
                    ptr = (void**)LUA.lua_touserdata(L, n);
                else
                    ptr = (void**)LUA.luaL_checkudata(L, n, metatable_name);

                T obj = (T)AdquireObject(ptr);
                if (obj != null) return obj;


                // this function should never return
                LUA.luaL_error(this.L, $"{metatable_name} object was disposed.");
            }

            return null;
        }

        public int CreateUserdata<T>(string metatable_name, T obj) where T : class {
            unsafe {
                Debug.Assert(this.L != null);

                if (obj == null) goto L_push_nil_and_return;

                void** userdata = (void**)LUA.lua_newuserdata(L, sizeof(void**));
                Debug.Assert(userdata != null, "Error: lua_newuserdata() returned NULL");

                if (LUA.luaL_getmetatable(L, metatable_name) != LUA.TTABLE) {
                    Console.Error.WriteLine($"[ERROR] luaL_getmetatable() missing '{metatable_name}'");
                    return 1;
                }

                void* ptr = AllocObject(obj);
                if (ptr == null) goto L_push_nil_and_return;

                *userdata = ptr;

                LUA.lua_setmetatable(L, -2);
                return 1;

L_push_nil_and_return:
                LUA.lua_pushnil(L);
                return 1;
            }
        }

        public int NullifyUserdata(string metatable_name) {
            unsafe {
                void** ptr;

                if (!LUA.lua_isnil(L, 1)) {
                    if (metatable_name == null)
                        ptr = (void**)LUA.lua_touserdata(L, 1);
                    else
                        ptr = (void**)LUA.luaL_checkudata(L, 1, metatable_name);

                    if (ptr != null) {
                        DeallocObject(*ptr);
                        *ptr = null;
                    }
                }
            }

            return 0;
        }


        private unsafe void* AllocObject(object obj) {
            if (obj == null) return null;
            GCHandle handle = GCHandle.Alloc(obj, GCHandleType.WeakTrackResurrection);
            return (void*)GCHandle.ToIntPtr(handle);
        }

        private unsafe void DeallocObject(void* ptr) {
            if (ptr == null) return;

            GCHandle handle = GCHandle.FromIntPtr((IntPtr)ptr);
            if (!handle.IsAllocated) return;
            handle.Free();
        }

        private unsafe object AdquireObject(void** ptr) {
            if (ptr == null || *ptr == null) return null;

            GCHandle handle = GCHandle.FromIntPtr((IntPtr)(*ptr));

            if (handle.IsAllocated)
                return handle.Target;
            else
                return null;
        }




        public ManagedLuaState Self {
            get
            {
                unsafe {
                    if (this.L == null) {
                        throw new InvalidOperationException("LuaState is not initialized");
                    }

                    LUA.lua_pushlightuserdata(L, (void*)ManagedLuaState.LUA_REGISTRY_TABLE_KEY);
                    LUA.lua_gettable(L, LUA.REGISTRYINDEX);
                    void* ptr = LUA.lua_touserdata(L, -1);
                    LUA.lua_pop(L, 1);

                    return ManagedLuaState.SelfFrom(ptr);
                }
            }
        }

        public object Context { get => this.Self.context; }


        public string luaL_optstring(int arg, string d) {
            unsafe {
                return LUA.luaL_optstring(L, arg, d);
            }
        }

        public float luaL_checkfloat(int arg) {
            unsafe {
                double value = LUA.luaL_checknumber(L, arg);
                return (float)value;
            }
        }

        public float luaL_optionalfloat(int arg) {
            unsafe {
                double value = LUA.luaL_optnumber(L, arg, Double.NaN);
                return (float)value;
            }
        }

        public bool luaL_checkboolean(int arg) {
            unsafe {
                int value = LUA.lua_toboolean(L, arg);
                return value != 0;
            }
        }

        public void lua_pushnumber(double n) {
            unsafe {
                LUA.lua_pushnumber(L, n);
            }
        }

        public void lua_pushboolean(bool b) {
            unsafe {
                LUA.lua_pushboolean(L, b ? 1 : 0);
            }
        }

        public void lua_pushstring(string s) {
            unsafe {
                LUA.lua_pushstring(L, s);
            }
        }

        public void lua_pushinteger(long i) {
            unsafe {
                LUA.lua_pushinteger(L, i);
            }
        }

        public void lua_pushnil() {
            unsafe {
                LUA.lua_pushnil(L);
            }
        }

        public void lua_pushunsigned(ulong u) {
            unsafe {
                LUA.lua_pushinteger(L, (long)u);
            }
        }

        public int luaL_error(string message) {
            unsafe {
                LUA.lua_pushliteral(L, message);
                return LUA.lua_error(L);
            }
        }

        public string lua_tostring(int i) {
            unsafe {
                return LUA.lua_tostring(L, i);
            }
        }

        public int luaL_argerror(int arg, string extramsg) {
            unsafe {
                return LUA.luaL_argerror(L, arg, extramsg);
            }
        }

        public T luaL_testudata<T>(int ud, string tname) where T : class {
            IntPtr opaque_handle;
            unsafe {
                void* ptr = LUA.luaL_testudata(L, ud, tname);

                if (ptr == null) goto L_destroyed;

                opaque_handle = new IntPtr(ptr);
            }

            GCHandle handle = GCHandle.FromIntPtr(opaque_handle);
            if (!handle.IsAllocated) goto L_destroyed;

            return (T)handle.Target;

L_destroyed:
            unsafe {
                // this function should never return
                LUA.luaL_argerror(this.L, ud, $"{tname} object was disposed.");
            }
            return null;
        }

        public bool lua_isnil(int n) {
            unsafe {
                return LUA.lua_isnil(L, n);
            }
        }

        public string luaL_checkstring(int n) {
            unsafe {
                return LUA.luaL_checkstring(L, n);
            }
        }

        public void lua_pushlstring(byte[] s, int len) {
            unsafe {
                fixed (byte* ptr = s) LUA.lua_pushlstring(L, ptr, (IntPtr)len);
            }
        }

        public void lua_pushvalue(int idx) {
            unsafe {
                LUA.lua_pushvalue(L, idx);
            }
        }

        public long luaL_checkinteger(int arg) {
            unsafe {
                return LUA.luaL_checkinteger(L, arg);
            }
        }

        public double luaL_checknumber(int arg) {
            unsafe {
                return LUA.luaL_checknumber(L, arg);
            }
        }

        public int lua_gettop() {
            unsafe {
                return LUA.lua_gettop(L);
            }
        }

    }
}

