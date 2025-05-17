import math
from concurrent.futures import ProcessPoolExecutor, as_completed
import os
import csv

##### WAVE Attack Analysis for TPRAC for Figure 7#####
NUM_tREFI = 8192 # 
tREFI1 = 3900 # tREFI for normal temperature (< 75C)
tREFI2 = 1950 # tREFI for high temperature (>=75C)
tRC = 52
tRFC1 = 295 # tRFC for 16Gb chip
tRFC2 = 410 # tRFC for 32/64Gb chips
ACT_RFM = 350 // tRC

def calc_required_parameters(is_normal_temp, is_16gb_chip):
    if is_normal_temp:
        if is_16gb_chip:
            act_tREFI = (tREFI1 - tRFC1) // tRC 
        else:
            act_tREFI = (tREFI1 - tRFC2) // tRC
    else:
        if is_16gb_chip:
            act_tREFI = (tREFI2 - tRFC1) // tRC
        else:
            act_tREFI = (tREFI2 - tRFC2) // tRC
    act_tREFW = act_tREFI * NUM_tREFI
    return act_tREFW

def attack_simulation(R1, act_TBRFM, act_tREFW):
    R_next = R1
    act_remain = act_tREFW
    N_RH = 0
    prev_unused_act = 0

    # print(f"Initial Conditions -> R1: {R1}, act_TBRFM: {act_TBRFM}, act_tREFW: {act_tREFW}")

    while act_remain > 0:
        act_for_mitigation = R_next+prev_unused_act
        num_mitigation_per_round = (act_for_mitigation) // act_TBRFM
        prev_unused_act = act_for_mitigation - num_mitigation_per_round * act_TBRFM
        act_remain -= (R_next + num_mitigation_per_round*ACT_RFM)# For each round, uniformly activate each member row
        R_next -= num_mitigation_per_round
        N_RH += 1

        # print(f"Step {N_RH}: act_remain={act_remain}, R_next={R_next}, prev_unused_act={prev_unused_act}, num_mitigation_per_round={num_mitigation_per_round}")

        if R_next <= 0:
            break

    # print(f"Final Result -> N_RH: {N_RH}")
    return N_RH

def sweep_act_TBRFM_per_chip(MAX_R1, is_16gb_chip, is_normal_temp, act_TBRFM):
    max_NRH = 0
    max_NRH_info = None
    
    act_tREFW = calc_required_parameters(is_normal_temp, is_16gb_chip)

    for R1 in range(1, MAX_R1):
        N_RH = attack_simulation(R1, act_TBRFM, act_tREFW)
        if N_RH < max_NRH:
            break

        if N_RH > max_NRH:
            max_NRH = N_RH
            max_NRH_info = (R1, act_TBRFM, max_NRH)
    
    return max_NRH_info
            

def parallel_sweep_simulations():
    chip_sizes = {
        # "16Gb": 64 * 1024,
        "32Gb": 128 * 1024,
        # "64Gb": 256 * 1024
    }
    
    act_TBRFM_values = {
        # "16Gb": [10, 12, 21, 26, 27, 43, 49, 50, 60, 62, 110, 127, 131, 243, 244, 261, 270, 530, 542, 547],
        "32Gb": [10, 12, 21, 26, 43, 49, 50, 60, 110, 127, 244, 261, 530, 544],
        # "64Gb": [10, 12, 21, 26, 43, 47, 50, 60, 110, 127, 244, 261, 530, 544]
    }
    
    # temperature_conditions = [True, False]  # True for normal temp, False for high temp
    temperature_conditions = [True]  # True for normal temp, False for high temp
    
    results = []

    with ProcessPoolExecutor() as executor:
        future_to_params = {}

        for chip, max_r1 in chip_sizes.items():
            for is_normal_temp in temperature_conditions:
                for act_TBRFM in act_TBRFM_values[chip]:
                    future = executor.submit(sweep_act_TBRFM_per_chip, max_r1, chip == "16Gb", is_normal_temp, act_TBRFM)
                    future_to_params[future] = (chip, is_normal_temp, act_TBRFM)

        for future in future_to_params:
            chip, is_normal_temp, act_TBRFM = future_to_params[future]
            try:
                result = future.result()
                if result is not None:
                    R1, _, max_NRH = result  # we already know act_TBRFM
                    results.append([chip, act_TBRFM, R1, max_NRH])
                    print(f"Chip: {chip}, Temp: {'Normal' if is_normal_temp else 'High'}, act_TBRFM: {act_TBRFM} -> R1: {R1}, max_NRH: {max_NRH}")
                else:
                    print(f"Chip: {chip}, Temp: {'Normal' if is_normal_temp else 'High'}, act_TBRFM: {act_TBRFM} -> No result")
            except Exception as e:
                print(f"Error in simulation for Chip: {chip}, Temp: {'Normal' if is_normal_temp else 'High'}, act_TBRFM: {act_TBRFM}: {e}")

        # Save results to CSV
        with open("results_wave_attack.csv", "w", newline="") as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(["Chip", "act_TBRFM", "R1", "max_NRH"])
            writer.writerows(results)



# Run the parallel simulations
parallel_sweep_simulations()


    
