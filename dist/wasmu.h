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
#define WASMU_U64 uint64_t
#define WASMU_I64 int64_t
#define WASMU_F64 double

#endif

#ifndef WASMU_USE_CUSTOM_STDLIB

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

#ifndef WASMU_MEMORY_BLOCK_SIZE
#define WASMU_MEMORY_BLOCK_SIZE 1024
#endif

#ifndef WASMU_IMPORT_RESOLUTION_DEPTH
#define WASMU_IMPORT_RESOLUTION_DEPTH 16
#endif

#ifndef WASMU_MAX_TABLE_SIZE
#define WASMU_MAX_TABLE_SIZE 1024
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
typedef WASMU_U64 wasmu_U64;
typedef WASMU_I64 wasmu_I64;
typedef WASMU_F64 wasmu_F64;

typedef WASMU_U64 wasmu_UInt;
typedef WASMU_I64 wasmu_Int;
typedef WASMU_F64 wasmu_Float;

#define WASMU_TRUE 1
#define WASMU_FALSE 0
#define WASMU_NULL 0

#define WASMU_NEW(type) (type*)WASMU_MALLOC(sizeof(type))

#define WASMU_READ(position) wasmu_read(module, position)
#define WASMU_NEXT() wasmu_readNext(module)
#define WASMU_AVAILABLE() (module->position < module->codeSize)

#define WASMU_INIT_ENTRIES(entriesPtr, countPtr) do { \
        entriesPtr = (__typeof__(entriesPtr))WASMU_MALLOC(0); \
        countPtr = 0; \
    } while (0)

#define WASMU_ADD_ENTRY(entriesPtr, countPtr, entry) do { \
        entriesPtr = (__typeof__(entriesPtr))WASMU_REALLOC(entriesPtr, sizeof((entriesPtr)[0]) * (++(countPtr))); \
        entriesPtr[(countPtr) - 1] = entry; \
    } while (0)

#define WASMU_GET_ENTRY(entriesPtr, countPtr, index) (index < countPtr ? &((entriesPtr)[index]) : WASMU_NULL)

