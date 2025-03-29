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
SLURM_RETRY_DELAY = 5 * SECONDS_IN_MINUTE 

# Number of threads used for the personal computer runs
PERSONAL_RUN_THREADS = 80

# Number of instructions the slowest core must execute before the simulation ends
NUM_EXPECTED_INSTS_LOW = 200_000_000
WARMUP_INSTS_LOW = 50_000_000

NUM_EXPECTED_INSTS_HIGH = 100_000_000
WARMUP_INSTS_HIGH = 25_000_000
### For test
# NUM_EXPECTED_INSTS_LOW = 1000
# WARMUP_INSTS_LOW = 50_000
# NUM_EXPECTED_INSTS_HIGH = 1000
# WARMUP_INSTS_HIGH = 50_000

NUM_RANKS = 4
NUM_CH = 1
NUM_CORE = 4
########## Processor Configurations
branch_predictor_list = ['hashed_perceptron']
# branch_predictor_list = ['bimodal', 'gshare', 'perceptron', 'hashed_perceptron']
prefetcher_list = ['spp_dev']
# prefetcher_list =   ['next_line', 'no', 'va_ampm_lite', 'spp_dev']
replacement_list = ['srrip']

########## Memory Configurations
### List of evaluated RowHammer mitigation mechanisms
# mitigation_list = ['Baseline', 'TPRAC',  'ABO_Only', 'ABO_RFM', 'TPRAC-TREFpertREFI', 'TPRAC-TREFper4tREFI', 'TPRAC-TREFper3tREFI', 'TPRAC-TREFper2tREFI'  ] 
# mitigation_list = ['TPRAC-TREFper4tREFI', 'TPRAC-TREFper3tREFI', 'TPRAC-TREFper2tREFI', 'TPRAC-TREFpertREFI']
# mitigation_list = ['Baseline', 'ABO_Only', 'ABO_RFM', 'TPRAC']
mitigation_list = ['TPRAC-TREFper4tREFI']
# mitigation_list = ['Baseline']

##### RH Thresholds #####
# NRH_lists = [128, 256, 512, 1024, 2048, 4096]
NRH_lists = [256]

##### PRAC Levels -- # of RFMs per ABO
# PRAC_levels = [1, 2, 4]
PRAC_levels = [1]
# PRAC_levels = [2, 4]
# PRAC_levels = [2]
# PRAC_levels = [4]



params_list = [
    mitigation_list,
    branch_predictor_list,
    prefetcher_list,
    replacement_list,
    NRH_lists,
    PRAC_levels
]

PARAM_STR_LIST = [ 
    "mitigation",  
    "branch_predictor",
    "prefetcher",
    "replacement",
    "NRH",
    "PRAC_level"
]

def get_multicore_params_list():
    params = list(itertools.product(*params_list))
    return params

def make_stat_str(param_list, delim="_"):
    return delim.join([str(param) for param in param_list])

if __name__ == "__main__":
    multicore_params = get_multicore_params_list()
    print(multicore_params)