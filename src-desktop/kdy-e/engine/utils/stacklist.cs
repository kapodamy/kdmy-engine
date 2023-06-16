using System;

namespace Engine.Utils;

public class StackList<T> {
    private T[] array;
    private int length;

    public StackList() : this(16) {
    }

    public StackList(int initialCapacity) {
        this.array = new T[initialCapacity];
        this.length = 0;
    }

    public int Length { get => this.length; }

    public int Capacity { get => array.Length; }

    public void Push(T item) {
        int index = this.length;
        this.length++;
        if (this.length > array.Length) Array.Resize(ref this.array, this.array.Length + 16);
        array[index] = item;
    }

    public T Pop() {
        T item;
        if (this.length < 1) {
            item = default(T);
        } else {
            length--;
            item = array[length];
            array[length] = default(T);
        }
        return item;
    }

    public void Clear() {
        if (this.length < 1) return;
        for (int i = 0 ; i < this.length ; i++) this.array[i] = default(T);
        this.length = 0;
    }

    public void TrimExcess() {
        if (this.length > this.array.Length) return;
        Array.Resize(ref this.array, this.length);
    }

    public T this[int index] {
        get
        {
            if (index < 0 || index >= this.length) throw new ArgumentOutOfRangeException("index");
            return this.array[index];
        }
    }

}
