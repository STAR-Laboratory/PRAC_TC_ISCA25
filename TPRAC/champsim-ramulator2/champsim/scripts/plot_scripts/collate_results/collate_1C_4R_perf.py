import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import warnings
from scipy.stats import gmean

# multi_cores_out_path = '../../../temp_results/TPRAC_ISCA2025/4cores_results'
multi_cores_out_path = '../../../temp_results/TPRAC_ISCA2025/4cores_results/L1D-IPStride/With_RD_Merge'

df = pd.DataFrame(columns=["mitigation", "workload"])
df_baseline = pd.DataFrame(columns=["mitigation", "workload"])

test_workload_list = ['nutch', 'cloud9', 'classification', 'cassandra']

mitigation_list = ["Baseline", "ABO_Only", 'ABO_RFM', 'TPRAC', 'TPRAC-TREFper4tREFI', 'TPRAC-TREFper3tREFI', 'TPRAC-TREFper2tREFI', 'TPRAC-TREFpertREFI']
# mitigation_list = ["Baseline"]
for mitigation in mitigation_list:
    result_path = multi_cores_out_path + "/" + mitigation +"/stats/"
    result_list = [x[:-4] for x in os.listdir(result_path) if x.endswith(".txt")]
    for result_filename in result_list:
        result_file = open(result_path + result_filename + ".txt", "r")
        if mitigation == "Baseline":
            temp = result_filename.split("_")[0]
            # NRH = 1024
            # PRAC_level = 1
            if temp == "hashed":
                branch = result_filename.split("_")[0] + "_"+ result_filename.split("_")[1]
                pref_temp = result_filename.split("_")[2]
                if pref_temp == "no":
                    pref = pref_temp
                    workload = result_filename.split("_")[4]
                    if workload in test_workload_list:
                        workload = result_filename.split("_")[4] + "_" + result_filename.split("_")[5]

                elif pref_temp == "va":
                    pref = result_filename.split("_")[2] + "_" + result_filename.split("_")[3] + "_" + result_filename.split("_")[4]
                    workload = result_filename.split("_")[6]
                    if workload in test_workload_list:
                        workload = result_filename.split("_")[6] + "_" + result_filename.split("_")[7]
                else:
                    pref = result_filename.split("_")[2] + "_" + result_filename.split("_")[3]
                    workload = result_filename.split("_")[5]    
                    if workload in test_workload_list:
                        workload = result_filename.split("_")[5] + "_" + result_filename.split("_")[6]
            else:
                branch = result_filename.split("_")[0]
                pref = result_filename.split("_")[1] + "_" + result_filename.split("_")[2]
                workload = result_filename.split("_")[4]
                if workload in test_workload_list:
                    workload = result_filename.split("_")[4] + "_" + result_filename.split("_")[5]
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
                    if workload in test_workload_list:
                        workload = result_filename.split("_")[6] + "_" + result_filename.split("_")[7]
                elif pref_temp == "va":
                    pref = result_filename.split("_")[2] + "_" + result_filename.split("_")[3] + "_" + result_filename.split("_")[4]
                    NRH = int(result_filename.split("_")[6])
                    PRAC_level = int(result_filename.split("_")[7])
                    workload = result_filename.split("_")[8]
                    if workload in test_workload_list:
                        workload = result_filename.split("_")[8] + "_" + result_filename.split("_")[9]
                else:
                    pref = result_filename.split("_")[2] + "_" + result_filename.split("_")[3]
                    NRH = int(result_filename.split("_")[5])
                    PRAC_level = int(result_filename.split("_")[6])
                    workload = result_filename.split("_")[7]
                    if workload in test_workload_list:
                        workload = result_filename.split("_")[7] + "_" + result_filename.split("_")[8]
            else:
                branch = result_filename.split("_")[0]
                pref = result_filename.split("_")[1] + "_" + result_filename.split("_")[2]
                NRH = int(result_filename.split("_")[4])
                PRAC_level = int(result_filename.split("_")[5])
                workload = result_filename.split("_")[6]
                if workload in test_workload_list:
                    workload = result_filename.split("_")[6] + "_" + result_filename.split("_")[7]

        workload_prefix = workload.split("_")[0]
        if workload_prefix in test_workload_list:
            if workload_prefix == 'classification': 
                if workload.split("_")[1] == "phase1":
                    workload = workload_prefix
                else:
                    continue
            elif workload_prefix == 'cloud9': 
                if workload.split("_")[1] == "phase1":
                    workload = workload_prefix
                else:
                    continue
            elif workload_prefix == 'cassandra': 
                if workload.split("_")[1] == "phase0":
                    workload = workload_prefix
                else:
                    continue
            elif workload_prefix == 'nutch': 
                if workload.split("_")[1] == "phase1":
                    workload = workload_prefix
                else:
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
        # num_tREFI_period=0
        # num_tREFW_period=0

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
        ### TODO: Change to do this conversion in the qprac source code
        mitigation_energy *= 0.17654164
        ### Energy Calcuations
        total_energy += mitigation_energy 
        if total_energy != 0:
            mitigation_energy_rate = float(mitigation_energy/total_energy) * 100

        # Create a new DataFrame for the new row
        if mitigation in ["Baseline"]:
            new_row = pd.DataFrame({
                'mitigation': [mitigation],
                'branch_predictor': [branch],
                'prefetcher': [pref],
                'workload': [workload],
                'WS': [WS],
                'MPKI': [mpki],
                'RBMPKI': [rbmpki],
                # 'NRH': [NRH],  # <-- Ensure NRH exists, even if it's empty
                # 'Prac Level': [PRAC_level],  # <-- Ensure Prac Level exists
                'Total Energy': [total_energy],
                "Mitigation Energy": [mitigation_energy],
                'ME Percentage': [mitigation_energy_rate]
            })
            df_baseline = pd.concat([df_baseline, new_row], ignore_index=True)
        else:
            new_row = pd.DataFrame({
                'mitigation': [mitigation],
                'branch_predictor': [branch],
                'prefetcher': [pref],
                'workload': [workload],
                'WS': [WS],
                'MPKI': [mpki],
                'RBMPKI': [rbmpki],
                'NRH' : [NRH],
                'Prac Level': [PRAC_level],
                'Total Energy': [total_energy],
                "Mitigation Energy": [mitigation_energy],
                'ME Percentage': [mitigation_energy_rate]
            })
            df = pd.concat([df, new_row], ignore_index=True)

