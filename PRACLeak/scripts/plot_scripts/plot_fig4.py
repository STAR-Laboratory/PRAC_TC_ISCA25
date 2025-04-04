import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

def prefix_sum(lst):
    for i in range(1, len(lst)):
        lst[i] += lst[i - 1]
    return lst

def dram_cycle_to_ns(dram_cycle):
    return int(dram_cycle * 0.625)

def cpu_cycle_to_ns(cpu_cycle):
    dram_cycle = cpu_cycle * 3 / 8
    return int(dram_cycle * 0.625)

def read_data(filename, lst_x, lst_y):
    with open(filename, "r") as file:
        for line in file:
            line = line.strip('\n').split(" ")
            lst_x.append(cpu_cycle_to_ns(int(line[0])))
            lst_y.append(cpu_cycle_to_ns(int(line[1])))
            if lst_y[-1] > 500:
                print(lst_x[-1], lst_y[-1], line)

def plot_scatter(ax, lst_x, lst_y):
    ax.scatter(lst_x, lst_y, s=10)
    # ax.set_xlabel('Time (ns)')
    ax.set_ylabel('Mem Access\nLatency (ns)')

    formatter = ticker.FuncFormatter(lambda x, _: f"{int(x/1000)}K")
    ax.xaxis.set_major_formatter(formatter)

NUM_CACHELINE = 16
SIZE_CACHELINE = 64 #64 bytes * 8

base_addr = 5726634048
base_row = 21845

controller_file = "../../results/stats/side_channel/0.ch0"
latency_file = "../../results/stats/side_channel/latency_0.out"

with open(controller_file, "r") as file:
    lines = file.readlines()

MAX_TIME = dram_cycle_to_ns(int(lines[-1].split(", ")[0])) // 1000 + 1
print(MAX_TIME)
counts = [[0] * MAX_TIME for _ in range(NUM_CACHELINE)]
# RFMcounts = [0] * MAX_TIME
time_list = [i for i in range(MAX_TIME)]
rfmx, rfmy = [], []

for i in range(len(lines) - 1):
    line = lines[i].split(", ")

    time = dram_cycle_to_ns(int(line[0])) // 1000

    if line[1] == "RFMab":
        # RFMcounts[time] += 1
        rfmx.append(time)
        rfmy.append(1)
        continue
    elif line[1] != "ACT":
        continue
    
    cacheline = (int(line[3]) << 5) | (int(line[4]) << 2) | int(line[5])
    row = int(line[6])
    if cacheline == 0 or cacheline > 16 or row != base_row:
        continue

    # addr = int(line[-1])
    # if (addr < base_addr) or (addr >= base_addr + 1024):
    #     print(i, addr)
    #     continue

    # counts[(addr - base_addr) // SIZE_CACHELINE][time] += 1
    counts[cacheline - 1][time] += 1

# Create the plot
fig, (ax0, ax1, ax2) = plt.subplots(3, 1, figsize=(6,6), gridspec_kw={'height_ratios': [0.8, 0.5, 1]})

ax1.sharex(ax2)
# Plot Latency
x1, y1 = [], []
read_data(latency_file, x1, y1)
plot_scatter(ax0, x1, y1)
ax0.set_ylim(0, 2000)
ax1.set_ylim(0, 1.5)

# Plot RFM counts
# ax1.plot(time_list, prefix_sum(RFMcounts), 'g', label='RFM')
ax1.stem(rfmx, rfmy)

# Plot ACT counts
for i in range(1, NUM_CACHELINE):
    ax2.plot(time_list, prefix_sum(counts[i]), label="Cacheline"+str(i))
ax2.plot(time_list, prefix_sum(counts[0]), color="darkblue", label="Cacheline"+str(0))

left_cutoff = 289
right_cutoff = 325
ax2.axvspan(0, left_cutoff, color="khaki", alpha=0.5, label="Left region")
# ax2.axvspan(left_cutoff, right_cutoff, color="white", alpha=0.5, label="Middle region")
ax2.axvspan(right_cutoff, max(time_list), color="lightcoral", alpha=0.5, label="Right region")

formatter = ticker.FuncFormatter(lambda x, _: f"{int(x)}K")
ax1.xaxis.set_major_formatter(formatter)
ax2.xaxis.set_major_formatter(formatter)

# ax1.set_xlabel('Time (ns)')
ax1.set_ylabel('RFM Count')
ax2.set_xlabel('Time (ns)')
ax2.set_ylabel('Activation Count')
ax2.set_title("")

# Annotations
plt.figtext(0.14, 0.3, r"$\text{N}_\text{BO}$ = 256", fontsize=12, color="red")
plt.figtext(0.17, 0.37, "Victim Activations", fontsize=12, color="black")
plt.figtext(0.50, 0.37, "Attacker Activations", fontsize=12, color="black")

plt.figtext(0.7, 0.29, "Row 0", fontsize=12, fontweight='bold', color="darkblue")
plt.figtext(0.7, 0.14, "Other rows", fontsize=12, fontweight='bold', color="#1f77b4")

max_y, cor_x = max(zip(y1, x1))
ax0.annotate(
    "ABO",
    xy=(cor_x, max_y),
    xytext=(-50, -5),
    textcoords='offset points',
    arrowprops=dict(arrowstyle="->", color="red", lw=2),
    color="red",
    fontsize=12,
    # fontweight='bold'
)
ax2.axhline(y=256, color='r', linestyle=':')

# ax2.legend(loc="upper left", fontsize=5)

# ax1.grid(True)
# ax2.grid(True)

# plt.legend()
plt.subplots_adjust(hspace=0.5)
plt.savefig('../../results/plots/Figure4.pdf', bbox_inches='tight')
print("Figure 4 generated.")