#########################################################################################
# fpga prototype makefile
#########################################################################################

#########################################################################################
# general path variables
#########################################################################################
base_dir=$(abspath ..)
sim_dir=$(abspath .)

#########################################################################################
# include shared variables
#########################################################################################

# Verify vivado is available, capture and display the version if it is...
ifeq "$(findstring clean,${MAKECMDGOALS})" ""
ifeq (,$(shell which vivado))
$(error Vivado is not available.  Please install and ensure the environment script is sourced.)
else
VIVADO_VERSION := $(shell vivado -version | head -1 | grep -o -P '(?<=\s).*(?=\s)')
$(info Vivado $(VIVADO_VERSION) detected.)
endif
endif

# Set default SUB_PROJECT
SUB_PROJECT ?= cep_arty100t

# Bootrom targets are "cleared" for CEP FPGA targets, since
# build is controlled from within the chisel configuration
ifeq ($(SUB_PROJECT),cep_vcu118)
	SBT_PROJECT       ?= fpga_platforms
	MODEL             ?= VCU118FPGATestHarness
	VLOG_MODEL        ?= VCU118FPGATestHarness
	MODEL_PACKAGE     ?= chipyard.fpga.vcu118
	CONFIG            ?= RocketVCU118CEPConfig
	CONFIG_PACKAGE    ?= chipyard.fpga.vcu118
	GENERATOR_PACKAGE ?= chipyard
	TOP               ?= ChipTop
	BOARD             ?= vcu118
	FPGA_BRAND        ?= xilinx
	SORT_SCRIPT		  := $(base_dir)/scripts/sort-blackbox.py
	SORT_FILE         := $(base_dir)/cep_sort.f
	BOOTROM_TARGETS	  :=
endif

# Bootrom targets are "cleared" for CEP FPGA targets, since
# build is controlled from within the chisel configuration
ifeq ($(SUB_PROJECT),cep_arty100t)
	SBT_PROJECT       ?= fpga_platforms
	MODEL             ?= Arty100TFPGATestHarness
	VLOG_MODEL        ?= Arty100TFPGATestHarness
	MODEL_PACKAGE     ?= chipyard.fpga.arty100t
	CONFIG            ?= RocketArty100TCEPConfig
	CONFIG_PACKAGE    ?= chipyard.fpga.arty100t
	GENERATOR_PACKAGE ?= chipyard
	TOP               ?= ChipTop
	BOARD             ?= arty_a7_100
	FPGA_BRAND        ?= xilinx
	SORT_SCRIPT		  := $(base_dir)/scripts/sort-blackbox.py
	SORT_FILE         := $(base_dir)/cep_sort.f
	BOOTROM_TARGETS	  :=
endif

# Bootrom targets are "cleared" for CEP FPGA targets, since
# build is controlled from within the chisel configuration
ifeq ($(SUB_PROJECT),cep_vc707)
	SBT_PROJECT       ?= fpga_platforms
	MODEL             ?= VC707FPGATestHarness
	VLOG_MODEL        ?= VC707FPGATestHarness
	MODEL_PACKAGE     ?= chipyard.fpga.vc707
	CONFIG            ?= RocketVC707CEPConfig
	CONFIG_PACKAGE    ?= chipyard.fpga.vc707
	GENERATOR_PACKAGE ?= chipyard
	TOP               ?= ChipTop
	BOARD             ?= vc707
	FPGA_BRAND        ?= xilinx
	SORT_SCRIPT		  := $(base_dir)/scripts/sort-blackbox.py
	SORT_FILE         := $(base_dir)/cep_sort.f
	BOOTROM_TARGETS	  :=
endif

