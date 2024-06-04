using System;
using System.Collections;

namespace Engine.Utils;

public class ArrayList<T> {

    private const int DEFAULT_CAPACITY = 16;
    private const int LENGTH_MULTIPLE = 8;

    private int length;
    private int size;
    private T[] array;
    private ArrayListComparer comparer;

    public ArrayList() : this(ArrayList<T>.DEFAULT_CAPACITY) { }

    public ArrayList(int initial_capacity) {
        if (initial_capacity < 1) initial_capacity = ArrayList<T>.DEFAULT_CAPACITY;

        this.array = new T[initial_capacity];
        this.length = initial_capacity;
        this.size = 0;
        this.comparer = new ArrayListComparer();
    }

    public void Destroy(bool keep_array_alive) {
        if (!keep_array_alive) this.array = null;
        //free(arraylist);
    }

    public void Destroy2(out int size_ptr, ref T[] array_ptr) {
        size_ptr = this.size;

        if (this.size < 1) {
            array_ptr = null;
        } else {
            this.Trim();
            array_ptr = this.array;
        }

        this.array = null;
        //free(arraylist);
    }

    public void Destroy3(Action<T> free_function) {
        for (int i = 0 ; i < this.size ; i++) {
            if (this.array[i] != null) free_function(this.array[i]);
        }
        //free(this.array);
        //free(arraylist);
    }


    public int Size() {
        return this.size;
    }


    public T Get(int index) {
        if (index < 0 || index >= this.size)
            throw new ArgumentOutOfRangeException("arraylist_get() index of bounds size index=" + index + " size=" + this.size);

        return this.array[index];
    }

    public T Add(T item) {
        int size = this.size + 1;

        if (size > this.length) {
            this.length = size + (LENGTH_MULTIPLE - (size % LENGTH_MULTIPLE));
            Array.Resize<T>(ref this.array, this.length);
        }

        this.array[this.size] = item;
        this.size = size;

        //
        // Note:
        //      if "T" is struct, this should return "ref this.array[this.size]" or make
        //      an "ArrayList" exclusive for structs allow the same behavior like "class"
        //

        return item;
    }

    public void Set(int index, T item) {
        if (index < 0 || index >= this.size)
            throw new ArgumentOutOfRangeException("arraylist_set() index of bounds size index=" + index + " size=" + this.size);

        this.array[index] = item;
    }

    public void Clear() {
        this.size = 0;
        for (int i = 0 ; i < this.length ; i++) this.array[i] = default(T);
    }

    public int Trim() {
        if (this.size == this.length) return this.size;

        if (this.size < 1)
            this.length = LENGTH_MULTIPLE;
        else
            this.length = this.size;

        Array.Resize<T>(ref this.array, this.length);

        return this.size;
    }

    public T[] PeekArray() {
        return this.array;
    }

    public int IndexOf(T item) {
        for (int i = 0 ; i < this.size ; i++) {
            if ((this.array[i] == null && item == null) || this.array[i].Equals(item))
                return i;
        }
        return -1;
    }

    public bool Has(T item) {
        for (int i = 0 ; i < this.size ; i++) {
            if ((this.array[i] == null && item == null) || this.array[i].Equals(item))
                return true;
        }
        return false;
    }

    public int Remove(T item) {
        int count = 0;
        int j = 0;

        for (int i = 0 ; i < this.size ; i++) {
            if ((this.array[i] == null && item == null) || this.array[i].Equals(item)) {
                count++;
                continue;
            }
            if (j != i) this.array[j] = this.array[i];
            j++;
        }

        this.size = j;
        return count;
    }

    public bool RemoveAt(int index) {
        if (index < 0 || index >= this.size) return false;

        int j = index;
        for (int i = index + 1 ; i < this.size ; i++, j++) {
            this.array[j] = this.array[i];
        }

        this.size = j;
        return true;
    }

    public void CutSize(int new_size) {
        if (new_size < this.size) this.size = Math.Max(new_size, 0);
    }

    public ArrayList<T> Clone() {
        Type type = typeof(T);

        ArrayList<T> copy = new ArrayList<T>(this.size);
        copy.size = this.size;

        if (typeof(ICloneable).IsAssignableFrom(typeof(T))) {
            for (int i = 0 ; i < this.size ; i++) {
                copy.array[i] = (T)CloneUtils.CloneObject((ICloneable)this.array[i]);
            }
        } else {
            for (int i = 0 ; i < this.size ; i++) {
                copy.array[i] = this.array[i];
            }
        }

        return copy;
    }

    public void Sort(SortDelegate sort_fn) {
        this.comparer.sort_fn = sort_fn;
        Array.Sort(this.array, 0, this.size, this.comparer);
        this.comparer.sort_fn = null;
    }


    public System.Collections.Generic.IEnumerator<T> GetEnumerator() {
        return new ArrayListEnumerator(this);
    }


    private struct ArrayListEnumerator : System.Collections.Generic.IEnumerator<T> {
        private readonly ArrayList<T> arraylist;
        private int index;

        public ArrayListEnumerator(ArrayList<T> arraylist) {
            this.arraylist = arraylist;
            this.index = -1;
        }


        public bool MoveNext() {
            this.index++;
            return this.index < arraylist.size;
        }

        public void Reset() {
            this.index = -1;
        }

        void IDisposable.Dispose() {

        }


        object System.Collections.IEnumerator.Current => this.arraylist.array[this.index];

        public T Current => this.arraylist.array[this.index];

    }

    private struct ArrayListComparer : System.Collections.Generic.IComparer<T> {

        public SortDelegate sort_fn;

        public int Compare(T x, T y) {
            return sort_fn(x, y);
        }
    }


    public delegate int SortDelegate(T obj_a, T obj_b);

}