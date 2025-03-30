import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import warnings

df_wave = pd.read_csv('../security_analysis/results_wave_attack.csv')
df_wave['N_RH'] = df_wave['max_NRH']

# Get the maximum N_RH for each act_TBRFM, keeping the corresponding Chip
integrated_df = (
    df_wave.sort_values(by="N_RH", ascending=False)
    .drop_duplicates(subset=["act_TBRFM", "Chip"], keep="first")
    .sort_values(by="act_TBRFM")
)
# Reset the index for the final DataFrame
integrated_df.reset_index(drop=True, inplace=True)

# Mapping of N_RH to WS
RFM_frequency_mapping = {
    10: 0.25,
    26: 0.5,
    43: 0.75,
    60: 1,
    127: 2,
    261: 4,
    530: 8,
}
integrated_df['RFM_Frequency'] = integrated_df['act_TBRFM'].map(RFM_frequency_mapping)
integrated_df = integrated_df.dropna(subset=['RFM_Frequency']).reset_index(drop=True)

# Save target results
df_wave_target = integrated_df.reset_index(drop=True)
df_wave_target.to_csv("../results/csvs/results_fig7.csv", index=False)