wasmu_Bool wasmu_parseTypesSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count typesCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < typesCount; i++) {
        WASMU_DEBUG_LOG("Read signature type at %08x", module->position);

        switch (WASMU_NEXT()) {
            case WASMU_SIGNATURE_TYPE_FUNCTION:
            {
                WASMU_DEBUG_LOG("Signature type: function");

                wasmu_FunctionSignature signature;

                WASMU_INIT_ENTRIES(signature.parameters, signature.parametersCount);
                WASMU_INIT_ENTRIES(signature.results, signature.resultsCount);

                wasmu_Count parametersCount = wasmu_readUInt(module);

                for (wasmu_Count j = 0; j < parametersCount; j++) {
                    WASMU_ADD_ENTRY(signature.parameters, signature.parametersCount, WASMU_NEXT());
                }

                wasmu_Count resultsCount = wasmu_readUInt(module);
                
                for (wasmu_Count j = 0; j < resultsCount; j++) {
                    WASMU_ADD_ENTRY(signature.results, signature.resultsCount, WASMU_NEXT());
                }

                WASMU_ADD_ENTRY(module->functionSignatures, module->functionSignaturesCount, signature);

                WASMU_DEBUG_LOG("Parameters count: %d", signature.parametersCount);
                WASMU_DEBUG_LOG("Results count: %d", signature.resultsCount);

                break;
            }

            default:
                WASMU_DEBUG_LOG("Signature type not implemented");

                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }

        WASMU_DEBUG_LOG("End of signature type");
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseFunctionSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count functionsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < functionsCount; i++) {
        wasmu_Function function;

        function.signatureIndex = wasmu_readUInt(module);

        WASMU_ADD_ENTRY(module->functions, module->functionsCount, function);

        WASMU_DEBUG_LOG("Add function - signature: %d", function.signatureIndex);
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseSections(wasmu_Module* module) {
    WASMU_DEBUG_LOG("Parse sections");

    static wasmu_U8 magic[] = {0x00, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00};

    for (wasmu_Count i = 0; i < sizeof(magic); i++) {
        if (WASMU_NEXT() != magic[i]) {
            WASMU_DEBUG_LOG("Invalid magic at %08x", module->position - 1);

            return WASMU_FALSE;
        }
    }

    WASMU_DEBUG_LOG("Magic matched");

    while (WASMU_AVAILABLE()) {
        WASMU_DEBUG_LOG("Read section at %08x", module->position);

        switch (WASMU_NEXT()) {
            case WASMU_SECTION_TYPE:
                WASMU_DEBUG_LOG("Section: type");
                if (!wasmu_parseTypesSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_FUNCTION:
                WASMU_DEBUG_LOG("Section: function");
                if (!wasmu_parseFunctionSection(module)) {return WASMU_FALSE;}
                break;

            // TODO: Implement these sections (skipping for now)
            case WASMU_SECTION_EXPORT:
            case WASMU_SECTION_CODE:
            case WASMU_SECTION_CUSTOM:
            {
                WASMU_DEBUG_LOG("Section: to implement");

                wasmu_Count size = wasmu_readUInt(module);

                for (wasmu_Count i = 0; i < size; i++) {
                    WASMU_NEXT();
                }

                break;
            }

            default:
                WASMU_DEBUG_LOG("Section type not implemented");

                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }

        WASMU_DEBUG_LOG("End of section");
    }

    return WASMU_TRUE;
}