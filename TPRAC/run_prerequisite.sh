#!/bin/bash
CHAMPSIM_RAMULATOR_DIR="$PWD/champsim-ramulator2"
## 1. Install python dependences
# echo "---------------------------"
# echo ""
# echo "#####################"
# echo "[INFO] 1. Installing Python dependencies"
# echo "#####################"
# pip3 install -r python_dependencies.txt

# ## 2. Download the required traces
# echo "---------------------------"
# echo ""
# echo "#####################"
# echo "[INFO] 2. Downloading Required Traces"
# echo "#####################"
# # Create the traces directory if it doesn't exist
# mkdir -p traces
# if [ "$(ls -A traces/)" ]; then
#   echo "traces directory already contains the traces. Skipping download"
# else
#   echo "traces directory is empty"
#   echo "Downloading the required traces into the traces directory"
#   bash "$PWD/download_traces.sh"
#   echo "Decompressing the downloaded traces into the traces directory"
#   tar -xzvf TPRAC_Traces.tar.gz --no-same-owner -C traces/
#   rm -r TPRAC_Traces.tar.gz
# fi

# ## 3. Build Ramulator2
# echo "---------------------------"
# echo ""
# echo "#####################"
# echo "[INFO] 3. Building Ramulator2"
# echo "#####################"
# cd "$CHAMPSIM_RAMULATOR_DIR/ramulator2" 
# rm -rf ./build/
# bash "$PWD/build.sh"

## 4. Install required packages for ChampSim and Build ChampSim
echo "---------------------------"
echo ""
echo "#####################"
echo "[INFO] 4. Installing Required Packages for ChampSim and Building It"
echo "#####################"
cd "$CHAMPSIM_RAMULATOR_DIR/champsim"
echo "$PWD"
bash "./download_vcpkg.sh" 