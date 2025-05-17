#!/bin/bash
echo "[INFO] Running TPRAC Security Analysis for Figure 7"
cd security_analysis
echo "[INFO] 1. With Activation Counter Reset"
python3 wave_attack.py
echo "[INFO] 2. Without Activation Counter Reset"
python3 wave_attack_no_reset.py
cd ../