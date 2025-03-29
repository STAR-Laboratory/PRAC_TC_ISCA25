#!/bin/bash

echo "---------------------------"
echo ""
echo "#######################################################"
echo "[INFO] Setup Ramulator2 Library Path for ChampSim"
echo "#######################################################"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RAMULATOR_DIR="$SCRIPT_DIR/../ramulator2"

export LD_LIBRARY_PATH="$RAMULATOR_DIR:$LD_LIBRARY_PATH"
echo "$LD_LIBRARY_PATH"
