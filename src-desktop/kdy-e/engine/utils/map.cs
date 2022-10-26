using System;
using System.Collections;
using System.Collections.Generic;

namespace Engine.Utils {
    public class Map<T> {

        private readonly Dictionary<int, T> table;

        public Map() {
            table = new Dictionary<int, T>(16);
        }


        public void Set(int id, T obj) {
            table.Add(id, obj);
        }

        public void Delete(int id) {
            table.Remove(id);
        }

        public bool Has(int id) {
            return table.ContainsKey(id);
        }

        public void Delete(T obj) {
            foreach (KeyValuePair<int, T> keyValuePair in table) {
                if ((keyValuePair.Value == null && obj == null) || keyValuePair.Value.Equals(obj)) {
                    table.Remove(keyValuePair.Key);
                    return;
                }
            }
        }

        public IEnumerator GetEnumerator() {
            return new MapEnumerator(table);
        }


        private class MapEnumerator : IEnumerator, IDisposable {
            private Dictionary<int, T> table;
            private Dictionary<int, T>.ValueCollection.Enumerator values;

            public MapEnumerator(Dictionary<int, T> table) {
                this.table = table;
                Reset();
            }


            public bool MoveNext() {
                return this.values.MoveNext();
            }

            public object Current {
                get { return values.Current; }
            }

            public void Reset() {
                this.values.Dispose();
                this.values = table.Values.GetEnumerator();
            }

            public void Dispose() {
                this.values.Dispose();
            }
        }

    }
}
