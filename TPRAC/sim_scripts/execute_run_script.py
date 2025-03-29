import os
import time
import argparse
import importlib
from concurrent.futures import ThreadPoolExecutor

argparser = argparse.ArgumentParser(
    prog="ExecuteRunScript",
    description="Execute a simulation run script"
)

argparser.add_argument("-s", "--slurm", action="store_true")
argparser.add_argument("-rc", "--run_config", required=True, help="Python config file (e.g., run_config_fig9.py)")

args = argparser.parse_args()
run_config = importlib.import_module(args.run_config)

SLURM = args.slurm

def check_running_jobs():
    return int(os.popen(f"squeue -u {run_config.SLURM_USERNAME} -h | wc -l").read())

def run_slurm(commands):
    for cmd in commands:
        while check_running_jobs() >= run_config.MAX_SLURM_JOBS:
            print(f"[INFO] Maximum Slurm Job limit ({run_config.MAX_SLURM_JOBS}) reached. Retrying in {run_config.SLURM_RETRY_DELAY} seconds")
            time.sleep(run_config.SLURM_RETRY_DELAY)
        # print(cmd)
        os.system(cmd)
        time.sleep(run_config.SLURM_SUBMIT_DELAY)

def run_personal(commands):
    with ThreadPoolExecutor(max_workers=run_config.PERSONAL_RUN_THREADS) as executor:
        def run_command(cmd):
            os.system(f"echo \"Running: {cmd}\"")
            os.system(cmd)
        executor.map(run_command, commands)

if __name__ == "__main__":
    lines = []
    with open("run.sh", "r") as f:
        lines = [l.strip() for l in f.readlines()]
    
    if SLURM:
        run_slurm(lines)
    else:
        run_personal(lines) 