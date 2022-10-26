#CC = C:/msys64/mingw32/bin/gcc.exe
CFLAGS = -Wall -Wfloat-conversion -Wextra -std=c99
SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
LIBSx86 = libs/x86/libvorbisfile-3.dll libs/x86/libportaudio.dll
LIBSx64 = libs/x64/libvorbisfile-3.dll libs/x64/libportaudio.dll
INCLUDE = -Iinclude

# in non windows use -lpthread
#LIBS += -lpthread
#LIBS += -lsynchapi

# for cubeb use instead of portaudio
#LIBS += -lole32 

build-debug:
	windres src/info.rc output/info.o
	$(CC) -o output/main.exe $(CFLAGS) -g $(SOURCES) $(INCLUDE) $(LIBSx86) output/info.o -DDEBUG 

build-release:
	$(info "TODO: export ONLY sndbridge_* functions...")
	windres src/info.rc output/info.o
	$(CC) -shared -o output/kdy_AICA.dll $(CFLAGS) -s -O2 $(SOURCES) $(INCLUDE) $(LIBSx86) output/info.o

build-release-scurve:
	$(info "TODO: export ONLY sndbridge_* functions...")
	windres src/info.rc output/info.o
	$(CC) -shared -o "output/kdy_AICA (s-curve fading).dll" $(CFLAGS) -s -O2 $(SOURCES) $(INCLUDE) $(LIBSx86) output/info.o -DSCURVE


build-release-x64:
	$(info "TODO: export ONLY sndbridge_* functions...")
	windres src/info.rc output/info.o
	$(CC) -shared -o output/kdy_AICA.dll $(CFLAGS) -s -O2 $(SOURCES) $(INCLUDE) $(LIBSx64) output/info.o

build-release-x64-scurve:
	$(info "TODO: export ONLY sndbridge_* functions...")
	windres src/info.rc output/info.o
	$(CC) -shared -o "output/kdy_AICA (s-curve fading).dll" $(CFLAGS) -s -O2 $(SOURCES) $(INCLUDE) $(LIBSx64) output/info.o -DSCURVE
