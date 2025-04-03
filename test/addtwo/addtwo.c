#include "../helper.h"

TEST {
    PART("Load code");

    wasmu_Context* context = wasmu_newContext();
    char* code;
    unsigned int size = LOAD("build/addtwo.wasm", &code);

    wasmu_Module* module = wasmu_newModule(context);

    wasmu_load(module, code, size);

    ASSERT(module->codeSize == size, "Code not loaded");

    PART("Parse sections");

    ASSERT(wasmu_parseSections(module), "Parsing failed");


    PART("Get exported function");

    wasmu_Int addTwo = wasmu_getExportedFunction(module, "addTwo");

    ASSERT(addTwo != -1, "Function not found");

    PART("Run retrieved function");

    ASSERT(wasmu_runFunction(module, addTwo), "Error encountered while running function");

    PASS();
}