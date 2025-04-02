typedef WASMU_BOOL wasmu_Bool;
typedef WASMU_COUNT wasmu_Count;
typedef WASMU_U8 wasmu_U8;
typedef WASMU_I8 wasmu_I8;
typedef WASMU_U16 wasmu_U16;
typedef WASMU_I16 wasmu_I16;
typedef WASMU_U32 wasmu_U32;
typedef WASMU_I32 wasmu_I32;
typedef WASMU_F32 wasmu_F32;

#define WASMU_TRUE 1
#define WASMU_FALSE 0
#define WASMU_NULL 0

#define WASMU_NEW(type) (type*)WASMU_MALLOC(sizeof(type))