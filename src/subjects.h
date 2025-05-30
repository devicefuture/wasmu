wasmu_ValueType wasmu_getOpcodeSubjectType(wasmu_Opcode opcode) {
    switch (opcode) {
        case WASMU_OP_I32_LOAD:
        case WASMU_OP_I32_LOAD8_S:
        case WASMU_OP_I32_LOAD8_U:
        case WASMU_OP_I32_LOAD16_S:
        case WASMU_OP_I32_LOAD16_U:
        case WASMU_OP_I32_STORE:
        case WASMU_OP_I32_STORE8:
        case WASMU_OP_I32_STORE16:
        case WASMU_OP_I32_CONST:
        case WASMU_OP_I32_EQZ:
        case WASMU_OP_I32_EQ:
        case WASMU_OP_I32_NE:
        case WASMU_OP_I32_LT_S:
        case WASMU_OP_I32_LT_U:
        case WASMU_OP_I32_GT_S:
        case WASMU_OP_I32_GT_U:
        case WASMU_OP_I32_LE_S:
        case WASMU_OP_I32_LE_U:
        case WASMU_OP_I32_GE_S:
        case WASMU_OP_I32_GE_U:
        case WASMU_OP_I32_CLZ:
        case WASMU_OP_I32_CTZ:
        case WASMU_OP_I32_POPCNT:
        case WASMU_OP_I32_ADD:
        case WASMU_OP_I32_SUB:
        case WASMU_OP_I32_MUL:
        case WASMU_OP_I32_DIV_S:
        case WASMU_OP_I32_DIV_U:
        case WASMU_OP_I32_REM_S:
        case WASMU_OP_I32_REM_U:
        case WASMU_OP_I32_AND:
        case WASMU_OP_I32_OR:
        case WASMU_OP_I32_XOR:
        case WASMU_OP_I32_SHL:
        case WASMU_OP_I32_SHR_S:
        case WASMU_OP_I32_SHR_U:
        case WASMU_OP_I32_ROTL:
        case WASMU_OP_I32_ROTR:
        case WASMU_OP_I32_WRAP_I64:
        case WASMU_OP_I32_TRUNC_F32_S:
        case WASMU_OP_I32_TRUNC_F32_U:
        case WASMU_OP_I32_TRUNC_F64_S:
        case WASMU_OP_I32_TRUNC_F64_U:
        case WASMU_OP_I32_REINTERPRET_F32:
            return WASMU_VALUE_TYPE_I32;

        case WASMU_OP_I64_LOAD:
        case WASMU_OP_I64_LOAD8_S:
        case WASMU_OP_I64_LOAD8_U:
        case WASMU_OP_I64_LOAD16_S:
        case WASMU_OP_I64_LOAD16_U:
        case WASMU_OP_I64_LOAD32_S:
        case WASMU_OP_I64_LOAD32_U:
        case WASMU_OP_I64_STORE:
        case WASMU_OP_I64_STORE8:
        case WASMU_OP_I64_STORE16:
        case WASMU_OP_I64_STORE32:
        case WASMU_OP_I64_CONST:
        case WASMU_OP_I64_EQZ:
        case WASMU_OP_I64_EQ:
        case WASMU_OP_I64_NE:
        case WASMU_OP_I64_LT_S:
        case WASMU_OP_I64_LT_U:
        case WASMU_OP_I64_GT_S:
        case WASMU_OP_I64_GT_U:
        case WASMU_OP_I64_LE_S:
        case WASMU_OP_I64_LE_U:
        case WASMU_OP_I64_GE_S:
        case WASMU_OP_I64_GE_U:
        case WASMU_OP_I64_CLZ:
        case WASMU_OP_I64_CTZ:
        case WASMU_OP_I64_POPCNT:
        case WASMU_OP_I64_ADD:
        case WASMU_OP_I64_SUB:
        case WASMU_OP_I64_MUL:
        case WASMU_OP_I64_DIV_S:
        case WASMU_OP_I64_DIV_U:
        case WASMU_OP_I64_REM_S:
        case WASMU_OP_I64_REM_U:
        case WASMU_OP_I64_AND:
        case WASMU_OP_I64_OR:
        case WASMU_OP_I64_XOR:
        case WASMU_OP_I64_SHL:
        case WASMU_OP_I64_SHR_S:
        case WASMU_OP_I64_SHR_U:
        case WASMU_OP_I64_ROTL:
        case WASMU_OP_I64_ROTR:
        case WASMU_OP_I64_EXTEND_I32_S:
        case WASMU_OP_I64_EXTEND_I32_U:
        case WASMU_OP_I64_TRUNC_F32_S:
        case WASMU_OP_I64_TRUNC_F32_U:
        case WASMU_OP_I64_TRUNC_F64_S:
        case WASMU_OP_I64_TRUNC_F64_U:
        case WASMU_OP_I64_REINTERPRET_F64:
            return WASMU_VALUE_TYPE_I64;

        case WASMU_OP_F32_LOAD:
        case WASMU_OP_F32_STORE:
        case WASMU_OP_F32_CONST:
        case WASMU_OP_F32_EQ:
        case WASMU_OP_F32_NE:
        case WASMU_OP_F32_LT:
        case WASMU_OP_F32_GT:
        case WASMU_OP_F32_LE:
        case WASMU_OP_F32_GE:
        case WASMU_OP_F32_ABS:
        case WASMU_OP_F32_NEG:
        case WASMU_OP_F32_CEIL:
        case WASMU_OP_F32_FLOOR:
        case WASMU_OP_F32_TRUNC:
        case WASMU_OP_F32_NEAREST:
        case WASMU_OP_F32_SQRT:
        case WASMU_OP_F32_ADD:
        case WASMU_OP_F32_SUB:
        case WASMU_OP_F32_MUL:
        case WASMU_OP_F32_DIV:
        case WASMU_OP_F32_MIN:
        case WASMU_OP_F32_MAX:
        case WASMU_OP_F32_COPYSIGN:
        case WASMU_OP_F32_CONVERT_I32_S:
        case WASMU_OP_F32_CONVERT_I32_U:
        case WASMU_OP_F32_CONVERT_I64_S:
        case WASMU_OP_F32_CONVERT_I64_U:
        case WASMU_OP_F32_DEMOTE_F64:
        case WASMU_OP_F32_REINTERPRET_I32:
            return WASMU_VALUE_TYPE_F32;

        case WASMU_OP_F64_LOAD:
        case WASMU_OP_F64_STORE:
        case WASMU_OP_F64_CONST:
        case WASMU_OP_F64_EQ:
        case WASMU_OP_F64_NE:
        case WASMU_OP_F64_LT:
        case WASMU_OP_F64_GT:
        case WASMU_OP_F64_LE:
        case WASMU_OP_F64_GE:
        case WASMU_OP_F64_ABS:
        case WASMU_OP_F64_NEG:
        case WASMU_OP_F64_CEIL:
        case WASMU_OP_F64_FLOOR:
        case WASMU_OP_F64_TRUNC:
        case WASMU_OP_F64_NEAREST:
        case WASMU_OP_F64_SQRT:
        case WASMU_OP_F64_ADD:
        case WASMU_OP_F64_SUB:
        case WASMU_OP_F64_MUL:
        case WASMU_OP_F64_DIV:
        case WASMU_OP_F64_MIN:
        case WASMU_OP_F64_MAX:
        case WASMU_OP_F64_COPYSIGN:
        case WASMU_OP_F64_CONVERT_I32_S:
        case WASMU_OP_F64_CONVERT_I32_U:
        case WASMU_OP_F64_CONVERT_I64_S:
        case WASMU_OP_F64_CONVERT_I64_U:
        case WASMU_OP_F64_PROMOTE_F32:
        case WASMU_OP_F64_REINTERPRET_I64:
            return WASMU_VALUE_TYPE_F64;

        default:
            return WASMU_VALUE_TYPE_I32;
    }
}

