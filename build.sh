#!/bin/bash

CC=gcc

mkdir -p dist

echo -n > dist/wasmu.h

function include {
    (
        echo "// $1"
        echo
        cat $1
        echo
        echo
    ) >> dist/wasmu.h
}

cp src/config.h dist/wasmu-config.h

tee -a dist/wasmu.h > /dev/null << EOF
#ifndef WASMU_H_
#define WASMU_H_

EOF

include src/config.h
include src/common.h
include src/declarations.h
include src/contexts.h
include src/parser.h

tee -a dist/wasmu.h > /dev/null << EOF
#endif
EOF

for testPath in test/*/; do
    test=$(basename $testPath)

    mkdir -p test/$test/build

    $CC -Idist/ test/$test/$test.c -DTEST_NAME=\"$test\" -o test/$test/build/$test

    if [ -f test/$test/script.sh ]; then
        chmod +x test/$test/script.sh

        pushd test/$test
            ./script.sh
        popd
    fi
done

if [ "$1" == "--test" ]; then
    allTestsPassed=true

    for testPath in test/*/; do
        test=$(basename $testPath)

        test/$test/build/$test > test/$test/build/test.log

        if [ $? == 0 ]; then
            echo "Test passed: $test"
        else
            echo "Test failed: $test"
            echo "---------"
            cat test/$test/build/test.log
            echo "---------"

            allTestsPassed=false
        fi
    done

    if [ $allTestsPassed == true ]; then
        echo "All tests passed"
    else
        exit 1
    fi
fi