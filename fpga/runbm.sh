#!/bin/bash
#SBATCH -A hpc-prf-haqc
#SBATCH --job-name=qcv-bm
#SBATCH -p fpga
#SBATCH --constraint=xilinx_u280_xrt2.16
#SBATCH -t 1:00:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --mem=32G
#SBATCH --output=bm_log_%A.out

# handle missing args
if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Error: Two arguments required. First for the job id of the xclbin builds, second for the range (e.g., 51-53)."
    exit 1
fi
job_id=$1
range=$2

# configuration
build_dir="/dev/shm/build_${SLURM_JOB_ID}/"
source_dir="${SLURM_SUBMIT_DIR}"
res_dir="${PC2PFS}/hpc-prf-haqc/philip/qc_fpga/res/"
output_dir="${PC2PFS}/hpc-prf-haqc/philip/build_out/bm_dir_${SLURM_JOB_ID}/"

# load modules and configure environment
module reset
module load fpga
module load fpga/xilinx/xrt/2.16
export TMPDIR="/dev/shm"
export SLURM_TMPDIR="/dev/shm"
export XILINX_LOCAL_USER_DATA="no" # to prevent "Failed to install all user apps" error

# setup project files
rm -rf "${build_dir}"
mkdir -p "${build_dir}"
cp -r "${source_dir}" "${build_dir}"
cd "${build_dir}" || { echo "Failed to change directory to ${build_dir}"; exit 1; }
# if cp didn't directly copy contents, move into dir
# shellcheck disable=SC2164
cd fpga

# define arguments array: QUARTET_ARGS_LIST
source canonical_quartet_args.sh

# execute
cd src || { echo "Failed to change directory to src/"; exit 1; }

# prepare output directory
mkdir -p "${output_dir}"

IFS="-" read start end <<< "$range"
for i in $(seq $start $end); do
    echo "================== Processing task ID: $i =================="
    xclbin_path="${PC2PFS}/hpc-prf-haqc/philip/build_out/build_dir_${job_id}_${i}/build_dir.hw.xilinx_u280_gen3x16_xdma_1_202211_1/qc_fpga.xclbin"
    echo "Using xclbin: ${xclbin_path}"

    make -B host ${QUARTET_ARGS_LIST[$i]}
    am_abcd_val=$(echo "${QUARTET_ARGS_LIST[$i]}" | sed -n 's/.*AM_ABCD=\([0-9]*\).*/\1/p')
    ./qc_fpga --xclbin "${xclbin_path}" -b --mob "${res_dir}/mobs/synmol_med.mob" --ref "${res_dir}/refs/ref_synmol_med_${am_abcd_val}.bin"

    rm "power_data_$i.txt"
    (
        while true; do
            xilinx_power >> "power_data_$i.txt"
            sleep 0.5
        done
    ) &
    loop_pid=$!
    ./qc_fpga --xclbin "${xclbin_path}" --mob "${res_dir}/mobs/synmol_med.mob" -b -d 6
    kill loop_pid
    cp ./power_data* "${output_dir}"

    cat "bm_results_${am_abcd_val}.json"
    echo "Power Consumption: (Watts)"
    awk '{print $2}' "power_data_$i.txt" | sed 's/W//' | sort -n | tail -1

    /scratch/hpc-prf-haqc/philip/vitis_data_util/target/debug/bm_collect "${job_id}" "${SLURM_JOB_ID}" "$i" "${xclbin_path}" "bm_results_${am_abcd_val}.json" "power_data_$i.txt"
done

# copy results
cp ./*.bin "${output_dir}"
