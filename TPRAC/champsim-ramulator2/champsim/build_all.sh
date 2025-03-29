#!/bin/bash

# Set the directory containing configuration files
CONFIG_DIR="/scratch/st-prashnr-1/jeonghyun/champsim-ramulator2/champsim/configurations/champsim_configs/TPRAC_ISCA2025/4cores-1CH-4RA-Updated"

# Loop through all JSON configuration files in the directory
for config_file in "$CONFIG_DIR"/*.json; do
    # Call build_single.sh with the full JSON filename
    ./build_champsim.sh "$config_file"
done

echo "All configurations have been built successfully."
