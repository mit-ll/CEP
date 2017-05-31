default: top

top:
	verilator --cc --clk clk --default-language 1364-2001 -I. modexp.v modexp_core.v montprod.v residue.v adder.v shl.v shr.v blockmem1r1w.v blockmem2r1w.v blockmem2r1wptr.v  blockmem2rptr1w.v --top-module modexp -exe tb_top.cpp
	make -j -C obj_dir -f Vmodexp.mk Vmodexp
	./obj_dir/Vmodexp
core:

sys:

vcd:
	verilator --trace --trace-depth 1 --cc --clk clk --default-language 1364-2001 -I. modexp.v modexp_core.v montprod.v residue.v adder.v shl.v shr.v blockmem1r1w.v blockmem2r1w.v blockmem2r1wptr.v  blockmem2rptr1w.v --top-module modexp -exe tb_top.cpp
	make -j -C obj_dir -f Vmodexp.mk Vmodexp
	./obj_dir/Vmodexp

modexp.sim:
	iverilog -Wall -o modexp.sim tb_modexp.v modexp.v modexp_core.v montprod.v residue.v adder.v shl.v shr.v blockmem1r1w.v blockmem2r1w.v blockmem2r1wptr.v blockmem2rptr1w.v

clean:
	rm -rf obj_dir *~
