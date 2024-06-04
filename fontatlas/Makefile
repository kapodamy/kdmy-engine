CFLAGS = -Wall -Wfloat-conversion -Wextra -std=c17
EMCC = "${EMSDK}/upstream/emscripten/emcc"
SOURCES = $(wildcard src/*.c)  $(wildcard src/utils/*.c)
WINDOWS_STUBS_SOURCES = $(wildcard src/windows_stubs/*.c)
INCLUDES = -Iinclude -Iinclude/freetype2 -Isrc
EMMODULE = -s MODULARIZE -s EXPORT_NAME=ModuleFontAtlas
EMMODULE := $(EMMODULE) -s EXPORTED_FUNCTIONS=@./src/exports.txt --pre-js ./src/kdmyEngine.js
EMMODULE := $(EMMODULE) -s INCOMING_MODULE_JS_API=print,printErr
EMFLAGS = -s STRICT=1 -s ALLOW_MEMORY_GROWTH=1 ${KDMYENGINEFLAGS} ${CFLAGS}
KDMYENGINEPATH = ./../src-javascript/engine/externals

build-release:
	$(EMCC) $(SOURCES) libs/libfreetype.a $(INCLUDES) -O2 -std=c17 $(EMFLAGS) $(EMMODULE) -o output/module_fontatlas.js
	cp ./output/module_fontatlas.js ${KDMYENGINEPATH}/module_fontatlas.js
	cp ./output/module_fontatlas.wasm ${KDMYENGINEPATH}/module_fontatlas.wasm

build-debug:
	$(EMCC) $(SOURCES) libs/libfreetype.a $(INCLUDES) -O0 -std=c17 -g3 $(EMFLAGS) $(EMMODULE) -o output/module_fontatlas.js

build-debug-windows:
	$(CC) $(WINDOWS_STUBS_SOURCES) $(SOURCES) -Wall libs/x64/freetype.dll $(INCLUDES) -O0 -g -std=c17 -o output/main.exe
