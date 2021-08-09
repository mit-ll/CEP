#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
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
COSIM_NAME  = $(shell cd ../..; basename `pwd`)
DUT_TOP_DIR = $(shell cd ../../..; pwd )
BLD_DIR     = $(shell cd ..; pwd )
TEST_SUITE  = $(shell basename ${BLD_DIR})
TEST_DIR    = $(shell cd .; pwd )
TEST_NAME   = $(shell basename `pwd`)
SIM_DIR     = ${DUT_TOP_DIR}/${COSIM_NAME}
ELF_FILE    = ${RISCV_TEST_DIR}/isa/${TEST_NAME}

#
# Top target!!!
#
all: .vrun_flag 

#
# override anything here before calling the common file
#
include ${BLD_DIR}/common.make



