using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using LuaNativeMethods;

namespace Engine.Externals.LuaInterop;

internal unsafe struct LuascriptObject {
    internal void* obj_ptr;
    internal int lua_ref;
    internal bool was_allocated_by_lua;
}

internal static unsafe class LuaInteropHelpers {

    public const int SHARED_ARRAY_CHUNK_SIZE = 16;

    private static lua_CFunction delegate_handle_error = luascript_handle_error;
    public static lua_CFunction delegate_print = luascript_print;
    public static lua_WarnFunction delegate_print_warning = luascript_print_warning;
    private static readonly object key_object;
    public static readonly void* luascript_key_ptr;

    static LuaInteropHelpers() {
        key_object = new object();
        GCHandle handle = GCHandle.Alloc(key_object, GCHandleType.Normal);
        luascript_key_ptr = GCHandle.ToIntPtr(handle).ToPointer();
    }


    internal static int get_lua_reference(ManagedLuaState luascript, void* obj) {
        LuascriptObject* shared_array = luascript.shared_array;
        int shared_size = luascript.shared_size;

        for (int i = 0 ; i < shared_size ; i++) {
            if (shared_array[i].obj_ptr == obj) {
                return shared_array[i].lua_ref;
            }
        }

        return LUA.NOREF;
    }

    internal static LuascriptObject* add_lua_reference(ManagedLuaState luascript, void* obj, int @ref, bool allocated) {
        int index;

        // find and empty slot
        LuascriptObject* shared_array = luascript.shared_array;
        int shared_size = luascript.shared_size;

        for (int i = 0 ; i < shared_size ; i++) {
            if (shared_array[i].obj_ptr == null) {
                index = i;
                goto L_store_and_return;
            }
        }

        // no empty slots, grow the array and add it
        index = luascript.shared_size;
        luascript.shared_size += SHARED_ARRAY_CHUNK_SIZE;

        int new_size = luascript.shared_size * sizeof(LuascriptObject);
        luascript.shared_array = (LuascriptObject*)Marshal.ReAllocHGlobal((nint)luascript.shared_array, new_size);
        Debug.Assert(luascript.shared_array != null, "reallocation failed");

        for (int i = 0 ; i < SHARED_ARRAY_CHUNK_SIZE ; i++) {
            LuascriptObject* entry = &luascript.shared_array[i + index];
            entry->lua_ref = LUA.NOREF;
            entry->obj_ptr = null;
            entry->was_allocated_by_lua = false;
        }

L_store_and_return:
        luascript.shared_array[index].obj_ptr = obj;
        luascript.shared_array[index].lua_ref = @ref;
        luascript.shared_array[index].was_allocated_by_lua = allocated;

        return &luascript.shared_array[index];
    }

    internal static int remove_lua_reference(ManagedLuaState luascript, void* obj) {
        LuascriptObject* shared_array = luascript.shared_array;
        int shared_size = luascript.shared_size;

        for (int i = 0 ; i < shared_size ; i++) {
            if (shared_array[i].obj_ptr == obj) {
                int @ref = shared_array[i].lua_ref;
                shared_array[i].obj_ptr = null;
                shared_array[i].lua_ref = LUA.REFNIL;
                shared_array[i].was_allocated_by_lua = false;
                return @ref;
            }
        }

        return LUA.NOREF;
    }
    static LuascriptObject* read_luascript_object(lua_State* L, int ud, string tname) {
        LuascriptObject* udata = (LuascriptObject*)LUA.luaL_checkudata(L, ud, tname);

        if (udata == null || udata->obj_ptr == null || udata->lua_ref == LUA.REFNIL || udata->lua_ref == LUA.NOREF) {
            return null;
        }

        return udata;
    }

    static void* read_userdata(lua_State* L, int ud, string tname) {
        LuascriptObject* udata = read_luascript_object(L, ud, tname);

        if (udata == null) {
            LUA.luaL_error(L, $"{tname} object was destroyed.");
            return null;
        }

        return udata->obj_ptr;
    }

