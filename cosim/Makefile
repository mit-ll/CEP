#//************************************************************************
#// Copyright (C) 2020 Massachusetts Institute of Technology
#//
#// File Name:      Makefile
#// Program:        Common Evaluation Platform (CEP)
#// Description:    
#// Notes:          
#//
#//************************************************************************

all: runAll

# Always in regression mode
override REGRESS = 1
override NOWAVE  = 1
override BOARD_SIM = 1
override COVERAGE  = 1
UPDATE_INFO = 1
#
#
COSIM_NAME  = $(shell basename `pwd`)
#
DUT_TOP_DIR = $(shell cd ..; pwd | ./${COSIM_NAME}/bin/strip_net.pl )
BLD_DIR     = $(shell cd ./bfmTests; pwd | ../bin/strip_net.pl )
TEST_SUITE  = $(shell basename ${BLD_DIR})
TOP_DIR     = $(shell cd .; pwd | ./bin/strip_net.pl )
SIM_DIR     = ${DUT_TOP_DIR}/${COSIM_NAME}

#
# Either run one at a time or batch the whole thing if supported (TBA)
#

TEST_GROUP  =  	bfmTests	\
		bareMetalTests

runAll: 
	make cleanAll
	make libsBuild
	@for i in ${TEST_GROUP}; do				\
		rm -f $${i}/$${i}.log;				\
		(cd $${i}; make STAND_ALONE=0 runAll); 		\
	done

summary: .force
	@date > ${TOP_DIR}/regressionSummary
	@for i in ${TEST_GROUP}; do								\
		(cd $${i}; make -s STAND_ALONE=0 summary | sort | tee -a ${TOP_DIR}/regressionSummary);	\
	done

# use "coverage open <file>.ucdb" under vsim to open and view
merge: .force
	@for i in ${TEST_GROUP}; do	\
		(cd $${i}; make merge);	\
	done

#
# override anything here before calling the common file
#
include ./common.make


