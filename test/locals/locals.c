#include "../helper.h"

struct ExpectedFunction {
    char* name;
    unsigned int parametersCount;
    unsigned int resultsCount;
    unsigned int localsCount;
};

TEST {
    PART("Load code");

    wasmu_Context* context = wasmu_newContext();
    char* code;
    unsigned int size = LOAD("build/locals.wasm", &code);

    wasmu_Module* module = wasmu_newModule(context);

    wasmu_load(module, code, size);
    wasmu_parseSections(module);

    PART("Get exported functions");

    struct ExpectedFunction expectedFunctions[] = {
        {"noLocals", 0, 0, 0},
        {"paramsOnly", 4, 0, 0},
        {"resultsOnly", 0, 4, 0},
        {"paramsResults", 2, 2, 0},
        {"localsOnly", 0, 0, 4},
        {"paramsResultsLocals", 2, 2, 2}
    };

    for (unsigned int i = 0; i < sizeof(expectedFunctions) / sizeof(expectedFunctions[0]); i++) {
        struct ExpectedFunction expectedFunction = expectedFunctions[i];

        printf("Get function: \"%s\"\n", expectedFunction.name);

        wasmu_Function* function = wasmu_getExportedFunction(module, expectedFunction.name);

        ASSERT(function, "Function not found");

        wasmu_FunctionSignature* functionSignature = WASMU_GET_ENTRY(
            module->functionSignatures,
            module->functionSignaturesCount,
            function->signatureIndex
        );

        ASSERT(functionSignature, "No signature attached to function");

        ASSERT(functionSignature->parametersCount == expectedFunction.parametersCount, "Parameters count is incorrect");
        ASSERT(functionSignature->resultsCount == expectedFunction.resultsCount, "Returns count is incorrect");
        ASSERT(function->localsCount == expectedFunction.localsCount, "Locals count is incorrect");
    }

    PART("Run function calls");

    printf("Get function: \"setLocal\"\n");

    wasmu_Function* setLocal = wasmu_getExportedFunction(module, "setLocal");

    ASSERT(setLocal, "Function not found");

    wasmu_pushInt(context, 4, 12); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, setLocal), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 24, "Result is not 24");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"teeLocal\"\n");

    wasmu_Function* teeLocal = wasmu_getExportedFunction(module, "teeLocal");

    ASSERT(teeLocal, "Function not found");

    wasmu_pushInt(context, 4, 12); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, teeLocal), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 24, "Result is not 24");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    PASS();
}