#include "../helper.h"

TEST {
    PART("Load code");

    wasmu_Context* context = wasmu_newContext();
    char* code;
    unsigned int size = LOAD("build/globals.wasm", &code);

    wasmu_Module* module = wasmu_newModule(context);

    wasmu_load(module, code, size);
    wasmu_parseSections(module);

    PART("Get exported globals");

    wasmu_TypedValue* globalA = wasmu_getExportedGlobal(module, "a");

    ASSERT(globalA, "Global not found");
    ASSERT(globalA->type == WASMU_VALUE_TYPE_I32, "Global type is not I32");
    ASSERT(globalA->value.asInt == 10, "Global value is not 10");

    wasmu_TypedValue* globalB = wasmu_getExportedGlobal(module, "b");

    ASSERT(globalB, "Global not found");
    ASSERT(globalB->type == WASMU_VALUE_TYPE_I32, "Global type is not I32");
    ASSERT(globalB->value.asInt == 20, "Global value is not 20");

    PART("Run function calls");

    printf("Get function: \"getGlobals\"\n");

    wasmu_Function* getGlobals = wasmu_getExportedFunction(module, "getGlobals");

    ASSERT(getGlobals, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, getGlobals), "Error encountered while running function");

    ASSERT(context->valueStack.position == 8, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 2, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 20, "Result is not 20");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");
    ASSERT(wasmu_popInt(context, 4) == 10, "Result is not 10");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"setGlobals\"\n");

    wasmu_Function* setGlobals = wasmu_getExportedFunction(module, "setGlobals");

    ASSERT(setGlobals, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, setGlobals), "Error encountered while running function");

    ASSERT(context->valueStack.position == 8, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 2, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 40, "Result is not 40");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");
    ASSERT(wasmu_popInt(context, 4) == 30, "Result is not 30");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    PART("Destroy context");

    wasmu_destroyContext(context);

    PASS();
}