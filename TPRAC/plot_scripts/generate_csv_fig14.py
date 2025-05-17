import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import warnings
from scipy.stats import gmean

multi_cores_out_path = '../results'

df = pd.DataFrame(columns=["mitigation", "workload"])
df_baseline = pd.DataFrame(columns=["mitigation", "workload"])

mitigation_list = ["Baseline", 'TPRAC', 'TPRAC-TREFpertREFI', 'TPRAC-TREFper4tREFI', 'TPRAC-NoReset', 'TPRAC-NoReset-TREFpertREFI', 'TPRAC-NoReset-TREFper4tREFI']
for mitigation in mitigation_list:
    result_path = multi_cores_out_path + "/" + mitigation +"/stats/"
    result_list = [x[:-4] for x in os.listdir(result_path) if x.endswith(".txt")]
    for result_filename in result_list:
        result_file = open(result_path + result_filename + ".txt", "r")
        if mitigation == "Baseline":
            temp = result_filename.split("_")[0]
            if temp == "hashed":
                branch = result_filename.split("_")[0] + "_"+ result_filename.split("_")[1]
                pref_temp = result_filename.split("_")[2]
                if pref_temp == "no":
                    pref = pref_temp
                    workload = result_filename.split("_")[4]
                elif pref_temp == "va":
                    pref = result_filename.split("_")[2] + "_" + result_filename.split("_")[3] + "_" + result_filename.split("_")[4]
                    workload = result_filename.split("_")[6]
                else:
                    pref = result_filename.split("_")[2] + "_" + result_filename.split("_")[3]
                    workload = result_filename.split("_")[5]    
            else:
                branch = result_filename.split("_")[0]
                pref = result_filename.split("_")[1] + "_" + result_filename.split("_")[2]
                workload = result_filename.split("_")[4]
        #### Process other mitigations
        else:
            temp = result_filename.split("_")[0]
            if temp == "hashed":
                branch = result_filename.split("_")[0] + "_"+ result_filename.split("_")[1]
                pref_temp = result_filename.split("_")[2]
                if pref_temp == "no":
                    pref = pref_temp
                    NRH = int(result_filename.split("_")[4])
                    PRAC_level = int(result_filename.split("_")[5])
                    workload = result_filename.split("_")[6]
                elif pref_temp == "va":
                    pref = result_filename.split("_")[2] + "_" + result_filename.split("_")[3] + "_" + result_filename.split("_")[4]
                    NRH = int(result_filename.split("_")[6])
                    PRAC_level = int(result_filename.split("_")[7])
                    workload = result_filename.split("_")[8]
                else:
                    pref = result_filename.split("_")[2] + "_" + result_filename.split("_")[3]
                    NRH = int(result_filename.split("_")[5])
                    PRAC_level = int(result_filename.split("_")[6])
                    workload = result_filename.split("_")[7]
                if PRAC_level != 1:
                    continue
                if NRH != 1024 and workload in ['602.gcc', 'nutch']:
                    continue
            else:
                branch = result_filename.split("_")[0]
                pref = result_filename.split("_")[1] + "_" + result_filename.split("_")[2]
                NRH = int(result_filename.split("_")[4])
                PRAC_level = int(result_filename.split("_")[5])
                workload = result_filename.split("_")[6]
                if PRAC_level != 1:
                    continue
                if NRH != 1024 and workload in ['602.gcc', 'nutch']:
                    continue
                
        w0=''
        w1=''
        w2=''
        w3=''
        ipc_0 = 0
        ipc_1 = 0
        ipc_2 = 0
        ipc_3 = 0
        num_inst_0=0
        num_inst_1=0
        num_inst_2=0
        num_inst_3=0
        num_llc_misses = 0
        mpki = 0.0
        num_rd_reqs=0
        num_wr_reqs=0
        wr_reqs_ratio = 0.0
        num_row_hits = 0
        num_row_misses = 0
        num_row_conflicts = 0

        total_energy = 0.0
        mitigation_energy = 0.0
        for line in result_file.readlines():
            if "CPU 0" in line and "runs" in line:
                w0 = line.split("/")[-1].split("-")[0].split("_")[0].strip()
            if "CPU 1" in line and "runs" in line:
                w1 = line.split("/")[-1].split("-")[0].split("_")[0].strip()
            if "CPU 2" in line and "runs" in line:
                w2 = line.split("/")[-1].split("-")[0].split("_")[0].strip()
            if "CPU 3" in line and "runs" in line:
                w3 = line.split("/")[-1].split("-")[0].split("_")[0].strip()
            if "Simulation complete CPU 0" in line and "cumulative IPC:" in line:
                ipc_0 = float(line.split("cumulative IPC:")[1].split()[0].strip())
                num_inst_0 = float(line.split("instructions:")[1].split()[0].strip())
            if "Simulation complete CPU 1" in line and "cumulative IPC:" in line:
                ipc_1 = float(line.split("cumulative IPC:")[1].split()[0].strip())
                num_inst_1 = float(line.split("instructions:")[1].split()[0].strip())
            if "Simulation complete CPU 2" in line and "cumulative IPC:" in line:
                ipc_2 = float(line.split("cumulative IPC:")[1].split()[0].strip())
                num_inst_2 = float(line.split("instructions:")[1].split()[0].strip())
            if "Simulation complete CPU 3" in line and "cumulative IPC:" in line:
                ipc_3 = float(line.split("cumulative IPC:")[1].split()[0].strip())
                num_inst_3 = float(line.split("instructions:")[1].split()[0].strip())
            if any(cpu in line for cpu in ["cpu0->LLC TOTAL", "cpu1->LLC TOTAL", "cpu2->LLC TOTAL", "cpu3->LLC TOTAL"]):
                num_llc_misses += int(line.split("MISS:")[1].split()[0].strip())
            if (" total_num_read_requests" in line):
                num_rd_reqs = int(line.split(" ")[-1])
            if (" total_num_write_requests" in line):
                num_wr_reqs = int(line.split(" ")[-1])
            if (" total_energy:" in line):
                total_energy += float(line.split(" ")[-1])   
            if (" qprac_mitigation_energy:" in line):
                mitigation_energy += float(line.split(" ")[-1])
            if ("num_row_misses:" in line):
                num_row_misses += int(line.split(" ")[-1])      
            if ("num_row_conflicts:" in line):
                num_row_conflicts += int(line.split(" ")[-1])    
            if ("num_row_hits:" in line):
                num_row_hits += int(line.split(" ")[-1])    

        result_file.close()
             
        if ipc_0 == 0 and ipc_1 == 0 and ipc_2 == 0 and ipc_3 == 0:
            continue
        num_total_insts = num_inst_0 + num_inst_1 + num_inst_2 + num_inst_3
        num_llc_misses = num_llc_misses/2 ## To deal with duplicate output in ChampSim
        ## LLC MPKI calculations
        if num_total_insts != 0:
            mpki = float(num_llc_misses/(num_total_insts/1000))   
        WS = ipc_0 + ipc_1 + ipc_2 + ipc_3
        if num_rd_reqs != 0 or num_wr_reqs !=0:
            wr_reqs_ratio = float(int(num_wr_reqs)/int(num_rd_reqs + num_wr_reqs))
        ## RBMPKI calculations
        if num_total_insts != 0:
            rbmpki = float((num_row_misses)/(num_total_insts/1000))
        ### Scale down mitigation energy considering RFM environemt (Similarto Refreshes)
        mitigation_energy *= 0.17654164
        ### Energy Calcuations
        total_energy += mitigation_energy 
        if total_energy != 0:
            mitigation_energy_rate = float(mitigation_energy/total_energy) * 100

        # Create a new DataFrame for the new row
        if mitigation in ["Baseline"]:
            new_row = pd.DataFrame({
                'mitigation': [mitigation],
                'workload': [workload],
                'WS': [WS],
                'MPKI': [mpki],
                'RBMPKI': [rbmpki],
            })
            df_baseline = pd.concat([df_baseline, new_row], ignore_index=True)
        else:
            new_row = pd.DataFrame({
                'mitigation': [mitigation],
                'workload': [workload],
                'WS': [WS],
                'MPKI': [mpki],
                'RBMPKI': [rbmpki],
                'NRH' : [NRH],
                'Prac Level': [PRAC_level]
            })
            df = pd.concat([df, new_row], ignore_index=True)

