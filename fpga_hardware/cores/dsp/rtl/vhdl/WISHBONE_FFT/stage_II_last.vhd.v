// File ./stage_II_last.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
// vhd2vl settings:
//  * Verilog Module Declaration Style: 2001

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

//Component for Stages using BF2II (last stage only) 
//When BF2II is the last butterfly, there is no   multiplier after it 
//Input is a standar d logic vector of data_width -add_g 
//data_width - wid th  of the internal busses 
//add_g - Add growth variable - if 1, data_width grows by 1, if 0 then - 0 
// no timescale needed

module stage_II_last(
input wire [data_width - 1 - add_g:0] prvs_r,
input wire [data_width - 1 - add_g:0] prvs_i,
input wire t,
input wire s,
input wire clock,
input wire enable,
input wire resetn,
output wire [data_width - 1:0] tonext_r,
output wire [data_width - 1:0] tonext_i
);

parameter [31:0] data_width=14;
parameter [31:0] add_g=1;



wire [data_width - 1:0] toreg_r;
wire [data_width - 1:0] toreg_i;
wire [data_width - 1:0] fromreg_r;
wire [data_width - 1:0] fromreg_i;
wire [data_width - 1:0] tonext_r_aux;
wire [data_width - 1:0] tonext_i_aux;

  shiftreg1 #(
      .data_width(data_width))
  regr(
      .clock(clock),
    .enable(enable),
    .clear(1'b 0),
    .read_data(fromreg_r),
    .write_data(toreg_r),
    .resetn(resetn));

  shiftreg1 #(
      .data_width(data_width))
  regi(
      .clock(clock),
    .enable(enable),
    .clear(1'b 0),
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
    .tonext_r(tonext_r_aux),
    .tonext_i(tonext_i_aux));

  shiftreg1 #(
      .data_width(data_width))
  regsegr(
      .clock(clock),
    .enable(1'b 1),
    .clear(1'b 0),
    .read_data(tonext_r),
    .write_data(tonext_r_aux),
    .resetn(resetn));

  shiftreg1 #(
      .data_width(data_width))
  regsegi(
      .clock(clock),
    .enable(1'b 1),
    .clear(1'b 0),
    .read_data(tonext_i),
    .write_data(tonext_i_aux),
    .resetn(resetn));


endmodule
