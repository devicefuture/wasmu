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
            wasmu_Count oldSize = memory->size;
            wasmu_Count newSize = WASMU_MEMORY_ALIGN_BLOCK(index + 1);

            if (newPagesCount > memory->maxPages) {
                return WASMU_FALSE;
            }

            #ifdef WASMU_MEMORY_SIZE_CHECKER
                if (!(WASMU_MEMORY_SIZE_CHECKER(newSize))) {
                    return WASMU_FALSE;
                }
            #endif

            if (newPagesCount > memory->pagesCount) {
                memory->pagesCount = newPagesCount;
            }

            memory->size = newSize;
            memory->data = (wasmu_U8*)WASMU_REALLOC(memory->data, memory->size + 1);

            for (wasmu_Count j = oldSize; j < newSize + 1; j++) {
                memory->data[j] = 0;
            }

            WASMU_DEBUG_LOG("Increase memory size to %d", memory->size);
        }

        memory->data[index++] = value & 0xFF;
        value >>= 8;
    }

    return WASMU_TRUE;
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
    wasmu_UInt sign = *value >> ((dataSize * 8) - 1);

    if (!sign) {
        return;
    }

    wasmu_UInt mask = -1;

    *value |= (mask << (dataSize * 8));
}