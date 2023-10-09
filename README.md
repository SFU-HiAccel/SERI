# SERI: High-Throughput <ins>S</ins>treaming Accelerator for <ins>E</ins>lectron <ins>R</ins>epulsion <ins>I</ins>ntegrals 

SERI is a high-throughput **S**treaming accelerator for **E**lectron **R**epulsion **I**ntegral (ERI) computation on HBM-based FPGAs. 

Electron repulsion integrals represent are the largest bottleneck in ab initio molecular dynamics (AIMD) quantum chemistry simulations.
These simulations describe the atomic interactions at the electronic level using quantum mechanics. Thus, it can accurately model reactive and complex systems, which are not possible for classical molecular dynamics.

SERI achieves an average speedup of 9.80x over the previous best-performing FPGA design, 
a 3.21x speedup over a 64-core AMD EPYC 7713 CPU, 
and a 15.64x speedup over an Nvidia A40 GPU. 
It reaches a peak throughput of 23.8 GERIS (10^9 ERIs per
second) on one Alveo U280 FPGA

## System Requirements

SERI is designed to be built with Vitis & Vivado 2023.2 (recommended 140+ GB RAM) and run on an Alveo U280 FPGA.

NOTE: This design rides on the edge of Vitis HLS's synthesis abilities and encounters many bugs which are sometimes only present for a select few quartet classes. 
The workarounds that have been implemented have only been tested with this version of Vitis & Vivado (2023.2).

## How to Build & Run

SERI can be built from the `fpga/src` directory using the make command with the following configuration options
- `TARGET` can be one of `sw_emu`|`hw_emu`|`hw`
- `AM_ABCD` selects which canonical quartet class to build for by specifying four angular momentum digits corresponding to the desired orbitals. (ex [fd|ps] = 3210) A list of the angular momentum values for the 55 canonical quartet classes can be found in [quartet_list.txt](quartet_list.txt).
  - s-orbital: angular momentum = 0
  - p-orbital: angular momentum = 1
  - d-orbital: angular momentum = 2
  - f-orbital: angular momentum = 3
- `FREQ_MHZ` (Optional) specify the target frequency for the build. If unspecified, will select automatic configuration
- `INP_PAR` (Optional) specify the input parallelism for the build. If unspecified, will select automatic configuration
- `BUILD_THREADS` (Optional: default 10) specify the number of jobs available to Vitis and Vivado to use (increasing this may require more RAM)

Example:
```shell
cd fpga/src
make -B run TARGET=hw AM_ABCD=2120
```
*NOTE:* Some quartet classes have high resource utilization that may require multiple build attempts to succeed.

Additionally, multiple build strategies are used, and some quartet classes rely on specific build strategies to P&R.
Sometimes, due to insufficient memory or other reasons, a strategy required to P&R that quartet class may crash. 
In those cases the build must be re-run such that all strategies can conclude without an unexpected crash.  

The `-B` option is recommended as all build products must be regenerated for each quartet class. Not including this flag may result in the makefile incorrectly thinking a build product is up-to-date, when in fact it is for a mis-matched quartet class.



### Extra notes on verification

All the reference files for the `synmol_tiny` test molecule have been included in the repo. 
However, due to the large file size of the outputs, the reference files for the larger versions are not included in this repo, but can be downloaded below:
- [synmol_med reference files](https://drive.google.com/file/d/1jZnrk7hPuvyaXfPNYkfmxlnB2TfMU3Vk/view?usp=sharing) (13GB)
