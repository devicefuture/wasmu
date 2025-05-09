wasmu_Float wasmu_abs(wasmu_Float value) {
    return value < 0 ? value * -1 : value;
}

wasmu_Float wasmu_neg(wasmu_Float value) {
    return value * -1;
}