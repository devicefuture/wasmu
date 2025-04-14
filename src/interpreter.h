#define WASMU_ASSERT_POP_TYPE(type) do { \
    wasmu_ValueType poppedType = wasmu_popType(context); \
    \
    if (poppedType != type) { \
        WASMU_DEBUG_LOG("Expected type 0x%02x but got 0x%02x", type, poppedType); \
        context->errorState = WASMU_ERROR_STATE_TYPE_MISMATCH; \
        return WASMU_FALSE; \
    } \
} while (0)

#define WASMU_FF_SKIP_HERE() if (context->fastForward) {break;}
#define WASMU_FF_STEP_IN() if (!wasmu_fastForwardStepInLabel(context)) {return WASMU_FALSE;}
#define WASMU_FF_STEP_OUT() if (!wasmu_fastForwardStepOutLabel(context)) {return WASMU_FALSE;}

wasmu_Bool wasmu_pushLabel(wasmu_Context* context, wasmu_Opcode opcode, wasmu_Count resultsCount) {
    wasmu_LabelStack* stack = &context->labelStack;

    if (context->callStack.count == 0) {
        return WASMU_FALSE;
    }

    stack->count++;

    if (stack->count > stack->size) {
        stack->size = stack->count;
        stack->labels = (wasmu_Label*)WASMU_REALLOC(stack->labels, stack->size * sizeof(wasmu_Label));
    }

    stack->labels[stack->count - 1] = (wasmu_Label) {
        .opcode = opcode,
        .callIndex = context->callStack.count - 1,
        .position = context->activeModule->position,
        .resultsCount = resultsCount,
        .typeStackBase = context->typeStack.count,
        .valueStackBase = context->valueStack.position
    };

    return WASMU_TRUE;
}

wasmu_Bool wasmu_popLabel(wasmu_Context* context, wasmu_Count targetCallIndex, wasmu_Label* returnedLabel) {
    wasmu_LabelStack* stack = &context->labelStack;

    if (stack->count == 0) {
        WASMU_DEBUG_LOG("Label stack underflow");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_FALSE;
    }

    wasmu_Label label = stack->labels[stack->count - 1];

    if (targetCallIndex != -1 && label.callIndex != targetCallIndex) {
        WASMU_DEBUG_LOG("Label stack underflow for this function call");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_FALSE;
    }

    if (returnedLabel) {
        *returnedLabel = label;
    }

    stack->count--;

    return WASMU_TRUE;
}

wasmu_Bool wasmu_getLabel(wasmu_Context* context, wasmu_Count labelIndex, wasmu_Count targetCallIndex, wasmu_Label* returnedLabel) {
    // Function does not throw error as callers may use it to check if a label exists

    wasmu_LabelStack* stack = &context->labelStack;

    if (labelIndex >= stack->count) {
        return WASMU_FALSE;
    }

    /*
        Label indexes start from the end of the label stack instead of the
        start, so they must be converted into absolute indexes to reference a
        label on the stack's data structure.
    */

    wasmu_Label label = stack->labels[stack->count - labelIndex - 1];

    if (targetCallIndex != -1 && label.callIndex != targetCallIndex) {
        return WASMU_FALSE;
    }

    if (returnedLabel) {
        *returnedLabel = label;
    }

    return WASMU_TRUE;
}

void wasmu_populateActiveCallInfo(wasmu_Context* context, wasmu_Call call) {
    /*
        This is done to cache pointers to the active module, function and
        function signature, instead of having to check the top of the call stack
        and then resolve indexes to pointers every time we want to step our
        context.
    */

    wasmu_Module** moduleEntry = WASMU_GET_ENTRY(context->modules, context->modulesCount, call.moduleIndex);
    wasmu_Module* module = *moduleEntry;

    context->activeModule = module;
    context->activeModuleIndex = call.moduleIndex;

    if (context->activeModule) {
        context->activeFunction = WASMU_GET_ENTRY(module->functions, module->functionsCount, call.functionIndex);
        context->activeFunctionSignature = WASMU_GET_ENTRY(module->functionSignatures, module->functionSignaturesCount, context->activeFunction->signatureIndex);

        context->currentStackLocals = (wasmu_StackLocal*)WASMU_REALLOC(context->currentStackLocals, 0);
        context->currentStackLocalsCount = 0;

        wasmu_Count currentPosition = call.valueStackBase;

        for (wasmu_Count i = 0; i < context->activeFunctionSignature->parametersCount; i++) {
            wasmu_ValueType type = context->activeFunctionSignature->parameters[i];
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_StackLocal local = {
                .position = currentPosition,
                .type = WASMU_LOCAL_TYPE_PARAMETER,
                .valueType = type,
                .size = size
            };

            WASMU_ADD_ENTRY(context->currentStackLocals, context->currentStackLocalsCount, local);

            currentPosition += size;
        }

        for (wasmu_Count i = 0; i < context->activeFunctionSignature->resultsCount; i++) {
            wasmu_ValueType type = context->activeFunctionSignature->results[i];
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_StackLocal local = {
                .position = currentPosition,
                .type = WASMU_LOCAL_TYPE_RESULT,
                .valueType = type,
                .size = size
            };

            WASMU_ADD_ENTRY(context->currentStackLocals, context->currentStackLocalsCount, local);

            currentPosition += size;
        }

        for (wasmu_Count i = 0; i < context->activeFunction->localsCount; i++) {
            wasmu_ValueType type = context->activeFunction->locals[i];
            wasmu_Count size = wasmu_getValueTypeSize(type);

            wasmu_StackLocal local = {
                .position = currentPosition,
                .type = WASMU_LOCAL_TYPE_LOCAL,
                .valueType = type,
                .size = size
            };

            WASMU_ADD_ENTRY(context->currentStackLocals, context->currentStackLocalsCount, local);

            currentPosition += size;
        }
    }

    context->currentTypeStackBase = call.typeStackBase;
    context->currentValueStackBase = call.valueStackBase;
}

