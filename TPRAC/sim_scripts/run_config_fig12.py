import itertools
import os

SECONDS_IN_MINUTE = 60

# Slurm username
SLURM_USERNAME = "$USER" 

# Maximum Slurm jobs (default: 500)
MAX_SLURM_JOBS = int(os.getenv('MAX_SLURM_JOBS', 500))

# Delay between submitting Slurm jobs (while job limit is not reached)
SLURM_SUBMIT_DELAY = 0.1 

# Delay between retrying Slurm job submission (when job limit is reached)
SLURM_RETRY_DELAY = 5 * SECONDS_IN_MINUTE 

# Number of threads used for the personal computer runs (default: 40)
PERSONAL_RUN_THREADS = int(os.getenv('PERSONAL_RUN_THREADS', 40))

# Number of instructions the slowest core must execute before the simulation ends
NUM_EXPECTED_INSTS_SPEC = 200_000_000
WARMUP_INSTS_SPEC = 50_000_000

NUM_EXPECTED_INSTS_CLOUD = 100_000_000
WARMUP_INSTS_CLOUD = 25_000_000

NUM_RANKS = 4
NUM_CH = 1
NUM_CORE = 4
########## Processor Configurations
branch_predictor_list = ['hashed_perceptron']
prefetcher_list = ['spp_dev']
replacement_list = ['srrip']

########## Memory Configurations
### List of evaluated RowHammer mitigation mechanisms
mitigation_list = ['TPRAC-TREFper4tREFI', 'TPRAC-TREFper3tREFI', 'TPRAC-TREFper2tREFI', 'TPRAC-TREFpertREFI']

##### RH Thresholds #####
NRH_lists = [1024]

##### PRAC Levels -- # of RFMs per ABO
PRAC_levels = [1]

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