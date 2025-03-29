import os, yaml, copy, itertools


ramulator_path= "/scratch/st-prashnr-1/jeonghyun/PRAC/Ramulator2-PRAC"
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

NUM_RANKS = 4
NUM_INSRUCTIONS = 100_000_000

output_path = project_path +  "/TimingOverhead/SC/"+str(NUM_RANKS)+"R_per_CH/100M/MOP/Closed"

trace_path = ramulator_path + "/cputraces"

#############TODO: Change the ramulator path
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
    {ramulator_dir}/ramulator2 -f {config_name}"


traces = [
#   "random_10.trace",
#   "stream_10.trace",
  "401.bzip2",
  "403.gcc",
  "429.mcf",
  "433.milc",
  "434.zeusmp",
  "435.gromacs",
  "436.cactusADM",
  "437.leslie3d",
  "444.namd",
  "445.gobmk",
  "447.dealII",
  "450.soplex",
  "456.hmmer",
  "458.sjeng",
  "459.GemsFDTD",
  "462.libquantum",
  "464.h264ref",
  "470.lbm",
  "471.omnetpp",
  "473.astar",
  "481.wrf",
  "482.sphinx3",
  "483.xalancbmk",
  "500.perlbench",
  "502.gcc",
  "505.mcf",
  "507.cactuBSSN",
  "508.namd",
  "510.parest",
  "511.povray",
  "519.lbm",
  "520.omnetpp",
  "523.xalancbmk",
  "525.x264",
  "526.blender",
  "531.deepsjeng",
  "538.imagick",
  "541.leela",
  "544.nab",
  "549.fotonik3d",
  "557.xz",
#   "bfs_dblp",
#   "bfs_cm2003",
#   "bfs_ny",
  "grep_map0",
  "h264_decode",
  "h264_encode",
  "jp2_decode",
  "jp2_encode",
  "tpcc64",
  "tpch17",
  "tpch2",
  "tpch6",
  "wc_8443", # wordcount-8443
  "wc_map0", # wordcount-map0
  "ycsb_abgsave",
  "ycsb_aserver",
  "ycsb_bserver",
  "ycsb_cserver",
  "ycsb_dserver",
  "ycsb_eserver"
]

# trace_combination_filename = "multicore_traces.txt"

# trace_names = []
# with open(trace_combination_filename, "r") as trace_combination_file:
#     for line in trace_combination_file:
#         line = line.strip()
#         if(line == ""):
#             continue
#         trace_list = line.split(",")[1:]
#         trace_names += trace_list

# trace_names = list(set(trace_names))


for workload in traces:
    for mitigation in ["PRAC_WO_Mitigation", "Baseline"]:
    # for mitigation in ["PRAC_WO_Mitigation"]:
        # for num_channel in [1, 2, 4, 8, 16]:
        for num_channel in [1]:
            for path in [output_path + "/" + mitigation + "/stats", output_path + "/" + mitigation + "/configs", output_path + "/" + mitigation + "/cmd_count", output_path + "/" + mitigation + "/dram_trace", output_path + "/" + mitigation + "/errors"]:
                if not os.path.exists(path):
                    os.makedirs(path)

            result_filename = output_path + "/" + mitigation + "/stats/" + str(num_channel) + "_" + workload + ".txt"
            config_filename = output_path + "/" + mitigation + "/configs/" + str(num_channel) + "_" + workload + ".yaml"
            cmd_count_filename = output_path + "/" + mitigation + "/cmd_count/" + str(num_channel) + "_" + workload + ".cmd.count"
            dram_trace_filename = output_path + "/" + mitigation + "/dram_trace/" + str(num_channel) + "_" + workload + ".dram.trace"
            # only for baseline and one num_channel
            act_profile_filename = output_path + "/" + mitigation + "/act_count/" + str(num_channel) + "_" + workload + ".act.count"
            error_filename = output_path + "/" + mitigation + "/errors/" + str(num_channel) +"_" + workload + ".err"

            config = copy.deepcopy(base_config)

            result_file = open(result_filename, "w")
            config_file = open(config_filename, "w")

            ###### Currently hardcoded for 4-core simulations ######
            workload_name_list = [workload] * 4
            #print(workload_name_list)
            workload_name_list_dir = [(trace_path + "/" + x) for x in workload_name_list]
            #print(workload_name_list_dir)


            config['MemorySystem']['DRAM']['impl'] = 'DDR5-PRAC'
            config['Frontend']['traces'] = workload_name_list_dir
            config['Frontend']['num_expected_insts'] = int(NUM_INSRUCTIONS)
            # config['Frontend']['num_max_cycles'] = 3000000000
            config['Frontend']['Translation']['max_addr'] = int(32*1024*1024*1024*int(NUM_RANKS)*int(num_channel))
            #### Set here if we want to test different DRAM modules
            config['MemorySystem']['DRAM']['org']['preset'] = 'DDR5_32Gb_x8'

            config['MemorySystem']['BHDRAMController']['impl'] = 'BLOCKHAMMERDRAMController'
            # config['MemorySystem']['BHDRAMController']['impl'] = 'BHDRAMController'
            config['MemorySystem']['BHDRAMController']['BHScheduler']['impl'] = 'BHScheduler'

            config['MemorySystem']['AddrMapper']['impl'] = 'MOP4CLXOR'

            ## Set Page policy here
            # config['MemorySystem']['BHDRAMController']['RowPolicy']['impl'] = 'OpenRowPolicy'
            config['MemorySystem']['BHDRAMController']['RowPolicy']['impl'] = 'ClosedRowPolicy'

            ## Set cap for closed page here
            if config['MemorySystem']['BHDRAMController']['RowPolicy']['impl'] == 'ClosedRowPolicy':
                config['MemorySystem']['BHDRAMController']['RowPolicy']['cap'] = 1
                # config['MemorySystem']['BHDRAMController']['RowPolicy']['cap'] = 4

            ### Set # of Channels and Ranks Here
            config['MemorySystem']['DRAM']['org']['channel'] = num_channel
            config['MemorySystem']['DRAM']['org']['rank'] = NUM_RANKS
            config['MemorySystem']['BHDRAMController']['plugins'][0]['ControllerPlugin']['path'] = cmd_count_filename
            if(mitigation == "PRAC_WO_Mitigation"):
                config['MemorySystem']['DRAM']['PRAC'] = True
            elif(mitigation == "Baseline"):
                config['MemorySystem']['BHDRAMController']['plugins'].append({'ControllerPlugin' : {'impl': 'ActProfile', 'path': act_profile_filename}})

            # Prepare the Slurm command
            job_name = f"{mitigation}-{num_channel}_{workload}"
            SRUN_CMD = SRUN_COMMAND_LINE.format(
                ramulator_dir=ramulator_path,
                output_file_name=result_filename,
                error_file_name=error_filename,
                job_name=job_name,
                config_name=config_filename,
            )
            # For larger memory requirements
            if workload in ["wc_8443", "wc_map0", "random_10.trace", "stream_10.trace", "429.mcf", "519.lbm", "h264_decode","bfs_dblp","bfs_cm2003",  "bfs_ny"]:
                SRUN_CMD = SRUN_CMD.replace("--mem=6000M", "--mem=24G")



            yaml.dump(config, config_file, default_flow_style=False)
            config_file.close()
            result_file.write(SRUN_CMD + "\n")
            result_file.close()

            # print(cmd)
            print("Running: " + str(len(workload_name_list))+ " homogenous traces = " + workload + ", mitigation = " + mitigation + ", num_channel = " + str(num_channel))
            os.system(SRUN_CMD +" &")
