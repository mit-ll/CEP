#//************************************************************************
#// Copyright 2022 Massachusets Institute of Technology
#// SPDX License Identifier: BSD-2-Clause
#//
#// File Name:      
#// Program:        Common Evaluation Platform (CEP)
#// Description:    
#// Notes:          
#//
#//************************************************************************

# Without the following, RHEL7 does not execute the simulation process properly
.NOTPARALLEL:

COSIM_TOP_DIR   	= $(strip $(shell cd ../..; pwd))
REPO_TOP_DIR 		= $(strip $(shell cd ${COSIM_TOP_DIR}/../..; pwd))
TEST_SUITE_DIR		= $(strip $(shell cd .; pwd))
TEST_SUITE_NAME		= $(shell cd .; basename `pwd`)

default: usage

TEST_LIST = 		\
	accessTest		\
	clintTest		\
	lockTest		\
	macroMix		\
	miscTests		\
	multiLock		\
	multiThread		\
	plicPrioIntr	\
	plicTest		\
	regTest			\
	scratchpadTest	\
	srotMemTest		\
	srotBadKeys		\
	srotKeyTest

# Tests yet to run/fix/debug
#	srotErrorTest

runAll: cleanSuite
	@for i in ${TEST_LIST}; do 									\
		make -C ${TEST_SUITE_DIR}/$${i} all; 					\
	done

summary:
	@date > ${TEST_SUITE_DIR}/testSuiteSummary
	@for i in ${TEST_LIST}; do				\
		cat ${TEST_SUITE_DIR}/$${i}/status | sort | tee -a testSuiteSummary;	\
	done

# Include current test suite common.make
include ./common.make


