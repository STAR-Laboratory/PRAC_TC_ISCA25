import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import ast
import os

# Use TrueType fonts for PDF and PS outputs
plt.rcParams['pdf.fonttype'] = 42  # TrueType fonts for PDFs
plt.rcParams['ps.fonttype'] = 42  # TrueType fonts for PS files

df=pd.read_csv('../results/csvs/results_fig7.csv')
# Set up the plot
fig, ax = plt.subplots(figsize=(7, 2))
plt.rc('font', size=12)
# Use Seaborn's 'tab10' color palette
sns.set_style("whitegrid")
palette = sns.color_palette("tab10")
colors = {impl: palette[i % len(palette)] for i, impl in enumerate(df['Chip'].unique())}

# Define bar width and x-tick positions
bar_width = 0.2  # Desired smaller bar width
x_ticks = [0.25, 0.5, 0.75, 1, 2, 4, 8]
num_bars = len(df['Chip'].unique())
# Create a mapping for x-tick labels
x_tick_labels = {0.25: '0.25 tREFI', 0.5: '0.5 tREFI', 0.75: '0.75 tREFI', 1: '1 tREFI', 2: '2 tREFI', 4: '4 tREFI', 8: '8 tREFI'}
x_tick_positions = np.linspace(0, len(x_ticks) - 1, len(x_ticks))  # Adjust spacing here
bar_positions = {}

# Calculate bar positions
for idx, tick in enumerate(x_ticks):
    base_x = x_tick_positions[idx]
    bar_positions[tick] = [base_x - (bar_width * num_bars) / 2 + j * bar_width for j in range(num_bars)]

# Plot bars
for tick in x_ticks:
    subset = df[df['RFM_Frequency'] == tick]
    if subset.empty:
        continue  # Skip empty subsets
    for i, chip in enumerate(df['Chip'].unique()):
        chip_subset = subset[subset['Chip'] == chip]
        if not chip_subset.empty:
            value = chip_subset['N_RH'].values[0]
            x_position = bar_positions[tick][i] + bar_width / 2  # Adjust position for centering
            color = colors[chip]
            ax.bar(x_position, value, width=bar_width, color=color, edgecolor='black', label=chip if tick == x_ticks[0] else "")

# Customize legend, labels, and ticks
handles, labels = ax.get_legend_handles_labels()
by_label = dict(zip(labels, handles))  # Remove duplicate labels from the legend
ax.legend(by_label.values(), by_label.keys(), loc='best', ncol=3, fancybox=True, shadow=False, fontsize=12)

# Set xticks to specific num_RFM values and ensure correct spacing
ax.set_xticks(x_tick_positions)
ax.set_xticklabels([x_tick_labels[tick] for tick in x_ticks])  # Use mapped labels
ax.set_xlabel('Timing-Based RFM Intervals (TB-Window)', fontsize=12)
ax.set_ylabel('Maximum Activations to \n a Target Row (T$_{MAX}$)', fontsize=12)

# Set y-scale to log
ax.set_yscale('log')
ax.set_yticks([128, 256, 512, 1024, 2048, 4096])
ax.get_yaxis().set_major_formatter(plt.ScalarFormatter())

# Modify legend
handles, labels = ax.get_legend_handles_labels()
custom_labels = ['16Gb: 64K Rows', '32Gb: 128K Rows', '64Gb: 256K Rows']
ax.legend(handles, custom_labels, loc='best', fontsize=12)

ax.set_xlim(-0.5, len(x_ticks)-0.5)
ax.tick_params(axis='both', which='major', labelsize=12)
## Save plots
plots_dir = '../results/plots'
os.makedirs(plots_dir, exist_ok=True)
fig.savefig(os.path.join(plots_dir, 'Figure7.pdf'), dpi=600, bbox_inches='tight')
print(f"Figure 7 (MAX ACTs to a Target Row as TB-Window Varies (Security Analysis)) Generated")
