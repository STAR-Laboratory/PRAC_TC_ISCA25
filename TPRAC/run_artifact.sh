#!/bin/bash

# Function to display usage instructions
usage() {
    echo "Usage: $0 --method <slurm|personal> --artifact <all|main>"
    echo "
Options:"
    echo "  --method <slurm|personal>    Specify the execution method"
    echo "  --artifact <all|main>        Specify whether to run all artifacts or just main results"
    exit 1
}

# Initialize variables
METHOD=""
ARTIFACT=""

#### Run configuration settings
PERSONAL_RUN_THREADS=40 # Set Maximum concurrent threads here
#### Set Here if you use SLURM
SLURM_PART_NAME="skylake"
SLURM_PART_DEF_MEM='6G' 
MAX_SLURM_JOBS=1000      # Set Maximum slurm jobs here

# Parse arguments
while [[ "$#" -gt 0 ]]; do
    case "$1" in
        --method)
            METHOD="$2"
            shift 2
            ;;
        --artifact)
            ARTIFACT="$2"
            shift 2
            ;;
        *)
            echo "[ERROR] Unknown option: $1"
            usage
            ;;
    esac
done

# Validate required arguments
if [[ -z "$METHOD" ]]; then
    echo "[ERROR] --method is required"
    usage
fi

if [[ -z "$ARTIFACT" ]]; then
    echo "[ERROR] --artifact is required"
    usage
fi

##### 1. Run prerequiste: Install dependencies, download traces, build frameworks
echo "----------------------------------"
echo ""
echo "##################################"
echo "[INFO] Start Prerequiste Steps"
echo "##################################"
bash "$PWD/run_prerequisite.sh"

##### 2. Run Evaluation
echo "----------------------------------"
echo ""
echo "##################################"
echo "[INFO] Running TPRAC Evaluation"
echo "##################################"
# Set MAX_SLURM_JOBS and PERSONAL_RUN_THREADS as environment variables
export MAX_SLURM_JOBS=$MAX_SLURM_JOBS
export PERSONAL_RUN_THREADS=$PERSONAL_RUN_THREADS

# 2.1. Run Security Analysis first
echo "[INFO] Running TPRAC Security Analysis: Figure 7"
bash "$PWD/run_fig7.sh"

# 2.2 Run Performance Experiments
# Execute based on method and artifact choice
if [[ "$METHOD" == "slurm" ]]; then
    echo "[INFO] Running experiments with SLURM"
    if [[ "$ARTIFACT" == "all" ]]; then
        echo "[INFO] Running all required experiments"
        SLURM_PART_NAME="$SLURM_PART_NAME" SLURM_PART_DEF_MEM="$SLURM_PART_DEF_MEM" bash "$PWD/run_slurm_fig10.sh"
        SLURM_PART_NAME="$SLURM_PART_NAME" SLURM_PART_DEF_MEM="$SLURM_PART_DEF_MEM" bash "$PWD/run_slurm_fig11.sh"
        SLURM_PART_NAME="$SLURM_PART_NAME" SLURM_PART_DEF_MEM="$SLURM_PART_DEF_MEM" bash "$PWD/run_slurm_fig12.sh"
        SLURM_PART_NAME="$SLURM_PART_NAME" SLURM_PART_DEF_MEM="$SLURM_PART_DEF_MEM" bash "$PWD/run_slurm_fig13.sh"
        SLURM_PART_NAME="$SLURM_PART_NAME" SLURM_PART_DEF_MEM="$SLURM_PART_DEF_MEM" bash "$PWD/run_slurm_fig14.sh"
      
    elif [[ "$ARTIFACT" == "main" ]]; then
        echo "[INFO] Running main performance experiment only (Figure 10)"
        SLURM_PART_NAME="$SLURM_PART_NAME" SLURM_PART_DEF_MEM="$SLURM_PART_DEF_MEM" bash "$PWD/run_slurm_fig10.sh"
    else
        echo "[ERROR] Invalid artifact choice: $ARTIFACT"
        usage
    fi

elif [[ "$METHOD" == "personal" ]]; then
    echo "[INFO] Running experiments on personal server"
    if [[ "$ARTIFACT" == "all" ]]; then
        echo "[INFO] We highly recommend running the main performance experiment and reviewing the results first before proceeding with all experiments if using a personal server. Running all experiments on a personal server with limited resources (e.g., < 256GB DRAM and < 40 cores) can take a significant amount of time, potentially taking up to a week."
        echo "[INFO] Running Experiments for Figure 10"
        bash "$PWD/run_ps_fig10.sh"
        bash "$PWD/run_ps_except_main_experiment.sh"
    elif [[ "$ARTIFACT" == "main" ]]; then
        echo "[INFO] Running main performance experiment only (Figure 10)"
        bash "$PWD/run_ps_fig10.sh"
    else
        echo "[ERROR] Invalid artifact choice: $ARTIFACT"
        usage
    fi
else
    echo "[ERROR] Invalid method: $METHOD"
    usage
fi
