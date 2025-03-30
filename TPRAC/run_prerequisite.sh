#!/bin/bash
echo ""
echo "######################################################################################"
echo "[IMPORTANT] Please Run: source setup_lib_path.sh always before running this script"
echo "######################################################################################"
echo ""

# Optional runtime check for LD_LIBRARY_PATH
if [[ "$LD_LIBRARY_PATH" != *"ramulator2"* ]]; then
    echo "[WARNING] LD_LIBRARY_PATH does not appear to include 'ramulator2'."
    echo "          You may encounter linking errors unless you run: source setup_lib_path.sh"
    echo ""
fi

CHAMPSIM_RAMULATOR_DIR="$PWD/champsim-ramulator2"
## 1. Install python dependences
echo "-----------------------------------------"
echo ""
echo "#########################################"
echo "[INFO] 1. Installing Python dependencies"
echo "#########################################"
pip3 install -r python_dependencies.txt

## 2. Download the required traces ~3mins w/ 40 threads
echo "-----------------------------------------"
echo ""
echo "#########################################"
echo "[INFO] 2. Downloading Required Traces"
echo "#########################################"
# Create the traces directory if it doesn't exist
mkdir -p traces
if [ "$(ls -A traces/)" ]; then
  echo "traces directory already contains the traces. Skipping download"
else
  echo "traces directory is empty"
  echo "Downloading the required traces into the traces directory"
  bash "$PWD/download_traces.sh"
fi

## 3. Build Ramulator2
echo "-------------------------------"
echo ""
echo "###############################"
echo "[INFO] 3. Building Ramulator2"
echo "###############################"
cd "$CHAMPSIM_RAMULATOR_DIR/ramulator2" 
rm -rf ./build/
bash "$PWD/build.sh"

## 4. Install required packages for ChampSim and Build ChampSim
echo "--------------------------------------------------------------------"
echo ""
echo "####################################################################"
echo "[INFO] 4. Installing Required Packages for ChampSim and Building It"
echo "####################################################################"
cd "$CHAMPSIM_RAMULATOR_DIR/champsim"
echo "$PWD"
## 4.1. install vcpkg 
bash "./download_vcpkg.sh" 
## 4.2. generate champsim configurations
bash "./generate_champsim_config.sh"
## 4.3. build all champsim configurations: ~25 mins
bash "./build_all.sh"

