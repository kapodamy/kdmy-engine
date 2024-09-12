#ifndef _fs_h
#define _fs_h

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "foreach.h"
#include "arraybuffer.h"


/**
 * folder interator. Important: do not use "return" or "longjmp", first use "break" to avoid memory leaks
 */
#define FS_FOLDER_ITERATOR(INSTANCE, VARIABLE_TYPE, VARIABLE_NAME) foreach__3(             \
    VARIABLE_TYPE, VARIABLE_NAME, INSTANCE, FSFolderEnumerator,                            \
    fs_folder_enumerate, fs_folder_enumerate_next, fs_folder_enumerate_close, /*(void**)*/ \
)

#define FS_STRING_SEPARATOR \
    (char[]) { FS_CHAR_SEPARATOR, '\0' }


typedef struct {
    const char* name;
    int length;
} FSFolderEnumeratorEntry;

typedef struct {
    bool __run;
    int32_t ___index;
    int32_t ___length;
    FSFolderEnumeratorEntry* ___entries;
} FSFolderEnumerator;

extern const char FS_CHAR_SEPARATOR;
extern const char FS_CHAR_SEPARATOR_REJECT;
extern const char* FS_FILENAME_INVALID_CHARS;
extern const size_t FS_TEMP_MAX_LENGTH;
extern const char* FS_ASSETS_FOLDER;
extern const char* FS_ASSETS_FOLDER_NO_OVERRIDE;
extern const char* FS_EXPANSIONS_FOLDER;
extern const char* FS_ASSETS_COMMON_FOLDER;
extern const char* FS_NO_OVERRIDE_COMMON;

void fs_init_kdy();

char* fs_readtext(const char* src);
ArrayBuffer fs_readarraybuffer(const char* src);

bool fs_file_exists(const char* src);
bool fs_folder_exists(const char* src);
int64_t fs_file_length(const char* src);
FSFolderEnumerator fs_folder_enumerate(const char* src);
bool fs_folder_enumerate_next(FSFolderEnumerator* folder_enumerator, FSFolderEnumeratorEntry** folder_entry);
bool fs_folder_enumerate_close(FSFolderEnumerator* folder_enumerator);
bool fs_is_invalid_filename(const char* filename);
char* fs_get_parent_folder(const char* filename);
char* fs_build_path(const char* base_path, const char* filename);
char* fs_build_path2(const char* reference_filename, const char* filename);
char* fs_get_filename_without_extension(const char* filename);
char* fs_resolve_path(const char* src);

void fs_set_working_folder(const char* base_path, bool get_parent_of_base_path);
void fs_set_working_subfolder(const char* sub_path);
char* fs_get_full_path(const char* path);
char* fs_get_full_path_and_override(const char* path);
void fs_override_common_folder(const char* base_path);
void fs_folder_stack_push();
void fs_folder_stack_pop();


#endif
