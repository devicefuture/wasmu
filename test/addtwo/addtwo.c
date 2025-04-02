#include "../helper.h"
#include "../../dist/wasmu.h"

TEST {
    PART("Load code");

    wasmu_Context* context = wasmu_newContext();
    char* code;
    unsigned int size = LOAD("build/addtwo.wasm", &code);

    wasmu_load(context, code, size);

    ASSERT(context->codeSize == size, "Code not loaded");

    ASSERT(wasmu_parseSections(context), "Parsing failed");

    PASS();
}