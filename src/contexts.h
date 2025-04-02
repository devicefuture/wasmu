wasmu_Context* wasmu_newContext() {
    WASMU_DEBUG_LOG("Create new context");

    wasmu_Context* context = WASMU_NEW(wasmu_Context);

    context->errorState = WASMU_ERROR_STATE_NONE;

    WASMU_INIT_ENTRIES(context->modules, context->modulesCount);

    return context;
}