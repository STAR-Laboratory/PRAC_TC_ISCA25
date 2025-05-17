import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import os

# Font setup for PDF/PS outputs
plt.rcParams['pdf.fonttype'] = 42
plt.rcParams['ps.fonttype'] = 42

# Load the data
df = pd.read_csv('../results/csvs/results_fig7.csv')

df = df[df['RFM_Frequency'].isin([0.25, 0.5, 0.75, 1, 2, 4])]

sns.set_style("whitegrid")
fig, ax = plt.subplots(figsize=(7, 2))
plt.rc('font', size=12)

# Set colors for 'reset' types
palette = sns.color_palette("tab10")
reset_labels = ['with_reset', 'no_reset']
display_labels = {'with_reset': 'With Activation Counter Reset', 'no_reset': 'Without Activation Counter Reset'}
colors = {label: palette[i] for i, label in enumerate(reset_labels)}

# RFM frequencies and bar setup
x_ticks = [0.25, 0.5, 0.75, 1, 2, 4]
x_tick_labels = {0.25: '0.25 tREFI', 0.5: '0.5 tREFI', 0.75: '0.75 tREFI', 1: '1 tREFI', 2: '2 tREFI', 4: '4 tREFI'}
x_tick_positions = np.linspace(0, len(x_ticks) - 1, len(x_ticks))
bar_width = 0.25
num_bars = len(reset_labels)

# Compute bar positions
bar_positions = {
    tick: [x_tick_positions[i] - (bar_width * num_bars) / 2 + j * bar_width for j in range(num_bars)]
    for i, tick in enumerate(x_ticks)
}

# Plot bars
for tick in x_ticks:
    subset = df[df['RFM_Frequency'] == tick]
    if subset.empty:
        continue
    for i, reset_type in enumerate(reset_labels):
        val = subset[subset['reset'] == reset_type]
        if not val.empty:
            value = val['N_RH'].values[0]
            x_pos = bar_positions[tick][i] + bar_width / 2
            ax.bar(x_pos, value, width=bar_width, color=colors[reset_type],
                   edgecolor='black', label=display_labels[reset_type] if tick == x_ticks[0] else "")

# Configure axis and labels
ax.set_xticks(x_tick_positions)
ax.set_xticklabels([x_tick_labels[t] for t in x_ticks])
ax.set_xlabel('Timing-Based RFM Intervals (TB-Window)', fontsize=12)
ax.set_ylabel('Maximum Activations to \n a Target Row (T$_{MAX}$)', fontsize=11)

# Log-scale y-axis
ax.set_yscale('log')
ax.set_yticks([128, 256, 512, 1024, 2048, 4096])
ax.get_yaxis().set_major_formatter(plt.ScalarFormatter())

# Legend
handles, labels = ax.get_legend_handles_labels()
by_label = dict(zip(labels, handles))
ax.legend(by_label.values(), by_label.keys(), loc='best', fontsize=11)

# Layout tweaks
ax.set_xlim(-0.5, len(x_ticks)-0.5)
ax.tick_params(axis='both', which='major', labelsize=11)
plt.grid(True, linestyle=':')

# Save figure
plots_dir = '../results/plots'
os.makedirs(plots_dir, exist_ok=True)
fig.savefig(os.path.join(plots_dir, 'Figure7.pdf'), dpi=600, bbox_inches='tight')
print("Figure 7 Generated")