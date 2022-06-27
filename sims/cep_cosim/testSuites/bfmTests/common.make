#--------------------------------------------------------------------------------------
# Copyright 2022 Massachusets Institute of Technology
#
# File Name:      common.make
# Program:        Common Evaluation Platform (CEP)
# Description:    common.make for bfmTests test suite
# Notes:          
#
#--------------------------------------------------------------------------------------

# override anything here before calling the top 
override DUT_SIM_MODE			= BFM

# Call root cosimulation common.make
include ${COSIM_TOP_DIR}/common.make
