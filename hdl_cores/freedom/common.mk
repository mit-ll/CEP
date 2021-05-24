# See LICENSE for license details.

# Required variables:
# - MODEL
# - PROJECT
# - CONFIG_PROJECT
# - CONFIG
# - BUILD_DIR
# - FPGA_DIR

# Optional variables:
# - EXTRA_FPGA_VSRCS

# export to bootloader
export ROMCONF=$(BUILD_DIR)/$(CONFIG_PROJECT).$(CONFIG).rom.conf

# export to fpga-shells
export FPGA_TOP_SYSTEM=$(MODEL)
export FPGA_BUILD_DIR=$(BUILD_DIR)/$(FPGA_TOP_SYSTEM)
export fpga_common_script_dir=$(FPGA_DIR)/common/tcl
export fpga_board_script_dir=$(FPGA_DIR)/$(BOARD)/tcl

export BUILD_DIR

EXTRA_FPGA_VSRCS ?=
PATCHVERILOG ?= ""
BOOTROM_DIR ?= ""

base_dir := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
export rocketchip_dir := $(base_dir)/rocket-chip
SBT ?= java -jar $(rocketchip_dir)/sbt-launch.jar

# Build firrtl.jar and put it where chisel3 can find it.
FIRRTL_JAR ?= $(rocketchip_dir)/firrtl/utils/bin/firrtl.jar
#FIRRTL ?= java -Xmx2G -Xss8M -cp $(FIRRTL_JAR) firrtl.Driver
# 10/27/20 tony d. as recommended from the message
#
FIRRTL ?= java -Xmx2G -Xss8M -cp $(FIRRTL_JAR) firrtl.stage.FirrtlMain

$(FIRRTL_JAR): $(shell find $(rocketchip_dir)/firrtl/src/main/scala -iname "*.scala")
	$(MAKE) -C $(rocketchip_dir)/firrtl SBT="$(SBT)" root_dir=$(rocketchip_dir)/firrtl build-scala
	touch $(FIRRTL_JAR)
	mkdir -p $(rocketchip_dir)/lib
	cp -p $(FIRRTL_JAR) rocket-chip/lib
	mkdir -p $(rocketchip_dir)/chisel3/lib
	cp -p $(FIRRTL_JAR) $(rocketchip_dir)/chisel3/lib

# Build .fir
firrtl := $(BUILD_DIR)/$(CONFIG_PROJECT).$(CONFIG).fir
$(firrtl): $(shell find $(base_dir)/src/main/scala -name '*.scala') $(FIRRTL_JAR)
	mkdir -p $(dir $@)
	$(SBT) "runMain freechips.rocketchip.system.Generator $(BUILD_DIR) $(PROJECT) $(MODEL) $(CONFIG_PROJECT) $(CONFIG)"

.PHONY: firrtl
firrtl: $(firrtl)

# Build .v
verilog := $(BUILD_DIR)/$(CONFIG_PROJECT).$(CONFIG).v
$(verilog): $(firrtl) $(FIRRTL_JAR)
	$(FIRRTL) -i $(firrtl) -o $@ -X verilog
ifneq ($(PATCHVERILOG),"")
	$(PATCHVERILOG)
endif

.PHONY: verilog
verilog: $(verilog)

romgen := $(BUILD_DIR)/$(CONFIG_PROJECT).$(CONFIG).rom.v
#
# Modified by Tony D. 04/16/21
# Calling the Makefile under bootrom 2 times: one for fpga to run on VC707, one for fpga in simulation.
# For later ASIC versions, just add 2 more make.
#
# Must passed in 2 arguments: 
# SDNAME & DEFINES
# Possible defines = -D_USE_xSPI  and/or -D_NO_SD_DOWNLOAD
# 
# NOTE: the -D_USE_xExe is added (and xExeHacked.c) to both sdboot_fpga_hw/sim as test to verify that we can execute codes from bootrom
# without main memory for stack point and private data.
#
#            (via xExeHacked.c version of xSPI)
#
#       They should be removed once the real xSPI controller is in for ASIC or replace xExeHacked.c with appropriate file...
#
# NOTE2: for simulation, we just need to make "hex" target instead of "romgen" when calling Make
#
$(romgen): $(verilog)
ifneq ($(BOOTROM_DIR),"")
	$(MAKE) -C $(BOOTROM_DIR) SDNAME=sdboot_fpga_sim EXTRA_CFLAGS="-g" DEFINES="-D_NO_SD_DOWNLOAD -D_USE_xEXE xExeHacked.c" hex
	$(MAKE) -C $(BOOTROM_DIR) SDNAME=sdboot_fpga_hw  EXTRA_CFLAGS="-g" DEFINES="                  -D_USE_xEXE xExeHacked.c" romgen
	mv $(BUILD_DIR)/rom.v $@
endif

.PHONY: romgen
romgen: $(romgen)

f := $(BUILD_DIR)/$(CONFIG_PROJECT).$(CONFIG).vsrcs.F
$(f):
	echo $(VSRCS) > $@

incdirs := $(BUILD_DIR)/$(CONFIG_PROJECT).$(CONFIG).incdirs
$(incdirs):
	echo $(INCDIRS) > $@

bit := $(BUILD_DIR)/obj/$(MODEL).bit
$(bit): $(romgen) $(f) $(incdirs)
	cd $(BUILD_DIR); vivado \
		-nojournal -mode batch \
		-source $(fpga_common_script_dir)/vivado.tcl \
		-tclargs \
		-top-module "$(MODEL)" \
		-F "$(f)" \
		-include_dirs "$(incdirs)" \
		-ip-vivado-tcls "$(shell find '$(BUILD_DIR)' -name '*.vivado.tcl')" \
		-board "$(BOARD)"


# Build .mcs
mcs := $(BUILD_DIR)/obj/$(MODEL).mcs
$(mcs): $(bit)
	cd $(BUILD_DIR); vivado -nojournal -mode batch -source $(fpga_common_script_dir)/write_cfgmem.tcl -tclargs $(BOARD) $@ $<

.PHONY: mcs
mcs: $(mcs)

# Build simulation files
sim: $(romgen) $(f) $(incdirs)
	cd $(BUILD_DIR); vivado \
		-nojournal -mode batch \
		-source $(fpga_common_script_dir)/prep_simulation.tcl \
		-tclargs \
		-top-module "$(MODEL)" \
		-F "$(f)" \
		-include_dirs "$(incdirs)" \
		-ip-vivado-tcls "$(shell find '$(BUILD_DIR)' -name '*.vivado.tcl')" \
		-board "$(BOARD)"

# Build Libero project
prjx := $(BUILD_DIR)/libero/$(MODEL).prjx
$(prjx): $(verilog)
	cd $(BUILD_DIR); libero SCRIPT:$(fpga_common_script_dir)/libero.tcl SCRIPT_ARGS:"$(BUILD_DIR) $(MODEL) $(PROJECT) $(CONFIG) $(BOARD)"

.PHONY: prjx
prjx: $(prjx)

# Clean
.PHONY: clean
clean:
ifneq ($(BOOTROM_DIR),"")
	$(MAKE) -C $(BOOTROM_DIR) clean
endif
	$(MAKE) -C $(FPGA_DIR) clean
	rm -rf $(BUILD_DIR)
