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

    wasmu_Function* addTwo = wasmu_getExportedFunction(module, "addTwo");

    ASSERT(addTwo, "Function not found");

    PART("Add arguments to value stack");

    wasmu_pushInt(context, 4, 1); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);
    wasmu_pushInt(context, 4, 2); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

    ASSERT(context->valueStack.position == 8, "Value stack is not at correct position");

    PART("Run retrieved function");

    ASSERT(wasmu_runFunction(module, addTwo), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 3, "Result is not 3");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    PART("Destroy context");

    wasmu_destroyContext(context);

    PASS();
}