#!/bin/bash
# Base Zenodo link for the files
BASE_URL="https://zenodo.org/records/15104056/files"

# Target download directory
TARGET_DIR="traces"

# Create directory if it doesn't exist
mkdir -p "$TARGET_DIR"

# Number of concurrent downloads
# TODO: Change this line to get parameter from run_artifact.sh
MAX_JOBS=10

# List of all trace files to download
FILES=(
401.bzip2-38B.champsimtrace.xz
403.gcc-16B.champsimtrace.xz
410.bwaves-1963B.champsimtrace.xz
416.gamess-875B.champsimtrace.xz
429.mcf-22B.champsimtrace.xz
433.milc-274B.champsimtrace.xz
435.gromacs-111B.champsimtrace.xz
436.cactusADM-1804B.champsimtrace.xz
437.leslie3d-134B.champsimtrace.xz
444.namd-120B.champsimtrace.xz
445.gobmk-17B.champsimtrace.xz
447.dealII-3B.champsimtrace.xz
450.soplex-247B.champsimtrace.xz
453.povray-252B.champsimtrace.xz
454.calculix-104B.champsimtrace.xz
456.hmmer-191B.champsimtrace.xz
458.sjeng-1088B.champsimtrace.xz
459.GemsFDTD-1169B.champsimtrace.xz
464.h264ref-30B.champsimtrace.xz
465.tonto-1914B.champsimtrace.xz
470.lbm-1274B.champsimtrace.xz
471.omnetpp-188B.champsimtrace.xz
473.astar-153B.champsimtrace.xz
481.wrf-1170B.champsimtrace.xz
482.sphinx3-1100B.champsimtrace.xz
483.xalancbmk-127B.champsimtrace.xz
600.perlbench_s-1273B.champsimtrace.xz
602.gcc_s-1850B.champsimtrace.xz
603.bwaves_s-1080B.champsimtrace.xz
605.mcf_s-1152B.champsimtrace.xz
607.cactuBSSN_s-2421B.champsimtrace.xz
619.lbm_s-2676B.champsimtrace.xz
620.omnetpp_s-141B.champsimtrace.xz
621.wrf_s-575B.champsimtrace.xz
623.xalancbmk_s-325B.champsimtrace.xz
625.x264_s-33B.champsimtrace.xz
627.cam4_s-490B.champsimtrace.xz
628.pop2_s-17B.champsimtrace.xz
631.deepsjeng_s-928B.champsimtrace.xz
638.imagick_s-4128B.champsimtrace.xz
641.leela_s-1052B.champsimtrace.xz
644.nab_s-12459B.champsimtrace.xz
648.exchange2_s-1227B.champsimtrace.xz
649.fotonik3d_s-10881B.champsimtrace.xz
654.roms_s-1007B.champsimtrace.xz
657.xz_s-2302B.champsimtrace.xz
cassandra_core0.trace.xz
cassandra_core1.trace.xz
cassandra_core2.trace.xz
cassandra_core3.trace.xz
classification_core0.trace.xz
classification_core1.trace.xz
classification_core2.trace.xz
classification_core3.trace.xz
cloud9_core0.trace.xz
cloud9_core1.trace.xz
cloud9_core2.trace.xz
cloud9_core3.trace.xz
nutch_core0.trace.xz
nutch_core1.trace.xz
nutch_core2.trace.xz
nutch_core3.trace.xz
)

echo "Start downloading Traces"
# Download in parallel
job_count=0
for file in "${FILES[@]}"; do
    wget -c "$BASE_URL/$file" -O "$TARGET_DIR/$file" &

    ((job_count++))
    if (( job_count % MAX_JOBS == 0 )); then
        wait  # Wait for all background jobs to finish
    fi
done

wait  # Final wait to ensure all downloads complete
echo "✅ All trace files downloaded to $TARGET_DIR/"
