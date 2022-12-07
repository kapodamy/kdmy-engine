using System.Runtime.InteropServices;

namespace Engine.Externals.LuaInterop {

    public sealed class LuaTableFunction {
        public readonly string name;
        public readonly LuaCallback func;

        public LuaTableFunction(string name, LuaCallback func) {
            this.name = name;
            this.func = func;
        }
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