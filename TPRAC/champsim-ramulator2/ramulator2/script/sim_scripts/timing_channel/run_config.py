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
# NUM_EXPECTED_INSTS = 250_000_000
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
mitigation_list = ["1RFM-8tREFI"]
# mitigation_list = ["Baseline", "1RFM-4tREFI", "1RFM-2tREFI" , "1RFM-1tREFI", "2RFM-1tREFI", "4RFM-1tREFI"]
# mitigation_list = ["Baseline"]
# List of evaluated RowHammer thresholds
NUM_CHANNELS = [1]
Interface_Speeds = [8000]



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
    ###For PRAC
    config['MemorySystem']['DRAM']['PRAC'] = True
    # if mitigation in ['Baseline']:
    #     config['MemorySystem'][CONTROLLER]['impl'] = 'OPTDRAMController'
    # else:
    #     config['MemorySystem'][CONTROLLER]['impl'] = 'PRACOPTDRAMController'
    #     config['MemorySystem'][CONTROLLER][SCHEDULER]['impl'] = 'PRACScheduler'
    config['MemorySystem']['DRAM']['org']['rank'] = NUM_RANKS
    config['MemorySystem']['DRAM']['org']['channel'] = num_channel
    config['MemorySystem'][CONTROLLER]['RowPolicy']['impl'] = 'ClosedRowPolicy'
    config["MemorySystem"][CONTROLLER]["RowPolicy"]["cap"] = 4
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
    elif mitigation == "1RFM-8tREFI":
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'num_rfm_per_tREFI': 0.125
            }
        })
    elif mitigation == "1RFM-4tREFI":
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'num_rfm_per_tREFI': 0.25
            }
        })
    elif mitigation == "1RFM-2tREFI":
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'num_rfm_per_tREFI': 0.50
            }
        })
    elif mitigation == "1RFM-1tREFI":
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'num_rfm_per_tREFI': 1
            }
        })   
    elif mitigation == "2RFM-1tREFI":
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'num_rfm_per_tREFI': 2
            }
        })
    elif mitigation == "3RFM-1tREFI":
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'num_rfm_per_tREFI': 3
            }
        })
    elif mitigation == "4RFM-1tREFI":
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'num_rfm_per_tREFI': 4
            }
        })
    

if __name__ == "__main__":
    multicore_params = get_multicore_params_list()
    print(multicore_params)