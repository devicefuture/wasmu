wasmu_Bool wasmu_callFunctionByIndex(wasmu_Context* context, wasmu_Count moduleIndex, wasmu_Count functionIndex) {
    wasmu_Module** modulePtr = WASMU_GET_ENTRY(context->modules, context->modulesCount, moduleIndex);
    wasmu_Module* module = *modulePtr;
    wasmu_Module* callingModule = module;

    if (!module) {
        return WASMU_FALSE;
    }

    wasmu_Function* function = WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);

    if (!function) {
        WASMU_DEBUG_LOG("Unknown function");
        context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
        return WASMU_FALSE;
    }

    wasmu_Count resolutionDepth = 0;

    while (function->importIndex != -1) {
        if (resolutionDepth >= WASMU_IMPORT_RESOLUTION_DEPTH) {
            WASMU_DEBUG_LOG("Import resolution depth exceeded");
            context->errorState = WASMU_ERROR_STATE_DEPTH_EXCEEDED;
            return WASMU_FALSE;
        }

        resolutionDepth++;

        wasmu_Import* moduleImport = WASMU_GET_ENTRY(module->imports, module->importsCount, function->importIndex);

        if (!moduleImport || moduleImport->type != WASMU_EXPORT_TYPE_FUNCTION) {
            WASMU_DEBUG_LOG("Unknown import");
            context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
            return WASMU_FALSE;
        }

        if (moduleImport->resolvedModuleIndex == -1) {
            WASMU_DEBUG_LOG("Import has not yet been resolved");
            context->errorState = WASMU_ERROR_STATE_PRECONDITION_FAILED;
            return WASMU_FALSE;
        }

        moduleIndex = moduleImport->resolvedModuleIndex;
        module = *WASMU_GET_ENTRY(context->modules, context->modulesCount, moduleIndex);

        if (!module) {
            WASMU_DEBUG_LOG("Unknown resolved module");
            context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
            return WASMU_FALSE;
        }

        functionIndex = moduleImport->data.asFunctionIndex;
        function = WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);

        if (!function) {
            WASMU_DEBUG_LOG("Unknown resolved function");
            context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
            return WASMU_FALSE;
        }
    }

    if (context->callStack.count > 0) {
        // Save current position on current topmost call
        context->callStack.calls[context->callStack.count - 1].position = callingModule->position;
    }

    module->position = function->codePosition;

    wasmu_Count valueStackBase = context->valueStack.position;
    wasmu_FunctionSignature* signature = WASMU_GET_ENTRY(module->functionSignatures, module->functionSignaturesCount, function->signatureIndex);

    if (!signature) {
        WASMU_DEBUG_LOG("Unknown function signature");
        context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
        return WASMU_FALSE;
    }

    WASMU_DEBUG_LOG("Decrease stack base by %d for %d parameters", signature->parametersStackSize, signature->parametersCount);

    valueStackBase -= signature->parametersStackSize;

    for (wasmu_Count i = 0; i < function->localsCount; i++) {
        wasmu_Count typeSize = wasmu_getValueTypeSize(function->locals[i]);

        WASMU_DEBUG_LOG("Allocate local on stack - type: 0x%02x (typeSize: %d)", function->locals[i], typeSize);

        wasmu_pushInt(context, typeSize, 0);
    }

    if (context->typeStack.count < signature->parametersCount) {
        WASMU_DEBUG_LOG("Not enough items on value stack to call function (current: %d, needed: %d)", context->typeStack.count, signature->parametersCount);
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_FALSE;
    }

    wasmu_pushCall(context, (wasmu_Call) {
        .moduleIndex = moduleIndex,
        .functionIndex = functionIndex,
        .position = function->codePosition,
        .typeStackBase = context->typeStack.count - signature->parametersCount,
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
        WASMU_DEBUG_LOG("Module does not exist in context");
        module->context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
        return WASMU_FALSE;
    }

    for (wasmu_Count i = 0; i < module->functionsCount; i++) {
        if (&(module->functions[i]) == function) {
            functionIndex = i;
            break;
        }
    }

    if (functionIndex == -1) {
        WASMU_DEBUG_LOG("Function does not exist in module");
        module->context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
        return WASMU_FALSE;
    }

    return wasmu_callFunctionByIndex(context, moduleIndex, functionIndex);
}

void wasmu_returnFromFunction(wasmu_Context* context) {
    wasmu_ValueStack* stack = &context->valueStack;

    wasmu_Count resultsOffset = 0;
    wasmu_Count totalLocalsSize = 0;

    // First, get the total sizes of parameters, results and locals, popping all types

    for (wasmu_Count i = 0; i < context->currentStackLocalsCount; i++) {
        wasmu_StackLocal local = context->currentStackLocals[i];

        totalLocalsSize += local.size;

        if (local.type == WASMU_LOCAL_TYPE_PARAMETER || local.type == WASMU_LOCAL_TYPE_LOCAL) {
            resultsOffset += local.size;
        }
    }

    wasmu_Count base = context->currentValueStackBase;
    wasmu_Count nonLocalsSize = stack->position - base - totalLocalsSize;

    // Ensure that results offset also accounts for stack values related to parameters, results and locals

    if (nonLocalsSize > 0) {
        WASMU_DEBUG_LOG("Popping non-locals - size %d", nonLocalsSize);

        resultsOffset += nonLocalsSize;
    }

    // Then if there are non-result values, remove them from the stack to clean it, shifting the results up

    for (wasmu_Count i = 0; i < resultsOffset; i++) {
        stack->data[base + i] = stack->data[base + resultsOffset + i];
    }

    stack->position -= resultsOffset;

    // Finally, restore type stack base and re-add result types to type stack

    context->typeStack.count = context->currentTypeStackBase;

    for (wasmu_Count i = 0; i < context->currentStackLocalsCount; i++) {
        wasmu_StackLocal local = context->currentStackLocals[i];

        if (local.type == WASMU_LOCAL_TYPE_RESULT) {
            wasmu_pushType(context, local.valueType);
        }
    }

    // Now we can jump back to where we were in the calling function by popping from the call stack and restoring the position

    wasmu_popCall(context, WASMU_NULL);

    if (context->callStack.count > 0) {
        context->activeModule->position = context->callStack.calls[context->callStack.count - 1].position;
    }
}