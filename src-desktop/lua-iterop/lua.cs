﻿using System;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
using System.Text;

namespace LuaNativeMethods;

using lua_Integer = Int64;
using lua_Number = Double;

[StructLayout(LayoutKind.Sequential)]
internal struct luaL_Reg {
    public string name;
    public Delegate func;
}

internal struct lua_State { }
internal struct lua_KFunction { }

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal unsafe delegate int lua_CFunction(lua_State* L);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal unsafe delegate void lua_WarnFunction(void* ud, char* msg, int tocont);



internal unsafe static class LUA {

    public const int MULTRET = -1;
    public const int OK = 0;
    public const int YIELD = 1;

    public const int TNONE = -1;
    public const int TNIL = 0;
    public const int TBOOLEAN = 1;
    public const int TLIGHTUSERDATA = 2;
    public const int TNUMBER = 3;
    public const int TSTRING = 4;
    public const int TTABLE = 5;
    public const int TFUNCTION = 6;
    public const int TUSERDATA = 7;
    public const int TTHREAD = 8;

    public const int LUAI_MAXSTACK = 1000000;
    public const int REGISTRYINDEX = -LUAI_MAXSTACK - 1000;

    public const int NOREF = -2;
    public const int REFNIL = -1;

    public const int ERRRUN = 2;
    public const int ERRSYNTAX = 3;
    public const int ERRMEM = 4;
    public const int ERRERR = 5;

    private const string DLL = "lua";// "lua.dll"

    private static byte* STRING_BUFFER;
    private static int STRING_BUFFER_LENGTH = 256;

    static LUA() {
        STRING_BUFFER = (byte*)NativeMemory.Alloc((nuint)STRING_BUFFER_LENGTH);
    }


    public static string lua_tostring(lua_State* L, int i) {
        nint len;
        char* ptr = lua_tolstring(L, i, &len);

        return MarshalStringBack(ptr, len);
    }

    public static void lua_remove(lua_State* L, int idx) {
        lua_rotate(L, idx, -1);
        lua_pop(L, 1);
    }

    public static void lua_pop(lua_State* L, int n) {
        lua_settop(L, -n - 1);
    }

    public static int lua_pcall(lua_State* L, int n, Int32 r, Int32 f) {
        return lua_pcallk(L, n, r, f, 0, null);
    }

    public static int luaL_dostring(lua_State* L, string s) {
        int ret = luaL_loadstring(L, s);
        if (ret != 0) return ret;
        return lua_pcall(L, 0, LUA.MULTRET, 0);
    }

    public static void lua_newtable(lua_State* L) {
        lua_createtable(L, 0, 0);
    }

    public static void lua_pushliteral(lua_State* L, string s) {
        lua_pushstring(L, s);
    }

    public static void lua_pushcfunction(lua_State* L, Delegate f) {
        nint ptr = f == null ? 0x00 : Marshal.GetFunctionPointerForDelegate(f);
        lua_pushcclosure(L, ptr, 0);
    }

    public static void* lua_newuserdata(lua_State* L, int s) {
        return lua_newuserdatauv(L, s, 1);
    }

    public static int luaL_getmetatable(lua_State* L, string n) {
        return lua_getfield(L, LUA.REGISTRYINDEX, n);
    }

    public static string luaL_optstring(lua_State* L, int arg, string d) {
        nint len;
        char* ptr = luaL_optlstring(L, arg, null, &len);

        return MarshalStringBack(ptr, len);
    }

    public static int luaL_argerror(lua_State* L, int arg, string extramsg) {
        return luaL_argerror(L, arg, MarshalString(extramsg));
    }

    public static string luaL_checkstring(lua_State* L, int n) {
        nint len;
        char* ptr = luaL_checklstring(L, n, &len);
        return MarshalStringBack(ptr, len);
    }

    public static int lua_upvalueindex(int i) {
        return LUA.REGISTRYINDEX - i;
    }

    public static void luaL_traceback(lua_State* L, lua_State* L1, string msg, int level) {
        luaL_traceback(L, L1, MarshalString(msg), level);
    }

