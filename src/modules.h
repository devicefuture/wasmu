wasmu_Module* wasmu_newModule(wasmu_Context* context) {
    wasmu_Module* module = WASMU_NEW(wasmu_Module);

    module->context = context;
    module->name = WASMU_NULL;
    module->code = WASMU_NULL;
    module->codeSize = 0;
    module->position = 0;
    module->nextFunctionIndexForCode = 0;

    WASMU_INIT_ENTRIES(module->customSections, module->customSectionsCount);
    WASMU_INIT_ENTRIES(module->functionSignatures, module->functionSignaturesCount);
    WASMU_INIT_ENTRIES(module->imports, module->importsCount);
    WASMU_INIT_ENTRIES(module->functions, module->functionsCount);
    WASMU_INIT_ENTRIES(module->tables, module->tablesCount);
    WASMU_INIT_ENTRIES(module->memories, module->memoriesCount);
    WASMU_INIT_ENTRIES(module->globals, module->globalsCount);
    WASMU_INIT_ENTRIES(module->exports, module->exportsCount);

    WASMU_ADD_ENTRY(context->modules, context->modulesCount, module);

    return module;
}

void wasmu_destroyModule(wasmu_Module* module) {
    for (wasmu_Count i = 0; i < module->customSectionsCount; i++) {
        wasmu_CustomSection customSection = module->customSections[i];

        WASMU_FREE(customSection.name.chars);
    }

    for (wasmu_Count i = 0; i < module->functionSignaturesCount; i++) {
        wasmu_FunctionSignature functionSignature = module->functionSignatures[i];

        WASMU_FREE(functionSignature.parameters);
        WASMU_FREE(functionSignature.results);
    }

    for (wasmu_Count i = 0; i < module->importsCount; i++) {
        wasmu_Import moduleImport = module->imports[i];

        WASMU_FREE(moduleImport.moduleName.chars);
        WASMU_FREE(moduleImport.name.chars);
    }

    for (wasmu_Count i = 0; i < module->functionsCount; i++) {
        wasmu_Function function = module->functions[i];

        if (function.locals) {
            WASMU_FREE(function.locals);
        }
    }

    for (wasmu_Count i = 0; i < module->tablesCount; i++) {
        wasmu_Table table = module->tables[i];

        WASMU_FREE(table.entries);
    }

    for (wasmu_Count i = 0; i < module->memoriesCount; i++) {
        wasmu_Memory memory = module->memories[i];

        WASMU_FREE(memory.data);
    }

    for (wasmu_Count i = 0; i < module->exportsCount; i++) {
        wasmu_Export moduleExport = module->exports[i];

        WASMU_FREE(moduleExport.name.chars);
    }

    if (module->name) {
        WASMU_FREE(module->name);
    }

    WASMU_FREE(module->customSections);
    WASMU_FREE(module->functionSignatures);
    WASMU_FREE(module->imports);
    WASMU_FREE(module->functions);
    WASMU_FREE(module->tables);
    WASMU_FREE(module->memories);
    WASMU_FREE(module->globals);
    WASMU_FREE(module->exports);
    WASMU_FREE(module);
}

void wasmu_load(wasmu_Module* module, wasmu_U8* code, wasmu_Count codeSize) {
    WASMU_DEBUG_LOG("Load code - size: %d", codeSize);

    module->code = code;
    module->codeSize = codeSize;
    module->position = 0;
}

void wasmu_assignModuleName(wasmu_Module* module, const wasmu_U8* name) {
    module->name = wasmu_copyChars(name);
}

wasmu_U8 wasmu_read(wasmu_Module* module, wasmu_Count position) {
    if (!module->code || position >= module->codeSize) {
        module->context->errorState = WASMU_ERROR_STATE_MEMORY_OOB;

        return 0;
    }

    return module->code[position];
}

wasmu_U8 wasmu_readNext(wasmu_Module* module) {
    return wasmu_read(module, module->position++);
}

