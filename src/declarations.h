typedef enum {
    WASMU_ERROR_STATE_NONE = 0,
    WASMU_ERROR_STATE_NOT_IMPLEMENTED,
    WASMU_ERROR_STATE_MEMORY_OOB,
    WASMU_ERROR_STATE_CODE_BODY_MISMATCH,
    WASMU_ERROR_STATE_STACK_UNDERFLOW,
    WASMU_ERROR_STATE_TYPE_MISMATCH
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
    wasmu_Count valueStackBase;
} wasmu_Call;

typedef struct wasmu_CallStack {
    wasmu_Call* calls;
    wasmu_Count size;
    wasmu_Count count;
} wasmu_CallStack;

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
    wasmu_ValueType valueType;
    wasmu_Count size;
} wasmu_StackLocal;

typedef struct wasmu_Context {
    wasmu_ErrorState errorState;
    struct wasmu_Module** modules;
    wasmu_Count modulesCount;
    wasmu_CallStack callStack;
    wasmu_TypeStack typeStack;
    wasmu_ValueStack valueStack;
    struct wasmu_Module* activeModule;
    struct wasmu_Function* activeFunction;
    struct wasmu_FunctionSignature* activeFunctionSignature;
    wasmu_Count currentValueStackBase;
    wasmu_StackLocal* currentStackLocals;
    wasmu_Count currentStackLocalsCount;
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
wasmu_Bool wasmu_step(wasmu_Context* context);
wasmu_Bool wasmu_runFunction(wasmu_Module* module, wasmu_Function* function);