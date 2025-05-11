wasmu_Context* wasmu_newContext() {
    WASMU_DEBUG_LOG("Create new context");

    wasmu_Context* context = WASMU_NEW(wasmu_Context);

    context->errorState = WASMU_ERROR_STATE_NONE;
    context->callStack.calls = (wasmu_Call*)WASMU_MALLOC(0);
    context->callStack.size = 0;
    context->callStack.count = 0;
    context->labelStack.labels = (wasmu_Label*)WASMU_MALLOC(0);
    context->labelStack.size = 0;
    context->labelStack.count = 0;
    context->typeStack.types = (wasmu_ValueType*)WASMU_MALLOC(0);
    context->typeStack.size = 0;
    context->typeStack.count = 0;
    context->valueStack.data = (wasmu_U8*)WASMU_MALLOC(0);
    context->valueStack.size = 0;
    context->valueStack.position = 0;
    context->activeModule = WASMU_NULL;
    context->activeModuleIndex = 0;
    context->activeFunction = WASMU_NULL;
    context->activeFunctionSignature = WASMU_NULL;
    context->currentTypeStackBase = 0;
    context->currentValueStackBase = 0;
    context->currentStackLocals = (wasmu_StackLocal*)WASMU_MALLOC(0);
    context->currentStackLocalsCount = 0;
    context->fastForward = WASMU_FALSE;
    context->fastForwardTargetOpcode = WASMU_OP_UNREACHABLE;
    context->fastForwardLabelDepth = 0;

    WASMU_INIT_ENTRIES(context->modules, context->modulesCount);

    return context;
}

void wasmu_destroyContext(wasmu_Context* context) {
    for (wasmu_Count i = 0; i < context->modulesCount; i++) {
        wasmu_destroyModule(context->modules[i]);
    }

    WASMU_FREE(context->callStack.calls);
    WASMU_FREE(context->labelStack.labels);
    WASMU_FREE(context->typeStack.types);
    WASMU_FREE(context->valueStack.data);
    WASMU_FREE(context->currentStackLocals);
    WASMU_FREE(context->modules);
    WASMU_FREE(context);
}

wasmu_Bool wasmu_isRunning(wasmu_Context* context) {
    return context->callStack.count > 0;
}