# Bootrom targets are "cleared" for CEP FPGA targets, since
# build is controlled from within the chisel configuration
ifeq ($(SUB_PROJECT),cep_big_vc707)
	SBT_PROJECT       ?= fpga_platforms
	MODEL             ?= VC707FPGATestHarness
	VLOG_MODEL        ?= VC707FPGATestHarness
	MODEL_PACKAGE     ?= chipyard.fpga.vc707
	CONFIG            ?= RocketVC707BigCEPConfig
	CONFIG_PACKAGE    ?= chipyard.fpga.vc707
	GENERATOR_PACKAGE ?= chipyard
	TOP               ?= ChipTop
	BOARD             ?= vc707
	FPGA_BRAND        ?= xilinx
	SORT_SCRIPT		  := $(base_dir)/scripts/sort-blackbox.py
	SORT_FILE         := $(base_dir)/cep_sort.f
	BOOTROM_TARGETS	  :=
endif

ifeq ($(SBT_PROJECT),)
$(error Invalid SUB_PROJECT)
endif

include $(base_dir)/variables.mk

# default variables to build the arty example
# setup the board to use

.PHONY: default
default: bitstream

#########################################################################################
# misc. directories
#########################################################################################
fpga_dir := $(base_dir)/fpga/fpga-shells/$(FPGA_BRAND)
fpga_common_script_dir := $(fpga_dir)/common/tcl

#########################################################################################
# setup misc. sim files
#########################################################################################

# add the clock gate wrapper
SIM_FILE_REQS += $(ROCKETCHIP_RSRCS_DIR)/vsrc/EICG_wrapper.v

# add the testbench if needed for sim
ifdef TB
  SIM_FILE_REQS += $(base_dir)/generators/chipyard/src/main/resources/vsrc/$(TB).v
endif

# copy files but ignore *.h files in *.f (match vcs)
$(sim_files): $(SIM_FILE_REQS) | $(build_dir)
	cp -f $^ $(build_dir)
	$(foreach file,\
		$^,\
		$(if $(filter %.h,$(file)),\
			,\
			echo "$(addprefix $(build_dir)/, $(notdir $(file)))" >> $@;))

#########################################################################################
# import other necessary rules and variables
#########################################################################################
include $(base_dir)/common.mk

#########################################################################################
# copy from other directory
#########################################################################################
all_vsrcs := \
	$(sim_vsrcs) \
	$(base_dir)/generators/sifive-blocks/vsrc/SRLatch.v \
	$(fpga_dir)/common/vsrc/PowerOnResetFPGAOnly.v

#########################################################################################
# vivado rules
#########################################################################################
# combine all sources into single .f
synth_list_f := $(build_dir)/$(long_name).vsrcs.f
$(synth_list_f): $(sim_common_files) $(all_vsrcs)
	$(foreach file,$(all_vsrcs),echo "$(file)" >> $@;)
	cat $(sim_common_files) >> $@
# These steps are only relevant when building CEP-related targets
ifneq (,$(findstring cep,$(SUB_PROJECT)))
	@# Call the blackbox sorting script
	@${SORT_SCRIPT} ${synth_list_f} ${SORT_FILE}
endif

BIT_FILE := $(build_dir)/obj/$(MODEL).bit
$(BIT_FILE): $(synth_list_f)
	cd $(build_dir); vivado \
		-nojournal -mode batch \
		-source $(fpga_common_script_dir)/vivado.tcl \
		-tclargs \
			-top-module "$(MODEL)" \
			-F "$(synth_list_f)" \
			-board "$(BOARD)" \
			-ip-vivado-tcls "$(shell find '$(build_dir)' -name '*.vivado.tcl')"

.PHONY: bitstream
bitstream: $(BIT_FILE)

.PHONY: debug-bitstream
debug-bitstream: $(build_dir)/obj/post_synth.dcp
	cd $(build_dir); vivado \
		-nojournal -mode batch \
		-source $(sim_dir)/scripts/run_impl_bitstream.tcl \
		-tclargs \
			$(build_dir)/obj/post_synth.dcp \
			$(BOARD) \
			$(build_dir)/debug_obj \
			$(fpga_common_script_dir)

