#include "../helper.h"

TEST {
    PART("Load code");

    wasmu_Context* context = wasmu_newContext();
    char* code;
    unsigned int size = LOAD("build/numbers.wasm", &code);

    wasmu_Module* module = wasmu_newModule(context);

    wasmu_load(module, code, size);

    ASSERT(wasmu_parseSections(module), "Parsing failed");

    printf("Get function: \"addI64\"\n");

    wasmu_Function* addI64 = wasmu_getExportedFunction(module, "addI64");

    ASSERT(addI64, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, addI64), "Error encountered while running function");

    ASSERT(context->valueStack.position == 8, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 8) == 0x123456789ABCDF00, "Result is not 0x123456789ABCDF00");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I64, "Result type is not I64");

    printf("Get function: \"truncF64S\"\n");

    wasmu_Function* truncF64S = wasmu_getExportedFunction(module, "truncF64S");

    ASSERT(truncF64S, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, truncF64S), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == -10, "Result is not -10");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"subF32\"\n");

    wasmu_Function* subF32 = wasmu_getExportedFunction(module, "subF32");

    ASSERT(subF32, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, subF32), "Error encountered while running function");

    ASSERT(context->valueStack.position == 8, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 8) == -4, "Result is not -4");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I64, "Result type is not I64");

    printf("Get function: \"divF32S\"\n");

    wasmu_Function* divF32S = wasmu_getExportedFunction(module, "divF32S");

    ASSERT(divF32S, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, divF32S), "Error encountered while running function");

    ASSERT(context->valueStack.position == 8, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 8) == -5, "Result is not -5");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I64, "Result type is not I64");

    printf("Get function: \"divF32U\"\n");

    wasmu_Function* divF32U = wasmu_getExportedFunction(module, "divF32U");

    ASSERT(divF32U, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, divF32U), "Error encountered while running function");

    ASSERT(context->valueStack.position == 8, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 8) == 4611686018427387904, "Result is not 4611686018427387904");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I64, "Result type is not I64");

    printf("Get function: \"wrap\"\n");

    wasmu_Function* wrap = wasmu_getExportedFunction(module, "wrap");

    ASSERT(wrap, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, wrap), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 4) == 0x76543210, "Result is not 0x76543210");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I32, "Result type is not I32");

    printf("Get function: \"extendS\"\n");

    wasmu_Function* extendS = wasmu_getExportedFunction(module, "extendS");

    ASSERT(extendS, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, extendS), "Error encountered while running function");

    ASSERT(context->valueStack.position == 8, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 8) == -10, "Result is not -10");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I64, "Result type is not I64");

    printf("Get function: \"extendU\"\n");

    wasmu_Function* extendU = wasmu_getExportedFunction(module, "extendU");

    ASSERT(extendU, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, extendU), "Error encountered while running function");

    ASSERT(context->valueStack.position == 8, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 8) == 4294967286, "Result is not 4294967286");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I64, "Result type is not I64");

    printf("Get function: \"demote\"\n");

    wasmu_Function* demote = wasmu_getExportedFunction(module, "demote");

    ASSERT(demote, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, demote), "Error encountered while running function");

    ASSERT(context->valueStack.position == 4, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    wasmu_Float value = wasmu_popFloat(context, WASMU_VALUE_TYPE_F32);

    ASSERT(value == (wasmu_F32)123.456789, "Result is not 123.456789");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_F32, "Result type is not F32");

    printf("Get function: \"promote\"\n");

    wasmu_Function* promote = wasmu_getExportedFunction(module, "promote");

    ASSERT(promote, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, promote), "Error encountered while running function");

    ASSERT(context->valueStack.position == 8, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popFloat(context, WASMU_VALUE_TYPE_F64) == (wasmu_F32)123.456789, "Result is not 123.456789");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_F64, "Result type is not F64");

    printf("Get function: \"reinterpretToI64\"\n");

    wasmu_Function* reinterpretToI64 = wasmu_getExportedFunction(module, "reinterpretToI64");

    ASSERT(reinterpretToI64, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, reinterpretToI64), "Error encountered while running function");

    ASSERT(context->valueStack.position == 8, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popInt(context, 8) == 4638355772470722560, "Result is not 4638355772470722560");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_I64, "Result type is not I64");

    printf("Get function: \"reinterpretToF64\"\n");

    wasmu_Function* reinterpretToF64 = wasmu_getExportedFunction(module, "reinterpretToF64");

    ASSERT(reinterpretToF64, "Function not found");

    ASSERT(context->valueStack.position == 0, "Value stack is not at correct position");

    ASSERT(wasmu_runFunction(module, reinterpretToF64), "Error encountered while running function");

    ASSERT(context->valueStack.position == 8, "Value stack is not at correct position");
    ASSERT(context->typeStack.count == 1, "Type stack is not at correct count");

    ASSERT(wasmu_popFloat(context, WASMU_VALUE_TYPE_F64) == 123, "Result is not 123");
    ASSERT(wasmu_popType(context) == WASMU_VALUE_TYPE_F64, "Result type is not F64");

    PART("Destroy context");

    wasmu_destroyContext(context);

    PASS();
}