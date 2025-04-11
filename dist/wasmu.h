#ifndef WASMU_H_
#define WASMU_H_

// src/config.h

#ifndef WASMU_CONFIG_H_
#define WASMU_CONFIG_H_

#ifndef WASMU_USE_CUSTOM_TYPES

#include <stdint.h>

#define WASMU_BOOL int
#define WASMU_COUNT unsigned int
#define WASMU_U8 uint8_t
#define WASMU_I8 int8_t
#define WASMU_U16 uint16_t
#define WASMU_I16 int16_t
#define WASMU_U32 uint32_t
#define WASMU_I32 int32_t
#define WASMU_F32 float

#endif

#define WASMU_USE_STDLIB

#ifdef WASMU_USE_STDLIB

#include <stdio.h>
#include <stdlib.h>

#define WASMU_MALLOC malloc
#define WASMU_REALLOC realloc
#define WASMU_FREE free

#define WASMU_PRINTF printf

#endif

#ifdef WASMU_DEBUG
    #define WASMU_DEBUG_LOG(format, ...) WASMU_PRINTF(format "\n", ##__VA_ARGS__)
#else
    #define WASMU_DEBUG_LOG(...)
#endif

#endif

// src/common.h

typedef WASMU_BOOL wasmu_Bool;
typedef WASMU_COUNT wasmu_Count;
typedef WASMU_U8 wasmu_U8;
typedef WASMU_I8 wasmu_I8;
typedef WASMU_U16 wasmu_U16;
typedef WASMU_I16 wasmu_I16;
typedef WASMU_U32 wasmu_U32;
typedef WASMU_I32 wasmu_I32;
typedef WASMU_F32 wasmu_F32;

typedef WASMU_U32 wasmu_UInt;
typedef WASMU_I32 wasmu_Int;
typedef WASMU_F32 wasmu_Float;

#define WASMU_TRUE 1
#define WASMU_FALSE 0
#define WASMU_NULL 0

#define WASMU_NEW(type) (type*)WASMU_MALLOC(sizeof(type))

#define WASMU_READ(position) wasmu_read(module, position)
#define WASMU_NEXT() wasmu_readNext(module)
#define WASMU_AVAILABLE() (module->position < module->codeSize)

#define WASMU_INIT_ENTRIES(entriesPtr, countPtr) do { \
        entriesPtr = WASMU_MALLOC(0); \
        countPtr = 0; \
    } while (0)

#define WASMU_ADD_ENTRY(entriesPtr, countPtr, entry) do { \
        entriesPtr = WASMU_REALLOC(entriesPtr, sizeof((entriesPtr)[0]) * (++(countPtr))); \
        entriesPtr[(countPtr) - 1] = entry; \
    } while (0)

#define WASMU_GET_ENTRY(entriesPtr, countPtr, index) (index < countPtr ? &((entriesPtr)[index]) : WASMU_NULL)

wasmu_Bool wasmu_charsEqual(wasmu_U8* a, wasmu_U8* b) {
    wasmu_Count i = 0;

    while (WASMU_TRUE) {
        if (a[i] == '\0' && b[i] == '\0') {
            return WASMU_TRUE;
        }

        if (a[i] == '\0' || b[i] == '\0') {
            return WASMU_FALSE;
        }

        if (a[i] != b[i]) {
            return WASMU_FALSE;
        }

        i++;
    }
}

// src/opcodes.h

