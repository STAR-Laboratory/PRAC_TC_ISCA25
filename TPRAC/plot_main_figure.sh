#!/bin/bash
# TODO: Add security analysis scripts here
cd plot_scripts
echo "[INFO] Collating Results for Figure 9"
python3 generate_csv_fig9.py

echo "[INFO] Plotting Figure 9"
python3 plot_fig9.py

cd ../