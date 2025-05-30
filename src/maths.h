wasmu_Float wasmu_abs(wasmu_Float value) {
    return value < 0 ? value * -1 : value;
}

wasmu_Float wasmu_neg(wasmu_Float value) {
    return value * -1;
}

wasmu_Float wasmu_ceil(wasmu_Float value) {
    wasmu_Int intValue = value;

    return value - intValue == 0 ? value : intValue + 1;
}

wasmu_Float wasmu_floor(wasmu_Float value) {
    wasmu_Int intValue = value;

    return value == intValue || value >= 0 ? intValue : intValue - 1;
}

wasmu_Float wasmu_trunc(wasmu_Float value) {
    return (wasmu_Int)value;
}

wasmu_Float wasmu_nearest(wasmu_Float value) {
    return (wasmu_Int)(value < 0 ? value - 0.5 : value + 0.5);
}

// @source reference https://stackoverflow.com/a/49991852
// @licence ccbysa3
wasmu_Float wasmu_sqrt(wasmu_Float value) {
    wasmu_Float result = 1;

    for (wasmu_Count i = 0; i < 11; i++) {
        result -= ((result * result) - value) / (2 * result);
    }

    return result;
}

wasmu_Float wasmu_min(wasmu_Float a, wasmu_Float b) {
    return a > b ? b : a;
}

wasmu_Float wasmu_max(wasmu_Float a, wasmu_Float b) {
    return a < b ? b : a;
}

wasmu_Float wasmu_copysign(wasmu_Float a, wasmu_Float b) {
    return (a < 0 && b < 0) || (a >= 0 && b >= 0) ? a : -a;
}