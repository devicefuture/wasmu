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
    (func (export "branchInBlock") (result i32)
        (block (result i32)
            i32.const 1
            br 0
            drop
            i32.const 2
        )
        i32.const 3
        i32.add
    )
    (func (export "nearBranchInNestedBlocks") (result i32)
        (block (result i32)
            (block (result i32)
                i32.const 1
                br 0
                drop
                i32.const 2
            )
            drop
            i32.const 3
        )
        i32.const 4
        i32.add
    )
    (func (export "farBranchInNestedBlocks") (result i32)
        (block (result i32)
            (block (result i32)
                i32.const 1
                br 1
                drop
                i32.const 2
            )
            drop
            i32.const 3
        )
        i32.const 4
        i32.add
    )
    (func (export "loopCumulativeSum") (param $count i32) (result i32) (local $sum i32)
        (loop $iter
            local.get $sum
            local.get $count
            i32.add
            local.set $sum
            local.get $count
            i32.const 1
            i32.sub
            local.tee $count
            br_if $iter
        )
        local.get $sum
    )
    (func (export "excessStackValues") (result i32)
        (block (result i32 i32)
            i32.const 1
            i32.const 2
            i32.const 3
            i32.const 4
            br 0
            i32.const 4
        )
        i32.const 5
        i32.add
        i32.add
    )
)