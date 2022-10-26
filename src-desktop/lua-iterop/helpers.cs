using System;
using System.Runtime.InteropServices;
using LuaNativeMethods;

namespace Engine.Externals.LuaInterop {

    public sealed class LuaTableFunction {
        public string name;
        public LuaCallback func;
    }

    public struct LuaIntegerConstant {
        public string variable;
        public long value;
    }

    public struct LuaStringConstant {
        public string variable;
        public string value;
    }


    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate int LuaCallback(LuaState lua);

}