#//************************************************************************
#// Copyright 2022 Massachusets Institute of Technology
#// SPDX short identifier: BSD-2-Clause
#//
#// File Name:      
#// Program:        Common Evaluation Platform (CEP)
#// Description:    
#// Notes:          
#//
#//************************************************************************
#
#
#
COSIM_DIR_NAME  = $(shell cd ../..; basename `pwd`)
REPO_TOP_DIR = $(shell cd ../../..; pwd | ./${COSIM_DIR_NAME}/bin/strip_net.pl )
BLD_DIR     = $(shell cd ..; pwd | ../bin/strip_net.pl )
TEST_SUITE  = $(shell basename ${BLD_DIR})
TEST_DIR    = $(shell cd .; pwd | ../../bin/strip_net.pl )
TEST_NAME   = $(shell basename `pwd`)
SIM_DIR     = ${REPO_TOP_DIR}/${COSIM_DIR_NAME}
ELF_FILE    = ${RISCV_TEST_DIR}/isa/${TEST_NAME}

#
# override anything here before calling the common file
#
NO_BUILTIN_ELF_MODE = 1

#
# Top target!!!
#	
#
# This test needs *openocd* to control the JTAG port
#
OPENOCD_CHECK = $(filter Debugger, $(shell openocd --version 2>&1 | grep Debugger))


ifeq "$(OPENOCD_CHECK)" "Debugger"
all: .vrun_flag
	@echo "**** OpenOcd app is found .. Will run this test.... :$(OPENOCD_CHECK):"
else
all:
	@echo "**** OpenOcd app not found.. Skipping this test.... :$(OPENOCD_CHECK):"
endif

include ${BLD_DIR}/common.make




