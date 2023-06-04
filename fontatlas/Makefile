CFLAGS = -Wall
EMCC = emcc
SOURCES = $(wildcard src/*.c)
WINDOWS_STUBS_SOURCES = $(wildcard src/windows_stubs/*.c)
HEADERS = $(wildcard src/*.h)
# KDMYENGINEFLAGS = -DJAVASCRIPT -s EXPORTED_FUNCTIONS=@src/exports.txt -s USE_FREETYPE=1
KDMYENGINEFLAGS = -DJAVASCRIPT -s EXPORTED_FUNCTIONS=@src/exports.txt -s libs/libfreetype.a
EMFLAGS = -s STRICT=1 -s ALLOW_MEMORY_GROWTH=1 ${KDMYENGINEFLAGS} ${CFLAGS}

build-release-dll-x86:
	windres src/info.rc output/info.o
	$(CC) -shared -o output/kdy_fontatlas.dll -Wall -std=c99 -O2 $(SOURCES) -Iinclude libs/x86/freetype.dll output/info.o

build-release-dll-x64:
	windres src/info.rc output/info.o
	$(CC) -shared -o output/kdy_fontatlas.dll -Wall -std=c99 -O2 $(SOURCES) -Iinclude libs/x64/freetype.dll output/info.o

build-release:
	$(EMCC) $(SOURCES) -Iinclude -O2 $(EMFLAGS) -o output/fontatlas.js
	# cp .\output\fontatlas.wasm .\..\engine\externals\fontatlas.wasm


##############################
# Important: emscripten must be intalled or present in the PATH variable 
# normally under "%userprofile%\AppData\Local\emsdk\upstream\emscripten"
	

build-debug:
	$(EMCC) $(SOURCES) -Iinclude -O0 -g3 $(EMFLAGS) -o output/fontatlas.js
	# cp .\output\fontatlas.wasm .\..\engine\externals\fontatlas.wasm

build-debug-windows:
	$(CC) $(WINDOWS_STUBS_SOURCES) $(SOURCES) -Wall libs/x64/freetype.dll -Iinclude -O0 -g -std=c99 -o output/main.exe
