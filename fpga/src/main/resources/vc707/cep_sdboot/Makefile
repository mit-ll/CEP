# Default parameters
PBUS_CLK 		?= 1000000

# RISCV environment variable must be set
REPO_ROOT_DIR := $(shell git rev-parse --show-toplevel)
ROOT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := $(ROOT_DIR)/build

CC			=$(RISCV)/bin/riscv64-unknown-elf-gcc
OBJCOPY 	=$(RISCV)/bin/riscv64-unknown-elf-objcopy
OBJDUMP 	=$(RISCV)/bin/riscv64-unknown-elf-objdump
CFLAGS 		=  -mcmodel=medany -O2 -Wall -fno-common -fno-builtin-printf -fno-builtin-puts
CFLAGS 		+= -I $(ROOT_DIR)/include -I.
CFLAGS 		+= -mabi=lp64 -march=rv64ima
CFLAGS 		+= -DENTROPY=0 -DNONSMP_HART=0
LFLAGS 		=  -static -nostdlib -nostartfiles -T sdboot.elf.lds
SDSOURCE	= ${REPO_ROOT_DIR}/sims/cep_cosim/bootrom/sd.c

default: elf bin dump

elf := $(BUILD_DIR)/sdboot.elf
$(elf): head.S ${SDSOURCE} kprintf.c syscalls_bootrom.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -DVC707 -DTL_CLK="$(PBUS_CLK)UL" $(LFLAGS) -o $@ head.S ${SDSOURCE} kprintf.c syscalls_bootrom.c

.PHONY: elf
elf: $(elf)

bin := $(BUILD_DIR)/sdboot.bin
$(bin): $(elf)
	mkdir -p $(BUILD_DIR)
	$(OBJCOPY) -O binary --change-addresses=-0x10000 $< $@

.PHONY: bin
bin: $(bin)

dump := $(BUILD_DIR)/sdboot.dump
$(dump): $(elf)
	$(OBJDUMP) -D -S $< > $@

.PHONY: dump
dump: $(dump)

.PHONY: clean
clean::
	rm -rf $(BUILD_DIR)
