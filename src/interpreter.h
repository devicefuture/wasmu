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

wasmu_Bool wasmu_callFunction(wasmu_Module* module, wasmu_Count functionIndex) {
    wasmu_Context* context = module->context;
    wasmu_Count moduleIndex = -1;

    for (wasmu_Count i = 0; i < context->modulesCount; i++) {
        if (context->modules[i] == module) {
            moduleIndex = i;
            break;
        }
    }

    if (moduleIndex == -1) {
        return WASMU_FALSE;
    }

    wasmu_Function* function = WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);

    if (!function) {
        return WASMU_FALSE;
    }

    module->position = function->codePosition;

    wasmu_pushCall(context, (wasmu_Call) {
        .moduleIndex = moduleIndex,
        .functionIndex = functionIndex,
        .position = function->codePosition
    });

    WASMU_DEBUG_LOG(
        "Pushed call - moduleIndex: %d, functionIndex: %d, position: 0x%08x (size: %d)",
        moduleIndex, functionIndex, function->codePosition, function->codeSize
    );

    return WASMU_TRUE;
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

    WASMU_DEBUG_LOG("Execute opcode: %02x", opcode);

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

wasmu_Bool wasmu_runFunction(wasmu_Module* module, wasmu_Count functionIndex) {
    WASMU_DEBUG_LOG("Call function: %d", functionIndex);

    wasmu_Bool called = wasmu_callFunction(module, functionIndex);

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