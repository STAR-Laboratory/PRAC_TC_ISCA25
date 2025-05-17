import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import warnings

df_reset = pd.read_csv('../security_analysis/results_wave_attack.csv')
df_no_reset = pd.read_csv('../security_analysis/results_wave_attack_no_reset.csv')

# Filter for 32Gb chip only
df_reset = df_reset[df_reset['Chip'] == '32Gb'].copy()
df_no_reset = df_no_reset[df_no_reset['Chip'] == '32Gb'].copy()

# Add source column to distinguish the datasets
df_reset['reset'] = 'with_reset'
df_no_reset['reset'] = 'no_reset'

# Combine both datasets
df_combined = pd.concat([df_reset, df_no_reset], ignore_index=True)

# Rename max_NRH for clarity
df_combined['N_RH'] = df_combined['max_NRH']

# Get the maximum N_RH for each act_TBRFM, keeping the corresponding Chip
integrated_df = (
    df_combined.sort_values(by="N_RH", ascending=False)
    .drop_duplicates(subset=["act_TBRFM", "Chip"], keep="first")
    .sort_values(by="act_TBRFM")
)
# Reset the index for the final DataFrame
integrated_df.reset_index(drop=True, inplace=True)

# Keep max N_RH per act_TBRFM per reset type
integrated_df = (
    df_combined
    .sort_values(by="N_RH", ascending=False)
    .drop_duplicates(subset=["act_TBRFM", "reset"], keep="first")
    .sort_values(by=["reset", "act_TBRFM"])
    .reset_index(drop=True)
)

# Mapping of act_TBRFM to RFM frequency
RFM_frequency_mapping = {
    10: 0.25,
    26: 0.5,
    43: 0.75,
    60: 1,
    127: 2,
    261: 4,
    530: 8,
}

# Apply mapping
integrated_df['RFM_Frequency'] = integrated_df['act_TBRFM'].map(RFM_frequency_mapping)

# Drop rows without a frequency mapping
integrated_df = integrated_df.dropna(subset=['RFM_Frequency']).reset_index(drop=True)

# Save target results
df_wave_target = integrated_df.reset_index(drop=True)
# Ensure the results/csvs directory exists
csv_dir = '../results/csvs'
os.makedirs(csv_dir, exist_ok=True)

df_wave_target.to_csv(os.path.join(csv_dir, 'results_fig7.csv'), index=False)