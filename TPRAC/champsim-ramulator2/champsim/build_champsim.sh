#!/bin/bash
# TODO: Temporary check build time
start_time=$(date +%s)
# Check if the config file is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <config_json_file>"
    exit 1
fi

CONFIG_FILE="$1"  # e.g., "champsim-hashed_perceptron-spp_dev-srrip-ABO_Only-1024-PRAC1.json"

# Dynamically determine the directory where this script resides
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CHAMPSIM_DIR="$SCRIPT_DIR"

# Change to the ChampSim directory
cd "$CHAMPSIM_DIR" || exit 1

echo "Building ChampSim binary for configuration: $CONFIG_FILE"

# Step 1: Run config.sh with the full JSON filename
./config.sh "$CONFIG_FILE"

# Step 2: Build ChampSim
make -j8 ## Set # of cores for build here

echo "Finished building: $CONFIG_FILE"
# TODO: Temporary check build time
end_time=$(date +%s)
elapsed=$(( end_time - start_time ))
echo ""
echo "✅ All configurations built successfully."
printf "🕒 Total time: %02d:%02d:%02d (hh:mm:ss)\n" $((elapsed/3600)) $(( (elapsed/60)%60 )) $((elapsed%60))
