#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
#// SPDX License Identifier: MIT
#//
#// File Name:      
#// Program:        Common Evaluation Platform (CEP)
#// Description:    
#// Notes:          
#//
#//************************************************************************
#
# Can be override
#
STAND_ALONE = 1

all: runAll

#
# needed by common.make
#
COSIM_NAME  = $(shell cd ..; basename `pwd`)
DUT_TOP_DIR =  $(shell cd ../..; pwd | ./${COSIM_NAME}/bin/strip_net.pl )
BLD_DIR     =  $(shell cd .; pwd | ../bin/strip_net.pl )
TEST_SUITE  =  $(shell basename ${BLD_DIR})
SIM_DIR     = ${DUT_TOP_DIR}/${COSIM_NAME}

# Always in regression mode
override REGRESS = 1

#
# Top target!!!
#

#
# Either run one at a time or batch the wholething if supported
#


#
# override anything here before calling the common file
#

include ../common.make

#
# List of test under this directory
#

BFM_TEST_LIST = \
	regTest		\
	macroMix	\
	playback	\
	lockTest	\
	multiLock	\
	ddr3Test	\
	memPreload	\
	miscTests	\
	accessTest	\
	clintTest	\
	plicTest	\

TESTINFO_LIST := $(sort $(foreach t,${TALUS_TEST_LIST},${BLD_DIR}/${t}/testInfo.txt))

#
# talus tests use lot of memory => need big machines
#

runAll: 
ifeq (${STAND_ALONE},1)
	make cleanAll
	make libsBuild
endif
	-rm -rf ${BLD_DIR}/*_work ${BLD_DIR}/.*work_dependList.make
	-rm -f ${BLD_DIR}/*/*.elf
	-rm -rf ${BLD_DIR}/.*_dependList* ${BLD_DIR}/.is_checked
	make buildSim
	@for i in ${BFM_TEST_LIST}; do							\
		echo "    PEND : ${BLD_DIR}/$${i}"  > ${BLD_DIR}/$${i}/status;		\
		(cd ${BLD_DIR}/$${i}; rm -f *.o*; 					\
		make NOWAVE=1 TL_CAPTURE=${TL_CAPTURE} C2C_CAPTURE=${C2C_CAPTURE} TEST_NAME=$${i});			\
	done

summary:
	@date > regressionSummary
	@for i in ${BFM_TEST_LIST}; do					\
		cat ${BLD_DIR}/$${i}/status | tee -a regressionSummary;	\
	done

FPGA_INST_NAME     ?= /cep_tb/fpga

merge:
	@if test -d ${DUT_COVERAGE_PATH}; then	\
		(cd ${DUT_COVERAGE_PATH}; ${VCOVER_CMD} merge -out ${TEST_SUITE}.ucdb ${DUT_COVERAGE_PATH}/*.ucdb;) \
	fi

# fixme !!! These need to be in a do file
cov_report:
	coverage open ../coverage/des3.ucdb
	coverage report -html -output covhtmlreport -details -assert -directive -cvg -code bcefst -threshL 50 -threshH 90




