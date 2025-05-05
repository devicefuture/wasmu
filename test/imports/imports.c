#include "../helper.h"

wasmu_Bool hello(wasmu_Context* context) {
    printf("Hello, world!\n");

    wasmu_Int b = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
    wasmu_Int a = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
    wasmu_Int result = (a * a) + b;

    printf("Popped %d and %d, pushing %d\n", a, b, result);

    wasmu_pushInt(context, 4, result); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

    return WASMU_TRUE;
}

TEST {
    wasmu_Context* context = wasmu_newContext();

    PART("Load code for module A");

    char* aCode;
    unsigned int aSize = LOAD("build/a.wasm", &aCode);

    wasmu_Module* aModule = wasmu_newModule(context);

    wasmu_load(aModule, aCode, aSize);

    ASSERT(wasmu_parseSections(aModule), "Parsing failed");

    ASSERT(aModule->importsCount == 2, "Imports count is not 2");

    PART("Load code for module B");

    char* bCode;
    unsigned int bSize = LOAD("build/b.wasm", &bCode);

    wasmu_Module* bModule = wasmu_newModule(context);

    wasmu_load(bModule, bCode, bSize);

    ASSERT(wasmu_parseSections(bModule), "Parsing failed");

    ASSERT(bModule->importsCount == 1, "Imports count is not 1");

    PART("Set up module C");

    wasmu_Module* cModule = wasmu_newModule(context);

    wasmu_addNativeFunction(cModule, "hello", hello);

    PART("Link modules");

    wasmu_assignModuleName(aModule, "a");
    wasmu_assignModuleName(bModule, "b");
    wasmu_assignModuleName(cModule, "c");

    ASSERT(wasmu_resolveModuleImports(aModule), "Failed to resolve A module imports");
    ASSERT(wasmu_resolveModuleImports(bModule), "Failed to resolve B module imports");

    printf("Get function: \"testCallB\"\n");

    wasmu_Function* testCallB = wasmu_getExportedFunction(aModule, "testCallB");

    ASSERT(testCallB, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(aModule, testCallB), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 27, "Result is not 27");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"testCallA\"\n");

    wasmu_Function* testCallA = wasmu_getExportedFunction(bModule, "testCallA");

    ASSERT(testCallA, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(bModule, testCallA), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 12, "Result is not 12");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"testCallC\"\n");

    wasmu_Function* testCallC = wasmu_getExportedFunction(aModule, "testCallC");

    ASSERT(testCallC, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(aModule, testCallC), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 22, "Result is not 22");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    PASS();
}