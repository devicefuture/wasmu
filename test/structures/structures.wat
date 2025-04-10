(module
    (func (export "basicBlock") (param i32) (result i32)
        (block (result i32)
            local.get 0
            local.get 0
            i32.add
        )
        local.get 0
        i32.add
    )
)