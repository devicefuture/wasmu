wasmu_Module* wasmu_newModule(wasmu_Context* context) {
    wasmu_Module* module = WASMU_NEW(wasmu_Module);

    module->context = context;
    module->code = WASMU_NULL;
    module->codeSize = 0;
    module->position = 0;
    module->nextFunctionIndexForCode = 0;

    WASMU_INIT_ENTRIES(module->customSections, module->customSectionsCount);
    WASMU_INIT_ENTRIES(module->functionSignatures, module->functionSignaturesCount);
    WASMU_INIT_ENTRIES(module->functions, module->functionsCount);
    WASMU_INIT_ENTRIES(module->exports, module->exportsCount);

    WASMU_ADD_ENTRY(context->modules, context->modulesCount, module);

    return module;
}

void wasmu_load(wasmu_Module* module, wasmu_U8* code, wasmu_Count codeSize) {
    WASMU_DEBUG_LOG("Load code - size: %d", codeSize);

    module->code = code;
    module->codeSize = codeSize;
    module->position = 0;
}

wasmu_U8 wasmu_read(wasmu_Module* module, wasmu_Count position) {
    if (!module->code || position >= module->codeSize) {
        module->context->errorState = WASMU_ERROR_STATE_MEMORY_OOB;

        return 0;
    }

    return module->code[position];
}

wasmu_U8 wasmu_readNext(wasmu_Module* module) {
    return wasmu_read(module, module->position++);
}

// Unsigned integers are encoded as LEB128
// @source reference https://en.wikipedia.org/wiki/LEB128
// @licence ccbysa4
wasmu_UInt wasmu_readUInt(wasmu_Module* module) {
    wasmu_UInt result = 0;
    wasmu_UInt shift = 0;
    wasmu_U8 byte;

    do {
        byte = WASMU_NEXT();
        result |= (byte & 0b01111111) << shift;
        shift += 7;
    } while ((byte & 0b10000000) != 0);

    return result;
}

// Signed integers are encoded as LEB128
// @source reference https://en.wikipedia.org/wiki/LEB128
// @licence ccbysa4
wasmu_Int wasmu_readInt(wasmu_Module* module) {
    wasmu_Int result = 0;
    wasmu_UInt shift = 0;
    wasmu_U8 byte;

    do {
        byte = WASMU_NEXT();
        result |= (byte & 0b01111111) << shift;
        shift += 7;
    } while ((byte & 0b10000000) != 0);

    if (shift < sizeof(result) / 8 && result & 0b10000000) {
        result |= (~0 << shift);
    }

    return result;
}

wasmu_String wasmu_readString(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_U8* chars = (wasmu_U8*)WASMU_MALLOC(size);

    for (unsigned int i = 0; i < size; i++) {
        chars[i] = WASMU_NEXT();
    }

    return (wasmu_String) {
        .size = size,
        .chars = chars
    };
}

wasmu_U8* wasmu_getNullTerminatedChars(wasmu_String string) {
    wasmu_U8* chars = (wasmu_U8*)WASMU_MALLOC(string.size + 1);

    for (wasmu_Count i = 0; i < string.size; i++) {
        chars[i] = string.chars[i];
    }

    chars[string.size] = '\0';

    return chars;
}

wasmu_Bool wasmu_stringEqualsChars(wasmu_String a, const wasmu_U8* b) {
    wasmu_U8* chars = wasmu_getNullTerminatedChars(a);
    wasmu_Bool result = wasmu_charsEqual(chars, b);

    free(chars);

    return result;
}

wasmu_Count wasmu_getValueTypeSize(wasmu_ValueType type) {
    switch (type) {
        case WASMU_VALUE_TYPE_I32:
        case WASMU_VALUE_TYPE_F32:
            return 4;

        default:
            WASMU_DEBUG_LOG("Unknown value type: 0x%02x", type);
            return 0;
    }
}

wasmu_Int wasmu_getExportedFunctionIndex(wasmu_Module* module, const wasmu_U8* name) {
    for (wasmu_Count i = 0; i < module->exportsCount; i++) {
        wasmu_Export moduleExport = module->exports[i];

        if (moduleExport.type == WASMU_EXPORT_TYPE_FUNCTION && wasmu_stringEqualsChars(moduleExport.name, name)) {
            return moduleExport.data.asFunctionIndex;
        }
    }

    return -1;
}

wasmu_Function* wasmu_getExportedFunction(wasmu_Module* module, const wasmu_U8* name) {
    wasmu_Int functionIndex = wasmu_getExportedFunctionIndex(module, name);

    if (functionIndex == -1) {
        return WASMU_NULL;
    }

    return WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);
}