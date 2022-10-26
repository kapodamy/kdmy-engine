using System;
using LuaNativeMethods;

namespace Engine.Externals.LuaInterop {
    public class LuaTableBuilder {

        private readonly Field[] fields;
        private int size;


        public LuaTableBuilder(int max_field_count) {
            fields = new Field[max_field_count];
        }


        public void AddNumber(string name, double n) {
            fields[size].name = name;
            fields[size].value = n;
            fields[size].kind = 0;
            size++;
        }

        public void AddInteger(string name, long i) {
            fields[size].name = name;
            fields[size].value = i;
            fields[size].kind = 1;
            size++;
        }

        public void AddString(string name, string s) {
            fields[size].name = name;
            fields[size].value = s;
            fields[size].kind = 2;
            size++;
        }

        public void AddBoolean(string name, bool b) {
            fields[size].name = name;
            fields[size].value = b;
            fields[size].kind = 3;
            size++;
        }

        public void AddNil(string name) {
            fields[size].name = name;
            fields[size].value = null;
            fields[size].kind = 4;
            size++;
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

        private struct Field {
            public string name;
            public object value;
            public int kind;
        }

    }

}