# Ensure the results/csvs directory exists
csv_dir = '../results/csvs'
os.makedirs(csv_dir, exist_ok=True)

### Baseline Data
df_baseline_perf = df_baseline.pivot(index=['workload'], columns=['mitigation'], values='WS').reset_index()

df_perf = df.pivot(index=['workload', 'NRH', 'Prac Level'], 
                   columns=['mitigation'], values='WS').reset_index()
df_perf = df_perf.merge(df_baseline_perf, on=['workload'], how='left')

for mitigation in set(mitigation_list) - set(['Baseline']):
    df_perf[mitigation] = df_perf[mitigation] / df_perf['Baseline']

df_perf.drop(columns=['Baseline'], inplace=True)

benchmark_suites = {
    'SPEC2K6 (26)': ['401.bzip2', '403.gcc', '410.bwaves', '416.gamess', '429.mcf' '433.milc', '435.gromacs', '436.cactusADM', '437.leslie3d', 
                     '444.namd', '445.gobmk', '447.dealII', '450.soplex', '453.povray', '454.calculix', 
                     '456.hmmer', '458.sjeng', '459.GemsFDTD', '464.h264ref', '465.tonto', '470.lbm', '471.omnetpp', 
                     '473.astar', '481.wrf', '482.sphinx3', '483.xalancbmk'], 
    'SPEC2K17 (20)': ['600.perlbench', '602.gcc', '603.bwaves', '605.mcf', '607.cactuBSSN', 
                      '619.lbm', '620.omnetpp', '621.wrf', '623.xalancbmk', '625.x264', 
                      '627.cam4', '628.pop2', '631.deepsjeng', '638.imagick', '641.leela', 
                      '644.nab', '648.exchange2', '649.fotonik3d', '654.roms', '657.xz'], 
    'CloudSuite (4)': ['cassandra', 'classification', 'cloud9', 'nutch'], 
}

