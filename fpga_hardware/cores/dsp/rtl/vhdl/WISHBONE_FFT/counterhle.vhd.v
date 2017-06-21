// File ./counterhle.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
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

//Counter with resetn and load enable 
//When load enable is high, it counts. 
//When load enable is low, it stops counting. 
//When a reset is triggered, it resets to zero. 
// no timescale needed

module counterhle(
clock,
resetn,
enable,
clear,
countout
);

parameter [31:0] width=3;
input clock;
input resetn;
input enable;
input clear;
output [width - 1:0] countout;

wire clock;
wire resetn;
wire enable;
wire clear;
wire [width - 1:0] countout;


reg [width - 1:0] count;

  always @(posedge clock or posedge resetn or posedge enable) begin
    if((resetn == 1'b 0)) begin
      count <= {(((width - 1))-((0))+1){1'b0}};
    end else begin
      if((enable == 1'b 1)) begin
        if((clear == 1'b 1)) begin
          count <= {(((width - 1))-((0))+1){1'b0}};
        end
        else begin
          count <= ((count)) + 1'b 1;
        end
      end
    end
  end

  assign countout = count;

endmodule
