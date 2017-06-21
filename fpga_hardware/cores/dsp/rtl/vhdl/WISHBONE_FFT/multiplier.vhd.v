// File ./multiplier.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
// vhd2vl settings:
//  * Verilog Module Declaration Style: 1995

// vhd2vl is Free (libre) Software:
//   Copyright (C) 2001 Vincenzo Liguori - Ocean Logic Pty Ltd
//     http://www.ocean-logic.com
//   Modifications Copyright (C) 2006 Mark Gonzales - PMC Sierra Inc
//   Modifications (C) 2010 Shankar Giri
//   Modifications Copyright (C) 2002, 2005, 2008-2010 Larry Doolittle - LBNL
//     http://doolittle.icarus.com/~larry/vhd2vl/
//
//   vhd2vl comes with ABSOLUTELY NO WARRANTY.  Always check the resulting
//   Verilog for correctness, ideally with a formal verification tool.
//
//   You are welcome to redistribute vhd2vl under certain conditions.
//   See the license (GPLv2) file included with the source for details.

// The result of translation follows.  Its copyright status should be
// considered unchanged from the original VHDL.

// no timescale needed

module multiplier(
inst_A,
inst_B,
PRODUCT_inst
);

parameter [31:0] inst_width1=16;
parameter [31:0] inst_width2=16;
input [inst_width1 - 1:0] inst_A;
input [inst_width2 - 1:0] inst_B;
output [inst_width1 + inst_width2 - 1:0] PRODUCT_inst;

wire [inst_width1 - 1:0] inst_A;
wire [inst_width2 - 1:0] inst_B;
wire [inst_width1 + inst_width2 - 1:0] PRODUCT_inst;


wire [inst_width1 + inst_width2 - 1:0] mult_sig;

  assign mult_sig = ((inst_A)) * ((inst_B));
  assign PRODUCT_inst = (mult_sig);

endmodule
