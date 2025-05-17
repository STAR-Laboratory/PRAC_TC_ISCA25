import yaml
import os
from calc_rh_parameters import *

def LoadConfig(file_path):
    """Load the default Ramulator2 configuration from a YAML file."""
    with open(file_path, 'r') as f:
        return yaml.safe_load(f)

def SaveConfig(config, output_file):
    """Save the modified YAML configuration to a new file."""
    with open(output_file + ".yaml", "w") as f:
        yaml.dump(config, f, default_flow_style=False)

def ModifyandSaveConfig(config, mitigation, NRH, PRAC_level):
    ## 1. Set Frontend
    config['Frontend']['clock_ratio'] = 1 # Change this based on DRAM interface
    config['Frontend']['impl'] = 'ChampSim'

    ## 2. Set Common Memory System Feature
    config['MemorySystem']['clock_ratio'] = 1 # Change this based on DRAM interface
    config['MemorySystem']['impl'] = 'BHDRAMSystem'
    config['MemorySystem']['AddrMapper']['impl'] = 'MOP4CLXOR'

    ## Memory Controller Configurations
    CONTROLLER = 'BHDRAMController'
    SCHEDULER  = 'BHScheduler'
    config['MemorySystem'][CONTROLLER]['RowPolicy']['impl'] = 'ClosedRowPolicy'
    config['MemorySystem'][CONTROLLER]['RowPolicy']['cap'] = 4

    ## DRAM Configurations
    dram_chip_capacity = '32Gb'
    dram_org_preset = 'DDR5_'+dram_chip_capacity+'_x8'
    dram_interface = 8000 # DRAM interface speed: 4800, 6400, 8000
    dram_timing_preset = 'DDR5_'+str(dram_interface)+'B'

    ### Set dram channels and ranks here
    dram_channels = 1
    dram_ranks = 4

    dram_base_info = str(dram_channels)+'CH_'+str(dram_ranks)+'RA_DDR5_'+dram_chip_capacity+'_'+str(dram_interface)

    config['MemorySystem']['DRAM']['PRAC'] = True
    config['MemorySystem']['DRAM']['timing']['preset'] = dram_timing_preset

    config['MemorySystem']['DRAM']['org']['preset'] = dram_org_preset
    config['MemorySystem']['DRAM']['org']['channel'] = dram_channels
    config['MemorySystem']['DRAM']['org']['rank'] = dram_ranks

    """Modify the configuration based on the provided dictionary of changes."""
    if mitigation == "Baseline":
        config['MemorySystem'][CONTROLLER]['impl'] = 'OPTDRAMController'
        config['MemorySystem'][CONTROLLER][SCHEDULER]['impl'] = 'BHScheduler'                
    else:
        config['MemorySystem'][CONTROLLER]['impl'] = 'PRACOPTDRAMController'
        config['MemorySystem'][CONTROLLER][SCHEDULER]['impl'] = 'PRACScheduler'

    if mitigation == "ABO_Only":
        NBO = get_abo_only_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
    elif mitigation == "ABO_RFM":
        NBO = get_abo_only_parameters(NRH)
        BAT = get_bat_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'BATBasedRFM',
                'bat': BAT,
                'rfm_type': 0,
                'enable_early_counter_reset': True
            }
        })
    elif mitigation == "TPRAC":
        NBO = get_abo_only_parameters(NRH)
        TB_RFM_window = get_tprac_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'TB_RFM_window': TB_RFM_window,
                'rfm_type': 0
            }
        })
    elif mitigation == "TPRAC-TREFpertREFI":
        NBO = get_abo_only_parameters(NRH)
        TB_RFM_window = get_tprac_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 1,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'TB_RFM_window': TB_RFM_window,
                'rfm_type': 0,
                'targeted_ref_ratio': 1
            }
        })
    elif mitigation == "TPRAC-TREFper2tREFI":
        NBO = get_abo_only_parameters(NRH)
        TB_RFM_window = get_tprac_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 2,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'TB_RFM_window': TB_RFM_window,
                'rfm_type': 0,
                'targeted_ref_ratio': 2
            }
        })
    elif mitigation == "TPRAC-TREFper3tREFI":
        NBO = get_abo_only_parameters(NRH)
        TB_RFM_window = get_tprac_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 3,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'TB_RFM_window': TB_RFM_window,
                'rfm_type': 0,
                'targeted_ref_ratio': 3
            }
        })
    elif mitigation == "TPRAC-TREFper4tREFI":
        NBO = get_abo_only_parameters(NRH)
        TB_RFM_window = get_tprac_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 4,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'TB_RFM_window': TB_RFM_window,
                'rfm_type': 0,
                'targeted_ref_ratio': 4
            }
        })

    elif mitigation == "TPRAC-NoReset":
        NBO = get_abo_only_parameters(NRH)
        TB_RFM_window = get_tprac_noreset_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 0,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'TB_RFM_window': TB_RFM_window,
                'rfm_type': 0
            }
        })
    elif mitigation == "TPRAC-NoReset-TREFpertREFI":
        NBO = get_abo_only_parameters(NRH)
        TB_RFM_window = get_tprac_noreset_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 1,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'TB_RFM_window': TB_RFM_window,
                'rfm_type': 0,
                'targeted_ref_ratio': 1
            }
        })
    elif mitigation == "TPRAC-NoReset-TREFper2tREFI":
        NBO = get_abo_only_parameters(NRH)
        TB_RFM_window = get_tprac_noreset_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 2,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'TB_RFM_window': TB_RFM_window,
                'rfm_type': 0,
                'targeted_ref_ratio': 2
            }
        })
    elif mitigation == "TPRAC-NoReset-TREFper4tREFI":
        NBO = get_abo_only_parameters(NRH)
        TB_RFM_window = get_tprac_noreset_parameters(NRH)
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'QPRAC',
                'abo_delay_acts': PRAC_level,
                'abo_recovery_refs': PRAC_level,
                'abo_act_ns': 180,
                'abo_threshold': NBO,
                'psq_size': 5,
                'targeted_ref_frequency': 4,
                'enable_opportunistic_mitigation': True
            }
        })
        config['MemorySystem'][CONTROLLER]['plugins'].append({
            'ControllerPlugin' : {
                'impl': 'TimingBasedRFM',
                'TB_RFM_window': TB_RFM_window,
                'rfm_type': 0,
                'targeted_ref_ratio': 4
            }
        })

    ramulator2_config_name_base = f"{dram_base_info}"
    if mitigation == "Baseline":
        ramulator2_config_name = f"{ramulator2_config_name_base}-{mitigation}"
    else:
        ramulator2_config_name = f"{ramulator2_config_name_base}-{mitigation}-{NRH}-PRAC{PRAC_level}"
    ramulator2_config_path = os.path.join(ramulator2_config_dir, ramulator2_config_name)
    ### Save the modified configuration
    SaveConfig(config, ramulator2_config_path)

