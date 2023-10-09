#
# Copyright 2019-2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# makefile-generator v1.0.3
#

############################## Help Section ##############################
ifneq ($(findstring Makefile, $(MAKEFILE_LIST)), Makefile)
help:
	$(ECHO) "Please do not directly run this makefile. Use the regular 'make' command instead."
endif

############################## Makefile Settings ##############################
MAKEFLAGS := --jobs=7

############################## Setting up Project Variables ##############################
TARGET := hw
VPP_LDFLAGS :=
include ./utils.mk

TEMP_DIR := ./temp_dir.$(TARGET).$(XSA)
BUILD_DIR := ./build_dir.$(TARGET).$(XSA)

LINK_OUTPUT := $(BUILD_DIR)/qc_fpga.link.xclbin
PACKAGE_OUT = ./package.$(TARGET)

BUILD_THREADS ?= 10
AM_ABCD ?= 3310
ifeq ($(TARGET), hw)
    INP_PAR ?= $(shell cat auto_params.txt | awk -v am=$(AM_ABCD) '$$1 == am {print $$3}')
else
    INP_PAR ?= 1
endif
FREQ_MHZ ?= $(shell cat auto_params.txt | awk -v am=$(AM_ABCD) '$$1 == am {print $$2}')
MULTIPLICITY = $(INP_PAR)

COMPILE_ARGS = -DAM_ABCD=$(AM_ABCD)
CMD_ARGS = --xclbin $(BUILD_DIR)/qc_fpga.xclbin
# If running on hw use larger molecule with benchmark mode
ifeq ($(TARGET), hw)
    CMD_ARGS += -b --mob ../../res/mobs/synmol_med.mob
else
    CMD_ARGS += --mob ../../res/mobs/synmol_tiny.mob --ref ../../res/refs/ref_synmol_tiny_$(AM_ABCD).bin
endif
CXXFLAGS += -I$(XILINX_XRT)/include -I$(XILINX_VIVADO)/include -Wall -O0 -g -std=c++1y
LDFLAGS += -L$(XILINX_XRT)/lib -pthread -lOpenCL

########################## Checking if PLATFORM in allowlist #######################
PLATFORM_BLOCKLIST += nodma 
############################## Setting up Host Variables ##############################
#Include Required Host Source Files
SRC_EXTENSIONS := cpp h hpp
HOST_SRCS_DIRS := $(XF_PROJ_ROOT)/include ./host ./common
CXXFLAGS += -I$(XF_PROJ_ROOT)/include -I.
HOST_SRCS += $(foreach dir,$(HOST_SRCS_DIRS),$(foreach ext,$(SRC_EXTENSIONS),$(shell find $(dir) -name '*.$(ext)')))
KERNEL_SRCS_DIRS := ./device ./common
KERNEL_SRCS += $(foreach dir,$(KERNEL_SRCS_DIRS),$(foreach ext,$(SRC_EXTENSIONS),$(shell find $(dir) -name '*.$(ext)')))

# Host compiler global settings
CXXFLAGS += -fmessage-length=0
LDFLAGS += -lrt -lstdc++ 

############################## Setting up Kernel Variables ##############################
# Kernel compiler global settings
VPP_FLAGS += --save-temps --hls.pre_tcl hls_config.tcl
VPP_FLAGS += -t $(TARGET) --platform $(PLATFORM)
VPP_FLAGS += --hls.clock $(FREQ_MHZ)000000:k_preparation,k_recurrence_relations,k_rr_a,k_rr_b,k_buffer_permutation,k_gaussian_quadrature_a,k_gaussian_quadrature_b,k_find_bmax,k_compress_store
VPP_FLAGS += --kernel_frequency $(FREQ_MHZ)

EXECUTABLE = ./qc_fpga
EMCONFIG_DIR = $(TEMP_DIR)

############################## Setting Targets ##############################
.PHONY: all clean cleanall docs emconfig
all: check-platform check-device check-vitis $(EXECUTABLE) $(BUILD_DIR)/qc_fpga.xclbin emconfig

.PHONY: host
host: $(EXECUTABLE)

.PHONY: build
build: check-vitis check-device $(BUILD_DIR)/qc_fpga.xclbin

.PHONY: xclbin
xclbin: build

############################## Setting Rules for Binary Containers (Building Kernels) ##############################
cfg_conn_gen: cfg_connectivity.cpp
	g++ -o $@ $^ $(COMPILE_ARGS)

