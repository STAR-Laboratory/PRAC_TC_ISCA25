import itertools
from calc_rh_parameters import *

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


# # List of evaluated RowHammer mitigation mechanisms
# mitigation_list = ["ABO-Only", "BAT-RFM", "TPRAC-S", "TPRAC-P", "QPRAC-ABO-Only", "QPRAC-BAT-RFM", "QPRAC-TPRAC-S", "QPRAC-TPRAC-P"]
# mitigation_list = ["ABO-Only", "BAT-RFM", "TPRAC-S", "TPRAC-P"]
# mitigation_list = ["ABO-Only", "BAT-RFM", "TPRAC-S"]
# mitigation_list = ["TPRAC-S", "QPRAC-TPRAC-S", "BAT-RFM", "QPRAC-ABO-Only", "QPRAC-BAT-RFM"]
# mitigation_list = ["QPRAC-ABO-Only", "QPRAC-BAT-RFM", 'QPRAC-TPRAC-S']
# mitigation_list = ["TPRAC-P-0.0", "TPRAC-P-0.1",  "TPRAC-P-0.2",  "TPRAC-P-0.3", "TPRAC-P-0.4", "TPRAC-P-0.5",  "TPRAC-P-0.6",  "TPRAC-P-0.7",  "TPRAC-P-0.8",  "TPRAC-P-0.9",  "TPRAC-P-1.0",]
mitigation_list = ["TPRAC-P-1.0",]
# mitigation_list = ["BAT-RFM"]
# List of evaluated RowHammer thresholds
NUM_CHANNELS = [1]
Interface_Speeds = [8000]
##### RH Thresholds #####
# TODO: Update these values with the accurate one later
# NRH_lists = [256, 512, 1024, 2048, 4096]
# NRH_lists = [256, 512]
NRH_lists = [512]
##### PRAC Levels -- # of RFMs per ABO#####
PRAC_levels = [1, 2, 4]
# PRAC_levels = [1]
# PRAC_levels = [2, 4]



params_list = [
    mitigation_list,
    NUM_CHANNELS,
    Interface_Speeds,
    NRH_lists,
    PRAC_levels
]

