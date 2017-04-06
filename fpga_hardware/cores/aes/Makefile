default: core

core:
	verilator --cc --clk clk --default-language 1364-2001 -I. aes_128.v table.v round.v --top-module aes_128 -exe tb_core.cpp
	make -j -C obj_dir -f Vaes_128.mk Vaes_128
	./obj_dir/Vaes_128

top:
	verilator --cc --clk clk --default-language 1364-2001 sha3_top.v -exe tb_top.cpp
	make -j -C obj_dir -f Vsha3_top.mk Vsha3_top
	./obj_dir/Vsha3_top

clean:
	rm -rf obj_dir