    /*internal static int is_userdata_equals(lua_State* L) {
        bool equals;

        int type_a = LUA.lua_type(L, 1);
        int type_b = LUA.lua_type(L, 2);

        if (type_a != LUA.TUSERDATA || type_b != LUA.TUSERDATA) {
            equals = false;
        } else {
            void* a = LUA.lua_touserdata(L, 1);
            void* b = LUA.lua_touserdata(L, 2);

            equals = (a == null && b == null) || (a != b);
        }

                LUA.lua_pushboolean(L, equals ? 1 : 0);

        return 1;
    }*/



    internal static int luascript_create_userdata(ManagedLuaState luascript, object obj, string metatable_name, bool allocated) {
        lua_State* L = luascript.L;

        if (obj == null) {
            LUA.lua_pushnil(L);
            return 1;
        }

        void* obj_ptr = luascript.handle_references.AddReference(obj, allocated);

        int @ref = get_lua_reference(luascript, obj_ptr);
        if (@ref != LUA.NOREF) {
            // recover userdata back into stack
            LUA.lua_pushinteger(L, @ref);
            LUA.lua_gettable(L, LUA.REGISTRYINDEX);
        } else {
            LuascriptObject* udata = (LuascriptObject*)LUA.lua_newuserdata(L, sizeof(LuascriptObject));
            Debug.Assert(udata != null, "can not create userdata");

            LUA.luaL_getmetatable(L, metatable_name);
            LUA.lua_setmetatable(L, -2);

            LUA.lua_pushvalue(L, -1);
            @ref = LUA.luaL_ref(L, LUA.REGISTRYINDEX);

            udata->lua_ref = @ref;
            udata->obj_ptr = obj_ptr;
            udata->was_allocated_by_lua = allocated;

            add_lua_reference(luascript, obj_ptr, @ref, allocated);
        }

        return 1;
    }

    internal static void luascript_remove_userdata(ManagedLuaState luascript, void* obj) {
        Debug.Assert(obj != null);

        lua_State* L = luascript.L;

        // remove object from shared array
        int @ref = remove_lua_reference(luascript, obj);
        if (@ref == LUA.NOREF) return;

        // adquire lua userdata
        LUA.lua_pushinteger(L, @ref);
        LUA.lua_gettable(L, LUA.REGISTRYINDEX);

        // nullify userdata, this is already in the stack if was called by lua GC
        LuascriptObject* udata = (LuascriptObject*)LUA.lua_touserdata(L, LUA.lua_gettop(L));
        if (udata != null) {
            udata->lua_ref = LUA.REFNIL;
            udata->obj_ptr = null;
            udata->was_allocated_by_lua = false;
        }
        LUA.lua_pop(L, 1);

        // remove from lua registry
        LUA.luaL_unref(L, LUA.REGISTRYINDEX, @ref);
    }


    internal static void* luascript_read_userdata(lua_State* L, string check_metatable_name) {
        if (LUA.lua_isnil(L, 1)) {
            LUA.luaL_error(L, $"{check_metatable_name} was null (nil in lua).");
            return null;
        }

        return read_userdata(L, 1, check_metatable_name);
    }

    internal static void* luascript_read_nullable_userdata(lua_State* L, int idx, string check_metatable_name) {
        if (LUA.lua_isnil(L, idx))
            return null;
        else
            return read_userdata(L, idx, check_metatable_name);
    }


    internal static ManagedLuaState luascript_get_instance(lua_State* L) {
        LUA.lua_pushlightuserdata(L, luascript_key_ptr);
        LUA.lua_gettable(L, LUA.REGISTRYINDEX);

        void* ptr = LUA.lua_touserdata(L, -1);

        GCHandle handle = GCHandle.FromIntPtr((nint)ptr);
        ManagedLuaState luascript = (ManagedLuaState)handle.Target;

        Debug.Assert(luascript != null);
        LUA.lua_pop(L, 1);

        return luascript;
    }

