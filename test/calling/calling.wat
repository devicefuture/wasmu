(module
    (func $addThree (export "addThree") (param i32 i32 i32) (result i32)
        local.get 0 ;; Add unused value to stack to test cleaning the value stack when returning
        local.get 0
        local.get 1
        i32.add
        local.get 2
        i32.add
        return
    )
    (func $addFour (export "addFour") (param i32 i32 i32 i32) (result i32)
        local.get 0
        local.get 1
        i32.add
        local.get 2
        i32.add
        local.get 3
        i32.add
    )
    (func (export "addSeven") (param i32 i32 i32 i32 i32 i32 i32) (result i32)
        local.get 0
        local.get 1
        local.get 2
        call $addThree
        local.get 3
        local.get 4
        local.get 5
        local.get 6
        call $addFour
        i32.add
    )
)