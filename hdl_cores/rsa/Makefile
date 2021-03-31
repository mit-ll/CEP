#************************************************************************
# Copyright 2021 Massachusetts Institute of Technology
# SPDX short identifier: BSD-2-Clause
#
# File Name:      Makefile 
# Program:        Common Evaluation Platform (CEP)
# Description:    
# Notes:          
#************************************************************************
default:
	@echo "sys        : Create a VMEM file to be loaded onto a SoC"
	@echo "ver        : Verilator - verify the core plus a Wishbone bus interface"
	@echo "             INCOMPLETE: verilator bug"
	@echo "ver_modexp : Verilator - Verify the core with original bus interface"
	@echo "             INCOMPLETE: verilator bug"
	@echo "vcd        : Verilator - Generate VCD for ver"
	@echo "vcd_modexp : Verilator - Generate VCD for ver_modexp"
	@echo "top        : iVerilog  - Verify the core plus a Wishbone bus interface"
	@echo "modexp     : iVerilog  - Verify the core with original bus interface"
	@echo "modexp_orig: iVerilog  - Verify the core with original testbench"
	@echo ""
	@echo "gen        : create gen directory for testing"
	@echo "key        : extract public and private key"
	@echo "pub        : extract public key using Open SSL"
	@echo "pri        : extract private key using Open SSL"
	@echo "a2h        : ascii to hex converter"
	@echo "h2a        : hex to ascii converter"
	@echo "file       : loading\print\writing data with file"
	@echo "pText      : request and generate plain text"
	@echo "gText      : generate decrpted text"
	@echo ""
	@echo "tKey       : test key - full functionality"
	@echo "tTop       : test top - full functionality"
	@echo ""
	@echo "clean      : Remove all generated files"

sys:
	riscv64-unknown-elf-g++ -Wall -pedantic -03 tb_sys.cpp -o tb_sys
	 #or1k-elf-g++ -Wall -pedantic -O3 ../../software/bootloader/vc707_CEP.S tb_sys.cpp -o tb_sys
	riscv64-unknown-elf-objdump -D tb_sys > tb_sys.S
	 #or1k-elf-objdump -D tb_sys > tb_sys.S
	riscv64-unknown-elf-objcopy -O binary tb_sys tb_sys.bin
	 #or1k-elf-objcopy -O binary tb_sys tb_sys.bin
	../../software/utils/bin2vmem tb_sys.bin > tb_sys.vmem
	cp tb_sys.vmem ../ram/sram.vmem

ver:
	verilator --cc --clk clk --default-language 1364-2001 -I. ./rtl/modexp_top.v ./rtl/modexp_core.v ./rtl/montprod.v ./rtl/residue.v ./rtl/adder.v ./rtl/shl.v ./rtl/shr.v ./rtl/blockmem1r1w.v ./rtl/blockmem2r1w.v ./rtl/blockmem2r1wptr.v  ./rtl/blockmem2rptr1w.v --top-module modexp_top -exe tb_top.cpp
	make -j -C obj_dir -f Vmodexp_top.mk Vmodexp_top
	./obj_dir/Vmodexp_top

ver_modexp:
	verilator --cc --clk clk --default-language 1364-2001 -I. ./rtl/modexp.v ./rtl/modexp_core.v ./rtl/montprod.v ./rtl/residue.v ./rtl/adder.v ./rtl/shl.v ./rtl/shr.v ./rtl/blockmem1r1w.v ./rtl/blockmem2r1w.v ./rtl/blockmem2r1wptr.v  ./rtl/blockmem2rptr1w.v --top-module modexp -exe tb_modexp.cpp
	make -j -C obj_dir -f Vmodexp.mk Vmodexp
	./obj_dir/Vmodexp

vcd:
	verilator --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 -I. ./rtl/modexp_top.v ./rtl/modexp_core.v ./rtl/montprod.v ./rtl/residue.v ./rtl/adder.v ./rtl/shl.v ./rtl/shr.v ./rtl/blockmem1r1w.v ./rtl/blockmem2r1w.v ./rtl/blockmem2r1wptr.v  ./rtl/blockmem2rptr1w.v --top-module modexp_top -exe tb_top.cpp
	make -j -C obj_dir -f Vmodexp_top.mk Vmodexp_top
	./obj_dir/Vmodexp_top
	gtkwave ./obj_dir/Vmodexp_top.vcd

