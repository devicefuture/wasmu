(module
    (func (export "addI64") (result i64)
        i64.const 0x0123456789ABCDEF
        i64.const 0x1111111111111111
        i64.add
    )
)