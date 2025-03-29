import os
import yaml
import copy
import argparse
import pandas as pd

from run_config import *

argparser = argparse.ArgumentParser(
    prog="RunSlurm",
    description="Run ChampSim with Ramulator2 simulations using Slurm"
)

argparser.add_argument("-csd", "--champsim_directory")
argparser.add_argument("-csbd", "--champsim_bin_directory")
argparser.add_argument("-wd", "--working_directory")
argparser.add_argument("-td", "--trace_directory")
argparser.add_argument("-rd", "--result_directory")
argparser.add_argument("-pn", "--partition_name")
argparser.add_argument("-pdm", "--partition_default_memory")
argparser.add_argument("-pbm", "--partition_big_memory")
argparser.add_argument("-tl", "--trace_list", default="4core_simulations.txt", help="Path to the trace list file")

args = argparser.parse_args()

CHAMPSIM_DIR = args.champsim_directory
CHAMPSIM_BIN_DIR = args.champsim_bin_directory
WORK_DIR = args.working_directory
TRACE_DIR = args.trace_directory
RESULT_DIR = args.result_directory
PARTITION_NAME = args.partition_name
PARTITION_DEF_MEM = args.partition_default_memory
PARTITION_BIG_MEM = args.partition_big_memory
TRACE_LIST_FILE = args.trace_list

SBATCH_CMD = "sbatch --cpus-per-task=1 --nodes=1 --ntasks=1 --time=36:00:00 \
                --mail-user=jhwoo36@ece.ubc.ca --mail-type=FAIL --account=st-prashnr-1"

CMD_HEADER = "#!/bin/bash"

CMD_BASE = f"{CHAMPSIM_BIN_DIR}/champsim"

# Ensure directories exist
for mitigation in mitigation_list:
    for path in [
            f"{RESULT_DIR}/{mitigation}/stats",
            f"{RESULT_DIR}/{mitigation}/errors",
            f"{RESULT_DIR}/{mitigation}/cmd_count"
        ]:
        os.makedirs(path, exist_ok=True)

# Read trace groups from file
with open(TRACE_LIST_FILE, "r") as f:
    trace_groups = [line.strip().split(",") for line in f if line.strip()]

for trace_group in trace_groups:
    # Extract the workload name, remove unnecessary parts from the filename
    trace_names = trace_group[0].split("-")[:2]  # Get the first two parts (e.g., "401.bzip2")
    trace_names = "_".join(trace_names)  # Join them with an underscore

    # Clean up additional extensions
    trace_names = trace_names.split(".champsimtrace.xz")[0]  # Remove ".champsimtrace.xz"
    trace_names = trace_names.split("_core0")[0]  # Remove any "_core0" suffix

high_memory_intensive_workloads = ['410.bwaves_1963B', '429.mcf_22B', '433.milc_274B', '437.leslie3d_134B',
                                   '450.soplex_247B', '462.libquantum_714B', '470.lbm_1274B', '471.omnetpp_188B',
                                    '605.mcf_s_1152B', '619.lbm_s_2676B', '620.omnetpp_s_141B', '649.fotonik3d_s_10881B', '654.roms_s_1007B',
                                    'nutch_phase0', 'nutch_phase1', 'nutch_phase2', 'nutch_phase3', 'nutch_phase4', 'nutch_phase5', 
                                    'cloud9_phase0', 'cloud9_phase1', 'cloud9_phase2', 'cloud9_phase3', 'cloud9_phase4', 'cloud9_phase5', 
                                    'cassandra_phase0', 'cassandra_phase1', 'cassandra_phase2', 'cassandra_phase3', 'cassandra_phase4', 'cassandra_phase5',
                                    'classification_phase0', 'classification_phase1', 'classification_phase2', 'classification_phase3', 'classification_phase4', 'classification_phase5']


# test_workloads = ['649.fotonik3d_s_10881B', '654.roms_s_1007B']
test_workloads = ['433.milc_274B']
# test_workloads = []

