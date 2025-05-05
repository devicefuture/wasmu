(module
    (import "a" "hello" (func $a (param i32 i32) (result i32)))
    (func (export "hello") (param i32) (result i32)
        local.get 0
        local.get 0
        i32.mul
    )
    (func (export "testCallA") (result i32)
        i32.const 3
        i32.const 4
        call $a
    )
)