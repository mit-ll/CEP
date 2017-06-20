// File ./comp_mult.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
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

//Since both the real and imaginary values       have the same number of  fractio nal bits, 
//  there is no need to   truncate. 
// no timescale needed

module comp_mult(
input wire [inst_width1 - 1:0] Re1,
input wire [inst_width1 - 1:0] Im1,
input wire [inst_width2 - 1:0] Re2,
input wire [inst_width2 - 1:0] Im2,
output wire [inst_width1 + inst_width2:0] Re,
output wire [inst_width1 + inst_width2:0] Im
);

parameter [31:0] inst_width1=14;
parameter [31:0] inst_width2=14;



//multiplier outputs 
wire [inst_width1 + inst_width2 - 1:0] product1;  //re1*re2 
wire [inst_width1 + inst_width2 - 1:0] product2;  //i m1*im2 
wire [inst_width1 + inst_width2 - 1:0] product3;
wire [inst_width1 + inst_width2 - 1:0] product4;

  multiplier #(
      .inst_width1(inst_width1),
    .inst_width2(inst_width2))
  U1(
      .inst_A(Re1),
    .inst_B(Re2),
    .PRODUCT_inst(product1));

  multiplier #(
      .inst_width1(inst_width1),
    .inst_width2(inst_width2))
  U2(
      .inst_A(Im1),
    .inst_B(Im2),
    .PRODUCT_inst(product2));

  multiplier #(
      .inst_width1(inst_width1),
    .inst_width2(inst_width2))
  U3(
      .inst_A(Re1),
    .inst_B(Im2),
    .PRODUCT_inst(product3));

  multiplier #(
      .inst_width1(inst_width2),
    .inst_width2(inst_width1))
  U4(
      .inst_A(Re2),
    .inst_B(Im1),
    .PRODUCT_inst(product4));

  subtract #(
      .inst_width(inst_width1 + inst_width2))
  U5(
      .inst_A(product1),
    .inst_B(product2),
    .DIFF(Re));

  adder #(
      .inst_width(inst_width1 + inst_width2))
  U6(
      .inst_A(product3),
    .inst_B(product4),
    .SUM(Im));


endmodule
