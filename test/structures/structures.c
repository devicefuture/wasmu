#include "../helper.h"

TEST {
    PART("Load code");

    wasmu_Context* context = wasmu_newContext();
    char* code;
    unsigned int size = LOAD("build/structures.wasm", &code);

    wasmu_Module* module = wasmu_newModule(context);

    wasmu_load(module, code, size);
    wasmu_parseSections(module);

    PART("Run function calls");

    printf("Get function: \"basicBlock\"\n");

    wasmu_Function* basicBlock = wasmu_getExportedFunction(module, "basicBlock");

    ASSERT(basicBlock, "Function not found");

    wasmu_pushInt(context, 4, 12); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, basicBlock), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 36, "Result is not 36");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    PASS();
}