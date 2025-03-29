#!/bin/bash

# Check if the config file is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <config_json_file>"
    exit 1
fi

CONFIG_FILE="$1"  # Full JSON filename (e.g., "champsim-hashed_perceptron-spp_dev-srrip-ABO_Only-1024-PRAC1.json")

# Set the ChampSim directory (adjust if needed)
CHAMPSIM_DIR="/scratch/st-prashnr-1/jeonghyun/champsim-ramulator2/champsim"

# Change to the ChampSim directory
cd "$CHAMPSIM_DIR" || exit 1

echo "Building ChampSim binary for configuration: $CONFIG_FILE"

# Step 1: Run config.sh with the full JSON filename
./config.sh "$CONFIG_FILE"

# Step 2: Build ChampSim
make -j$(nproc)  # Use all available CPU cores for faster compilation

echo "Finished building: $CONFIG_FILE"