typedef enum {
    WASMU_OP_UNREACHABLE = 0x00,
    WASMU_OP_NOP = 0x01,
    WASMU_OP_BLOCK = 0x02,
    WASMU_OP_LOOP = 0x03,
    WASMU_OP_IF = 0x04,
    WASMU_OP_ELSE = 0x05,
    WASMU_OP_END = 0x0B,
    WASMU_OP_BR = 0x0C,
    WASMU_OP_BR_IF = 0x0D,
    WASMU_OP_BR_TABLE = 0x0E,
    WASMU_OP_RETURN = 0x0F,
    WASMU_OP_CALL = 0x10,
    WASMU_OP_CALL_INDIRECT = 0x11,
    WASMU_OP_DROP = 0x1A,
    WASMU_OP_SELECT = 0x1B,
    WASMU_OP_LOCAL_GET = 0x20,
    WASMU_OP_LOCAL_SET = 0x21,
    WASMU_OP_LOCAL_TEE = 0x22,
    WASMU_OP_GLOBAL_GET = 0x23,
    WASMU_OP_GLOBAL_SET = 0x24,
    WASMU_OP_I32_LOAD = 0x28,
    WASMU_OP_I64_LOAD = 0x29,
    WASMU_OP_F32_LOAD = 0x2A,
    WASMU_OP_F64_LOAD = 0x2B,
    WASMU_OP_I32_LOAD8_S = 0x2C,
    WASMU_OP_I32_LOAD8_U = 0x2D,
    WASMU_OP_I32_LOAD16_S = 0x2E,
    WASMU_OP_I32_LOAD16_U = 0x2F,
    WASMU_OP_I64_LOAD8_S = 0x30,
    WASMU_OP_I64_LOAD8_U = 0x31,
    WASMU_OP_I64_LOAD16_S = 0x32,
    WASMU_OP_I64_LOAD16_U = 0x33,
    WASMU_OP_I64_LOAD32_S = 0x34,
    WASMU_OP_I64_LOAD32_U = 0x35,
    WASMU_OP_I32_STORE = 0x36,
    WASMU_OP_I64_STORE = 0x37,
    WASMU_OP_F32_STORE = 0x38,
    WASMU_OP_F64_STORE = 0x39,
    WASMU_OP_I32_STORE8 = 0x3A,
    WASMU_OP_I32_STORE16 = 0x3B,
    WASMU_OP_I64_STORE8 = 0x3C,
    WASMU_OP_I64_STORE16 = 0x3D,
    WASMU_OP_I64_STORE32 = 0x3E,
    WASMU_OP_MEMORY_SIZE = 0x3F,
    WASMU_OP_MEMORY_GROW = 0x40,
    WASMU_OP_I32_CONST = 0x41,
    WASMU_OP_I64_CONST = 0x42,
    WASMU_OP_F32_CONST = 0x43,
    WASMU_OP_F64_CONST = 0x44,
    WASMU_OP_I32_EQZ = 0x45,
    WASMU_OP_I32_EQ = 0x46,
    WASMU_OP_I32_NE = 0x47,
    WASMU_OP_I32_LT_S = 0x48,
    WASMU_OP_I32_LT_U = 0x49,
    WASMU_OP_I32_GT_S = 0x4A,
    WASMU_OP_I32_GT_U = 0x4B,
    WASMU_OP_I32_LE_S = 0x4C,
    WASMU_OP_I32_LE_U = 0x4D,
    WASMU_OP_I32_GE_S = 0x4E,
    WASMU_OP_I32_GE_U = 0x4F,
    WASMU_OP_I64_EQZ = 0x50,
    WASMU_OP_I64_EQ = 0x51,
    WASMU_OP_I64_NE = 0x52,
    WASMU_OP_I64_LT_S = 0x53,
    WASMU_OP_I64_LT_U = 0x54,
    WASMU_OP_I64_GT_S = 0x55,
    WASMU_OP_I64_GT_U = 0x56,
    WASMU_OP_I64_LE_S = 0x57,
    WASMU_OP_I64_LE_U = 0x58,
    WASMU_OP_I64_GE_S = 0x59,
    WASMU_OP_I64_GE_U = 0x5A,
    WASMU_OP_F32_EQ = 0x5B,
    WASMU_OP_F32_NE = 0x5C,
    WASMU_OP_F32_LT = 0x5D,
    WASMU_OP_F32_GT = 0x5E,
    WASMU_OP_F32_LE = 0x5F,
    WASMU_OP_F32_GE = 0x60,
    WASMU_OP_F64_EQ = 0x61,
    WASMU_OP_F64_NE = 0x62,
    WASMU_OP_F64_LT = 0x63,
    WASMU_OP_F64_GT = 0x64,
    WASMU_OP_F64_LE = 0x65,
    WASMU_OP_F64_GE = 0x66,
    WASMU_OP_I32_CLZ = 0x67,
    WASMU_OP_I32_CTZ = 0x68,
    WASMU_OP_I32_POPCNT = 0x69,
    WASMU_OP_I32_ADD = 0x6A,
    WASMU_OP_I32_SUB = 0x6B,
    WASMU_OP_I32_MUL = 0x6C,
    WASMU_OP_I32_DIV_S = 0x6D,
    WASMU_OP_I32_DIV_U = 0x6E,
    WASMU_OP_I32_REM_S = 0x6F,
    WASMU_OP_I32_REM_U = 0x70,
    WASMU_OP_I32_AND = 0x71,
    WASMU_OP_I32_OR = 0x72,
    WASMU_OP_I32_XOR = 0x73,
    WASMU_OP_I32_SHL = 0x74,
    WASMU_OP_I32_SHR_S = 0x75,
    WASMU_OP_I32_SHR_U = 0x76,
    WASMU_OP_I32_ROTL = 0x77,
    WASMU_OP_I32_ROTR = 0x78,
    WASMU_OP_I64_CLZ = 0x79,
    WASMU_OP_I64_CTZ = 0x7A,
    WASMU_OP_I64_POPCNT = 0x7B,
    WASMU_OP_I64_ADD = 0x7C,
    WASMU_OP_I64_SUB = 0x7D,
    WASMU_OP_I64_MUL = 0x7E,
    WASMU_OP_I64_DIV_S = 0x7F,
    WASMU_OP_I64_DIV_U = 0x80,
    WASMU_OP_I64_REM_S = 0x81,
    WASMU_OP_I64_REM_U = 0x82,
    WASMU_OP_I64_AND = 0x83,
    WASMU_OP_I64_OR = 0x84,
    WASMU_OP_I64_XOR = 0x85,
    WASMU_OP_I64_SHL = 0x86,
    WASMU_OP_I64_SHR_S = 0x87,
    WASMU_OP_I64_SHR_U = 0x88,
    WASMU_OP_I64_ROTL = 0x89,
    WASMU_OP_I64_ROTR = 0x8A,
    WASMU_OP_F32_ABS = 0x8B,
    WASMU_OP_F32_NEG = 0x8C,
    WASMU_OP_F32_CEIL = 0x8D,
    WASMU_OP_F32_FLOOR = 0x8E,
    WASMU_OP_F32_TRUNC = 0x8F,
    WASMU_OP_F32_NEAREST = 0x90,
    WASMU_OP_F32_SQRT = 0x91,
    WASMU_OP_F32_ADD = 0x92,
    WASMU_OP_F32_SUB = 0x93,
    WASMU_OP_F32_MUL = 0x94,
    WASMU_OP_F32_DIV = 0x95,
    WASMU_OP_F32_MIN = 0x96,
    WASMU_OP_F32_MAX = 0x97,
    WASMU_OP_F32_COPYSIGN = 0x98,
    WASMU_OP_F64_ABS = 0x99,
    WASMU_OP_F64_NEG = 0x9A,
    WASMU_OP_F64_CEIL = 0x9B,
    WASMU_OP_F64_FLOOR = 0x9C,
    WASMU_OP_F64_TRUNC = 0x9D,
    WASMU_OP_F64_NEAREST = 0x9E,
    WASMU_OP_F64_SQRT = 0x9F,
    WASMU_OP_F64_ADD = 0xA0,
    WASMU_OP_F64_SUB = 0xA1,
    WASMU_OP_F64_MUL = 0xA2,
    WASMU_OP_F64_DIV = 0xA3,
    WASMU_OP_F64_MIN = 0xA4,
    WASMU_OP_F64_MAX = 0xA5,
    WASMU_OP_F64_COPYSIGN = 0xA6,
    WASMU_OP_I32_WRAP_I64 = 0xA7,
    WASMU_OP_I32_TRUNC_F32_S = 0xA8,
    WASMU_OP_I32_TRUNC_F32_U = 0xA9,
    WASMU_OP_I32_TRUNC_F64_S = 0xAA,
    WASMU_OP_I32_TRUNC_F64_U = 0xAB,
    WASMU_OP_I64_EXTEND_I32_S = 0xAC,
    WASMU_OP_I64_EXTEND_I32_U = 0xAD,
    WASMU_OP_I64_TRUNC_F32_S = 0xAE,
    WASMU_OP_I64_TRUNC_F32_U = 0xAF,
    WASMU_OP_I64_TRUNC_F64_S = 0xB0,
    WASMU_OP_I64_TRUNC_F64_U = 0xB1,
    WASMU_OP_F32_CONVERT_I32_S = 0xB2,
    WASMU_OP_F32_CONVERT_I32_U = 0xB3,
    WASMU_OP_F32_CONVERT_I64_S = 0xB4,
    WASMU_OP_F32_CONVERT_I64_U = 0xB5,
    WASMU_OP_F32_DEMOTE_F64 = 0xB6,
    WASMU_OP_F64_CONVERT_I32_S = 0xB7,
    WASMU_OP_F64_CONVERT_I32_U = 0xB8,
    WASMU_OP_F64_CONVERT_I64_S = 0xB9,
    WASMU_OP_F64_CONVERT_I64_U = 0xBA,
    WASMU_OP_F64_PROMOTE_F32 = 0xBB,
    WASMU_OP_I32_REINTERPRET_F32 = 0xBC,
    WASMU_OP_I64_REINTERPRET_F64 = 0xBD,
    WASMU_OP_F32_REINTERPRET_I32 = 0xBE,
    WASMU_OP_F64_REINTERPRET_I64 = 0xBF
} wasmu_Opcode;

// src/declarations.h

typedef enum {
    WASMU_ERROR_STATE_NONE = 0,
    WASMU_ERROR_STATE_NOT_IMPLEMENTED,
    WASMU_ERROR_STATE_MEMORY_OOB,
    WASMU_ERROR_STATE_CODE_BODY_MISMATCH,
    WASMU_ERROR_STATE_STACK_UNDERFLOW,
    WASMU_ERROR_STATE_TYPE_MISMATCH,
    WASMU_ERROR_STATE_REACHED_UNREACHABLE,
    WASMU_ERROR_STATE_INVALID_INDEX
} wasmu_ErrorState;

typedef enum {
    WASMU_VALUE_TYPE_I32 = 0x7F,
    WASMU_VALUE_TYPE_F32 = 0x7D
} wasmu_ValueType;

