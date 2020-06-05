#//************************************************************************
#// Copyright (C) 2020 Massachusetts Institute of Technology
#//
#// File Name:      Makefile     
#// Program:        Common Evaluation Platform (CEP)
#// Description:    Testbench makefile
#// Notes:          
#//	to run , type "make" 
#//
#//************************************************************************

DUT_NAME = gps

#
#
TB_NAME  = ${DUT_NAME}_tb
#
#
#
all: gps_stimulus.txt
	vlog -64 -sv +acc *.sv *.v  +define+DUT_NAME=${DUT_NAME} +define+TB_NAME=${TB_NAME}
	vopt -64 +nolibcell +nospecify +notimingchecks ${TB_NAME} -o ${TB_NAME}_opt
	vsim -64 ${TB_NAME}_opt -batch -do vsim.do -logfile ${TB_NAME}.log -wlf ${TB_NAME}.wlf

gps_stimulus.txt : gps_stimulus.txt.gz
	gunzip -c $< > $@

clean:
	rm -rf work *.wlf *.log *.txt


