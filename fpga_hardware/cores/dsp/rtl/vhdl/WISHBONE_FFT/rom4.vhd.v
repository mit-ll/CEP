// File ./rom4.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
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

// Rom file for twiddle factors 
// ../../../rtl/vhdl/WISHBONE_FFT/rom4.vhd contains 16 points of 16 width 
//  for a 1024 point fft.
// no timescale needed

module rom4(
input wire clk,
input wire [3:0] address,
output reg [data_width - 1:0] datar,
output reg [data_width - 1:0] datai
);

parameter [31:0] data_width=16;
parameter [31:0] address_width=4;




  always @(posedge address or posedge clk) begin
    case(address)
    4'b 0000 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    4'b 0001 : begin
      datar <= 16'b 0101101010000010;
      datai <= 16'b 1010010101111110;
      //128
    end
    4'b 0010 : begin
      datar <= 16'b 0000000000000000;
      datai <= 16'b 1000000000000001;
      //256
    end
    4'b 0011 : begin
      datar <= 16'b 1010010101111110;
      datai <= 16'b 1010010101111110;
      //384
    end
    4'b 0100 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    4'b 0101 : begin
      datar <= 16'b 0111011001000001;
      datai <= 16'b 1100111100000101;
      //64
    end
    4'b 0110 : begin
      datar <= 16'b 0101101010000010;
      datai <= 16'b 1010010101111110;
      //128
    end
    4'b 0111 : begin
      datar <= 16'b 0011000011111011;
      datai <= 16'b 1000100110111111;
      //192
    end
    4'b 1000 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    4'b 1001 : begin
      datar <= 16'b 0011000011111011;
      datai <= 16'b 1000100110111111;
      //192
    end
    4'b 1010 : begin
      datar <= 16'b 1010010101111110;
      datai <= 16'b 1010010101111110;
      //384
    end
    4'b 1011 : begin
      datar <= 16'b 1000100110111111;
      datai <= 16'b 0011000011111011;
      //576
    end
    4'b 1100 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    4'b 1101 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    4'b 1110 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    4'b 1111 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    default : begin
      for (i=data_width - 1; i >= 0; i = i - 1) begin
        datar[i] <= 1'b 0;
        datai[i] <= 1'b 0;
      end
    end
    endcase
  end


endmodule
