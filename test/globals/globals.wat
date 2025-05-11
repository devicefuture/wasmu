(module
    (global $a (export "a") (mut i32) (i32.const 10))
    (global $b (export "b") (mut i32) (i32.const 20))
    (func (export "getGlobals") (result i32 i32)
        global.get $a
        global.get $b
    )
    (func (export "setGlobals") (result i32 i32)
        i32.const 30
        global.set $a
        i32.const 40
        global.set $b
        global.get $a
        global.get $b
    )
)