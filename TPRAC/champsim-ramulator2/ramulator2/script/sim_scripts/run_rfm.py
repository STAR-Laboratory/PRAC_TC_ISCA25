import os, yaml, copy, itertools

ramulator_path= "/scratch/st-prashnr-1/jeonghyun/ramulator2"
project_path = ramulator_path + "/prac_study"

base_config_path = project_path + "/configs"
base_config_file = base_config_path + "/DDR5_baseline_closed_mitigation.yaml"
base_config = None
with open(base_config_file, 'r') as stream:
    try:
        base_config = yaml.safe_load(stream)
    except yaml.YamlError as exc:
        print(exc)
if(base_config == None):
    print("Error: base config is None")
    exit(1)


output_path = project_path +  "/rfm_results/optimized_MC/results_4cores/"
trace_path = ramulator_path + "/cputraces"

# the command line slurm will execute
SRUN_COMMAND_LINE = "\
    srun --cpus-per-task=1 --nodes=1 --ntasks=1 \
    --time=48:00:00 \
    --mem=6000M \
    --chdir={ramulator_dir} \
    --output={output_file_name} \
    --account=st-prashnr-1 \
    --error={error_file_name} \
    --partition=skylake \
    --mail-user=jhwoo36@ece.ubc.ca \
    --mail-type=FAIL \
    --job-name='{job_name}' \
    {ramulator_dir}/build/ramulator2 -f {config_name}"

traces = [
#   "random_10.trace",
#   "stream_10.trace",
#   "401.bzip2",
#   "403.gcc",
#   "429.mcf",
#   "433.milc",
#   "434.zeusmp",
#   "435.gromacs",
#   "436.cactusADM",
#   "437.leslie3d",
#   "444.namd",
#   "445.gobmk",
#   "447.dealII",
#   "450.soplex",
#   "456.hmmer",
#   "458.sjeng",
#   "459.GemsFDTD",
#   "462.libquantum",
#   "464.h264ref",
#   "470.lbm",
#   "471.omnetpp",
#   "473.astar",
#   "481.wrf",
#   "482.sphinx3",
#   "483.xalancbmk",
#   "500.perlbench",
#   "502.gcc",
#   "505.mcf",
#   "507.cactuBSSN",
#   "508.namd",
#   "510.parest",
#   "511.povray",
#   "519.lbm",
#   "520.omnetpp",
#   "523.xalancbmk",
#   "525.x264",
#   "526.blender",
#   "531.deepsjeng",
#   "538.imagick",
#   "541.leela",
#   "544.nab",
#   "549.fotonik3d",
#   "557.xz",
# #   #"bfs_dblp",
# #   #"bfs_cm2003",
# #   #"bfs_ny",
  "grep_map0",
# #   #"h264_decode",
#   "h264_encode",
#   "jp2_decode",
#   "jp2_encode",
#   "tpcc64",
#   "tpch17",
#   "tpch2",
#   "tpch6",
#   "wc_8443", # wordcount-8443
#   "wc_map0", # wordcount-map0
#   "ycsb_abgsave",
#   "ycsb_aserver",
#   "ycsb_bserver",
#   "ycsb_cserver",
#   "ycsb_dserver",
#   "ycsb_eserver"
]