typedef enum {
    WASMU_LOCAL_TYPE_PARAMETER,
    WASMU_LOCAL_TYPE_RESULT,
    WASMU_LOCAL_TYPE_LOCAL
} wasmu_LocalType;

typedef enum {
    WASMU_SECTION_CUSTOM = 0,
    WASMU_SECTION_TYPE = 1,
    WASMU_SECTION_IMPORT = 2,
    WASMU_SECTION_FUNCTION = 3,
    WASMU_SECTION_TABLE = 4,
    WASMU_SECTION_MEMORY = 5,
    WASMU_SECTION_GLOBAL = 6,
    WASMU_SECTION_EXPORT = 7,
    WASMU_SECTION_START = 8,
    WASMU_SECTION_ELEMENT = 9,
    WASMU_SECTION_CODE = 10,
    WASMU_SECTION_DATA = 11,
    WASMU_SETCION_DATA_COUNT = 12
} wasmu_SectionType;

typedef enum wasmu_SignatureType {
    WASMU_SIGNATURE_TYPE_FUNCTION = 0x60
} wasmu_SignatureType;

typedef enum wasmu_ExportType {
    WASMU_EXPORT_TYPE_FUNCTION = 0x00
} wasmu_ExportType;

typedef struct wasmu_String {
    wasmu_Count size;
    wasmu_U8* chars;
} wasmu_String;

typedef struct wasmu_Call {
    wasmu_Count moduleIndex;
    wasmu_Count functionIndex;
    wasmu_Count position;
    wasmu_Count typeStackBase;
    wasmu_Count valueStackBase;
} wasmu_Call;

typedef struct wasmu_CallStack {
    wasmu_Call* calls;
    wasmu_Count size;
    wasmu_Count count;
} wasmu_CallStack;

typedef struct wasmu_Label {
    wasmu_Opcode opcode;
    wasmu_Count callIndex;
    wasmu_Count position;
} wasmu_Label;

typedef struct wasmu_LabelStack {
    wasmu_Label* labels;
    wasmu_Count size;
    wasmu_Count count;
} wasmu_LabelStack;

typedef struct wasmu_TypeStack {
    wasmu_ValueType* types;
    wasmu_Count size;
    wasmu_Count count;
} wasmu_TypeStack;

typedef struct wasmu_ValueStack {
    wasmu_U8* data;
    wasmu_Count size;
    wasmu_Count position;
} wasmu_ValueStack;

typedef struct wasmu_StackLocal {
    wasmu_Count position;
    wasmu_LocalType type;
    wasmu_ValueType valueType;
    wasmu_Count size;
} wasmu_StackLocal;

typedef struct wasmu_Context {
    wasmu_ErrorState errorState;
    struct wasmu_Module** modules;
    wasmu_Count modulesCount;
    wasmu_CallStack callStack;
    wasmu_LabelStack labelStack;
    wasmu_TypeStack typeStack;
    wasmu_ValueStack valueStack;
    struct wasmu_Module* activeModule;
    wasmu_Count activeModuleIndex;
    struct wasmu_Function* activeFunction;
    struct wasmu_FunctionSignature* activeFunctionSignature;
    wasmu_Count currentTypeStackBase;
    wasmu_Count currentValueStackBase;
    wasmu_StackLocal* currentStackLocals;
    wasmu_Count currentStackLocalsCount;
    wasmu_Bool fastForward;
    wasmu_Opcode fastForwardTargetOpcode;
    wasmu_Count fastForwardLabelDepth;
} wasmu_Context;

typedef struct wasmu_Module {
    wasmu_Context* context;
    wasmu_U8* code;
    wasmu_Count codeSize;
    wasmu_Count position;
    struct wasmu_CustomSection* customSections;
    wasmu_Count customSectionsCount;
    struct wasmu_FunctionSignature* functionSignatures;
    wasmu_Count functionSignaturesCount;
    struct wasmu_Function* functions;
    wasmu_Count functionsCount;
    struct wasmu_Export* exports;
    wasmu_Count exportsCount;
    wasmu_Count nextFunctionIndexForCode;
} wasmu_Module;

typedef struct wasmu_CustomSection {
    wasmu_String name;
    wasmu_Count dataPosition;
    wasmu_Count dataSize;
} wasmu_CustomSection;

typedef struct wasmu_FunctionSignature {
    wasmu_ValueType* parameters;
    wasmu_Count parametersCount;
    wasmu_Count parametersStackSize;
    wasmu_ValueType* results;
    wasmu_Count resultsCount;
} wasmu_FunctionSignature;

typedef struct wasmu_Function {
    wasmu_Count signatureIndex;
    wasmu_Count codePosition;
    wasmu_Count codeSize;
    wasmu_ValueType* locals;
    wasmu_Count localsCount;
} wasmu_Function;

typedef struct wasmu_Export {
    wasmu_String name;
    wasmu_ExportType type;
    union {
        wasmu_Count asFunctionIndex;
    } data;
} wasmu_Export;

wasmu_Context* wasmu_newContext();
wasmu_Bool wasmu_isRunning(wasmu_Context* context);

wasmu_Module* wasmu_newModule(wasmu_Context* context);
void wasmu_load(wasmu_Module* module, wasmu_U8* code, wasmu_Count codeSize);
wasmu_U8 wasmu_read(wasmu_Module* module, wasmu_Count position);
wasmu_U8 wasmu_readNext(wasmu_Module* module);
wasmu_UInt wasmu_readUInt(wasmu_Module* module);
wasmu_Int wasmu_readInt(wasmu_Module* module);
wasmu_String wasmu_readString(wasmu_Module* module);
wasmu_U8* wasmu_getNullTerminatedChars(wasmu_String string);
wasmu_Bool wasmu_stringEqualsChars(wasmu_String a, wasmu_U8* b);
wasmu_Count wasmu_getValueTypeSize(wasmu_ValueType type);
wasmu_Int wasmu_getExportedFunctionIndex(wasmu_Module* module, wasmu_U8* name);
wasmu_Function* wasmu_getExportedFunction(wasmu_Module* module, wasmu_U8* name);

wasmu_Bool wasmu_parseSections(wasmu_Module* module);

void wasmu_pushType(wasmu_Context* context, wasmu_ValueType type);
wasmu_ValueType wasmu_popType(wasmu_Context* context);
void wasmu_pushInt(wasmu_Context* context, wasmu_Count bytes, wasmu_Int value);
wasmu_Int wasmu_popInt(wasmu_Context* context, wasmu_Count bytes);
wasmu_Bool wasmu_callFunctionByIndex(wasmu_Context* context, wasmu_Count moduleIndex, wasmu_Count functionIndex);
wasmu_Bool wasmu_callFunction(wasmu_Module* module, wasmu_Function* function);
wasmu_Bool wasmu_fastForward(wasmu_Context* context, wasmu_Opcode targetOpcode, wasmu_Count* positionResult);
wasmu_Bool wasmu_step(wasmu_Context* context);
wasmu_Bool wasmu_runFunction(wasmu_Module* module, wasmu_Function* function);

// src/contexts.h

