CFLAGS = -Wall -Wfloat-conversion -Wextra -std=c17
EMCC = emcc
SOURCES = $(wildcard src/*.c)
WINDOWS_STUBS_SOURCES = $(wildcard src/windows_stubs/*.c)
HEADERS = $(wildcard src/*.h)
KDMYENGINEFLAGS = -DJAVASCRIPT -s EXPORTED_FUNCTIONS=@src/exports.txt
EMFLAGS = -s STRICT=1 -s LLD_REPORT_UNDEFINED=1 -s ASYNCIFY=1 -s ALLOW_MEMORY_GROWTH=1 ${KDMYENGINEFLAGS} ${CFLAGS}
EMMODULE = -s EXPORT_NAME=ModuleLuaScript

wasm-build-release:
	$(EMCC) $(SOURCES) lib/liblua.a -Iinclude -O2 $(EMFLAGS) ${EMMODULE} -o output/luascript.js
	# cp .\output\luascript.wasm .\..\engine\externals\luascript.wasm

wasm-build-debug:
	$(EMCC) $(SOURCES) lib/liblua.a -Iinclude -g $(EMFLAGS) ${EMMODULE} -o output/luascript.js
	# cp .\output\luascript.wasm .\..\engine\externals\luascript.wasm

windows-build-debug:
	$(CC) $(WINDOWS_STUBS_SOURCES) $(SOURCES) lib/lua54.dll -Iinclude -O0 -g ${CFLAGS} -o output/main.exe
