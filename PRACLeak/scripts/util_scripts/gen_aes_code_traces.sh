#!/bin/bash

generate_hex_with_zeros() {
    iter=$1
    hex=""

    for i in {1..640}; do
        hex="$hex$(printf '%x' $((RANDOM % 16)))"
    done

    echo "$hex"
}

iteration=$1
export PIN_ROOT= your/path/to/pin/tools/pin-3.7-97619-g0d0c92f4f-gcc-linux

cd ./trace_generator
mkdir -p AES_traces

for ((i = 0; i < 256; i++)); do
    filename=./AES_traces/"$i"

    hex_input=$(generate_hex_with_zeros "$iteration")

    ./tracegenerator.sh -t "$filename".out -dcache off -mode cpu -paddr off -ifetch off -- ../AES_implementation/aesrand_in "$iteration" "$i" "$hex_input" > "$filename".log

    python3 ../scripts/util_scripts/clean_trace.py "$filename".out "$filename".trace

    rm -r "$filename".out
    echo Generated $[i+1]/256.

done

cp -r --no-preserve=mode,ownership AES_traces ../traces/
sudo chmod -R a+rw ../traces/AES_traces
rm -r AES_traces

cd -

