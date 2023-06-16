#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_ASYNC_JS_PRFX(bool, fs_readfile, (const char* path, uint8_t** buffer_ptr, uint32_t* size_ptr), {
    try {
      if (buffer_ptr == 0) return 0;

      let arraybuffer = await fs_readarraybuffer(kdmyEngine_ptrToString(path));
      if (!arraybuffer) return 0;

      let ptr = _malloc(arraybuffer.byteLength);
      if (ptr == 0) {
        console.error("__asyncjs__fs_readfile() out-of-memory, size required was " + arraybuffer.byteLength);
        return 0;
      }

      (new Uint8Array(wasmMemory.buffer)).set(new Uint8Array(arraybuffer), ptr);

      kdmyEngine_set_uint32(buffer_ptr, ptr);
      kdmyEngine_set_uint32(size_ptr, arraybuffer.byteLength);

      return 1;
    } catch (e) {
      console.error(e);
      return 0;
    }
    });
#endif

static int script_fs_readfile(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);

    uint8_t* buffer = NULL;
    uint32_t size = 0;
    bool result = fs_readfile(path, &buffer, &size);

    if (result) {
        lua_pushlstring(L, (const char*)buffer, (size_t)size);
        free(buffer);
    } else {
        lua_pushnil(L);
    }

    return 1;
}


void script_fs_register(lua_State* L) {
    lua_pushcfunction(L, script_fs_readfile);
    lua_setglobal(L, "fs_readfile");
}

