#include "commons.h"
#include "psshader.h"

static PSShader_t stub = {};

PSShader psshader_init(const char* vertex_sourcecode, const char* fragment_sourcecode) {
    print_stub("psshader_init", "vertex_sourcecode=%s fragment_sourcecode=%s", vertex_sourcecode, fragment_sourcecode);
    return &stub;
}

void psshader_destroy(PSShader psshader) {
    print_stub("psshader_destroy", "psshader=%p", psshader);
}

int32_t psshader_set_uniform_any(PSShader psshader, const char* name, const double values[]) {
    print_stub("psshader_set_uniform_any", "psshader=%p name=%s values=%p", psshader, values);
    return 0;
}
bool psshader_set_uniform1f(PSShader psshader, const char* name, float value) {
    print_stub("psshader_set_uniform1f", "psshader=%p name=%s value1=%f", psshader, name, value);
    return 1;
}
bool psshader_set_uniform1i(PSShader psshader, const char* name, int32_t value) {
    print_stub("psshader_set_uniform1i", "psshader=%p name=%s value1=%i", psshader, name, value);
    return 1;
}
