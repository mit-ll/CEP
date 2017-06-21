// File ./twiddle_mult.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
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

//Twiddle multiplier 
//7/17/02  
//Uses a both inputs same width complex multiplier 
//Won't sign extend output, but will truncate it down 
//(mult_width + twiddle_width >= output_width) 
// 
//Twiddle factors are limited to -1 < twdl < 1. 
//(twiddle factor can't be 0b1000000000) 
// no timescale needed

module twiddle_mult(
data_r,
data_i,
twdl_r,
twdl_i,
out_r,
out_i
);

parameter [31:0] mult_width=7;
parameter [31:0] twiddle_width=3;
parameter [31:0] output_width=9;
input [mult_width - 1:0] data_r;
input [mult_width - 1:0] data_i;
input [twiddle_width - 1:0] twdl_r;
input [twiddle_width - 1:0] twdl_i;
output [output_width - 1:0] out_r;
output [output_width - 1:0] out_i;

wire [mult_width - 1:0] data_r;
wire [mult_width - 1:0] data_i;
wire [twiddle_width - 1:0] twdl_r;
wire [twiddle_width - 1:0] twdl_i;
reg [output_width - 1:0] out_r;
reg [output_width - 1:0] out_i;


wire [twiddle_width + mult_width:0] mult_out_r;
wire [twiddle_width + mult_width:0] mult_out_i;

  comp_mult #(
      .inst_width1(mult_width),
    .inst_width2(twiddle_width))
  U1(
      .Re1(data_r),
    .Im1(data_i),
    .Re2(twdl_r),
    .Im2(twdl_i),
    .Re(mult_out_r),
    .Im(mult_out_i));

  always @(mult_out_r or mult_out_i) begin
    out_r <= mult_out_r[(twiddle_width + mult_width - 1):(twiddle_width + mult_width - output_width)];
    out_i <= mult_out_i[(twiddle_width + mult_width - 1):(twiddle_width + mult_width - output_width)];
  end


endmodule