wasmu_Context* wasmu_newContext() {
    WASMU_DEBUG_LOG("Create new context");

    wasmu_Context* context = WASMU_NEW(wasmu_Context);

    context->errorState = WASMU_ERROR_STATE_NONE;
    context->callStack.calls = WASMU_MALLOC(0);
    context->callStack.size = 0;
    context->callStack.count = 0;
    context->labelStack.labels = WASMU_MALLOC(0);
    context->labelStack.size = 0;
    context->labelStack.count = 0;
    context->typeStack.types = WASMU_MALLOC(0);
    context->typeStack.size = 0;
    context->typeStack.count = 0;
    context->valueStack.data = WASMU_MALLOC(0);
    context->valueStack.size = 0;
    context->valueStack.position = 0;
    context->activeModule = WASMU_NULL;
    context->activeModuleIndex = 0;
    context->activeFunction = WASMU_NULL;
    context->activeFunctionSignature = WASMU_NULL;
    context->currentTypeStackBase = 0;
    context->currentValueStackBase = 0;
    context->currentStackLocals = WASMU_MALLOC(0);
    context->currentStackLocalsCount = 0;
    context->fastForward = WASMU_FALSE;
    context->fastForwardTargetOpcode = WASMU_OP_UNREACHABLE;
    context->fastForwardLabelDepth = 0;

    WASMU_INIT_ENTRIES(context->modules, context->modulesCount);

    return context;
}

wasmu_Bool wasmu_isRunning(wasmu_Context* context) {
    return context->callStack.count > 0;
}

// src/modules.h

wasmu_Module* wasmu_newModule(wasmu_Context* context) {
    wasmu_Module* module = WASMU_NEW(wasmu_Module);

    module->context = context;
    module->code = WASMU_NULL;
    module->codeSize = 0;
    module->position = 0;
    module->nextFunctionIndexForCode = 0;

    WASMU_INIT_ENTRIES(module->customSections, module->customSectionsCount);
    WASMU_INIT_ENTRIES(module->functionSignatures, module->functionSignaturesCount);
    WASMU_INIT_ENTRIES(module->functions, module->functionsCount);
    WASMU_INIT_ENTRIES(module->exports, module->exportsCount);

    WASMU_ADD_ENTRY(context->modules, context->modulesCount, module);

    return module;
}

void wasmu_load(wasmu_Module* module, wasmu_U8* code, wasmu_Count codeSize) {
    WASMU_DEBUG_LOG("Load code - size: %d", codeSize);

    module->code = code;
    module->codeSize = codeSize;
    module->position = 0;
}

wasmu_U8 wasmu_read(wasmu_Module* module, wasmu_Count position) {
    if (!module->code || position >= module->codeSize) {
        module->context->errorState = WASMU_ERROR_STATE_MEMORY_OOB;

        return 0;
    }

    return module->code[position];
}

wasmu_U8 wasmu_readNext(wasmu_Module* module) {
    return wasmu_read(module, module->position++);
}

// Unsigned integers are encoded as LEB128
// @source reference https://en.wikipedia.org/wiki/LEB128
// @licence ccbysa4
wasmu_UInt wasmu_readUInt(wasmu_Module* module) {
    wasmu_UInt result = 0;
    wasmu_UInt shift = 0;
    wasmu_U8 byte;

    do {
        byte = WASMU_NEXT();
        result |= (byte & 0b01111111) << shift;
        shift += 7;
    } while ((byte & 0b10000000) != 0);

    return result;
}

// Signed integers are encoded as LEB128
// @source reference https://en.wikipedia.org/wiki/LEB128
// @licence ccbysa4
wasmu_Int wasmu_readInt(wasmu_Module* module) {
    wasmu_Int result = 0;
    wasmu_UInt shift = 0;
    wasmu_U8 byte;

    do {
        byte = WASMU_NEXT();
        result |= (byte & 0b01111111) << shift;
        shift += 7;
    } while ((byte & 0b10000000) != 0);

    if (shift < sizeof(result) / 8 && result & 0b10000000) {
        result |= (~0 << shift);
    }

    return result;
}

wasmu_String wasmu_readString(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_U8* chars = WASMU_MALLOC(size);

    for (unsigned int i = 0; i < size; i++) {
        chars[i] = WASMU_NEXT();
    }

    return (wasmu_String) {
        .size = size,
        .chars = chars
    };
}

wasmu_U8* wasmu_getNullTerminatedChars(wasmu_String string) {
    wasmu_U8* chars = WASMU_MALLOC(string.size + 1);

    for (wasmu_Count i = 0; i < string.size; i++) {
        chars[i] = string.chars[i];
    }

    chars[string.size] = '\0';

    return chars;
}

wasmu_Bool wasmu_stringEqualsChars(wasmu_String a, wasmu_U8* b) {
    wasmu_U8* chars = wasmu_getNullTerminatedChars(a);
    wasmu_Bool result = wasmu_charsEqual(chars, b);

    free(chars);

    return result;
}

wasmu_Count wasmu_getValueTypeSize(wasmu_ValueType type) {
    switch (type) {
        case WASMU_VALUE_TYPE_I32:
        case WASMU_VALUE_TYPE_F32:
            return 4;

        default:
            WASMU_DEBUG_LOG("Unknown value type: 0x%02x", type);
            return 0;
    }
}

wasmu_Int wasmu_getExportedFunctionIndex(wasmu_Module* module, wasmu_U8* name) {
    for (wasmu_Count i = 0; i < module->exportsCount; i++) {
        wasmu_Export export = module->exports[i];

        if (export.type == WASMU_EXPORT_TYPE_FUNCTION && wasmu_stringEqualsChars(export.name, name)) {
            return export.data.asFunctionIndex;
        }
    }

    return -1;
}

wasmu_Function* wasmu_getExportedFunction(wasmu_Module* module, wasmu_U8* name) {
    wasmu_Int functionIndex = wasmu_getExportedFunctionIndex(module, name);

    if (functionIndex == -1) {
        return WASMU_NULL;
    }

    return WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);
}

// src/parser.h