# Ensure the results/csvs directory exists
csv_dir = '../../stats/L1D-IPStride-With_RD-Merge'
os.makedirs(csv_dir, exist_ok=True)
### Baseline Data
df_baseline.to_csv(csv_dir + '/1C_4R-workload_characteristics.csv', index=False)
df_baseline_perf = df_baseline.pivot(index=['branch_predictor', 'prefetcher', 'workload'], columns=['mitigation'], values='WS').reset_index()
df_baseline_energy = df_baseline.pivot(index=['branch_predictor', 'prefetcher', 'workload'], columns=['mitigation'], values='Total Energy').reset_index()

df_baseline_rbmpki = df_baseline.pivot(index=['branch_predictor', 'prefetcher', 'workload'], columns=['mitigation'], values='RBMPKI').reset_index()

df.to_csv(csv_dir + '/TPRAC-1C_4R-rawdata.csv', index=False)
df_perf = df.pivot(index=['branch_predictor', 'prefetcher', 'workload', 'NRH', 'Prac Level'], 
                   columns=['mitigation'], values='WS').reset_index()
df_perf = df_perf.merge(df_baseline_perf, on=['branch_predictor', 'prefetcher', 'workload'], how='left')

df_normalized_energy =  df.pivot(index=['branch_predictor', 'prefetcher', 'workload', 'NRH', 'Prac Level'], 
                   columns=['mitigation'], values='Total Energy').reset_index()

df_normalized_energy = df_normalized_energy.merge(df_baseline_energy, on=['branch_predictor', 'prefetcher', 'workload'], how='left')

### Calculate the RBMPKI
df_rbmpki = df.pivot(index=['branch_predictor', 'prefetcher', 'workload', 'NRH', 'Prac Level'], 
                   columns=['mitigation'], values='RBMPKI').reset_index()
df_rbmpki = df_rbmpki.merge(df_baseline_rbmpki, on=['branch_predictor', 'prefetcher', 'workload'], how='left')


#### Energy calcultion
# Create mapping of (branch_predictor, prefetcher, workload) -> Baseline Energy
baseline_energy_map = df_baseline.set_index(['branch_predictor', 'prefetcher', 'workload'])['Total Energy'].to_dict()
baseline_mitigation_energy_map = df_baseline.set_index(['branch_predictor', 'prefetcher', 'workload'])['Mitigation Energy'].to_dict()

# Map Baseline Energy values to df
df['Baseline Energy'] = df.set_index(['branch_predictor', 'prefetcher', 'workload']).index.map(baseline_energy_map)
df['Baseline Mitigation Energy'] = df.set_index(['branch_predictor', 'prefetcher', 'workload']).index.map(baseline_mitigation_energy_map)

# =====================
# Step 2: Compute Normalized Energy & Normalized Mitigation Energy
# =====================
df['Normalized Energy'] = df['Total Energy'] / df['Baseline Energy']
df['Normalized Mitigation Energy'] = df['Mitigation Energy'] / df['Baseline Mitigation Energy']

