#!/bin/bash

echo "[INFO] Always set path for Ramulator2 library first before running experiments!"
### Manually set SLURM variables here (if not passed as environment variables)
SLURM_PART_NAME="${SLURM_PART_NAME:-skylake}"  # Default value is "skylake" if not set
SLURM_PART_DEF_MEM="${SLURM_PART_DEF_MEM:-6G}"  # Default memory size is '6G' if not set

# Export MAX_SLURM_JOBS with default values if not already set
export MAX_SLURM_JOBS=${MAX_SLURM_JOBS:-1000}

# Check if SLURM variables are set (if manually overridden or passed from environment)
echo "[INFO] SLURM_PART_NAME: $SLURM_PART_NAME"
echo "[INFO] SLURM_PART_DEF_MEM: $SLURM_PART_DEF_MEM"

export PYTHONPATH="$PWD/sim_scripts"
python3 "$PWD/sim_scripts/setup_slurm.py" \
    --champsim_directory "$PWD/champsim-ramulator2/champsim" \
    --champsim_bin_directory "$PWD/champsim-ramulator2/champsim/bin/4cores-1CH-4RA" \
    --working_directory "$PWD/sim_scripts" \
    --trace_directory "$PWD/traces" \
    --result_directory "$PWD/results" \
    --partition_name "$SLURM_PART_NAME" \
    --partition_default_memory "$SLURM_PART_DEF_MEM" \
    --trace_list "$PWD/trace_lists/4core_traces.txt" \
    -rc "run_config_fig11"

echo "[INFO] Starting ChampSim simulations for Figure 11"
python3 "$PWD/sim_scripts/execute_run_script.py" -rc "run_config_fig11" --slurm

# TODO: Show how long it takes for simulations
echo "[INFO] Fired all simulations for Figure 11"
rm "$PWD/run.sh" 