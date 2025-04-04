import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

def to_ns(cpu_cycle):
    dram_cycle = cpu_cycle * 3 / 8
    return int(dram_cycle * 0.625)

def read_data(filename, lst_x, lst_y):
    with open(filename, "r") as file:
        for line in file:
            line = line.strip('\n').split(" ")
            lst_x.append(to_ns(int(line[0])))
            lst_y.append(to_ns(int(line[1])))
            if lst_y[-1] > 450:
                print(lst_x[-1], lst_y[-1])


def plot_scatter(ax, lst_x, lst_y):
    ax.scatter(lst_x, lst_y, s=8)
    ax.set_ylabel('Mem Access\nLatency (ns)')
    ax.axhline(y=430, color='r', linestyle=':', label="430 ns")

    formatter = ticker.FuncFormatter(lambda x, _: f"{int(x/1000)}K")
    ax.xaxis.set_major_formatter(formatter)

file1 = "../../results/stats/latency/4_rfm.out"
file2 = "../../results/stats/latency/2_rfm.out"
file3 = "../../results/stats/latency/1_rfm.out"
file4 = "../../results/stats/latency/0_rfm.out"

x1, x2, x3, x4 = [], [], [], []
y1, y2, y3, y4 = [], [], [], []
read_data(file1, x1, y1)
read_data(file2, x2, y2)
read_data(file3, x3, y3)
read_data(file4, x4, y4)

# fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 6))
fig, (ax1, ax2, ax3, ax4) = plt.subplots(4, 1, figsize=(6, 6), gridspec_kw={'height_ratios': [1, 1, 1, 1]}, sharey=True)
ax1.set_ylim(0, 2000)
plot_scatter(ax1, x1, y1)
plot_scatter(ax2, x2, y2)
plot_scatter(ax3, x3, y3)
plot_scatter(ax4, x4, y4)

# Annonate
ax1.annotate(
    "ABO",
    xy=(339587, 1777),
    xytext=(40, -15),
    textcoords='offset points',
    arrowprops=dict(arrowstyle="->", color="red", lw=2),
    color="red",
    fontsize=12,
    fontweight='bold'
)
ax1.annotate(
    "",
    xy=(566013, 1479),
    xytext=(-35, -5),
    textcoords='offset points',
    arrowprops=dict(arrowstyle="->", color="red", lw=2)
)

ax2.annotate(
    "ABO",
    xy=(338886, 1077),
    xytext=(40, -5),
    textcoords='offset points',
    arrowprops=dict(arrowstyle="->", color="red", lw=2),
    color="red",
    fontsize=12,
    fontweight='bold'
)
ax2.annotate(
    "",
    xy=(568288, 1072),
    xytext=(-37, 0),
    textcoords='offset points',
    arrowprops=dict(arrowstyle="->", color="red", lw=2)
)

ax3.annotate(
    "ABO",
    xy=(338242, 479),
    xytext=(40, 10),
    textcoords='offset points',
    arrowprops=dict(arrowstyle="->", color="red", lw=2),
    color="red",
    fontsize=12,
    fontweight='bold'
)
ax3.annotate(
    "",
    xy=(563813, 501),
    xytext=(-37, 10),
    textcoords='offset points',
    arrowprops=dict(arrowstyle="->", color="red", lw=2)
)

ax4.set_xlabel('Time (ns)')

ax1.set_title('4 RFMs per ABO', fontweight='bold')
ax2.set_title('2 RFMs per ABO', fontweight='bold')
ax3.set_title('1 RFM per ABO', fontweight='bold')
ax4.set_title('No ABO', fontweight='bold')

plt.subplots_adjust(hspace=0.9)
plt.savefig('../../results/plots/Figure3.pdf', bbox_inches='tight')
print("Figure 3 generated.")