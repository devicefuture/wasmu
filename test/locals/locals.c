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

    PASS();
}