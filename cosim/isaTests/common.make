#//************************************************************************
#// Copyright 2021 Massachusetts Institute of Technology
#//
#// File Name:      
#// Program:        Common Evaluation Platform (CEP)
#// Description:    
#// Notes:          
#//
#//************************************************************************
#
# override anything here before calling the top 
#
# must have this one for auto-dependentcy detection

override  DUT_SIM_MODE	= BARE
ifeq (${NO_BUILDIN_ELF_MODE},1)
else
override  DUT_ELF_MODE  = BUILTIN
endif
override  DUT_XILINX_TOP_MODULE = cep_tb
#
include ${DUT_TOP_DIR}/${COSIM_NAME}/common.make


