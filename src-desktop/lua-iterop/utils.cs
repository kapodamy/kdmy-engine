using System.Runtime.InteropServices;

namespace Engine.Externals.LuaInterop;

public sealed class LuaTableFunction {
    public readonly string name;
    public readonly LuaCallback func;

    public LuaTableFunction(string name, LuaCallback func) {
        this.name = name;
        this.func = func;
    }
}

public struct LuaIntegerConstant {
    public readonly string variable;
    public readonly long value;

    public LuaIntegerConstant(string variable, long value) {
        this.variable = variable;
        this.value = value;
    }
}

public struct LuaStringConstant {
    public string variable;
    public string value;
}

public delegate int LuaUserdataNew<T>(LuaState L, T obj);


[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate int LuaCallback(LuaState lua);