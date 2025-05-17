#!/bin/bash
cd plot_scripts

echo "[INFO] Collating Results for Figure 7"
python3 generate_csv_fig7.py

echo "[INFO] Plotting Figure 7"
python3 plot_fig7.py

echo "[INFO] Collating Results for Figure 10"
python3 generate_csv_fig10.py

echo "[INFO] Plotting Figure 10"
python3 plot_fig10.py

cd ../