wasmu_Bool wasmu_charsEqual(const wasmu_U8* a, const wasmu_U8* b) {
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

wasmu_U8* wasmu_copyChars(const wasmu_U8* source) {
    if (!source) {
        return WASMU_NULL;
    }

    wasmu_U8* copy = WASMU_MALLOC(1);

    if (!copy) {
        return WASMU_NULL;
    }

    copy[0] = '\0';

    wasmu_Count i = 0;

    while (*source) {
        copy[i++] = *source;
        copy = WASMU_REALLOC(copy, i + 1);

        if (!copy) {
            return WASMU_NULL;
        }

        copy[i] = '\0';

        source++;
    }

    return copy;
}

wasmu_Count wasmu_countLeadingZeros(wasmu_UInt value, wasmu_Count size) {
    wasmu_Count totalBits = size * 8;
    wasmu_Count bitsChecked = 0;
    wasmu_Count bitsAfter = 0;

    while (value > 0xFF) {
        value >>= 8;
        bitsChecked += 8;
        bitsAfter += 8;
    }

    while (bitsChecked < totalBits) {
        bitsChecked++;

        if (value & 1) {
            bitsAfter = bitsChecked;
        }

        value >>= 1;
    }

    return bitsChecked - bitsAfter;
}

wasmu_Count wasmu_countTrailingZeros(wasmu_UInt value, wasmu_Count size) {
    wasmu_Count bits = 0;

    while (value & 0xFF == 0) {
        value >>= 8;
        bits += 8;
    }

    while (value & 1 == 0) {
        value >>= 1;
        bits++;
    }

    if (bits > size * 8) {
        return size * 8;
    }

    return bits;
}

wasmu_Count wasmu_countOnes(wasmu_UInt value, wasmu_Count size) {
    wasmu_Count bits = 0;

    while (value) {
        if (value & 1) {
            bits++;
        }

        value >>= 1;
    }

    if (bits > size * 8) {
        return size * 8;
    }

    return bits;
}

// @source reference https://en.wikipedia.org/wiki/Circular_shift
// @licence ccbysa4
wasmu_UInt wasmu_rotateLeft(wasmu_UInt value, wasmu_Count size, wasmu_Count shift) {
    if ((shift &= (size * 8) - 1) == 0) {
        return value;
    }

    return (value << shift) | (value >> ((size * 8) - shift));
}

// @source reference https://en.wikipedia.org/wiki/Circular_shift
// @licence ccbysa4
wasmu_UInt wasmu_rotateRight(wasmu_UInt value, wasmu_Count size, wasmu_Count shift) {
    if ((shift &= (size * 8) - 1) == 0) {
        return value;
    }

    return (value >> shift) | (value << ((size * 8) - shift));
}

wasmu_Bool wasmu_isNan(wasmu_Float value) {
    return value != value;
}

// @source reference https://stackoverflow.com/a/2249173
// @licence ccbysa2.5
wasmu_Bool wasmu_isInfinity(wasmu_Float value) {
    return !wasmu_isNan(value) && wasmu_isNan(value - value);
}

void wasmu_signExtend(wasmu_Int* value, wasmu_Count bytes) {
    if (bytes == sizeof(wasmu_Int)) {
        return;
    }

    if (*value & ((wasmu_Int)1 << ((bytes * 8) - 1))) {
        wasmu_UInt mask = -1;

        mask <<= bytes * 8;
        *value |= mask;
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
    WASMU_ERROR_STATE_INVALID_INDEX,
    WASMU_ERROR_STATE_PRECONDITION_FAILED,
    WASMU_ERROR_STATE_DEPTH_EXCEEDED,
    WASMU_ERROR_STATE_IMPORT_NOT_FOUND,
    WASMU_ERROR_STATE_LIMIT_EXCEEDED
} wasmu_ErrorState;

typedef enum {
    WASMU_VALUE_TYPE_I32 = 0x7F,
    WASMU_VALUE_TYPE_I64 = 0x7E,
    WASMU_VALUE_TYPE_F32 = 0x7D,
    WASMU_VALUE_TYPE_F64 = 0x7C
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
    WASMU_SECTION_DATA_COUNT = 12
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

typedef struct wasmu_TypedValue {
    wasmu_ValueType type;
    union {
        wasmu_Int asInt;
        wasmu_Float asFloat;
    } value;
} wasmu_TypedValue;

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
    wasmu_Count resultsCount;
    wasmu_Count resultsSize;
    wasmu_Count typeStackBase;
    wasmu_Count valueStackBase;
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
    wasmu_U8* name;
    wasmu_U8* code;
    wasmu_Count codeSize;
    wasmu_Count position;
    struct wasmu_CustomSection* customSections;
    wasmu_Count customSectionsCount;
    struct wasmu_FunctionSignature* functionSignatures;
    wasmu_Count functionSignaturesCount;
    struct wasmu_Import* imports;
    wasmu_Count importsCount;
    struct wasmu_Function* functions;
    wasmu_Count functionsCount;
    struct wasmu_Table* tables;
    wasmu_Count tablesCount;
    struct wasmu_Memory* memories;
    wasmu_Count memoriesCount;
    wasmu_TypedValue* globals;
    wasmu_Count globalsCount;
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
    wasmu_Count resultsStackSize;
} wasmu_FunctionSignature;

typedef struct wasmu_Import {
    wasmu_String moduleName;
    wasmu_String name;
    wasmu_ExportType type;
    wasmu_Count resolvedModuleIndex;
    union {
        wasmu_Count asFunctionIndex;
    } data;
} wasmu_Import;

typedef wasmu_Bool (*wasmu_NativeFunction)(wasmu_Context* context);

typedef struct wasmu_Function {
    wasmu_Count signatureIndex;
    wasmu_Count importIndex;
    wasmu_Count codePosition;
    wasmu_Count codeSize;
    wasmu_NativeFunction nativeFunction;
    wasmu_ValueType* locals;
    wasmu_Count localsCount;
} wasmu_Function;

typedef struct wasmu_Table {
    wasmu_Count* entries;
    wasmu_Count entriesCount;
} wasmu_Table;

typedef struct wasmu_Memory {
    wasmu_U8* data;
    wasmu_Count size;
    wasmu_Count pagesCount;
    wasmu_Count minPages;
    wasmu_Count maxPages;
} wasmu_Memory;

typedef struct wasmu_Export {
    wasmu_String name;
    wasmu_ExportType type;
    union {
        wasmu_Count asFunctionIndex;
    } data;
} wasmu_Export;

typedef union wasmu_FloatConverter {
    wasmu_I32 asI32;
    wasmu_I64 asI64;
    wasmu_F32 asF32;
    wasmu_F64 asF64;
} wasmu_FloatConverter;

wasmu_Context* wasmu_newContext();
wasmu_Bool wasmu_isRunning(wasmu_Context* context);

wasmu_Module* wasmu_newModule(wasmu_Context* context);
void wasmu_load(wasmu_Module* module, wasmu_U8* code, wasmu_Count codeSize);
wasmu_U8 wasmu_read(wasmu_Module* module, wasmu_Count position);
wasmu_U8 wasmu_readNext(wasmu_Module* module);
wasmu_UInt wasmu_readUInt(wasmu_Module* module);
wasmu_Int wasmu_readInt(wasmu_Module* module);
wasmu_Float wasmu_readFloat(wasmu_Module* module, wasmu_ValueType type);
wasmu_String wasmu_readString(wasmu_Module* module);
wasmu_U8* wasmu_getNullTerminatedChars(wasmu_String string);
wasmu_Bool wasmu_stringEqualsChars(wasmu_String a, const wasmu_U8* b);
wasmu_Count wasmu_getValueTypeSize(wasmu_ValueType type);
wasmu_Int wasmu_getExportedFunctionIndex(wasmu_Module* module, const wasmu_U8* name);
wasmu_Function* wasmu_getExportedFunction(wasmu_Module* module, const wasmu_U8* name);
wasmu_Bool wasmu_resolveModuleImports(wasmu_Module* module);
wasmu_Bool wasmu_addNativeFunction(wasmu_Module* module, const wasmu_U8* name, wasmu_NativeFunction nativeFunction);

wasmu_Bool wasmu_parseSections(wasmu_Module* module);

wasmu_Bool wasmu_memoryLoad(wasmu_Memory* memory, wasmu_Count index, wasmu_U8 byteCount, wasmu_UInt* value);
wasmu_Bool wasmu_memoryStore(wasmu_Memory* memory, wasmu_Count index, wasmu_U8 byteCount, wasmu_UInt value);
wasmu_Count wasmu_getDataSizeFromOpcode(wasmu_Opcode opcode);
void wasmu_signExtendValue(wasmu_Opcode opcode, wasmu_UInt* value);

void wasmu_pushType(wasmu_Context* context, wasmu_ValueType type);
wasmu_ValueType wasmu_popType(wasmu_Context* context);
void wasmu_pushInt(wasmu_Context* context, wasmu_Count bytes, wasmu_Int value);
wasmu_Int wasmu_popInt(wasmu_Context* context, wasmu_Count bytes);

wasmu_Bool wasmu_callFunctionByIndex(wasmu_Context* context, wasmu_Count moduleIndex, wasmu_Count functionIndex);
wasmu_Bool wasmu_callFunction(wasmu_Module* module, wasmu_Function* function);

wasmu_Bool wasmu_fastForward(wasmu_Context* context, wasmu_Opcode targetOpcode, wasmu_Count* positionResult, wasmu_Bool errorOnSearchFail);
wasmu_Bool wasmu_step(wasmu_Context* context);
wasmu_Bool wasmu_runFunction(wasmu_Module* module, wasmu_Function* function);

// src/subjects.h

wasmu_ValueType wasmu_getOpcodeSubjectType(wasmu_Opcode opcode) {
    switch (opcode) {
        case WASMU_OP_I32_LOAD:
        case WASMU_OP_I32_LOAD8_S:
        case WASMU_OP_I32_LOAD8_U:
        case WASMU_OP_I32_LOAD16_S:
        case WASMU_OP_I32_LOAD16_U:
        case WASMU_OP_I32_STORE:
        case WASMU_OP_I32_STORE8:
        case WASMU_OP_I32_STORE16:
        case WASMU_OP_I32_CONST:
        case WASMU_OP_I32_EQZ:
        case WASMU_OP_I32_EQ:
        case WASMU_OP_I32_NE:
        case WASMU_OP_I32_LT_S:
        case WASMU_OP_I32_LT_U:
        case WASMU_OP_I32_GT_S:
        case WASMU_OP_I32_GT_U:
        case WASMU_OP_I32_LE_S:
        case WASMU_OP_I32_LE_U:
        case WASMU_OP_I32_GE_S:
        case WASMU_OP_I32_GE_U:
        case WASMU_OP_I32_CLZ:
        case WASMU_OP_I32_CTZ:
        case WASMU_OP_I32_POPCNT:
        case WASMU_OP_I32_ADD:
        case WASMU_OP_I32_SUB:
        case WASMU_OP_I32_MUL:
        case WASMU_OP_I32_DIV_S:
        case WASMU_OP_I32_DIV_U:
        case WASMU_OP_I32_REM_S:
        case WASMU_OP_I32_REM_U:
        case WASMU_OP_I32_AND:
        case WASMU_OP_I32_OR:
        case WASMU_OP_I32_XOR:
        case WASMU_OP_I32_SHL:
        case WASMU_OP_I32_SHR_S:
        case WASMU_OP_I32_SHR_U:
        case WASMU_OP_I32_ROTL:
        case WASMU_OP_I32_ROTR:
        case WASMU_OP_I32_WRAP_I64:
        case WASMU_OP_I32_TRUNC_F32_S:
        case WASMU_OP_I32_TRUNC_F32_U:
        case WASMU_OP_I32_TRUNC_F64_S:
        case WASMU_OP_I32_TRUNC_F64_U:
        case WASMU_OP_I32_REINTERPRET_F32:
            return WASMU_VALUE_TYPE_I32;

        case WASMU_OP_I64_LOAD:
        case WASMU_OP_I64_LOAD8_S:
        case WASMU_OP_I64_LOAD8_U:
        case WASMU_OP_I64_LOAD16_S:
        case WASMU_OP_I64_LOAD16_U:
        case WASMU_OP_I64_LOAD32_S:
        case WASMU_OP_I64_LOAD32_U:
        case WASMU_OP_I64_STORE:
        case WASMU_OP_I64_STORE8:
        case WASMU_OP_I64_STORE16:
        case WASMU_OP_I64_STORE32:
        case WASMU_OP_I64_CONST:
        case WASMU_OP_I64_EQZ:
        case WASMU_OP_I64_EQ:
        case WASMU_OP_I64_NE:
        case WASMU_OP_I64_LT_S:
        case WASMU_OP_I64_LT_U:
        case WASMU_OP_I64_GT_S:
        case WASMU_OP_I64_GT_U:
        case WASMU_OP_I64_LE_S:
        case WASMU_OP_I64_LE_U:
        case WASMU_OP_I64_GE_S:
        case WASMU_OP_I64_GE_U:
        case WASMU_OP_I64_CLZ:
        case WASMU_OP_I64_CTZ:
        case WASMU_OP_I64_POPCNT:
        case WASMU_OP_I64_ADD:
        case WASMU_OP_I64_SUB:
        case WASMU_OP_I64_MUL:
        case WASMU_OP_I64_DIV_S:
        case WASMU_OP_I64_DIV_U:
        case WASMU_OP_I64_REM_S:
        case WASMU_OP_I64_REM_U:
        case WASMU_OP_I64_AND:
        case WASMU_OP_I64_OR:
        case WASMU_OP_I64_XOR:
        case WASMU_OP_I64_SHL:
        case WASMU_OP_I64_SHR_S:
        case WASMU_OP_I64_SHR_U:
        case WASMU_OP_I64_ROTL:
        case WASMU_OP_I64_ROTR:
        case WASMU_OP_I64_EXTEND_I32_S:
        case WASMU_OP_I64_EXTEND_I32_U:
        case WASMU_OP_I64_TRUNC_F32_S:
        case WASMU_OP_I64_TRUNC_F32_U:
        case WASMU_OP_I64_TRUNC_F64_S:
        case WASMU_OP_I64_TRUNC_F64_U:
        case WASMU_OP_I64_REINTERPRET_F64:
            return WASMU_VALUE_TYPE_I64;

        case WASMU_OP_F32_LOAD:
        case WASMU_OP_F32_STORE:
        case WASMU_OP_F32_CONST:
        case WASMU_OP_F32_EQ:
        case WASMU_OP_F32_NE:
        case WASMU_OP_F32_LT:
        case WASMU_OP_F32_GT:
        case WASMU_OP_F32_LE:
        case WASMU_OP_F32_GE:
        case WASMU_OP_F32_ABS:
        case WASMU_OP_F32_NEG:
        case WASMU_OP_F32_CEIL:
        case WASMU_OP_F32_FLOOR:
        case WASMU_OP_F32_TRUNC:
        case WASMU_OP_F32_NEAREST:
        case WASMU_OP_F32_SQRT:
        case WASMU_OP_F32_ADD:
        case WASMU_OP_F32_SUB:
        case WASMU_OP_F32_MUL:
        case WASMU_OP_F32_DIV:
        case WASMU_OP_F32_MIN:
        case WASMU_OP_F32_MAX:
        case WASMU_OP_F32_COPYSIGN:
        case WASMU_OP_F32_CONVERT_I32_S:
        case WASMU_OP_F32_CONVERT_I32_U:
        case WASMU_OP_F32_CONVERT_I64_S:
        case WASMU_OP_F32_CONVERT_I64_U:
        case WASMU_OP_F32_DEMOTE_F64:
        case WASMU_OP_F32_REINTERPRET_I32:
            return WASMU_VALUE_TYPE_F32;

        case WASMU_OP_F64_LOAD:
        case WASMU_OP_F64_STORE:
        case WASMU_OP_F64_CONST:
        case WASMU_OP_F64_EQ:
        case WASMU_OP_F64_NE:
        case WASMU_OP_F64_LT:
        case WASMU_OP_F64_GT:
        case WASMU_OP_F64_LE:
        case WASMU_OP_F64_GE:
        case WASMU_OP_F64_ABS:
        case WASMU_OP_F64_NEG:
        case WASMU_OP_F64_CEIL:
        case WASMU_OP_F64_FLOOR:
        case WASMU_OP_F64_TRUNC:
        case WASMU_OP_F64_NEAREST:
        case WASMU_OP_F64_SQRT:
        case WASMU_OP_F64_ADD:
        case WASMU_OP_F64_SUB:
        case WASMU_OP_F64_MUL:
        case WASMU_OP_F64_DIV:
        case WASMU_OP_F64_MIN:
        case WASMU_OP_F64_MAX:
        case WASMU_OP_F64_COPYSIGN:
        case WASMU_OP_F64_CONVERT_I32_S:
        case WASMU_OP_F64_CONVERT_I32_U:
        case WASMU_OP_F64_CONVERT_I64_S:
        case WASMU_OP_F64_CONVERT_I64_U:
        case WASMU_OP_F64_PROMOTE_F32:
        case WASMU_OP_F64_REINTERPRET_I64:
            return WASMU_VALUE_TYPE_F64;

        default:
            return WASMU_VALUE_TYPE_I32;
    }
}

wasmu_ValueType wasmu_getOpcodeObjectType(wasmu_Opcode opcode) {
    switch (opcode) {
        case WASMU_OP_I64_EXTEND_I32_S:
        case WASMU_OP_I64_EXTEND_I32_U:
        case WASMU_OP_F32_CONVERT_I32_S:
        case WASMU_OP_F32_CONVERT_I32_U:
        case WASMU_OP_F64_CONVERT_I32_S:
        case WASMU_OP_F64_CONVERT_I32_U:
        case WASMU_OP_F32_REINTERPRET_I32:
            return WASMU_VALUE_TYPE_I32;

        case WASMU_OP_I32_WRAP_I64:
        case WASMU_OP_F32_CONVERT_I64_S:
        case WASMU_OP_F32_CONVERT_I64_U:
        case WASMU_OP_F64_CONVERT_I64_S:
        case WASMU_OP_F64_CONVERT_I64_U:
        case WASMU_OP_F64_REINTERPRET_I64:
            return WASMU_VALUE_TYPE_I64;

        case WASMU_OP_I32_TRUNC_F32_S:
        case WASMU_OP_I32_TRUNC_F32_U:
        case WASMU_OP_I64_TRUNC_F32_S:
        case WASMU_OP_I64_TRUNC_F32_U:
        case WASMU_OP_I32_REINTERPRET_F32:
        case WASMU_OP_F64_PROMOTE_F32:
            return WASMU_VALUE_TYPE_F32;

        case WASMU_OP_I32_TRUNC_F64_S:
        case WASMU_OP_I32_TRUNC_F64_U:
        case WASMU_OP_I64_TRUNC_F64_S:
        case WASMU_OP_I64_TRUNC_F64_U:
        case WASMU_OP_I64_REINTERPRET_F64:
        case WASMU_OP_F32_DEMOTE_F64:
            return WASMU_VALUE_TYPE_F64;


        default:
            return WASMU_VALUE_TYPE_I32;
    }
}

wasmu_Bool wasmu_opcodeIsSigned(wasmu_Opcode opcode) {
    switch (opcode) {
        case WASMU_OP_I32_LOAD8_S:
        case WASMU_OP_I32_LOAD16_S:
        case WASMU_OP_I32_LT_S:
        case WASMU_OP_I32_GT_S:
        case WASMU_OP_I32_LE_S:
        case WASMU_OP_I32_GE_S:
        case WASMU_OP_I32_DIV_S:
        case WASMU_OP_I32_REM_S:
        case WASMU_OP_I32_SHR_S:
        case WASMU_OP_I32_TRUNC_F32_S:
        case WASMU_OP_I32_TRUNC_F64_S:
        case WASMU_OP_I64_LOAD8_S:
        case WASMU_OP_I64_LOAD16_S:
        case WASMU_OP_I64_LOAD32_S:
        case WASMU_OP_I64_LT_S:
        case WASMU_OP_I64_GT_S:
        case WASMU_OP_I64_LE_S:
        case WASMU_OP_I64_GE_S:
        case WASMU_OP_I64_DIV_S:
        case WASMU_OP_I64_REM_S:
        case WASMU_OP_I64_SHR_S:
        case WASMU_OP_I64_EXTEND_I32_S:
        case WASMU_OP_I64_TRUNC_F32_S:
        case WASMU_OP_I64_TRUNC_F64_S:
        case WASMU_OP_F32_CONVERT_I32_S:
        case WASMU_OP_F32_CONVERT_I64_S:
        case WASMU_OP_F64_CONVERT_I32_S:
        case WASMU_OP_F64_CONVERT_I64_S:
            return WASMU_TRUE;

        default:
            return WASMU_FALSE;
    }
}

// src/contexts.h

wasmu_Context* wasmu_newContext() {
    WASMU_DEBUG_LOG("Create new context");

    wasmu_Context* context = WASMU_NEW(wasmu_Context);

    context->errorState = WASMU_ERROR_STATE_NONE;
    context->callStack.calls = (wasmu_Call*)WASMU_MALLOC(0);
    context->callStack.size = 0;
    context->callStack.count = 0;
    context->labelStack.labels = (wasmu_Label*)WASMU_MALLOC(0);
    context->labelStack.size = 0;
    context->labelStack.count = 0;
    context->typeStack.types = (wasmu_ValueType*)WASMU_MALLOC(0);
    context->typeStack.size = 0;
    context->typeStack.count = 0;
    context->valueStack.data = (wasmu_U8*)WASMU_MALLOC(0);
    context->valueStack.size = 0;
    context->valueStack.position = 0;
    context->activeModule = WASMU_NULL;
    context->activeModuleIndex = 0;
    context->activeFunction = WASMU_NULL;
    context->activeFunctionSignature = WASMU_NULL;
    context->currentTypeStackBase = 0;
    context->currentValueStackBase = 0;
    context->currentStackLocals = (wasmu_StackLocal*)WASMU_MALLOC(0);
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
    module->name = WASMU_NULL;
    module->code = WASMU_NULL;
    module->codeSize = 0;
    module->position = 0;
    module->nextFunctionIndexForCode = 0;

    WASMU_INIT_ENTRIES(module->customSections, module->customSectionsCount);
    WASMU_INIT_ENTRIES(module->functionSignatures, module->functionSignaturesCount);
    WASMU_INIT_ENTRIES(module->imports, module->importsCount);
    WASMU_INIT_ENTRIES(module->functions, module->functionsCount);
    WASMU_INIT_ENTRIES(module->tables, module->tablesCount);
    WASMU_INIT_ENTRIES(module->memories, module->memoriesCount);
    WASMU_INIT_ENTRIES(module->globals, module->globalsCount);
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

void wasmu_assignModuleName(wasmu_Module* module, const wasmu_U8* name) {
    module->name = wasmu_copyChars(name);
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
        result |= ((wasmu_Int)byte & 0b01111111) << shift;
        shift += 7;
    } while ((byte & 0b10000000) != 0);

    if (shift < sizeof(result) * 8 && (byte & 0b01000000) != 0) {
        result |= ((wasmu_Int)(~0) << shift);
    }

    return result;
}

wasmu_Float wasmu_readFloat(wasmu_Module* module, wasmu_ValueType type) {
    wasmu_Count size = wasmu_getValueTypeSize(type);
    wasmu_UInt rawValue = 0;

    for (wasmu_Count i = 0; i < size; i++) {
        rawValue |= (wasmu_UInt)WASMU_NEXT() << (i * 8);
    }

    switch (type) {
        case WASMU_VALUE_TYPE_F32:
        default:
            return ((wasmu_FloatConverter) {.asI32 = rawValue}).asF32;

        case WASMU_VALUE_TYPE_F64:
            return ((wasmu_FloatConverter) {.asI64 = rawValue}).asF64;
    }
}

wasmu_String wasmu_readString(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_U8* chars = (wasmu_U8*)WASMU_MALLOC(size);

    for (unsigned int i = 0; i < size; i++) {
        chars[i] = WASMU_NEXT();
    }

    return (wasmu_String) {
        .size = size,
        .chars = chars
    };
}

wasmu_U8* wasmu_getNullTerminatedChars(wasmu_String string) {
    wasmu_U8* chars = (wasmu_U8*)WASMU_MALLOC(string.size + 1);

    for (wasmu_Count i = 0; i < string.size; i++) {
        chars[i] = string.chars[i];
    }

    chars[string.size] = '\0';

    return chars;
}

wasmu_String wasmu_charsToString(const wasmu_U8* chars) {
    wasmu_U8* copy = wasmu_copyChars(chars);
    wasmu_Count size = 0;

    while (chars[size]) {
        size++;
    }

    return (wasmu_String) {
        .size = size,
        .chars = copy
    };
}

wasmu_Bool wasmu_stringEqualsChars(wasmu_String a, const wasmu_U8* b) {
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

        case WASMU_VALUE_TYPE_I64:
        case WASMU_VALUE_TYPE_F64:
            return 8;

        default:
            WASMU_DEBUG_LOG("Unknown value type: 0x%02x", type);
            return 0;
    }
}

wasmu_Int wasmu_getExportedFunctionIndex(wasmu_Module* module, const wasmu_U8* name) {
    for (wasmu_Count i = 0; i < module->exportsCount; i++) {
        wasmu_Export moduleExport = module->exports[i];

        if (moduleExport.type == WASMU_EXPORT_TYPE_FUNCTION && wasmu_stringEqualsChars(moduleExport.name, name)) {
            return moduleExport.data.asFunctionIndex;
        }
    }

    return -1;
}

wasmu_Function* wasmu_getExportedFunction(wasmu_Module* module, const wasmu_U8* name) {
    wasmu_Count functionIndex = wasmu_getExportedFunctionIndex(module, name);

    if (functionIndex == -1) {
        return WASMU_NULL;
    }

    return WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);
}

