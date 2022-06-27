#--------------------------------------------------------------------------------------
# Copyright 2022 Massachusets Institute of Technology
#
# File Name:      common.make
# Program:        Common Evaluation Platform (CEP)
# Description:    common.make for isaTests test suite
# Notes:          
#
#--------------------------------------------------------------------------------------

# override anything here before calling the top 
override DUT_SIM_MODE			= BARE
override RISCV_TESTS			= 1

ifeq (${NO_BUILTIN_ELF_MODE},1)
override ELF_MODE   			= LOCAL
else
override ELF_MODE 	 			= BUILTIN
endif

# Call root cosimulation common.make
include ${COSIM_TOP_DIR}/common.make
