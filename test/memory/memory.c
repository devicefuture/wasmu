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
    ASSERT(module->memories[0].maxPages == 4, "Memory max pages count is not 4");

    printf("Get function: \"storeLoad\"\n");

    wasmu_Function* storeLoad = wasmu_getExportedFunction(module, "storeLoad");

    ASSERT(storeLoad, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, storeLoad), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 4321, "Result is not 4321");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"getSize\"\n");

    wasmu_Function* getSize = wasmu_getExportedFunction(module, "getSize");

    ASSERT(getSize, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, getSize), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 1, "Result is not 1");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"grow\"\n");

    wasmu_Function* grow = wasmu_getExportedFunction(module, "grow");

    ASSERT(grow, "Function not found");

    printf("Test growing by 4 pages\n");

    wasmu_pushInt(context, 4, 4); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, grow), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == -1, "Result is not -1");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Test growing by 3 pages\n");

    wasmu_pushInt(context, 4, 3); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, grow), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 1, "Result is not 1");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"readData\"\n");

    wasmu_Function* readData = wasmu_getExportedFunction(module, "readData");

    ASSERT(readData, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, readData), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    unsigned long expectedValue = (
        (unsigned long)('A' | ('B' << 8) | ('C' << 16) | ('D' << 24)) +
        (unsigned long)('E' | ('F' << 8) | ('G' << 16) | ('H' << 24))
    );

    ASSERT(wasmu_popInt(context, 4) == (int)expectedValue, "Result is not expected value");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"storeLoad16\"\n");

    wasmu_Function* storeLoad16 = wasmu_getExportedFunction(module, "storeLoad16");

    ASSERT(storeLoad16, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, storeLoad16), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == -1234, "Result is not -1234");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    PART("Destroy context");

    wasmu_destroyContext(context);

    PASS();
}