wasmu_Bool wasmu_resolveModuleImportData(wasmu_Import* import, wasmu_Module* resolvedModule) {
    wasmu_U8* name = wasmu_getNullTerminatedChars(import->name);
    wasmu_Bool isSuccess = WASMU_FALSE;

    switch (import->type) {
        case WASMU_EXPORT_TYPE_FUNCTION: {
            wasmu_Count functionIndex = wasmu_getExportedFunctionIndex(resolvedModule, name);

            if (functionIndex == -1) {
                WASMU_DEBUG_LOG("Unable to resolve imported function - name: %s", name);
                resolvedModule->context->errorState = WASMU_ERROR_STATE_IMPORT_NOT_FOUND;
                goto exit;
            }

            import->data.asFunctionIndex = functionIndex;

            break;
        }

        default:
            WASMU_DEBUG_LOG("Import type not implemented when resolving");
            resolvedModule->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
            goto exit;
    }

    isSuccess = WASMU_TRUE;

    exit:

    WASMU_FREE(name);

    return isSuccess;
}

wasmu_Bool wasmu_resolveModuleImports(wasmu_Module* module) {
    wasmu_Context* context = module->context;

    for (wasmu_Count i = 0; i < module->importsCount; i++) {
        wasmu_Import* moduleImport = &module->imports[i];

        if (moduleImport->resolvedModuleIndex != -1) {
            continue;
        }

        wasmu_U8* targetModuleName = wasmu_getNullTerminatedChars(moduleImport->moduleName);

        for (wasmu_Count j = 0; j < context->modulesCount; j++) {
            wasmu_Module* currentModule = context->modules[j];

            if (currentModule->name && wasmu_charsEqual(currentModule->name, targetModuleName)) {
                if (!wasmu_resolveModuleImportData(moduleImport, currentModule)) {
                    return WASMU_FALSE;
                }

                moduleImport->resolvedModuleIndex = j;

                break;
            }
        }

        if (moduleImport->resolvedModuleIndex == -1) {
            WASMU_DEBUG_LOG("Unable to resolve module import - moduleName: %s", targetModuleName);
            context->errorState = WASMU_ERROR_STATE_IMPORT_NOT_FOUND;
            return WASMU_FALSE;
        }

        WASMU_FREE(targetModuleName);
    }
}

