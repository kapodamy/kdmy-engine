using System;

namespace Engine.Utils;

public class NaNArgumentError : Exception {
    public NaNArgumentError() : base() { }
    public NaNArgumentError(string message) : base(message) { }
}

public static class CloneUtils {

    public static T CloneObject<T>(T obj) where T : class, ICloneable {
        if (obj == null) return null;
        return (T)obj.Clone();
    }

    public static T[] CloneStructArray<T>(T[] array, int elements) where T : struct {
        if (array == null) return null;
        if (array.Length < elements) throw new ArgumentOutOfRangeException("array");

        T[] copy = new T[elements];
        Array.Copy(array, 0, copy, 0, elements);

        return copy;
    }

    public static T[] CloneClassArray<T>(T[] array, int elements) where T : class, ICloneable {
        if (array == null) return null;
        if (array.Length < elements) throw new ArgumentOutOfRangeException("array");

        T[] copy = new T[elements];

        for (int i = 0 ; i < elements ; i++) {
            copy[i] = (T)array[i].Clone();
        }

        return copy;
    }

}

public static class ArrayUtils {

    public static void Sort<T>(T[] array, int index, int length, Comparison<T> comparison) {
        if (length < 1) return;
        Span<T> span = new Span<T>(array, index, length);
        span.Sort(comparison);
    }

}
