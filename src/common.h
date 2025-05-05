typedef WASMU_BOOL wasmu_Bool;
typedef WASMU_COUNT wasmu_Count;
typedef WASMU_U8 wasmu_U8;
typedef WASMU_I8 wasmu_I8;
typedef WASMU_U16 wasmu_U16;
typedef WASMU_I16 wasmu_I16;
typedef WASMU_U32 wasmu_U32;
typedef WASMU_I32 wasmu_I32;
typedef WASMU_F32 wasmu_F32;
typedef WASMU_U64 wasmu_U64;
typedef WASMU_I64 wasmu_I64;
typedef WASMU_F64 wasmu_F64;

typedef WASMU_U64 wasmu_UInt;
typedef WASMU_I64 wasmu_Int;
typedef WASMU_F64 wasmu_Float;

#define WASMU_TRUE 1
#define WASMU_FALSE 0
#define WASMU_NULL 0

#define WASMU_NEW(type) (type*)WASMU_MALLOC(sizeof(type))

#define WASMU_READ(position) wasmu_read(module, position)
#define WASMU_NEXT() wasmu_readNext(module)
#define WASMU_AVAILABLE() (module->position < module->codeSize)

#define WASMU_INIT_ENTRIES(entriesPtr, countPtr) do { \
        entriesPtr = (__typeof__(entriesPtr))WASMU_MALLOC(0); \
        countPtr = 0; \
    } while (0)

#define WASMU_ADD_ENTRY(entriesPtr, countPtr, entry) do { \
        entriesPtr = (__typeof__(entriesPtr))WASMU_REALLOC(entriesPtr, sizeof((entriesPtr)[0]) * (++(countPtr))); \
        entriesPtr[(countPtr) - 1] = entry; \
    } while (0)

#define WASMU_GET_ENTRY(entriesPtr, countPtr, index) (index < countPtr ? &((entriesPtr)[index]) : WASMU_NULL)

wasmu_Bool wasmu_charsEqual(const wasmu_U8* a, const wasmu_U8* b) {
    wasmu_Count i = 0;

    while (WASMU_TRUE) {
        if (a[i] == '\0' && b[i] == '\0') {
            return WASMU_TRUE;
        }

        if (a[i] == '\0' || b[i] == '\0') {
            return WASMU_FALSE;
        }

        if (a[i] != b[i]) {
            return WASMU_FALSE;
        }

        i++;
    }
}

wasmu_U8* wasmu_copyChars(const wasmu_U8* source) {
    if (!source) {
        return WASMU_NULL;
    }

    wasmu_U8* copy = WASMU_MALLOC(1);

    if (!copy) {
        return WASMU_NULL;
    }

    copy[0] = '\0';

    wasmu_Count i = 0;

    while (*source) {
        copy[i++] = *source;
        copy = WASMU_REALLOC(copy, i + 1);

        if (!copy) {
            return WASMU_NULL;
        }

        copy[i] = '\0';

        source++;
    }

    return copy;
}

wasmu_Count wasmu_countLeadingZeros(wasmu_UInt value, wasmu_Count size) {
    wasmu_Count totalBits = size * 8;
    wasmu_Count bitsChecked = 0;
    wasmu_Count bitsAfter = 0;

    while (value > 0xFF) {
        value >>= 8;
        bitsChecked += 8;
        bitsAfter += 8;
    }

    while (bitsChecked < totalBits) {
        bitsChecked++;

        if (value & 1) {
            bitsAfter = bitsChecked;
        }

        value >>= 1;
    }

    return bitsChecked - bitsAfter;
}

wasmu_Count wasmu_countTrailingZeros(wasmu_UInt value, wasmu_Count size) {
    wasmu_Count bits = 0;

    while (value & 0xFF == 0) {
        value >>= 8;
        bits += 8;
    }

    while (value & 1 == 0) {
        value >>= 1;
        bits++;
    }

    if (bits > size * 8) {
        return size * 8;
    }

    return bits;
}

wasmu_Count wasmu_countOnes(wasmu_UInt value, wasmu_Count size) {
    wasmu_Count bits = 0;

    while (value) {
        if (value & 1) {
            bits++;
        }

        value >>= 1;
    }

    if (bits > size * 8) {
        return size * 8;
    }

    return bits;
}

// @source reference https://en.wikipedia.org/wiki/Circular_shift
// @licence ccbysa4
wasmu_UInt wasmu_rotateLeft(wasmu_UInt value, wasmu_Count size, wasmu_Count shift) {
    if ((shift &= (size * 8) - 1) == 0) {
        return value;
    }

    return (value << shift) | (value >> ((size * 8) - shift));
}

// @source reference https://en.wikipedia.org/wiki/Circular_shift
// @licence ccbysa4
wasmu_UInt wasmu_rotateRight(wasmu_UInt value, wasmu_Count size, wasmu_Count shift) {
    if ((shift &= (size * 8) - 1) == 0) {
        return value;
    }

    return (value >> shift) | (value << ((size * 8) - shift));
}