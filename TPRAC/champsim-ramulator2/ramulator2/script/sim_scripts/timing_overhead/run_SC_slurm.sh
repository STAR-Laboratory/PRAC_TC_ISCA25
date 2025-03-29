#! /bin/bash

SLURM_PART_NAME="skylake"

echo "[INFO] Generating Ramulator2 configurations and run scripts for single core benign workloads"
python3 setup_SC_slurm.py \
    --ramulator_directory "/scratch/st-prashnr-1/jeonghyun/ramulator2" \
    --working_directory "$PWD" \
    --base_config "/scratch/st-prashnr-1/jeonghyun/ramulator2/configs/DDR5_baseline_closed_mitigation.yaml" \
    --trace_combination "$PWD/../traces/4cores_traces.txt" \
    --trace_directory "/scratch/st-prashnr-1/jeonghyun/PRAC/Ramulator2-PRAC/cputraces" \
    --result_directory "/scratch/st-prashnr-1/jeonghyun/ramulator2/prac_study/TimingOverhead/SC/OPTMC/352rob" \
    --partition_name "$SLURM_PART_NAME"

echo "[INFO] Starting Ramulator2 benign simulations"
python3 execute_run_SC_script.py --slurm

echo "[INFO] You can track run status with the <check_run_status.sh> script"
rm "$PWD/run_SC.sh" 