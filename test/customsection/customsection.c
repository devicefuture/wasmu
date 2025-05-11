#include "../helper.h"

TEST {
    PART("Load code");

    wasmu_Context* context = wasmu_newContext();
    char* code;
    unsigned int size = LOAD("build/customsection.wasm", &code);

    wasmu_Module* module = wasmu_newModule(context);

    wasmu_load(module, code, size);

    ASSERT(wasmu_parseSections(module), "Parsing failed");

    ASSERT(module->customSectionsCount == 2, "Custom section count is not 2");
    ASSERT(wasmu_stringEqualsChars(module->customSections[0].name, "TestA"), "First custom section name is incorrect");
    ASSERT(wasmu_stringEqualsChars(module->customSections[1].name, "TestB"), "Second custom section name is incorrect");

    PART("Destroy context");

    wasmu_destroyContext(context);

    PASS();
}