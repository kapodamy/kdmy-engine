#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "commons.h"


bool fs_readfile(const char* path, uint8_t** buffer_ptr, uint32_t* size_ptr) {
    const uint8_t stub[] = "abc123";
    print_stub("fs_readfile", "path=%s buffer_ptr=%p size_ptr=%p", path, buffer_ptr, size_ptr);

    const uint8_t stub_strfile[] = "abc123";
    const size_t stub_size = sizeof(stub);

    uint8_t* buf = malloc(stub_size);
    memcpy(buf, stub_strfile, stub_size);

    *buffer_ptr = buf;
    *size_ptr = stub_size;

    return 1;
}

