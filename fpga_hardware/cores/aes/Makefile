default:
	@echo "core: Verify just the core"
	@echo "top:  Verify the core plus a Wishbone bus interface"
OB	@echo "sys:  Create a VMEM file to be loaded onto a SoC for full-system verification"
	@echo "clean: Remove all generated files"

core:
	verilator --cc --clk clk --default-language 1364-2001 -I. aes_192.v table.v round.v --top-module aes_192 -exe tb_core.cpp
	make -j -C obj_dir -f Vaes_192.mk Vaes_192
	./obj_dir/Vaes_192

top:
	verilator --cc --clk clk --default-language 1364-2001 aes_top.v aes_192.v table.v round.v --top-module aes_top -exe tb_top.cpp
	make -j -C obj_dir -f Vaes_top.mk Vaes_top
	./obj_dir/Vaes_top

vcd:
	verilator --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 aes_top.v aes_192.v table.v round.v --top-module aes_top -exe tb_top.cpp
	make -j -C obj_dir -f Vaes_top.mk Vaes_top
	./obj_dir/Vaes_top
	gtkwave ./obj_dir/Vaes_top.vcd

sys:
	or1k-elf-g++ -Wall -pedantic -O3 ../../../software/ml509SRAM.S tb_sys.cpp -o tb_sys
	or1k-elf-objdump -D tb_sys > tb_sys.S
	or1k-elf-objcopy -O binary tb_sys tb_sys.bin
	../../../software/utils/bin2vmem tb_sys.bin > tb_sys.vmem

clean:
	rm -rf obj_dir *~ tb_sys.S tb_sys.bin tb_sys.vmem tb_sys
