(module
    (memory 1 4)
    (data (i32.const 1024) "ABCDEFGH")
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
    (func (export "readData") (result i32)
        i32.const 1024
        i32.load
        i32.const 1028
        i32.load
        i32.add
    )
    (func (export "storeLoad16") (result i32)
        i32.const 2048
        i32.const -1234
        i32.store16
        i32.const 2048
        i32.load16_s
    )
)