vcd_modexp:
	verilator --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 -I. ./rtl/modexp.v ./rtl/modexp_core.v ./rtl/montprod.v ./rtl/residue.v ./rtl/adder.v ./rtl/shl.v ./rtl/shr.v ./rtl/blockmem1r1w.v ./rtl/blockmem2r1w.v ./rtl/blockmem2r1wptr.v  ./rtl/blockmem2rptr1w.v --top-module modexp -exe tb_modexp.cpp
	make -j -C obj_dir -f Vmodexp.mk Vmodexp
	./obj_dir/Vmodexp
	gtkwave ./obj_dir/Vmodexp.vcd

top:
	mkdir -p ./iverilog/
	iverilog -Wall  tb_top.v ./rtl/modexp_top.v ./rtl/modexp_core.v ./rtl/montprod.v ./rtl/residue.v ./rtl/adder.v ./rtl/shl.v ./rtl/shr.v ./rtl/blockmem1r1w.v ./rtl/blockmem2r1w.v ./rtl/blockmem2r1wptr.v ./rtl/blockmem2rptr1w.v -o ./iverilog/top.sim
	./iverilog/top.sim

modexp:
	mkdir -p ./iverilog/
	iverilog -Wall tb_modexp.v ./rtl/modexp.v ./rtl/modexp_core.v ./rtl/montprod.v ./rtl/residue.v ./rtl/adder.v ./rtl/shl.v ./rtl/shr.v ./rtl/blockmem1r1w.v ./rtl/blockmem2r1w.v ./rtl/blockmem2r1wptr.v ./rtl/blockmem2rptr1w.v -o ./iverilog/modexp.sim
	./iverilog/modexp.sim

modexp_orig:
	mkdir -p ./iverilog/
	iverilog -Wall tb_modexp_orig.v ./rtl/modexp.v ./rtl/modexp_core.v ./rtl/montprod.v ./rtl/residue.v ./rtl/adder.v ./rtl/shl.v ./rtl/shr.v ./rtl/blockmem1r1w.v ./rtl/blockmem2r1w.v ./rtl/blockmem2r1wptr.v ./rtl/blockmem2rptr1w.v -o ./iverilog/modexp_orig.sim
	./iverilog/modexp_orig.sim 

gen:
	mkdir ./gen -p

key:
	$(MAKE) gen
	gcc ./src/extKeys.c -o extKeys
	./extKeys

pub:
	$(MAKE) gen
	openssl rsa -pubin -inform PEM -text -noout -in ./pem/pubkey.pem > ./gen/pubkeySSL.txt

pri:
	$(MAKE) gen	
	openssl rsa -text -noout -in ./pem/prikey.pem > ./gen/prikeySSL.txt

a2h:
	gcc ./src/ascii2hex.c -o a2h
	./a2h

h2a:
	gcc ./src/hex2ascii.c -o h2a
	./h2a

file:
	gcc ./src/file.c -o file
	./file

pText:
	$(MAKE) gen	
	@read -p "Enter Plaintext:" TEXT;\
		echo $$TEXT > ./gen/pText.txt;

gText:
	cp ./gen/pTextHEX ./gen/gTextHEX

tKey:
	$(MAKE) clean
	$(MAKE) gen
	$(MAKE) pText
	$(MAKE) a2h
	$(MAKE) gText
	$(MAKE) h2a
	more ./gen/gText.txt

tTop:
	$(MAKE) clean
	$(MAKE) gen
	$(MAKE) key
	$(MAKE) pText
	$(MAKE) a2h
	$(MAKE) top 
	$(MAKE) gText
	$(MAKE) h2a
	more ./gen/gText.txt

clean:
	rm -rf obj_dir iverilog gen extKeys a2h h2a *~ *.S *.bin *.vmem `find . -type f -executable`


