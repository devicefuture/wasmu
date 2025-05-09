#!/bin/bash

CC=clang

if command -v clang-14 2>&1 /dev/null; then
    CC=clang-14
fi

$CC \
    --target=wasm32 \
    -z stack-size=16384 \
    -fno-rtti \
    -nostdlib \
    -mexec-model=reactor \
    -Wl,--initial-memory=65536 \
    -Wl,--allow-undefined-file=cpp.syms \
    -Wl,--no-entry \
    -Wl,--export=__heap_base \
    -Wl,--export=__wasm_call_ctors \
    -Wl,-error-limit=0 \
    cpp.cpp -o build/cpp.wasm