GEN_PREPROCESSED_CONSTS = gcc -E -P $(COMPILE_ARGS) common/param_config.h
CFG_TEMPLATE_VARIABLES := BUILD_THREADS ORD_RYS NUM_ERIS_PORTS BP_FIFO_COUNT
gen_vpp_config: cfg_conn_gen
	# extract derived constant values
	$(eval NUM_ERIS_PORTS=$(shell $(GEN_PREPROCESSED_CONSTS) | grep 'pp_num_eris_ports' | tr -d -c 0-9))
	$(eval BP_FIFO_COUNT=$(shell $(GEN_PREPROCESSED_CONSTS) | grep 'pp_bp_fifo_count' | tr -d -c 0-9))
	$(eval GQ_KERNEL_SPLIT=$(shell $(GEN_PREPROCESSED_CONSTS) | grep 'pp_gq_kernel_split' | tr -d -c 0-9))
	$(eval BP_BYPASS=$(shell $(GEN_PREPROCESSED_CONSTS) | grep 'pp_bp_bypass' | tr -d -c 0-9))
	@echo "-------------------------------------------------------------------------------------------------"
	@echo "Using NUM_ERIS_PORTS=${NUM_ERIS_PORTS} BP_FIFO_COUNT=${BP_FIFO_COUNT} GQ_KERNEL_SPLIT=${GQ_KERNEL_SPLIT} BP_BYPASS=${BP_BYPASS} MULTIPLICITY=${MULTIPLICITY} FREQ_MHZ=${FREQ_MHZ}"
	@echo "-------------------------------------------------------------------------------------------------"
	@if [ -z "$(NUM_ERIS_PORTS)" ]; then \
		echo "NUM_ERIS_PORTS is empty. Build failed."; \
		exit 1; \
	fi
	@if [ -z "$(BP_FIFO_COUNT)" ]; then \
		echo "BP_FIFO_COUNT is empty. Build failed."; \
		exit 1; \
	fi
	@if [ -z "$(GQ_KERNEL_SPLIT)" ]; then \
		echo "GQ_KERNEL_SPLIT is empty. Build failed."; \
		exit 1; \
	fi
	@if [ -z "$(BP_BYPASS)" ]; then \
		echo "BP_BYPASS is empty. Build failed."; \
		exit 1; \
	fi
	# create generated cfg file from template cfg file
	cp vpp_template.cfg vpp_generated.cfg
	cp pre_place_template.tcl pre_place_generated.tcl
	@$(foreach var,$(CFG_TEMPLATE_VARIABLES),\
		$(eval var_value := $($(var))) \
		for i in $$(seq 1 $(var_value)); do \
			sed -i 's/#\$$('"$(var)"'>='"$$i"')//g' vpp_generated.cfg; \
		done; \
		sed -i 's/#\$$('"$(var)"'=='"$(var_value)"')//g' vpp_generated.cfg; \
		sed -i "s/\$$$(var)/$(var_value)/g" vpp_generated.cfg; \
	)
	./cfg_conn_gen vpp_generated.cfg pre_place_generated.tcl $(MULTIPLICITY)
	@echo "Template CFG and pre-place processed"

$(TEMP_DIR)/k_preparation.xo: device/k_preparation.cpp | gen_vpp_config
	mkdir -p $(TEMP_DIR)
	v++ -c $(VPP_FLAGS) $(VPP_COMPILE_FLAGS) $(COMPILE_ARGS) -I. -k k_preparation --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'

$(TEMP_DIR)/k_recurrence_relations.xo: device/k_recurrence_relations.cpp | gen_vpp_config
	mkdir -p $(TEMP_DIR)
	v++ -c $(VPP_FLAGS) $(VPP_COMPILE_FLAGS) $(COMPILE_ARGS) -I. -k k_recurrence_relations --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'

$(TEMP_DIR)/k_rr_a.xo: device/k_rr_a.cpp | gen_vpp_config
	mkdir -p $(TEMP_DIR)
	v++ -c $(VPP_FLAGS) $(VPP_COMPILE_FLAGS) $(COMPILE_ARGS) -I. -k k_rr_a --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'

$(TEMP_DIR)/k_rr_b.xo: device/k_rr_b.cpp | gen_vpp_config
	mkdir -p $(TEMP_DIR)
	v++ -c $(VPP_FLAGS) $(VPP_COMPILE_FLAGS) $(COMPILE_ARGS) -I. -k k_rr_b --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'

$(TEMP_DIR)/k_buffer_permutation.xo: device/k_buffer_permutation.cpp | gen_vpp_config
	mkdir -p $(TEMP_DIR)
	v++ -c $(VPP_FLAGS) $(VPP_COMPILE_FLAGS) $(COMPILE_ARGS) -I. -k k_buffer_permutation --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'

$(TEMP_DIR)/k_gaussian_quadrature_a.xo: device/k_gaussian_quadrature_a.cpp | gen_vpp_config
	mkdir -p $(TEMP_DIR)
	v++ -c $(VPP_FLAGS) $(VPP_COMPILE_FLAGS) $(COMPILE_ARGS) -I. -k k_gaussian_quadrature_a --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'

$(TEMP_DIR)/k_gaussian_quadrature_b.xo: device/k_gaussian_quadrature_b.cpp | gen_vpp_config
	mkdir -p $(TEMP_DIR)
	v++ -c $(VPP_FLAGS) $(VPP_COMPILE_FLAGS) $(COMPILE_ARGS) -I. -k k_gaussian_quadrature_b --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'