if __name__ == "__main__":
    # Load the default configuration
    default_config_path = './DDR5_baseline_closed_mitigation.yaml'
    ## 1. Define output directory
    project_name = 'TPRAC_ISCA2025'
    ramulator2_config_dir = os.path.join(os.getcwd(), project_name)
    os.makedirs(ramulator2_config_dir, exist_ok=True)
    ## 2. Set required parameters: Ex) RowHammer thresholds, mitigation lists
    # mitigation_lists = ['Baseline', 'ABO_Only', 'ABO_RFM', 'TPRAC']
    # mitigation_lists = ['TPRAC-TREFper4tREFI', 'TPRAC-TREFper3tREFI', 'TPRAC-TREFper2tREFI', 'TPRAC-TREFpertREFI']
    mitigation_lists = ['TPRAC-NoReset', 'TPRAC-NoReset-TREFper4tREFI', 'TPRAC-NoReset-TREFper2tREFI', 'TPRAC-NoReset-TREFpertREFI']
    NRH_lists = [128, 256, 512, 1024, 2048, 4096]
    # PRAC_level_lists = [1, 2, 4]    
    PRAC_level_lists = [1]    

    for mitigation in mitigation_lists:
        for NRH in NRH_lists:
            for PRAC_level in PRAC_level_lists:
                if mitigation == "Baseline" and NRH != 1024 and PRAC_level != 1:
                    continue
                if PRAC_level != 1 and NRH != 1024:
                    continue
                config = LoadConfig(default_config_path)
                ModifyandSaveConfig(config,mitigation, NRH, PRAC_level)

    print(f"All Configurations are Generated.")