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
#
override DUT_SIM_MODE	= BFM
override DUT_XILINX_TOP_MODULE = cep_tb
#
include ${DUT_TOP_DIR}/${COSIM_NAME}/common.make
