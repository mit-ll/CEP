default: core

core:
	verilator --cc --clk clk --default-language 1364-2001 sha256.v -exe tb_core.cpp
	make -j -C obj_dir -f Vsha256.mk Vsha256
	./obj_dir/Vsha256

top:
	verilator --cc --clk clk --default-language 1364-2001 sha256_top.v -exe tb_top.cpp
	make -j -C obj_dir -f Vsha256_top.mk Vsha256_top
	./obj_dir/Vsha256_top

clean:
	rm -rf obj_dir *~
