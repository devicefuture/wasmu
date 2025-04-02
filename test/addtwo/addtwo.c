#include "../helper.h"
#include "../../dist/wasmu.h"

TEST {
    PART("Load code");

    wasmu_Context* context = wasmu_newContext();
    char* code;
    unsigned int size = LOAD("build/addtwo.wasm", &code);

    printf("Code size: %d\n", size);

    PASS();
}