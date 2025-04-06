wasmu_Context* wasmu_newContext() {
    WASMU_DEBUG_LOG("Create new context");

    wasmu_Context* context = WASMU_NEW(wasmu_Context);

    context->errorState = WASMU_ERROR_STATE_NONE;
    context->callStack.calls = WASMU_MALLOC(0);
    context->callStack.size = 0;
    context->callStack.count = 0;
    context->valueStack.data = WASMU_MALLOC(0);
    context->valueStack.size = 0;
    context->valueStack.position = 0;
    context->activeModule = WASMU_NULL;
    context->activeFunction = WASMU_NULL;
    context->activeFunctionSignature = WASMU_NULL;
    context->currentValueStackBase = 0;
    context->currentStackLocals = WASMU_MALLOC(0);
    context->currentStackLocalsCount = 0;

    WASMU_INIT_ENTRIES(context->modules, context->modulesCount);

    return context;
}

wasmu_Bool wasmu_isRunning(wasmu_Context* context) {
    return context->callStack.count > 0;
}