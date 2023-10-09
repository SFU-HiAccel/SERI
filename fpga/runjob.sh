#!/bin/bash
#SBATCH -A hpc-prf-haqc
#SBATCH --job-name=qcv-bld
#SBATCH -p normal
#SBATCH --array=0-54
#SBATCH -t 14:00:00
#SBATCH --cpus-per-task=10
#SBATCH --mem-per-cpu=8G
#SBATCH --output=build_log_%A_%a.out

# configuration
build_dir="/dev/shm/build_${SLURM_ARRAY_TASK_ID}/"
source_dir="${SLURM_SUBMIT_DIR}"
output_dir="${PC2PFS}/hpc-prf-haqc/philip/build_out/build_dir_${SLURM_ARRAY_JOB_ID}_${SLURM_ARRAY_TASK_ID}/"

# load modules and configure environment
module reset
module load fpga
module load fpga/xilinx/vivado/23.2
module load fpga/xilinx/vitis/23.2
module load fpga/xilinx/xrt/2.16
module load fpga/xilinx/u280/xdma_202211_1
export TMPDIR="/dev/shm"
export SLURM_TMPDIR="/dev/shm"
export XILINX_LOCAL_USER_DATA="no" # to prevent "Failed to install all user apps" error

# setup project files
rm -rf "${build_dir}"
mkdir -p "${build_dir}"
cp -r "${source_dir}"/* "${build_dir}"
cd "${build_dir}" || { echo "Failed to change directory to ${build_dir}"; exit 1; }

# define arguments array: QUARTET_ARGS_LIST
source canonical_quartet_args.sh

# execute
cd src || { echo "Failed to change directory to src/"; exit 1; }

if [ "$1" = "v++" ]; then
  # shellcheck disable=SC2086
  time make build TARGET=hw BUILD_THREADS=10 ${QUARTET_ARGS_LIST[$SLURM_ARRAY_TASK_ID]}
elif [ "$1" = "tapa" ]; then
  # setup additional dependencies
  module load lang
  module load Anaconda3
  module load lang/Python/3.9.5-GCCcore-10.3.0
  module load compiler
  module load compiler/Clang/12.0.1-GCCcore-10.3.0

  # override llvm and clang include paths for tapa
  export TAPA_LLVM_INCLUDE_PATH=/opt/software/pc2/EB-SW/software/Clang/12.0.1-GCCcore-10.3.0/include/c++/v1/
  export TAPA_CLANG_INCLUDE_INCLUDE_PATH=
  export TAPA_CLANG_LIB_INCLUDE_PATH=/opt/software/pc2/EB-SW/software/Clang/12.0.1-GCCcore-10.3.0/lib/clang/12.0.1/include
  # explicitly provide non-standard platforms dir
  export TAPA_XILINX_PLATFORMS_DIR=$PLATFORM_PATH

  # add local paths
  PATH=/scratch/hpc-prf-haqc/.local/bin:$PATH
  CPATH=/scratch/hpc-prf-haqc/.local/include:$CPATH

  # add conda env paths
  CPATH=/scratch/hpc-prf-haqc/philip/.conda/envs/tapa/include:$CPATH
  LD_LIBRARY_PATH=/scratch/hpc-prf-haqc/philip/.conda/envs/tapa/lib:$LD_LIBRARY_PATH
  LIBRARY_PATH=/scratch/hpc-prf-haqc/philip/.conda/envs/tapa/lib:$LIBRARY_PATH

  gcc --version
  # shellcheck disable=SC2086
  time conda run -n tapa --no-capture-output make tapa TARGET=hw BUILD_THREADS=10 ${QUARTET_ARGS_LIST[$SLURM_ARRAY_TASK_ID]}
  ls -al
else
  echo "Error: Script must run with either 'v++' or 'tapa' specified in first arg. (ex: sbatch ... runjob.sh v++)"
  exit 1
fi

# copy results
#cp -r ./temp_dir*/ "${output_dir}/"
cp -r ./temp_dir*/k_*/k_*/k_*/solution/syn/report "${output_dir}/"
cp -r ./temp_dir*/logs/ "${output_dir}/"
cp -r ./temp_dir*/reports/ "${output_dir}/"
cp -r ./build_dir*/ "${output_dir}/"
cp ./*.bin "${output_dir}"