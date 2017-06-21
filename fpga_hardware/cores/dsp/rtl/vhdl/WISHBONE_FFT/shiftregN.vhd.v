// File ./shiftregN.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
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

// hds  header_start 
// age shift registe -n st                    r, data_width bits wide. 
// no timescale needed

module shiftregN(
clock,
enable,
read_data,
write_data,
resetn
);

parameter [31:0] data_width=25;
parameter [31:0] n=254;
input clock;
input enable;
output [data_width - 1:0] read_data;
input [data_width - 1:0] write_data;
input resetn;

wire clock;
wire enable;
wire [data_width - 1:0] read_data;
wire [data_width - 1:0] write_data;
wire resetn;

// Declarations 

// hds interface_end 

wire [data_width - 1:0] registerFile[0:n];

  assign registerFile[0] = write_data;
  assign read_data = registerFile[n];
  genvar i;
  generate for (i=0; i <= n - 1; i = i + 1) begin: registers
      shiftreg1 #(
          .data_width(data_width))
    regi(
          .clock(clock),
      .enable(enable),
      .clear(1'b 0),
      .read_data(registerFile[i + 1]),
      .write_data(registerFile[i]),
      .resetn(resetn));

  end
  endgenerate

endmodule
