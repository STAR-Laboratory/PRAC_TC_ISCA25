import json
import os

def LoadConfig(file_path):
    """Load the default ChampSim configuration from a JSON file."""
    with open(file_path, 'r') as f:
        return json.load(f)
    
def ModifyConfig(config, modifications):
    """Modify the configuration based on the provided dictionary of changes."""
    for key, value in modifications.items():
        keys = key.split('.')  # Support nested keys like "LLC.sets"
        d = config
        for i, k in enumerate(keys[:-1]):
            if isinstance(d, list):
                # If we encounter a list, assume we modify the first element
                d = d[0]  
            d = d[k]  # Navigate to the nested dictionary

        # Handle the final key update
        last_key = keys[-1]
        if isinstance(d, list):
            d[0][last_key] = value  # Modify the first element in the list
        else:
            d[last_key] = value  # Standard dictionary update


def SaveConfig(config, output_path):
    """"Save the modified configuration to a new JSON file."""
    with open(output_path+'.json', 'w') as f:
        json.dump(config, f, indent=4)

def SetLLCSetSize(per_core_llc_size, num_cores, num_ways):
    """Set corresponding LLC Set Size. per-core LLC Size is in MB"""
    total_LLC_size = per_core_llc_size * (1024 * 1024) * num_cores # in Byte
    num_sets = int((total_LLC_size/64) // num_ways) # default block size is 64B
    return num_sets

if __name__ == "__main__":
    # Load the default configuration
    default_config_path = '../../champsim_config.json'
    config = LoadConfig(default_config_path)

    ## 1. Set LLC parameters
    num_cores = 4 # 4 8 
    num_ways = 16
    per_core_llc_size = 2 ## in MB
    num_sets = SetLLCSetSize(per_core_llc_size, num_cores, num_ways)

    ## 2. Set Ramulator2 Configuration directory
    ramulator2_config_dir = '/scratch/st-prashnr-1/jeonghyun/champsim-ramulator2/champsim/configurations/ramulator2_configs/TPRAC_ISCA2025'
    
    # Baseline DRAM Module info. Prefix of ramulator2 config names
    dram_info = '1CH_4RA_DDR5_32Gb_8000'
    # dram_info = '2CH_4RA_DDR5_32Gb_8000'


    ## 3. Set michoarchitecutre components
    # prefetcher = "spp_dev" ## Available: no, next_line, ip_stride, spp_dev, va_ampm_lite
    prefetcher = ['no', 'next_line', 'spp_dev', 'va_ampm_lite']
    # branch_predictor = "hashed_perceptron"  ## Available: bimodal, gshare, hashed_perceptron, perceptron
    branch_predictor = ['bimodal', 'gshare', 'hashed_perceptron', 'perceptron']
    llc_replacement = "srrip" ## Availbe: lru, random, srrip, drrip, ship

    ## 4. Set baseline champsim binary name -> Foramt: champsim-bp-pref-repl-memory
    champsim_bin_dir = f"{num_cores}cores-1CH-4RA-Updated"
    # champsim_bin_dir = f"{num_cores}cores-2CH-4RA"

    ## 5. Set required parameters: Ex) RowHammer thresholds, mitigation lists
    mitigation_lists = ['Baseline', 'TPRAC-TREFper4tREFI', 'TPRAC-TREFper3tREFI', 'TPRAC-TREFper2tREFI', 'TPRAC-TREFpertREFI', 'ABO_Only', 'ABO_RFM', 'TPRAC']
    # mitigation_lists = ['Baseline', 'ABO_Only', 'ABO_RFM', 'TPRAC']
    NRH_lists = [128, 256, 512, 1024, 2048, 4096]
    
    PRAC_level_lists = [1, 2, 4]    
    # PRAC_level_lists = [1]    

    for branch in branch_predictor:
        for pref in prefetcher:
            champsim_bin_base = champsim_bin_dir+"/champsim-"+branch+"-"+pref+"-"+llc_replacement
            for mitigation in mitigation_lists:
                for NRH in NRH_lists:
                    for PRAC_level in PRAC_level_lists:
                        if mitigation == "Baseline" and NRH != 1024 and PRAC_level != 1:
                            continue
                        if PRAC_level != 1:
                            if NRH != 1024:
                                continue
                        if mitigation in ['TPRAC-TREFper4tREFI', 'TPRAC-TREFper3tREFI', 'TPRAC-TREFper2tREFI', 'TPRAC-TREFpertREFI']:
                            if PRAC_level != 1:
                                continue
                        ### For main performance results
                        if pref == "spp_dev":
                            ### For prefecther sensitivity sutdy
                            if branch != "hashed_perceptron":
                                if PRAC_level != 1 or NRH != 1024 or mitigation in ['ABO_Only', 'ABO_RFM', 'TPRAC-TREFper3tREFI', 'TPRAC-TREFper2tREFI']:
                                    continue
                            ### For branch predictor sesntivity study
                        else:
                            if branch == "hashed_perceptron":
                                if PRAC_level != 1 or NRH != 1024 or mitigation in ['ABO_Only', 'ABO_RFM', 'TPRAC-TREFper3tREFI', 'TPRAC-TREFper2tREFI']:
                                    continue
                            else:
                                continue
                        ### Set final bin names and paths
                        if mitigation == "Baseline":
                            champsim_bin = champsim_bin_base+"-"+mitigation
                            ramulator2_confg_name = dram_info+"-"+mitigation+'.yaml'
                        else:
                            champsim_bin = champsim_bin_base+"-"+mitigation+'-'+str(NRH)+'-PRAC'+str(PRAC_level)
                            ramulator2_confg_name = dram_info+"-"+mitigation+"-"+str(NRH)+'-PRAC'+str(PRAC_level)+'.yaml'
                        ramulator2_config_path = os.path.join(ramulator2_config_dir, ramulator2_confg_name)

                        ### Define necessary modifications
                        modifications = {
                            ## Basic info
                            "executable_name": champsim_bin,
                            "num_cores": num_cores,
                            ## Core
                            "ooo_cpu.branch_predictor": branch,
                            "L1D.prefetcher": 'ip_stride',  
                            ## LLC
                            "LLC.sets": num_sets,
                            "LLC.ways": num_ways,
                            "LLC.prefetcher": pref,
                            "LLC.replacement": llc_replacement,
                            ## Memory
                            "physical_memory.config_path": ramulator2_config_path
                        }

                        ModifyConfig(config, modifications)

                        ### Save the modified configuration
                        output_dir_base = "TPRAC_ISCA2025"
                        output_dir = os.path.join(output_dir_base, champsim_bin_dir)
                        os.system("mkdir -p "+output_dir)
                        output_file = os.path.join(output_dir_base, champsim_bin)
                        SaveConfig(config, output_file)

    print(f"All Configurations are Generated.")