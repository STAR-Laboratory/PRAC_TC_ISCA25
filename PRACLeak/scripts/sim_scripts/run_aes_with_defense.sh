#!/bin/bash

# Please run this script in the PRACLeak directory

RAMULATOR_PATH="./ramulator2"
CONFIG_PATH="./results/configs/AES_with_defense"

echo "Running Figure 5"

echo "Generating configuration files ..."
python3 ./scripts/util_scripts/gen_aes_yaml.py ./results/configs/example_AES_with_defense.yaml ./results/configs/AES_with_defense/ --with_defense

echo "Running simulations ..."

for ((i = 0; i < 256; i++)); do
    "$RAMULATOR_PATH/ramulator2" -f "$CONFIG_PATH/$i.yaml" > ./results/stats/AES_with_defense.log
    echo Test $[i+1]/256 done.
done