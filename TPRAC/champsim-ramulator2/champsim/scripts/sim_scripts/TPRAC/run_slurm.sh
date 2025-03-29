#!/bin/bash
echo "[INFO] Always set path for ramulator2 library first before running it!"
### Manually set SLURM variables here (if not passed as environment variables)
SLURM_PART_NAME="${SLURM_PART_NAME:-skylake}"  # Default value is "skylake" if not set
SLURM_PART_DEF_MEM="${SLURM_PART_DEF_MEM:-6G}"  # Default memory size is '6G' if not set
SLRUM_PART_BIG_MEM="${SLRUM_PART_BIG_MEM:-12G}"  # Default memory size is '12G' if not set

# Export MAX_SLURM_JOBS with default values if not already set
export MAX_SLURM_JOBS=${MAX_SLURM_JOBS:-1000}

# Check if SLURM variables are set (if manually overridden or passed from environment)
echo "[INFO] SLURM_PART_NAME: $SLURM_PART_NAME"
echo "[INFO] SLURM_PART_DEF_MEM: $SLURM_PART_DEF_MEM"
echo "[INFO] SLRUM_PART_BIG_MEM: $SLRUM_PART_BIG_MEM"

echo "[INFO] Run ChampSim with Ramulator2"

##Example trace lists Use the list below after completing tests
# --trace_list "$PWD/../../trace_lists/4core_test.txt" //For test
# trace_list "$PWD/../../trace_lists/4core_traces.txt"
python3 setup_slurm.py \
    --champsim_directory "$PWD/../../.." \
    --champsim_bin_directory "$PWD/../../../bin/4cores-1CH-4RA-Updated" \
    --working_directory "$PWD" \
    --trace_directory "../../../traces" \
    --result_directory "$PWD/../../../temp_results/TPRAC_ISCA2025/4cores_results/L1D-IPStride/With_RD_Merge" \
    --partition_name "$SLURM_PART_NAME" \
    --partition_default_memory "$SLURM_PART_DEF_MEM" \
    --partition_big_memory "$SLRUM_PART_BIG_MEM" \
    --trace_list "$PWD/../../trace_lists/4core_traces.txt"

echo "[INFO] Starting ChampSim simulations"
python3 execute_run_script.py --slurm

echo "[INFO] Fired all simulations"
rm "$PWD/run.sh" 