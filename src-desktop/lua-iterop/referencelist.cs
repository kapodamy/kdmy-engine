using System;
using System.Collections.Generic;

namespace Engine.Externals.LuaInterop {

    internal unsafe class ReferenceList : IDisposable {

        private readonly List<object> list;

        public ReferenceList() {
            list = new List<object>();
        }

        public void AddIfNotExists(object obj) {
            foreach (object entry in list) {
                if (Object.ReferenceEquals(entry, obj)) return;
            }
            list.Add(obj);
        }

        public void Remove(object obj) {
            int index = 0;
            foreach (object entry in list) {
                if (Object.ReferenceEquals(entry, obj)) {
                    list.RemoveAt(index);
                }
                index++;
            }
        }

        public bool Contains(object obj) {
            foreach (object entry in list) {
                if (Object.ReferenceEquals(entry, obj)) return true;
            }
            return false;
        }

        public void Dispose() {
            list.Clear();
        }

    }
}
