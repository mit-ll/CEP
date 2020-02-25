#//************************************************************************
#// Copyright (C) 2020 Massachusetts Institute of Technology
#//
#// File Name:      
#// Program:        Common Evaluation Platform (CEP)
#// Description:    
#// Notes:          
#//
#//************************************************************************

# RISCV environment variable must be set

CC=$(RISCV)/bin/riscv64-unknown-elf-gcc
OBJCOPY=$(RISCV)/bin/riscv64-unknown-elf-objcopy
OBJDMP=$(RISCV)/bin/riscv64-unknown-elf-objdump
CFLAGS=-march=rv64imac -mcmodel=medany -O2 -std=gnu11 -Wall -nostartfiles 
CFLAGS+= -fno-common -g -DENTROPY=0 -mabi=lp64 -DNONSMP_HART=0 
CFLAGS+= -I ./include -I.
LFLAGS=-static -nostdlib -L ./linker -T sdboot.elf.lds

export CONFIG_PROJECT := sifive.freedom.unleashed
export CONFIG := DevKitU500FPGADesign_WithDevKit50MHz

#
# OR can be passed in from top
#
#BUILD_DIR := /home/aduong/CEP/CEP-master/hdl_cores/freedom/builds/vc707-u500devkit
BUILD_DIR := ../../../hdl_cores/freedom/builds/vc707-u500devkit

dts := $(BUILD_DIR)/$(CONFIG_PROJECT).$(CONFIG).dts
dtb := $(BUILD_DIR)/$(CONFIG_PROJECT).$(CONFIG).dtb
clk := $(BUILD_DIR)/$(CONFIG_PROJECT).$(CONFIG).tl_clock.h

all: head.S  bootbare.c 
	$(CC) $(CFLAGS) -include $(clk) -DDEVICE_TREE='"$(dtb)"' $(LFLAGS) -o bootbare.elf head.S bootbare.c 
	$(OBJCOPY) -O binary bootbare.elf bootbare.bin
	${OBJDMP} -S bootbare.elf > bootbare.dump
	od -t x4 -An -w4 -v bootbare.bin > bootbare.hex

$(clk): $(dts)
	awk '/tlclk {/ && !f{f=1; next}; f && match($$0, /^.*clock-frequency.*<(.*)>.*/, arr) { print "#define TL_CLK " arr[1] "UL"}' $< > $@.tmp
	mv $@.tmp $@

$(dtb): $(dts)
	dtc -I dts -O dtb -o $@ $<

.PHONY: clean
clean::
	rm -rf *.bin *.hex *.elf
