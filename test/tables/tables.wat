(module
    (table 3 funcref)
    (elem (i32.const 0) $a $b $c)
    (elem (i32.const 1) $c $b)
    (func $a (param i32) (result i32)
        local.get 0
        i32.const 2
        i32.add
    )
    (func $b (param i32) (result i32)
        local.get 0
        i32.const 2
        i32.sub
    )
    (func $c (param i32) (result i32)
        local.get 0
        i32.const 2
        i32.mul
    )
    (func (export "callIndirect") (param $mode i32) (param $value i32) (result i32)
        local.get $value
        local.get $mode
        call_indirect
        i32.const 3
        i32.mul
    )
)