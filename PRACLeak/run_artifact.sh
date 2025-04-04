#!/bin/bash

# Default values (optional)
use_sample=false

# Parse arguments
for arg in "$@"; do
  case $arg in
    --use-sample)
      use_sample=true
      shift
      ;;
    *)
      echo "Usage: $0 [--use-sample]"
      exit 1
      ;;
  esac
done

# Generate/Download Traces
if [ "$use_sample" = true ]; then
  echo "#####################"
  echo "[INFO] Downloading AES traces"
  echo "#####################"

  mkdir -p traces/
  if [ "$(ls -A traces/AES_traces)" ]; then
    echo "Traces had been downloaded. Skipping download."
  else
    echo "Downloading traces ..."
    wget -O AES_traces.zip "https://zenodo.org/records/15104056/files/AES_traces.zip?download=1"
    echo "Decompressing traces ..."
    unzip AES_traces.zip -d traces/
    rm -r AES_traces.zip
  fi

else
  echo "#####################"
  echo "[INFO] Generating AES traces"
  echo "#####################"

  export PIN_ROOT= your/path/to/pin/tools/pin-3.7-97619-g0d0c92f4f-gcc-linux

  echo "Building trace generator"
  cd trace_generator
  bash ./build.sh
  cd -
  
  echo "Generating AES traces"

  # Compile AES
  g++ ./AES_implementation/aesrand_in.c -o ./AES_implementation/aesrand_in

  mkdir -p traces/
  sudo bash ./scripts/util_scripts/gen_aes_code_traces.sh 200
fi

# Generate other traces
echo "#####################"
echo "[INFO] Generating other supplementary traces"
echo "#####################"

python3 ./scripts/util_scripts/gen_aes_attack_traces.py "$PWD/traces/AES_attackers"
# TODO covert channel traces
wget simple_AES.trace "https://zenodo.org/records/15113127/files/simple_AES.trace?download=1"
cp simple_AES.trace ./traces/
rm simple_AES.trace

# Build artifacts
echo "#####################"
echo "[INFO] Building Ramulator2"
echo "#####################"

# mkdir -p ramulator2/build
cd ramulator2/build
cmake ..
make -j
cp ./ramulator2 ../ramulator2
cd -

# Run experiments
echo "#####################"
echo "[INFO] Running experiments"
echo "#####################"

# Figure 3
bash ./scripts/sim_scripts/run_latency.sh

# Figure 4
bash ./scripts/sim_scripts/run_side_channel.sh

# Figure 5
bash ./scripts/sim_scripts/run_aes_no_defense.sh

# Figure 13
bash ./scripts/sim_scripts/run_aes_with_defense.sh


