#!/bin/bash
echo "---------------------------"
echo ""
echo "#########################################"
echo "[INFO] Building All ChampSim Binaries"
echo "#########################################"
# TODO: Temporary check build time
start_time=$(date +%s)
# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set the config directory relative to this script's location
CONFIG_DIR="$SCRIPT_DIR/configurations/champsim_configs/TPRAC_ISCA2025/4cores-1CH-4RA"

# Sanity check
if [ ! -d "$CONFIG_DIR" ]; then
    echo "Config directory not found: $CONFIG_DIR"
    exit 1
fi

# Loop through all JSON configuration files in the directory
for config_file in "$CONFIG_DIR"/*.json; do
    ./build_champsim.sh "$config_file"
done

echo "All configurations have been built successfully."
# TODO: Temporary check build time
end_time=$(date +%s)
elapsed=$(( end_time - start_time ))
echo ""
echo "✅ All configurations built successfully."
printf "🕒 Total time: %02d:%02d:%02d (hh:mm:ss)\n" $((elapsed/3600)) $(( (elapsed/60)%60 )) $((elapsed%60))

