#include "../helper.h"

TEST {
    PART("Load code");

    wasmu_Context* context = wasmu_newContext();
    char* code;
    unsigned int size = LOAD("build/memory.wasm", &code);

    wasmu_Module* module = wasmu_newModule(context);

    wasmu_load(module, code, size);

    ASSERT(wasmu_parseSections(module), "Parsing failed");

    ASSERT(module->memoriesCount == 1, "Memory count is not 1");
    ASSERT(module->memories[0].minPages == 1, "Memory min pages count is not 1");
    ASSERT(module->memories[0].maxPages == 2, "Memory max pages count is not 2");

    PASS();
}