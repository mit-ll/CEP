#
# modified by Tony D. 04/16/21
# The caller MUST pass in 2 arguments
# SDNAME & DEFINES
#

# RISCV environment variable must be set

CC=$(RISCV)/bin/riscv64-unknown-elf-gcc
AR=$(RISCV)/bin/riscv64-unknown-elf-gcc-ar
OBJCOPY=$(RISCV)/bin/riscv64-unknown-elf-objcopy
OBJDUMP=$(RISCV)/bin/riscv64-unknown-elf-objdump
CFLAGS=-march=rv64imac -mcmodel=medany -O2 -std=gnu11 -Wall -nostartfiles 
CFLAGS+= -fno-common -DENTROPY=0 -mabi=lp64 -DNONSMP_HART=0 
CFLAGS+= -I $(BOOTROM_DIR)/include -I.
CFLAGS+= ${EXTRA_CFLAGS}
LFLAGS=-static -nostdlib -L $(BOOTROM_DIR)/linker -T sdboot.elf.lds

dts := $(BUILD_DIR)/$(CONFIG_PROJECT).$(CONFIG).dts
dtb := $(BUILD_DIR)/$(CONFIG_PROJECT).$(CONFIG).dtb
clk := $(BUILD_DIR)/$(CONFIG_PROJECT).$(CONFIG).tl_clock.h

$(clk): $(dts)
	awk '/tlclk {/ && !f{f=1; next}; f && match($$0, /^.*clock-frequency.*<(.*)>.*/, arr) { print "#define TL_CLK " arr[1] "UL"}' $< > $@.tmp
	mv $@.tmp $@

$(dtb): $(dts)
	dtc -I dts -O dtb -o $@ $<

.PHONY: dtb
dtb: $(dtb)

elf := $(BUILD_DIR)/${SDNAME}.elf
$(elf): $(dtb) head.S kprintf.c sd.c $(clk)
	$(CC) $(CFLAGS) -include $(clk) -DDEVICE_TREE='"$(dtb)"' $(LFLAGS) -o $@ head.S sd.c kprintf.c ${DEFINES} 

.PHONY: elf
elf: $(elf)

bin := $(BUILD_DIR)/${SDNAME}.bin
$(bin): $(elf)
	$(OBJCOPY) -O binary $< $@
	$(OBJDUMP) -S -C -d -l -x ${elf} > ${elf}.dump

.PHONY: bin
bin: $(bin)

hex := $(BUILD_DIR)/${SDNAME}.hex
$(hex): $(bin)
	od -t x4 -An -w4 -v $< > $@

.PHONY: hex
hex: $(hex)

romgen := $(BUILD_DIR)/rom.v
$(romgen): $(hex)
	$(rocketchip_dir)/scripts/vlsi_rom_gen $(ROMCONF) $< > $@

.PHONY: romgen
romgen: $(romgen)

.PHONY: clean
clean::
	rm -rf $(hex) $(elf)
