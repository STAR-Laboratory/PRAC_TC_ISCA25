#!/bin/bash

echo "[INFO] Always set path for Ramulator2 library first before running experiments!"
### Manually set SLURM variables here (if not passed as environment variables)
export PERSONAL_RUN_THREADS=${PERSONAL_RUN_THREADS:-40}

rm -r "$PWD/run.sh"

export PYTHONPATH="$PWD/sim_scripts"
python3 "$PWD/sim_scripts/setup_ps.py" \
    --champsim_directory "$PWD/champsim-ramulator2/champsim" \
    --champsim_bin_directory "$PWD/champsim-ramulator2/champsim/bin/4cores-1CH-4RA" \
    --working_directory "$PWD/sim_scripts" \
    --trace_directory "$PWD/traces" \
    --result_directory "$PWD/results" \
    --trace_list "$PWD/trace_lists/4core_traces.txt" \
    -rc "run_config_fig12"

echo "[INFO] Starting ChampSim simulations for Figure 12"
python3 "$PWD/sim_scripts/execute_run_script.py" -rc "run_config_fig12"