wasmu_Context* wasmu_newContext() {
    WASMU_DEBUG_LOG("Create new context");

    wasmu_Context* context = WASMU_NEW(wasmu_Context);

    context->errorState = WASMU_ERROR_STATE_NONE;
    context->code = WASMU_NULL;
    context->codeSize = 0;
    context->position = 0;

    WASMU_INIT_ENTRIES(context->functionTypes, context->functionTypesCount);

    return context;
}

void wasmu_load(wasmu_Context* context, wasmu_U8* code, wasmu_Count codeSize) {
    WASMU_DEBUG_LOG("Load code - size: %d", codeSize);

    context->code = code;
    context->codeSize = codeSize;
    context->position = 0;
}

wasmu_U8 wasmu_read(wasmu_Context* context, wasmu_Count position) {
    if (!context->code || position >= context->codeSize) {
        context->errorState = WASMU_ERROR_STATE_MEMORY_OOB;

        return 0;
    }

    return context->code[position];
}

wasmu_U8 wasmu_readNext(wasmu_Context* context) {
    return wasmu_read(context, context->position++);
}

wasmu_UInt wasmu_readUInt(wasmu_Context* context) {
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

wasmu_Int wasmu_readInt(wasmu_Context* context) {
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