void wasmu_pushCall(wasmu_Context* context, wasmu_Call call) {
    wasmu_CallStack* stack = &context->callStack;

    // TODO: Allow call stack to have a maximum size defined to catch infinite recursion
    stack->count++;

    if (stack->count > stack->size) {
        stack->size = stack->count;
        stack->calls = (wasmu_Call*)WASMU_REALLOC(stack->calls, stack->size * sizeof(wasmu_Call));
    }

    stack->calls[stack->count - 1] = call;

    wasmu_populateActiveCallInfo(context, call);
}

wasmu_Bool wasmu_popCall(wasmu_Context* context, wasmu_Call* returnedCall) {
    wasmu_CallStack* stack = &context->callStack;

    if (stack->count == 0) {
        WASMU_DEBUG_LOG("Call stack underflow");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_FALSE;
    }

    if (returnedCall) {
        *returnedCall = stack->calls[stack->count - 1];
    }

    stack->count--;

    while (context->labelStack.count > 0 && context->labelStack.labels[context->labelStack.count - 1].callIndex >= stack->count) {
        // Remove labels from label stack that reference a call that no longer exists
        wasmu_popLabel(context, -1, WASMU_NULL);
    }

    if (stack->count > 0) {
        wasmu_populateActiveCallInfo(context, stack->calls[stack->count - 1]);
    } else {
        context->activeModule = WASMU_NULL;
    }

    return WASMU_TRUE;
}

void wasmu_pushType(wasmu_Context* context, wasmu_ValueType type) {
    wasmu_TypeStack* stack = &context->typeStack;

    stack->count++;

    if (stack->count > stack->size) {
        stack->size = stack->count;
        stack->types = (wasmu_ValueType*)WASMU_REALLOC(stack->types, stack->size * sizeof(wasmu_ValueType));
    }

    stack->types[stack->count - 1] = type;
}

wasmu_ValueType wasmu_popType(wasmu_Context* context) {
    wasmu_TypeStack* stack = &context->typeStack;

    if (stack->count == 0) {
        WASMU_DEBUG_LOG("Type stack underflow");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return WASMU_VALUE_TYPE_I32;
    }

    wasmu_ValueType type = stack->types[stack->count - 1];

    stack->count--;

    return type;
}

void wasmu_growValueStack(wasmu_ValueStack* stack, wasmu_Count newPosition) {
    if (newPosition > stack->size) {
        stack->size = newPosition;
        stack->data = (wasmu_U8*)WASMU_REALLOC(stack->data, stack->size);
    }
}

wasmu_Int wasmu_stackGetInt(wasmu_Context* context, wasmu_Count position, wasmu_Count bytes) {
    wasmu_Int value = 0;
    wasmu_ValueStack* stack = &context->valueStack;

    for (wasmu_Count i = 0; i < bytes; i++) {
        value |= stack->data[position + i] << (i * 8);
    }

    return value;
}

void wasmu_stackSetInt(wasmu_Context* context, wasmu_Count position, wasmu_Count bytes, wasmu_Int value) {
    wasmu_ValueStack* stack = &context->valueStack;

    wasmu_growValueStack(stack, position + bytes);

    for (wasmu_Count i = 0; i < bytes; i++) {
        stack->data[position + i] = value & 0xFF;
        value >>= 8;
    }
}

void wasmu_pushInt(wasmu_Context* context, wasmu_Count bytes, wasmu_Int value) {
    wasmu_ValueStack* stack = &context->valueStack;

    wasmu_stackSetInt(context, stack->position, bytes, value);

    stack->position += bytes;
}

wasmu_Int wasmu_popInt(wasmu_Context* context, wasmu_Count bytes) {
    wasmu_ValueStack* stack = &context->valueStack;

    if (stack->position < bytes) {
        WASMU_DEBUG_LOG("Value stack underflow");
        context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
        return 0;
    }

    stack->position -= bytes;

    return wasmu_stackGetInt(context, stack->position, bytes);
}