$(TEMP_DIR)/k_find_bmax.xo: device/k_find_bmax.cpp | gen_vpp_config
	mkdir -p $(TEMP_DIR)
	v++ -c $(VPP_FLAGS) $(VPP_COMPILE_FLAGS) $(COMPILE_ARGS) -I. -k k_find_bmax --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'

$(TEMP_DIR)/k_compress_store.xo: device/k_compress_store.cpp | gen_vpp_config
	mkdir -p $(TEMP_DIR)
	v++ -c $(VPP_FLAGS) $(VPP_COMPILE_FLAGS) $(COMPILE_ARGS) -I. -k k_compress_store --temp_dir $(TEMP_DIR)  -I'$(<D)' -o'$@' '$<'

XO_FILES := $(TEMP_DIR)/k_preparation.xo $(TEMP_DIR)/k_gaussian_quadrature_b.xo $(TEMP_DIR)/k_find_bmax.xo $(TEMP_DIR)/k_compress_store.xo
GQ_KERNEL_SPLIT := $(shell $(GEN_PREPROCESSED_CONSTS) | grep 'pp_gq_kernel_split' | tr -d -c 0-9)
ifeq ($(GQ_KERNEL_SPLIT),2)
XO_FILES += $(TEMP_DIR)/k_gaussian_quadrature_a.xo
endif
BP_BYPASS := $(shell $(GEN_PREPROCESSED_CONSTS) | grep 'pp_bp_bypass' | tr -d -c 0-9)
ifeq ($(BP_BYPASS),0)
XO_FILES += $(TEMP_DIR)/k_buffer_permutation.xo
endif
RR_KERNEL_SPLIT := $(shell $(GEN_PREPROCESSED_CONSTS) | grep 'pp_rr_kernel_split' | tr -d -c 0-9)
ifeq ($(RR_KERNEL_SPLIT),2)
XO_FILES += $(TEMP_DIR)/k_rr_a.xo $(TEMP_DIR)/k_rr_b.xo
else
XO_FILES += $(TEMP_DIR)/k_recurrence_relations.xo
endif

.PHONY: synth
synth: $(XO_FILES)

$(BUILD_DIR)/qc_fpga.xclbin: $(XO_FILES)
	mkdir -p $(BUILD_DIR)
	v++ -l $(VPP_FLAGS) $(VPP_LDFLAGS) --temp_dir $(TEMP_DIR) --config vpp_generated.cfg -o'$(LINK_OUTPUT)' $(+)
	v++ -p $(LINK_OUTPUT) $(VPP_FLAGS) --package.out_dir $(PACKAGE_OUT) -o $(BUILD_DIR)/qc_fpga.xclbin

############################## Setting Rules for Host (Building Host Executable) ##############################
$(EXECUTABLE): $(HOST_SRCS) | check-xrt
		g++ -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(COMPILE_ARGS)


# separate target for IDE analysis that doesn't include XRT check
XILINX_INCLUDES ?= -I/tools/Xilinx/Vitis_HLS/2023.2/include -I/opt/xilinx/xrt/include/xrt/
ide_analysis: $(HOST_SRCS) $(KERNEL_SRCS) tests gen_vpp_config
		g++ -o ide_analysis_host $(HOST_SRCS)   $(CXXFLAGS) $(LDFLAGS) $(COMPILE_ARGS) $(XILINX_INCLUDES)
		g++ -o ide_analysis_krnl $(KERNEL_SRCS) $(CXXFLAGS) $(LDFLAGS) $(COMPILE_ARGS) $(XILINX_INCLUDES)

emconfig:$(EMCONFIG_DIR)/emconfig.json
$(EMCONFIG_DIR)/emconfig.json:
	emconfigutil --platform $(PLATFORM) --od $(EMCONFIG_DIR)

############################## Setting Essential Checks and Running Rules ##############################
run: all
ifeq ($(TARGET),$(filter $(TARGET),sw_emu hw_emu))
	cp -rf $(EMCONFIG_DIR)/emconfig.json .
	XCL_EMULATION_MODE=$(TARGET) $(EXECUTABLE) $(CMD_ARGS)
else
	$(EXECUTABLE) $(CMD_ARGS)
endif

.PHONY: test
test: $(EXECUTABLE)
ifeq ($(TARGET),$(filter $(TARGET),sw_emu hw_emu))
	XCL_EMULATION_MODE=$(TARGET) $(EXECUTABLE) $(CMD_ARGS)
else
	$(EXECUTABLE) $(CMD_ARGS)
endif

############################## Cleaning Rules ##############################
# Cleaning stuff
clean:
	-$(RMDIR) $(EXECUTABLE) $(XCLBIN)/{*sw_emu*,*hw_emu*} 
	-$(RMDIR) profile_* TempConfig system_estimate.xtxt *.rpt *.csv 
	-$(RMDIR) src/*.ll *v++* .Xil emconfig.json dltmp* xmltmp* *.log *.jou *.wcfg *.wdb

cleanall: clean
	-$(RMDIR) build_dir*
	-$(RMDIR) package.*
	-$(RMDIR) temp_dir* *xclbin.run_summary qemu-memory-_* emulation _vimage pl* start_simulation.sh *.xclbin

