#ifndef _psshader_h
#define _psshader_h

#include <stdint.h>
#include <stdbool.h>

/** desktop only */

typedef struct {
    int stub;
} PSShader_t;

typedef PSShader_t* PSShader;

PSShader psshader_init(const char* vertex_sourcecode, const char* fragment_sourcecode);
void psshader_destroy(PSShader psshader);
int32_t psshader_set_uniform_any(PSShader psshader, const char* name, const double values[]);
bool psshader_set_uniform1f(PSShader psshader, const char* name, float value);
bool psshader_set_uniform1i(PSShader psshader, const char* name, int32_t value);

#endif