wasmu_ValueType wasmu_getOpcodeObjectType(wasmu_Opcode opcode) {
    switch (opcode) {
        case WASMU_OP_I64_EXTEND_I32_S:
        case WASMU_OP_I64_EXTEND_I32_U:
        case WASMU_OP_F32_CONVERT_I32_S:
        case WASMU_OP_F32_CONVERT_I32_U:
        case WASMU_OP_F64_CONVERT_I32_S:
        case WASMU_OP_F64_CONVERT_I32_U:
        case WASMU_OP_F32_REINTERPRET_I32:
            return WASMU_VALUE_TYPE_I32;

        case WASMU_OP_I32_WRAP_I64:
        case WASMU_OP_F32_CONVERT_I64_S:
        case WASMU_OP_F32_CONVERT_I64_U:
        case WASMU_OP_F64_CONVERT_I64_S:
        case WASMU_OP_F64_CONVERT_I64_U:
        case WASMU_OP_F64_REINTERPRET_I64:
            return WASMU_VALUE_TYPE_I64;

        case WASMU_OP_I32_TRUNC_F32_S:
        case WASMU_OP_I32_TRUNC_F32_U:
        case WASMU_OP_I64_TRUNC_F32_S:
        case WASMU_OP_I64_TRUNC_F32_U:
        case WASMU_OP_I32_REINTERPRET_F32:
        case WASMU_OP_F64_PROMOTE_F32:
            return WASMU_VALUE_TYPE_F32;

        case WASMU_OP_I32_TRUNC_F64_S:
        case WASMU_OP_I32_TRUNC_F64_U:
        case WASMU_OP_I64_TRUNC_F64_S:
        case WASMU_OP_I64_TRUNC_F64_U:
        case WASMU_OP_I64_REINTERPRET_F64:
        case WASMU_OP_F32_DEMOTE_F64:
            return WASMU_VALUE_TYPE_F64;


        default:
            return WASMU_VALUE_TYPE_I32;
    }
}

