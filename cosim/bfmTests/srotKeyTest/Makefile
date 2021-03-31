#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
<<<<<<< HEAD:cosim/bfmTests/srotKeyTest/Makefile
#// SPDX License Identifier: MIT
=======
#// SPDX short identifier: BSD-2-Clause
>>>>>>> 6494113db2448733228b0f6659bfa0a7fedc93c0:cosim/bfmTests/fragmentors/Makefile
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
DUT_TOP_DIR =  $(shell cd ../../..; pwd | ./${COSIM_NAME}/bin/strip_net.pl )
BLD_DIR     =  $(shell cd ..; pwd | ../bin/strip_net.pl )
TEST_SUITE  =  $(shell basename ${BLD_DIR})
TEST_DIR    =  $(shell cd .; pwd | ../../bin/strip_net.pl )
TEST_NAME   =  $(shell basename `pwd`)
SIM_DIR     = ${DUT_TOP_DIR}/${COSIM_NAME}

#
# Top target!!!
#
all: .vrun_flag

#
# override anything here before calling the common file
#
include ${BLD_DIR}/common.make

