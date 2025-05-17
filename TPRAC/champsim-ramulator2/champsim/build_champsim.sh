#!/bin/bash
# Check if the config file is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <config_json_file>"
    exit 1
fi

CONFIG_FILE="$1"  # e.g., "champsim-hashed_perceptron-spp_dev-srrip-ABO_Only-1024-PRAC1.json"

# Dynamically determine the directory where this script resides
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CHAMPSIM_DIR="$SCRIPT_DIR"

# Extract binary name: just the filename without .json
BINARY_NAME="$(basename "$CONFIG_FILE" .json)"

#### NOTE: We currently skip recompilation of binaries if they already exist.
if [ -f "$CHAMPSIM_DIR/bin/4cores-1CH-4RA/$BINARY_NAME" ]; then
    echo "[SKIP] Binary already exists: $BINARY_NAME"
    exit 0
fi

# Change to the ChampSim directory
cd "$CHAMPSIM_DIR" || exit 1

echo "Building ChampSim binary for configuration: $CONFIG_FILE"

# Step 1: Run config.sh with the full JSON filename
./config.sh "$CONFIG_FILE"

# Build ChampSim binary using 8 cores
if ! make -j4; then
    echo "[ERROR] Build failed for: $CONFIG_FILE"
    exit 1
fi

echo "Finished building: $CONFIG_FILE"