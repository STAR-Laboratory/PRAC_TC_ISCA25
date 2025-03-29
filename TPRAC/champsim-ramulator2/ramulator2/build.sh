#!/bin/bash
# Create the build directory
mkdir build
cd build

# Default to Release build if no argument is provided
BUILD_TYPE=${1:-Release}

# Configure the project
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

# Build the project using all available cores
make -j$(nproc)

# Copy the resulting binary to the project root
cp ./ramulator2 ../ramulator2

# Move back to the root directory
cd ..

