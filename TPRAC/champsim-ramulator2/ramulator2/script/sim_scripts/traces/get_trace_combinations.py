import random

## Workload RBMPKI Information
# workload,Channel,Baseline,PRAC_WO_Mitigation
# 401.bzip2,1.0,1.017875993892744,1.019357993883852
# 403.gcc,1.0,0.18103799927584802,0.18117799927528802
# 429.mcf,1.0,72.84213770863145,72.85809570856762
# 433.milc,1.0,16.07225990356644,16.127553903234677
# 434.zeusmp,1.0,35.99790178401259,36.27140385491439
# 435.gromacs,1.0,0.44349,0.443268
# 436.cactusADM,1.0,5.705013965769916,5.7506799654959195
# 437.leslie3d,1.0,11.971697952113209,11.86556795253773
# 444.namd,1.0,0.023954,0.024004
# 445.gobmk,1.0,0.528738,0.529504
# 447.dealII,1.0,0.0274,0.027538
# 450.soplex,1.0,16.94111993223552,16.99557793201769
# 456.hmmer,1.0,0.830491998339016,0.832335998335328
# 458.sjeng,1.0,0.533695997865216,0.533749997865
# 459.GemsFDTD,1.0,22.644883909420464,22.891042
# 462.libquantum,1.0,9.493653962025384,9.671095961315617
# 464.h264ref,1.0,0.13505999972988,0.135221999729556
# 470.lbm,1.0,34.787776,34.782102
# 471.omnetpp,1.0,10.721067935673592,10.732311935606127
# 473.astar,1.0,8.49248998301502,8.52552998294894
# 481.wrf,1.0,0.016937999898372,0.017061999897628
# 482.sphinx3,1.0,6.823062,6.87302
# 483.xalancbmk,1.0,11.972543976054911,11.989831976020337
# 500.perlbench,1.0,1.52120999087274,1.5227979908632119
# 502.gcc,1.0,0.8760199982479601,0.8769819982460361
# 505.mcf,1.0,4.649995981400016,4.662447981350208
# 507.cactuBSSN,1.0,3.89546998441812,3.896084
# 508.namd,1.0,0.289371999421256,0.290023999419952
# 510.parest,1.0,6.3758179872483645,6.396361987207277
# 511.povray,1.0,0.00745999997016,0.007487999970048001
# 519.lbm,1.0,30.852062,30.887364
# 520.omnetpp,1.0,12.198175975603649,12.214127975571744
# 523.xalancbmk,1.0,1.34191,1.342386
# 525.x264,1.0,0.369687997781872,0.369821997781068
# 526.blender,1.0,0.194766,0.195184
# 531.deepsjeng,1.0,0.43705599825177605,0.437077998251688
# 538.imagick,1.0,0.009415999962336,0.009415999962336
# 541.leela,1.0,0.0094499999433,0.009471999943168
# 544.nab,1.0,0.17122999931508,0.17128999931484
# 549.fotonik3d,1.0,23.116573953766853,23.52720595294559
# 557.xz,1.0,3.6645659853417363,3.6657339853370643
# bfs_cm2003,1.0,56.074534,54.502114
# bfs_ny,1.0,53.81044767713731,52.23596379105615
# grep_map0,1.0,2.7544779944910442,2.69091999461816
# h264_decode,1.0,94.40612181118776,91.79423563282306
# h264_encode,1.0,0.028151999943696,0.028085999943828002
# jp2_decode,1.0,5.160271989679456,5.099655989800688
# jp2_encode,1.0,3.130773993738452,3.1061499937877
# stream_10.trace,1.0,23.082223907671107,23.07376590770494
# tpcc64,1.0,2.843177994313644,2.8630119942739762
# tpch17,1.0,5.545953977816184,5.5675479777298085
# tpch2,1.0,7.806702,7.847774
# tpch6,1.0,1.049057993705652,1.052003993687976
# wc_8443,1.0,5.30225,5.216746
# wc_map0,1.0,5.263707989472584,5.1770179896459645
# ycsb_abgsave,1.0,1.062587995749648,1.0664419957342322
# ycsb_aserver,1.0,2.827821983033068,2.8334999829989997
# ycsb_bserver,1.0,1.853177996293644,1.860915996278168
# ycsb_cserver,1.0,1.741075996517848,1.747363996505272
# ycsb_dserver,1.0,1.487531991074808,1.491953991048276
# ycsb_eserver,1.0,2.058011995883976,2.0610119958779762




num_samples_per_group = 15

output_filename = "4cores_traces.txt"
output_file = open(output_filename, "w")

### For 8-core simulations
# group_list = ["HHHHHHHH", "MMMMMMMM", "LLLLLLLL", 
#               "HHHHMMMM", "HHHHLLLL", "MMMMLLLL",
#               "HHHHMMLL", "HHMMMMLL", "HHMMLLLL"]

### For 4-core simulations
group_list = ['HHHH', 'MMMM', 'LLLL',
              'HHMM', 'HHLL', 'MMLL']

high_traces = ["429.mcf", "433.milc", "434.zeusmp", "437.leslie3d", "450.soplex", 
                "459.GemsFDTD", "470.lbm", "471.omnetpp", "483.xalancbmk", "519.lbm", 
                "520.omnetpp", "549.fotonik3d"]  # RBMPKI >=10 -- 12 / 57 workloads
                
medium_traces = ["436.cactusADM", "462.libquantum", "473.astar", "482.sphinx3", 
                  "505.mcf", "507.cactuBSSN", "510.parest", "557.xz", "grep_map0",
                  "jp2_decode", "jp2_encode", "tpcc64", "tpch17", "tpch2",
                  "wc_8443", "wc_map0", "ycsb_aserver", "ycsb_eserver"] # 2 <= RBMPKI <10 -- 18 / 57 workloads

low_traces = ["401.bzip2", "403.gcc", "435.gromacs",  "444.namd",
               "445.gobmk", "447.dealII", "456.hmmer", "458.sjeng", "464.h264ref",
                "481.wrf", "500.perlbench", "502.gcc", "508.namd", "511.povray",  
                "523.xalancbmk", "525.x264", "526.blender", "531.deepsjeng", 
                "538.imagick", "541.leela", "544.nab", 
                "h264_encode", "tpch6",  "ycsb_abgsave",
                "ycsb_bserver", "ycsb_cserver", "ycsb_dserver"] # 0 <= RBMPKI < 2 -- 27 / 57 workloads

trace_num = 0
for group in group_list:
    num_h = group.count("H")
    num_m = group.count("M")
    num_l = group.count("L")

    for i in range(num_samples_per_group):
        highs = random.sample(high_traces, num_h)
        mids = random.sample(medium_traces, num_m)
        lows = random.sample(low_traces, num_l)

        traces = highs + mids + lows
        output_file.write("MIX"+str(trace_num)+",")
        output_file.write(group+ ",")
        output_file.write(",".join(traces) + "\n")

        trace_num += 1
