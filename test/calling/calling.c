#include "../helper.h"

TEST {
    PART("Load code");

    wasmu_Context* context = wasmu_newContext();
    char* code;
    unsigned int size = LOAD("build/calling.wasm", &code);

    wasmu_Module* module = wasmu_newModule(context);

    wasmu_load(module, code, size);

    ASSERT(wasmu_parseSections(module), "Parsing failed");

    PART("Get exported function");

    wasmu_Function* addSeven = wasmu_getExportedFunction(module, "addSeven");

    PART("Add arguments to value stack");

    for (unsigned int i = 1; i <= 7; i++) {
        wasmu_pushInt(context, 4, i);
        wasmu_pushType(context, WASMU_VALUE_TYPE_I32);
    }

    ASSERT(context->valueStack.position == 4 * 7, "Value stack is not at correct position");

    PART("Run retrieved function");

    ASSERT(wasmu_runFunction(module, addSeven), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 28, "Result is not 28");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    PASS();
}