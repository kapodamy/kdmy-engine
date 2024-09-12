#ifndef _preloadcache_h
#define _preloadcache_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PRELOADCACHE_PRELOAD_FILENAME "preload.ini"


//
//  forward reference
//

typedef struct ArrayBuffer_s* ArrayBuffer;
typedef struct ArrayPointerList_s* ArrayPointerList;

#ifdef MALLOC_CHK_ENABLE
void preloadcache_malloc_chk_enable_autofilter_entries(bool enabled);
#endif

void preloadcache_clear();
bool preloadcache_flush(size_t needed_space);
bool preloadcache_append(char* native_path, bool allow_flush, bool low_piority, ArrayBuffer arraybuffer);
ArrayBuffer preloadcache_adquire(const char* native_path);
bool preloadcache_release(ArrayBuffer adquired_arraybuffer);
bool preloadcache_has(ArrayBuffer arraybuffer);
int64_t preloadcache_stat(const char* native_path);
int32_t preloadcache_add_file_list(const char* src_filelist);

#endif
