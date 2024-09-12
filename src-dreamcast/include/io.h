#ifndef _io_h
#define _io_h

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "arraybuffer.h"


#define IO_4Z "\x20\x20\x20\x20"


typedef int64_t IOFolderHandle;

bool io_open_folder(const char* absolute_path, IOFolderHandle* out_hnd);
bool io_read_folder(IOFolderHandle fldrhnd, const char** name, int* size);
void io_close_folder(IOFolderHandle* fldrhnd);

ArrayBuffer io_read_arraybuffer(const char* absolute_path);
bool io_resource_exists(const char* absolute_path, bool expect_file, bool expect_folder);
int64_t io_file_size(const char* absolute_path);
char* io_get_native_path(const char* absolute_path, bool is_file, bool is_folder, bool resolve_expansion);
void io_override_file_extension(char* native_path_4Z, bool check_if_folder);

#endif
