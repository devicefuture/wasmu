#include "../helper.h"

TEST {
    PART("Load code");

    wasmu_Context* context = wasmu_newContext();
    char* code;
    unsigned int size = LOAD("build/tables.wasm", &code);

    wasmu_Module* module = wasmu_newModule(context);

    wasmu_load(module, code, size);

    ASSERT(wasmu_parseSections(module), "Parsing failed");

    ASSERT(module->tablesCount == 1, "Table count is not 1");

    PART("Run function calls");

    printf("Get function: \"callIndirect\"\n");

    wasmu_Function* callIndirect = wasmu_getExportedFunction(module, "callIndirect");

    const unsigned int expectedResults[] = {24, 36, 12};

    for (unsigned int i = 0; i < 3; i++) {
        printf("Call case %d\n", i);

        wasmu_pushInt(context, 4, i); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);
        wasmu_pushInt(context, 4, 6); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

        ASSERT(context->valueStack.position == 4 * 2, "Value stack is not at correct position");

        ASSERT(wasmu_runFunction(module, callIndirect), "Error encountered while running function");

        ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
        ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

        unsigned int result = wasmu_popInt(context, 4);

        printf("Result: %d\n", result);

        ASSERT(result == expectedResults[i], "Result is not expected value");
        ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");
    }

    PART("Destroy context");

    wasmu_destroyContext(context);

    PASS();
}