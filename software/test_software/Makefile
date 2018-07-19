default:
	@echo "The following targets can be built:"
	@echo "hello_world                                       : Build and copy a simple hello world program"
	@echo "clean                                             : Remove all generated files"

hello_world:
	or1k-elf-g++ -Wall -pedantic -O3 ../bootloader/vc707_CEP.S hello_world.cpp -o hello_world.out
	or1k-elf-objdump -D hello_world.out > hello_world.S
	or1k-elf-objcopy -O binary hello_world.out hello_world.bin
	../utils/bin2vmem hello_world.bin > hello_world.vmem
	cp hello_world.vmem ../../hdl_cores/ram_wb/sram.vmem

clean:
	rm -rf obj_dir *~ *.S *.bin *.vmem *.out

