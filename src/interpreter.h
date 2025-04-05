void wasmu_populateActiveCallInfo(wasmu_Context* context, wasmu_Call call) {
    /*
        This is done to cache pointers to the active module and function,
        instead of having to check the top of the call stack and then resolve
        indexes to pointers every time we want to step our context.
    */

    wasmu_Module** moduleEntry = WASMU_GET_ENTRY(context->modules, context->modulesCount, call.moduleIndex);
    wasmu_Module* module = *moduleEntry;

    context->activeModule = module;

    if (context->activeModule) {
        context->activeFunction = WASMU_GET_ENTRY(module->functions, module->functionsCount, call.functionIndex);
    }

    context->currentValueStackBase = call.valueStackBase;
}

void wasmu_pushCall(wasmu_Context* context, wasmu_Call call) {
    wasmu_CallStack* stack = &context->callStack;

    // TODO: Allow call stack to have a maximum size defined to catch infinite recursion
    stack->count++;

    if (stack->count > stack->size) {
        stack->size = stack->count;
        stack->calls = WASMU_REALLOC(stack->calls, stack->size * sizeof(wasmu_Call));
    }

    stack->calls[stack->count - 1] = call;

    wasmu_populateActiveCallInfo(context, call);
}

wasmu_Bool wasmu_popCall(wasmu_Context* context, wasmu_Call* returnedCall) {
    wasmu_CallStack* stack = &context->callStack;

    if (stack->count == 0) {
        return WASMU_FALSE;
    }

    stack->count--;

    if (stack->count == 0) {
        context->activeModule = WASMU_NULL;

        return WASMU_TRUE;
    }

    wasmu_Call call = stack->calls[stack->count - 1];

    wasmu_populateActiveCallInfo(context, call);

    if (returnedCall) {
        *returnedCall = call;
    }

    return WASMU_TRUE;
}

void wasmu_growValueStack(wasmu_ValueStack* stack, wasmu_Count newPosition) {
    if (newPosition > stack->size) {
        stack->size = newPosition;
        stack->data = WASMU_REALLOC(stack->data, stack->size);
    }
}

wasmu_I32 wasmu_stackGetI32(wasmu_Context* context, wasmu_Count index) {
    wasmu_I32 value = 0;
    wasmu_ValueStack* stack = &context->valueStack;

    for (wasmu_Count i = 0; i < 4; i++) {
        value <<= 8;
        value |= stack->data[index + i];
    }

    return value;
}

void wasmu_stackSetI32(wasmu_Context* context, wasmu_Count index, wasmu_I32 value) {
    wasmu_ValueStack* stack = &context->valueStack;

    wasmu_growValueStack(stack, index + 4);

    for (wasmu_Count i = 0; i < 4; i++) {
        stack->data[index + i] = value & 0xFF;
        value >>= 8;
    }
}

void wasmu_pushI32(wasmu_Context* context, wasmu_I32 value) {
    wasmu_ValueStack* stack = &context->valueStack;

    wasmu_stackSetI32(context, stack->position, value);

    stack->position += 4;
}

wasmu_I32 wasmu_popI32(wasmu_Context* context) {
    wasmu_ValueStack* stack = &context->valueStack;

    if (stack->position < 4) {
        WASMU_DEBUG_LOG("Value stack underflow");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return 0;
    }

    wasmu_I32 value = wasmu_stackGetI32(context, stack->position);

    stack->position -= 4;

    return value;
}

wasmu_Bool wasmu_callFunctionByIndex(wasmu_Context* context, wasmu_Count moduleIndex, wasmu_Count functionIndex) {
    wasmu_Module** modulePtr = WASMU_GET_ENTRY(context->modules, context->modulesCount, moduleIndex);
    wasmu_Module* module = *modulePtr;

    if (!module) {
        return WASMU_FALSE;
    }

    wasmu_Function* function = WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);

    if (!function) {
        return WASMU_FALSE;
    }

    module->position = function->codePosition;

    wasmu_Count valueStackBase = context->valueStack.position;
    wasmu_FunctionSignature* signature = WASMU_GET_ENTRY(module->functionSignatures, module->functionSignaturesCount, function->signatureIndex);

    valueStackBase -= signature->parametersStackSize;

    WASMU_DEBUG_LOG("Decrease stack base by %d for %d parameters", signature->parametersStackSize, signature->parametersCount);

    wasmu_pushCall(context, (wasmu_Call) {
        .moduleIndex = moduleIndex,
        .functionIndex = functionIndex,
        .position = function->codePosition,
        .valueStackBase = valueStackBase
    });

    WASMU_DEBUG_LOG(
        "Pushed call - moduleIndex: %d, functionIndex: %d, position: 0x%08x, valueStackBase: %d (size: %d)",
        moduleIndex, functionIndex, function->codePosition, valueStackBase, function->codeSize
    );

    return WASMU_TRUE;
}

wasmu_Bool wasmu_callFunction(wasmu_Module* module, wasmu_Function* function) {
    wasmu_Context* context = module->context;
    wasmu_Count moduleIndex = -1;
    wasmu_Count functionIndex = -1;

    for (wasmu_Count i = 0; i < context->modulesCount; i++) {
        if (context->modules[i] == module) {
            moduleIndex = i;
            break;
        }
    }

    if (moduleIndex == -1) {
        return WASMU_FALSE;
    }

    for (wasmu_Count i = 0; i < module->functionsCount; i++) {
        if (&(module->functions[i]) == function) {
            functionIndex = i;
            break;
        }
    }

    if (functionIndex == -1) {
        return WASMU_FALSE;
    }

    return wasmu_callFunctionByIndex(context, moduleIndex, functionIndex);
}

wasmu_Bool wasmu_step(wasmu_Context* context) {
    if (!wasmu_isRunning(context)) {
        return WASMU_FALSE;
    }

    wasmu_Module* module = context->activeModule;

    if (!module) {
        return WASMU_FALSE;
    }

    wasmu_U8 opcode = WASMU_NEXT();

    WASMU_DEBUG_LOG("Execute opcode: 0x%02x", opcode);

    // TODO: Implement actual functionality of these opcodes

    switch (opcode) {
        case WASMU_OP_LOCAL_GET:
        {
            wasmu_Function* function = context->activeFunction;
            wasmu_Count localIndex = wasmu_readUInt(module);

            WASMU_DEBUG_LOG("Get local: %d", localIndex);

            break;
        }

        case WASMU_OP_END:
        {
            WASMU_DEBUG_LOG("End");

            wasmu_popCall(context, WASMU_NULL);

            break;
        }

        case WASMU_OP_I32_ADD:
        {
            WASMU_DEBUG_LOG("Add I32");

            break;
        }

        default:
            WASMU_DEBUG_LOG("Opcode not implemented");
            context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
            return WASMU_FALSE;
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_runFunction(wasmu_Module* module, wasmu_Function* function) {
    WASMU_DEBUG_LOG("Run function");

    wasmu_Bool called = wasmu_callFunction(module, function);

    if (!called) {
        return WASMU_FALSE;
    }

    while (wasmu_isRunning(module->context)) {
        WASMU_DEBUG_LOG("Step");

        if (!wasmu_step(module->context)) {
            return WASMU_FALSE;
        }
    }

    WASMU_DEBUG_LOG("Function returned");

    return WASMU_TRUE;
}