#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
#// SPDX License Identifier: MIT
#//
#// File Name:      Makefile     
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Testbench makefile for Tilelink Transaction Level simulation
#// Notes:          
#// Usage: To run single core simulation:
#//
#//    make [CORE=<one_of_supported_core>]  [VERBOSE=0|1] (default CORE=aes)
#//
#// Other make commands:
#//    make runAll  	<-- run all cores simulation
#//    make vectors 	<-- prepare stimulus for simulation
#//    make clean	<-- clean all output files from simulations
#//
#//************************************************************************
#
# Name one of the supported cores to run: <aes|md5|des3|sha256|gps|dft|idft|fir|iir>
#
LLKI_EN     = 1

CORE        = aes
#
# Turn on read/write messages during simulation. Default is ON
#
VERBOSE     = 1
#
#
# Add Cadence supports ro tun on RHEL7 machine
#
CADENCE  = 0
ifeq (${CADENCE},1)
DEFINE_LIST += +define+CADENCE
export CADENCE_ENABLE_AVSREQ_63188_PHASE_1:=1
export CADENCE_ENABLE_AVSREQ_44905_PHASE_1:=1
# also might need to set  as sudo
#sudo setsebool -P mmap_low_allowed 1
endif

#
# -------------------------------------------
# STOP!!!
# DO NOT TOUCH ANYTHING BELOW THIS LINE (unles you know what you are doing :-) )
# -------------------------------------------
#
# Check that the core is one of the supported cores below
#
CORE_STR            = aes:md5:des3:sha256:gps:dft:idft:fir:iir:rsa
#
#
#
CORE_LIST           = $(subst  :, ,${CORE_STR})
SUPPORTED_CORE_LIST = ":$(CORE_STR):"
# Added :<core>: for exact match
CORE2CHECK = :${CORE}:
ifeq "$(findstring ${CORE2CHECK},${SUPPORTED_CORE_LIST})" ""
$(error : ERROR: "${CORE}" is not one of supported cores: ${SUPPORTED_CORE_LIST})
endif
#
# derived some macros/defines to be used in simulation
#
UPPER_NAME  = $(shell echo ${CORE} | tr '[:lower:]' '[:upper:]')
#
# top level chip netlist contains all the tilelink wrappers for all cores.
#
CEP_PLUSARG_READER    = ../../hdl_cores/freedom/rocket-chip/src/main/resources/vsrc/plusarg_reader.v
CEP_TOP_LEVEL_NETLIST = ../../hdl_cores/freedom/builds/vc707-u500devkit/sifive.freedom.unleashed.DevKitU500FPGADesign_WithDevKit50MHz.v

#
# Verilog file list for the selected core
#
#CORE_VFILES = ../${CORE}_sim/*.v
# pull them from hdl_core
CORE_VFILES = 	-y ../../hdl_cores/aes \
		-y ../../hdl_cores/des3 \
		-y ../../hdl_cores/dsp \
		-y ../../hdl_cores/gps \
		-y ../../hdl_cores/md5 \
		-y ../../hdl_cores/rsa/rtl \
		-y ../../hdl_cores/sha256 \
		-y ../../generated_dsp_code	\
		-v ../../hdl_cores/aes/table.v	\
		-v ../../hdl_cores/aes/round.v

#
# Where to look for files
#
VLOG_DIR    =   ./supports

