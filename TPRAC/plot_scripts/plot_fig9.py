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

df_ws_NRH_1024 = pd.read_csv('../results/csvs/results_fig9.csv')
df_ws_NRH_1024 = df_ws_NRH_1024[(df_ws_NRH_1024['NRH'] == 1024) & (df_ws_NRH_1024['Prac Level'] == 1)]

mitigation_interest = ['ABO_Only', 'ABO_RFM', 'TPRAC']

df_melted = pd.melt(
    df_ws_NRH_1024, 
    id_vars=['workload', 'NRH', 'Prac Level'], 
    value_vars=mitigation_interest, 
    var_name='Mitigations', 
    value_name='WS'
)
### For renaming mitigations
rename_mapping = {
    'ABO_Only': 'ABO-Only',
    'ABO_RFM': 'ABO+ACB-RFM',
}
# Replace the values in the PRAC_Implementation column
df_melted['Mitigations'] = df_melted['Mitigations'].replace(rename_mapping)

# Filter the data for high MPKI workloads
workloads_high_mpki = [
    '410.bwaves', '429.mcf', '433.milc', '436.cactusADM', '450.soplex', '459.GemsFDTD', '470.lbm', '471.omnetpp', '473.astar', '482.sphinx3', '483.xalancbmk', '--', # SPEC2K6: 12
    '605.mcf', '607.cactuBSSN', '619.lbm', '620.omnetpp', '627.cam4', '628.pop2',  '649.fotonik3d', '654.roms', '---', #SPEC2K17: 8
     'nutch', 'cloud9', 'classification', 'cassandra', '-', 
     'SPEC2K6 (26)', 'SPEC2K17 (20)', 'CloudSuite (4)', 'All (50)'
]

# Filter the rows in df_melted that match the specified workloads
df_high_mpki = df_melted[df_melted['workload'].isin(workloads_high_mpki)]

# Set here again if we have changed the name
mitigation_interest = ["ABO-Only", 'ABO+ACB-RFM', 'TPRAC']
df_filtered = df_high_mpki[df_high_mpki['Mitigations'].isin(mitigation_interest)]
df_filtered['Mitigations'] = pd.Categorical(df_filtered['Mitigations'], categories=mitigation_interest, ordered=True)

#### Prepare plotting
# Sort workloads by Benchmark_Types first to ensure the correct order
sns.set_palette('tab10')
sns.set_style("whitegrid")

# Set the global font family
fig, ax = plt.subplots(figsize=(12,2))
plt.rc('font', size=10)

xtick_order = workloads_high_mpki
ax = sns.barplot(x='workload', y='WS',hue='Mitigations', order=xtick_order, data=df_filtered, edgecolor='black')


ax.set_xticks(np.arange(len(xtick_order)))
ax.set_xticklabels([w if w not in ['-', '--', '---'] else '' for w in xtick_order], ha='right', rotation=45, fontsize=11)

# Get the positions of the x-ticks
tick_labels = ax.get_xticklabels()

# Find the positions of geomean labels
geomean_labels = ['CloudSuite (4)', 'SPEC2K6 (26)', 'SPEC2K17 (20)',  'All (50)']
for tick_label in tick_labels:
    if tick_label.get_text() in geomean_labels:
        tick_label.set_fontweight('bold')

ax.axhline(y=1.0, color='r', linestyle='-', linewidth=2)
ax.axvline(11, 0, 1, color='red', linestyle = '--', linewidth=2)
ax.axvline(20, 0, 1, color='red', linestyle = '--', linewidth=2)
ax.axvline(25, 0, 1, color='red', linestyle = '--', linewidth=2)

ax.text(3.5, 1.01, 'SPEC2K6', fontweight='bold', fontsize=11)
ax.text(14, 1.01, 'SPEC2K17', fontweight='bold', fontsize=11)
ax.text(21, 1.01, 'CloudSuite', fontweight='bold', fontsize=11)
ax.text(26.5, 1.01, 'GMEAN', fontweight='bold', fontsize=11)
ax.set_yticks([0.8, 0.85, 0.9, 0.95, 1.0])
ax.set_xlabel('')
ax.set_ylabel('Normalized Performance', fontsize=12)
ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.25), ncol=5, fancybox=True, shadow=False, fontsize=11)
ax.tick_params(axis='x', which='major', labelsize=11)
ax.tick_params(axis='y', which='major', labelsize=11)

ax.set_ylim(0.8,1.04)
ax.set_xlim(-0.8, 29.8)
# Draw an oval around the starting y-axis (0.8)
ellipse = patches.Ellipse((-1.68, 0.8), width=1.5, height=0.05, edgecolor='red', fill=False, clip_on=False, facecolor='none', linewidth=1.5)
ax.add_patch(ellipse)

plt.grid(True, linestyle=':')
plots_dir = '../results/plots'
os.makedirs(plots_dir, exist_ok=True)
fig.savefig(os.path.join(plots_dir, 'Figure9.pdf'), dpi=600, bbox_inches='tight')
print(f"Figure 9 (Main Performance Results) Generated")