wasmu_Bool wasmu_parseCustomSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count positionBeforeName = module->position;

    wasmu_CustomSection customSection;

    customSection.name = wasmu_readString(module);
    customSection.dataPosition = module->position;
    customSection.dataSize = size - (module->position - positionBeforeName);

    WASMU_ADD_ENTRY(module->customSections, module->customSectionsCount, customSection);

    #ifdef WASMU_DEBUG
        wasmu_U8* nameChars = wasmu_getNullTerminatedChars(customSection.name);

        WASMU_DEBUG_LOG(
            "Add custom section - name: \"%s\", dataPosition: 0x%08x, dataSize: %d",
            nameChars, customSection.dataPosition, customSection.dataSize
        );

        WASMU_FREE(nameChars);
    #endif

    module->position += customSection.dataSize;

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseTypesSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count typesCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < typesCount; i++) {
        switch (WASMU_NEXT()) {
            case WASMU_SIGNATURE_TYPE_FUNCTION:
            {
                WASMU_DEBUG_LOG("Signature type: function");

                wasmu_FunctionSignature signature;

                signature.parametersStackSize = 0;

                WASMU_INIT_ENTRIES(signature.parameters, signature.parametersCount);
                WASMU_INIT_ENTRIES(signature.results, signature.resultsCount);

                wasmu_Count parametersCount = wasmu_readUInt(module);

                for (wasmu_Count j = 0; j < parametersCount; j++) {
                    wasmu_ValueType parameterType = WASMU_NEXT();

                    signature.parametersStackSize += wasmu_getValueTypeSize(parameterType);

                    WASMU_ADD_ENTRY(signature.parameters, signature.parametersCount, parameterType);
                }

                wasmu_Count resultsCount = wasmu_readUInt(module);
                
                for (wasmu_Count j = 0; j < resultsCount; j++) {
                    WASMU_ADD_ENTRY(signature.results, signature.resultsCount, WASMU_NEXT());
                }

                WASMU_ADD_ENTRY(module->functionSignatures, module->functionSignaturesCount, signature);

                WASMU_DEBUG_LOG("Add function signature - parametersCount: %d, resultsCount: %d", signature.parametersCount, signature.resultsCount);

                break;
            }

            default:
                WASMU_DEBUG_LOG("Signature type not implemented");
                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseFunctionSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count functionsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < functionsCount; i++) {
        wasmu_Function function;

        function.signatureIndex = wasmu_readUInt(module);
        function.codePosition = 0;
        function.codeSize = 0;

        WASMU_INIT_ENTRIES(function.locals, function.localsCount);

        WASMU_ADD_ENTRY(module->functions, module->functionsCount, function);

        WASMU_DEBUG_LOG(
            "Add function - signature: %d, position: 0x%08x, size: %d, localsCount: %d",
            function.signatureIndex, function.codePosition, function.codeSize, function.localsCount
        );
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseExportSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count exportsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < exportsCount; i++) {
        wasmu_Export export;

        export.name = wasmu_readString(module);
        export.type = WASMU_NEXT();

        switch (export.type) {
            case WASMU_EXPORT_TYPE_FUNCTION:
            {
                WASMU_DEBUG_LOG("Export type: function");

                export.data.asFunctionIndex = wasmu_readUInt(module);

                #ifdef WASMU_DEBUG
                    wasmu_U8* nameChars = wasmu_getNullTerminatedChars(export.name);

                    WASMU_DEBUG_LOG("Add function export - name: \"%s\", functionIndex: %d", nameChars, export.data.asFunctionIndex);

                    WASMU_FREE(nameChars);
                #endif

                break;
            }

            default:
                WASMU_DEBUG_LOG("Export type not implemented");
                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }

        WASMU_ADD_ENTRY(module->exports, module->exportsCount, export);
    }
}

