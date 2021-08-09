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
# Can be override
#
STAND_ALONE = 1

all: runAll

#
# needed by common.make
#
COSIM_NAME  = $(shell cd ..; basename `pwd`)
DUT_TOP_DIR = $(shell cd ../..; pwd | ./${COSIM_NAME}/bin/strip_net.pl )
BLD_DIR     = $(shell cd .; pwd | ../bin/strip_net.pl )
TEST_SUITE  = $(shell basename ${BLD_DIR})
SIM_DIR     = ${DUT_TOP_DIR}/${COSIM_NAME}


# Always in regression mode
override REGRESS        = 1
override DUT_SIM_MODE	= BARE
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
VIRT_TEST_LIST = 		\
	misalignLdSt2Levels	\
	dTLB			\
	l0_iTLB	 		\
	l1_iTLB			\
	l2_iTLB			\
	l2_scratch		\
	l2_ucache		\
	pt3levels		\
	directMap2Levels	\
	superpage2Level		\
	special1Level 		\

#	directMap1Level 	\

#x	rdwrDat_sv39	\


TESTINFO_LIST := $(sort $(foreach t,${TALUS_TEST_LIST},${BLD_DIR}/${t}/testInfo.txt))

#
# talus tests use lot of memory => need big machines
#

runAll: 
ifeq (${STAND_ALONE},1)
	make cleanAll
	make buildLibs
endif
	-rm -rf ${BLD_DIR}/*_work ${BLD_DIR}/.*work_dependList.make
	-rm -f ${BLD_DIR}/*/*.elf
	-rm -rf ${BLD_DIR}/.*_dependList* ${BLD_DIR}/.is_checked
	make buildSim
	@for i in ${VIRT_TEST_LIST}; do							\
		echo "    PEND : ${BLD_DIR}/$${i}"  > ${BLD_DIR}/$${i}/status;		\
		(cd ${BLD_DIR}/$${i}; rm -f *.o*; 					\
		make NOWAVE=1 TEST_NAME=$${i});						\
	done

summary:
	@date > regressionSummary
	@for i in ${VIRT_TEST_LIST}; do				\
		cat ${BLD_DIR}/$${i}/status | tee -a regressionSummary;	\
	done




