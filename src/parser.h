wasmu_Bool wasmu_parseTypesSection(wasmu_Context* context) {
    wasmu_Count size = wasmu_readUInt(context);
    wasmu_Count typesCount = wasmu_readUInt(context);

    for (wasmu_Count i = 0; i < typesCount; i++) {
        switch (WASMU_NEXT()) {
            case WASMU_VALUE_TYPE_FUNCTION:
            {
                wasmu_FunctionType functionType;

                WASMU_INIT_ENTRIES(functionType.parameters, functionType.parametersCount);
                WASMU_INIT_ENTRIES(functionType.results, functionType.resultsCount);

                wasmu_Count parametersCount = wasmu_readUInt(context);

                for (wasmu_Count j = 0; j < parametersCount; j++) {
                    WASMU_ADD_ENTRY(functionType.parameters, functionType.parametersCount, WASMU_NEXT());
                }

                wasmu_Count resultsCount = wasmu_readUInt(context);
                

                for (wasmu_Count j = 0; j < resultsCount; j++) {
                    WASMU_ADD_ENTRY(functionType.results, functionType.resultsCount, WASMU_NEXT());
                }

                WASMU_ADD_ENTRY(context->functionTypes, context->functionTypesCount, functionType);

                break;
            }

            default:
                context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseFunctionSection(wasmu_Context* context) {
    wasmu_Count size = wasmu_readUInt(context);

    for (wasmu_Count i = 0; i < size; i++) {
        WASMU_NEXT();
    }
}

wasmu_Bool wasmu_parseSections(wasmu_Context* context) {
    static wasmu_U8 magic[] = {0x00, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00};

    for (wasmu_Count i = 0; i < sizeof(magic); i++) {
        if (WASMU_NEXT() != magic[i]) {
            return WASMU_FALSE;
        }
    }

    while (WASMU_AVAILABLE()) {
        switch (WASMU_NEXT()) {
            case WASMU_SECTION_TYPE:
                if (!wasmu_parseTypesSection(context)) {return WASMU_FALSE;}
                break;

            // TODO: Implement these sections (skipping for now)
            case WASMU_SECTION_FUNCTION:
            case WASMU_SECTION_EXPORT:
            case WASMU_SECTION_CODE:
            case WASMU_SECTION_CUSTOM:
            {
                wasmu_Count size = wasmu_readUInt(context);

                for (wasmu_Count i = 0; i < size; i++) {
                    WASMU_NEXT();
                }

                break;
            }

            default:
                context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }
    }

    return WASMU_TRUE;
}