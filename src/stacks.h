#define WASMU_ASSERT_POP_TYPE(type) do { \
    wasmu_ValueType poppedType = wasmu_popType(context); \
    \
    if (poppedType != type) { \
        WASMU_DEBUG_LOG("Expected type 0x%02x but got 0x%02x", type, poppedType); \
        context->errorState = WASMU_ERROR_STATE_TYPE_MISMATCH; \
        return WASMU_FALSE; \
    } \
} while (0)

wasmu_Bool wasmu_pushLabel(wasmu_Context* context, wasmu_Opcode opcode, wasmu_Count resultsCount, wasmu_Count resultsSize) {
    wasmu_LabelStack* stack = &context->labelStack;

    if (context->callStack.count == 0) {
        return WASMU_FALSE;
    }

    stack->count++;

    if (stack->count > stack->size) {
        stack->size = stack->count;
        stack->labels = (wasmu_Label*)WASMU_REALLOC(stack->labels, stack->size * sizeof(wasmu_Label));
    }

    stack->labels[stack->count - 1] = (wasmu_Label) {
        .opcode = opcode,
        .callIndex = context->callStack.count - 1,
        .position = context->activeModule->position,
        .resultsCount = resultsCount,
        .resultsSize = resultsSize,
        .typeStackBase = context->typeStack.count,
        .valueStackBase = context->valueStack.position
    };

    return WASMU_TRUE;
}

wasmu_Bool wasmu_popLabel(wasmu_Context* context, wasmu_Count targetCallIndex, wasmu_Label* returnedLabel) {
    wasmu_LabelStack* stack = &context->labelStack;

    if (stack->count == 0) {
        WASMU_DEBUG_LOG("Label stack underflow");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_FALSE;
    }

    wasmu_Label label = stack->labels[stack->count - 1];

    if (targetCallIndex != -1 && label.callIndex != targetCallIndex) {
        WASMU_DEBUG_LOG("Label stack underflow for this function call");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_FALSE;
    }

    if (returnedLabel) {
        *returnedLabel = label;
    }

    stack->count--;

    return WASMU_TRUE;
}

wasmu_Bool wasmu_getLabel(wasmu_Context* context, wasmu_Count labelIndex, wasmu_Count targetCallIndex, wasmu_Label* returnedLabel) {
    // Function does not throw error as callers may use it to check if a label exists

    wasmu_LabelStack* stack = &context->labelStack;

    if (labelIndex >= stack->count) {
        return WASMU_FALSE;
    }

    /*
        Label indexes start from the end of the label stack instead of the
        start, so they must be converted into absolute indexes to reference a
        label on the stack's data structure.
    */

    wasmu_Label label = stack->labels[stack->count - labelIndex - 1];

    if (targetCallIndex != -1 && label.callIndex != targetCallIndex) {
        return WASMU_FALSE;
    }

    if (returnedLabel) {
        *returnedLabel = label;
    }

    return WASMU_TRUE;
}

void wasmu_populateActiveCallInfo(wasmu_Context* context, wasmu_Call call) {
    /*
        This is done to cache pointers to the active module, function and
        function signature, instead of having to check the top of the call stack
        and then resolve indexes to pointers every time we want to step our
        context.
    */

    wasmu_Module** moduleEntry = WASMU_GET_ENTRY(context->modules, context->modulesCount, call.moduleIndex);
    wasmu_Module* module = *moduleEntry;

    context->activeModule = module;
    context->activeModuleIndex = call.moduleIndex;

    if (context->activeModule) {
        context->activeFunction = WASMU_GET_ENTRY(module->functions, module->functionsCount, call.functionIndex);
        context->activeFunctionSignature = WASMU_GET_ENTRY(module->functionSignatures, module->functionSignaturesCount, context->activeFunction->signatureIndex);

        context->currentStackLocals = (wasmu_StackLocal*)WASMU_REALLOC(context->currentStackLocals, 0);
        context->currentStackLocalsCount = 0;

        wasmu_Count currentPosition = call.valueStackBase;

        for (wasmu_Count i = 0; i < context->activeFunctionSignature->parametersCount; i++) {
            wasmu_ValueType type = context->activeFunctionSignature->parameters[i];
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_StackLocal local = {
                .position = currentPosition,
                .type = WASMU_LOCAL_TYPE_PARAMETER,
                .valueType = type,
                .size = size
            };

            WASMU_ADD_ENTRY(context->currentStackLocals, context->currentStackLocalsCount, local);

            currentPosition += size;
        }

        for (wasmu_Count i = 0; i < context->activeFunctionSignature->resultsCount; i++) {
            wasmu_ValueType type = context->activeFunctionSignature->results[i];
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_StackLocal local = {
                .position = currentPosition,
                .type = WASMU_LOCAL_TYPE_RESULT,
                .valueType = type,
                .size = size
            };

            WASMU_ADD_ENTRY(context->currentStackLocals, context->currentStackLocalsCount, local);

            currentPosition += size;
        }

        for (wasmu_Count i = 0; i < context->activeFunction->localsCount; i++) {
            wasmu_ValueType type = context->activeFunction->locals[i];
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_StackLocal local = {
                .position = currentPosition,
                .type = WASMU_LOCAL_TYPE_LOCAL,
                .valueType = type,
                .size = size
            };

            WASMU_ADD_ENTRY(context->currentStackLocals, context->currentStackLocalsCount, local);

            currentPosition += size;
        }
    }

    context->currentTypeStackBase = call.typeStackBase;
    context->currentValueStackBase = call.valueStackBase;
}