def get_multicore_run_commands():
    run_commands = []
    multicore_params = get_multicore_params_list()
    for config in multicore_params:
        mitigation, branch_predictor, prefetcher, replacement, NRH, PRAC_level = config
        if mitigation == "Baseline":
            if NRH != 1024 or PRAC_level != 1:
                continue
        if PRAC_level != 1:
            if NRH != 1024:
                continue
        if mitigation in ['TPRAC-TREFper4tREFI', 'TPRAC-TREFper3tREFI', 'TPRAC-TREFper2tREFI', 'TPRAC-TREFpertREFI']:
            if PRAC_level != 1:
                continue
        ### For main performance results
        if prefetcher == "spp_dev":
            ### For prefecther sensitivity sutdy
            if branch_predictor != "hashed_perceptron":
                if PRAC_level != 1 or NRH != 1024 or mitigation in ['ABO_Only', 'ABO_RFM', 'TPRAC-TREFper3tREFI', 'TPRAC-TREFper2tREFI']:
                    continue
            ### For branch predictor sesntivity study
        else:
            if branch_predictor == "hashed_perceptron":
                if PRAC_level != 1 or NRH != 1024 or mitigation in ['ABO_Only', 'ABO_RFM', 'TPRAC-TREFper3tREFI', 'TPRAC-TREFper2tREFI']:
                    continue
            else:
                continue
        if mitigation == "Baseline":
            stat_str = make_stat_str(config[1:4])
        else:
            stat_str = make_stat_str(config[1:])
        for trace_group in trace_groups:
            # Extract the workload name, remove unnecessary parts from the filename
            trace_names = trace_group[0].split("-")[:2]  # Get the first two parts (e.g., "401.bzip2")
            trace_names = "_".join(trace_names)  # Join them with an underscore
            # Clean up additional extensions
            trace_names = trace_names.split(".champsimtrace.xz")[0]  # Remove ".champsimtrace.xz"
            trace_names = trace_names.split("_core0")[0]  # Remove any "_core0" suffix

            result_filename = f"{RESULT_DIR}/{mitigation}/stats/{stat_str}_{trace_names}.txt"
            error_filename = f"{RESULT_DIR}/{mitigation}/errors/{stat_str}_{trace_names}.txt"
            cmd_count_filename = f"{RESULT_DIR}/{mitigation}/cmd_count/{stat_str}_{trace_names}.cmd.count"

            # Join all traces in the trace group dynamically
            full_traces = " ".join(f"{TRACE_DIR}/{trace}" for trace in trace_group)

            if test_workloads:
                if trace_names not in test_workloads:
                    continue

            if trace_names in high_memory_intensive_workloads:
                NUM_EXPECTED_INSTS = NUM_EXPECTED_INSTS_HIGH
                WARMUP_INSTS = WARMUP_INSTS_HIGH
            else:
                NUM_EXPECTED_INSTS = NUM_EXPECTED_INSTS_LOW
                WARMUP_INSTS = WARMUP_INSTS_LOW
            # Construct the command step by step
            if mitigation == 'Baseline':
                CMD = (
                    f"{CMD_BASE}-{branch_predictor}-{prefetcher}-{replacement}-{mitigation} "
                    f"--warmup-instructions {WARMUP_INSTS} --simulation-instructions {NUM_EXPECTED_INSTS} "
                    f"{full_traces}"
                )
            else:
                CMD = (
                    f"{CMD_BASE}-{branch_predictor}-{prefetcher}-{replacement}-{mitigation}-{NRH}-PRAC{PRAC_level} "
                    f"--warmup-instructions {WARMUP_INSTS} --simulation-instructions {NUM_EXPECTED_INSTS} "
                    f"{full_traces}"
                )

            ### Write the Slurm batch script
            sbatch_filename = f"{WORK_DIR}/run_scripts/{mitigation}_{stat_str}_{trace_names}.sh"
            with open(sbatch_filename, "w") as sbatch_file:
                sbatch_file.write(f"{CMD_HEADER}\nexport RAMULATOR_COMMAND_COUNTER_PATH={cmd_count_filename}\n{CMD}\n")
                # Add post-processing to check for empty error file
                sbatch_file.write(f"\n# Post-processing to check for empty error file\n")
                sbatch_file.write(f"ERROR_FILE={error_filename}\n")
                sbatch_file.write(f"if [ ! -s \"$ERROR_FILE\" ]; then\n")
                sbatch_file.write(f"    rm \"$ERROR_FILE\"\n")
                sbatch_file.write(f"    echo \"Error file is empty, deleting $ERROR_FILE\"\n")
                sbatch_file.write(f"fi\n")
            job_name = f"{mitigation}_{stat_str}_{trace_names}"
            sb_cmd = f"{SBATCH_CMD} --mem={PARTITION_DEF_MEM} --chdir={WORK_DIR} --output={result_filename}"
            # if trace_names in ["wc_8443", "wc_map0", "429.mcf", "459.GemsFDTD", "507.cactuBSSN", "519.lbm", "549.fotonik3d"]:
            #     sb_cmd += f" --mem={PARTITION_BIG_MEM}"
            # else:
            #     sb_cmd += f" --mem={PARTITION_DEF_MEM}"
            sb_cmd += f" --error={error_filename} --partition={PARTITION_NAME} --job-name='{job_name}'"
            sb_cmd += f" {sbatch_filename}"

            run_commands.append(sb_cmd)
    return run_commands

### Create run_scripts directory
os.system(f"rm -rf {WORK_DIR}/run_scripts")
os.system(f"mkdir -p {WORK_DIR}/run_scripts")

multi_cmds = get_multicore_run_commands()

# Write the run script
with open("run.sh", "w") as f:
    f.write(f"{CMD_HEADER}\n")
    for cmd in multi_cmds:
        f.write(f"{cmd}\n")

os.system("chmod uog+x run.sh")