def calculate_geometric_mean(series):
    return np.prod(series) ** (1 / len(series))

def add_geomean_rows(df):
    geomean_rows = []  # List to collect new rows
    for NRH in df['NRH'].unique():
        for prac_level in df['Prac Level'].unique():
            for suite_name, workloads in benchmark_suites.items():
                suite_df = df[(df['workload'].isin(workloads)) & 
                                (df['NRH'] == NRH) & 
                                (df['Prac Level'] == prac_level)]
                
                if not suite_df.empty:
                    geomeans = {}

                    # Dynamically calculate geometric means for each mitigation
                    for mitigation in mitigation_list:
                        geomeans[mitigation] = calculate_geometric_mean(suite_df[mitigation])

                    # Create a new row
                    geomean_row = {
                        'workload': suite_name,  
                        'NRH': NRH, 
                        'Prac Level': prac_level, 
                        **geomeans
                    }
                    geomean_rows.append(geomean_row)  # Append to the list

    # Convert list of rows to DataFrame
    geomean_df = pd.DataFrame(geomean_rows)
    
    return pd.concat([df, geomean_df], ignore_index=True)

def add_all_workloads_geomean_rows(df):
    geomean_rows = []  # List to collect new rows
    for NRH in df['NRH'].unique():
        for prac_level in df['Prac Level'].unique():
            Channel_df = df[(df['NRH'] == NRH) & 
                            (df['Prac Level'] == prac_level)]
            
            if not Channel_df.empty:
                geomean_values = {}
                for mitigation in mitigation_list:
                    if mitigation in Channel_df.columns and not Channel_df[mitigation].empty:  # Check existence
                        geomean_values[mitigation] = calculate_geometric_mean(Channel_df[mitigation])

                # Create a new row
                geomean_row = {
                    'workload': 'All (50)',  
                    'NRH': NRH, 
                    'Prac Level': prac_level, 
                    **geomean_values
                }
                geomean_rows.append(geomean_row)

    return pd.concat([df, pd.DataFrame(geomean_rows)], ignore_index=True)


mitigation_list = ['TPRAC', 'TPRAC-TREFpertREFI', 'TPRAC-TREFper4tREFI', 'TPRAC-NoReset', 'TPRAC-NoReset-TREFpertREFI', 'TPRAC-NoReset-TREFper4tREFI']
new_column_order = ['workload', 'NRH', 'Prac Level'] + mitigation_list

geomean_df = add_geomean_rows(df_perf)
geomean_df = add_all_workloads_geomean_rows(geomean_df)
geomean_df = geomean_df[new_column_order]


### Results for main performance and energy results using hashed_perceptron and spp_dev 
print(geomean_df[geomean_df['workload'] == 'All (50)'])
geomean_df.to_csv(os.path.join(csv_dir, 'results_fig14.csv'), index=False)