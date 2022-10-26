using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using LuaNativeMethods;

namespace Engine.Externals.LuaInterop {
    internal unsafe class UserdataList : IDisposable {

        private readonly List<Entry> table;
        private readonly lua_State* L;


        public UserdataList(lua_State* luastate) {
            table = new List<Entry>();
            L = luastate;
        }

        public void* CreateOrRetrieveUserdata(object obj) {
            if (obj == null) return null;
            if (obj.GetType().IsValueType) return null;


            //
            // Note: This does not work, ¿but why? https://stackoverflow.com/questions/73691623
            //
            /*
            foreach (Entry entry in this.table) {
                if (!entry.reference.IsAllocated) continue;
                if (Object.ReferenceEquals(entry.reference.Target, obj)) {
                    return entry.userdata;
                }
            }
            */

            GCHandle reference = GCHandle.Alloc(obj, GCHandleType.WeakTrackResurrection);
            void** userdata = (void**)LUA.lua_newuserdata(L, sizeof(void**));

            Debug.Assert(userdata != null, "Error: lua_newuserdata() returned NULL");

            this.table.Add(new Entry() {
                reference = reference,
                userdata = userdata
            });

            *userdata = GCHandle.ToIntPtr(reference).ToPointer();

            return userdata;
        }

        public object DeleteUserdata(void* userdata) {
            int index = 0;
            object obj = null;

            foreach (Entry entry in this.table) {
                if (entry.userdata != userdata) {
                    index++;
                    continue;
                }

                if (entry.reference.IsAllocated) {
                    obj = entry.reference.Target;
                    entry.reference.Free();
                }

                this.table.RemoveAt(index);
                break;
            }

            return obj;
        }

        public void Clear() {
            foreach (Entry entry in this.table) {
                //
                // Important: do not null the userdata pointer (*entry.userdata = null)
                //            the pointer can be an allocated object and nulling it
                //            creates a memory leak
                //
                if (entry.reference.IsAllocated) entry.reference.Free();
            }
            this.table.Clear();
        }

        public void Dispose() {
            Clear();
        }


        private class Entry {
            public GCHandle reference;
            public void** userdata;
        }

    }
}
