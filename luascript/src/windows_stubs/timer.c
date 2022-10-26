#include <time.h>
#include <stdint.h>

uint64_t timer_ms_gettime64() {
    return (uint64_t)clock();
}

