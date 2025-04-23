(module
    (memory 1 2)
    (func (export "storeLoad") (result i32)
        i32.const 1234
        i32.const 4321
        i32.store
        i32.const 1234
        i32.load
    )
)