// Unsigned integers are encoded as LEB128
// @source reference https://en.wikipedia.org/wiki/LEB128
// @licence ccbysa4
wasmu_UInt wasmu_readUInt(wasmu_Module* module) {
    wasmu_UInt result = 0;
    wasmu_UInt shift = 0;
    wasmu_U8 byte;

    do {
        byte = WASMU_NEXT();
        result |= (byte & 0b01111111) << shift;
        shift += 7;
    } while ((byte & 0b10000000) != 0);

    return result;
}

// Signed integers are encoded as LEB128
// @source reference https://en.wikipedia.org/wiki/LEB128
// @licence ccbysa4
wasmu_Int wasmu_readInt(wasmu_Module* module) {
    wasmu_Int result = 0;
    wasmu_UInt shift = 0;
    wasmu_U8 byte;

    do {
        byte = WASMU_NEXT();
        result |= ((wasmu_Int)byte & 0b01111111) << shift;
        shift += 7;
    } while ((byte & 0b10000000) != 0);

    if (shift < sizeof(result) * 8 && (byte & 0b01000000) != 0) {
        result |= ((wasmu_Int)(~0) << shift);
    }

    return result;
}

wasmu_Float wasmu_readFloat(wasmu_Module* module, wasmu_ValueType type) {
    wasmu_Count size = wasmu_getValueTypeSize(type);
    wasmu_UInt rawValue = 0;

    for (wasmu_Count i = 0; i < size; i++) {
        rawValue |= (wasmu_UInt)WASMU_NEXT() << (i * 8);
    }

    switch (type) {
        case WASMU_VALUE_TYPE_F32:
        default:
            return ((wasmu_FloatConverter) {.asI32 = WASMU_STATIC_CAST(wasmu_I32, rawValue)}).asF32;

        case WASMU_VALUE_TYPE_F64:
            return ((wasmu_FloatConverter) {.asI64 = WASMU_STATIC_CAST(wasmu_I64, rawValue)}).asF64;
    }
}

wasmu_String wasmu_readString(wasmu_Module* module) {
    wasmu_Count size = wasmu_readUInt(module);
    wasmu_U8* chars = (wasmu_U8*)WASMU_MALLOC(size);

    for (unsigned int i = 0; i < size; i++) {
        chars[i] = WASMU_NEXT();
    }

    return (wasmu_String) {
        .size = size,
        .chars = chars
    };
}

wasmu_U8* wasmu_getNullTerminatedChars(wasmu_String string) {
    wasmu_U8* chars = (wasmu_U8*)WASMU_MALLOC(string.size + 1);

    for (wasmu_Count i = 0; i < string.size; i++) {
        chars[i] = string.chars[i];
    }

    chars[string.size] = '\0';

    return chars;
}

wasmu_String wasmu_charsToString(const wasmu_U8* chars) {
    wasmu_U8* copy = wasmu_copyChars(chars);
    wasmu_Count size = 0;

    while (chars[size]) {
        size++;
    }

    return (wasmu_String) {
        .size = size,
        .chars = copy
    };
}

wasmu_Bool wasmu_stringEqualsChars(wasmu_String a, const wasmu_U8* b) {
    wasmu_U8* chars = wasmu_getNullTerminatedChars(a);
    wasmu_Bool result = wasmu_charsEqual(chars, b);

    free(chars);

    return result;
}

wasmu_Count wasmu_getValueTypeSize(wasmu_ValueType type) {
    switch (type) {
        case WASMU_VALUE_TYPE_I32:
        case WASMU_VALUE_TYPE_F32:
            return 4;

        case WASMU_VALUE_TYPE_I64:
        case WASMU_VALUE_TYPE_F64:
            return 8;

        default:
            WASMU_DEBUG_LOG("Unknown value type: 0x%02x", type);
            return 0;
    }
}

wasmu_Int wasmu_getExportIndex(wasmu_Module* module, const wasmu_U8* name, wasmu_ExportType type) {
    for (wasmu_Count i = 0; i < module->exportsCount; i++) {
        wasmu_Export moduleExport = module->exports[i];

        if (moduleExport.type == type && wasmu_stringEqualsChars(moduleExport.name, name)) {
            return moduleExport.index;
        }
    }

    return -1;
}

