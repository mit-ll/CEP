default:
	@echo "core: Verify just the core"
	@echo "top:  Verify the core plus a Wishbone bus interface"
	@echo "sys:  Create a VMEM file to be loaded onto a SoC for full-system verification"
	@echo "clean: Remove all generated files"

core:
	verilator --cc --clk clk --default-language 1364-2001 pancham.v -exe tb_core.cpp
	make -j -C obj_dir -f Vpancham.mk Vpancham
	./obj_dir/Vpancham

top:
	verilator --cc --clk clk --default-language 1364-2001 md5_top.v -exe tb_top.cpp
	make -j -C obj_dir -f Vmd5_top.mk Vmd5_top
	./obj_dir/Vmd5_top

sys:
	or1k-elf-g++ -Wall -pedantic -O3 ../../../software/ml509SRAM.S tb_sys.cpp -o tb_sys
	or1k-elf-objdump -D tb_sys > tb_sys.S
	or1k-elf-objcopy -O binary tb_sys tb_sys.bin
	../../../software/utils/bin2vmem tb_sys.bin > tb_sys.vmem

vcd:
	verilator --trace --trace-depth 5 --cc --clk clk --default-language 1364-2001 md5_top.v -exe tb_top.cpp
	make -j -C obj_dir -f Vmd5_top.mk Vmd5_top
	./obj_dir/Vmd5_top
	gtkwave ./obj_dir/Vmd5_top.vcd

clean:
	rm -rf obj_dir *~ tb_sys.S tb_sys.bin tb_sys.vmem tb_sys
