#include <stdint.h>
#include <profileapi.h>

static LARGE_INTEGER frequency;
static uint64_t offset;

uint64_t timer_ms_gettime64() {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    uint64_t timestamp = (counter.QuadPart * 1000) / frequency.QuadPart;
    return timestamp - offset;
}

static void __attribute__((constructor)) timer_init() {
    QueryPerformanceFrequency(&frequency);
    offset = timer_ms_gettime64();
}
