wasmu_Bool wasmu_parseCustomSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count positionBeforeName = module->position;

    wasmu_CustomSection customSection;

    customSection.name = wasmu_readString(module);
    customSection.dataPosition = module->position;
    customSection.dataSize = size - (module->position - positionBeforeName);

    WASMU_ADD_ENTRY(module->customSections, module->customSectionsCount, customSection);

    #ifdef WASMU_DEBUG
        wasmu_U8* nameChars = wasmu_getNullTerminatedChars(customSection.name);

        WASMU_DEBUG_LOG(
            "Add custom section - name: \"%s\", dataPosition: 0x%08x, dataSize: %d",
            nameChars, customSection.dataPosition, customSection.dataSize
        );

        WASMU_FREE(nameChars);
    #endif

    module->position += customSection.dataSize;

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseTypesSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count typesCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < typesCount; i++) {
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

                WASMU_DEBUG_LOG("Add function signature - parametersCount: %d, resultsCount: %d", signature.parametersCount, signature.resultsCount);

                break;
            }

            default:
                WASMU_DEBUG_LOG("Signature type not implemented");
                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseFunctionSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count functionsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < functionsCount; i++) {
        wasmu_Function function;

        function.signatureIndex = wasmu_readUInt(module);
        function.codePosition = 0;
        function.codeSize = 0;
        function.declarationsCount = 0;

        WASMU_ADD_ENTRY(module->functions, module->functionsCount, function);

        WASMU_DEBUG_LOG(
            "Add function - signature: %d, position: 0x%08x, size: %d, declarationsCount: %d",
            function.signatureIndex, function.codePosition, function.codeSize, function.declarationsCount
        );
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseExportSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readInt(module);
    wasmu_Count exportsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < exportsCount; i++) {
        wasmu_Export export;

        export.name = wasmu_readString(module);
        export.type = WASMU_NEXT();

        switch (export.type) {
            case WASMU_EXPORT_TYPE_FUNCTION:
            {
                WASMU_DEBUG_LOG("Export type: function");

                export.data.asFunctionIndex = wasmu_readUInt(module);

                #ifdef WASMU_DEBUG
                    wasmu_U8* nameChars = wasmu_getNullTerminatedChars(export.name);

                    WASMU_DEBUG_LOG("Add function export - name: \"%s\", functionIndex: %d", nameChars, export.data.asFunctionIndex);

                    WASMU_FREE(nameChars);
                #endif

                break;
            }

            default:
                WASMU_DEBUG_LOG("Export type not implemented");
                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }

        WASMU_ADD_ENTRY(module->exports, module->exportsCount, export);
    }
}

wasmu_Bool wasmu_parseCodeSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readInt(module);
    wasmu_Count bodiesCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < bodiesCount; i++) {
        wasmu_Count functionIndex = module->nextFunctionIndexForCode++;

        wasmu_Function* function = WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);

        if (!function) {
            WASMU_DEBUG_LOG("No function exists for code body");

            module->context->errorState = WASMU_ERROR_STATE_CODE_BODY_MISMATCH;

            return WASMU_FALSE;
        }

        function->codeSize = wasmu_readUInt(module);

        wasmu_Count positionBeforeDeclarationsCount = module->position;

        function->declarationsCount = wasmu_readUInt(module);
        function->codePosition = module->position;

        WASMU_DEBUG_LOG("Add code - position: 0x%08x, size: %d (ends: 0x%08x)", function->codePosition, function->codeSize, function->codePosition + function->codeSize - 1);

        module->position = positionBeforeDeclarationsCount + function->codeSize;
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseSections(wasmu_Module* module) {
    WASMU_DEBUG_LOG("Parse sections");

    static wasmu_U8 magic[] = {0x00, 0x61, 0x73, 0x6D, 0x01, 0x00, 0x00, 0x00};

    for (wasmu_Count i = 0; i < sizeof(magic); i++) {
        if (WASMU_NEXT() != magic[i]) {
            WASMU_DEBUG_LOG("Invalid magic at 0x%08x", module->position - 1);

            return WASMU_FALSE;
        }
    }

    WASMU_DEBUG_LOG("Magic matched");

    while (WASMU_AVAILABLE()) {
        WASMU_DEBUG_LOG("Read section at 0x%08x", module->position);

        switch (WASMU_NEXT()) {
            case WASMU_SECTION_CUSTOM:
                WASMU_DEBUG_LOG("Section: custom");
                if (!wasmu_parseCustomSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_TYPE:
                WASMU_DEBUG_LOG("Section: type");
                if (!wasmu_parseTypesSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_FUNCTION:
                WASMU_DEBUG_LOG("Section: function");
                if (!wasmu_parseFunctionSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_EXPORT:
                WASMU_DEBUG_LOG("Section: export");
                if (!wasmu_parseExportSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_CODE:
                WASMU_DEBUG_LOG("Section: code");
                if (!wasmu_parseCodeSection(module)) {return WASMU_FALSE;}
                break;

            default:
                WASMU_DEBUG_LOG("Section type not implemented");
                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }

        WASMU_DEBUG_LOG("End of section");
    }

    return WASMU_TRUE;
}