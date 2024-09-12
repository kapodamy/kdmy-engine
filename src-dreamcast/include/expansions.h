#ifndef _expansions_h
#define _expansions_h

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


extern const char* EXPANSIONS_PATH;
extern const char* EXPANSIONS_SYMBOLIC_PATH;
extern const char* EXPANSIONS_FUNKIN_EXPANSION_NAME;
extern const char* EXPANSIONS_FUNKIN_EXPANSION_PATH;
extern const char* EXPANSIONS_CHAIN_FILENAME;
extern const char* EXPANSIONS_ABOUT_FILENAME;
extern const char EXPANSIONS_COMMENT_CHAR;
extern const char* EXPANSIONS_SELF_NAME;

extern char** expansions_chain_array;
extern int32_t expansions_chain_array_size;
extern char* expansions_overrided_weeks_folder;

void expansions_load(const char* expansion_name);
char* expansions_resolve_path(const char* path, bool is_file, bool is_folder);
char* expansions_get_path_from_expansion(const char* path, int32_t expansion_index);

#endif
