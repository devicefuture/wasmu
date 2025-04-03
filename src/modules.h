wasmu_Module* wasmu_newModule(wasmu_Context* context) {
    wasmu_Module* module = WASMU_NEW(wasmu_Module);

    module->code = WASMU_NULL;
    module->codeSize = 0;
    module->position = 0;
    module->nextFunctionIndexForCode = 0;

    WASMU_INIT_ENTRIES(module->functionSignatures, module->functionSignaturesCount);
    WASMU_INIT_ENTRIES(module->functions, module->functionsCount);

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

wasmu_UInt wasmu_readUInt(wasmu_Module* module) {
    wasmu_UInt result = 0;
    wasmu_UInt shift = 0;
    wasmu_U8 byte;

    do {
        byte = WASMU_NEXT();
        result |= (byte & 0b01111111) << shift;
        shift += 7;
    } while (byte & 0b10000000 == 0);

    return result;
}

wasmu_Int wasmu_readInt(wasmu_Module* module) {
    wasmu_Int result = 0;
    wasmu_UInt shift = 0;
    wasmu_U8 byte;

    do {
        byte = WASMU_NEXT();
        result |= (byte & 0b01111111) << shift;
        shift += 7;
    } while (byte & 0b10000000 == 0);

    if (shift < sizeof(result) / 8 && result & 0b10000000) {
        result |= (~0 << shift);
    }

    return result;
}