for workload in traces:
    # for mitigation in ["RFMsb-Reset", "RFMab-Reset"]:
    # for mitigation in ["RFMab-Reset"]:
    # for mitigation in ["Baseline", "Baseline-RoBaRaCoCh"]:
    # for mitigation in ["RFMsb", "RFMpb"]:
    for mitigation in ["RFMsb"]:
        #  for BAT in [140,100,70,40]:
        #  for BAT in [70,40,10,5,2,1]:
         for BAT in [1]:
            output_path_final = output_path
            for path in [output_path_final + "/" + mitigation + "/stats", output_path_final + "/" + mitigation + "/configs", output_path_final + "/" + mitigation + "/cmd_count", output_path_final + "/" + mitigation + "/dram_trace", output_path_final + "/" + mitigation + "/errors"]:
                if not os.path.exists(path):
                    os.makedirs(path)

            result_filename = output_path_final + "/" + mitigation + "/stats/" + str(BAT) + "_" + workload + ".txt"
            config_filename = output_path_final + "/" + mitigation + "/configs/" + str(BAT) + "_" + workload + ".yaml"
            cmd_count_filename = output_path_final + "/" + mitigation + "/cmd_count/" + str(BAT) + "_" + workload + ".cmd.count"
            dram_trace_filename = output_path_final + "/" + mitigation + "/dram_trace/" + str(BAT) + "_" + workload + ".dram.trace"
            error_filename = output_path_final + "/" + mitigation + "/errors/" + str(BAT) + "_" +  workload + ".err"
            if mitigation in ["Baseline", "Baseline-RoBaRaCoCh"]:
                result_filename = output_path_final + "/" + mitigation + "/stats/"+ workload + ".txt"
                config_filename = output_path_final + "/" + mitigation + "/configs/" + workload + ".yaml"
                cmd_count_filename = output_path_final + "/" + mitigation + "/cmd_count/" +  workload + ".cmd.count"
                dram_trace_filename = output_path_final + "/" + mitigation + "/dram_trace/"+  workload + ".dram.trace"
                error_filename = output_path_final + "/" + mitigation + "/errors/"+  workload + ".err"
                # only for baseline and one Trh
                act_profile_filename = output_path_final + "/" + mitigation + "/act_count/" + workload + ".act.count"

            config = copy.deepcopy(base_config)

            result_file = open(result_filename, "w")
            config_file = open(config_filename, "w")

            ###### Currently hardcoded for 4-core simulations ######
            workload_name_list = [workload] * 4
            #print(workload_name_list)
            workload_name_list_dir = [(trace_path + "/" + x) for x in workload_name_list]
            #print(workload_name_list_dir)

            config['MemorySystem']['DRAM']['impl'] = 'DDR5-PRAC'
            config['MemorySystem']['BHDRAMController']['impl'] = 'OPTDRAMController'
            # config['MemorySystem']['BHDRAMController']['impl'] = 'BHDRAMController'
            config['MemorySystem']['BHDRAMController']['BHScheduler']['impl'] = 'BHScheduler'
            config['Frontend']['traces'] = workload_name_list_dir
            config['Frontend']['num_expected_insts'] = int(2000000000/len(workload_name_list))
            #config['Frontend']['num_max_cycles'] = 3000000000
            #config['MemorySystem']['AddrMapper']['impl'] = 'RoBaRaCoCh_with_rit'
            config['MemorySystem']['AddrMapper']['impl'] = 'MOP4CLXOR'
            config['MemorySystem']['BHDRAMController']['plugins'][0]['ControllerPlugin']['path'] = cmd_count_filename
            if(mitigation == "RFMsb"):
                config['MemorySystem']['BHDRAMController']['plugins'].append({'ControllerPlugin' : {'impl': 'BATBasedRFM', 'bat': BAT, 'rfm_type': 1, 'enable_early_counter_reset': 'false'}})
            elif(mitigation == "RFMpb"):
                config['MemorySystem']['BHDRAMController']['plugins'].append({'ControllerPlugin' : {'impl': 'BATBasedRFM', 'bat': BAT, 'rfm_type': 2, 'enable_early_counter_reset': 'false'}})
            elif(mitigation == "RFMab-Reset"):
                config['MemorySystem']['BHDRAMController']['plugins'].append({'ControllerPlugin' : {'impl': 'BATBasedRFM', 'bat': BAT, 'rfm_type': 0, 'enable_early_counter_reset': 'true'}})
            elif(mitigation == "RFMsb-Reset"):
                config['MemorySystem']['BHDRAMController']['plugins'].append({'ControllerPlugin' : {'impl': 'BATBasedRFM', 'bat': BAT, 'rfm_type': 1, 'enable_early_counter_reset': 'true'}})
            elif(mitigation == "Baseline-RoBaRaCoCh"):
                config['MemorySystem']['AddrMapper']['impl'] = 'RoBaRaCoCh_with_rit'

            # Prepare the Slurm command
            job_name = f"{mitigation}-{BAT}_{workload}"
            SRUN_CMD = SRUN_COMMAND_LINE.format(
                ramulator_dir=ramulator_path,
                output_file_name=result_filename,
                error_file_name=error_filename,
                job_name=job_name,
                config_name=config_filename,
            )
            # For larger memory requirements
            if workload in ["bfs_dblp", "bfs_ny", "bfs_cm2003", "wc_8443", "wc_map0", "random_10.trace", "429.mcf", "519.lbm"]:
                SRUN_CMD = SRUN_CMD.replace("--mem=6000M", "--mem=24G")

            yaml.dump(config, config_file, default_flow_style=False)
            config_file.close()
            result_file.write(SRUN_CMD + "\n")
            result_file.close()

            # print(SRUN_CMD)
            print("Running: " + str(len(workload_name_list))+ " homogenous traces = " + workload + ", mitigation = " + mitigation + ", BAT = " + str(BAT))
            os.system(SRUN_CMD +" &")

