# When Mitigations Backfire: Timing Channel Attacks and Defense for PRAC-Based Rowhammer Mitigations (ISCA 2025)

## Introduction
This repository contains the artifact for our ISCA 2025 paper:  
**"When Mitigations Backfire: Timing Channel Attacks and Defense for PRAC-Based Rowhammer Mitigations"**

**Authors**:  [Jeonghyun Woo](https://jeonghyunwoo0306.github.io/) (University of British Columbia), Joyce Qu (University of Toronto), [Gururaj Saileshwar](https://gururaj-s.github.io/) (University of Toronto), and [Prashant Nair](https://prashantnair.bitbucket.io/) (University of British Columbia).

This artifact allows you to reproduce the key results for:
- **PRACLeak**: a timing-based covert and side channel attack against PRAC-based Rowhammer mitigations.
- **TPRAC**: our proposed mitigation that eliminates such information leakage.

## Acknowledgement
This artifact builds on and extends the [QPRAC (HPCA 2025)](https://github.com/sith-lab/qprac) artifact.

## Requirements

**Runtime Environment:**
We recommend using a modern Linux distribution with support for C++20. For example, Ubuntu 22.04 or later is recommended if you prefer Ubuntu. This artifact has been tested on Ubuntu 22.04 and Rocky Linux 9.4.

**Software Requirements:**
  - `g++` with **C++20** support. We highly recommend using **g++ 12 or newer**, as it significantly reduces the compilation time of ChampSim binaries. This artifact has been tested with **g++ 11 and g++ 12**.
  - `python3` (tested with versions 3.9 and 3.10)
  
**Hardware Recommendations:**
- **PRACLeak Evaluations:** Any modern desktop/laptop should suffice. A laptop with a 2-core CPU and 16GB of memory can perform the PRACLeak analysis within ~8 hours.

- **TPRAC Evaluations:**
  - We strongly recommend using [Slurm](https://slurm.schedmd.com/documentation.html) with a cluster capable of running bulk experiments (e.g., $\geq$ 500 jobs) to accelerate evaluations.
  - If using a personal server, we recommend a machine with at least **80 hardware threads with 128GB of memory** to run all evaluations in a reasonable time.

## Steps for PRACLeak Evaluation

Please run the following steps to run PRACLeak Evaluation and regenerate results and figures (Figures 3-5 and 13).

#### 1. Clone the Repository
```bash
git clone https://github.com/STAR-Laboratory/PRAC_Timing_Channel_ISCA25.git
```

#### 2. Set up the Pin tool (Optional)
Pin3.7 is needed for the [trace generator](/https://github.com/CMU-SAFARI/ramulator/tree/master/trace_generator) tool provided in [Ramulator](/https://github.com/CMU-SAFARI/ramulator), which we use to generate program traces for the AES T-table implementation. **This step is optional as we also provide scripts to download all generated traces.**

Pin3.7 can be downloaded [here](/https://software.intel.com/sites/landingpage/pintool/downloads/pin-3.7-97619-g0d0c92f4f-gcc-linux.tar.gz), or installed with the following script.

```bash
wget https://software.intel.com/sites/landingpage/pintool/downloads/pin-3.7-97619-g0d0c92f4f-gcc-linux.tar.gz
tar -xvzf pin-3.7-97619-g0d0c92f4f-gcc-linux.tar.gz
```

Then, set `PIN_ROOT` as the directory to the pin tool in the following scripts.
- In `PRAC_Timing_Channel_ISCA25/PRACLeak/run_artifact.sh`, on line 42, `export PIN_ROOT= your/path/to/pin/tools/pin-3.7-97619-g0d0c92f4f-gcc-linux`
- In `PRAC_TC_ISCA25/PRACLeak/scripts/util_scripts/gen_aes_code_traces.sh`, on line 15, `export PIN_ROOT= your/path/to/pin/tools/pin-3.7-97619-g0d0c92f4f-gcc-linux`

Finally, configure the pin tool as specified by trace generator.
- `cd $PIN_ROOT/source/tools/Config`
- add `-std=c++11 -faligned-new` to `makefile.unix.config:109`.
- New line should look like this:
    `TOOL_CXXFLAGS_NOOPT := -std=c++11 -faligned-new -Wall -Werror -Wno-unknown-pragmas -D__PIN__=1 -DPIN_CRT=1`

#### 3. Run the Artifact

Run the following commands to install dependencies, build trace generator and Ramulator2, and execute simulations. 

```bash
cd PRACLeak
bash ./run_artifact.sh --use_sample
```
We highly recommend using the `--use_sample` flag which allows you run without having to setup Pin, to download all necessary traces and to avoid regenerating them. Without the flag, you would need root priviledge in order to run trace generator:

```bash
cd PRACLeak
sudo bash ./run_artifact.sh
```

## Steps for TPRAC Evaluation

Please run the following steps to run TPRAC security analysis and performance evaluation and regenerate results and figures (Figures 7 and 9-12).

#### 1. Clone the Repository
Ensure you have already cloned the repository during the PRACLeak evaluation:
```bash
git clone https://github.com/STAR-Laboratory/PRAC_Timing_Channel_ISCA25.git
```
#### 2. Set Required Library Path
```bash
cd TPRAC/
source setup_lib_path.sh
```

#### 3. Set Simulation Configuration Parameters

##### Using [Slurm](https://slurm.schedmd.com/documentation.html)
Configure the following parameters in `./TPRAC/run_artifact.sh` or relevant [Slurm](https://slurm.schedmd.com/documentation.html) scripts (`run_slurm_fig*.sh`):
- **`SLURM_PART_NAME`**: Partition name for Slurm jobs.
- **`SLURM_PART_DEF_MEM`**: Default memory size for jobs (recommended: ≥ 6GB).
- **`MAX_SLURM_JOBS`**: Maximum number of Slurm jobs submitted.

##### Using a Personal Server
Configure the following parameter in `./TPRAC/run_artifact.sh` or `run_ps_fig*.sh`:
- **`PERSONAL_RUN_THREADS`**: Number of parallel threads to use for simulations.

#### 4. Run the Artifact
Run the following commands to install dependencies, download traces, generate ChampSim configurations, build ChampSim and Ramulator2, and execute simulations.
> **Note:**  Running all experiments on a personal server may take significant time (almost a week). Thus, if using a personal server, we highly recommend first running the security analysis (Figure 7) and main performance experiment (Figure 9) and reviewing the results before proceeding with the full set of experiments (Figure 7 and Figure 9-12).

##### Security Analysis (Figure 7) and Main Performance Experiment (Figure 9)
- **Using Slurm**: Faster (~20 hours on a cluster with 500+ cores).
  ```bash
  cd TPRAC/
  ./run_artifact.sh --method slurm --artifact main
  ```
- **Using a Personal Server**: Slower (~1 day on an Intel Xeon with 80 HW threads with 128GB memory).
  ```bash
  cd TPRAC/
  ./run_artifact.sh --method personal --artifact main
  ```

##### Security Analysis (Figure 7) and All Performance Experiments (Figures 9-12)
- **Using Slurm**: Faster (~2 days on a cluster with 500+ cores).
  ```bash
  cd TPRAC/
  ./run_artifact.sh -method slurm -artifact all
  ```
- **Using a Personal Server**: Slower (~1 week on an Intel Xeon with 80 HW threads with 128GB memory).   
  ```bash
  cd TPRAC/
  ./run_artifact.sh -method personal -artifact all
  ```   

#### 5. Collate Results and Generate Figures
After completing simulations, use the commands below to collate results and generate plots. Alternatively, use the Jupyter Notebook (`TPRAC/plot_scripts/plot.ipynb`). Generated figures (PDFs) can be found in `TPRAC/results/plots/`.

##### Security Analysis (Figure 7) and Main Performance Figure (Figure 9)
```bash
cd TPRAC/
./plot_main_figure.sh
```

##### All Figures (Figures 7 and 9–12)
```bash
cd TPRAC/
./plot_all_figures.sh
```
---

### Detailed Steps

#### Prerequisites
Install Python dependencies, download required traces, generate ChampSim configurations, and build ChampSim and Ramulator2:
   ```bash
   cd TPRAC/
   ./run_prerequisite.sh
   ```

#### Security Analysis: Figure 7 (Maximum Activations to a Target Row)
  ```bash
  cd TPRAC/
  ./run_fig7.sh
  ```

#### Performance Evaluation: Figure 9-12
**Set Required Library Path:**
```bash
cd TPRAC/
source setup_lib_path.sh
```

**Set simulation configuration parameters:**
- **Slurm**: Configure `SLURM_PART_NAME`, `SLURM_PART_DEF_MEM`, and `MAX_SLURM_JOBS` in `run_slurm_fig*.sh`.
- **Personal Server**: Configure `PERSONAL_RUN_THREADS` in `run_ps_fig*.sh`.


##### Using Slurm
- **Figure 9: Main Performance Result**:
  ```bash
  cd TPRAC/
  ./run_slurm_fig9.sh
  ```
- **Figure 10: Sensitivity to PRAC Levels (Number of RFMs per ABO)**:
  ```bash
  cd TPRAC/
  bash run_slurm_fig10.sh
  ```
- **Figure 11: Sensitivity to Targeted Refreshes**:
  ```bash
  cd TPRAC/
  bash run_slurm_fig11.sh
  ```
- **Figure 12: Sensitivity to Rowhammer Threshold**:
  ```bash
  cd TPRAC/
  bash run_slurm_fig12.sh
  ```

##### Using a Personal Server
- **Figure 9: Main Performance Result**:
  ```bash
  cd TPRAC/
  ./run_ps_fig9.sh
  ```
- **Figure 10: Sensitivity to PRAC Levels (Number of RFMs per ABO)**:
  ```bash
  cd TPRAC/
  ./run_ps_fig10.sh
  ```
- **Figure 11: Sensitivity to Targeted Refreshes**:
  ```bash
  cd TPRAC/
  ./run_ps_fig11.sh
  ```
- **Figure 12: Sensitivity to Rowhammer Threshold**:
  ```bash
  cd TPRAC/
  ./run_ps_fig12.sh
  ```

#### Collate Results
Once simulations complete, generate results (CSV files) using the commands below. Generated csv files can be found in `TPRAC/results/csvs/`.
- **Figure 7: MAX ACTs to a Row as TB-Window Varies (Security Analysis)**:
  ```bash
  cd TPRAC/plot_scripts
  python3 generate_csv_fig7.py
  ```
- **Figure 9: Main Results**:
  ```bash
  cd TPRAC/plot_scripts
  python3 generate_csv_fig9.py
  ```
- **Figure 10: Sensitivity to PRAC Levels (Number of RFMs per ABO)**:
  ```bash
  cd TPRAC/plot_scripts
  python3 generate_csv_fig10.py
  ```
- **Figure 11: Sensitivity to Targeted Refreshes**:
  ```bash
  cd TPRAC/plot_scripts
  python3 generate_csv_fig11.py
  ```
- **Figure 12: Sensitivity to Rowhammer Threshold**:
  ```bash
  cd TPRAC/plot_scripts
  python3 generate_csv_fig12.py
  ```

#### Generate Plots
After collating results, generate the plots using the commands below. Alternatively, use the Jupyter Notebook (`TPRAC/plot_scripts/plot.ipynb`). Generated Figures (PDFs) can be found in `TPRAC/results/plots/`.
- **Figure 7: MAX ACTs to a Row as TB-Window Varies (Security Analysis)**:
  ```bash
  cd TPRAC/plot_scripts
  python3 plot_fig7.py
  ```
- **Figure 9: Main Results**:
  ```bash
  cd TPRAC/plot_scripts
  python3 plot_fig9.py
  ```
- **Figure 10: Sensitivity to PRAC Levels (Number of RFMs per ABO)**:
  ```bash
  cd TPRAC/plot_scripts
  python3 plot_fig10.py
  ```
- **Figure 11: Sensitivity to Targeted Refreshes**:
  ```bash  
  cd TPRAC/plot_scripts
  python3 plot_fig11.py
  ```
- **Figure 12: Sensitivity to Rowhammer Threshold**:
  ```bash
  cd TPRAC/plot_scripts
  python3 plot_fig12.py
  ```