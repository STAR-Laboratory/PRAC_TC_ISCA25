#!/bin/bash

# Please run this script in the PRACLeak directory

RAMULATOR_PATH="./ramulator2"
YAML_PATH="./results/configs/example_side_channel.yaml"

echo "Running experiments for Figure 4 ..."

"$RAMULATOR_PATH/ramulator2" -f $YAML_PATH >> ./results/stats/side_channel.log
"$RAMULATOR_PATH/ramulator2" -f $YAML_PATH >> ./results/stats/side_channel.log

echo Test done.