void wasmu_pushCall(wasmu_Context* context, wasmu_Call call) {
    wasmu_CallStack* stack = &context->callStack;

    // TODO: Allow call stack to have a maximum size defined to catch infinite recursion
    stack->count++;

    if (stack->count > stack->size) {
        stack->size = stack->count;
        stack->calls = (wasmu_Call*)WASMU_REALLOC(stack->calls, stack->size * sizeof(wasmu_Call));
    }

    stack->calls[stack->count - 1] = call;

    wasmu_populateActiveCallInfo(context, call);
}

wasmu_Bool wasmu_popCall(wasmu_Context* context, wasmu_Call* returnedCall) {
    wasmu_CallStack* stack = &context->callStack;

    if (stack->count == 0) {
        WASMU_DEBUG_LOG("Call stack underflow");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_FALSE;
    }

    if (returnedCall) {
        *returnedCall = stack->calls[stack->count - 1];
    }

    stack->count--;

    while (context->labelStack.count > 0 && context->labelStack.labels[context->labelStack.count - 1].callIndex >= stack->count) {
        // Remove labels from label stack that reference a call that no longer exists
        wasmu_popLabel(context, -1, WASMU_NULL);
    }

    if (stack->count > 0) {
        wasmu_populateActiveCallInfo(context, stack->calls[stack->count - 1]);
    } else {
        context->activeModule = WASMU_NULL;
    }

    return WASMU_TRUE;
}

void wasmu_pushType(wasmu_Context* context, wasmu_ValueType type) {
    wasmu_TypeStack* stack = &context->typeStack;

    stack->count++;

    if (stack->count > stack->size) {
        stack->size = stack->count;
        stack->types = (wasmu_ValueType*)WASMU_REALLOC(stack->types, stack->size * sizeof(wasmu_ValueType));
    }

    stack->types[stack->count - 1] = type;
}

wasmu_ValueType wasmu_popType(wasmu_Context* context) {
    wasmu_TypeStack* stack = &context->typeStack;

    if (stack->count == 0) {
        WASMU_DEBUG_LOG("Type stack underflow");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_VALUE_TYPE_I32;
    }

    wasmu_ValueType type = stack->types[stack->count - 1];

    stack->count--;

    return type;
}

void wasmu_growValueStack(wasmu_ValueStack* stack, wasmu_Count newPosition) {
    if (newPosition > stack->size) {
        stack->size = newPosition;
        stack->data = (wasmu_U8*)WASMU_REALLOC(stack->data, stack->size);
    }
}

wasmu_Int wasmu_stackGetInt(wasmu_Context* context, wasmu_Count position, wasmu_Count bytes) {
    wasmu_Int value = 0;
    wasmu_ValueStack* stack = &context->valueStack;

    for (wasmu_Count i = 0; i < bytes; i++) {
        value |= stack->data[position + i] << (i * 8);
    }

    return value;
}

void wasmu_stackSetInt(wasmu_Context* context, wasmu_Count position, wasmu_Count bytes, wasmu_Int value) {
    wasmu_ValueStack* stack = &context->valueStack;

    wasmu_growValueStack(stack, position + bytes);

    for (wasmu_Count i = 0; i < bytes; i++) {
        stack->data[position + i] = value & 0xFF;
        value >>= 8;
    }
}

void wasmu_pushInt(wasmu_Context* context, wasmu_Count bytes, wasmu_Int value) {
    wasmu_ValueStack* stack = &context->valueStack;

    wasmu_stackSetInt(context, stack->position, bytes, value);

    stack->position += bytes;
}

wasmu_Int wasmu_popInt(wasmu_Context* context, wasmu_Count bytes) {
    wasmu_ValueStack* stack = &context->valueStack;

    if (stack->position < bytes) {
        WASMU_DEBUG_LOG("Value stack underflow");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return 0;
    }

    stack->position -= bytes;

    return wasmu_stackGetInt(context, stack->position, bytes);
}