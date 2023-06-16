using System;
using System.Runtime.InteropServices;
using System.Security;

namespace Engine.Platform;

internal unsafe partial class Memory {

    public readonly static MemoryCopyDelegate Copy;
    public readonly static MemoryClearDelegate Zeros;


    [DllImport("kernel32", SetLastError = false)]
    private static extern void RtlMoveMemory(byte* Destination, byte* Source, int Length);

    internal static unsafe void ZerosInternal(byte* src, long len) {
        uint* ptr = (uint*)src;
        for (int i = 0 ; i < len ; i++) ptr[i] = 0U;
    }


    static Memory() {
        Type type = typeof(Buffer);
        Copy = GetMethodByDelegate<MemoryCopyDelegate>(type, "Memcpy");
        Zeros = GetMethodByDelegate<MemoryClearDelegate>(type, "ZeroMemory");

        if (Copy == null) Copy = RtlMoveMemory;
        if (Zeros == null) Zeros = ZerosInternal;

    }


    [SecurityCritical]
    private static D GetMethodByDelegate<D>(Type target, string name) where D : Delegate {
        try {
            var flags = System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Static;
            var del = typeof(D);
            var delegate_params = del.GetMethod("Invoke").GetParameters();

            var del_args = new Type[delegate_params.Length];
            for (int i = 0 ; i < delegate_params.Length ; i++) del_args[i] = delegate_params[i].ParameterType;

            var method = target.GetMethod(name, flags, null, del_args, null);

            if (method == null)
                return null;
            else
                return (D)Delegate.CreateDelegate(del, method);
        } catch {
            return null;
        }
    }

    public unsafe delegate void MemoryCopyDelegate(byte* dest, byte* src, int len);
    public unsafe delegate void MemoryClearDelegate(byte* src, long len);
}
