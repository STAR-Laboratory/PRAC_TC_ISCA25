#!/bin/bash

cd scripts/plot_scripts

echo "[INFO] Plotting Figure 3"
python3 plot_fig3.py

echo "[INFO] Plotting Figure 4"
python3 plot_fig4.py

echo "[INFO] Plotting Figure 5"
python3 plot_fig5.py

echo "[INFO] Plotting Figure 13"
python3 plot_fig13.py

cd -

echo "[INFO] All PRACLeak-related plots are stored in PRACLeak/results/plots/".