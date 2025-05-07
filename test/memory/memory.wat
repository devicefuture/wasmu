(module
    (memory 1 4)
    (func (export "storeLoad") (result i32)
        i32.const 1234
        i32.const 4321
        i32.store
        i32.const 1234
        i32.load
    )
    (func (export "getSize") (result i32)
        memory.size
    )
    (func (export "grow") (param i32) (result i32)
        local.get 0
        memory.grow
    )
)