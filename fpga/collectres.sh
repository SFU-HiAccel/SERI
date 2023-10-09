#!/bin/bash

# handle missing args
if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Error: Two arguments required. First for the job id of the xclbin builds, second for the range (e.g., 51-53)."
    exit 1
fi
build_id=$1
range=$2

IFS="-" read start end <<< "$range"
for i in $(seq $start $end); do
    echo "================== Processing task ID: $i =================="
    extracted_part=$(grep "BEST Implementation Run" "${PC2PFS}/hpc-prf-haqc/philip/build_out/build_dir_${build_id}_${i}/link/link.steps.log" | awk '{print $NF}')
    echo "Best impl strategy: ${extracted_part}"
    if [ -z "$extracted_part" ]; then
        extracted_part="impl_1"
    fi
    slr_util_rpt_path="${PC2PFS}/hpc-prf-haqc/philip/build_out/build_dir_${build_id}_${i}/reports/link/imp/${extracted_part}_slr_util_routed.rpt"
    echo "Using slr_util_rpt_path: ${slr_util_rpt_path}"

    /scratch/hpc-prf-haqc/philip/vitis_data_util/target/debug/build_collect "${build_id}" "$i" "${slr_util_rpt_path}"
done
