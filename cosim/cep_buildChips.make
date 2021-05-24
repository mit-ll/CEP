#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File Name:      cep_buildChips.make
#// Program:        Common Evaluation Platform (CEP)
#// Description:    
#// Notes:          
#//
#//************************************************************************

#
#
#
DUT_BFM_XILINX_TOP_FILE  := ${BHV_DIR}/VCShell_bfm.v
DUT_BARE_XILINX_TOP_FILE := ${BHV_DIR}/VCShell_bare.v
DUT_RAW_XILINX_TOP_FILE  := ${DUT_TOP_DIR}/hdl_cores/freedom/builds/vc707-u500devkit/sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.v

# need to make local file with edit to support virtual mode
DUT_DDR3_FILE            += ddr3.v
DUT_DDR3_PARAM_FILE      += 1024Mb_ddr3_parameters.vh

# replace this line {BL_MAX*DQ_BITS{1'bx}} to {BL_MAX*DQ_BITS{1'b0}} for virtual mode support
${BHV_DIR}/ddr3.v: ${VIVADO_PATH}/data/rsb/non_default_iprepos/micron_ddr3_v2_0/hdl/ddr3.v
	cp -f ${VIVADO_PATH}/data/rsb/non_default_iprepos/micron_ddr3_v2_0/hdl/1024Mb_ddr3_parameters.vh ${BHV_DIR}/.
	sed -e 's/BL_MAX\*DQ_BITS{1'\''bx/BL_MAX\*DQ_BITS{1'\''b0/g' ${VIVADO_PATH}/data/rsb/non_default_iprepos/micron_ddr3_v2_0/hdl/ddr3.v > ${BHV_DIR}/ddr3.v

#
# BFM
#
ifeq "$(findstring BFM,${DUT_SIM_MODE})" "BFM"
DUT_XILINX_TOP_FILE     := ${DUT_BFM_XILINX_TOP_FILE}
DUT_VLOG_ARGS           += +define+BFM_MODE
COMMON_CFLAGS	        += -DBFM_MODE
#
# Bare Metal
#
else ifeq "$(findstring BARE,${DUT_SIM_MODE})" "BARE"
DUT_XILINX_TOP_FILE     := ${DUT_BARE_XILINX_TOP_FILE}
DUT_VLOG_ARGS           += +define+BARE_MODE
RISCV_WRAPPER           = ./riscv_wrapper.elf
#
#
#
else
ERROR_MESSAGE   = "DUT_SIM_MODE=${DUT_SIM_MODE} is either not supported or please check spelling"
endif

ifeq (${USE_DPI},1)
DUT_VLOG_ARGS           += +define+USE_DPI
endif
# DDR3 model
DUT_VLOG_ARGS           += +define+x1Gb+sg125+x8+den1024Mb
#
# -----------------------------------------------------------------------
# RISCV dependent stuffs
# -----------------------------------------------------------------------
#
ROCKET_DIR	= ${DUT_TOP_DIR}/hdl_cores/freedom/rocket-chip/src/main
FREEDOM_DIR = ${DUT_TOP_DIR}/hdl_cores/freedom/src/main
XLNX_IP_DIR     = ${DUT_TOP_DIR}/hdl_cores/freedom/builds/vc707-u500devkit/obj/ip
BARE_SRC_DIR    = ${SIM_DIR}/drivers/bare
BARE_OBJ_DIR    = ${SIM_DIR}/drivers/bare

RISCV            ?= /opt/riscv
RISCV_GCC         = ${RISCV}/bin/riscv64-unknown-elf-gcc
RISCV_OBJDUMP     = ${RISCV}/bin/riscv64-unknown-elf-objdump
RISCV_HEXDUMP     = /usr/bin/hexdump
#
# RISCV-TESTS : isa and benchmark
#
#RISCV_TEST_DIR     = ${RISCV}/riscv64-unknown-elf/share/riscv-tests
RISCV_TEST_DIR     = ${DUT_TOP_DIR}/software/riscv-tests

#
# Files/stuffs need to build bare metal tests in virtual mode
#
#riscv64-unknown-elf-gcc -march=rv64g -mabi=lp64 -static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles -g -DENTROPY=0xdb1550c -std=gnu99 -O2 -I/data/aduong/CEP/CEP_v2p8/software/riscv-tests/isa/../env/v -I/data/aduong/CEP/CEP_v2p8/software/riscv-tests/isa/macros/scalar -T/data/aduong/CEP/CEP_v2p8/software/riscv-tests/isa/../env/v/link.ld /data/aduong/CEP/CEP_v2p8/software/riscv-tests/isa/../env/v/entry.S /data/aduong/CEP/CEP_v2p8/software/riscv-tests/isa/../env/v/*.c rv64uc/rvc.S -o rv64uc-v-rvc
#riscv64-unknown-elf-objdump -S -C -d -l -x --disassemble-all --disassemble-zeroes --section=.text --section=.text.startup --section=.text.init --section=.data rv64uc-v-rvc > rv64uc-v-rvc.dump

RISCV_VIRT_CFLAGS  += -march=rv64g -mabi=lp64 -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles -DENTROPY=0xdb1550c -static 
RISCV_VIRT_LFLAGS  += -T${SIM_DIR}/drivers/virtual/link.ld
RISCV_VIRT_CFILES  += ${SIM_DIR}/drivers/virtual/*.c ${SIM_DIR}/drivers/virtual/multi_entry.S 
RISCV_VIRT_INC     += -I${SIM_DIR}/drivers/virtual -I${RISCV_TEST_DIR}/isa/macros/scalar 

#
# flags to compile codes for bare metal
#
RISCV_BARE_CFLAG  += -DBARE_MODE -static -DRISCV_CPU
RISCV_BARE_CFLAG  += -mcmodel=medany -Wall -O2 -g -fno-common -nostdlib -fno-builtin-printf -I${BARE_SRC_DIR} $(DRIVER_INC_LIST) -I ${SIM_DIR}/share -I ${SIM_DIR}/include
RISCV_BARE_LDFLAG += -static -nostdlib -nostartfiles -lgcc -DBARE_MODE 
RISCV_BARE_LDFILE  = ${BARE_SRC_DIR}/link.ld
RISCV_BARE_CRTFILE = ${BARE_SRC_DIR}/crt.S

# .C-style ONLY
RISC_BARE_SRC_LIST := $(subst .c,.bobj,$(notdir $(wildcard ${BARE_SRC_DIR}/*.c)))
RISC_BARE_OBJECTS  := $(foreach t,${RISC_BARE_SRC_LIST}, ${BARE_OBJ_DIR}/${t})

CEP_SRC_FILES            = $(wildcard ${SIM_DIR}/drivers/cep_tests/*.cc)
CEP_BARE_OBJECTS       := $(subst .cc,.bobj,${CEP_SRC_FILES})

#
# Bare metal booloader
#
#RICSV_BARE_BOOT_DIR       := ${SIM_DIR}/drivers/bootbare
#RISCV_BARE_BOOT_ROM       := bootbare.hex

# use the one build during verilog generation
RICSV_BARE_BOOT_DIR       := ${DUT_TOP_DIR}/hdl_cores/freedom/builds/vc707-u500devkit
RISCV_BARE_BOOT_ROM       := sdboot_fpga_sim.hex

%.hex: ${INC_DIR}/cep_adrMap.h %.c
	(cd ${RICSV_BARE_BOOT_DIR}; make clean; make;)

CEP_DIAG_FILES          = $(wildcard ${SIM_DIR}/drivers/diag/*.cc)
CEP_BARE_DIAG_OBJECTS   := $(subst .cc,.bobj,${CEP_DIAG_FILES})
#
# -----------------------------------------------------------------------
# rules to build object file for bare metal under sim (bobj)
# -----------------------------------------------------------------------
#

${BARE_OBJ_DIR}/crt.bobj: ${RISCV_BARE_CRTFILE}
	$(RISCV_GCC) $(RISCV_BARE_CFLAG) -c $< -o $@

${BARE_OBJ_DIR}/%.bobj: ${BARE_OBJ_DIR}/%.c
	$(RISCV_GCC) $(RISCV_BARE_CFLAG) -c $< -o $@


riscv_wrapper.bobj: riscv_wrapper.cc
	$(RISCV_GCC) $(RISCV_BARE_CFLAG) -c $< -o $@

ifeq "$(findstring BUILTIN,${DUT_ELF_MODE})" "BUILTIN"
# blank it
undefine RISCV_WRAPPER_ELF
else

RISCV_WRAPPER_ELF = ${RISCV_WRAPPER}

#
# with -g, tests in virtual adr will run forever when it takes a page fault..!! (sending stuffs to console and stop)
# so build with -g for dump file only
#
#
ifeq (${DUT_IN_VIRTUAL},1)
riscv_wrapper.elf: riscv_virt.S riscv_wrapper.cc ${RISCV_VIRT_CFILES}
	$(RISCV_GCC) ${RISCV_VIRT_CFLAGS} ${RISCV_VIRT_LFLAGS} -g ${RISCV_VIRT_INC} $^ -o riscv_withG.elf
	${RISCV_OBJDUMP} -S -C -d -l -x riscv_withG.elf > riscv_wrapper.dump; rm riscv_withG.elf;
	$(RISCV_GCC) ${RISCV_VIRT_CFLAGS} ${RISCV_VIRT_LFLAGS} ${RISCV_VIRT_INC} $^ -o riscv_wrapper.elf
	${RISCV_HEXDUMP} -C riscv_wrapper.elf > riscv_wrapper.hex
	${BIN_DIR}/createPassFail.pl riscv_wrapper.dump PassFail.hex
else
riscv_wrapper.elf: riscv_wrapper.bobj ${BARE_OBJ_DIR}/crt.bobj ${RISC_BARE_OBJECTS} ${CEP_BARE_OBJECTS} ${CEP_BARE_DIAG_OBJECTS}
	$(RISCV_GCC) -T ${RISCV_BARE_LDFILE} ${RISCV_BARE_LDFLAG} $^ -o $@
	${RISCV_OBJDUMP} -S -C -d -l -x riscv_wrapper.elf > riscv_wrapper.dump
	${RISCV_HEXDUMP} -C riscv_wrapper.elf > riscv_wrapper.hex

endif

%.bobj: %.cc ${VERILOG_DEFINE_LIST} 
	$(RISCV_GCC) $(RISCV_BARE_CFLAG) -c $< -o $@

endif
#
# ***************************************************************
# below are strickly for vlog/vcom to build CEP ASIC/FPGA for sim
# ***************************************************************
#

#
# ===========================================
# List of files for questa/modelsim to compile and build
# ===========================================
#
#
# vendor independent, strickly synthesizable RTL codes
#
# NOTE: dft_top.v and idft_top.v have duplicate modules such as memMod, memMod_dist, etc.. use -v for one of them
DUT_COMMON_FILES = ${DUT_TOP_DIR}/hdl_cores/aes/table.v 	\
	${DUT_TOP_DIR}/hdl_cores/aes/round.v			\
	${DUT_TOP_DIR}/hdl_cores/aes/aes_192.v 			\
	${DUT_TOP_DIR}/hdl_cores/llki/llki_pkg.sv 		\
	${DUT_TOP_DIR}/hdl_cores/llki/top_pkg.sv 		\
	$(DUT_TOP_DIR)/opentitan/hw/ip/tlul/rtl/tlul_pkg.sv	\
	${DUT_TOP_DIR}/hdl_cores/llki/tlul_err.sv 		\
	${DUT_TOP_DIR}/hdl_cores/llki/tlul_adapter_reg.sv 	\
	$(DUT_TOP_DIR)/opentitan/hw/ip/prim/rtl/prim_util_pkg.sv	\
	${DUT_TOP_DIR}/generated_dsp_code/dft_top.v		\
	-v ${DUT_TOP_DIR}/generated_dsp_code/idft_top.v		\
	${BHV_DIR}/ddr3.v					\

#
# created from chisel
#
#RISCV_FILES	= ${ROCKET_DIR}/resources/vsrc/AsyncResetReg.v  \

RISCV_FILES	= \
		  ${ROCKET_DIR}/resources/vsrc/plusarg_reader.v	\
		  ${ROCKET_DIR}/resources/vsrc/EICG_wrapper.v

FREEDOM_FILES = ${FREEDOM_DIR}/resources/vsrc/AnalogToUInt.v \
		${FREEDOM_DIR}/resources/vsrc/UIntToAnalog.v

#
# ==================================
# Xilinx related stuffs if selected
# ==================================
#

DUT_XILINX_FILES = ${VIVADO_PATH}/data/verilog/src/glbl.v	\
	${DUT_TOP_DIR}/hdl_cores/freedom/fpga-shells/xilinx/common/vsrc/PowerOnResetFPGAOnly.v 	\
	${DUT_TOP_DIR}/hdl_cores/freedom/builds/vc707-u500devkit/sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.rom.v \
	${XLNX_IP_DIR}/vc707mig1gb/vc707mig1gb/user_design/rtl/vc707mig1gb_mig_sim.v \
	${XLNX_IP_DIR}/vc707mig1gb/vc707mig1gb/user_design/rtl/vc707mig1gb.v 
#
#
#
DUT_XILINX_TOP_MODULE	= cep_tb
DUT_XILINX_TOP_TB 	= ${DVT_DIR}/${DUT_XILINX_TOP_MODULE}.v
XILINX_MODELSIM_INI     = ${SIM_DIR}/xil_lib/modelsim.ini
XILINX_LIBRARY_LIST     = -L unisims_ver -L unimacro_ver -L unifast_ver -L secureip -L xpm
DUT_XILINX_VOPT_ARGS	= +acc -64 +nolibcell +nospecify +notimingchecks -modelsimini ${XILINX_MODELSIM_INI} ${WORK_DIR}.glbl ${XILINX_LIBRARY_LIST}
DUT_XILINX_VLOG_ARGS	= +acc -64 -sv +define+RANDOMIZE_MEM_INIT+RANDOMIZE_REG_INIT+RANDOM="1'b0"
DUT_XILINX_VCOM_ARGS	= -64 -93 -modelsimini ${XILINX_MODELSIM_INI}
DUT_XILINX_VSIM_ARGS	= -64 -modelsimini ${XILINX_MODELSIM_INI} ${XILINX_LIBRARY_LIST} -warning 3363 -warning 3053 -warning 8630

#
# ===========================================
# Derived path/files based on DUT_VENDOR and mode of sim
# ===========================================
#
# default is XILINX for now
#
ifeq "$(findstring XILINX,${DUT_VENDOR})" "XILINX"
DUT_TOP_TB  	  := ${DUT_XILINX_TOP_TB}
DUT_VERILOG_FLIST := ${DUT_COMMON_FILES} ${FREEDOM_FILES} ${RISCV_FILES} ${DUT_XILINX_FILES} ${DUT_XILINX_TOP_FILE}
DUT_MODULE	  := ${DUT_XILINX_TOP_MODULE}
DUT_OPT_MODULE	  := ${DUT_XILINX_TOP_MODULE}_opt	
DUT_VOPT_ARGS	  += ${DUT_XILINX_VOPT_ARGS} 
DUT_VLOG_ARGS	  += ${DUT_XILINX_VLOG_ARGS}
DUT_VSIM_ARGS	  += ${DUT_XILINX_VSIM_ARGS} +initreg+0 +initmem+0
else
# For ASIC with TSMC or standard ASIC library!!!
ERROR_MESSAGE   = "DUT_VENDOR=${DUT_VENDOR} is either not supported or please check spelling"
endif

# add config file
DUT_TB_FILES_LIST  = ${DUT_XILINX_TOP_TB} 


FPGA_INST_NAME     = /cep_tb/fpga


#
# add config file
#
DUT_TB_FILES_LIST  = ${DUT_XILINX_TOP_TB} 


#
# ===========================================
# Search list
# ===========================================
#	${VENDOR_DIR}/micron 	\



SEARCH_DIR_LIST := ${DVT_DIR}		\
		${BLD_DIR} 		\
		${XLNX_IP_DIR}/corePLL	\
		${XLNX_IP_DIR}/vc707mig1gb/vc707mig1gb/user_design/rtl/phy	\
		${XLNX_IP_DIR}/vc707mig1gb/vc707mig1gb/user_design/rtl/ui	\
		${XLNX_IP_DIR}/vc707mig1gb/vc707mig1gb/user_design/rtl/axi	\
		${XLNX_IP_DIR}/vc707mig1gb/vc707mig1gb/user_design/rtl/ecc	\
		${XLNX_IP_DIR}/vc707mig1gb/vc707mig1gb/user_design/rtl/ip_top	\
		${XLNX_IP_DIR}/vc707mig1gb/vc707mig1gb/user_design/rtl/controller	\
		${XLNX_IP_DIR}/vc707mig1gb/vc707mig1gb/user_design/rtl/clocking	\
		${XLNX_IP_DIR}/vc707mig1gb/vc707mig1gb/user_design/rtl		\
		${DUT_TOP_DIR}/hdl_cores/aes					\
		${DUT_TOP_DIR}/hdl_cores/des3					\
		${DUT_TOP_DIR}/hdl_cores/gps					\
		${DUT_TOP_DIR}/hdl_cores/md5					\
		${DUT_TOP_DIR}/hdl_cores/rsa/rtl				\
		${DUT_TOP_DIR}/hdl_cores/sha256					\
		${DUT_TOP_DIR}/hdl_cores/dsp					\
		${DUT_TOP_DIR}/hdl_cores/llki 					\
		$(DUT_TOP_DIR)/opentitan/hw/ip/tlul/rtl 			\
		$(DUT_TOP_DIR)/opentitan/hw/ip/prim/rtl 			\
		$(DUT_TOP_DIR)/opentitan/hw/ip/prim_generic/rtl 		\
		${BHV_DIR} 		


INCDIR_LIST     := ${SEARCH_DIR_LIST}	\
		${DUT_TOP_DIR}/hdl_cores/freedom/builds/vc707-u500devkit/obj/ip/corePLL

#
# Create DUT_XILINX_BFM_FILE from raw for BFM mode
#
${DUT_BFM_XILINX_TOP_FILE}: ${DUT_RAW_XILINX_TOP_FILE}
	echo "\`include \"config.v\"" > ${DUT_BFM_XILINX_TOP_FILE}
	sed -e 's/RocketTile tile/RocketTile_beh tile/g' \
	    -e 's/IBUF_DELAY_VALUE(0)/IBUF_DELAY_VALUE("0")/' ${DUT_RAW_XILINX_TOP_FILE} >> ${DUT_BFM_XILINX_TOP_FILE}
	touch $@

#
# FIXME!!!!
#
${DUT_BARE_XILINX_TOP_FILE}: ${DUT_RAW_XILINX_TOP_FILE}
	echo "\`include \"config.v\"" > ${DUT_BARE_XILINX_TOP_FILE}
	sed -e 's/IBUF_DELAY_VALUE(0)/IBUF_DELAY_VALUE("0")/' ${DUT_RAW_XILINX_TOP_FILE} >> ${DUT_BARE_XILINX_TOP_FILE}
	touch $@


#
#
# Use Auto generate makefile from vmake output else force a rebuild
#
#
DEPEND_MAKEFILE = ${BLD_DIR}/${SIM_DEPEND_TARGET}.make

#
# if makefile is there derived from vmake, use it else force a rebuild
#
ifeq ($(wildcard $(DEPEND_MAKEFILE)),)
${BLD_DIR}/${SIM_DEPEND_TARGET}: .force
	if test ! -d ${WORK_DIR}; then ${VLIB_CMD} ${WORK_DIR}; fi
	${VMAP_CMD} ${WORK_NAME} ${WORK_DIR}
	@echo "Force a make because file ${BLD_DIR}/${SIM_DEPEND_TARGET}.make not detected"
	touch $@

${BLD_DIR}/${SIM_DEPEND_TARGET}_VHDL_LIST: .force
	touch $@

${BLD_DIR}/${SIM_DEPEND_TARGET}_OTHER_LIST: .force
	touch $@
else
include ${BLD_DIR}/${SIM_DEPEND_TARGET}.make
endif

#
# Build Verilog and VHDL seperately to save time since VHDL takes a long time
#
${BLD_DIR}/.buildVcom : ${VERILOG_DEFINE_LIST} ${SIM_DIR}/common.make ${SIM_DIR}/cep_buildChips.make  ${BLD_DIR}/${SIM_DEPEND_TARGET}_VHDL_LIST 
	@for i in ${CEP_VHDL_FLIST}; do                        		\
		${VCOM_CMD} -work ${WORK_DIR} ${DUT_VCOM_ARGS} $$i;		\
	done
	touch $@


${BLD_DIR}/.buildVlog : ${VERILOG_DEFINE_LIST} ${SIM_DIR}/common.make ${SIM_DIR}/cep_buildChips.make ${DUT_XILINX_TOP_FILE} ${BLD_DIR}/${SIM_DEPEND_TARGET}_OTHER_LIST ${BHV_DIR}/ddr3.v
	$(RM) ${BLD_DIR}/searchPaths_build
	@for i in ${SEARCH_DIR_LIST}; do                        			\
		echo "-y" $${i} >> ${BLD_DIR}/searchPaths_build;			\
	done
	@for i in ${INCDIR_LIST}; do                            			\
		echo "+incdir+"$${i} >> ${BLD_DIR}/searchPaths_build;    		\
	done
	${VLOG_CMD} -work ${WORK_DIR} -f ${BLD_DIR}/searchPaths_build +libext+.v +libext+.sv ${BLD_DIR}/config.v ${DUT_VLOG_ARGS} +define+MODELSIM -incr ${DUT_VERILOG_FLIST} ${DUT_XILINX_TOP_TB}
	touch $@

#
# Now do the optimization after vlog and vcom
# make sure vlog is done first!!
# after that auto-generate the dependencies if there is any change
#
ifeq (${CADENCE},0)
${BLD_DIR}/_info: ${BLD_DIR}/.is_checked ${BLD_DIR}/.buildVlog ${BLD_DIR}/.buildVcom ${RICSV_BARE_BOOT_DIR}/${RISCV_BARE_BOOT_ROM}
	${VOPT_CMD} -work ${WORK_DIR} ${DUT_VOPT_ARGS} ${WORK_DIR}.${DUT_MODULE} -o ${DUT_OPT_MODULE}
	(cd ${BLD_DIR}; ${VMAKE_CMD} ${WORK_NAME} > ${BLD_DIR}/.vmake_out; ${MKDEPEND} ${BLD_DIR}/.vmake_out ${BLD_DIR} ${SIM_DEPEND_TARGET} )
	touch $@
else
${BLD_DIR}/_info: ${BLD_DIR}/.is_checked ${BLD_DIR}/.cadenceBuild ${RICSV_BARE_BOOT_DIR}/${RISCV_BARE_BOOT_ROM}
	touch $@
endif


buildSim: .force ${BLD_DIR}/_info 
