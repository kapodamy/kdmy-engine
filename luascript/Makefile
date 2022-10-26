CFLAGS = -Wall -Wfloat-conversion -Wextra
EMCC = emcc
SOURCES = $(wildcard src/*.c)
WINDOWS_STUBS_SOURCES = $(wildcard src/windows_stubs/*.c)
HEADERS = $(wildcard src/*.h)
KDMYENGINEFLAGS = -DJAVASCRIPT -s EXPORTED_FUNCTIONS=@src/exports.txt
EMFLAGS = -s STRICT=1 -s LLD_REPORT_UNDEFINED -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1 ${KDMYENGINEFLAGS} ${CFLAGS}

build-release:
	$(EMCC) $(SOURCES) lib/liblua.a -Iinclude -O2 $(EMFLAGS) -o output/luascript.js
	# cp .\output\luascript.wasm .\..\engine\externals\luascript.wasm

build-debug:
	$(EMCC) $(SOURCES) lib/liblua.a -Iinclude -g $(EMFLAGS) -o output/luascript.js
	# cp .\output\luascript.wasm .\..\engine\externals\luascript.wasm

build-debug-windows:
	$(CC) $(WINDOWS_STUBS_SOURCES) $(SOURCES) lib/lua54.dll -Iinclude -O0 -g -std=c99 -o output/main.exe
