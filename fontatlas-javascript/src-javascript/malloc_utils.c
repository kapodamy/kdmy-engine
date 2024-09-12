#include "malloc_utils.h"
#include "logger.h"

void malloc_warn2(const char* type_name) {
    logger_error("Failed to allocate '%s' type, out-of-memory.", type_name);
}
