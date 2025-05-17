import os
import yaml
import copy
import argparse
import pandas as pd
import importlib

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
argparser.add_argument("-tl", "--trace_list", default="4core_simulations.txt", help="Path to the trace list file")
argparser.add_argument("-rc", "--run_config", required=True, help="Python file with run config (e.g., run_config_fig9)")

args = argparser.parse_args()

CHAMPSIM_DIR = args.champsim_directory
CHAMPSIM_BIN_DIR = args.champsim_bin_directory
WORK_DIR = args.working_directory
TRACE_DIR = args.trace_directory
RESULT_DIR = args.result_directory
PARTITION_NAME = args.partition_name
PARTITION_DEF_MEM = args.partition_default_memory
TRACE_LIST_FILE = args.trace_list
run_config = importlib.import_module(args.run_config)

#### Customize the SBATCH_CMD if you wnat to add more options (e.g., account, time, etc)
SBATCH_CMD = "sbatch --cpus-per-task=1 --nodes=1 --ntasks=1"


CMD_HEADER = "#!/bin/bash"

CMD_BASE = f"{CHAMPSIM_BIN_DIR}/champsim"

# Ensure directories exist
for mitigation in run_config.mitigation_list:
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

cloudsuite_workloads = ['nutch', 'cloud9', 'cassandra', 'classification']

# target_workloads = []

def get_multicore_run_commands():
    run_commands = []
    multicore_params = run_config.get_multicore_params_list()
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
            stat_str = run_config.make_stat_str(config[1:4])
        else:
            stat_str = run_config.make_stat_str(config[1:])
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

            ###Enable here if you want to run workloads in taget_workloads list
            # if target_workloads:
            #     if trace_names not in target_workloads:
            #         continue

            if trace_names in cloudsuite_workloads:
                NUM_EXPECTED_INSTS = run_config.NUM_EXPECTED_INSTS_CLOUD
                WARMUP_INSTS = run_config.WARMUP_INSTS_CLOUD
            else:
                NUM_EXPECTED_INSTS = run_config.NUM_EXPECTED_INSTS_SPEC
                WARMUP_INSTS = run_config.WARMUP_INSTS_SPEC
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