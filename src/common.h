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

#define WASMU_READ(position) wasmu_read(context, position)
#define WASMU_NEXT() wasmu_readNext(context)
#define WASMU_AVAILABLE() (context->position < context->codeSize)

#define WASMU_INIT_ENTRIES(entriesPtr, countPtr) do { \
        entriesPtr = WASMU_MALLOC(0); \
        countPtr = 0; \
    } while (0)

#define WASMU_ADD_ENTRY(entriesPtr, countPtr, entry) do { \
        entriesPtr = WASMU_REALLOC(entriesPtr, sizeof((entriesPtr)[0]) * (++(countPtr))); \
        entriesPtr[(countPtr) - 1] = entry; \
    } while (0)