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
# TODO: Change below after testing the artifact
# SLURM_PART_NAME=""
SLURM_PART_NAME="skylake"
SLURM_PART_DEF_MEM='4G' 
SLRUM_PART_BIG_MEM='12G' 
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
echo "---------------------------"
echo ""
echo "#####################"
echo "[INFO] Strating Prerequiste Steps"
echo "#####################"
bash "$PWD/run_prerequisite.sh"