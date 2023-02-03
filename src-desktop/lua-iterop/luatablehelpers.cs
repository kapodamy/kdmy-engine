using LuaNativeMethods;

namespace Engine.Externals.LuaInterop {

    public class LuaTableHelpers {

        public static long GetFieldInteger(LuaState L, int idx, string name, long def_value) {
            L.lua_getfield(idx, name);
            long ret = L.luaL_optinteger(L.lua_gettop(), def_value);
            L.lua_pop(1);
            return ret;
        }

        public static string GetFieldString(LuaState L, int idx, string name, string def_value) {
            L.lua_getfield(idx, name);
            string ret = L.luaL_optstring(L.lua_gettop(), def_value);
            L.lua_pop(1);
            return ret;
        }

        public static double GetFieldNumber(LuaState L, int idx, string name, double def_value) {
            L.lua_getfield(idx, name);
            double ret = L.luaL_optnumber(L.lua_gettop(), def_value);
            L.lua_pop(1);
            return ret;
        }

        public static bool GetFieldBoolean(LuaState L, int idx, string name, bool def_value) {
            LuaType type = L.lua_getfield(idx, name);

            bool ret;
            if (type == LuaType.TNONE)
                ret = def_value;
            else
                ret = L.lua_toboolean(L.lua_gettop());

            L.lua_pop(1);
            return ret;
        }

        public static bool GetFieldTable(LuaState L, int idx, string name) {
            LuaType type = L.lua_getfield(idx, name);
            if (type == LuaType.TTABLE) return true;

            L.lua_pop(1);
            return false;
        }

        public static bool GetArrayItemAsTable(LuaState L, int idx, int index_in_table) {
            // this performs "local item = array[index_in_table]"
            L.lua_pushinteger(index_in_table);
            LuaType type = L.lua_gettable(idx);

            if (type == LuaType.TTABLE) return true;

            L.lua_pop(1);
            return false;
        }

        public static bool HasTableField(LuaState L, int idx, string name, LuaType desired_type) {
            LuaType type = L.lua_getfield(idx, name);
            L.lua_pop(1);
            return type == desired_type;
        }



        public static void AddFieldInteger(LuaState L, string name, long integer) {
            unsafe {
                L.lua_pushinteger(integer);
                LUA.lua_setfield(L.L, -2, name);
            }
        }

        public static void AddFieldString(LuaState L, string name, string @string) {
            unsafe {
                L.lua_pushstring(@string);
                LUA.lua_setfield(L.L, -2, name);
            }
        }

        public static void AddFieldNumber(LuaState L, string name, double number) {
            unsafe {
                L.lua_pushnumber(number);
                LUA.lua_setfield(L.L, -2, name);
            }
        }

        public static void AddFieldBoolean(LuaState L, string name, bool boolean) {
            unsafe {
                L.lua_pushboolean(boolean);
                LUA.lua_setfield(L.L, -2, name);
            }
        }

        public static void AddFieldNull(LuaState L, string name) {
            unsafe {
                L.lua_pushnil();
                LUA.lua_setfield(L.L, -2, name);
            }
        }

        public static void AddFieldTable(LuaState L, string name, int narr, int nrec) {
            unsafe {
                L.lua_pushstring(name);
                LUA.lua_createtable(L.L, narr, nrec);
            }
        }

        public static void AddFieldArrayItemAsTable(LuaState L, int narr, int nrec, int index_in_table) {
            unsafe {
                L.lua_pushinteger(index_in_table);
                LUA.lua_createtable(L.L, narr, nrec);
            }
        }

    }
}
