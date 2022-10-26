#include "test.h"

void* exported_function3() {
    imported_function("%%%exported_function3%%%", 456789);
    return 0;
}