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