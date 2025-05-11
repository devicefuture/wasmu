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

#ifndef WASMU_FN_PREFIX
    #ifdef __cplusplus
        #define WASMU_FN_PREFIX inline
    #else
        #define WASMU_FN_PREFIX
    #endif
#endif

#ifndef WASMU_MEMORY_BLOCK_SIZE
#define WASMU_MEMORY_BLOCK_SIZE 1024
#endif

#ifndef WASMU_MEMORY_SIZE_CHECKER
/*
    You can define this as a function with the following signature:

    WASMU_BOOL sizeChecker(WASMU_COUNT newSize);

    This function will be called every time a module's memory needs to be
    increased. Return `WASMU_TRUE` to authorise the increase in memory, or
    `WASMU_FALSE` to reject it and throw an error
    (`WASMU_ERROR_STATE_MEMORY_OOB`).
*/
#endif

#ifndef WASMU_CALL_STACK_DEPTH
#define WASMU_CALL_STACK_DEPTH 64
#endif

#ifndef WASMU_IMPORT_RESOLUTION_DEPTH
#define WASMU_IMPORT_RESOLUTION_DEPTH 16
#endif

#ifndef WASMU_MAX_TABLE_SIZE
#define WASMU_MAX_TABLE_SIZE 1024
#endif