// File ./mux2_mmw.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
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

//Special 2 to 1 mux (mismatched width) 
//7/17/02 
// First input is data_width bits 
//Second input is data_width+1     bits 
//Ign ores highest bit of second  input 
// no timescale needed

module mux2_mmw(
input wire s,
input wire [data_width - 1:0] in0,
input wire [data_width:0] in1,
output reg [data_width - 1:0] data
);

parameter [31:0] data_width=35;



// hds interface_end 

  always @(in0 or in1 or s) begin
    if(s == 1'b 0) begin
      data <= in0;
    end
    else begin
      data <= in1[data_width - 1:0];
    end
  end


endmodule
