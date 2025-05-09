#include "../helper.h"

wasmu_Bool printChar(wasmu_Context* context) {
    char c = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

    printf("%c", c);

    return WASMU_TRUE;
}

TEST {
    PART("Load code");

    wasmu_Context* context = wasmu_newContext();
    char* code;
    unsigned int size = LOAD("build/cpp.wasm", &code);

    wasmu_Module* module = wasmu_newModule(context);

    wasmu_load(module, code, size);
    wasmu_parseSections(module);

    PART("Set up native module");

    wasmu_Module* nativeModule = wasmu_newModule(context);

    wasmu_addNativeFunction(nativeModule, "printChar", printChar);

    PART("Link modules");

    wasmu_assignModuleName(nativeModule, "native");

    ASSERT(wasmu_resolveModuleImports(module), "Failed to resolve module imports");

    PART("Run function calls");

    printf("Get function: \"start\"\n");

    wasmu_Function* start = wasmu_getExportedFunction(module, "start");

    ASSERT(start, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, start), "Error encountered while running function");

    PASS();
}