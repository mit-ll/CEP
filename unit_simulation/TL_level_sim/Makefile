#//************************************************************************
#// Copyright (C) 2020 Massachusetts Institute of Technology
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
CORE        = aes
#
# Turn on read/write messages during simulation. Default is ON
#
VERBOSE     = 1
#
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
DUT_NAME    = TL${UPPER_NAME}
TB_NAME     = ${DUT_NAME}_tb
#
# Where to look for files
#
VLOG_DIR    =   ./supports
#
# Verilog file list for the selected core
#
CORE_VFILES = ../${CORE}_sim/*.v
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
# top level chip netlist contains all the tilelink wrappers for all cores.
#
OTHERS_FILES = -v ./supports/VCShell.v 
#
# ------------------------------------------
# compile/optimize and simulate the selected core
# ------------------------------------------
#
all: ./supports/VCShell.v
	rm -rf work
	vlog -64 -sv +acc ${DEFINE_LIST} ./supports/tl_unit_tb.sv ${CORE_VFILES} ${OTHERS_FILES} ${SEARCH_DIR} ${INC_DIR} +libext+.v +libext+.sv 
	vopt -64 +nolibcell +nospecify +notimingchecks ${TB_NAME} -o ${TB_NAME}_opt
	vsim -64 ${TB_NAME}_opt -batch -do vsim.do -logfile ${CORE}.log -wlf ${CORE}.wlf


./supports/VCShell.v: ./supports/VCShell.v.gz
	gunzip -c $< > $@

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
	rm -rf work *.wlf *.log transcript
