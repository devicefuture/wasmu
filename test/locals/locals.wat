(module
    (func (export "noLocals")
        nop
    )
    (func (export "paramsOnly") (param i32 i32 f32 i32)
        local.get 0
        drop
        local.get 1
        drop
        local.get 2
        drop
        local.get 3
        drop
    )
    (func (export "resultsOnly") (result i32 i32 f32 i32)
        i32.const 1
        i32.const 2
        f32.const 3
        i32.const 4
    )
    (func (export "paramsResults") (param i32 f32) (result i32 f32)
        local.get 0
        drop
        local.get 1
        drop
        i32.const 1
        f32.const 2
    )
    (func (export "localsOnly") (local i32 i32 f32 i32)
        i32.const 1
        local.set 0
        i32.const 2
        local.set 1
        f32.const 3
        local.set 2
        i32.const 4
        local.set 3
    )
    (func (export "paramsResultsLocals") (param i32 f32) (result i32 f32) (local i32 f32)
        i32.const 1
        local.set 0
        f32.const 2
        local.set 1
        local.get 0
        drop
        local.get 1
        drop
        i32.const 1
        f32.const 2
    )
    (func (export "setLocal") (param i32) (result i32) (local i32)
        local.get 0
        local.set 1
        local.get 0
        local.get 1
        i32.add
    )
)