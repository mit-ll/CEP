#//************************************************************************
#// Copyright (C) 2020 Massachusetts Institute of Technology
#//
#// File Name:      Makefile     
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Testbench makefile
#// Notes:          
#//
#//************************************************************************

TB_NAME = idft_top_tb

all:
	vlog -64 +acc *.v
	vlog -64 +acc ../../generated_dsp_code/*.v
	vlog -64 -sv +acc *.sv
	vopt -64 +nolibcell +nospecify +notimingchecks ${TB_NAME} -o ${TB_NAME}_opt
	vsim -64 ${TB_NAME}_opt -batch -do vsim.do

clean:
	rm -rf work *.wlf
