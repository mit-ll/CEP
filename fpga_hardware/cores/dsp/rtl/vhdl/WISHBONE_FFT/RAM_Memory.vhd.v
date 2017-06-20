// File RAM_Memory.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
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

module RAM_Memory(
DATi,
DATo,
ADR_WB,
ADR_FFT,
W_R,
clk
);

parameter [31:0] Add_WordWidth=10;
parameter [31:0] Add_WordBits=1024;
parameter [31:0] Data_WordWidth=32;
input [Data_WordWidth - 1:0] DATi;
output [Data_WordWidth - 1:0] DATo;
input [Add_WordWidth - 1:0] ADR_WB;
input [Add_WordWidth - 1:0] ADR_FFT;
input W_R;
input clk;

wire [Data_WordWidth - 1:0] DATi;
reg [Data_WordWidth - 1:0] DATo;
wire [Add_WordWidth - 1:0] ADR_WB;
wire [Add_WordWidth - 1:0] ADR_FFT;
wire W_R;
wire clk;



reg [Data_WordWidth - 1:0] mem[((Add_WordBits)) - 1:0];
wire [Data_WordWidth - 1:0] reg0;

  always @(posedge clk) begin
    if((W_R == 1'b 1)) begin
      mem[ADR_FFT] <= DATi   ;  
    end
    DATo <= mem[ADR_WB];  			
  end


endmodule