wasmu_Bool wasmu_addNativeFunction(wasmu_Module* module, const wasmu_U8* name, wasmu_NativeFunction nativeFunction) {
    wasmu_Function function;

    function.importIndex = -1;
    function.nativeFunction = nativeFunction;

    WASMU_ADD_ENTRY(module->functions, module->functionsCount, function);

    wasmu_Export moduleExport;

    moduleExport.name = wasmu_charsToString(name);
    moduleExport.type = WASMU_EXPORT_TYPE_FUNCTION;
    moduleExport.data.asFunctionIndex = module->functionsCount - 1;

    WASMU_ADD_ENTRY(module->exports, module->exportsCount, moduleExport);

    return WASMU_TRUE;
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
                signature.resultsStackSize = 0;

                WASMU_INIT_ENTRIES(signature.parameters, signature.parametersCount);
                WASMU_INIT_ENTRIES(signature.results, signature.resultsCount);

                wasmu_Count parametersCount = wasmu_readUInt(module);

                for (wasmu_Count j = 0; j < parametersCount; j++) {
                    wasmu_ValueType parameterType = (wasmu_ValueType)WASMU_NEXT();

                    signature.parametersStackSize += wasmu_getValueTypeSize(parameterType);

                    WASMU_ADD_ENTRY(signature.parameters, signature.parametersCount, parameterType);
                }

                wasmu_Count resultsCount = wasmu_readUInt(module);
                
                for (wasmu_Count j = 0; j < resultsCount; j++) {
                    wasmu_ValueType resultType = (wasmu_ValueType)WASMU_NEXT();

                    signature.resultsStackSize += wasmu_getValueTypeSize(resultType);

                    WASMU_ADD_ENTRY(signature.results, signature.resultsCount, resultType);
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

wasmu_Bool wasmu_parseImportSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count importsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < importsCount; i++) {
        wasmu_Import moduleImport;

        moduleImport.moduleName = wasmu_readString(module);
        moduleImport.name = wasmu_readString(module);
        moduleImport.type = (wasmu_ExportType)WASMU_NEXT();
        moduleImport.resolvedModuleIndex = -1;

        switch (moduleImport.type) {
            case WASMU_EXPORT_TYPE_FUNCTION:
            {
                WASMU_DEBUG_LOG("Import type: function");

                wasmu_Function function;

                function.signatureIndex = wasmu_readUInt(module);
                function.importIndex = module->importsCount;
                function.nativeFunction = WASMU_NULL;

                WASMU_ADD_ENTRY(module->functions, module->functionsCount, function);

                #ifdef WASMU_DEBUG
                    wasmu_U8* moduleNameChars = wasmu_getNullTerminatedChars(moduleImport.moduleName);
                    wasmu_U8* nameChars = wasmu_getNullTerminatedChars(moduleImport.name);

                    WASMU_DEBUG_LOG(
                        "Add function import - moduleName: \"%s\", name: \"%s\", functionIndex: %d, signatureIndex: %d",
                        moduleNameChars, nameChars, module->functionsCount - 1, function.signatureIndex
                    );

                    WASMU_FREE(moduleNameChars);
                    WASMU_FREE(nameChars);
                #endif

                break;
            }

            default:
                WASMU_DEBUG_LOG("Import type not implemented");
                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }

        WASMU_ADD_ENTRY(module->imports, module->importsCount, moduleImport);
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseFunctionSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count functionsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < functionsCount; i++) {
        wasmu_Function function;

        function.signatureIndex = wasmu_readUInt(module);
        function.importIndex = -1;
        function.nativeFunction = WASMU_NULL;
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

wasmu_Bool wasmu_parseTableSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count tablesCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < tablesCount; i++) {
        wasmu_Table table;

        WASMU_INIT_ENTRIES(table.entries, table.entriesCount);

        WASMU_ADD_ENTRY(module->tables, module->tablesCount, table);

        WASMU_NEXT(); // Table type — just assume it's a `funcref` for now
        WASMU_NEXT(); // Limits flags — not required for now
        WASMU_NEXT(); // Initial size — also not required; table can grow when needed

        WASMU_DEBUG_LOG("Add table");
    }
}

