default: core

core:
	verilator --cc --clk clk --default-language 1364-2001 -I. aes_128.v table.v round.v --top-module aes_128 -exe tb_core.cpp
	make -j -C obj_dir -f Vaes_128.mk Vaes_128
	./obj_dir/Vaes_128

top:
	verilator --cc --clk clk --default-language 1364-2001 aes_top.v aes_128.v table.v round.v --top-module aes_top -exe tb_top.cpp
	make -j -C obj_dir -f Vaes_top.mk Vaes_top
	./obj_dir/Vaes_top

clean:
	rm -rf obj_dir *~
