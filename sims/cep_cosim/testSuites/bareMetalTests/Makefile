 #************************************************************************
# Copyright 2022 Massachusets Institute of Technology
# SPDX short identifier: BSD-2-Clause
#
# File Name:      Makefile
# Program:        Common Evaluation Platform (CEP)
# Description:    Makefile for bareMetalTest test suite
# Notes:          
#
#************************************************************************

# Causes a cleaning prior to running the test suite
STAND_ALONE 	?= 1

COSIM_TOP_DIR   	= $(strip $(shell cd ../..; pwd))
REPO_TOP_DIR 		= $(strip $(shell cd ${COSIM_TOP_DIR}/../..; pwd))
TEST_SUITE_DIR		= $(strip $(shell cd .; pwd))
TEST_SUITE_NAME		= $(shell cd .; basename `pwd`)

default: usage

# List of tests to run
TEST_LIST 		= 		\
	accessTest 			\
	atomicOps			\
	cacheCoherence		\
	clintIntr 			\
	clintTest 			\
	csrTest 			\
	fullBoot 			\
	idcache_smc 		\
	lockfreeAtomic 		\
	lockTest 			\
	macroMix 			\
	macro2Mix 			\
	macro3Mix 			\
	macro4Mix 			\
	miscTests 			\
	multiLock 			\
	plicTest 			\
	regTest 			\
	srotMemTest			\

# Tests yet to run/fix/debug
#plicPrioIntr
#extIntr
#lsrcOps


runAll: 
	@for i in ${TEST_LIST}; do 									\
		(cd ${TEST_SUITE_DIR}/$${i}; make cleanTest; make);		\
	done

summary:
	@date > ${TEST_SUITE_DIR}/testSuiteSummary
	@for i in ${TEST_LIST}; do				\
		cat ${TEST_SUITE_DIR}/$${i}/status | sort | tee -a testSuiteSummary;	\
	done

# Include current test suite common.make
include ./common.make


