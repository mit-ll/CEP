default:
	@echo "sys        : Create a VMEM file to be loaded onto a SoC"
	@echo "ver        : Verilator - verify the core plus a Wishbone bus interface"
	@echo "             INCOMPLETE: verilator bug"
	@echo "ver_modexp : Verilator - Verify the core with original bus interface"
	@echo "             INCOMPLETE: verilator bug"
	@echo "vcd        : Verilator - Generate VCD for ver"
	@echo "vcd_modexp : Verilator - Generate VCD for ver_modexp"
	@echo "ides3p     : iVerilog  - Verify the core plus a Wishbone bus interface"
	@echo "idesp      : iVerilog  - Verify the core plus a Wishbone bus interface"
	@echo "ides3      : iVerilog  - Verify the core plus a Wishbone bus interface"
	@echo "ides       : iVerilog  - Verify the core plus a Wishbone bus interface"
	@echo ""
	@echo "clean      : Remove all generated files"

sys:
	or1k-elf-g++ -Wall -pedantic -O3 ../../../software/ml509SRAM.S tb_sys.cpp -o tb_sys
	or1k-elf-objdump -D tb_sys > tb_sys.S
	or1k-elf-objcopy -O binary tb_sys tb_sys.bin
	../../../software/utils/bin2vmem tb_sys.bin > tb_sys.vmem

top:

core:
	verilator -Wno-fatal --cc --clk clk --default-language 1364-2001 -I. ./rtl/common/crp.v ./rtl/common/sbox1.v ./rtl/common/sbox2.v ./rtl/common/sbox3.v ./rtl/common/sbox4.v ./rtl/common/sbox5.v ./rtl/common/sbox6.v  ./rtl/common/sbox7.v ./rtl/common/sbox8.v ./rtl/area_opt/key_sel3.v ./rtl/area_opt/des3.v --top-module des3 -exe tb_core.cpp
	make -j -C obj_dir -f Vdes3.mk Vdes3
	./obj_dir/Vdes3

vcd:
	verilator -Wno-fatal --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 -I. ./rtl/common/crp.v ./rtl/common/sbox1.v ./rtl/common/sbox2.v ./rtl/common/sbox3.v ./rtl/common/sbox4.v ./rtl/common/sbox5.v ./rtl/common/sbox6.v  ./rtl/common/sbox7.v ./rtl/common/sbox8.v ./rtl/area_opt/key_sel3.v ./rtl/area_opt/des3.v --top-module des3 -exe tb_core.cpp
	make -j -C obj_dir -f Vdes3.mk Vdes3
	./obj_dir/Vdes3
	gtkwave ./obj_dir/Vdes3.vcd

ides3p:
	mkdir -p ./iverilog/
	iverilog -Wall ./tb/des3_test_po.v ./rtl/common/crp.v ./rtl/common/sbox1.v ./rtl/common/sbox2.v ./rtl/common/sbox3.v ./rtl/common/sbox4.v ./rtl/common/sbox5.v ./rtl/common/sbox6.v  ./rtl/common/sbox7.v ./rtl/common/sbox8.v ./rtl/perf_opt/key_sel.v ./rtl/perf_opt/des.v ./rtl/perf_opt/des3.v -o ./iverilog/des3_test_po.sim
	./iverilog/des3_test_po.sim

idesp:
	mkdir -p ./iverilog/
	iverilog -Wall ./tb/des_test_po.v ./rtl/common/crp.v ./rtl/common/sbox1.v ./rtl/common/sbox2.v ./rtl/common/sbox3.v ./rtl/common/sbox4.v ./rtl/common/sbox5.v ./rtl/common/sbox6.v  ./rtl/common/sbox7.v ./rtl/common/sbox8.v ./rtl/perf_opt/key_sel.v ./rtl/perf_opt/des.v -o ./iverilog/des_test_po.sim
	./iverilog/des_test_po.sim

ides3:
	mkdir -p ./iverilog/
	iverilog -Wall ./tb/des3_test_ao.v ./rtl/common/crp.v ./rtl/common/sbox1.v ./rtl/common/sbox2.v ./rtl/common/sbox3.v ./rtl/common/sbox4.v ./rtl/common/sbox5.v ./rtl/common/sbox6.v  ./rtl/common/sbox7.v ./rtl/common/sbox8.v ./rtl/area_opt/key_sel3.v ./rtl/area_opt/des3.v -o ./iverilog/des3_test_ao.sim
	./iverilog/des3_test_ao.sim

ides:
	mkdir -p ./iverilog/
	iverilog -Wall ./tb/des_test_ao.v ./rtl/common/crp.v ./rtl/common/sbox1.v ./rtl/common/sbox2.v ./rtl/common/sbox3.v ./rtl/common/sbox4.v ./rtl/common/sbox5.v ./rtl/common/sbox6.v  ./rtl/common/sbox7.v ./rtl/common/sbox8.v ./rtl/area_opt/key_sel.v ./rtl/area_opt/des.v -o ./iverilog/des_test_ao.sim
	./iverilog/des_test_ao.sim

clean:
	rm -rf obj_dir *~
	rm -rf iverilog *~
	rm -f tb_sys
	rm -f tb_sys.S
	rm -f tb_sys.bin
	rm -f tb_sys.vmem

