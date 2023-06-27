using System;

namespace Engine.Utils;

public class NaNArgumentError : Exception {
    public NaNArgumentError() : base() { }
    public NaNArgumentError(string message) : base(message) { }
}

public static class CloneUtils {

    public static T[] CloneArray<T>(T[] array, int elements) {
        if (array == null) return null;

        T[] copy = new T[elements];
        Array.Copy(array, copy, elements);

        return array;
    }

}
