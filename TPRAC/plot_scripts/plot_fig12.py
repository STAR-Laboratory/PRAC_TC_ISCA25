import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import warnings
import matplotlib.font_manager as fm
import matplotlib.patches as patches

# Use TrueType fonts for PDF and PS outputs
plt.rcParams['pdf.fonttype'] = 42  # TrueType fonts for PDFs
plt.rcParams['ps.fonttype'] = 42  # TrueType fonts for PS files

df_prac_level_study =  pd.read_csv('../results/csvs/results_fig12.csv')
df_prac_level_study = df_prac_level_study[(df_prac_level_study['NRH'].isin([128, 256, 512, 1024, 2048, 4096])) &
                                          (df_prac_level_study['Prac Level'] == 1) &
                                           (df_prac_level_study['workload'] == 'All (50)')]
mitigation_interest = ['ABO_Only', 'ABO_RFM', 'TPRAC', 'TPRAC-TREFper4tREFI','TPRAC-TREFper2tREFI','TPRAC-TREFpertREFI']

df_melted = pd.melt(
    df_prac_level_study, 
    id_vars=['workload', 'NRH', 'Prac Level'], 
    value_vars=mitigation_interest, 
    var_name='Mitigations', 
    value_name='WS'
)
### For renaming mitigations
rename_mapping = {
    'ABO_Only': 'ABO-Only',
    'ABO_RFM': 'ABO+ACB-RFM',
    'TPRAC-TREFper4tREFI': 'TPRAC w/ 1 Targeted per 4 tREFI',
    'TPRAC-TREFper3tREFI': 'TPRAC w/ 1 Targeted per 3 tREFI',
    'TPRAC-TREFper2tREFI': 'TPRAC w/ 1 Targeted per 2 tREFI',
    'TPRAC-TREFpertREFI': 'TPRAC w/ 1 Targeted per 1 tREFI',
    'TPRAC': 'TPRAC w/o Targeted'
}

# Replace the values in the PRAC_Implementation column
df_melted['Mitigations'] = df_melted['Mitigations'].replace(rename_mapping)
# Calculate performance overhead
df_melted['Performance_Overhead'] = (1 - df_melted['WS'])*100

# Set up the plot
fig, ax = plt.subplots(figsize=(7.5, 2))
plt.rc('font', size=12)
# Use Seaborn's 'tab10' color palette
palette = sns.color_palette("tab10")
colors = {impl: palette[i % len(palette)] for i, impl in enumerate(df_melted['Mitigations'].unique())}


# Define bar width and x-tick positions
bar_width = 0.1  # Desired smaller bar width
x_ticks = [128, 256, 512, 1024, 2048, 4096]
num_bars = len(df_melted['Mitigations'].unique())

x_tick_positions = np.linspace(0, len(x_ticks) - 1, len(x_ticks))  # Adjust spacing here
bar_positions = {}

# Calculate bar positions
for idx, tick in enumerate(x_ticks):
    base_x = x_tick_positions[idx]
    bar_positions[tick] = [base_x - (bar_width * num_bars) / 2 + j * bar_width for j in range(num_bars)]

# Plot bars
for tick in x_ticks:
    subset = df_melted[df_melted['NRH'] == tick]
    for i, prac_impl in enumerate(df_melted['Mitigations'].unique()):
        value = subset[subset['Mitigations'] == prac_impl]['WS'].values[0]
        x_position = bar_positions[tick][i] + bar_width / 2  # Adjust position for centering
        color = colors[prac_impl]
        ax.bar(x_position, value, width=bar_width, color=color, edgecolor='black', label=prac_impl if tick == x_ticks[0] else "")


# Add vertical lines for better visual separation
trh_values = np.sort(df_melted['NRH'].unique())
for i in range(len(trh_values) - 1):
    ax.axvline(x=i + 0.5, color='grey', linestyle='-', alpha=0.5)

# Customize legend, labels, and ticks
handles, labels = ax.get_legend_handles_labels()
by_label = dict(zip(labels, handles))  # Remove duplicate labels from the legend

# Set xticks to specific num_RFM values and ensure correct spacing
ax.set_xticks(x_tick_positions)
ax.set_xticklabels(x_ticks)

ax.set_xlabel('RowHammer Threshold (N$_{RH}$)', fontsize=12)
ax.set_ylabel('Normalized Performance', fontsize=12)
ax.tick_params(axis='both', which='major', labelsize=12)
ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.55), ncol=2, fancybox=True, shadow=False, fontsize=11)

ax.axhline(y=1.0, color='r', linestyle='-', linewidth=2)

ax.set_ylim(0.6,1.008)
ax.set_yticks([0.6, 0.7, 0.8, 0.9, 1])
# ax.set_xlim(-0.5, 2.5)
# Draw an oval around the starting y-axis (0.8)
ellipse = patches.Ellipse((-0.81, 0.6), width=0.4, height=0.07, edgecolor='red', fill=False, clip_on=False, facecolor='none', linewidth=1.5)
ax.add_patch(ellipse)

plots_dir = '../results/plots'
os.makedirs(plots_dir, exist_ok=True)
fig.savefig(os.path.join(plots_dir, 'Figure12.pdf'), dpi=600, bbox_inches='tight')
print(f"Figure 12 (Sensitivity to Rowhammer Threshold) Generated")
