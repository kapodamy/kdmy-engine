using System;
using System.Collections;
using System.Collections.Generic;

namespace Engine.Utils;

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

    public IEnumerator<T> GetEnumerator() {
        return new MapEnumerator(table);
    }


    private struct MapEnumerator : IEnumerator<T> {
        private Dictionary<int, T>.Enumerator values;

        public MapEnumerator(Dictionary<int, T> table) {
            this.values = table.GetEnumerator();
        }


        public bool MoveNext() {
            return this.values.MoveNext();
        }

        void IEnumerator.Reset() {
            ((IEnumerator)this.values).Reset();
        }

        void IDisposable.Dispose() {
            this.values.Dispose();
        }


        object IEnumerator.Current => values.Current.Value;

        public T Current => values.Current.Value;

    }

}
