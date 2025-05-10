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