PARAM_STR_LIST = [
    "mitigation",
    "channels",
    "interface_speeds",
    "NRH",
    "PRAC_level"
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

def add_mitigation(config, mitigation, interface_speed, num_channel, NRH, PRAC_level):
    config['Frontend']['inst_window_depth'] = 352
    ###For PRAC
    config['MemorySystem']['DRAM']['PRAC'] = True
    if mitigation in ['Baseline']:
        config['MemorySystem'][CONTROLLER]['impl'] = 'OPTDRAMController'
    else:
        config['MemorySystem'][CONTROLLER]['impl'] = 'PRACOPTDRAMController'
        config['MemorySystem'][CONTROLLER][SCHEDULER]['impl'] = 'PRACScheduler'
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
    elif mitigation == "ABO-Only":
        NBO = get_abo_only_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 131072,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
    elif mitigation == "BAT-RFM":
        BAT = get_bat_parameters(NRH)
        NBO = get_abo_only_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 131072,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'BATBasedRFM',
                'bat': BAT,
                'rfm_type': 0,
                'enable_early_counter_reset': True
            }
        })
    elif mitigation == "TPRAC-S":
        num_rfm_per_tREFI = get_tprac_s_parameters(NRH)
        NBO = get_abo_only_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 131072,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'num_rfm_per_tREFI': num_rfm_per_tREFI
            }
        })
    elif mitigation == "QPRAC-ABO-Only":
        NBO = get_abo_only_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
    elif mitigation == "QPRAC-BAT-RFM":
        BAT = get_bat_parameters(NRH)
        NBO = get_abo_only_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'BATBasedRFM',
                'bat': BAT,
                'rfm_type': 0,
                'enable_early_counter_reset': True
            }
        })
    elif mitigation == "QPRAC-TPRAC-S":
        num_rfm_per_tREFI = get_tprac_s_parameters(NRH)
        NBO = get_abo_only_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'num_rfm_per_tREFI': num_rfm_per_tREFI
            }
        })
    elif mitigation == "TPRAC-P-0.1":
        # num_rfm_per_tREFI = get_tprac_s_parameters(NRH)
        num_rfm_per_tREFI = 0.25
        # NBO = get_abo_only_parameters(NRH)
        NBO = 1000
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TPRAC_P',
                'num_rfm_per_tREFI': num_rfm_per_tREFI,
                'random_rfm_probability': 0.1
            }
        })
    elif mitigation == "TPRAC-P-0.2":
        # num_rfm_per_tREFI = get_tprac_s_parameters(NRH)
        num_rfm_per_tREFI = 0.25
        # NBO = get_abo_only_parameters(NRH)
        NBO = 1000
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TPRAC_P',
                'num_rfm_per_tREFI': num_rfm_per_tREFI,
                'random_rfm_probability': 0.2
            }
        })
    elif mitigation == "TPRAC-P-0.3":
        # num_rfm_per_tREFI = get_tprac_s_parameters(NRH)
        num_rfm_per_tREFI = 0.25
        # NBO = get_abo_only_parameters(NRH)
        NBO = 1000
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TPRAC_P',
                'num_rfm_per_tREFI': num_rfm_per_tREFI,
                'random_rfm_probability': 0.3
            }
        })
    elif mitigation == "TPRAC-P-0.4":
        # num_rfm_per_tREFI = get_tprac_s_parameters(NRH)
        num_rfm_per_tREFI = 0.25
        # NBO = get_abo_only_parameters(NRH)
        NBO = 1000
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TPRAC_P',
                'num_rfm_per_tREFI': num_rfm_per_tREFI,
                'random_rfm_probability': 0.4
            }
        })
    elif mitigation == "TPRAC-P-0.5":
        # num_rfm_per_tREFI = get_tprac_s_parameters(NRH)
        num_rfm_per_tREFI = 0.25
        # NBO = get_abo_only_parameters(NRH)
        NBO = 1000
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TPRAC_P',
                'num_rfm_per_tREFI': num_rfm_per_tREFI,
                'random_rfm_probability': 0.5
            }
        })
    elif mitigation == "TPRAC-P-0.6":
        # num_rfm_per_tREFI = get_tprac_s_parameters(NRH)
        num_rfm_per_tREFI = 0.25
        # NBO = get_abo_only_parameters(NRH)
        NBO = 1000
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TPRAC_P',
                'num_rfm_per_tREFI': num_rfm_per_tREFI,
                'random_rfm_probability': 0.6
            }
        })
    elif mitigation == "TPRAC-P-0.7":
        # num_rfm_per_tREFI = get_tprac_s_parameters(NRH)
        num_rfm_per_tREFI = 0.25
        # NBO = get_abo_only_parameters(NRH)
        NBO = 1000
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TPRAC_P',
                'num_rfm_per_tREFI': num_rfm_per_tREFI,
                'random_rfm_probability': 0.7
            }
        })
    elif mitigation == "TPRAC-P-0.8":
        # num_rfm_per_tREFI = get_tprac_s_parameters(NRH)
        num_rfm_per_tREFI = 0.25
        # NBO = get_abo_only_parameters(NRH)
        NBO = 1000
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TPRAC_P',
                'num_rfm_per_tREFI': num_rfm_per_tREFI,
                'random_rfm_probability': 0.8
            }
        })
    elif mitigation == "TPRAC-P-0.9":
        # num_rfm_per_tREFI = get_tprac_s_parameters(NRH)
        num_rfm_per_tREFI = 0.25
        # NBO = get_abo_only_parameters(NRH)
        NBO = 1000
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TPRAC_P',
                'num_rfm_per_tREFI': num_rfm_per_tREFI,
                'random_rfm_probability': 0.9
            }
        })
    elif mitigation == "TPRAC-P-1.0":
        # num_rfm_per_tREFI = get_tprac_s_parameters(NRH)
        num_rfm_per_tREFI = 0.25
        # NBO = get_abo_only_parameters(NRH)
        NBO = 1000
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TPRAC_P',
                'num_rfm_per_tREFI': num_rfm_per_tREFI,
                'random_rfm_probability': 1.0
            }
        })


if __name__ == "__main__":
    multicore_params = get_multicore_params_list()
    print(multicore_params)