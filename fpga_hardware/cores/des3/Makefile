default:
	@echo "sys      : Create a VMEM file to be loaded onto a SoC"
	@echo "top      : Verilator - verify the core with a Wishbone bus interface"
	@echo "core     : Verilator - Verify the core"
	@echo "vcd      : Verilator - Generate VCD for top"
	@echo "vcd_core : Verilator - Generate VCD for core"
	@echo "clean    : Remove all generated files"

sys:
	or1k-elf-g++ -Wall -Wno-long-long -pedantic -O3 ../../../software/ml509SRAM.S tb_sys.cpp -o tb_sys
	or1k-elf-objdump -D tb_sys > tb_sys.S
	or1k-elf-objcopy -O binary tb_sys tb_sys.bin
	../../../software/utils/bin2vmem tb_sys.bin > tb_sys.vmem

top:
	verilator --cc --clk clk --default-language 1364-2001 -I. crp.v sbox1.v sbox2.v sbox3.v sbox4.v sbox5.v sbox6.v  sbox7.v sbox8.v key_sel3.v des3.v des3_top.v --top-module des3_top -exe tb_top.cpp
	make -j -C obj_dir -f Vdes3_top.mk Vdes3_top
	./obj_dir/Vdes3_top

core:
	verilator --cc --clk clk --default-language 1364-2001 -I. crp.v sbox1.v sbox2.v sbox3.v sbox4.v sbox5.v sbox6.v  sbox7.v sbox8.v key_sel3.v des3.v --top-module des3 -exe tb_core.cpp
	make -j -C obj_dir -f Vdes3.mk Vdes3
	./obj_dir/Vdes3

vcd:
	verilator --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 -I. crp.v sbox1.v sbox2.v sbox3.v sbox4.v sbox5.v sbox6.v  sbox7.v sbox8.v key_sel3.v des3.v des3_top.v --top-module des3_top -exe tb_top.cpp
	make -j -C obj_dir -f Vdes3_top.mk Vdes3_top
	./obj_dir/Vdes3_top
	gtkwave ./obj_dir/Vdes3_top.vcd

vcd_core:
	verilator --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 -I. crp.v sbox1.v sbox2.v sbox3.v sbox4.v sbox5.v sbox6.v  sbox7.v sbox8.v key_sel3.v des3.v --top-module des3 -exe tb_core.cpp
	make -j -C obj_dir -f Vdes3.mk Vdes3
	./obj_dir/Vdes3
	gtkwave ./obj_dir/Vdes3.vcd

clean:
	rm -rf obj_dir *~
	rm -f tb_sys
	rm -f tb_sys.S
	rm -f tb_sys.bin
	rm -f tb_sys.vmem