wasmu_Bool wasmu_opcodeIsSigned(wasmu_Opcode opcode) {
    switch (opcode) {
        case WASMU_OP_I32_LOAD8_S:
        case WASMU_OP_I32_LOAD16_S:
        case WASMU_OP_I32_LT_S:
        case WASMU_OP_I32_GT_S:
        case WASMU_OP_I32_LE_S:
        case WASMU_OP_I32_GE_S:
        case WASMU_OP_I32_DIV_S:
        case WASMU_OP_I32_REM_S:
        case WASMU_OP_I32_SHR_S:
        case WASMU_OP_I32_TRUNC_F32_S:
        case WASMU_OP_I32_TRUNC_F64_S:
        case WASMU_OP_I64_LOAD8_S:
        case WASMU_OP_I64_LOAD16_S:
        case WASMU_OP_I64_LOAD32_S:
        case WASMU_OP_I64_LT_S:
        case WASMU_OP_I64_GT_S:
        case WASMU_OP_I64_LE_S:
        case WASMU_OP_I64_GE_S:
        case WASMU_OP_I64_DIV_S:
        case WASMU_OP_I64_REM_S:
        case WASMU_OP_I64_SHR_S:
        case WASMU_OP_I64_EXTEND_I32_S:
        case WASMU_OP_I64_TRUNC_F32_S:
        case WASMU_OP_I64_TRUNC_F64_S:
        case WASMU_OP_F32_CONVERT_I32_S:
        case WASMU_OP_F32_CONVERT_I64_S:
        case WASMU_OP_F64_CONVERT_I32_S:
        case WASMU_OP_F64_CONVERT_I64_S:
            return WASMU_TRUE;

        default:
            return WASMU_FALSE;
    }
}