    internal static void luascript_set_instance(ManagedLuaState luascript) {
        lua_State* L = luascript.L;
        void* ptr = GCHandle.ToIntPtr(luascript.self).ToPointer();

        LUA.lua_pushlightuserdata(L, luascript_key_ptr);
        LUA.lua_pushlightuserdata(L, ptr);
        LUA.lua_settable(L, LUA.REGISTRYINDEX);
    }


    internal static int luascript_userdata_tostring(lua_State* L, string check_metatable_name) {
        void* udata = luascript_read_userdata(L, check_metatable_name);
        string ptr = new IntPtr(udata).ToString();
        LUA.lua_pushstring(L, $"{{{check_metatable_name} {ptr}}}");
        return 1;
    }

    internal static int luascript_userdata_gc(ManagedLuaState luascript, string check_metatable_name) {
        lua_State* L = luascript.L;
        LuascriptObject* udata = (LuascriptObject*)LUA.luaL_checkudata(L, 1, check_metatable_name);

        if (udata != null && (udata->lua_ref != LUA.REFNIL && udata->lua_ref != LUA.NOREF)) {
            void* obj = udata->obj_ptr;
            luascript_remove_userdata(luascript, obj);
        }

        return 0;
    }

    internal static int luascript_userdata_destroy(ManagedLuaState luascript, string check_metatable_name) {
        lua_State* L = luascript.L;
        LuascriptObject* udata = read_luascript_object(L, 1, check_metatable_name);

        if (udata == null) return 0;

        void* obj_ptr = udata->obj_ptr;
        bool was_allocated_by_lua = udata->was_allocated_by_lua;
        LUA.lua_pop(L, 1);

        luascript_remove_userdata(luascript_get_instance(L), obj_ptr);

        if (was_allocated_by_lua) {
            GCHandle handle = GCHandle.FromIntPtr((nint)obj_ptr);
            if (handle.IsAllocated) {
                object obj = handle.Target;

                if (obj is IDisposable disposable) {
                    disposable.Dispose();
                } else {
                    Action destructor = (Action)Delegate.CreateDelegate(typeof(Action), obj, "Destroy");
                    destructor();
                }
            }
        }

        return 0;
    }

    internal static bool luascript_userdata_is_allocated(lua_State* L, string check_metatable_name) {
        LuascriptObject* udata = read_luascript_object(L, 1, check_metatable_name);

        if (udata == null) return false;

        return udata->was_allocated_by_lua;
    }


    internal static int luascript_handle_error(lua_State* L) {
        string msg = LUA.lua_tostring(L, -1);
        LUA.luaL_traceback(L, L, msg, 2);
        LUA.lua_remove(L, -2);

        // keep the error (message & traceback)  in the stack
        return 1;
    }

    internal static int luascript_pcallk(lua_State* L, int arguments_count, int results_count) {
        // push error handler
        LUA.lua_pushcfunction(L, delegate_handle_error);
        LUA.lua_insert(L, 1);

        int result = LUA.lua_pcallk(L, arguments_count, results_count, 1, 0, null);

        // remove error handler
        LUA.lua_remove(L, 1);

        return result;
    }

    internal static void luascript_print_warning(void* ud, char* msg, int tocont) {
        if (msg == null) {
            if (tocont != 0) Console.WriteLine();
            return;
        }

        string str = Marshal.PtrToStringUTF8((nint)msg);

        if (tocont != 0)
            Console.Write(str);
        else
            Console.WriteLine(str);
    }

    internal static int luascript_print(lua_State* L) {
        int args_count = LUA.lua_gettop(L);
        StringBuilder message = new StringBuilder(128);

        for (int i = 1 ; i <= args_count ; i++) {
            nint len;
            char* str = LUA.luaL_tolstring(L, i, &len);

            // lua adds a tab character between each argument
            if (i > 1) message.Append('\t');

            message.Append(LUA.MarshalStringBack(str, len));
            LUA.lua_pop(L, 1);
        }

        Console.WriteLine(message.ToString());
        return 0;
    }

}

