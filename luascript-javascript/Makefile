CFLAGS = -Wall -Wfloat-conversion -Wextra -std=c17
EMCC = "${EMSDK}/upstream/emscripten/emcc"
SOURCES = $(wildcard src/*.c)
EMFLAGS = -s STRICT=1 -s ASYNCIFY=1 -s ALLOW_MEMORY_GROWTH=1
EMMODULE = -s MODULARIZE=1 -s EXPORT_NAME=ModuleLua
EMMODULE := $(EMMODULE) -s EXPORTED_FUNCTIONS=@./src/exports.txt --pre-js ./src/kdmyEngine.js
EMMODULE := $(EMMODULE) -s INCOMING_MODULE_JS_API=print,printErr
EMMODULE := $(EMMODULE) -s ALLOW_TABLE_GROWTH -s EXPORTED_RUNTIME_METHODS=addFunction,removeFunction
KDMYENGINEPATH = ./../src-javascript/engine/externals


wasm-build-release:
	$(EMCC) $(SOURCES) lib/liblua.a -Iinclude -O2 $(EMFLAGS) $(EMMODULE) ${CFLAGS} -o output/lua.js
	cp ./output/lua.wasm ${KDMYENGINEPATH}/lua.wasm
	cp ./output/lua.js ${KDMYENGINEPATH}/lua.js

wasm-build-debug:
	$(EMCC) $(SOURCES) lib/liblua.a -Iinclude -g $(EMFLAGS) $(EMMODULE) ${CFLAGS} -o output/lua.js
#	cp ./output/luascript.wasm ${KDMYENGINEPATH}/luascript.wasm

