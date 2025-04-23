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

    printf("Get function: \"storeLoad\"\n");

    wasmu_Function* storeLoad = wasmu_getExportedFunction(module, "storeLoad");

    ASSERT(storeLoad, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, storeLoad), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 4321, "Result is not 4321");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    PASS();
}