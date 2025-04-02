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