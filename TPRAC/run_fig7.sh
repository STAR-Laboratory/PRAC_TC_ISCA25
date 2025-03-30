#!/bin/bash
cd security_analysis
echo "[INFO] Running Security Analysis for Figure 7"
python3 wave_attack.py
cd ../plot_scripts
echo "[INFO] Collating Results for Figure 7"
python3 generate_csv_fig7.py
echo "[INFO] Plotting Figure 7"
python3 plot_fig7.py
cd ../