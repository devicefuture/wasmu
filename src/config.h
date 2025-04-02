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

void* _wasmu_malloc(WASMU_COUNT size) {
    return malloc(size);
}

void* _wasmu_realloc(void* ptr, WASMU_COUNT size) {
    return realloc(ptr, size);
}

void _wasmu_free(void* ptr) {
    free(ptr);
}

#define WASMU_MALLOC _wasmu_malloc
#define WASMU_REALLOC _wasmu_realloc
#define WASMU_FREE _wasmu_free

#endif

#endif