wasmu_Bool wasmu_parseCodeSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count bodiesCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < bodiesCount; i++) {
        wasmu_Count functionIndex = module->nextFunctionIndexForCode++;

        wasmu_Function* function = WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);

        if (!function) {
            WASMU_DEBUG_LOG("No function exists for code body");

            module->context->errorState = WASMU_ERROR_STATE_CODE_BODY_MISMATCH;

            return WASMU_FALSE;
        }

        function->codeSize = wasmu_readUInt(module);

        wasmu_Count positionBeforeLocals = module->position;

        wasmu_Count localDeclarationsCount = wasmu_readUInt(module);

        for (wasmu_Count j = 0; j < localDeclarationsCount; j++) {
            wasmu_Count typeLocalsCount = wasmu_readUInt(module);
            wasmu_ValueType type = WASMU_NEXT();

            WASMU_DEBUG_LOG("Add local declaration - type 0x%02x, count %d", type, typeLocalsCount);

            for (wasmu_Count k = 0; k < typeLocalsCount; k++) {
                WASMU_ADD_ENTRY(function->locals, function->localsCount, type);
            }
        }

        function->codePosition = module->position;

        WASMU_DEBUG_LOG("Add code - position: 0x%08x, size: %d (ends: 0x%08x)", function->codePosition, function->codeSize, function->codePosition + function->codeSize - 1);

        module->position = positionBeforeLocals + function->codeSize;
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseSections(wasmu_Module* module) {
    WASMU_DEBUG_LOG("Parse sections");

    static wasmu_U8 magic[] = {0x00, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00};

    for (wasmu_Count i = 0; i < sizeof(magic); i++) {
        if (WASMU_NEXT() != magic[i]) {
            WASMU_DEBUG_LOG("Invalid magic at 0x%08x", module->position - 1);

            return WASMU_FALSE;
        }
    }

    WASMU_DEBUG_LOG("Magic matched");

    while (WASMU_AVAILABLE()) {
        WASMU_DEBUG_LOG("Read section at 0x%08x", module->position);

        switch (WASMU_NEXT()) {
            case WASMU_SECTION_CUSTOM:
                WASMU_DEBUG_LOG("Section: custom");
                if (!wasmu_parseCustomSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_TYPE:
                WASMU_DEBUG_LOG("Section: type");
                if (!wasmu_parseTypesSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_FUNCTION:
                WASMU_DEBUG_LOG("Section: function");
                if (!wasmu_parseFunctionSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_EXPORT:
                WASMU_DEBUG_LOG("Section: export");
                if (!wasmu_parseExportSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_CODE:
                WASMU_DEBUG_LOG("Section: code");
                if (!wasmu_parseCodeSection(module)) {return WASMU_FALSE;}
                break;

            default:
                WASMU_DEBUG_LOG("Section type not implemented");
                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }

        WASMU_DEBUG_LOG("End of section");
    }

    return WASMU_TRUE;
}

// src/interpreter.h

#define WASMU_ASSERT_POP_TYPE(type) do { \
    wasmu_ValueType poppedType = wasmu_popType(context); \
    \
    if (poppedType != type) { \
        WASMU_DEBUG_LOG("Expected type 0x%02x but got 0x%02x", type, poppedType); \
        context->errorState = WASMU_ERROR_STATE_TYPE_MISMATCH; \
        return WASMU_FALSE; \
    } \
} while (0)

#define WASMU_FF_SKIP_HERE() if (context->fastForward) {break;}
#define WASMU_FF_STEP_IN() if (!wasmu_fastForwardStepInLabel(context)) {return WASMU_FALSE;}
#define WASMU_FF_STEP_OUT() if (!wasmu_fastForwardStepOutLabel(context)) {return WASMU_FALSE;}

wasmu_Bool wasmu_pushLabel(wasmu_Context* context, wasmu_Opcode opcode) {
    wasmu_LabelStack* stack = &context->labelStack;

    if (context->callStack.count == 0) {
        return WASMU_FALSE;
    }

    stack->count++;

    if (stack->count > stack->size) {
        stack->size = stack->count;
        stack->labels = WASMU_REALLOC(stack->labels, stack->size * sizeof(wasmu_Label));
    }

    stack->labels[stack->count - 1] = (wasmu_Label) {
        .opcode = opcode,
        .callIndex = context->callStack.count - 1,
        .position = context->activeModule->position
    };

    return WASMU_TRUE;
}

wasmu_Bool wasmu_popLabel(wasmu_Context* context, wasmu_Count targetCallIndex, wasmu_Label* returnedLabel) {
    wasmu_LabelStack* stack = &context->labelStack;

    if (stack->count == 0) {
        WASMU_DEBUG_LOG("Label stack underflow");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_FALSE;
    }

    wasmu_Label label = stack->labels[stack->count - 1];

    if (targetCallIndex != -1 && label.callIndex != targetCallIndex) {
        WASMU_DEBUG_LOG("Label stack underflow for this function call");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_FALSE;
    }

    if (returnedLabel) {
        *returnedLabel = label;
    }

    stack->count--;

    return WASMU_TRUE;
}

wasmu_Bool wasmu_getLabel(wasmu_Context* context, wasmu_Count labelIndex, wasmu_Count targetCallIndex, wasmu_Label* returnedLabel) {
    // Function does not throw error as callers may use it to check if a label exists

    wasmu_LabelStack* stack = &context->labelStack;

    if (labelIndex >= stack->count) {
        return WASMU_FALSE;
    }

    /*
        Label indexes start from the end of the label stack instead of the
        start, so they must be converted into absolute indexes to reference a
        label on the stack's data structure.
    */

    wasmu_Label label = stack->labels[stack->count - labelIndex - 1];

    if (targetCallIndex != -1 && label.callIndex != targetCallIndex) {
        return WASMU_FALSE;
    }

    if (returnedLabel) {
        *returnedLabel = label;
    }

    return WASMU_TRUE;
}

void wasmu_populateActiveCallInfo(wasmu_Context* context, wasmu_Call call) {
    /*
        This is done to cache pointers to the active module, function and
        function signature, instead of having to check the top of the call stack
        and then resolve indexes to pointers every time we want to step our
        context.
    */

    wasmu_Module** moduleEntry = WASMU_GET_ENTRY(context->modules, context->modulesCount, call.moduleIndex);
    wasmu_Module* module = *moduleEntry;

    context->activeModule = module;
    context->activeModuleIndex = call.moduleIndex;

    if (context->activeModule) {
        context->activeFunction = WASMU_GET_ENTRY(module->functions, module->functionsCount, call.functionIndex);
        context->activeFunctionSignature = WASMU_GET_ENTRY(module->functionSignatures, module->functionSignaturesCount, context->activeFunction->signatureIndex);

        context->currentStackLocals = WASMU_REALLOC(context->currentStackLocals, 0);
        context->currentStackLocalsCount = 0;

        wasmu_Count currentPosition = call.valueStackBase;

        for (wasmu_Count i = 0; i < context->activeFunctionSignature->parametersCount; i++) {
            wasmu_ValueType type = context->activeFunctionSignature->parameters[i];
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_StackLocal local = {
                .position = currentPosition,
                .type = WASMU_LOCAL_TYPE_PARAMETER,
                .valueType = type,
                .size = size
            };

            WASMU_ADD_ENTRY(context->currentStackLocals, context->currentStackLocalsCount, local);

            currentPosition += size;
        }

        for (wasmu_Count i = 0; i < context->activeFunctionSignature->resultsCount; i++) {
            wasmu_ValueType type = context->activeFunctionSignature->results[i];
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_StackLocal local = {
                .position = currentPosition,
                .type = WASMU_LOCAL_TYPE_RESULT,
                .valueType = type,
                .size = size
            };

            WASMU_ADD_ENTRY(context->currentStackLocals, context->currentStackLocalsCount, local);

            currentPosition += size;
        }

        for (wasmu_Count i = 0; i < context->activeFunction->localsCount; i++) {
            wasmu_ValueType type = context->activeFunction->locals[i];
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_StackLocal local = {
                .position = currentPosition,
                .type = WASMU_LOCAL_TYPE_LOCAL,
                .valueType = type,
                .size = size
            };

            WASMU_ADD_ENTRY(context->currentStackLocals, context->currentStackLocalsCount, local);

            currentPosition += size;
        }
    }

    context->currentTypeStackBase = call.typeStackBase;
    context->currentValueStackBase = call.valueStackBase;
}

void wasmu_pushCall(wasmu_Context* context, wasmu_Call call) {
    wasmu_CallStack* stack = &context->callStack;

    // TODO: Allow call stack to have a maximum size defined to catch infinite recursion
    stack->count++;

    if (stack->count > stack->size) {
        stack->size = stack->count;
        stack->calls = WASMU_REALLOC(stack->calls, stack->size * sizeof(wasmu_Call));
    }

    stack->calls[stack->count - 1] = call;

    wasmu_populateActiveCallInfo(context, call);
}

wasmu_Bool wasmu_popCall(wasmu_Context* context, wasmu_Call* returnedCall) {
    wasmu_CallStack* stack = &context->callStack;

    if (stack->count == 0) {
        WASMU_DEBUG_LOG("Call stack underflow");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_FALSE;
    }

    if (returnedCall) {
        *returnedCall = stack->calls[stack->count - 1];
    }

    stack->count--;

    while (context->labelStack.count > 0 && context->labelStack.labels[context->labelStack.count - 1].callIndex >= stack->count) {
        // Remove labels from label stack that reference a call that no longer exists
        wasmu_popLabel(context, -1, WASMU_NULL);
    }

    if (stack->count > 0) {
        wasmu_populateActiveCallInfo(context, stack->calls[stack->count - 1]);
    } else {
        context->activeModule = WASMU_NULL;
    }

    return WASMU_TRUE;
}

void wasmu_pushType(wasmu_Context* context, wasmu_ValueType type) {
    wasmu_TypeStack* stack = &context->typeStack;

    stack->count++;

    if (stack->count > stack->size) {
        stack->size = stack->count;
        stack->types = WASMU_REALLOC(stack->types, stack->size * sizeof(wasmu_ValueType));
    }

    stack->types[stack->count - 1] = type;
}

wasmu_ValueType wasmu_popType(wasmu_Context* context) {
    wasmu_TypeStack* stack = &context->typeStack;

    if (stack->count == 0) {
        WASMU_DEBUG_LOG("Type stack underflow");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_VALUE_TYPE_I32;
    }

    wasmu_ValueType type = stack->types[stack->count - 1];

    stack->count--;

    return type;
}

void wasmu_growValueStack(wasmu_ValueStack* stack, wasmu_Count newPosition) {
    if (newPosition > stack->size) {
        stack->size = newPosition;
        stack->data = WASMU_REALLOC(stack->data, stack->size);
    }
}

wasmu_Int wasmu_stackGetInt(wasmu_Context* context, wasmu_Count position, wasmu_Count bytes) {
    wasmu_Int value = 0;
    wasmu_ValueStack* stack = &context->valueStack;

    for (wasmu_Count i = 0; i < bytes; i++) {
        value |= stack->data[position + i] << (i * 8);
    }

    return value;
}

void wasmu_stackSetInt(wasmu_Context* context, wasmu_Count position, wasmu_Count bytes, wasmu_Int value) {
    wasmu_ValueStack* stack = &context->valueStack;

    wasmu_growValueStack(stack, position + bytes);

    for (wasmu_Count i = 0; i < bytes; i++) {
        stack->data[position + i] = value & 0xFF;
        value >>= 8;
    }
}

void wasmu_pushInt(wasmu_Context* context, wasmu_Count bytes, wasmu_Int value) {
    wasmu_ValueStack* stack = &context->valueStack;

    wasmu_stackSetInt(context, stack->position, bytes, value);

    stack->position += bytes;
}

wasmu_Int wasmu_popInt(wasmu_Context* context, wasmu_Count bytes) {
    wasmu_ValueStack* stack = &context->valueStack;

    if (stack->position < bytes) {
        WASMU_DEBUG_LOG("Value stack underflow");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return 0;
    }

    stack->position -= bytes;

    return wasmu_stackGetInt(context, stack->position, bytes);
}

wasmu_Bool wasmu_callFunctionByIndex(wasmu_Context* context, wasmu_Count moduleIndex, wasmu_Count functionIndex) {
    wasmu_Module** modulePtr = WASMU_GET_ENTRY(context->modules, context->modulesCount, moduleIndex);
    wasmu_Module* module = *modulePtr;

    if (!module) {
        return WASMU_FALSE;
    }

    wasmu_Function* function = WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);

    if (!function) {
        return WASMU_FALSE;
    }

    if (context->callStack.count > 0) {
        // Save current position on current topmost call
        context->callStack.calls[context->callStack.count - 1].position = module->position;
    }

    module->position = function->codePosition;

    wasmu_Count valueStackBase = context->valueStack.position;
    wasmu_FunctionSignature* signature = WASMU_GET_ENTRY(module->functionSignatures, module->functionSignaturesCount, function->signatureIndex);

    WASMU_DEBUG_LOG("Decrease stack base by %d for %d parameters", signature->parametersStackSize, signature->parametersCount);

    valueStackBase -= signature->parametersStackSize;

    for (wasmu_Count i = 0; i < function->localsCount; i++) {
        wasmu_Count typeSize = wasmu_getValueTypeSize(function->locals[i]);

        WASMU_DEBUG_LOG("Allocate local on stack - type: 0x%02x (typeSize: %d)", function->locals[i], typeSize);

        wasmu_pushInt(context, typeSize, 0);
    }

    if (context->typeStack.count < signature->parametersCount) {
        WASMU_DEBUG_LOG("Not enough items on value stack to call function (current: %d, needed: %d)", context->typeStack.count, signature->parametersCount);
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_FALSE;
    }

    wasmu_pushCall(context, (wasmu_Call) {
        .moduleIndex = moduleIndex,
        .functionIndex = functionIndex,
        .position = function->codePosition,
        .typeStackBase = context->typeStack.count - signature->parametersCount,
        .valueStackBase = valueStackBase
    });

    WASMU_DEBUG_LOG(
        "Pushed call - moduleIndex: %d, functionIndex: %d, position: 0x%08x, valueStackBase: %d (size: %d)",
        moduleIndex, functionIndex, function->codePosition, valueStackBase, function->codeSize
    );

    return WASMU_TRUE;
}

wasmu_Bool wasmu_callFunction(wasmu_Module* module, wasmu_Function* function) {
    wasmu_Context* context = module->context;
    wasmu_Count moduleIndex = -1;
    wasmu_Count functionIndex = -1;

    for (wasmu_Count i = 0; i < context->modulesCount; i++) {
        if (context->modules[i] == module) {
            moduleIndex = i;
            break;
        }
    }

    if (moduleIndex == -1) {
        return WASMU_FALSE;
    }

    for (wasmu_Count i = 0; i < module->functionsCount; i++) {
        if (&(module->functions[i]) == function) {
            functionIndex = i;
            break;
        }
    }

    if (functionIndex == -1) {
        return WASMU_FALSE;
    }

    return wasmu_callFunctionByIndex(context, moduleIndex, functionIndex);
}

void wasmu_returnFromFunction(wasmu_Context* context) {
    wasmu_ValueStack* stack = &context->valueStack;

    wasmu_Count resultsOffset = 0;
    wasmu_Count totalLocalsSize = 0;

    // First, get the total sizes of parameters, results and locals, popping all types

    for (wasmu_Count i = 0; i < context->currentStackLocalsCount; i++) {
        wasmu_StackLocal local = context->currentStackLocals[i];

        totalLocalsSize += local.size;

        if (local.type == WASMU_LOCAL_TYPE_PARAMETER || local.type == WASMU_LOCAL_TYPE_LOCAL) {
            resultsOffset += local.size;
        }
    }

    wasmu_Count base = context->currentValueStackBase;
    wasmu_Count nonLocalsSize = stack->position - base - totalLocalsSize;

    // Ensure that results offset also accounts for stack values related to parameters, results and locals

    if (nonLocalsSize > 0) {
        WASMU_DEBUG_LOG("Popping non-locals - size %d", nonLocalsSize);

        resultsOffset += nonLocalsSize;
    }

    // Then if there are non-result values, remove them from the stack to clean it, shifting the results up

    if (resultsOffset > 0) {
        for (wasmu_Count i = 0; i < resultsOffset; i++) {
            stack->data[base + i] = stack->data[base + resultsOffset + i];
        }

        stack->position -= resultsOffset;
    }

    // Finally, restore type stack base and re-add result types to type stack

    context->typeStack.count = context->currentTypeStackBase;

    for (wasmu_Count i = 0; i < context->currentStackLocalsCount; i++) {
        wasmu_StackLocal local = context->currentStackLocals[i];

        if (local.type == WASMU_LOCAL_TYPE_RESULT) {
            wasmu_pushType(context, local.valueType);
        }
    }

    // Now we can jump back to where we were in the calling function by popping from the call stack and restoring the position

    wasmu_popCall(context, WASMU_NULL);

    if (context->callStack.count > 0) {
        context->activeModule->position = context->callStack.calls[context->callStack.count - 1].position;
    }
}

wasmu_Bool wasmu_fastForward(wasmu_Context* context, wasmu_Opcode targetOpcode, wasmu_Count* positionResult) {
    WASMU_DEBUG_LOG("Begin fast forward - targetOpcode: 0x%02x", targetOpcode);

    context->fastForward = WASMU_TRUE;
    context->fastForwardTargetOpcode = targetOpcode;
    context->fastForwardLabelDepth = 0;

    while (context->fastForward) {
        if (!wasmu_step(context)) {
            context->fastForward = WASMU_FALSE;

            return WASMU_FALSE;
        }
    }

    WASMU_DEBUG_LOG("End fast forward - position: 0x%08x", context->activeModule->position);

    if (positionResult) {
        *positionResult = context->activeModule->position;
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_fastForwardStepInLabel(wasmu_Context* context) {
    if (context->fastForward) {
        context->fastForwardLabelDepth++;
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_fastForwardStepOutLabel(wasmu_Context* context) {
    if (context->fastForward) {
        if (context->fastForwardLabelDepth == 0) {
            WASMU_DEBUG_LOG("Label depth counter underflow while fast forwarding");
            context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
            return WASMU_FALSE;
        }

        context->fastForwardLabelDepth--;
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_step(wasmu_Context* context) {
    if (!wasmu_isRunning(context)) {
        return WASMU_FALSE;
    }

    wasmu_Module* module = context->activeModule;

    if (!module) {
        return WASMU_FALSE;
    }

    wasmu_U8 opcode = WASMU_NEXT();

    if (context->errorState == WASMU_ERROR_STATE_MEMORY_OOB) {
        WASMU_DEBUG_LOG("No more opcodes to execute");
        return WASMU_FALSE;
    }

    if (context->fastForward) {
        if (context->fastForwardLabelDepth == 0 && opcode == context->fastForwardTargetOpcode) {
            WASMU_DEBUG_LOG("Match opcode - opcode: 0x%02x, position: 0x%08x", opcode, module->position);

            module->position--;
            context->fastForward = WASMU_FALSE;

            return WASMU_TRUE;
        }

        WASMU_DEBUG_LOG("Skip over instruction - opcode: 0x%02x, position: 0x%08x", opcode, module->position);
    } else {
        WASMU_DEBUG_LOG("Execute instruction - opcode: 0x%02x, position: 0x%08x", opcode, module->position);
    }

    switch (opcode) {
        case WASMU_OP_UNREACHABLE:
        {
            WASMU_FF_SKIP_HERE();

            WASMU_DEBUG_LOG("Reached unreachable operation");
            context->errorState = WASMU_ERROR_STATE_REACHED_UNREACHABLE;
            return WASMU_FALSE;
        }

        case WASMU_OP_NOP:
            break;

        case WASMU_OP_BLOCK:
        case WASMU_OP_LOOP:
        {
            wasmu_U8 blockType = WASMU_NEXT();

            WASMU_FF_STEP_IN();
            WASMU_FF_SKIP_HERE();

            // TODO: Count number of results to return so stack can be cleaned when leaving block

            WASMU_DEBUG_LOG("Block/loop");

            wasmu_pushLabel(context, opcode);

            break;
        }

        case WASMU_OP_END:
        {
            WASMU_FF_STEP_OUT();

            wasmu_Label label;

            if (wasmu_getLabel(context, 0, context->callStack.count - 1, &label)) {
                WASMU_DEBUG_LOG("End - leave structured instruction");

                wasmu_popLabel(context, -1, WASMU_NULL);
            } else {
                WASMU_DEBUG_LOG("End - return from function");

                wasmu_returnFromFunction(context);
            }

            break;
        }

        case WASMU_OP_BR:
        case WASMU_OP_BR_IF:
        {
            wasmu_Count labelIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_Label label;

            WASMU_DEBUG_LOG("Break/break if - labelIndex: %d", labelIndex);

            if (!wasmu_getLabel(context, labelIndex, context->callStack.count - 1, &label)) {
                WASMU_DEBUG_LOG("Label stack underflow");
                context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
                return WASMU_FALSE;
            }

            if (opcode == WASMU_OP_BR_IF) {
                wasmu_Int condition = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

                WASMU_DEBUG_LOG("Break if - condition: %d", condition);

                if (!condition) {
                    break;
                }
            }

            WASMU_DEBUG_LOG("Checking label from position 0x%08x", label.position);

            module->position = label.position;

            if (label.opcode == WASMU_OP_LOOP) {
                // No need to fast forward since we're looping back to start
                break;
            }

            if (!wasmu_fastForward(context, WASMU_OP_END, WASMU_NULL)) {
                return WASMU_FALSE;
            }

            break;
        }

        case WASMU_OP_RETURN:
        {
            WASMU_FF_SKIP_HERE();

            WASMU_DEBUG_LOG("Return");
            wasmu_returnFromFunction(context);
            break;
        }

        case WASMU_OP_CALL:
        {
            wasmu_Count functionIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            WASMU_DEBUG_LOG("Call function - index: %d", functionIndex);

            if (!wasmu_callFunctionByIndex(context, context->activeModuleIndex, functionIndex)) {
                WASMU_DEBUG_LOG("Unknown function");
                context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            break;
        }

        case WASMU_OP_DROP:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_popType(context);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            WASMU_DEBUG_LOG("Drop - type: 0x%02x (size: %d)", type, size);

            if (context->valueStack.position < size) {
                WASMU_DEBUG_LOG("Value stack underflow");
                context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
                return WASMU_FALSE;
            }

            context->valueStack.position -= size;

            break;
        }

        case WASMU_OP_SELECT:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_Int condition = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
            wasmu_Count ifFalseSize = wasmu_getValueTypeSize(wasmu_popType(context));
            wasmu_Int ifFalse = wasmu_popInt(context, ifFalseSize);
            wasmu_Count ifTrueSize = wasmu_getValueTypeSize(wasmu_popType(context));
            wasmu_Int ifTrue = wasmu_popInt(context, ifTrueSize);

            WASMU_DEBUG_LOG("Select - ifTrue: %d, ifFalse: %d, condition: %d (ifTrueSize: %d, ifFalseSize: %d)", ifTrue, ifFalse, condition, ifTrueSize, ifFalseSize);

            if (condition) {
                wasmu_pushInt(context, ifTrueSize, ifTrue);
            } else {
                wasmu_pushInt(context, ifFalseSize, ifFalse);
            }

            break;
        }

        case WASMU_OP_LOCAL_GET:
        {
            wasmu_Count localIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_StackLocal* local = WASMU_GET_ENTRY(context->currentStackLocals, context->currentStackLocalsCount, localIndex);
            wasmu_Int value = wasmu_stackGetInt(context, local->position, local->size);

            WASMU_DEBUG_LOG("Get local - index: %d (position: 0x%08x, size: %d, value: %d)", localIndex, local->position, local->size, value);

            wasmu_pushInt(context, local->size, value);
            wasmu_pushType(context, local->valueType);

            break;
        }

        case WASMU_OP_LOCAL_SET:
        case WASMU_OP_LOCAL_TEE:
        {
            wasmu_Count localIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_StackLocal* local = WASMU_GET_ENTRY(context->currentStackLocals, context->currentStackLocalsCount, localIndex);
            wasmu_Int value = wasmu_popInt(context, local->size); WASMU_ASSERT_POP_TYPE(local->valueType);

            WASMU_DEBUG_LOG("Set local - index: %d (position: 0x%08x, size: %d, value: %d)", localIndex, local->position, local->size, value);

            wasmu_stackSetInt(context, local->position, local->size, value);

            if (opcode == WASMU_OP_LOCAL_TEE) {
                wasmu_pushInt(context, local->size, value);
                wasmu_pushType(context, local->valueType);
            }

            break;
        }

        case WASMU_OP_I32_CONST:
        {
            wasmu_I32 value = wasmu_readInt(module);

            WASMU_FF_SKIP_HERE();

            WASMU_DEBUG_LOG("I32 const - value: %d", value);

            wasmu_pushInt(context, 4, value);
            wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

            break;
        }

        case WASMU_OP_I32_ADD:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_I32 b = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
            wasmu_I32 a = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

            WASMU_DEBUG_LOG("Add I32 - a: %d, b: %d (result: %d)", a, b, a + b);

            wasmu_pushInt(context, 4, a + b);

            break;
        }

        case WASMU_OP_I32_SUB:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_I32 b = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
            wasmu_I32 a = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

            WASMU_DEBUG_LOG("Sub I32 - a: %d, b: %d (result: %d)", a, b, a - b);

            wasmu_pushInt(context, 4, a - b);

            break;
        }

        default:
            WASMU_DEBUG_LOG("Opcode not implemented");
            context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
            return WASMU_FALSE;
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_runFunction(wasmu_Module* module, wasmu_Function* function) {
    WASMU_DEBUG_LOG("Run function");

    wasmu_Bool called = wasmu_callFunction(module, function);

    if (!called) {
        return WASMU_FALSE;
    }

    while (wasmu_isRunning(module->context)) {
        WASMU_DEBUG_LOG("Step");

        if (!wasmu_step(module->context)) {
            return WASMU_FALSE;
        }
    }

    WASMU_DEBUG_LOG("Function returned");

    return WASMU_TRUE;
}

#endif
