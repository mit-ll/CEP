default: core

core:
	verilator --cc --clk clk --default-language 1364-2001 pancham.v -exe tb_core.cpp
	make -j -C obj_dir -f Vpancham.mk Vpancham
	./obj_dir/Vpancham

top:
	verilator --cc --clk clk --default-language 1364-2001 md5_top.v -exe tb_top.cpp
	make -j -C obj_dir -f Vmd5_top.mk Vmd5_top
	./obj_dir/Vmd5_top

clean:
	rm -rf obj_dir