    public static void lua_insert(lua_State* L, int idx) {
        lua_rotate(L, (idx), 1);
    }


    public static bool lua_isfunction(lua_State* L, int n) {
        return lua_type(L, n) == LUA.TFUNCTION;
    }
    public static bool lua_istable(lua_State* L, int n) {
        return lua_type(L, n) == LUA.TTABLE;
    }
    public static bool lua_islightuserdata(lua_State* L, int n) {
        return lua_type(L, n) == LUA.TLIGHTUSERDATA;
    }
    public static bool lua_isnil(lua_State* L, int n) {
        return lua_type(L, n) == LUA.TNIL;
    }
    public static bool lua_isboolean(lua_State* L, int n) {
        return lua_type(L, n) == LUA.TBOOLEAN;
    }
    public static bool lua_isthread(lua_State* L, int n) {
        return lua_type(L, n) == LUA.TTHREAD;
    }
    public static bool lua_isnone(lua_State* L, int n) {
        return lua_type(L, n) == LUA.TNONE;
    }
    public static bool lua_isnoneornil(lua_State* L, int n) {
        return lua_type(L, n) <= 0;
    }
    public static lua_Number lua_tonumber(lua_State* L, int i) {
        return lua_tonumberx(L, i, null);
    }
    public static lua_Integer lua_tointeger(lua_State* L, int i) {
        return lua_tointegerx(L, i, null);
    }



    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern lua_State* luaL_newstate();

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void luaL_openlibs(lua_State* L);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int luaL_loadbufferx(lua_State* L, byte* buff, nint sz, byte* name, char* mode);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_rotate(lua_State* L, int idx, int n);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_settop(lua_State* L, int idx);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int lua_pcallk(lua_State* L, int nargs, int nresults, int errfunc, Int32 ctx, lua_KFunction* k);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_close(lua_State* L);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_pushlightuserdata(lua_State* L, void* p);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_settable(lua_State* L, int idx);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern char* lua_pushstring(lua_State* L, byte* s);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern void lua_setglobal(lua_State* L, byte* name);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_pushinteger(lua_State* L, lua_Integer n);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_pushnumber(lua_State* L, lua_Number n);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int luaL_loadstring(lua_State* L, byte* s);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int lua_getglobal(lua_State* L, byte* name);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_createtable(lua_State* L, int narr, int nrec);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int lua_gettable(lua_State* L, int idx);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void* lua_touserdata(lua_State* L, int idx);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void luaL_setfuncs(lua_State* L, luaL_Reg[] l, int nup);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_pushvalue(lua_State* L, int idx);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int luaL_newmetatable(lua_State* L, byte* tname);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_rawset(lua_State* L, int idx);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int lua_error(lua_State* L);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_pushcclosure(lua_State* L, nint fn, int n);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern void lua_setfield(lua_State* L, int idx, byte* k);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int lua_type(lua_State* L, int idx);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern void* luaL_checkudata(lua_State* L, int ud, byte* tname);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int luaL_error(lua_State* L, byte* fmt);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_pushnil(lua_State* L);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void* lua_newuserdatauv(lua_State* L, nint sz, int nuvalue);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    private static extern int lua_getfield(lua_State* L, int idx, byte* k);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int lua_setmetatable(lua_State* L, int objindex);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern char* lua_tolstring(lua_State* L, int idx, nint* len);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern char* luaL_optlstring(lua_State* L, int arg, char* def, nint* l);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern lua_Number luaL_checknumber(lua_State* L, int arg);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern lua_Number luaL_optnumber(lua_State* L, int arg, lua_Number def);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern lua_Integer luaL_optinteger(lua_State* L, int arg, lua_Integer def);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int lua_toboolean(lua_State* L, int idx);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern lua_Number lua_tonumberx(lua_State* L, int idx, int* isnum);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern lua_Integer lua_tointegerx(lua_State* L, int idx, int* isnum);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_pushboolean(lua_State* L, int b);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int luaL_argerror(lua_State* L, int arg, byte* extramsg);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void* luaL_testudata(lua_State* L, int ud, byte* tname);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern char* luaL_checklstring(lua_State* L, int arg, nint* l);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern char* lua_pushlstring(lua_State* L, byte* s, nint len);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern lua_Integer luaL_checkinteger(lua_State* L, int arg);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int lua_gettop(lua_State* L);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int luaL_ref(lua_State* L, int t);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void luaL_unref(lua_State* L, int t, int @ref);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void luaL_traceback(lua_State* L, lua_State* L1, byte* msg, int level);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern char* luaL_tolstring(lua_State* L, int idx, nint* len);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_setwarnf(lua_State* L, lua_WarnFunction f, void* ud);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern ulong lua_rawlen(lua_State* L, int index);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void lua_rawseti(lua_State* L, int index, int n);


