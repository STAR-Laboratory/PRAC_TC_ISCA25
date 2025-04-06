import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
import numpy as np
import os

NUM_TESTS = 256
VICTIM_CUTOFF   = 300
ATTACKER_CUTOFF = 310

victim_count   = np.array([[0.0] * 16 for _ in range(NUM_TESTS)])
attacker_count = np.array([[0.0] * 16 for _ in range(NUM_TESTS)])

base_row = 21845

def dram_cycle_to_ns(dram_cycle):
    return int(dram_cycle * 0.625)

def read_file(filename, i, base_row=base_row):
    if not os.path.exists(filename):
        print(f"File {filename} does not exist. Skipping.")
        return
    
    with open(filename, "r") as file:
        for line in file:
            line = line.strip('\n').split(", ")

            if line[1] == "RFMab":
                break
            elif line[1] != "ACT":
                continue

            cacheline = (int(line[3]) << 5) | (int(line[4]) << 2) | int(line[5])
            row = int(line[6])
            if cacheline == 0 or cacheline > 16 or row != base_row:
                continue
            
            time = dram_cycle_to_ns(int(line[0])) // 1000
            if time < VICTIM_CUTOFF:
                victim_count[i][cacheline - 1] += 1
            elif time > ATTACKER_CUTOFF:
                attacker_count[i][cacheline - 1] += 1

for i in range(NUM_TESTS):
    for j in range(16):
        if j * 16 < i and i < (j+1) * 16:
            continue
        attacker_count[i][j] = np.nan

for i in range(NUM_TESTS):
    filename = f"../../results/stats/AES_no_defense/{i}.ch0"
    read_file(filename, i)

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(6.5,3), gridspec_kw={'width_ratios': [1, 1]})

vmin = min(np.min(np.array(victim_count)), np.min(np.array(attacker_count)))
vmax = max(np.max(np.array(victim_count)), np.max(np.array(attacker_count)))

cax1 = ax1.imshow(victim_count.transpose(), cmap='cool', interpolation='nearest', aspect='auto', vmin=vmin, vmax=vmax)
ax1.set_title("Victim Activations")
ax1.set_xlabel(r"Value of Secret Key Byte 0 ($k_0$)")
ax1.set_ylabel("Row")
ax1.set_aspect("auto")
# fig.colorbar(cax1, ax=ax1, orientation='vertical')

cax2 = ax2.imshow(attacker_count.transpose(), cmap='cool', interpolation='nearest', aspect='auto', vmin=vmin, vmax=vmax)
ax2.set_title("Attacker Activations\nto Row Causing ABO")
ax2.set_xlabel(r"Value of Secret Key Byte 0 ($k_0$)")
ax2.set_ylabel("Row")
ax2.set_aspect("auto")

ax1.yaxis.set_major_locator(MaxNLocator(integer=True))
ax2.yaxis.set_major_locator(MaxNLocator(integer=True))

ticks = np.arange(0, 257, 16)
labels = [str(tick) if tick % 64 == 0 else "" for tick in ticks]
ax1.set_xticks(ticks)
ax1.set_xticklabels(labels)
ax2.set_xticks(ticks)
ax2.set_xticklabels(labels)

plt.tight_layout()
plt.subplots_adjust(wspace=0.3)

fig.colorbar(cax2, ax=[ax1,ax2], orientation='vertical')

# plt.savefig('heatmap_defense.png', dpi=300, bbox_inches='tight')
plt.savefig('../../results/plots/Figure5.pdf', bbox_inches='tight')
print("Figure 5 generated.")


