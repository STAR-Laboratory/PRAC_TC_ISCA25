import os
import yaml
import copy
import argparse
import pandas as pd

from run_config import *

argparser = argparse.ArgumentParser(
    prog="RunSlurm",
    description="Run ramulator2 simulations using Slurm"
)

argparser.add_argument("-rmd", "--ramulator_directory")
argparser.add_argument("-wd", "--working_directory")
argparser.add_argument("-bc", "--base_config")
argparser.add_argument("-tc", "--trace_combination")
argparser.add_argument("-td", "--trace_directory")
argparser.add_argument("-rd", "--result_directory")
argparser.add_argument("-pn", "--partition_name")

args = argparser.parse_args()

RAMULATOR_DIR = args.ramulator_directory
WORK_DIR = args.working_directory
BASE_CONFIG_FILE = args.base_config
TRACE_COMBINATION_FILE = args.trace_combination
TRACE_DIR = args.trace_directory
RESULT_DIR = args.result_directory
PARTITION_NAME = args.partition_name


SBATCH_CMD = "sbatch --cpus-per-task=1 --nodes=1 --ntasks=1 --time=24:00:00 \
    --mem=6000M --mail-user=jhwoo36@ece.ubc.ca --mail-type=FAIL --account=st-prashnr-1"


# Define the list of traces that want to rerun
# rerun_trace_list = ["MIX0", "MIX1", "MIX10", "MIX11", "MIX12", "MIX13", "MIX14", "MIX15", "MIX16", "MIX17", "MIX20", "MIX21", "MIX25"]
# rerun_trace_list = ["MIX0", "MIX1", "MIX10", "MIX11", "MIX13","MIX14","MIX17","MIX3", "MIX45", "MIX49", "MIX5", "MIX51", "MIX56", "MIX57", "MIX58", "MIX6", "MIX68", "MIX7", "MIX72", "MIX8", "MIX9"]
# rerun_trace_list = ["MIX11"]

CMD_HEADER = "#!/bin/bash"
CMD = f"{RAMULATOR_DIR}/ramulator2"

BASE_CONFIG = None

with open(BASE_CONFIG_FILE, "r") as f:
    try:
        BASE_CONFIG = yaml.safe_load(f)
    except Exception as e:
        print(e)

if BASE_CONFIG == None:
    print("[ERR] Could not read base config.")
    exit(0)

BASE_CONFIG["Frontend"]["num_expected_insts"] = NUM_EXPECTED_INSTS
if NUM_MAX_CYCLES > 0:
    BASE_CONFIG["Frontend"]["num_max_cycles"] = NUM_MAX_CYCLES 

TRACE_COMBS = {}
TRACE_TYPES = {}
with open(TRACE_COMBINATION_FILE, "r") as f:
    for line in f:
        line = line.strip()
        tokens = line.split(',')
        trace_name = tokens[0]
        trace_type = tokens[1]
        traces = tokens[2:]
        TRACE_COMBS[trace_name] = traces
        TRACE_TYPES[trace_name] = trace_type

for mitigation in mitigation_list:
    for path in [
            f"{RESULT_DIR}/{mitigation}/stats",
            f"{RESULT_DIR}/{mitigation}/errors",
            f"{RESULT_DIR}/{mitigation}/configs",
            f"{RESULT_DIR}/{mitigation}/cmd_count"
        ]:
        if not os.path.exists(path):
            os.makedirs(path)

def get_singlecore_run_commands():
    run_commands = []
    singlecore_params = get_singlecore_params_list()
    singlecore_traces, _ = get_trace_lists(TRACE_COMBINATION_FILE)
    for config in singlecore_params:
        mitigation, tRH = config
        stat_str = make_stat_str(config[1:])
        for trace in singlecore_traces:
            result_filename = f"{RESULT_DIR}/{mitigation}/stats/{stat_str}_{trace}.txt"
            error_filename = f"{RESULT_DIR}/{mitigation}/errors/{stat_str}_{trace}.txt"
            config_filename = f"{RESULT_DIR}/{mitigation}/configs/{stat_str}_{trace}.yaml"
            cmd_count_filename = f"{RESULT_DIR}/{mitigation}/cmd_count/{stat_str}_{trace}.cmd.count"
            config = copy.deepcopy(BASE_CONFIG)

            config["MemorySystem"][CONTROLLER]["plugins"][0]["ControllerPlugin"]["path"] = cmd_count_filename
                
            config["Frontend"]["traces"] = [f"{TRACE_DIR}/{trace}"]

            add_mitigation(config, mitigation, tRH)

            config_file = open(config_filename, "w")
            yaml.dump(config, config_file, default_flow_style=False)
            config_file.close()

            sbatch_filename = f"{WORK_DIR}/run_scripts/{mitigation}_{stat_str}_{trace}.sh"
            sbatch_file = open(sbatch_filename, "w")
            sbatch_file.write(f"{CMD_HEADER}\n{CMD} -f {config_filename}\n")
            sbatch_file.close()



            job_name = f"{mitigation}_{stat_str}_{trace}"
            sb_cmd = f"{SBATCH_CMD} --chdir={WORK_DIR} --output={result_filename}"
            if i % 2 == 1:
                sb_cmd = sb_cmd.replace("--mem=6000M", "--mem=4500M")
            if trace in ["bfs_dblp", "bfs_ny", "bfs_cm2003", "wc_8443", "wc_map0", "random_10.trace", "429.mcf", "519.lbm"]:
                if i % 2 == 0:
                    sb_cmd = sb_cmd.replace("--mem=6000M", "--mem=24G")
                else:
                    sb_cmd = sb_cmd.replace("--mem=4500M", "--mem=19G")
            sb_cmd += f" --error={error_filename} --partition={PARTITION_NAME} --job-name='{job_name}'"
            sb_cmd += f" {sbatch_filename}"

            if i % 2 == 1:
                sb_cmd = sb_cmd.replace("--partition=skylake", "--partition=cascade")
            run_commands.append(sb_cmd)
            i += 1
    return run_commands

