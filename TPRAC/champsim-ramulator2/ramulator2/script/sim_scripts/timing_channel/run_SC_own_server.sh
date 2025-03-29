#! /bin/bash

echo "[INFO] Generating Ramulator2 configurations and run scripts for single core benign workloads"
python3 setup_SC_own_server.py \
    --ramulator_directory "../../.." \
    --working_directory "$PWD" \
    --base_config "../../../configs/DDR5_baseline_closed_mitigation.yaml" \
    --trace_combination "$PWD/../traces/4cores_traces.txt" \
    --trace_directory "../../../cputraces" \
    --result_directory "../../../prac_study/TimingChannel/SC" \

echo "[INFO] Starting Ramulator2 benign simulations"
python3 execute_run_SC_script.py

echo "[INFO] You can track run status with the <check_run_status.sh> script"
rm "$PWD/run_SC.sh" 