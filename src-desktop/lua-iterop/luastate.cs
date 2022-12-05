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

                void* ptr = LuaInteropHelpers.luascript_read_userdata(L, metatable_name);
                GCHandle handle = GCHandle.FromIntPtr((IntPtr)ptr);

                if (handle.IsAllocated) return (T)handle.Target;

                LUA.luaL_error(this.L, $"{metatable_name} object was disposed by the engine.");
                return null;
            }
        }

        public T ReadNullableUserdata<T>(int idx, string metatable_name) where T : class {
            unsafe {
                void* ptr = LuaInteropHelpers.luascript_read_nullable_userdata(L, idx, metatable_name);
                GCHandle handle = GCHandle.FromIntPtr((IntPtr)ptr);

                if (handle.IsAllocated) return (T)handle.Target;

                LUA.luaL_error(this.L, $"{metatable_name} object was disposed by the engine.");
                return null;
            }
        }

        public int CreateUserdata<T>(string metatable_name, T obj) where T : class {
            unsafe {
                Debug.Assert(this.L != null);
                return LuaInteropHelpers.luascript_create_userdata(this.Self, obj, metatable_name, false);
            }
        }

        public int CreateAllocatedUserdata<T>(string metatable_name, T obj) where T : class {
            unsafe {
                Debug.Assert(this.L != null);
                return LuaInteropHelpers.luascript_create_userdata(this.Self, obj, metatable_name, true);
            }
        }

        public int ToString_userdata(string metatable_name) {
            unsafe {
                return LuaInteropHelpers.luascript_userdata_tostring(this.L, metatable_name);
            }
        }

        public int GC_userdata(string metatable_name) {
            unsafe {
                return LuaInteropHelpers.luascript_userdata_gc(this.Self, metatable_name);
            }
        }

        public bool IsUserdataAllocated(string metatable_name) {
            unsafe {
                return LuaInteropHelpers.luascript_userdata_is_allocated(this.L, metatable_name);
            }
        }

        public int DestroyUserdata(string metable_name) {
            unsafe {
                return LuaInteropHelpers.luascript_userdata_destroy(this.Self, metable_name);
            }
        }


        public ManagedLuaState Self {
            get
            {
                unsafe { return LuaInteropHelpers.luascript_get_instance(this.L); }
            }
        }

        public object Context {
            get
            {
                return this.Self.context;
            }
        }


        public string luaL_optstring(int arg, string d) {
            unsafe {
                return LUA.luaL_optstring(L, arg, d);
            }
        }

        public double luaL_optnumber(int arg, double def) {
            unsafe {
                double value = LUA.luaL_optnumber(L, arg, def);
                return value;
            }
        }

        public bool luaL_toboolean(int arg) {
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

