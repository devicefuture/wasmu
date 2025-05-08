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
    (func (export "divF32S") (result i64)
        i64.const -20
        f32.convert_i64_s
        f32.const 4
        f32.div
        i64.trunc_f32_s
    )
    (func (export "divF32U") (result i64)
        i64.const -20
        f32.convert_i64_u
        f32.const 4
        f32.div
        i64.trunc_f32_s
    )
    (func (export "wrap") (result i32)
        i64.const 0xFEDCBA9876543210
        i32.wrap_i64
    )
    (func (export "extendS") (result i64)
        i32.const -10
        i64.extend_i32_s
    )
    (func (export "extendU") (result i64)
        i32.const -10
        i64.extend_i32_u
    )
    (func (export "demote") (result f32)
        f64.const 123.456789
        f32.demote_f64
    )
    (func (export "promote") (result f64)
        f32.const 123.456789
        f64.promote_f32
    )
)