# =====================
# Step 3: Compute Geomean of Normalized Energy by NRH & Mitigation
# =====================
geomean_energy_df = (
    df.groupby(['NRH', 'mitigation'])['Normalized Energy']
    .apply(lambda x: gmean(x.dropna()) if all(x > 0) else np.nan)  # Compute geomean, handling NaN/zero cases
    .reset_index()
    .rename(columns={'Normalized Energy': 'Geomean Normalized Energy'})
)

# =====================
# Step 4: Compute Geomean of Normalized Mitigation Energy
# =====================
geomean_mitigation_energy_df = (
    df.groupby(['NRH', 'mitigation'])['Normalized Mitigation Energy']
    .apply(lambda x: gmean(x.dropna()) if all(x > 0) else np.nan)  # Compute geomean, handling NaN/zero cases
    .reset_index()
    .rename(columns={'Normalized Mitigation Energy': 'Geomean Normalized Mitigation Energy'})
)

# =====================
# Step 5: Compute Updated Mitigation Energy Percentage
# =====================
df['ME Percentage'] = df['Mitigation Energy'] / (df['Total Energy'] - df['Baseline Energy'])

# =====================
# Step 6: Save or Print Results
# =====================
geomean_energy_df.to_csv(csv_dir + '/TPRAC-1C_4R-geomean_energy.csv', index=False)
geomean_mitigation_energy_df.to_csv(csv_dir + '/TPRAC-1C_4R-geomean_mitigation_energy.csv', index=False)

for mitigation in set(mitigation_list) - set(['Baseline']):
    df_perf[mitigation] = df_perf[mitigation] / df_perf['Baseline']
    df_normalized_energy[mitigation] = df_normalized_energy[mitigation] / df_normalized_energy['Baseline']
    df_rbmpki[mitigation] = df_rbmpki[mitigation] / df_rbmpki['Baseline']

df_perf.drop(columns=['Baseline'], inplace=True)
df_normalized_energy.drop(columns=['Baseline'], inplace=True)
df_rbmpki.drop(columns=['Baseline'], inplace=True)

df_perf.to_csv(csv_dir + '/TPRAC-1C_4R-normalized_perf.csv', index=False)
df_normalized_energy.to_csv(csv_dir + '/TPRAC-1C_4R-normalized_energy.csv', index=False)
df_rbmpki.to_csv(csv_dir + '/TPRAC-1C_4R-normalized_rbmpki.csv', index=False)
benchmark_suites = {
    'SPEC2K6 (26)': ['401.bzip2', '403.gcc', '410.bwaves', '416.gamess', '429.mcf' '433.milc', '435.gromacs', '436.cactusADM', '437.leslie3d', 
                     '444.namd', '445.gobmk', '447.dealII', '450.soplex', '453.povray', '454.calculix', 
                     '456.hmmer', '458.sjeng', '459.GemsFDTD', '464.h264ref', '465.tonto', '470.lbm', '471.omnetpp', 
                     '473.astar', '481.wrf', '482.sphinx3', '483.xalancbmk'], # SPEC2K6: 26
    'SPEC2K17 (20)': ['600.perlbench', '602.gcc', '603.bwaves', '605.mcf', '607.cactuBSSN', 
                      '619.lbm', '620.omnetpp', '621.wrf', '623.xalancbmk', '625.x264', 
                      '627.cam4', '628.pop2', '631.deepsjeng', '638.imagick', '641.leela', 
                      '644.nab', '648.exchange2', '649.fotonik3d', '654.roms', '657.xz'], # SPEC2K17: 20
    'CloudSuite (4)': ['cassandra', 'classification_phase1', 'cloud9_phase0', 'nutch_phase1'], #tpc: 4
}

def calculate_geometric_mean(series):
    return np.prod(series) ** (1 / len(series))

