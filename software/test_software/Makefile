#************************************************************************
# Copyright (C) 2018 Massachusetts Institute of Technology
#
# File Name:      Makefile 
# Program:        Common Evaluation Platform (CEP)
# Description:    Makefile for the CEP regression suite
# Notes:          
#************************************************************************

default:
	@echo "sys   : Create a VMEM file to be loaded onto a SoC for full-system verification"
	@echo "clean : Remove all generated files"

sys:
	or1k-elf-g++ -Wall -Wno-long-long -pedantic -O3 -I ../../hdl_cores/aes ../bootloader/vc707_CEP.S cep_regression.cpp -o cep_regression
	or1k-elf-objdump -D cep_regression > cep_regression.S
	or1k-elf-objcopy -O binary cep_regression cep_regression.bin
	../../software/utils/bin2vmem cep_regression.bin > cep_regression.vmem
	cp cep_regression.vmem ../../hdl_cores/ram/sram.vmem

clean:
	rm -rf obj_dir iverilog *~ *.S *.bin *.vmem `find . -type f -executable`
