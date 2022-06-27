#************************************************************************
# Copyright 2022 Massachusets Institute of Technology
# SPDX short identifier: BSD-2-Clause
#
# File Name:      Makefile
# Program:        Common Evaluation Platform (CEP)
# Description:    Standalone Makefile for baremetal RISC-V executables
#                 to run on the CEP Arty100t build
# Notes:          
#
#************************************************************************

# RISCV *must* be defined (while BFM mode does not use RISCV executables, the SW process builds EVERYTHING, including RISCV)
ifndef RISCV
$(error RISCV is unset.  You must set RISCV yourself, or through the Chipyard auto-generated env file)
endif

BASEDIR 	:= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
INCDIR 		:= $(BASEDIR)/include
MAINPROGRAM	?= gpiotest

CC 			:= riscv64-unknown-elf-gcc
OBJDUMP 	:= riscv64-unknown-elf-objdump
OBJCOPY 	:= riscv64-unknown-elf-objcopy
CFLAGS 		:= -mcmodel=medany -std=gnu99 -O2 -fno-common -fno-builtin-printf -Wall -Wno-unused-function
CFLAGS 		+= -I $(INCDIR)
LDFLAGS 	:= -static -nostdlib -nostartfiles -lgcc
OBJS 		:= crt.o syscalls.o kprintf.o
LDSCRIPT 	:= link.ld
HDRS 		:= $(wildcard *.h) $(wildcard $(INCDIR)/*.h)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -o $@ -c $<
%.o: %.S $(hdrs)
	$(CC) $(CFLAGS) -D__ASSEMBLY__=1 -o $@ -c $<
%.elf: %.o $(OBJS) $(LDSCRIPT)
	$(CC) $(LDFLAGS) $(if $(LDSCRIPT),-T $(LDSCRIPT)) -o $@ $< $(OBJS)
%.img: %.elf
	$(OBJCOPY) -O binary $< $@
%.dump: %.elf
	$(OBJDUMP) -d $< > $@

.DEFAULT: img

.PHONY: img elf dump
img: $(addsuffix .img,$(MAINPROGRAM)) 
elf: $(addsuffix .elf,$(MAINPROGRAM))
dump: $(addsuffix .dump,$(MAINPROGRAM))

.PHONY: clean
clean:
	rm -f -- *.elf *.o *.dump *.img

.SUFFIXES:
.SUFFIXES: .o .c .S

sd_write: img
ifneq (,$(wildcard ${DISK}))
	sudo dd if=$(MAINPROGRAM).img of=$(DISK) bs=4096 conv=fsync
else
	$(error Invalid DISK specified for SD write)
endif