ifeq (${LLKI_EN},1)
OTHERS_FILES = -v ./supports/VCShell_llki.v ../llki_supports/llki_tl.sv ../llki_supports/llki_discrete.sv
DUT_NAME     = TL${UPPER_NAME}_mock
CORE_VFILES  += ../${CORE}_sim/*mock_tss.sv
VLOG_DIR     += ../llki_supports
DEFINE_LIST  += +define+USE_MOCK_LLKI +define+CORE_STRUCT_NAME=${UPPER_NAME}_LLKI_STRUCT
else
DUT_NAME     = TL${UPPER_NAME}
OTHERS_FILES = -v ./supports/VCShell.v
endif
OTHERS_FILES += ${CEP_PLUSARG_READER}

#
TB_NAME     = ${DUT_NAME}_tb


#
# and its search directories
#
SEARCH_DIR = $(foreach t,${VLOG_DIR}, -y ${t})
INC_DIR    = $(foreach t,${VLOG_DIR}, +incdir+${t})
#
# defines to be passed to simulation for the selected core
#
DEFINE_LIST += +define+DUT_NAME=${DUT_NAME} +define+CORE_NAME=${CORE}
DEFINE_LIST += +define+CMD_FILE="\"./sv_vectors/${CORE}_playback.h"\"
DEFINE_LIST += +define+VERBOSE=${VERBOSE}
#
# Need by top level file
#
DEFINE_LIST += +define+RANDOMIZE_MEM_INIT 
#
# ------------------------------------------
# compile/optimize and simulate the selected core
# ------------------------------------------
#
all: ./supports/VCShell_llki.v ./supports/VCShell_llki.v
ifeq (${CADENCE},1)
	rm -rf xcelium.d
	xrun -64 -sv +acc ../llki_supports/llki_discrete.sv  ${DEFINE_LIST} ./supports/tl_unit_tb.sv ${CORE_VFILES} ${OTHERS_FILES} ${SEARCH_DIR} ${INC_DIR} +libext+.v +libext+.sv  -xlm_bq1
else
	rm -rf work
	vlog -64 -sv +acc ${DEFINE_LIST} ./supports/tl_unit_tb.sv ${CORE_VFILES} ${OTHERS_FILES} ${SEARCH_DIR} ${INC_DIR} +libext+.v +libext+.sv 
	vopt -64 +nolibcell +nospecify +notimingchecks ${TB_NAME} -o ${TB_NAME}_opt
	vsim -64 ${TB_NAME}_opt -batch -do vsim.do -logfile ${CORE}.log -wlf ${CORE}.wlf
endif

#
# No LLKI
#

#./supports/VCShell.v: ./supports/VCShell.v.gz
#	gunzip -c $< > $@

./supports/VCShell.v: ${CEP_TOP_LEVEL_NETLIST}
	echo "\`include \"config.v\"" > $@
	sed -e 's/IBUF_DELAY_VALUE(0)/IBUF_DELAY_VALUE("0")/' $< >> $@
	touch $@

#
# While we dont have the real VCShell with LLKI included
# Modify the netlist to insert LLKI-TL and LLKI-discrete to the TL wrappers
#
./supports/VCShell_llki.v: ./supports/VCShell.v
	./supports/insertLLKI.pl $< $@

#
# ------------------------------------------
# Use this target to run all cores
# ------------------------------------------
#
runAll:
	@for i in ${CORE_LIST}; do	\
	   make CORE=$${i}; 		\
	done
	grep TEST *.log 

#
# ------------------------------------------
# Preprocess playback C-header files to be read by systemVerilog
#
# Execute this manually if there is any change in the captured *.h file
#
# ------------------------------------------
#
C_PLAYBACK_DIR   = ../../cosim/drivers/vectors
SV_PLAYBACK_DIR  = ./sv_vectors
C_PLAYBACK_FLIST = $(wildcard ${C_PLAYBACK_DIR}/*.h)
C_PLAYBACK_FILES = $(foreach t,${notdir ${C_PLAYBACK_FLIST}}, ${t})
#
# convert C-header files to system-verilog format
#
# NOTE: dft/idft are captured in a single file => need to split it (hence the last sed command)
#
vectors: 
	@for i in ${C_PLAYBACK_FILES}; do			\
	   echo "Converting " ${C_PLAYBACK_DIR}/$${i} " to " ${SV_PLAYBACK_DIR}/$${i}; \
	   sed  -e 's/0\x/\'\''h/g' \
		-e 's/#define/`define/g' \
		-e 's/WRITE/`WRITE/g' \
		-e 's/RD/`RD/g' \
		-e 's/#/`/g' \
		-e 's/`define `/`define /g' \
		-e 's/uint64_t/longint/g' \
		${C_PLAYBACK_DIR}/$${i} > ${SV_PLAYBACK_DIR}/$${i}; \
	done
	sed -e 's/dft/idft/g' \
	    -e 's/h0070050000/h0070060000/'  ${SV_PLAYBACK_DIR}/dft_playback.h > ${SV_PLAYBACK_DIR}/idft_playback.h
#
# Clean up
#
clean:
	rm -rf work *.wlf *.log transcript ./supports/VCShell*.v

