using System;

namespace Engine.Utils;

public class NaNArgumentError : Exception {
    public NaNArgumentError() : base() { }
    public NaNArgumentError(string message) : base(message) { }
}

public static class CloneUtils {

    public static T CloneStruct<T>(ref T struct_ptr) where T : struct {
        return struct_ptr;
    }

    public static T[] CloneArray<T>(T[] array_ptr, int elements) {
        if (elements < 1 && array_ptr == null) return null;
        if (array_ptr == null) throw new ArgumentNullException("array_ptr");

        T[] array = new T[elements];
        for (int i = 0 ; i < elements ; i++) array[i] = array_ptr[i];
        return array;
    }

    public static bool CloneStructAsArrayItems<T>(T[] array_ptr, int elements, ref T struct_ptr) where T : struct {
        if (array_ptr == null) return false;

        for (int i = 0 ; i < elements ; i++) array_ptr[i] = struct_ptr;

        return true;
    }

    public static bool CloneStructTo<T>(ref T struct_src, ref T struct_dest) where T : struct {
        struct_src = struct_dest;
        return true;
    }

}
