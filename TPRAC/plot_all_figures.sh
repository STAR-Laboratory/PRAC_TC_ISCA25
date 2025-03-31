#!/bin/bash
cd plot_scripts

echo "[INFO] Collating Results for Figure 7"
python3 generate_csv_fig7.py

echo "[INFO] Plotting Figure 7"
python3 plot_fig7.py

echo "[INFO] Collating Results for Figure 9"
python3 generate_csv_fig9.py

echo "[INFO] Plotting Figure 9"
python3 plot_fig9.py

echo "[INFO] Collating Results for Figure 10"
python3 generate_csv_fig10.py

echo "[INFO] Plotting Figure 10"
python3 plot_fig10.py

echo "[INFO] Collating Results for Figure 11"
python3 generate_csv_fig11.py

echo "[INFO] Plotting Figure 11"
python3 plot_fig11.py

echo "[INFO] Collating Results for Figure 12"
python3 generate_csv_fig12.py

echo "[INFO] Plotting Figure 12"
python3 plot_fig12.py
cd ../