#!/bin/bash
echo "---------------------------"
echo ""
echo "#########################################"
echo "[INFO] Generating ChampSim Configurations"
echo "#########################################"

# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Navigate to the champsim_configs directory
cd "$SCRIPT_DIR/configurations/champsim_configs" || {
    echo "Directory not found: $SCRIPT_DIR/configurations/champsim_configs"
    exit 1
}

# Run the Python script
python3 champsim_config_factory.py