    private static byte* MarshalString(string str) {
        if (str == null) return null;

        int size = Encoding.UTF8.GetByteCount(str) + 1;// + 1 for null terminator

        if (size > STRING_BUFFER_LENGTH) {
            STRING_BUFFER_LENGTH = Math.Min(size, 1024 * 1024);
            STRING_BUFFER = (byte*)NativeMemory.Realloc(STRING_BUFFER, (nuint)STRING_BUFFER_LENGTH);
        }

        Span<byte> span = new Span<byte>(STRING_BUFFER, STRING_BUFFER_LENGTH);
        int written = Encoding.UTF8.GetBytes(str, span);

        // null-terminator
        STRING_BUFFER[written] = 0x00;

        return STRING_BUFFER;
    }

    internal static string MarshalStringBack(char* ptr, nint len) {
        if (ptr == null) return null;

        int size = (int)len;

        if (size < 1)
            return String.Empty;
        else
            return Encoding.UTF8.GetString((byte*)ptr, size);
    }


    public static int lua_getfield(lua_State* L, int idx, string k) {
        return lua_getfield(L, idx, MarshalString(k));
    }

    public static int luaL_error(lua_State* L, string str) {
        //
        // FIMXE: "luaL_error()" uses longjmp and maybe can break "fixed"
        //        and thus creating a memory leak
        //
        return luaL_error(L, MarshalString(str));
    }

    public static void* luaL_checkudata(lua_State* L, int ud, string tname) {
        return luaL_checkudata(L, ud, MarshalString(tname));
    }

    public static void* luaL_testudata(lua_State* L, int ud, string tname) {
        return luaL_testudata(L, ud, MarshalString(tname));
    }

    public static void lua_setfield(lua_State* L, int idx, string k) {
        lua_setfield(L, idx, MarshalString(k));
    }

    public static int luaL_newmetatable(lua_State* L, string tname) {
        return luaL_newmetatable(L, MarshalString(tname));
    }

    public static int lua_getglobal(lua_State* L, string name) {
        return lua_getglobal(L, MarshalString(name));
    }

    public static int luaL_loadstring(lua_State* L, string s) {
        return luaL_loadstring(L, MarshalString(s));
    }

    public static void lua_setglobal(lua_State* L, string name) {
        lua_setglobal(L, MarshalString(name));
    }

    public static void lua_pushstring(lua_State* L, string s) {
        lua_pushstring(L, MarshalString(s));
    }

    public static int luaL_loadbufferx(lua_State* L, string buff, string name) {
        int buff_size = Encoding.UTF8.GetByteCount(buff);
        int name_size = Encoding.UTF8.GetByteCount(name);
        int offset = buff_size + 1;
        byte[] str = new byte[buff_size + name_size + 2];

        Encoding.UTF8.GetBytes(buff, 0, buff.Length, str, 0);
        Encoding.UTF8.GetBytes(name, 0, name.Length, str, offset);

        fixed (byte* str_ptr = str) {
            return luaL_loadbufferx(L, str_ptr, buff_size, str_ptr + offset, null);
        }
    }

}
