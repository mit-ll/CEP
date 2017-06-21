// File ./stage_II.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
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

//Component for  Stages using BF2II (second and every even stage) 
//Doesn 't handle last stage 
//Input is a stand ard logic vector of data_width-add_g 
//data_width - width of the internal busses 
//  Add growth variable - if 1, data_width gro add_g -   ws by 1,   if 0 then 0 
//mult_g - mult g  rowth variable - can r ange from 0 to twiddle_width+1 
//twiddle_width - width of the twiddle factor input 
//shift_stages - number of shift register stages 
// no timescale needed

module stage_II(
prvs_r,
prvs_i,
t,
s,
clock,
enable,
resetn,
fromrom_r,
fromrom_i,
tonext_r,
tonext_i
);

parameter [31:0] data_width=14;
parameter [31:0] add_g=1;
parameter [31:0] mult_g=9;
parameter [31:0] twiddle_width=10;
parameter [31:0] shift_stages=16;
input [data_width - 1 - add_g:0] prvs_r;
input [data_width - 1 - add_g:0] prvs_i;
input t;
input s;
input clock;
input enable;
input resetn;
input [twiddle_width - 1:0] fromrom_r;
input [twiddle_width - 1:0] fromrom_i;
output [data_width + mult_g - 1:0] tonext_r;
output [data_width + mult_g - 1:0] tonext_i;

wire [data_width - 1 - add_g:0] prvs_r;
wire [data_width - 1 - add_g:0] prvs_i;
wire t;
wire s;
wire clock;
wire enable;
wire resetn;
wire [twiddle_width - 1:0] fromrom_r;
wire [twiddle_width - 1:0] fromrom_i;
wire [data_width + mult_g - 1:0] tonext_r;
wire [data_width + mult_g - 1:0] tonext_i;


wire [data_width - 1:0] toreg_r;
wire [data_width - 1:0] toreg_i;
wire [data_width - 1:0] fromreg_r;
wire [data_width - 1:0] fromreg_i;
wire [data_width - 1:0] tomult_r;
wire [data_width - 1:0] tomult_i;
wire [data_width + mult_g - 1:0] tonext_r_aux;
wire [data_width + mult_g - 1:0] tonext_i_aux;

  shiftregN #(
      .data_width(data_width),
    .n(shift_stages))
  regr(
      .clock(clock),
    .enable(enable),
    .read_data(fromreg_r),
    .write_data(toreg_r),
    .resetn(resetn));

  shiftregN #(
      .data_width(data_width),
    .n(shift_stages))
  regi(
      .clock(clock),
    .enable(enable),
    .read_data(fromreg_i),
    .write_data(toreg_i),
    .resetn(resetn));

  BF2II #(
      .data_width(data_width),
    .add_g(add_g))
  btrfly(
      .fromreg_r(fromreg_r),
    .fromreg_i(fromreg_i),
    .prvs_r(prvs_r),
    .prvs_i(prvs_i),
    .t(t),
    .s(s),
    .toreg_r(toreg_r),
    .toreg_i(toreg_i),
    .tonext_r(tomult_r),
    .tonext_i(tomult_i));

  twiddle_mult #(
      .mult_width(data_width),
    .twiddle_width(twiddle_width),
    .output_width(data_width + mult_g))
  twiddle(
      .data_r(tomult_r),
    .data_i(tomult_i),
    .twdl_r(fromrom_r),
    .twdl_i(fromrom_i),
    .out_r(tonext_r_aux),
    .out_i(tonext_i_aux));

  shiftreg1 #(
      .data_width(data_width + mult_g))
  regsegr(
      .clock(clock),
    .enable(1'b 1),
    .clear(1'b 0),
    .read_data(tonext_r),
    .write_data(tonext_r_aux),
    .resetn(resetn));

  shiftreg1 #(
      .data_width(data_width + mult_g))
  regsegi(
      .clock(clock),
    .enable(1'b 1),
    .clear(1'b 0),
    .read_data(tonext_i),
    .write_data(tonext_i_aux),
    .resetn(resetn));


endmodule
