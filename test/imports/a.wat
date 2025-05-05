(module
    (import "b" "hello" (func $b (param i32) (result i32)))
    (import "c" "hello" (func $c (param i32 i32) (result i32)))
    (func (export "hello") (param i32 i32) (result i32)
        local.get 0
        local.get 1
        i32.mul
    )
    (func (export "testCallB") (result i32)
        i32.const 5
        call $b
        i32.const 2
        i32.add
    )
    (func (export "testCallC") (result i32)
        i32.const 3
        i32.const 2
        call $c
        i32.const 2
        i32.mul
    )
)