#########################################################################################
# sim rules
#########################################################################################

project-creation-script := $(sim_dir)/scripts/create_project.tcl

# generate the vivado project and add all sources
vivado-project := $(build_dir)/$(MODEL).xpr
$(vivado-project): $(synth_list_f) $(project-creation-script)
	cd $(build_dir); vivado \
		-nojournal -mode batch \
		-source  $(project-creation-script) \
		-tclargs \
			-top-module "$(MODEL)" \
			-F "$(synth_list_f)" \
			-ip-vivado-tcls "$(shell find '$(build_dir)' -name '*.vivado.tcl')" \
			-board "$(BOARD)"

fpga_sim_verilog_sources = $(build_dir)/fpga_sim_verilog_sources.f
fpga_sim_vhdl_sources = $(build_dir)/fpga_sim_vhdl_sources.f
fpga_sim_cc_sources = $(build_dir)/fpga_sim_cc_sources.f

# generate vcs collateral
vivado-vcs-collateral := $(fpga_sim_verilog_sources) $(fpga_sim_vhdl_sources) $(fpga_sim_cc_sources)
$(vivado-vcs-collateral): $(sim_dir)/scripts/generate_vcs_collateral.tcl $(vivado-project)
	cd $(build_dir); vivado $(vivado-project) \
		-nojournal -mode batch \
		-source $(sim_dir)/scripts/generate_vcs_collateral.tcl \
		-tclargs \
			$(FPGA_FAMILY) \
			$(build_dir) \
			$(MODEL) \
			$(TB)

include $(base_dir)/vcs.mk

sim_prefix = simv
sim = $(build_dir)/$(sim_prefix)-$(MODEL_PACKAGE)-$(CONFIG)

xilinx_lib = xil_defaultlib

VCS = vcs -full64
VLOGAN = vlogan -full64
VHDLAN = vhdlan -full64

# todo: deduplication between this and other VCS sim makefile
VLOGAN_OPTS = \
	-notice \
	-line \
	+lint=all,noVCDE,noONGS,noUI \
	-error=PCWM-L \
	-error=noZMMCM \
	-timescale=1ns/10ps \
	-quiet \
	-q \
	+rad \
	+vcs+lic+wait \
	+vc+list \
	-sverilog +systemverilogext+.sv+.svi+.svh+.svt -assert svaext +libext+.sv \
	+v2k +verilog2001ext+.v95+.vt+.vp +libext+.v \
	-debug_pp \
	+incdir+$(build_dir) \
	$(PREPROC_DEFINES)

# vcs-mx flow
$(sim): $(vivado-vcs-collateral)
	mkdir -p $(build_dir)/vcs_lib/xil_defaultlib
	if [ -s $(fpga_sim_vhdl_sources) ]; \
		then \
			cd $(build_dir); $(VHDLAN) -work $(xilinx_lib) -f $(fpga_sim_vhdl_sources) -l vhdlan.log; \
			fi; \
	if [ -s $(fpga_sim_verilog_sources) ]; then \
		cd $(build_dir); $(VLOGAN) $(VLOGAN_OPTS) +define+DEBUG -work $(xilinx_lib) -f $(fpga_sim_verilog_sources) -l vlogan.log; \
	fi; \
	if [ -s $(fpga_sim_cc_sources) ]; then \
		cd $(build_dir); $(VCS) $(VCS_CC_OPTS) $(xilinx_lib).$(TB) $(xilinx_lib).glbl -file $(fpga_sim_cc_sources) -o $@ -Mdir=$(build_dir)/$(long_name) -l vcs.log -debug_access+all; \
	fi

.PHONY: sim-fpga
sim-fpga: $(sim)

#########################################################################################
# general cleanup rules
#########################################################################################
.PHONY: clean
clean: cep_clean
	rm -rf $(gen_dir)
	rm -f *.log
	rm -f *.jou