wasmu_Function* wasmu_getExportedFunction(wasmu_Module* module, const wasmu_U8* name) {
    wasmu_Count functionIndex = wasmu_getExportIndex(module, name, WASMU_EXPORT_TYPE_FUNCTION);

    if (functionIndex == -1) {
        return WASMU_NULL;
    }

    return WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);
}

wasmu_TypedValue* wasmu_getExportedGlobal(wasmu_Module* module, const wasmu_U8* name) {
    wasmu_Count globalIndex = wasmu_getExportIndex(module, name, WASMU_EXPORT_TYPE_GLOBAL);

    if (globalIndex == -1) {
        return WASMU_NULL;
    }

    return WASMU_GET_ENTRY(module->globals, module->globalsCount, globalIndex);
}

wasmu_Bool wasmu_resolveModuleImportData(wasmu_Import* import, wasmu_Module* resolvedModule) {
    wasmu_U8* name = wasmu_getNullTerminatedChars(import->name);
    wasmu_Bool isSuccess = WASMU_FALSE;

    switch (import->type) {
        case WASMU_EXPORT_TYPE_FUNCTION: {
            wasmu_Count functionIndex = wasmu_getExportIndex(resolvedModule, name, WASMU_EXPORT_TYPE_FUNCTION);

            if (functionIndex == -1) {
                WASMU_DEBUG_LOG("Unable to resolve imported function - name: %s", name);
                resolvedModule->context->errorState = WASMU_ERROR_STATE_IMPORT_NOT_FOUND;
                goto exit;
            }

            import->index = functionIndex;

            break;
        }

        default:
            WASMU_DEBUG_LOG("Import type not implemented when resolving");
            resolvedModule->context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
            goto exit;
    }

    isSuccess = WASMU_TRUE;

    exit:

    WASMU_FREE(name);

    return isSuccess;
}

wasmu_Bool wasmu_resolveModuleImports(wasmu_Module* module) {
    wasmu_Context* context = module->context;

    for (wasmu_Count i = 0; i < module->importsCount; i++) {
        wasmu_Import* moduleImport = &module->imports[i];

        if (moduleImport->resolvedModuleIndex != -1) {
            continue;
        }

        wasmu_U8* targetModuleName = wasmu_getNullTerminatedChars(moduleImport->moduleName);

        for (wasmu_Count j = 0; j < context->modulesCount; j++) {
            wasmu_Module* currentModule = context->modules[j];

            if (currentModule->name && wasmu_charsEqual(currentModule->name, targetModuleName)) {
                if (!wasmu_resolveModuleImportData(moduleImport, currentModule)) {
                    return WASMU_FALSE;
                }

                moduleImport->resolvedModuleIndex = j;

                break;
            }
        }

        if (moduleImport->resolvedModuleIndex == -1) {
            WASMU_DEBUG_LOG("Unable to resolve module import - moduleName: %s", targetModuleName);
            context->errorState = WASMU_ERROR_STATE_IMPORT_NOT_FOUND;
            return WASMU_FALSE;
        }

        WASMU_FREE(targetModuleName);
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_addNativeFunction(wasmu_Module* module, const wasmu_U8* name, wasmu_NativeFunction nativeFunction) {
    wasmu_Function function;

    function.importIndex = -1;
    function.nativeFunction = nativeFunction;

    WASMU_INIT_ENTRIES(function.locals, function.localsCount);

    WASMU_ADD_ENTRY(module->functions, module->functionsCount, function);

    wasmu_Export moduleExport;

    moduleExport.name = wasmu_charsToString(name);
    moduleExport.type = WASMU_EXPORT_TYPE_FUNCTION;
    moduleExport.index = module->functionsCount - 1;

    WASMU_ADD_ENTRY(module->exports, module->exportsCount, moduleExport);

    return WASMU_TRUE;
}