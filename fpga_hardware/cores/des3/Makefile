default:
	@echo "sys        : Create a VMEM file to be loaded onto a SoC"
	@echo "top        : Verilator - verify the core with a Wishbone bus interface"
	@echo "core       : Verilator - Verify the core"
	@echo "vcd        : Verilator - Generate VCD for top"
	@echo "vcd_core   : Verilator - Generate VCD for core"
	@echo "iver       : iVerilog  - Runs original core testbench"
	@echo "clean      : Remove all generated files"

sys:
	or1k-elf-g++ -Wall -pedantic -O3 ../../../software/ml509SRAM.S tb_sys.cpp -o tb_sys
	or1k-elf-objdump -D tb_sys > tb_sys.S
	or1k-elf-objcopy -O binary tb_sys tb_sys.bin
	../../../software/utils/bin2vmem tb_sys.bin > tb_sys.vmem

top:
	verilator --cc --clk clk --default-language 1364-2001 -I. ./rtl/common/crp.v ./rtl/common/sbox1.v ./rtl/common/sbox2.v ./rtl/common/sbox3.v ./rtl/common/sbox4.v ./rtl/common/sbox5.v ./rtl/common/sbox6.v  ./rtl/common/sbox7.v ./rtl/common/sbox8.v ./rtl/key_sel3.v ./rtl/des3.v ./rtl/des3_top.v --top-module des3_top -exe tb_top.cpp
	make -j -C obj_dir -f Vdes3_top.mk Vdes3_top
	./obj_dir/Vdes3_top

core:
	verilator --cc --clk clk --default-language 1364-2001 -I. ./rtl/common/crp.v ./rtl/common/sbox1.v ./rtl/common/sbox2.v ./rtl/common/sbox3.v ./rtl/common/sbox4.v ./rtl/common/sbox5.v ./rtl/common/sbox6.v  ./rtl/common/sbox7.v ./rtl/common/sbox8.v ./rtl/key_sel3.v ./rtl/des3.v --top-module des3 -exe tb_core.cpp
	make -j -C obj_dir -f Vdes3.mk Vdes3
	./obj_dir/Vdes3

vcd:
	verilator --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 -I. ./rtl/common/crp.v ./rtl/common/sbox1.v ./rtl/common/sbox2.v ./rtl/common/sbox3.v ./rtl/common/sbox4.v ./rtl/common/sbox5.v ./rtl/common/sbox6.v  ./rtl/common/sbox7.v ./rtl/common/sbox8.v ./rtl/key_sel3.v ./rtl/des3.v ./rtl/des3_top.v --top-module des3_top -exe tb_top.cpp
	make -j -C obj_dir -f Vdes3_top.mk Vdes3_top
	./obj_dir/Vdes3_top
	gtkwave ./obj_dir/Vdes3_top.vcd

vcd_core:
	verilator --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 -I. ./rtl/common/crp.v ./rtl/common/sbox1.v ./rtl/common/sbox2.v ./rtl/common/sbox3.v ./rtl/common/sbox4.v ./rtl/common/sbox5.v ./rtl/common/sbox6.v  ./rtl/common/sbox7.v ./rtl/common/sbox8.v ./rtl/key_sel3.v ./rtl/des3.v --top-module des3 -exe tb_core.cpp
	make -j -C obj_dir -f Vdes3.mk Vdes3
	./obj_dir/Vdes3
	gtkwave ./obj_dir/Vdes3.vcd

iver:
	mkdir -p ./iverilog/
	iverilog -Wall ./tb_core.v ./rtl/common/crp.v ./rtl/common/sbox1.v ./rtl/common/sbox2.v ./rtl/common/sbox3.v ./rtl/common/sbox4.v ./rtl/common/sbox5.v ./rtl/common/sbox6.v  ./rtl/common/sbox7.v ./rtl/common/sbox8.v ./rtl/key_sel3.v ./rtl/des3.v -o ./iverilog/des3_core.sim
	./iverilog/des3_core.sim

clean:
	rm -rf obj_dir *~
	rm -rf iverilog *~
	rm -f tb_sys
	rm -f tb_sys.S
	rm -f tb_sys.bin
	rm -f tb_sys.vmem

