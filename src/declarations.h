typedef enum {
    WASMU_ERROR_STATE_NONE = 0,
    WASMU_ERROR_STATE_NOT_IMPLEMENTED,
    WASMU_ERROR_STATE_MEMORY_OOB,
    WASMU_ERROR_STATE_CODE_BODY_MISMATCH
} wasmu_ErrorState;

typedef enum {
    WASMU_VALUE_TYPE_I32 = 0x7F,
    WASMU_VALUE_TYPE_F32 = 0x7D
} wasmu_ValueType;

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

typedef struct wasmu_Context {
    wasmu_ErrorState errorState;
    struct wasmu_Module** modules;
    wasmu_Count modulesCount;
} wasmu_Context;

typedef struct wasmu_Module {
    wasmu_Context* context;
    wasmu_U8* code;
    wasmu_Count codeSize;
    wasmu_Count position;
    struct wasmu_FunctionSignature* functionSignatures;
    wasmu_Count functionSignaturesCount;
    struct wasmu_Function* functions;
    wasmu_Count functionsCount;
    wasmu_Count nextFunctionIndexForCode;
} wasmu_Module;

typedef struct wasmu_FunctionSignature {
    wasmu_ValueType* parameters;
    wasmu_Count parametersCount;
    wasmu_ValueType* results;
    wasmu_Count resultsCount;
} wasmu_FunctionSignature;

typedef struct wasmu_Function {
    wasmu_Count signatureIndex;
    wasmu_Count codePosition;
    wasmu_Count codeSize;
    wasmu_Count declarationsCount;

} wasmu_Function;

wasmu_Context* wasmu_newContext();

wasmu_Module* wasmu_newModule(wasmu_Context* context);
void wasmu_load(wasmu_Module* module, wasmu_U8* code, wasmu_Count codeSize);
wasmu_U8 wasmu_read(wasmu_Module* module, wasmu_Count position);
wasmu_U8 wasmu_readNext(wasmu_Module* module);
wasmu_UInt wasmu_readUInt(wasmu_Module* module);
wasmu_Int wasmu_readInt(wasmu_Module* module);

wasmu_Bool wasmu_parseSections(wasmu_Module* module);