def add_geomean_rows(df):
    geomean_rows = []  # List to collect new rows
    for branch in df['branch_predictor'].unique():
        for pref in df['prefetcher'].unique():
            for NRH in df['NRH'].unique():
                for prac_level in df['Prac Level'].unique():
                    for suite_name, workloads in benchmark_suites.items():
                        suite_df = df[(df['branch_predictor'] == branch) & 
                                      (df['prefetcher'] == pref) &
                                      (df['workload'].isin(workloads)) & 
                                      (df['NRH'] == NRH) & 
                                      (df['Prac Level'] == prac_level)]
                        
                        if not suite_df.empty:
                            geomeans = {}

                            # Dynamically calculate geometric means for each mitigation
                            for mitigation in mitigation_list:
                                geomeans[mitigation] = calculate_geometric_mean(suite_df[mitigation])

                            # Create a new row
                            geomean_row = {
                                'branch_predictor': branch,
                                'prefetcher': pref,
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
    for branch in df['branch_predictor'].unique():
        for pref in df['prefetcher'].unique():
            for NRH in df['NRH'].unique():
                for prac_level in df['Prac Level'].unique():
                    Channel_df = df[(df['branch_predictor'] == branch) & 
                                    (df['prefetcher'] == pref) &
                                    (df['NRH'] == NRH) & 
                                    (df['Prac Level'] == prac_level)]
                    
                    if not Channel_df.empty:
                        geomean_values = {}
                        for mitigation in mitigation_list:
                            if mitigation in Channel_df.columns and not Channel_df[mitigation].empty:  # Check existence
                                geomean_values[mitigation] = calculate_geometric_mean(Channel_df[mitigation])

                        # Create a new row
                        geomean_row = {
                            'branch_predictor': branch,
                            'prefetcher': pref,
                            'workload': 'All (50)',  
                            'NRH': NRH, 
                            'Prac Level': prac_level, 
                            **geomean_values
                        }
                        geomean_rows.append(geomean_row)

    return pd.concat([df, pd.DataFrame(geomean_rows)], ignore_index=True)


mitigation_list = ["ABO_Only", 'ABO_RFM', 'TPRAC', 'TPRAC-TREFper4tREFI', 'TPRAC-TREFper3tREFI', 'TPRAC-TREFper2tREFI', 'TPRAC-TREFpertREFI']
new_column_order = ['branch_predictor', 'prefetcher', 'workload', 'NRH', 'Prac Level'] + mitigation_list

geomean_df = add_geomean_rows(df_perf)
geomean_df = add_all_workloads_geomean_rows(geomean_df)
geomean_df = geomean_df[new_column_order]

geomean_norm_energy = add_geomean_rows(df_normalized_energy)
geomean_norm_energy = add_all_workloads_geomean_rows(geomean_norm_energy)
geomean_norm_energy = geomean_norm_energy[new_column_order]

geomean_rbmpki = add_geomean_rows(df_rbmpki)
geomean_rbmpki = add_all_workloads_geomean_rows(geomean_rbmpki)
geomean_rbmpki = geomean_rbmpki[new_column_order]

### Results for main performance and energy results using hashed_perceptron and spp_dev 
main_perf_df = geomean_df[(geomean_df['branch_predictor'] == 'hashed_perceptron') & (geomean_df['prefetcher'] == 'spp_dev')]
main_energy = geomean_norm_energy[(geomean_norm_energy['branch_predictor'] == 'hashed_perceptron') & (geomean_norm_energy['prefetcher'] == 'spp_dev')]
main_perf_rbmpki = geomean_rbmpki[(geomean_rbmpki['branch_predictor'] == 'hashed_perceptron') & (geomean_rbmpki['prefetcher'] == 'spp_dev')]

print(main_perf_df[main_perf_df['workload'] == 'All (50)'])
main_perf_df.to_csv(os.path.join(csv_dir, 'TPRAC-1C_4R-main_perf.csv'), index=False)
main_energy.to_csv(os.path.join(csv_dir, 'TPRAC-1C_4R-main_energy.csv'), index=False)
main_perf_rbmpki.to_csv(os.path.join(csv_dir, 'TPRAC-1C_4R-main_rbmpki.csv'), index=False)

sens_mitigation_list = ['TPRAC', 'TPRAC-TREFper4tREFI', 'TPRAC-TREFpertREFI']
sens_new_column_order = ['branch_predictor', 'prefetcher', 'workload'] + sens_mitigation_list

branch_sens_perf_df = geomean_df[(geomean_df['prefetcher'] == 'spp_dev') & (geomean_df['NRH'] == 1024) & (geomean_df['Prac Level'] == 1)]
branch_sens_perf_df = branch_sens_perf_df[sens_new_column_order]
print(branch_sens_perf_df[branch_sens_perf_df['workload'] == 'All (50)'])
branch_sens_perf_df.to_csv(os.path.join(csv_dir, 'TPRAC-1C_4R-branch_sens_perf.csv'), index=False)

pref_sens_perf_df = geomean_df[(geomean_df['branch_predictor'] == 'hashed_perceptron') & (geomean_df['NRH'] == 1024) & (geomean_df['Prac Level'] == 1)]
pref_sens_perf_df = pref_sens_perf_df[sens_new_column_order]
print(pref_sens_perf_df[pref_sens_perf_df['workload'] == 'All (50)'])
pref_sens_perf_df.to_csv(os.path.join(csv_dir, 'TPRAC-1C_4R-prefetcher_sens_perf.csv'), index=False)