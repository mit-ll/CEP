default:
	@echo "sys      : Create a VMEM file to be loaded onto a SoC"
	@echo "top      : Verilator - verify the core with a Wishbone bus interface"
	@echo "core     : Verilator - Verify the core"
	@echo "vcd      : Verilator - Generate VCD for top"
	@echo "vcd_core : Verilator - Generate VCD for core"
	@echo "clean    : Remove all generated files"

sys:
	or1k-elf-g++ -Wall -Wno-long-long -pedantic -O3 ../../../../software/ml509SRAM.S tb_sys.cpp -o tb_sys
	or1k-elf-objdump -D tb_sys > tb_sys.S
	or1k-elf-objcopy -O binary tb_sys tb_sys.bin
	../../../../software/utils/bin2vmem tb_sys.bin > tb_sys.vmem

top:
	verilator -Wno-fatal --cc --clk clk --default-language 1364-2001 -I. filter_fir.v fir_top.v --top-module fir_top -exe tb_top.cpp
	make -j -C obj_dir -f Vfir_top.mk Vfir_top
	./obj_dir/Vfir_top

core:
	verilator -Wno-fatal --cc --clk clk --default-language 1364-2001 -I. filter_fir.v --top-module FIR_filter -exe tb_core.cpp
	make -j -C obj_dir -f VFIR_filter.mk VFIR_filter
	./obj_dir/VFIR_filter

vcd:
	verilator -Wno-fatal --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 -I. filter_fir.v fir_top.v --top-module fir_top -exe tb_top.cpp
	make -j -C obj_dir -f Vfir_top.mk Vfir_top
	./obj_dir/Vfir_top
	gtkwave ./obj_dir/Vfir_top.vcd

vcd_core:
	verilator -Wno-fatal --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 -I. filter_fir.v --top-module FIR_filter -exe tb_core.cpp
	make -j -C obj_dir -f VFIR_filter.mk VFIR_filter
	./obj_dir/VFIR_filter
	gtkwave ./obj_dir/VFIR_filter.vcd

clean:
	rm -rf obj_dir *~
	rm -rf iverilog *~
	rm -f tb_sys
	rm -f tb_sys.S
	rm -f tb_sys.bin
	rm -f tb_sys.vmem

