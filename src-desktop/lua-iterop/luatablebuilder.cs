using System;
using LuaNativeMethods;

namespace Engine.Externals.LuaInterop {
    public class LuaTableBuilder {

        private Field[] fields;
        private int size;


        public LuaTableBuilder() : this(16) { }

        public LuaTableBuilder(int initial_field_count) {
            fields = new Field[initial_field_count + 1];
        }


        public void AddNumber(string name, double n) {
            int index = CheckSize();
            fields[index].name = name;
            fields[index].value = n;
            fields[index].kind = 0;
        }

        public void AddInteger(string name, long i) {
            int index = CheckSize();
            fields[index].name = name;
            fields[index].value = i;
            fields[index].kind = 1;
        }

        public void AddString(string name, string s) {
            int index = CheckSize();
            fields[index].name = name;
            fields[index].value = s;
            fields[index].kind = 2;
        }

        public void AddBoolean(string name, bool b) {
            int index = CheckSize();
            fields[index].name = name;
            fields[index].value = b;
            fields[index].kind = 3;
        }

        public void AddNil(string name) {
            int index = CheckSize();
            fields[index].name = name;
            fields[index].value = null;
            fields[index].kind = 4;
        }

        public int FieldsCount { get => size; }

        public void PushTable(LuaState luaState) {
            unsafe {
                lua_State* L = luaState.L;

                LUA.lua_createtable(L, 0, size);

                for (int i = 0 ; i < size ; i++) {

                    LUA.lua_pushstring(L, fields[i].name);
                    switch (fields[i].kind) {
                        case 0:
                            LUA.lua_pushnumber(L, (double)fields[i].value);
                            break;
                        case 1:
                            LUA.lua_pushinteger(L, (long)fields[i].value);
                            break;
                        case 2:
                            LUA.lua_pushstring(L, (string)fields[i].value);
                            break;
                        case 3:
                            LUA.lua_pushboolean(L, (bool)fields[i].value ? 1 : 0);
                            break;
                        case 4:
                            LUA.lua_pushnil(L);
                            break;
                        default:
                            throw new Exception("Unexpected type");
                    }

                    LUA.lua_settable(L, -3);
                }
            }
        }

        private int CheckSize() {
            if (size >= fields.Length) {
                Array.Resize(ref fields, size + 15);
            }
            int index = size;
            size++;
            return index;
        }

        private struct Field {
            public string name;
            public object value;
            public int kind;
        }

    }

}
