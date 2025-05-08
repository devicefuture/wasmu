(module
    (func (export "addI64") (result i64)
        i64.const 0x0123456789ABCDEF
        i64.const 0x1111111111111111
        i64.add
    )
    (func (export "truncF64S") (result i32)
        f64.const -10.5
        i32.trunc_f64_s
    )
    (func (export "subF32") (result i64)
        f32.const 3.5
        f32.const 8
        f32.sub
        i64.trunc_f32_s
    )
)