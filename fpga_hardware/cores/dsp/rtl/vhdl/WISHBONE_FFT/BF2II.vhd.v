// File BF2II.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
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

//Butterfly stage type 2 
//7/17/0 2  
// no timescale needed

module BF2II(
fromreg_r,
fromreg_i,
prvs_r,
prvs_i,
t,
s,
toreg_r,
toreg_i,
tonext_r,
tonext_i
);

parameter [31:0] data_width=13;
parameter [31:0] add_g=1;
input [data_width - 1:0] fromreg_r;
input [data_width - 1:0] fromreg_i;
input [data_width - 1 - add_g:0] prvs_r;
input [data_width - 1 - add_g:0] prvs_i;
input t;
input s;
output [data_width - 1:0] toreg_r;
output [data_width - 1:0] toreg_i;
output [data_width - 1:0] tonext_r;
output [data_width - 1:0] tonext_i;

wire [data_width - 1:0] fromreg_r;
wire [data_width - 1:0] fromreg_i;
wire [data_width - 1 - add_g:0] prvs_r;
wire [data_width - 1 - add_g:0] prvs_i;
wire t;
wire s;
wire [data_width - 1:0] toreg_r;
wire [data_width - 1:0] toreg_i;
wire [data_width - 1:0] tonext_r;
wire [data_width - 1:0] tonext_i;


reg [data_width - 1:0] prvs_ext_r;
reg [data_width - 1:0] prvs_ext_i;
wire [data_width:0] add1_out;
wire [data_width:0] add2_out;
wire [data_width:0] sub1_out;
wire [data_width:0] sub2_out;
reg [data_width:0] swapadd;
reg [data_width:0] swapsub;

  adder #(
      .inst_width(data_width))
  add1(
      .inst_A(prvs_ext_r),
    .inst_B(fromreg_r),
    .SUM(add1_out));

  adder #(
      .inst_width(data_width))
  add2(
      .inst_A(prvs_ext_i),
    .inst_B(fromreg_i),
    .SUM(add2_out));

  subtract #(
      .inst_width(data_width))
  sub1(
      .inst_A(fromreg_r),
    .inst_B(prvs_ext_r),
    .DIFF(sub1_out));

  subtract #(
      .inst_width(data_width))
  sub2(
      .inst_A(fromreg_i),
    .inst_B(prvs_ext_i),
    .DIFF(sub2_out));

  mux2_mmw #(
      .data_width(data_width))
  mux_1(
      .s(s),
    .in0(fromreg_r),
    .in1(add1_out),
    .data(tonext_r));

  mux2_mmw #(
      .data_width(data_width))
  mux_2(
      .s(s),
    .in0(fromreg_i),
    .in1(swapadd),
    .data(tonext_i));

  mux2_mmw #(
      .data_width(data_width))
  mux_3(
      .s(s),
    .in0(prvs_ext_r),
    .in1(sub1_out),
    .data(toreg_r));

  mux2_mmw #(
      .data_width(data_width))
  mux_4(
      .s(s),
    .in0(prvs_ext_i),
    .in1(swapsub),
    .data(toreg_i));

  always @(prvs_r or prvs_i or s or t) begin
    if((add_g == 1'b 1)) begin
      if((t == 1'b 0 && s == 1'b 1)) begin
        prvs_ext_r <= {prvs_i[data_width - 2],prvs_i};
        prvs_ext_i <= {prvs_r[data_width - 2],prvs_r};
      end
      else begin
        prvs_ext_r <= {prvs_r[data_width - 2],prvs_r};
        prvs_ext_i <= {prvs_i[data_width - 2],prvs_i};
      end
    end
    else begin
      if(t == 1'b0 && s == 1'b1) begin 
        prvs_ext_r <= prvs_i;
        prvs_ext_i <= prvs_r;
      end
      else begin
        prvs_ext_r <= prvs_r;
        prvs_ext_i <= prvs_i;
      end
    end
  end

  always @(add2_out or sub2_out or s or t) begin
    if((t == 1'b 0 && s == 1'b 1)) begin
      swapadd <= sub2_out;
      swapsub <= add2_out;
    end
    else begin
      swapadd <= add2_out;
      swapsub <= sub2_out;
    end
  end


endmodule
