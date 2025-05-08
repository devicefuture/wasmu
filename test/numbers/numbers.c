#include "../helper.h"

TEST {
    PART("Load code");

    wasmu_Context* context = wasmu_newContext();
    char* code;
    unsigned int size = LOAD("build/numbers.wasm", &code);

    wasmu_Module* module = wasmu_newModule(context);

    wasmu_load(module, code, size);

    ASSERT(wasmu_parseSections(module), "Parsing failed");

    printf("Get function: \"addI64\"\n");

    wasmu_Function* addI64 = wasmu_getExportedFunction(module, "addI64");

    ASSERT(addI64, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, addI64), "Error encountered while running function");

    ASSERT(context->valueStack.position == 8, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 8) == 0x123456789ABCDF00, "Result is not 0x123456789ABCDF00");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I64, "Result type is not I64");

    PASS();
}