wasmu_Bool wasmu_callFunctionByIndex(wasmu_Context* context, wasmu_Count moduleIndex, wasmu_Count functionIndex) {
    wasmu_Module** modulePtr = WASMU_GET_ENTRY(context->modules, context->modulesCount, moduleIndex);
    wasmu_Module* module = *modulePtr;

    if (!module) {
        return WASMU_FALSE;
    }

    wasmu_Function* function = WASMU_GET_ENTRY(module->functions, module->functionsCount, functionIndex);

    if (!function) {
        WASMU_DEBUG_LOG("Unknown function");
        context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
        return WASMU_FALSE;
    }

    if (context->callStack.count > 0) {
        // Save current position on current topmost call
        context->callStack.calls[context->callStack.count - 1].position = module->position;
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
        return WASMU_FALSE;
    }

    for (wasmu_Count i = 0; i < module->functionsCount; i++) {
        if (&(module->functions[i]) == function) {
            functionIndex = i;
            break;
        }
    }

    if (functionIndex == -1) {
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

    if (resultsOffset > 0) {
        for (wasmu_Count i = 0; i < resultsOffset; i++) {
            stack->data[base + i] = stack->data[base + resultsOffset + i];
        }

        stack->position -= resultsOffset;
    }

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

wasmu_Bool wasmu_fastForward(wasmu_Context* context, wasmu_Opcode targetOpcode, wasmu_Count* positionResult) {
    WASMU_DEBUG_LOG("Begin fast forward - targetOpcode: 0x%02x", targetOpcode);

    context->fastForward = WASMU_TRUE;
    context->fastForwardTargetOpcode = targetOpcode;
    context->fastForwardLabelDepth = 0;

    while (context->fastForward) {
        if (!wasmu_step(context)) {
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
        WASMU_DEBUG_LOG("No more opcodes to execute");
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
        {
            wasmu_U8 blockType = WASMU_READ(module->position);
            wasmu_Count resultsCount = 0;

            if (blockType == 0x40) {
                // Shorthand for void result type
                WASMU_NEXT();
            } else if (blockType >= 0x7C && blockType <= 0x7F) {
                // Shorthand for single-value result types
                resultsCount = 1;
                WASMU_NEXT();
            } else {
                wasmu_Count signatureIndex = wasmu_readUInt(module);
                wasmu_FunctionSignature* signature = WASMU_GET_ENTRY(module->functionSignatures, module->functionSignaturesCount, signatureIndex);

                if (!signature) {
                    WASMU_DEBUG_LOG("Unknown function signature");
                    context->errorState = WASMU_ERROR_STATE_INVALID_INDEX;
                    return WASMU_FALSE;
                }

                resultsCount += signature->resultsCount;
            }

            WASMU_FF_STEP_IN();
            WASMU_FF_SKIP_HERE();

            WASMU_DEBUG_LOG("Block/loop");

            wasmu_pushLabel(context, (wasmu_Opcode)opcode, resultsCount);

            break;
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
        {
            wasmu_Count labelIndex = wasmu_readUInt(module);

            WASMU_FF_SKIP_HERE();

            wasmu_Label label;

            WASMU_DEBUG_LOG("Break/break if - labelIndex: %d", labelIndex);

            if (!wasmu_getLabel(context, labelIndex, context->callStack.count - 1, &label)) {
                WASMU_DEBUG_LOG("Label stack underflow");
                context->errorState = WASMU_ERROR_STATE_STACK_UNDERFLOW;
                return WASMU_FALSE;
            }

            if (opcode == WASMU_OP_BR_IF) {
                wasmu_Int condition = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

                WASMU_DEBUG_LOG("Break if - condition: %d", condition);

                if (!condition) {
                    break;
                }
            }

            WASMU_DEBUG_LOG("Checking label from position 0x%08x", label.position);

            module->position = label.position;

            // TODO: Use results count to clean type and value stacks for removal of non-result values

            if (label.opcode == WASMU_OP_LOOP) {
                // No need to fast forward since we're looping back to start
                break;
            }

            if (!wasmu_fastForward(context, WASMU_OP_END, WASMU_NULL)) {
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

        case WASMU_OP_I32_CONST:
        {
            wasmu_I32 value = wasmu_readInt(module);

            WASMU_FF_SKIP_HERE();

            WASMU_DEBUG_LOG("I32 const - value: %d", value);

            wasmu_pushInt(context, 4, value);
            wasmu_pushType(context, WASMU_VALUE_TYPE_I32);

            break;
        }

        case WASMU_OP_I32_ADD:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_I32 b = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
            wasmu_I32 a = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

            WASMU_DEBUG_LOG("Add I32 - a: %d, b: %d (result: %d)", a, b, a + b);

            wasmu_pushInt(context, 4, a + b);

            break;
        }

        case WASMU_OP_I32_SUB:
        {
            WASMU_FF_SKIP_HERE();

            wasmu_I32 b = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);
            wasmu_I32 a = wasmu_popInt(context, 4); WASMU_ASSERT_POP_TYPE(WASMU_VALUE_TYPE_I32);

            WASMU_DEBUG_LOG("Sub I32 - a: %d, b: %d (result: %d)", a, b, a - b);

            wasmu_pushInt(context, 4, a - b);

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