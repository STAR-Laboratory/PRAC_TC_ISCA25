import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
from matplotlib.colors import LinearSegmentedColormap
import numpy as np

NUM_TESTS = 256
VICTIM_CUTOFF   = 300
ATTACKER_CUTOFF = 310

victim_count   = np.array([[0.0] * 16 for _ in range(NUM_TESTS)])
attacker_count = np.array([[0.0] * 16 for _ in range(NUM_TESTS)])

base_row = 21845

def dram_cycle_to_ns(dram_cycle):
    return int(dram_cycle * 0.625)

def read_file(filename, i, base_row=base_row, verbose=False):
    with open(filename, "r") as file:

        ACT_flag = False
        last_cacheline = 0

        for line in file:
            line = line.strip('\n').split(", ")

            if len(line) < 2 or (line[1] != 'ACT' and line[1] != 'RFMab'):
                continue

            if line[1] == 'ACT':
                cacheline = (int(line[3]) << 5) | (int(line[4]) << 2) | int(line[5])
                row = int(line[6])
                if cacheline == 0 or cacheline > 16 or row != base_row:
                    continue
                else:
                    ACT_flag = True
                    last_cacheline = cacheline

            time = dram_cycle_to_ns(int(line[0])) // 1000
            if time < VICTIM_CUTOFF and line[1] == 'ACT':
                victim_count[i][last_cacheline - 1] += 1
                    
            elif time > ATTACKER_CUTOFF:
                if line[1] == "RFMab":
                    if verbose: print(i, last_cacheline - 1, ACT_flag)
                    if ACT_flag:
                        ACT_flag = False
                        attacker_count[i][last_cacheline - 1] += 1
                        break
                        

# plot right figure
for i in range(NUM_TESTS):
    filename = f"../../results/stats/AES_with_defense/{i}.ch0"
    read_file(filename, i)

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(6, 3.5), gridspec_kw={'width_ratios': [1, 1]})
white_to_blue = LinearSegmentedColormap.from_list("white_to_blue", ["white", "darkturquoise"])

cax2 = ax2.imshow(attacker_count.transpose(), cmap='Blues', interpolation='nearest', aspect='auto')
ax2.set_title("Row Triggering First ABO\nfor Attacker\n(with defense)")
ax2.set_xlabel(r"Value of Secret Key Byte 0 ($k_0$)")
ax2.set_ylabel("Row")
ax2.set_aspect("auto")

# plot left figure
victim_count   = np.array([[0.0] * 16 for _ in range(NUM_TESTS)])
attacker_count = np.array([[0.0] * 16 for _ in range(NUM_TESTS)])

for i in range(NUM_TESTS):
    filename = f"../../results/stats/AES_no_defense/{i}.ch0"
    read_file(filename, i)

cax1 = ax1.imshow(attacker_count.transpose(), cmap='Blues', interpolation='nearest', aspect='auto')
ax1.set_title("Row Triggering First ABO\nfor Attacker\n(without defense)")
ax1.set_xlabel(r"Value of Secret Key Byte 0 ($k_0$)")
ax1.set_ylabel("Row")
ax1.set_aspect("auto")

# Configure common attributes
ax1.yaxis.set_major_locator(MaxNLocator(integer=True))
ax2.yaxis.set_major_locator(MaxNLocator(integer=True))

ticks = np.arange(0, 257, 16)
labels = [str(tick) if tick % 64 == 0 else "" for tick in ticks]
ax1.set_xticks(ticks)
ax1.set_xticklabels(labels)
ax2.set_xticks(ticks)
ax2.set_xticklabels(labels)

fig.text(0.28, 0.01, "(a)", fontsize=12, ha='center')
fig.text(0.78, 0.01, "(b)", fontsize=12, ha='center')

plt.tight_layout()
plt.subplots_adjust(wspace=0.3)

# plt.savefig('heatmap_defense.pdf', dpi=300, bbox_inches='tight')
plt.savefig('../../results/plots/Figure13.pdf', bbox_inches='tight')
print("Figure 13 generated.")



