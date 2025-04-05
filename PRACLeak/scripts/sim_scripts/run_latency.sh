#!/bin/bash

# Please run this script in the PRACLeak directory

RAMULATOR_PATH="./ramulator2"
CONFIG_PATH="./results/configs/latency"

echo "Running experiments for Figure 3 ..."

for i in 0 0 1 2 4; do
    "$RAMULATOR_PATH/ramulator2" -f "$CONFIG_PATH/${i}_RFM.yaml" >> ./results/stats/latency.log
    sleep 0.5
done

echo Test done.