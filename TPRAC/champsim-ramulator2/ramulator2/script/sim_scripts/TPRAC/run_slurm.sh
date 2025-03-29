#!/bin/bash

SLURM_PART_NAME="skylake"

echo "[INFO] Generating Ramulator2 configurations and run scripts for benign workloads"
python3 setup_slurm.py \
    --ramulator_directory "/scratch/st-prashnr-1/jeonghyun/ramulator2" \
    --working_directory "$PWD" \
    --base_config "/scratch/st-prashnr-1/jeonghyun/ramulator2/configs/DDR5_baseline_closed_mitigation.yaml" \
    --trace_combination "$PWD/../traces/4cores_traces.txt" \
    --trace_directory "/scratch/st-prashnr-1/jeonghyun/PRAC/Ramulator2-PRAC/cputraces" \
    --result_directory "/scratch/st-prashnr-1/jeonghyun/ramulator2/TPRAC_ISCA2025/4cores_results" \
    --partition_name "$SLURM_PART_NAME"

echo "[INFO] Starting Ramulator2 benign simulations"
python3 execute_run_script.py --slurm

echo "[INFO] You can track run status with the <check_run_status.sh> script"
rm "$PWD/run.sh" 