#include "emscripten.h"
#include "lua.h"


EM_JS(void, _doLuaExport, (size_t reg_idx_value), {
    kdmyEngine_doLuaExport(reg_idx_value);
});


EMSCRIPTEN_KEEPALIVE void __attribute__((constructor)) __ctor_kdmyEngine() {
    _doLuaExport(LUA_REGISTRYINDEX);
}
