CFLAGS = -Wall -Wfloat-conversion -Wextra -std=c99
EMCC = emcc
SOURCES = $(wildcard src/*.c)
TEST_SOURCES = $(wildcard src/test/*.c)
LIBS = lib/x64/avcodec-59.dll lib/x64/avformat-59.dll lib/x64/avutil-57.dll lib/x64/swresample-4.dll lib/x64/swscale-6.dll

windows-build-debug:
	$(CC) $(CFLAGS) $(TEST_SOURCES) $(SOURCES) $(LIBS) -Iinclude -O0 -ggdb3 -DDEBUG -o output/test.exe

build-release-x64:
	windres src/info.rc output/info.o
	$(CC) -shared -o output/kdy_FFgraph.dll $(CFLAGS) -Iinclude -s -O2 $(SOURCES) $(LIBS) -DDRELEASE output/info.o

