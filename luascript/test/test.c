#include <emscripten.h>
#include <stdlib.h>

EM_JS(void, imported_function, (const char* str, int number), {
    debugger;
    console.log(kdmyEngine_ptrToString(str), number);
});

void* exported_function1() {
    int* ptr = malloc(sizeof(int));
    *ptr = 12345;
    return ptr;
}
void exported_function2(void* ptr, const char* text) {
    int* nro = (int*)ptr;
    imported_function(text, *nro);
}