def get_multicore_run_commands():
    run_commands = []
    multicore_params = get_multicore_params_list()
    _, multicore_traces = get_trace_lists(TRACE_COMBINATION_FILE)
    for config in multicore_params:
        mitigation, num_channel, interface_speed, NRH, PRAC_level = config
        stat_str = make_stat_str(config[1:])
        i = 0
        for trace in multicore_traces:
            result_filename = f"{RESULT_DIR}/{mitigation}/stats/{stat_str}_{trace}.txt"
            error_filename = f"{RESULT_DIR}/{mitigation}/errors/{stat_str}_{trace}.txt"
            config_filename = f"{RESULT_DIR}/{mitigation}/configs/{stat_str}_{trace}.yaml"
            cmd_count_filename = f"{RESULT_DIR}/{mitigation}/cmd_count/{stat_str}_{trace}.cmd.count"
            config = copy.deepcopy(BASE_CONFIG)

            config["MemorySystem"][CONTROLLER]["plugins"][0]["ControllerPlugin"]["path"] = cmd_count_filename

            trace_comb = TRACE_COMBS[trace]
            traces = []
            for idx in range(len(trace_type)):
                cur_type = trace_type[idx]
                cur_trace = f"{TRACE_DIR}/{trace_comb[idx]}"
                traces.append(cur_trace)
            
            ### TODO: JUST FOR RERUN PURPOSE
            # if trace not in rerun_trace_list:
            #     continue
            
            config["Frontend"]["traces"] = traces

            add_mitigation(config, mitigation, interface_speed, num_channel, NRH, PRAC_level)

            config_file = open(config_filename, "w")
            yaml.dump(config, config_file, default_flow_style=False)
            config_file.close()

            sbatch_filename = f"{WORK_DIR}/run_scripts/{mitigation}_{stat_str}_{trace}.sh"
            sbatch_file = open(sbatch_filename, "w")
            sbatch_file.write(f"{CMD_HEADER}\n{CMD} -f {config_filename}\n")
            sbatch_file.close()

            job_name = f"{mitigation}_{stat_str}_{trace}"
            sb_cmd = f"{SBATCH_CMD} --chdir={WORK_DIR} --output={result_filename}"
            if i % 2 == 1:
                sb_cmd = sb_cmd.replace("--mem=6000M", "--mem=4500M")
            if trace in ["MIX45", "MIX8", "MIX57", "MIX58", "MIX17"]:
                if i % 2 == 0:
                    sb_cmd = sb_cmd.replace("--mem=6000M", "--mem=24G")
                else:
                    sb_cmd = sb_cmd.replace("--mem=4500M", "--mem=19G")
            sb_cmd += f" --error={error_filename} --partition={PARTITION_NAME} --job-name='{job_name}'"
            sb_cmd += f" {sbatch_filename}"

            if i % 2 == 1:
                sb_cmd = sb_cmd.replace("--partition=skylake", "--partition=cascade")
            run_commands.append(sb_cmd)
            i += 1
    return run_commands

os.system(f"rm -r {WORK_DIR}/run_scripts")
os.system(f"mkdir -p {WORK_DIR}/run_scripts")

# single_cmds = get_singlecore_run_commands()
multi_cmds = get_multicore_run_commands()

with open("run.sh", "w") as f:
    f.write(f"{CMD_HEADER}\n")
    # for cmd in single_cmds + multi_cmds:
        # f.write(f"{cmd}\n")
    for cmd in multi_cmds:
        f.write(f"{cmd}\n")

os.system("chmod uog+x run.sh")