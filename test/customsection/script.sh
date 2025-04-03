#!/bin/bash

echo -en "" > build/customsection.wasm

echo -en "\x00\x61\x73\x6D\x01\x00\x00\x00" >> build/customsection.wasm # Magic

echo -en "\x00\x0A" >> build/customsection.wasm # Section type and size
echo -en "\x05TestA" >> build/customsection.wasm # Section name
echo -en "\x01\x02\x03\x04" >> build/customsection.wasm # Section data

echo -en "\x00\x0C" >> build/customsection.wasm # Section type and size
echo -en "\x05TestB" >> build/customsection.wasm # Section name
echo -en "\x01\x02\x03\x04\x05\x06" >> build/customsection.wasm # Section data