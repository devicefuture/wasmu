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

    printf("Get function: \"branchInBlock\"\n");

    wasmu_Function* branchInBlock = wasmu_getExportedFunction(module, "branchInBlock");

    ASSERT(branchInBlock, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, branchInBlock), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 4, "Result is not 4");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"nearBranchInNestedBlocks\"\n");

    wasmu_Function* nearBranchInNestedBlocks = wasmu_getExportedFunction(module, "nearBranchInNestedBlocks");

    ASSERT(nearBranchInNestedBlocks, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, nearBranchInNestedBlocks), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 7, "Result is not 7");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"farBranchInNestedBlocks\"\n");

    wasmu_Function* farBranchInNestedBlocks = wasmu_getExportedFunction(module, "farBranchInNestedBlocks");

    ASSERT(farBranchInNestedBlocks, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, farBranchInNestedBlocks), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 5, "Result is not 5");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"loopCumulativeSum\"\n");

    wasmu_Function* loopCumulativeSum = wasmu_getExportedFunction(module, "loopCumulativeSum");

    ASSERT(loopCumulativeSum, "Function not found");

    wasmu_pushInt(context, 4, 4); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, loopCumulativeSum), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 10, "Result is not 10");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"excessStackValues\"\n");

    wasmu_Function* excessStackValues = wasmu_getExportedFunction(module, "excessStackValues");

    ASSERT(excessStackValues, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, excessStackValues), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 12, "Result is not 12");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"ifTrue\"\n");

    wasmu_Function* ifTrue = wasmu_getExportedFunction(module, "ifTrue");

    ASSERT(ifTrue, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, ifTrue), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 2, "Result is not 2");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"ifFalse\"\n");

    wasmu_Function* ifFalse = wasmu_getExportedFunction(module, "ifFalse");

    ASSERT(ifFalse, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, ifFalse), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 0, "Result is not 0");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"ifElseTrue\"\n");

    wasmu_Function* ifElseTrue = wasmu_getExportedFunction(module, "ifElseTrue");

    ASSERT(ifElseTrue, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, ifElseTrue), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 2, "Result is not 2");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"ifElseFalse\"\n");

    wasmu_Function* ifElseFalse = wasmu_getExportedFunction(module, "ifElseFalse");

    ASSERT(ifElseFalse, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, ifElseFalse), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 3, "Result is not 3");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"branchTable\"\n");

    wasmu_Function* branchTable = wasmu_getExportedFunction(module, "branchTable");

    ASSERT(branchTable, "Function not found");

    const unsigned int expectedResults[] = {8, 12, 4, 3, 3};

    for (unsigned int i = 0; i < 4; i++) {
        printf("Call case %d\n", i);

        wasmu_pushInt(context, 4, i); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);
        wasmu_pushInt(context, 4, 6); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

        ASSERT(context->valueStack.position == 4 * 2, "Value stack is not at correct position");

        ASSERT(wasmu_runFunction(module, branchTable), "Error encountered while running function");

        ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
        ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

        unsigned int result = wasmu_popInt(context, 4);

        printf("Result: %d\n", result);

        ASSERT(result == expectedResults[i], "Result is not expected value");
        ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");
    }

    printf("Get function: \"doubleNearBranchInNestedBlocks\"\n");

    wasmu_Function* doubleNearBranchInNestedBlocks = wasmu_getExportedFunction(module, "doubleNearBranchInNestedBlocks");

    ASSERT(doubleNearBranchInNestedBlocks, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, doubleNearBranchInNestedBlocks), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 1, "Result is not 1");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    PART("Destroy context");

    wasmu_destroyContext(context);

    PASS();
}