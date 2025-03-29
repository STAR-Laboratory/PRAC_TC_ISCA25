#!/bin/bash
set -e  # Exit immediately on error

# Initialize submodule
git submodule update --init --recursive

# Bootstrap vcpkg
./vcpkg/bootstrap-vcpkg.sh

# Install dependencies
./vcpkg/vcpkg install