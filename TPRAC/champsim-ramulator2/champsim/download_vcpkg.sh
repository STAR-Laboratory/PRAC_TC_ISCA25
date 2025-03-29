#!/bin/bash
set -e  # Exit immediately on error

echo "---------------------------"
echo ""
echo "#########################################"
echo "[INFO] Installing ChampSim Dependencies"
echo "#########################################"

# Initialize submodule
git submodule update --init --recursive

# Bootstrap vcpkg
./vcpkg/bootstrap-vcpkg.sh

# Install dependencies
./vcpkg/vcpkg install