wasmu_Bool wasmu_parseMemorySection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count memoriesCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < memoriesCount; i++) {
        wasmu_Memory memory;

        memory.data = (wasmu_U8*)WASMU_MALLOC(0);
        memory.size = 0;

        wasmu_U8 limitsFlag = WASMU_NEXT();

        memory.pagesCount = memory.minPages = wasmu_readUInt(module);
        memory.maxPages = limitsFlag == 0x01 ? wasmu_readUInt(module) : -1;

        WASMU_ADD_ENTRY(module->memories, module->memoriesCount, memory);

        WASMU_DEBUG_LOG("Add memory - minPages: %d, maxPages: %d", memory.minPages, memory.maxPages);
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseGlobalSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count globalsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < globalsCount; i++) {
        wasmu_TypedValue global;

        global.type = (wasmu_ValueType)WASMU_NEXT();

        WASMU_NEXT(); // Mutability — not required for now; just assume everything is mutable
        WASMU_NEXT(); // Const opcode — also not required until global importing is implemented

        switch (global.type) {
            case WASMU_VALUE_TYPE_I32:
                global.value.asInt = wasmu_readInt(module);
                WASMU_DEBUG_LOG("Add global - type: %d, value: %d", global.type, global.value.asInt);
                break;

            default:
                WASMU_DEBUG_LOG("Global value type not implemented");
                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }

        WASMU_NEXT(); // End opcode

        WASMU_ADD_ENTRY(module->globals, module->globalsCount, global);
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseExportSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count exportsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < exportsCount; i++) {
        wasmu_Export moduleExport;

        moduleExport.name = wasmu_readString(module);
        moduleExport.type = (wasmu_ExportType)WASMU_NEXT();

        switch (moduleExport.type) {
            case WASMU_EXPORT_TYPE_FUNCTION:
            {
                WASMU_DEBUG_LOG("Export type: function");

                moduleExport.data.asFunctionIndex = wasmu_readUInt(module);

                #ifdef WASMU_DEBUG
                    wasmu_U8* nameChars = wasmu_getNullTerminatedChars(moduleExport.name);

                    WASMU_DEBUG_LOG("Add function export - name: \"%s\", functionIndex: %d", nameChars, moduleExport.data.asFunctionIndex);

                    WASMU_FREE(nameChars);
                #endif

                break;
            }

            default:
                WASMU_DEBUG_LOG("Export type not implemented");
                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }

        WASMU_ADD_ENTRY(module->exports, module->exportsCount, moduleExport);
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseElementSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count elementSegmentsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < elementSegmentsCount; i++) {
        wasmu_Table* table = WASMU_GET_ENTRY(module->tables, module->tablesCount, 0);

        if (!table) {
            WASMU_DEBUG_LOG("No table exists for element segment");
            module->context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
            return WASMU_FALSE;
        }

        WASMU_NEXT(); // Segment flags — not required
        WASMU_NEXT(); // Const opcode — also not required

        wasmu_Count startIndex = wasmu_readUInt(module);

        WASMU_NEXT(); // End opcode

        WASMU_DEBUG_LOG("Start index: %d", startIndex);

        while (table->entriesCount < startIndex) {
            if (table->entriesCount == WASMU_MAX_TABLE_SIZE) {
                WASMU_DEBUG_LOG("Start index is past max table size");
                module->context->errorState = WASMU_ERROR_STATE_LIMIT_EXCEEDED;
            }

            WASMU_ADD_ENTRY(table->entries, table->entriesCount, -1);
        }

        wasmu_Count elementsCount = wasmu_readUInt(module);

        WASMU_DEBUG_LOG("Elements count: %d", elementsCount);

        for (wasmu_Count i = 0; i < elementsCount; i++) {
            wasmu_Count entry = wasmu_readUInt(module);

            WASMU_DEBUG_LOG("Add table element - index: %d, entry: %d", startIndex + i, entry);

            if (table->entriesCount == WASMU_MAX_TABLE_SIZE) {
                WASMU_DEBUG_LOG("Max table size reached");
                module->context->errorState = WASMU_ERROR_STATE_LIMIT_EXCEEDED;
            }

            if (table->entriesCount <= startIndex + i) {
                WASMU_ADD_ENTRY(table->entries, table->entriesCount, entry);
            } else {
                table->entries[startIndex + i] = entry;
            }
        }
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseCodeSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count bodiesCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < bodiesCount; i++) {
        wasmu_Count functionIndex = module->nextFunctionIndexForCode++;
        wasmu_Function* function = WASMU_NULL;

        while (WASMU_TRUE) {
            function = WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);

            if (!function) {
                WASMU_DEBUG_LOG("No function exists for code body");
                module->context->errorState = WASMU_ERROR_STATE_CODE_BODY_MISMATCH;
                return WASMU_FALSE;
            }

            if (function->importIndex == -1) {
                break;
            }

            functionIndex = module->nextFunctionIndexForCode++;
        }

        function->codeSize = wasmu_readUInt(module);

        wasmu_Count positionBeforeLocals = module->position;

        wasmu_Count localDeclarationsCount = wasmu_readUInt(module);

        for (wasmu_Count j = 0; j < localDeclarationsCount; j++) {
            wasmu_Count typeLocalsCount = wasmu_readUInt(module);
            wasmu_ValueType type = (wasmu_ValueType)WASMU_NEXT();

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

wasmu_Bool wasmu_parseDataSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count dataSegmentsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < dataSegmentsCount; i++) {
        wasmu_Memory* memory = WASMU_GET_ENTRY(module->memories, module->memoriesCount, 0);

        if (!memory) {
            WASMU_DEBUG_LOG("No memory is defined");
            module->context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
            return WASMU_FALSE;
        }

        WASMU_NEXT(); // Segment flags — not required
        WASMU_NEXT(); // Const opcode — also not required

        wasmu_Count startIndex = wasmu_readUInt(module);

        WASMU_NEXT(); // End opcode

        wasmu_Count dataSize = wasmu_readUInt(module);

        for (wasmu_Count j = 0; j < dataSize; j++) {
            wasmu_memoryStore(memory, startIndex + j, 1, WASMU_NEXT());
        }
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseDataCountSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count dataCount = wasmu_readUInt(module);

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

            case WASMU_SECTION_IMPORT:
                WASMU_DEBUG_LOG("Section: import");
                if (!wasmu_parseImportSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_FUNCTION:
                WASMU_DEBUG_LOG("Section: function");
                if (!wasmu_parseFunctionSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_TABLE:
                WASMU_DEBUG_LOG("Section: table");
                if (!wasmu_parseTableSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_MEMORY:
                WASMU_DEBUG_LOG("Section: memory");
                if (!wasmu_parseMemorySection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_GLOBAL:
                WASMU_DEBUG_LOG("Section: global");
                if (!wasmu_parseGlobalSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_EXPORT:
                WASMU_DEBUG_LOG("Section: export");
                if (!wasmu_parseExportSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_ELEMENT:
                WASMU_DEBUG_LOG("Section: element");
                if (!wasmu_parseElementSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_CODE:
                WASMU_DEBUG_LOG("Section: code");
                if (!wasmu_parseCodeSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_DATA:
                WASMU_DEBUG_LOG("Section: data");
                if (!wasmu_parseDataSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_DATA_COUNT:
                WASMU_DEBUG_LOG("Section: data count");
                if (!wasmu_parseDataCountSection(module)) {return WASMU_FALSE;}
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

// src/memory.h

#define WASMU_MEMORY_PAGE_SIZE 65536

#define WASMU_MEMORY_ALIGN_BLOCK(value) (((value) + WASMU_MEMORY_BLOCK_SIZE - 1) & ~(WASMU_MEMORY_BLOCK_SIZE - 1))

wasmu_Bool wasmu_memoryLoad(wasmu_Memory* memory, wasmu_Count index, wasmu_U8 byteCount, wasmu_UInt* value) {
    *value = 0;

    for (wasmu_Count i = 0; i < byteCount; i++) {
        if (index >= memory->size) {
            return WASMU_TRUE;
        }

        *value |= memory->data[index++] << (i * 8);
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_memoryStore(wasmu_Memory* memory, wasmu_Count index, wasmu_U8 byteCount, wasmu_UInt value) {
    for (wasmu_Count i = 0; i < byteCount; i++) {
        if (index >= memory->size) {
            wasmu_Count newPagesCount = index / WASMU_MEMORY_PAGE_SIZE;

            if (newPagesCount > memory->maxPages) {
                return WASMU_FALSE;
            }

            if (newPagesCount > memory->pagesCount) {
                memory->pagesCount = newPagesCount;
            }

            memory->size = WASMU_MEMORY_ALIGN_BLOCK(index);
            memory->data = (wasmu_U8*)WASMU_REALLOC(memory->data, memory->size);

            WASMU_DEBUG_LOG("Increase memory size to %d", memory->size);
        }

        memory->data[index++] = value & 0xFF;
        value >>= 8;
    }
}

wasmu_Count wasmu_getDataSizeFromOpcode(wasmu_Opcode opcode) {
    switch (opcode) {
        case WASMU_OP_I32_LOAD8_S:
        case WASMU_OP_I32_LOAD8_U:
        case WASMU_OP_I64_LOAD8_S:
        case WASMU_OP_I64_LOAD8_U:
        case WASMU_OP_I32_STORE8:
        case WASMU_OP_I64_STORE8:
            return 1;

        case WASMU_OP_I32_LOAD16_S:
        case WASMU_OP_I32_LOAD16_U:
        case WASMU_OP_I64_LOAD16_S:
        case WASMU_OP_I64_LOAD16_U:
        case WASMU_OP_I32_STORE16:
        case WASMU_OP_I64_STORE16:
            return 2;

        case WASMU_OP_I64_LOAD32_S:
        case WASMU_OP_I64_LOAD32_U:
        case WASMU_OP_I64_STORE32:
            return 4;

        default:
            return wasmu_getValueTypeSize(wasmu_getOpcodeSubjectType(opcode));
    }
}

void wasmu_signExtendValue(wasmu_Opcode opcode, wasmu_UInt* value) {
    switch (opcode) {
        case WASMU_OP_I32_LOAD8_S:
        case WASMU_OP_I64_LOAD8_S:
        case WASMU_OP_I32_LOAD16_S:
        case WASMU_OP_I64_LOAD16_S:
        case WASMU_OP_I64_LOAD32_S:
            break;

        default: return;
    }

    wasmu_Count dataSize = wasmu_getDataSizeFromOpcode(opcode);
    wasmu_UInt sign = *value >> (dataSize * 8) - 1;

    if (!sign) {
        return;
    }

    wasmu_UInt mask = -1;

    *value |= (mask << (dataSize * 8));
}

// src/stacks.h

#define WASMU_ASSERT_POP_TYPE(type) do { \
    wasmu_ValueType poppedType = wasmu_popType(context); \
    \
    if (poppedType != type) { \
        WASMU_DEBUG_LOG("Expected type 0x%02x but got 0x%02x", type, poppedType); \
        context->errorState = WASMU_ERROR_STATE_TYPE_MISMATCH; \
        return WASMU_FALSE; \
    } \
} while (0)

wasmu_Bool wasmu_pushLabel(wasmu_Context* context, wasmu_Opcode opcode, wasmu_Count resultsCount, wasmu_Count resultsSize) {
    wasmu_LabelStack* stack = &context->labelStack;

    if (context->callStack.count == 0) {
        return WASMU_FALSE;
    }

    stack->count++;

    if (stack->count > stack->size) {
        stack->size = stack->count;
        stack->labels = (wasmu_Label*)WASMU_REALLOC(stack->labels, stack->size * sizeof(wasmu_Label));
    }

    stack->labels[stack->count - 1] = (wasmu_Label) {
        .opcode = opcode,
        .callIndex = context->callStack.count - 1,
        .position = context->activeModule->position,
        .resultsCount = resultsCount,
        .resultsSize = resultsSize,
        .typeStackBase = context->typeStack.count,
        .valueStackBase = context->valueStack.position
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

        context->currentStackLocals = (wasmu_StackLocal*)WASMU_REALLOC(context->currentStackLocals, 0);
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
        stack->calls = (wasmu_Call*)WASMU_REALLOC(stack->calls, stack->size * sizeof(wasmu_Call));
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
        stack->types = (wasmu_ValueType*)WASMU_REALLOC(stack->types, stack->size * sizeof(wasmu_ValueType));
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
        stack->data = (wasmu_U8*)WASMU_REALLOC(stack->data, stack->size);
    }
}

wasmu_Int wasmu_stackGetInt(wasmu_Context* context, wasmu_Count position, wasmu_Count bytes) {
    wasmu_Int value = 0;
    wasmu_ValueStack* stack = &context->valueStack;

    for (wasmu_Count i = 0; i < bytes; i++) {
        value |= (wasmu_Int)(stack->data[position + i]) << (i * 8);
    }

    wasmu_signExtend(&value, bytes);

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

void wasmu_pushFloat(wasmu_Context* context, wasmu_ValueType type, wasmu_Float value) {
    if (wasmu_isNan(value) || wasmu_isInfinity(value)) {
        value = 0;
    }

    wasmu_FloatConverter converter;

    switch (type) {
        case WASMU_VALUE_TYPE_F32:
        default:
            converter.asF32 = value;
            wasmu_pushInt(context, 4, converter.asI32);
            break;

        case WASMU_VALUE_TYPE_F64:
            converter.asF64 = value;
            wasmu_pushInt(context, 8, converter.asI64);
            break;
    }
}

wasmu_Float wasmu_popFloat(wasmu_Context* context, wasmu_ValueType type) {
    wasmu_FloatConverter converter;

    switch (type) {
        case WASMU_VALUE_TYPE_F32:
        default:
            converter.asI32 = wasmu_popInt(context, 4);
            return converter.asF32;

        case WASMU_VALUE_TYPE_F64:
            converter.asI64 = wasmu_popInt(context, 8);
            return converter.asF64;
    }
}

// src/functions.h

wasmu_Bool wasmu_callFunctionByIndex(wasmu_Context* context, wasmu_Count moduleIndex, wasmu_Count functionIndex) {
    wasmu_Module** modulePtr = WASMU_GET_ENTRY(context->modules, context->modulesCount, moduleIndex);
    wasmu_Module* module = *modulePtr;
    wasmu_Module* callingModule = module;

    if (!module) {
        return WASMU_FALSE;
    }

    wasmu_Function* function = WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);

    if (!function) {
        WASMU_DEBUG_LOG("Unknown function");
        context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
        return WASMU_FALSE;
    }

    wasmu_Count resolutionDepth = 0;

    while (function->importIndex != -1) {
        if (resolutionDepth >= WASMU_IMPORT_RESOLUTION_DEPTH) {
            WASMU_DEBUG_LOG("Import resolution depth exceeded");
            context->errorState = WASMU_ERROR_STATE_DEPTH_EXCEEDED;
            return WASMU_FALSE;
        }

        resolutionDepth++;

        wasmu_Import* moduleImport = WASMU_GET_ENTRY(module->imports, module->importsCount, function->importIndex);

        if (!moduleImport || moduleImport->type != WASMU_EXPORT_TYPE_FUNCTION) {
            WASMU_DEBUG_LOG("Unknown import");
            context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
            return WASMU_FALSE;
        }

        if (moduleImport->resolvedModuleIndex == -1) {
            WASMU_DEBUG_LOG("Import has not yet been resolved");
            context->errorState = WASMU_ERROR_STATE_PRECONDITION_FAILED;
            return WASMU_FALSE;
        }

        moduleIndex = moduleImport->resolvedModuleIndex;
        module = *WASMU_GET_ENTRY(context->modules, context->modulesCount, moduleIndex);

        if (!module) {
            WASMU_DEBUG_LOG("Unknown resolved module");
            context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
            return WASMU_FALSE;
        }

        functionIndex = moduleImport->data.asFunctionIndex;
        function = WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);

        if (!function) {
            WASMU_DEBUG_LOG("Unknown resolved function");
            context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
            return WASMU_FALSE;
        }
    }

    if (function->nativeFunction) {
        WASMU_DEBUG_LOG("Call native function");

        return function->nativeFunction(context);
    }

    if (context->callStack.count > 0) {
        // Save current position on current topmost call
        context->callStack.calls[context->callStack.count - 1].position = callingModule->position;
    }

    module->position = function->codePosition;

    wasmu_Count valueStackBase = context->valueStack.position;
    wasmu_FunctionSignature* signature = WASMU_GET_ENTRY(module->functionSignatures, module->functionSignaturesCount, function->signatureIndex);

    if (!signature) {
        WASMU_DEBUG_LOG("Unknown function signature");
        context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
        return WASMU_FALSE;
    }

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
        WASMU_DEBUG_LOG("Module does not exist in context");
        module->context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
        return WASMU_FALSE;
    }

    for (wasmu_Count i = 0; i < module->functionsCount; i++) {
        if (&(module->functions[i]) == function) {
            functionIndex = i;
            break;
        }
    }

    if (functionIndex == -1) {
        WASMU_DEBUG_LOG("Function does not exist in module");
        module->context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
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
        WASMU_DEBUG_LOG("Popping non-locals - size: %d", nonLocalsSize);

        resultsOffset += nonLocalsSize;
    }

    // Then if there are non-result values, remove them from the stack to clean it, shifting the results up

    for (wasmu_Count i = 0; i < resultsOffset; i++) {
        stack->data[base + i] = stack->data[base + resultsOffset + i];
    }

    stack->position -= resultsOffset;

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

// src/interpreter.h

#define WASMU_FF_SKIP_HERE() if (context->fastForward) {break;}
#define WASMU_FF_STEP_IN() if (!wasmu_fastForwardStepInLabel(context)) {return WASMU_FALSE;}
#define WASMU_FF_STEP_OUT() if (!wasmu_fastForwardStepOutLabel(context)) {return WASMU_FALSE;}

#define WASMU_INT_OPERATOR(baseType, operator) { \
        WASMU_FF_SKIP_HERE(); \
        \
        wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode); \
        wasmu_Count size = wasmu_getValueTypeSize(type); \
        \
        baseType b = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type); \
        baseType a = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type); \
        \
        WASMU_DEBUG_LOG("Operator " #operator " - a: %ld, b: %ld (result: %ld)", a, b, a operator b); \
        \
        wasmu_pushInt(context, size, a operator b); \
        wasmu_pushType(context, type); \
        \
        break; \
    }

#define WASMU_FLOAT_OPERATOR(operator) { \
        WASMU_FF_SKIP_HERE(); \
        \
        wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode); \
        \
        wasmu_Float b = wasmu_popFloat(context, type); WASMU_ASSERT_POP_TYPE(type); \
        wasmu_Float a = wasmu_popFloat(context, type); WASMU_ASSERT_POP_TYPE(type); \
        \
        WASMU_DEBUG_LOG("Operator " #operator " - a: %f, b: %f (result: %f)", a, b, a operator b); \
        \
        wasmu_pushFloat(context, type, a operator b); \
        wasmu_pushType(context, type); \
        \
        break; \
    }

wasmu_Bool wasmu_fastForward(wasmu_Context* context, wasmu_Opcode targetOpcode, wasmu_Count* positionResult, wasmu_Bool errorOnSearchFail) {
    WASMU_DEBUG_LOG("Begin fast forward - targetOpcode: 0x%02x", targetOpcode);

    context->fastForward = WASMU_TRUE;
    context->fastForwardTargetOpcode = targetOpcode;
    context->fastForwardLabelDepth = 0;

    while (context->fastForward) {
        if (!wasmu_step(context)) {
            if (!errorOnSearchFail && context->errorState == WASMU_ERROR_STATE_STACK_UNDERFLOW) {
                // Opcode not found, so ignore error
                context->errorState = WASMU_ERROR_STATE_NONE;
            }

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
        WASMU_DEBUG_LOG("No more opcodes to execute - position: 0x%08x", module->position);
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
        case WASMU_OP_IF:
        {
            wasmu_U8 blockType = WASMU_READ(module->position);
            wasmu_Count resultsCount = 0;
            wasmu_Count resultsSize = 0;

            if (blockType == 0x40) {
                // Shorthand for void result type
                WASMU_NEXT();
            } else if (blockType >= 0x7C && blockType <= 0x7F) {
                // Shorthand for single-value result types
                resultsCount = 1;
                resultsSize = wasmu_getValueTypeSize((wasmu_ValueType)blockType);
                WASMU_NEXT();
            } else {
                wasmu_Count signatureIndex = wasmu_readUInt(module);
                wasmu_FunctionSignature* signature = WASMU_GET_ENTRY(module->functionSignatures, module->functionSignaturesCount, signatureIndex);

                if (!signature) {
                    WASMU_DEBUG_LOG("Unknown function signature");
                    context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                    return WASMU_FALSE;
                }

                resultsCount = signature->resultsCount;
                resultsSize = signature->resultsStackSize;
            }

            WASMU_FF_STEP_IN();
            WASMU_FF_SKIP_HERE();

            WASMU_DEBUG_LOG("Block/loop/if - resultsCount: %d, resultsSize: %d", resultsCount, resultsSize);

            wasmu_pushLabel(context, (wasmu_Opcode)opcode, resultsCount, resultsSize);

            if (opcode == WASMU_OP_IF) {
                wasmu_Int condition = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

                WASMU_DEBUG_LOG("If - condition: %d", condition);

                if (!condition) {
                    wasmu_Count originalPosition = module->position;

                    if (wasmu_fastForward(context, WASMU_OP_ELSE, WASMU_NULL, WASMU_FALSE)) {
                        // Jump to code after `else` but don't actually execute `else` opcode
                        WASMU_DEBUG_LOG("Jump to else");
                        WASMU_NEXT();

                        break;
                    }

                    WASMU_DEBUG_LOG("Jump to end");

                    module->position = originalPosition;

                    if (!wasmu_fastForward(context, WASMU_OP_END, WASMU_NULL, WASMU_TRUE)) {
                        return WASMU_FALSE;
                    }
                }
            }

            break;
        }

        case WASMU_OP_ELSE:
        {
            WASMU_DEBUG_LOG("Else");

            if (!wasmu_fastForward(context, WASMU_OP_END, WASMU_NULL, WASMU_TRUE)) {
                return WASMU_FALSE;
            }
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
        case WASMU_OP_BR_TABLE:
        {
            // Get the current label and decide whether to branch

            wasmu_Count labelIndex;

            if (opcode == WASMU_OP_BR_TABLE) {
                wasmu_Count targets = wasmu_readUInt(module);
                wasmu_Count targetIndex = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
                wasmu_Bool foundTarget = WASMU_FALSE;

                for (wasmu_Count i = 0; i < targets; i++) {
                    wasmu_Count targetLabelIndex = wasmu_readUInt(module);

                    if (i == targetIndex) {
                        foundTarget = WASMU_TRUE;
                        labelIndex = targetLabelIndex;
                    }
                }

                wasmu_Count defaultLabelIndex = wasmu_readUInt(module);

                if (!foundTarget) {
                    labelIndex = defaultLabelIndex;
                }
            } else {
                labelIndex = wasmu_readUInt(module);
            }

            WASMU_FF_SKIP_HERE();

            wasmu_Label label;

            WASMU_DEBUG_LOG("Branch/branch if/branch table - labelIndex: %d", labelIndex);

            if (!wasmu_getLabel(context, labelIndex, context->callStack.count - 1, &label)) {
                WASMU_DEBUG_LOG("Label stack underflow");
                context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
                return WASMU_FALSE;
            }

            if (opcode == WASMU_OP_BR_IF) {
                wasmu_Int condition = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

                WASMU_DEBUG_LOG("Branch if - condition: %d", condition);

                if (!condition) {
                    break;
                }
            }

            // Clean the stack to remove non-result values from the type and value stacks

            wasmu_Int nonResultsCount = context->typeStack.count - label.typeStackBase - label.resultsCount;
            wasmu_Int nonResultsSize = context->valueStack.position - label.valueStackBase - label.resultsSize;

            WASMU_DEBUG_LOG("Clean up stack - nonResultsCount: %d, nonResultsSize: %d", nonResultsCount, nonResultsSize);

            if (nonResultsCount < 0 || nonResultsSize < 0) {
                WASMU_DEBUG_LOG(nonResultsCount < 0 ? "Type stack overflow" : "Value stack underflow");
                context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
                return WASMU_FALSE;
            }

            // Clean the type stack first by shifting results up

            for (wasmu_Count i = 0; i < nonResultsCount; i++) {
                context->typeStack.types[label.typeStackBase + i] = context->typeStack.types[label.typeStackBase + nonResultsCount + i];
            }

            context->typeStack.count = label.typeStackBase + label.resultsCount;

            // Do the same for the value stack

            for (wasmu_Count i = 0; i < nonResultsSize; i++) {
                context->valueStack.data[label.valueStackBase + i] = context->valueStack.data[label.valueStackBase + nonResultsSize + i];
            }

            context->valueStack.position = label.valueStackBase + label.resultsSize;

            // Finally, jump to the position specified in the label

            WASMU_DEBUG_LOG("Checking label from position 0x%08x", label.position);

            module->position = label.position;

            if (label.opcode == WASMU_OP_LOOP) {
                // No need to fast forward since we're looping back to start
                break;
            }

            if (!wasmu_fastForward(context, WASMU_OP_END, WASMU_NULL, WASMU_TRUE)) {
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

        case WASMU_OP_CALL_INDIRECT:
        {
            wasmu_Count typeIndex = wasmu_readUInt(module);
            wasmu_Count tableIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_Count elementIndex = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

            wasmu_Table* table = WASMU_GET_ENTRY(module->tables, module->tablesCount, tableIndex);

            WASMU_DEBUG_LOG(
                "Call indirect - typeIndex: %d, tableIndex: %d, elementIndex: %d",
                typeIndex, tableIndex, elementIndex
            );

            if (!table) {
                WASMU_DEBUG_LOG("Unknown table");
                module->context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            wasmu_Count* functionIndex = WASMU_GET_ENTRY(table->entries, table->entriesCount, elementIndex);

            if (!functionIndex) {
                WASMU_DEBUG_LOG("Table element index is out of bounds");
                module->context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            WASMU_DEBUG_LOG("Resolve to function index - functionIndex: %d", *functionIndex);

            if (!wasmu_callFunctionByIndex(context, context->activeModuleIndex, *functionIndex)) {
                WASMU_DEBUG_LOG("Unknown function referred to by table element");
                context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }
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

        case WASMU_OP_GLOBAL_GET:
        {
            wasmu_Count globalIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_TypedValue* global = WASMU_GET_ENTRY(module->globals, module->globalsCount, globalIndex);
            wasmu_Count size = wasmu_getValueTypeSize(global->type);
            wasmu_Int value = global->value.asInt;

            WASMU_DEBUG_LOG("Get global - index: %d (type: %d, size: %d, value: %d)", globalIndex, global->type, size, value);

            wasmu_pushInt(context, size, value);
            wasmu_pushType(context, global->type);

            break;
        }

        case WASMU_OP_GLOBAL_SET:
        {
            wasmu_Count globalIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_TypedValue* global = WASMU_GET_ENTRY(module->globals, module->globalsCount, globalIndex);
            wasmu_Count size = wasmu_getValueTypeSize(global->type);
            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(global->type);

            WASMU_DEBUG_LOG("Set global - index: %d (type: %d, size: %d, value: %d)", globalIndex, global->type, size, value);

            global->value.asInt = value;

            break;
        }

        case WASMU_OP_I32_LOAD:
        case WASMU_OP_I64_LOAD:
        case WASMU_OP_F32_LOAD:
        case WASMU_OP_F64_LOAD:
        case WASMU_OP_I32_LOAD8_S:
        case WASMU_OP_I32_LOAD8_U:
        case WASMU_OP_I64_LOAD8_S:
        case WASMU_OP_I64_LOAD8_U:
        case WASMU_OP_I32_LOAD16_S:
        case WASMU_OP_I32_LOAD16_U:
        case WASMU_OP_I64_LOAD16_S:
        case WASMU_OP_I64_LOAD16_U:
        case WASMU_OP_I64_LOAD32_S:
        case WASMU_OP_I64_LOAD32_U:
        {
            wasmu_Count alignment = wasmu_readUInt(module);
            wasmu_Count offset = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);
            wasmu_Count dataSize = wasmu_getDataSizeFromOpcode(opcode);
            wasmu_Memory* memory = WASMU_GET_ENTRY(module->memories, module->memoriesCount, 0);

            if (!memory) {
                WASMU_DEBUG_LOG("No memory is defined");
                context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            wasmu_Count index = offset + wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
            wasmu_UInt value = 0;

            WASMU_DEBUG_LOG(
                "Load - alignment: %d, offset: 0x%08x (index: 0x%08x, dataSize: %d)",
                alignment, offset, index, dataSize
            );

            if (!wasmu_memoryLoad(memory, index, dataSize, &value)) {
                WASMU_DEBUG_LOG("Unable to load from memory");
                context->errorState = WASMU_ERROR_STATE_MEMORY_OOB;
                return WASMU_FALSE;
            }

            wasmu_signExtendValue(opcode, &value);

            WASMU_DEBUG_LOG("Loaded value: %ld", value);

            wasmu_pushInt(context, size, value);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_STORE:
        case WASMU_OP_I64_STORE:
        case WASMU_OP_F32_STORE:
        case WASMU_OP_F64_STORE:
        case WASMU_OP_I32_STORE8:
        case WASMU_OP_I64_STORE8:
        case WASMU_OP_I32_STORE16:
        case WASMU_OP_I64_STORE16:
        case WASMU_OP_I64_STORE32:
        {
            wasmu_Count alignment = wasmu_readUInt(module);
            wasmu_Count offset = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);
            wasmu_Count dataSize = wasmu_getDataSizeFromOpcode(opcode);
            wasmu_Memory* memory = WASMU_GET_ENTRY(module->memories, module->memoriesCount, 0);

            if (!memory) {
                WASMU_DEBUG_LOG("No memory is defined");
                context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            wasmu_UInt value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Count index = offset + wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

            WASMU_DEBUG_LOG(
                "Store - alignment: %d, offset: 0x%08x, value: %ld (index: 0x%08x, dataSize: %d)",
                alignment, offset, value, index, dataSize
            );

            if (!wasmu_memoryStore(memory, index, dataSize, value)) {
                WASMU_DEBUG_LOG("Unable to store to memory");
                context->errorState = WASMU_ERROR_STATE_MEMORY_OOB;
                return WASMU_FALSE;
            }

            break;
        }

        case WASMU_OP_MEMORY_SIZE:
        {
            wasmu_Count memoryIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_Memory* memory = WASMU_GET_ENTRY(module->memories, module->memoriesCount, memoryIndex);

            if (!memory) {
                WASMU_DEBUG_LOG("Unknown memory");
                context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            WASMU_DEBUG_LOG("Get memory size - (pages: %d)", memory->pagesCount);

            wasmu_pushInt(context, 4, memory->pagesCount); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

            break;
        }

        case WASMU_OP_MEMORY_GROW:
        {
            wasmu_Count memoryIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_Memory* memory = WASMU_GET_ENTRY(module->memories, module->memoriesCount, memoryIndex);

            if (!memory) {
                WASMU_DEBUG_LOG("Unknown memory");
                context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            wasmu_Count pagesToAdd = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
            wasmu_Count newPagesCount = memory->pagesCount + pagesToAdd;

            WASMU_DEBUG_LOG("Grow memory - pagesToAdd: %d, newPagesCount", pagesToAdd, newPagesCount);

            if (newPagesCount > memory->maxPages) {
                wasmu_pushInt(context, 4, -1); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);
                break;
            }

            wasmu_pushInt(context, 4, memory->pagesCount); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

            memory->pagesCount = newPagesCount;

            break;
        }

        case WASMU_OP_I32_CONST:
        case WASMU_OP_I64_CONST:
        {
            wasmu_Int value = wasmu_readInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            WASMU_DEBUG_LOG("Const - value: %ld", value);

            wasmu_pushInt(context, size, value);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_F32_CONST:
        case WASMU_OP_F64_CONST:
        {
            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Float value = wasmu_readFloat(module, type);

            WASMU_FF_SKIP_HERE();

            WASMU_DEBUG_LOG("Const - value: %f", value);

            wasmu_pushFloat(context, type, value);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_EQZ:
        case WASMU_OP_I64_EQZ:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);

            WASMU_DEBUG_LOG("Equal to zero - value: %ld (result: %ld)", value, value == 0);

            wasmu_pushInt(context, size, value == 0);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_EQ:
        case WASMU_OP_I64_EQ:
            WASMU_INT_OPERATOR(wasmu_Int, ==)

        case WASMU_OP_F32_EQ:
        case WASMU_OP_F64_EQ:
            WASMU_FLOAT_OPERATOR(==)

        case WASMU_OP_I32_NE:
        case WASMU_OP_I64_NE:
            WASMU_INT_OPERATOR(wasmu_Int, !=)

        case WASMU_OP_F32_NE:
        case WASMU_OP_F64_NE:
            WASMU_FLOAT_OPERATOR(!=)

        case WASMU_OP_I32_LT_S:
        case WASMU_OP_I64_LT_S:
            WASMU_INT_OPERATOR(wasmu_Int, <)

        case WASMU_OP_I32_LT_U:
        case WASMU_OP_I64_LT_U:
            WASMU_INT_OPERATOR(wasmu_UInt, <)

        case WASMU_OP_F32_LT:
        case WASMU_OP_F64_LT:
            WASMU_FLOAT_OPERATOR(<)

        case WASMU_OP_I32_GT_S:
        case WASMU_OP_I64_GT_S:
            WASMU_INT_OPERATOR(wasmu_Int, >)

        case WASMU_OP_I32_GT_U:
        case WASMU_OP_I64_GT_U:
            WASMU_INT_OPERATOR(wasmu_UInt, >)

        case WASMU_OP_F32_GT:
        case WASMU_OP_F64_GT:
            WASMU_FLOAT_OPERATOR(>)

        case WASMU_OP_I32_LE_S:
        case WASMU_OP_I64_LE_S:
            WASMU_INT_OPERATOR(wasmu_Int, <=)

        case WASMU_OP_I32_LE_U:
        case WASMU_OP_I64_LE_U:
            WASMU_INT_OPERATOR(wasmu_UInt, <=)

        case WASMU_OP_F32_LE:
        case WASMU_OP_F64_LE:
            WASMU_FLOAT_OPERATOR(<=)

        case WASMU_OP_I32_GE_S:
        case WASMU_OP_I64_GE_S:
            WASMU_INT_OPERATOR(wasmu_Int, >=)

        case WASMU_OP_I32_GE_U:
        case WASMU_OP_I64_GE_U:
            WASMU_INT_OPERATOR(wasmu_UInt, >=)

        case WASMU_OP_F32_GE:
        case WASMU_OP_F64_GE:
            WASMU_FLOAT_OPERATOR(>=)

        case WASMU_OP_I32_CLZ:
        case WASMU_OP_I64_CLZ:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Count result = wasmu_countLeadingZeros(value, size);

            WASMU_DEBUG_LOG("Count leading zeros - value: %ld (result: %ld)", value, result);

            wasmu_pushInt(context, size, result);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_CTZ:
        case WASMU_OP_I64_CTZ:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Count result = wasmu_countTrailingZeros(value, size);

            WASMU_DEBUG_LOG("Count trailing zeros - value: %ld (result: %ld)", value, result);

            wasmu_pushInt(context, size, result);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_POPCNT:
        case WASMU_OP_I64_POPCNT:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Count result = wasmu_countOnes(value, size);

            WASMU_DEBUG_LOG("Count ones - value: %ld (result: %ld)", value, result);

            wasmu_pushInt(context, size, result);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_ADD:
        case WASMU_OP_I64_ADD:
            WASMU_INT_OPERATOR(wasmu_Int, +)

        case WASMU_OP_F32_ADD:
        case WASMU_OP_F64_ADD:
            WASMU_FLOAT_OPERATOR(+)

        case WASMU_OP_I32_SUB:
        case WASMU_OP_I64_SUB:
            WASMU_INT_OPERATOR(wasmu_Int, -)

        case WASMU_OP_F32_SUB:
        case WASMU_OP_F64_SUB:
            WASMU_FLOAT_OPERATOR(-)

        case WASMU_OP_I32_MUL:
        case WASMU_OP_I64_MUL:
            WASMU_INT_OPERATOR(wasmu_Int, *)

        case WASMU_OP_F32_MUL:
        case WASMU_OP_F64_MUL:
            WASMU_FLOAT_OPERATOR(*)

        case WASMU_OP_I32_DIV_S:
        case WASMU_OP_I64_DIV_S:
            WASMU_INT_OPERATOR(wasmu_Int, /)

        case WASMU_OP_I32_DIV_U:
        case WASMU_OP_I64_DIV_U:
            WASMU_INT_OPERATOR(wasmu_UInt, /)

        case WASMU_OP_F32_DIV:
        case WASMU_OP_F64_DIV:
            WASMU_FLOAT_OPERATOR(/)

        case WASMU_OP_I32_REM_S:
        case WASMU_OP_I64_REM_S:
            WASMU_INT_OPERATOR(wasmu_Int, %)

        case WASMU_OP_I32_REM_U:
        case WASMU_OP_I64_REM_U:
            WASMU_INT_OPERATOR(wasmu_UInt, %)

        case WASMU_OP_I32_AND:
        case WASMU_OP_I64_AND:
            WASMU_INT_OPERATOR(wasmu_Int, &)

        case WASMU_OP_I32_OR:
        case WASMU_OP_I64_OR:
            WASMU_INT_OPERATOR(wasmu_Int, |)

        case WASMU_OP_I32_XOR:
        case WASMU_OP_I64_XOR:
            WASMU_INT_OPERATOR(wasmu_Int, ^)

        case WASMU_OP_I32_SHL:
        case WASMU_OP_I64_SHL:
            WASMU_INT_OPERATOR(wasmu_Int, <<)

        case WASMU_OP_I32_SHR_S:
        case WASMU_OP_I64_SHR_S:
            WASMU_INT_OPERATOR(wasmu_Int, >>)

        case WASMU_OP_I32_SHR_U:
        case WASMU_OP_I64_SHR_U:
            WASMU_INT_OPERATOR(wasmu_UInt, >>)

        case WASMU_OP_I32_ROTL:
        case WASMU_OP_I64_ROTL:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_Int shift = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Count result = wasmu_rotateLeft(value, size, shift);

            WASMU_DEBUG_LOG("Rotate left - value: %ld, shift: %ld (result: %ld)", value, shift, result);

            wasmu_pushInt(context, size, result);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_ROTR:
        case WASMU_OP_I64_ROTR:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_Int shift = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Count result = wasmu_rotateRight(value, size, shift);

            WASMU_DEBUG_LOG("Rotate right - value: %ld, shift: %ld (result: %ld)", value, shift, result);

            wasmu_pushInt(context, size, result);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_WRAP_I64:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_I64 value = wasmu_popInt(context, 8); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I64);

            WASMU_DEBUG_LOG("Wrap I64 into I32 - value: %ld", value);

            wasmu_pushInt(context, 4, value);
            wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

            break;
        }

        case WASMU_OP_I32_TRUNC_F32_S:
        case WASMU_OP_I32_TRUNC_F32_U:
        case WASMU_OP_I64_TRUNC_F32_S:
        case WASMU_OP_I64_TRUNC_F32_U:
        case WASMU_OP_I32_TRUNC_F64_S:
        case WASMU_OP_I32_TRUNC_F64_U:
        case WASMU_OP_I64_TRUNC_F64_S:
        case WASMU_OP_I64_TRUNC_F64_U:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);
            wasmu_ValueType objectType = wasmu_getOpcodeObjectType(opcode);
            wasmu_Float floatValue = wasmu_popFloat(context, objectType); WASMU_ASSERT_POP_TYPE(objectType);

            WASMU_DEBUG_LOG(
                "Truncate - floatValue: %f, size: %d, signed: %d",
                floatValue, size, wasmu_opcodeIsSigned(opcode)
            );

            if (wasmu_opcodeIsSigned(opcode)) {
                wasmu_Int intValue = (wasmu_Int)floatValue;

                wasmu_pushInt(context, size, intValue);
            } else {
                wasmu_pushInt(context, size, (wasmu_UInt)floatValue);
            }

            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I64_EXTEND_I32_S:
        case WASMU_OP_I64_EXTEND_I32_U:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_I32 value = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

            WASMU_DEBUG_LOG("Extend I32 into I64 - value: %d, signed: %d", value, wasmu_opcodeIsSigned(opcode));

            wasmu_pushInt(context, 4, value);
            wasmu_pushInt(context, 4, wasmu_opcodeIsSigned(opcode) && value < 0 ? -1 : 0);

            break;
        }

        case WASMU_OP_F32_CONVERT_I32_S:
        case WASMU_OP_F32_CONVERT_I32_U:
        case WASMU_OP_F32_CONVERT_I64_S:
        case WASMU_OP_F32_CONVERT_I64_U:
        case WASMU_OP_F64_CONVERT_I32_S:
        case WASMU_OP_F64_CONVERT_I32_U:
        case WASMU_OP_F64_CONVERT_I64_S:
        case WASMU_OP_F64_CONVERT_I64_U:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_ValueType objectType = wasmu_getOpcodeObjectType(opcode);
            wasmu_Count objectSize = wasmu_getValueTypeSize(objectType);
            wasmu_Float floatValue;

            if (wasmu_opcodeIsSigned(opcode)) {
                wasmu_Int intValue = wasmu_popInt(context, objectSize); WASMU_ASSERT_POP_TYPE(objectType);

                WASMU_DEBUG_LOG("Convert - intValue: %d, objectSize: %d, signed: 1", intValue, objectSize);

                floatValue = (wasmu_Float)intValue;
            } else {
                wasmu_UInt intValue = wasmu_popInt(context, objectSize); WASMU_ASSERT_POP_TYPE(objectType);

                WASMU_DEBUG_LOG("Convert - intValue: %d, objectSize: %d, signed: 0", intValue, objectSize);

                floatValue = (wasmu_Float)intValue;
            }

            wasmu_pushFloat(context, type, floatValue);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_F32_DEMOTE_F64:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_Float value = wasmu_popFloat(context, WASMU_VALUE_TYPE_F64); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_F64);

            WASMU_DEBUG_LOG("Demote F64 into F32 - value: %f", value);

            wasmu_pushFloat(context, WASMU_VALUE_TYPE_F32, value);
            wasmu_pushType(context, WASMU_VALUE_TYPE_F32);

            break;
        }

        case WASMU_OP_F64_PROMOTE_F32:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_Float value = wasmu_popFloat(context, WASMU_VALUE_TYPE_F32); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_F32);

            WASMU_DEBUG_LOG("Promote F32 into F64 - value: %f", value);

            wasmu_pushFloat(context, WASMU_VALUE_TYPE_F64, value);
            wasmu_pushType(context, WASMU_VALUE_TYPE_F64);

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
