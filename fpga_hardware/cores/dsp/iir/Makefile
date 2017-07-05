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
	verilator -Wno-fatal --cc --clk clk --default-language 1364-2001 -I. filter_iir.v iir_top.v --top-module iir_top -exe tb_top.cpp
	make -j -C obj_dir -f Viir_top.mk Viir_top
	./obj_dir/Viir_top

core:
	verilator -Wno-fatal --cc --clk clk --default-language 1364-2001 -I. filter_iir.v --top-module IIR_filter -exe tb_core.cpp
	make -j -C obj_dir -f VIIR_filter.mk VIIR_filter
	./obj_dir/VIIR_filter

vcd:
	verilator -Wno-fatal --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 -I. filter_iir.v iir_top.v --top-module iir_top -exe tb_top.cpp
	make -j -C obj_dir -f Viir_top.mk Viir_top
	./obj_dir/Viir_top
	gtkwave ./obj_dir/Viir_top.vcd

vcd_core:
	verilator -Wno-fatal --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 -I. filter_iir.v --top-module IIR_filter -exe tb_core.cpp
	make -j -C obj_dir -f VIIR_filter.mk VIIR_filter
	./obj_dir/VIIR_filter
	gtkwave ./obj_dir/VIIR_filter.vcd

clean:
	rm -rf obj_dir *~
	rm -rf iverilog *~
	rm -f tb_sys
	rm -f tb_sys.S
	rm -f tb_sys.bin
	rm -f tb_sys.vmem

