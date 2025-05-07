#define WASMU_FF_SKIP_HERE() if (context->fastForward) {break;}
#define WASMU_FF_STEP_IN() if (!wasmu_fastForwardStepInLabel(context)) {return WASMU_FALSE;}
#define WASMU_FF_STEP_OUT() if (!wasmu_fastForwardStepOutLabel(context)) {return WASMU_FALSE;}

#define WASMU_OPERATOR(baseType, operator) { \
        WASMU_FF_SKIP_HERE(); \
        \
        wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode); \
        wasmu_Count size = wasmu_getValueTypeSize(type); \
        \
        baseType b = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type); \
        baseType a = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type); \
        \
        WASMU_DEBUG_LOG("Operator " #operator " - a: %ld, b: %ld (result: %ld)", a, b, a operator b); \
        \
        wasmu_pushInt(context, size, a operator b); \
        wasmu_pushType(context, type); \
        \
        break; \
    }

wasmu_Bool wasmu_fastForward(wasmu_Context* context, wasmu_Opcode targetOpcode, wasmu_Count* positionResult, wasmu_Bool errorOnSearchFail) {
    WASMU_DEBUG_LOG("Begin fast forward - targetOpcode: 0x%02x", targetOpcode);

    context->fastForward = WASMU_TRUE;
    context->fastForwardTargetOpcode = targetOpcode;
    context->fastForwardLabelDepth = 0;

    while (context->fastForward) {
        if (!wasmu_step(context)) {
            if (!errorOnSearchFail && context->errorState == WASMU_ERROR_STATE_STACK_UNDERFLOW) {
                // Opcode not found, so ignore error
                context->errorState = WASMU_ERROR_STATE_NONE;
            }

            context->fastForward = WASMU_FALSE;

            return WASMU_FALSE;
        }
    }

    WASMU_DEBUG_LOG("End fast forward - position: 0x%08x", context->activeModule->position);

    if (positionResult) {
        *positionResult = context->activeModule->position;
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_fastForwardStepInLabel(wasmu_Context* context) {
    if (context->fastForward) {
        context->fastForwardLabelDepth++;
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_fastForwardStepOutLabel(wasmu_Context* context) {
    if (context->fastForward) {
        if (context->fastForwardLabelDepth == 0) {
            WASMU_DEBUG_LOG("Label depth counter underflow while fast forwarding");
            context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
            return WASMU_FALSE;
        }

        context->fastForwardLabelDepth--;
    }

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

    if (context->errorState == WASMU_ERROR_STATE_MEMORY_OOB) {
        WASMU_DEBUG_LOG("No more opcodes to execute - position: 0x%08x", module->position);
        return WASMU_FALSE;
    }

    if (context->fastForward) {
        if (context->fastForwardLabelDepth == 0 && opcode == context->fastForwardTargetOpcode) {
            WASMU_DEBUG_LOG("Match opcode - opcode: 0x%02x, position: 0x%08x", opcode, module->position);

            module->position--;
            context->fastForward = WASMU_FALSE;

            return WASMU_TRUE;
        }

        WASMU_DEBUG_LOG("Skip over instruction - opcode: 0x%02x, position: 0x%08x", opcode, module->position);
    } else {
        WASMU_DEBUG_LOG("Execute instruction - opcode: 0x%02x, position: 0x%08x", opcode, module->position);
    }

    switch (opcode) {
        case WASMU_OP_UNREACHABLE:
        {
            WASMU_FF_SKIP_HERE();

            WASMU_DEBUG_LOG("Reached unreachable operation");
            context->errorState = WASMU_ERROR_STATE_REACHED_UNREACHABLE;
            return WASMU_FALSE;
        }

        case WASMU_OP_NOP:
            break;

        case WASMU_OP_BLOCK:
        case WASMU_OP_LOOP:
        case WASMU_OP_IF:
        {
            wasmu_U8 blockType = WASMU_READ(module->position);
            wasmu_Count resultsCount = 0;
            wasmu_Count resultsSize = 0;

            if (blockType == 0x40) {
                // Shorthand for void result type
                WASMU_NEXT();
            } else if (blockType >= 0x7C && blockType <= 0x7F) {
                // Shorthand for single-value result types
                resultsCount = 1;
                resultsSize = wasmu_getValueTypeSize((wasmu_ValueType)blockType);
                WASMU_NEXT();
            } else {
                wasmu_Count signatureIndex = wasmu_readUInt(module);
                wasmu_FunctionSignature* signature = WASMU_GET_ENTRY(module->functionSignatures, module->functionSignaturesCount, signatureIndex);

                if (!signature) {
                    WASMU_DEBUG_LOG("Unknown function signature");
                    context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                    return WASMU_FALSE;
                }

                resultsCount = signature->resultsCount;
                resultsSize = signature->resultsStackSize;
            }

            WASMU_FF_STEP_IN();
            WASMU_FF_SKIP_HERE();

            WASMU_DEBUG_LOG("Block/loop/if - resultsCount: %d, resultsSize: %d", resultsCount, resultsSize);

            wasmu_pushLabel(context, (wasmu_Opcode)opcode, resultsCount, resultsSize);

            if (opcode == WASMU_OP_IF) {
                wasmu_Int condition = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

                WASMU_DEBUG_LOG("If - condition: %d", condition);

                if (!condition) {
                    wasmu_Count originalPosition = module->position;

                    if (wasmu_fastForward(context, WASMU_OP_ELSE, WASMU_NULL, WASMU_FALSE)) {
                        // Jump to code after `else` but don't actually execute `else` opcode
                        WASMU_DEBUG_LOG("Jump to else");
                        WASMU_NEXT();

                        break;
                    }

                    WASMU_DEBUG_LOG("Jump to end");

                    module->position = originalPosition;

                    if (!wasmu_fastForward(context, WASMU_OP_END, WASMU_NULL, WASMU_TRUE)) {
                        return WASMU_FALSE;
                    }
                }
            }

            break;
        }

        case WASMU_OP_ELSE:
        {
            WASMU_DEBUG_LOG("Else");

            if (!wasmu_fastForward(context, WASMU_OP_END, WASMU_NULL, WASMU_TRUE)) {
                return WASMU_FALSE;
            }
        }

        case WASMU_OP_END:
        {
            WASMU_FF_STEP_OUT();

            wasmu_Label label;

            if (wasmu_getLabel(context, 0, context->callStack.count - 1, &label)) {
                WASMU_DEBUG_LOG("End - leave structured instruction");

                wasmu_popLabel(context, -1, WASMU_NULL);
            } else {
                WASMU_DEBUG_LOG("End - return from function");

                wasmu_returnFromFunction(context);
            }

            break;
        }

        case WASMU_OP_BR:
        case WASMU_OP_BR_IF:
        case WASMU_OP_BR_TABLE:
        {
            // Get the current label and decide whether to branch

            wasmu_Count labelIndex;

            if (opcode == WASMU_OP_BR_TABLE) {
                wasmu_Count targets = wasmu_readUInt(module);
                wasmu_Count targetIndex = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
                wasmu_Bool foundTarget = WASMU_FALSE;

                for (wasmu_Count i = 0; i < targets; i++) {
                    wasmu_Count targetLabelIndex = wasmu_readUInt(module);

                    if (i == targetIndex) {
                        foundTarget = WASMU_TRUE;
                        labelIndex = targetLabelIndex;
                    }
                }

                wasmu_Count defaultLabelIndex = wasmu_readUInt(module);

                if (!foundTarget) {
                    labelIndex = defaultLabelIndex;
                }
            } else {
                labelIndex = wasmu_readUInt(module);
            }

            WASMU_FF_SKIP_HERE();

            wasmu_Label label;

            WASMU_DEBUG_LOG("Branch/branch if/branch table - labelIndex: %d", labelIndex);

            if (!wasmu_getLabel(context, labelIndex, context->callStack.count - 1, &label)) {
                WASMU_DEBUG_LOG("Label stack underflow");
                context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
                return WASMU_FALSE;
            }

            if (opcode == WASMU_OP_BR_IF) {
                wasmu_Int condition = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

                WASMU_DEBUG_LOG("Branch if - condition: %d", condition);

                if (!condition) {
                    break;
                }
            }

            // Clean the stack to remove non-result values from the type and value stacks

            wasmu_Int nonResultsCount = context->typeStack.count - label.typeStackBase - label.resultsCount;
            wasmu_Int nonResultsSize = context->valueStack.position - label.valueStackBase - label.resultsSize;

            WASMU_DEBUG_LOG("Clean up stack - nonResultsCount: %d, nonResultsSize: %d", nonResultsCount, nonResultsSize);

            if (nonResultsCount < 0 || nonResultsSize < 0) {
                WASMU_DEBUG_LOG(nonResultsCount < 0 ? "Type stack overflow" : "Value stack underflow");
                context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
                return WASMU_FALSE;
            }

            // Clean the type stack first by shifting results up

            for (wasmu_Count i = 0; i < nonResultsCount; i++) {
                context->typeStack.types[label.typeStackBase + i] = context->typeStack.types[label.typeStackBase + nonResultsCount + i];
            }

            context->typeStack.count = label.typeStackBase + label.resultsCount;

            // Do the same for the value stack

            for (wasmu_Count i = 0; i < nonResultsSize; i++) {
                context->valueStack.data[label.valueStackBase + i] = context->valueStack.data[label.valueStackBase + nonResultsSize + i];
            }

            context->valueStack.position = label.valueStackBase + label.resultsSize;

            // Finally, jump to the position specified in the label

            WASMU_DEBUG_LOG("Checking label from position 0x%08x", label.position);

            module->position = label.position;

            if (label.opcode == WASMU_OP_LOOP) {
                // No need to fast forward since we're looping back to start
                break;
            }

            if (!wasmu_fastForward(context, WASMU_OP_END, WASMU_NULL, WASMU_TRUE)) {
                return WASMU_FALSE;
            }

            break;
        }

        case WASMU_OP_RETURN:
        {
            WASMU_FF_SKIP_HERE();

            WASMU_DEBUG_LOG("Return");
            wasmu_returnFromFunction(context);
            break;
        }

        case WASMU_OP_CALL:
        {
            wasmu_Count functionIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            WASMU_DEBUG_LOG("Call function - index: %d", functionIndex);

            if (!wasmu_callFunctionByIndex(context, context->activeModuleIndex, functionIndex)) {
                WASMU_DEBUG_LOG("Unknown function");
                context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            break;
        }

        case WASMU_OP_CALL_INDIRECT:
        {
            wasmu_Count typeIndex = wasmu_readUInt(module);
            wasmu_Count tableIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_Count elementIndex = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

            wasmu_Table* table = WASMU_GET_ENTRY(module->tables, module->tablesCount, tableIndex);

            WASMU_DEBUG_LOG(
                "Call indirect - typeIndex: %d, tableIndex: %d, elementIndex: %d",
                typeIndex, tableIndex, elementIndex
            );

            if (!table) {
                WASMU_DEBUG_LOG("Unknown table");
                module->context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            wasmu_Count* functionIndex = WASMU_GET_ENTRY(table->entries, table->entriesCount, elementIndex);

            if (!functionIndex) {
                WASMU_DEBUG_LOG("Table element index is out of bounds");
                module->context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            WASMU_DEBUG_LOG("Resolve to function index - functionIndex: %d", *functionIndex);

            if (!wasmu_callFunctionByIndex(context, context->activeModuleIndex, *functionIndex)) {
                WASMU_DEBUG_LOG("Unknown function referred to by table element");
                context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }
        }

        case WASMU_OP_DROP:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_popType(context);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            WASMU_DEBUG_LOG("Drop - type: 0x%02x (size: %d)", type, size);

            if (context->valueStack.position < size) {
                WASMU_DEBUG_LOG("Value stack underflow");
                context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
                return WASMU_FALSE;
            }

            context->valueStack.position -= size;

            break;
        }

        case WASMU_OP_SELECT:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_Int condition = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
            wasmu_Count ifFalseSize = wasmu_getValueTypeSize(wasmu_popType(context));
            wasmu_Int ifFalse = wasmu_popInt(context, ifFalseSize);
            wasmu_Count ifTrueSize = wasmu_getValueTypeSize(wasmu_popType(context));
            wasmu_Int ifTrue = wasmu_popInt(context, ifTrueSize);

            WASMU_DEBUG_LOG("Select - ifTrue: %d, ifFalse: %d, condition: %d (ifTrueSize: %d, ifFalseSize: %d)", ifTrue, ifFalse, condition, ifTrueSize, ifFalseSize);

            if (condition) {
                wasmu_pushInt(context, ifTrueSize, ifTrue);
            } else {
                wasmu_pushInt(context, ifFalseSize, ifFalse);
            }

            break;
        }

        case WASMU_OP_LOCAL_GET:
        {
            wasmu_Count localIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_StackLocal* local = WASMU_GET_ENTRY(context->currentStackLocals, context->currentStackLocalsCount, localIndex);
            wasmu_Int value = wasmu_stackGetInt(context, local->position, local->size);

            WASMU_DEBUG_LOG("Get local - index: %d (position: 0x%08x, size: %d, value: %d)", localIndex, local->position, local->size, value);

            wasmu_pushInt(context, local->size, value);
            wasmu_pushType(context, local->valueType);

            break;
        }

        case WASMU_OP_LOCAL_SET:
        case WASMU_OP_LOCAL_TEE:
        {
            wasmu_Count localIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_StackLocal* local = WASMU_GET_ENTRY(context->currentStackLocals, context->currentStackLocalsCount, localIndex);
            wasmu_Int value = wasmu_popInt(context, local->size); WASMU_ASSERT_POP_TYPE(local->valueType);

            WASMU_DEBUG_LOG("Set local - index: %d (position: 0x%08x, size: %d, value: %d)", localIndex, local->position, local->size, value);

            wasmu_stackSetInt(context, local->position, local->size, value);

            if (opcode == WASMU_OP_LOCAL_TEE) {
                wasmu_pushInt(context, local->size, value);
                wasmu_pushType(context, local->valueType);
            }

            break;
        }

        case WASMU_OP_GLOBAL_GET:
        {
            wasmu_Count globalIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_TypedValue* global = WASMU_GET_ENTRY(module->globals, module->globalsCount, globalIndex);
            wasmu_Count size = wasmu_getValueTypeSize(global->type);
            wasmu_Int value = global->value.asInt;

            WASMU_DEBUG_LOG("Get global - index: %d (type: %d, size: %d, value: %d)", globalIndex, global->type, size, value);

            wasmu_pushInt(context, size, value);
            wasmu_pushType(context, global->type);

            break;
        }

        case WASMU_OP_GLOBAL_SET:
        {
            wasmu_Count globalIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_TypedValue* global = WASMU_GET_ENTRY(module->globals, module->globalsCount, globalIndex);
            wasmu_Count size = wasmu_getValueTypeSize(global->type);
            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(global->type);

            WASMU_DEBUG_LOG("Set global - index: %d (type: %d, size: %d, value: %d)", globalIndex, global->type, size, value);

            global->value.asInt = value;

            break;
        }

        case WASMU_OP_I32_LOAD:
        case WASMU_OP_I64_LOAD:
        case WASMU_OP_F32_LOAD:
        case WASMU_OP_F64_LOAD:
        case WASMU_OP_I32_LOAD8_S:
        case WASMU_OP_I32_LOAD8_U:
        case WASMU_OP_I64_LOAD8_S:
        case WASMU_OP_I64_LOAD8_U:
        case WASMU_OP_I32_LOAD16_S:
        case WASMU_OP_I32_LOAD16_U:
        case WASMU_OP_I64_LOAD16_S:
        case WASMU_OP_I64_LOAD16_U:
        case WASMU_OP_I64_LOAD32_S:
        case WASMU_OP_I64_LOAD32_U:
        {
            wasmu_Count alignment = wasmu_readUInt(module);
            wasmu_Count offset = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);
            wasmu_Count dataSize = wasmu_getDataSizeFromOpcode(opcode);
            wasmu_Memory* memory = WASMU_GET_ENTRY(module->memories, module->memoriesCount, 0);

            if (!memory) {
                WASMU_DEBUG_LOG("No memory is defined");
                context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            wasmu_Count index = offset + wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
            wasmu_UInt value = 0;

            WASMU_DEBUG_LOG(
                "Load - alignment: %d, offset: 0x%08x (index: 0x%08x, dataSize: %d)",
                alignment, offset, index, dataSize
            );

            if (!wasmu_memoryLoad(memory, index, dataSize, &value)) {
                WASMU_DEBUG_LOG("Unable to load from memory");
                context->errorState = WASMU_ERROR_STATE_MEMORY_OOB;
                return WASMU_FALSE;
            }

            wasmu_signExtendValue(opcode, &value);

            WASMU_DEBUG_LOG("Loaded value: %ld", value);

            wasmu_pushInt(context, size, value);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_STORE:
        case WASMU_OP_I64_STORE:
        case WASMU_OP_F32_STORE:
        case WASMU_OP_F64_STORE:
        case WASMU_OP_I32_STORE8:
        case WASMU_OP_I64_STORE8:
        case WASMU_OP_I32_STORE16:
        case WASMU_OP_I64_STORE16:
        case WASMU_OP_I64_STORE32:
        {
            wasmu_Count alignment = wasmu_readUInt(module);
            wasmu_Count offset = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);
            wasmu_Count dataSize = wasmu_getDataSizeFromOpcode(opcode);
            wasmu_Memory* memory = WASMU_GET_ENTRY(module->memories, module->memoriesCount, 0);

            if (!memory) {
                WASMU_DEBUG_LOG("No memory is defined");
                context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            wasmu_UInt value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Count index = offset + wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

            WASMU_DEBUG_LOG(
                "Store - alignment: %d, offset: 0x%08x, value: %ld (index: 0x%08x, dataSize: %d)",
                alignment, offset, value, index, dataSize
            );

            if (!wasmu_memoryStore(memory, index, dataSize, value)) {
                WASMU_DEBUG_LOG("Unable to store to memory");
                context->errorState = WASMU_ERROR_STATE_MEMORY_OOB;
                return WASMU_FALSE;
            }

            break;
        }

        case WASMU_OP_MEMORY_SIZE:
        {
            wasmu_Count memoryIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_Memory* memory = WASMU_GET_ENTRY(module->memories, module->memoriesCount, memoryIndex);

            if (!memory) {
                WASMU_DEBUG_LOG("Unknown memory");
                context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            WASMU_DEBUG_LOG("Get memory size - (pages: %d)", memory->pagesCount);

            wasmu_pushInt(context, 4, memory->pagesCount); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

            break;
        }

        case WASMU_OP_MEMORY_GROW:
        {
            wasmu_Count memoryIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_Memory* memory = WASMU_GET_ENTRY(module->memories, module->memoriesCount, memoryIndex);

            if (!memory) {
                WASMU_DEBUG_LOG("Unknown memory");
                context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                return WASMU_FALSE;
            }

            wasmu_Count pagesToAdd = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
            wasmu_Count newPagesCount = memory->pagesCount + pagesToAdd;

            WASMU_DEBUG_LOG("Grow memory - pagesToAdd: %d, newPagesCount", pagesToAdd, newPagesCount);

            if (newPagesCount > memory->maxPages) {
                wasmu_pushInt(context, 4, -1); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);
                break;
            }

            wasmu_pushInt(context, 4, memory->pagesCount); wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

            memory->pagesCount = newPagesCount;

            break;
        }

        case WASMU_OP_I32_CONST:
        case WASMU_OP_I64_CONST:
        case WASMU_OP_F32_CONST:
        case WASMU_OP_F64_CONST:
        {
            wasmu_Int value = wasmu_readInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            WASMU_DEBUG_LOG("Const - value: %ld", value);

            wasmu_pushInt(context, size, value);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_EQZ:
        case WASMU_OP_I64_EQZ:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);

            WASMU_DEBUG_LOG("Equal to zero - value: %ld (result: %ld)", value, value == 0);

            wasmu_pushInt(context, size, value == 0);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_EQ:
        case WASMU_OP_I64_EQ:
            WASMU_OPERATOR(wasmu_Int, ==)

        case WASMU_OP_F32_EQ:
        case WASMU_OP_F64_EQ:
            WASMU_OPERATOR(wasmu_Float, ==)

        case WASMU_OP_I32_NE:
        case WASMU_OP_I64_NE:
            WASMU_OPERATOR(wasmu_Int, !=)

        case WASMU_OP_F32_NE:
        case WASMU_OP_F64_NE:
            WASMU_OPERATOR(wasmu_Float, !=)

        case WASMU_OP_I32_LT_S:
        case WASMU_OP_I64_LT_S:
            WASMU_OPERATOR(wasmu_Int, <)

        case WASMU_OP_I32_LT_U:
        case WASMU_OP_I64_LT_U:
            WASMU_OPERATOR(wasmu_UInt, <)

        case WASMU_OP_F32_LT:
        case WASMU_OP_F64_LT:
            WASMU_OPERATOR(wasmu_Float, <)

        case WASMU_OP_I32_GT_S:
        case WASMU_OP_I64_GT_S:
            WASMU_OPERATOR(wasmu_Int, >)

        case WASMU_OP_I32_GT_U:
        case WASMU_OP_I64_GT_U:
            WASMU_OPERATOR(wasmu_UInt, >)

        case WASMU_OP_F32_GT:
        case WASMU_OP_F64_GT:
            WASMU_OPERATOR(wasmu_Float, >)

        case WASMU_OP_I32_LE_S:
        case WASMU_OP_I64_LE_S:
            WASMU_OPERATOR(wasmu_Int, <=)

        case WASMU_OP_I32_LE_U:
        case WASMU_OP_I64_LE_U:
            WASMU_OPERATOR(wasmu_UInt, <=)

        case WASMU_OP_F32_LE:
        case WASMU_OP_F64_LE:
            WASMU_OPERATOR(wasmu_Float, <=)

        case WASMU_OP_I32_GE_S:
        case WASMU_OP_I64_GE_S:
            WASMU_OPERATOR(wasmu_Int, >=)

        case WASMU_OP_I32_GE_U:
        case WASMU_OP_I64_GE_U:
            WASMU_OPERATOR(wasmu_UInt, >=)

        case WASMU_OP_F32_GE:
        case WASMU_OP_F64_GE:
            WASMU_OPERATOR(wasmu_Float, >=)

        case WASMU_OP_I32_CLZ:
        case WASMU_OP_I64_CLZ:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Count result = wasmu_countLeadingZeros(value, size);

            WASMU_DEBUG_LOG("Count leading zeros - value: %ld (result: %ld)", value, result);

            wasmu_pushInt(context, size, result);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_CTZ:
        case WASMU_OP_I64_CTZ:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Count result = wasmu_countTrailingZeros(value, size);

            WASMU_DEBUG_LOG("Count trailing zeros - value: %ld (result: %ld)", value, result);

            wasmu_pushInt(context, size, result);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_POPCNT:
        case WASMU_OP_I64_POPCNT:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Count result = wasmu_countOnes(value, size);

            WASMU_DEBUG_LOG("Count ones - value: %ld (result: %ld)", value, result);

            wasmu_pushInt(context, size, result);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_ADD:
        case WASMU_OP_I64_ADD:
            WASMU_OPERATOR(wasmu_Int, +)

        case WASMU_OP_F32_ADD:
        case WASMU_OP_F64_ADD:
            WASMU_OPERATOR(wasmu_Float, +)

        case WASMU_OP_I32_SUB:
        case WASMU_OP_I64_SUB:
            WASMU_OPERATOR(wasmu_Int, -)

        case WASMU_OP_F32_SUB:
        case WASMU_OP_F64_SUB:
            WASMU_OPERATOR(wasmu_Float, -)

        case WASMU_OP_I32_MUL:
        case WASMU_OP_I64_MUL:
            WASMU_OPERATOR(wasmu_Int, *)

        case WASMU_OP_F32_MUL:
        case WASMU_OP_F64_MUL:
            WASMU_OPERATOR(wasmu_Float, *)

        case WASMU_OP_I32_DIV_S:
        case WASMU_OP_I64_DIV_S:
            WASMU_OPERATOR(wasmu_Int, /)

        case WASMU_OP_I32_DIV_U:
        case WASMU_OP_I64_DIV_U:
            WASMU_OPERATOR(wasmu_UInt, /)

        case WASMU_OP_F32_DIV:
        case WASMU_OP_F64_DIV:
            WASMU_OPERATOR(wasmu_Float, /)

        case WASMU_OP_I32_REM_S:
        case WASMU_OP_I64_REM_S:
            WASMU_OPERATOR(wasmu_Int, %)

        case WASMU_OP_I32_REM_U:
        case WASMU_OP_I64_REM_U:
            WASMU_OPERATOR(wasmu_UInt, %)

        case WASMU_OP_I32_AND:
        case WASMU_OP_I64_AND:
            WASMU_OPERATOR(wasmu_Int, &)

        case WASMU_OP_I32_OR:
        case WASMU_OP_I64_OR:
            WASMU_OPERATOR(wasmu_Int, |)

        case WASMU_OP_I32_XOR:
        case WASMU_OP_I64_XOR:
            WASMU_OPERATOR(wasmu_Int, ^)

        case WASMU_OP_I32_SHL:
        case WASMU_OP_I64_SHL:
            WASMU_OPERATOR(wasmu_Int, <<)

        case WASMU_OP_I32_SHR_S:
        case WASMU_OP_I64_SHR_S:
            WASMU_OPERATOR(wasmu_Int, >>)

        case WASMU_OP_I32_SHR_U:
        case WASMU_OP_I64_SHR_U:
            WASMU_OPERATOR(wasmu_UInt, >>)

        case WASMU_OP_I32_ROTL:
        case WASMU_OP_I64_ROTL:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_Int shift = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Count result = wasmu_rotateLeft(value, size, shift);

            WASMU_DEBUG_LOG("Rotate left - value: %ld, shift: %ld (result: %ld)", value, shift, result);

            wasmu_pushInt(context, size, result);
            wasmu_pushType(context, type);

            break;
        }

        case WASMU_OP_I32_ROTR:
        case WASMU_OP_I64_ROTR:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_ValueType type = wasmu_getOpcodeSubjectType(opcode);
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_Int shift = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Int value = wasmu_popInt(context, size); WASMU_ASSERT_POP_TYPE(type);
            wasmu_Count result = wasmu_rotateRight(value, size, shift);

            WASMU_DEBUG_LOG("Rotate right - value: %ld, shift: %ld (result: %ld)", value, shift, result);

            wasmu_pushInt(context, size, result);
            wasmu_pushType(context, type);

            break;
        }

        default:
            WASMU_DEBUG_LOG("Opcode not implemented");
            context->errorState = WASMU_ERROR_STATE_NOT_IMPLEMENTED;
            return WASMU_FALSE;
    }

    return WASMU_TRUE;
}

wasmu_Bool wasmu_runFunction(wasmu_Module* module, wasmu_Function* function) {
    WASMU_DEBUG_LOG("Run function");

    wasmu_Bool called = wasmu_callFunction(module, function);

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