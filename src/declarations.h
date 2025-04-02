typedef enum {
    WASMU_ERROR_STATE_NONE = 0,
    WASMU_ERROR_STATE_NOT_IMPLEMENTED,
    WASMU_ERROR_STATE_MEMORY_OOB
} wasmu_ErrorState;

typedef enum {
    WASMU_DATA_TYPE_I32 = 0x7F,
    WASMU_DATA_TYPE_F32 = 0x7D
} wasmu_DataType;

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

typedef enum wasmu_ValueType {
    WASMU_VALUE_TYPE_FUNCTION = 0x60
} wasmu_ValueType;

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
    struct wasmu_FunctionType* functionTypes;
    wasmu_Count functionTypesCount;
} wasmu_Module;

typedef struct wasmu_FunctionType {
    wasmu_DataType* parameters;
    wasmu_Count parametersCount;
    wasmu_DataType* results;
    wasmu_Count resultsCount;
} wasmu_FunctionType;

wasmu_Context* wasmu_newContext();

wasmu_Module* wasmu_newModule(wasmu_Context* context);
void wasmu_load(wasmu_Module* module, wasmu_U8* code, wasmu_Count codeSize);
wasmu_U8 wasmu_read(wasmu_Module* module, wasmu_Count position);
wasmu_U8 wasmu_readNext(wasmu_Module* module);
wasmu_UInt wasmu_readUInt(wasmu_Module* module);
wasmu_Int wasmu_readInt(wasmu_Module* module);

wasmu_Bool wasmu_parseSections(wasmu_Module* module);