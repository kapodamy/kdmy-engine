CFLAGS = -Wall
EMCC = emcc
SOURCES = $(wildcard src/*.c)
WINDOWS_STUBS_SOURCES = $(wildcard src/windows_stubs/*.c)
HEADERS = $(wildcard src/*.h)
# KDMYENGINEFLAGS = -DJAVASCRIPT -s EXPORTED_FUNCTIONS=@src/exports.txt -s USE_FREETYPE=1
KDMYENGINEFLAGS = -DJAVASCRIPT -s EXPORTED_FUNCTIONS=@src/exports.txt -s libs/libfreetype.a
EMFLAGS = -s STRICT=1 -s ALLOW_MEMORY_GROWTH=1 ${KDMYENGINEFLAGS} ${CFLAGS}

build-release:
	$(EMCC) $(SOURCES) -Iinclude -O2 $(EMFLAGS) -o output/fontatlas.js
#   cp .\output\fontatlas.wasm .\..\engine\externals\fontatlas.wasm

build-debug:
	$(EMCC) $(SOURCES) -Iinclude -O0 -g3 $(EMFLAGS) -o output/fontatlas.js
#   cp .\output\fontatlas.wasm .\..\engine\externals\fontatlas.wasm

build-debug-windows:
	$(CC) $(WINDOWS_STUBS_SOURCES) $(SOURCES) -Wall libs/x64/freetype.dll -Iinclude -O0 -g -std=c99 -o output/main.exe
