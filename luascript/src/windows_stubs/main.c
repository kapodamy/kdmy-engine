#include <stdio.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "luascript.h"
#include "week.h"
#include "timer.h"


static RoundContext_t roundcontext = {};


static inline char* read_text(const char* path) {
    FILE* fp;
    long lSize;
    char* buffer;

    fp = fopen(path, "rb");
    if (!fp) perror(path), exit(1);

    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);

    /* allocate memory for entire content */
    buffer = calloc(1, lSize + 1);
    if (!buffer) fclose(fp), fputs("memory alloc fails", stderr), exit(1);

    /* copy the file into the buffer */
    if (1 != fread(buffer, lSize, 1, fp))
        fclose(fp), free(buffer), fputs("entire read fails", stderr), exit(1);

    /* do your work here, buffer is a string contains the whole text */

    fclose(fp);
    return buffer;
}

int main() {
    char* lua_text = read_text(".\\timer_test.lua");
    Luascript luascript = luascript_init(lua_text, "timer_test.lua", &roundcontext, false);
    free(lua_text);

    if (!luascript) return 1;

    int total = 10000 / 10;
    for (int i = 0; i < total; i++) {
        luascript_notify_timer_run(luascript, timer_ms_gettime64());
        Sleep(10);
    }

    luascript_destroy(&luascript);

    printf("done!\n");
    return 0;
}

int main2() {
    char* lua_text = read_text("C:\\Users\\kapodamy\\Desktop\\kdmy_engine\\assets\\weeks\\week2\\halloween\\week2.lua");
    Luascript luascript = luascript_init(lua_text, "week2.lua", &roundcontext, true);
    free(lua_text);

    if (!luascript) return 1;

    luascript_notify_beforeready(luascript, false);
    luascript_notify_frame(luascript, 1000 / 60);

    for (int32_t i = 0;i < 600;i++) {
        luascript_notify_beat(luascript, i, 10.333F);
    }

    luascript_notify_weekend(luascript, false);

    luascript_destroy(&luascript);

    return 0;
}

