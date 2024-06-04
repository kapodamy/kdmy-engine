#include "math2d.h"

int32_t math2d_poweroftwo_calc(int32_t dimmen) {
    int32_t size = 2;
    while (size < 0x80000) {
        if (size >= dimmen) break;
        size *= 2;
    }

    assert(size < 0x80000);

    return size;
}
