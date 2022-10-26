#ifndef _fs_h
#define _fs_h

#include <stdbool.h>
#include <stdint.h>

bool fs_readfile(const char* path, uint8_t** buffer_ptr, uint32_t* size_ptr);

#endif

