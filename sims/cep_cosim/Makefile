#************************************************************************
# Copyright 2022 Massachusets Institute of Technology
# SPDX short identifier: BSD-2-Clause
#
# File Name:      Makefile
# Program:        Common Evaluation Platform (CEP)
# Description:    Top Level Makefile for CEP CoSim
# Notes:          
#
#************************************************************************

COSIM_TOP_DIR   	= $(strip $(shell cd .; pwd))
REPO_TOP_DIR 		= $(strip $(shell cd ${COSIM_TOP_DIR}/../..; pwd))

# Each test suite will set its own values
DUT_SIM_MODE		= BFM
ELF_MODE   			= LOCAL

default: usage

# Tests to run as part of the default build target
TEST_SUITES  =  	bareMetalTests	\
					bfmTests		\
					isaTests

# Run tests for all members of the test group
runAll: 
	@for i in ${TEST_SUITES}; do											\
		(cd ${COSIM_TOP_DIR}/testSuites/$${i}; make STAND_ALONE=0 runAll); 	\
	done

# Print summary
summary: .force
	@date > ${COSIM_TOP_DIR}/regressionSummary
	@for i in ${TEST_SUITES}; do								\
		(cd ${COSIM_TOP_DIR}/testSuites/$${i}; make -s STAND_ALONE=0 summary | sort | tee -a ${COSIM_TOP_DIR}/regressionSummary);	\
	done

# Include the common makefile
include $(COSIM_TOP_DIR)/common.make