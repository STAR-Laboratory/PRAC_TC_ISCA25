import itertools

SECONDS_IN_MINUTE = 60

# Slurm username
SLURM_USERNAME = "$USER" 

# Maximum Slurm jobs
MAX_SLURM_JOBS = 1000 

# Delay between submitting Slurm jobs (while job limit is not reached)
SLURM_SUBMIT_DELAY = 0.1 

# Delay between retrying Slurm job submission (when job limit is reached)
SLURM_RETRY_DELAY = 1 * SECONDS_IN_MINUTE 

# Number of threads used for the personal computer runs
PERSONAL_RUN_THREADS = 80

# Memory histogram precision
MEM_HIST_PREC = 5

# Number of instructions the slowest core must execute before the simulation ends
NUM_EXPECTED_INSTS = 500_000_000
# NUM_EXPECTED_INSTS = 5_000

# Number of cycles the simulation should run
NUM_MAX_CYCLES = 0
# NUM_MAX_CYCLES = 5_000_000_000


CONTROLLER = "BHDRAMController"
SCHEDULER = "BHScheduler"
NUM_RANKS = 4

mitigation_list = []
tRH_list = []

# # List of evaluated RowHammer mitigation mechanisms
# mitigation_list = ["Baseline", "PRAC_WO_Mitigation", 
#                    "Baseline-ClosedCap4", "PRAC_WO_Mitigation-ClosedCap4"]
# mitigation_list = ["Baseline", "Baseline-ClosedCap4"]
mitigation_list = ["PRAC_WO_Mitigation-ClosedCap4"]
# mitigation_list = ["Baseline"]

# List of evaluated RowHammer thresholds
NUM_CHANNELS = [1]
# NUM_CHANNELS = [2, 4, 8, 16]
# NUM_CHANNELS = [2, 4]
# Interface_Speeds = [3200, 6400, 8800, 12800, 17600]
# Interface_Speeds = [4800, 8000]
Interface_Speeds = [4800]



params_list = [
    mitigation_list,
    NUM_CHANNELS,
    Interface_Speeds
]

PARAM_STR_LIST = [
    "mitigation",
    "channels",
    "interface_speeds",
]

def get_multicore_params_list():
    params = list(itertools.product(*params_list))
    # for mitigation in mitigation_list:
    #     for tRH in tRH_list:
    #         params.append((mitigation, tRH))
    return params


def get_trace_lists(trace_combination_file):
    trace_comb_line_count = 0
    multicore_trace_list = set()
    singlecore_trace_list = set()
    with open(trace_combination_file, "r") as f:
        for line in f:
            trace_comb_line_count += 1
            line = line.strip()
            tokens = line.split(',')
            trace_name = tokens[0]
            trace_list = tokens[2:]
            for trace in trace_list:
                singlecore_trace_list.add(trace)
            multicore_trace_list.add(trace_name)
    return singlecore_trace_list, multicore_trace_list

def make_stat_str(param_list, delim="_"):
    return delim.join([str(param) for param in param_list])

def add_mitigation(config, mitigation, interface_speed, num_channel):
    config['Frontend']['inst_window_depth'] = 352
    config['MemorySystem'][CONTROLLER]['impl'] = 'OPTDRAMController'
    config['MemorySystem']['DRAM']['org']['rank'] = NUM_RANKS
    config['MemorySystem']['DRAM']['org']['channel'] = num_channel
    if interface_speed == 3200:
        config['MemorySystem']['DRAM']['timing']['preset'] = 'DDR5_3200BN'
        config['MemorySystem']['clock_ratio'] = 4
        config['Frontend']['clock_ratio'] = 10
    if interface_speed == 4800:
        config['MemorySystem']['DRAM']['timing']['preset'] = 'DDR5_4800B'
        config['MemorySystem']['clock_ratio'] = 3
        config['Frontend']['clock_ratio'] = 5
    if interface_speed == 6400:
        config['MemorySystem']['DRAM']['timing']['preset'] = 'DDR5_6400B'
        config['MemorySystem']['clock_ratio'] = 4
        config['Frontend']['clock_ratio'] = 5
    if interface_speed == 8000:
        config['MemorySystem']['DRAM']['timing']['preset'] = 'DDR5_8000B'
        config['MemorySystem']['clock_ratio'] = 1
        config['Frontend']['clock_ratio'] = 1
    if interface_speed == 8800:
        config['MemorySystem']['DRAM']['timing']['preset'] = 'DDR5_8800B'
        config['MemorySystem']['clock_ratio'] = 10
        config['Frontend']['clock_ratio'] = 9
    if interface_speed == 12800:
        config['MemorySystem']['DRAM']['timing']['preset'] = 'DDR5_12800B'
        config['MemorySystem']['clock_ratio'] = 8
        config['Frontend']['clock_ratio'] = 5
    if interface_speed == 17600:
        config['MemorySystem']['DRAM']['timing']['preset'] = 'DDR5_17600B'
        config['MemorySystem']['clock_ratio'] = 11
        config['Frontend']['clock_ratio'] = 5
    if mitigation == "PRAC_WO_Mitigation":
        config['MemorySystem']['DRAM']['PRAC'] = True
    elif mitigation == "Baseline-ClosedCap4":
        config['MemorySystem'][CONTROLLER]['RowPolicy']['impl'] = 'ClosedRowPolicy'
        config["MemorySystem"][CONTROLLER]["RowPolicy"]["cap"] = 4
    elif mitigation == "Baseline-ClosedCap1":
        config['MemorySystem'][CONTROLLER]['RowPolicy']['impl'] = 'ClosedRowPolicy'
        config["MemorySystem"][CONTROLLER]["RowPolicy"]["cap"] = 1
    elif mitigation == "PRAC_WO_Mitigation-ClosedCap4":
        config['MemorySystem']['DRAM']['PRAC'] = True
        config['MemorySystem'][CONTROLLER]['RowPolicy']['impl'] = 'ClosedRowPolicy'
        config["MemorySystem"][CONTROLLER]["RowPolicy"]["cap"] = 4
    elif mitigation == "PRAC_WO_Mitigation-ClosedCap1":
        config['MemorySystem']['DRAM']['PRAC'] = True
        config['MemorySystem'][CONTROLLER]['RowPolicy']['impl'] = 'ClosedRowPolicy'
        config["MemorySystem"][CONTROLLER]["RowPolicy"]["cap"] = 1
    # elif mitigation == "UPRAC":
    #     NBO = get_prac_parameters(tRH)
    #     config['MemorySystem']['DRAM']['PRAC'] = True
    #     config['MemorySystem'][CONTROLLER]['impl'] = 'PRACDRAMController'
    #     config['MemorySystem']['BHDRAMController']['BHScheduler']['impl'] = 'PRACScheduler'
    #     config['MemorySystem']['BHDRAMController']['plugins'].append({
    #         'ControllerPlugin' : {
    #             'impl': 'PRAC', 
    #             'abo_delay_acts': 1, 
    #             'abo_recovery_refs': 1, 
    #             'abo_act_ns': 180, 
    #             'abo_threshold': NBO, 
    #             'enable_queue': 'false', 
    #             'enable_target_ref': 'false', 
    #             'target_ref_ratio': 5
    #             }})
    

   

if __name__ == "__main__":
    multicore_params = get_multicore_params_list()
    print(multicore_params)