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

                signature.parametersStackSize = 0;
                signature.resultsStackSize = 0;

                WASMU_INIT_ENTRIES(signature.parameters, signature.parametersCount);
                WASMU_INIT_ENTRIES(signature.results, signature.resultsCount);

                wasmu_Count parametersCount = wasmu_readUInt(module);

                for (wasmu_Count j = 0; j < parametersCount; j++) {
                    wasmu_ValueType parameterType = (wasmu_ValueType)WASMU_NEXT();

                    signature.parametersStackSize += wasmu_getValueTypeSize(parameterType);

                    WASMU_ADD_ENTRY(signature.parameters, signature.parametersCount, parameterType);
                }

                wasmu_Count resultsCount = wasmu_readUInt(module);
                
                for (wasmu_Count j = 0; j < resultsCount; j++) {
                    wasmu_ValueType resultType = (wasmu_ValueType)WASMU_NEXT();

                    signature.resultsStackSize += wasmu_getValueTypeSize(resultType);

                    WASMU_ADD_ENTRY(signature.results, signature.resultsCount, resultType);
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

wasmu_Bool wasmu_parseImportSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count importsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < importsCount; i++) {
        wasmu_Import moduleImport;

        moduleImport.moduleName = wasmu_readString(module);
        moduleImport.name = wasmu_readString(module);
        moduleImport.type = (wasmu_ExportType)WASMU_NEXT();
        moduleImport.resolvedModuleIndex = -1;

        switch (moduleImport.type) {
            case WASMU_EXPORT_TYPE_FUNCTION:
            {
                WASMU_DEBUG_LOG("Import type: function");

                wasmu_Function function;

                function.signatureIndex = wasmu_readUInt(module);
                function.importIndex = module->importsCount;
                function.nativeFunction = WASMU_NULL;

                WASMU_ADD_ENTRY(module->functions, module->functionsCount, function);

                #ifdef WASMU_DEBUG
                    wasmu_U8* moduleNameChars = wasmu_getNullTerminatedChars(moduleImport.moduleName);
                    wasmu_U8* nameChars = wasmu_getNullTerminatedChars(moduleImport.name);

                    WASMU_DEBUG_LOG(
                        "Add function import - moduleName: \"%s\", name: \"%s\", functionIndex: %d, signatureIndex: %d",
                        moduleNameChars, nameChars, module->functionsCount - 1, function.signatureIndex
                    );

                    WASMU_FREE(moduleNameChars);
                    WASMU_FREE(nameChars);
                #endif

                break;
            }

            default:
                WASMU_DEBUG_LOG("Import type not implemented");
                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }

        WASMU_ADD_ENTRY(module->imports, module->importsCount, moduleImport);
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseFunctionSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count functionsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < functionsCount; i++) {
        wasmu_Function function;

        function.signatureIndex = wasmu_readUInt(module);
        function.importIndex = -1;
        function.nativeFunction = WASMU_NULL;
        function.codePosition = 0;
        function.codeSize = 0;

        WASMU_INIT_ENTRIES(function.locals, function.localsCount);

        WASMU_ADD_ENTRY(module->functions, module->functionsCount, function);

        WASMU_DEBUG_LOG(
            "Add function - signature: %d, position: 0x%08x, size: %d, localsCount: %d",
            function.signatureIndex, function.codePosition, function.codeSize, function.localsCount
        );
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseMemorySection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count memoriesCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < memoriesCount; i++) {
        wasmu_Memory memory;

        memory.data = (wasmu_U8*)WASMU_MALLOC(0);
        memory.size = 0;

        wasmu_U8 limitsFlag = WASMU_NEXT();

        memory.pagesCount = memory.minPages = wasmu_readUInt(module);
        memory.maxPages = limitsFlag == 0x01 ? wasmu_readUInt(module) : -1;

        WASMU_ADD_ENTRY(module->memories, module->memoriesCount, memory);

        WASMU_DEBUG_LOG("Add memory - minPages: %d, maxPages: %d", memory.minPages, memory.maxPages);
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseGlobalSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count globalsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < globalsCount; i++) {
        wasmu_TypedValue global;

        global.type = (wasmu_ValueType)WASMU_NEXT();

        WASMU_NEXT(); // Mutability — not required for now; just assume everything is mutable
        WASMU_NEXT(); // Const opcode — also not required until global importing is implemented

        switch (global.type) {
            case WASMU_VALUE_TYPE_I32:
                global.value.asInt = wasmu_readInt(module);
                WASMU_DEBUG_LOG("Add global - type: %d, value: %d", global.type, global.value.asInt);
                break;

            default:
                WASMU_DEBUG_LOG("Global value type not implemented");
                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }

        WASMU_NEXT(); // End opcode

        WASMU_ADD_ENTRY(module->globals, module->globalsCount, global);
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseExportSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count exportsCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < exportsCount; i++) {
        wasmu_Export moduleExport;

        moduleExport.name = wasmu_readString(module);
        moduleExport.type = (wasmu_ExportType)WASMU_NEXT();

        switch (moduleExport.type) {
            case WASMU_EXPORT_TYPE_FUNCTION:
            {
                WASMU_DEBUG_LOG("Export type: function");

                moduleExport.data.asFunctionIndex = wasmu_readUInt(module);

                #ifdef WASMU_DEBUG
                    wasmu_U8* nameChars = wasmu_getNullTerminatedChars(moduleExport.name);

                    WASMU_DEBUG_LOG("Add function export - name: \"%s\", functionIndex: %d", nameChars, moduleExport.data.asFunctionIndex);

                    WASMU_FREE(nameChars);
                #endif

                break;
            }

            default:
                WASMU_DEBUG_LOG("Export type not implemented");
                module->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
                return WASMU_FALSE;
        }

        WASMU_ADD_ENTRY(module->exports, module->exportsCount, moduleExport);
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_parseCodeSection(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_Count bodiesCount = wasmu_readUInt(module);

    for (wasmu_Count i = 0; i < bodiesCount; i++) {
        wasmu_Count functionIndex = module->nextFunctionIndexForCode++;
        wasmu_Function* function = WASMU_NULL;

        while (WASMU_TRUE) {
            function = WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);

            if (!function) {
                WASMU_DEBUG_LOG("No function exists for code body");

                module->context->errorState = WASMU_ERROR_STATE_CODE_BODY_MISMATCH;

                return WASMU_FALSE;
            }

            if (function->importIndex == -1) {
                break;
            }

            functionIndex = module->nextFunctionIndexForCode++;
        }

        function->codeSize = wasmu_readUInt(module);

        wasmu_Count positionBeforeLocals = module->position;

        wasmu_Count localDeclarationsCount = wasmu_readUInt(module);

        for (wasmu_Count j = 0; j < localDeclarationsCount; j++) {
            wasmu_Count typeLocalsCount = wasmu_readUInt(module);
            wasmu_ValueType type = (wasmu_ValueType)WASMU_NEXT();

            WASMU_DEBUG_LOG("Add local declaration - type 0x%02x, count %d", type, typeLocalsCount);

            for (wasmu_Count k = 0; k < typeLocalsCount; k++) {
                WASMU_ADD_ENTRY(function->locals, function->localsCount, type);
            }
        }

        function->codePosition = module->position;

        WASMU_DEBUG_LOG("Add code - position: 0x%08x, size: %d (ends: 0x%08x)", function->codePosition, function->codeSize, function->codePosition + function->codeSize - 1);

        module->position = positionBeforeLocals + function->codeSize;
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

            case WASMU_SECTION_IMPORT:
                WASMU_DEBUG_LOG("Section: import");
                if (!wasmu_parseImportSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_FUNCTION:
                WASMU_DEBUG_LOG("Section: function");
                if (!wasmu_parseFunctionSection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_MEMORY:
                WASMU_DEBUG_LOG("Section: memory");
                if (!wasmu_parseMemorySection(module)) {return WASMU_FALSE;}
                break;

            case WASMU_SECTION_GLOBAL:
                WASMU_DEBUG_LOG("Section: global");
                if (!wasmu_parseGlobalSection(module)) {return WASMU_FALSE;}
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