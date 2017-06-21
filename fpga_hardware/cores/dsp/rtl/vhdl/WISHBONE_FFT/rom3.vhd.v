// File ./rom3.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
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

// Rom file for twiddle factors 
// ../../../rtl/vhdl/WISHBONE_FFT/rom3.vhd contains 64 points of 16 width 
//  for a 1024 point fft.
// no timescale needed

module rom3(
clk,
address,
datar,
datai
);

parameter [31:0] data_width=16;
parameter [31:0] address_width=6;
input clk;
input [5:0] address;
output [data_width - 1:0] datar;
output [data_width - 1:0] datai;

wire clk;
wire [5:0] address;
reg [data_width - 1:0] datar;
reg [data_width - 1:0] datai;



  always @(posedge address or posedge clk) begin
    case(address)
    6'b 000000 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 000001 : begin
      datar <= 16'b 0111110110001001;
      datai <= 16'b 1110011100000111;
      //32
    end
    6'b 000010 : begin
      datar <= 16'b 0111011001000001;
      datai <= 16'b 1100111100000101;
      //64
    end
    6'b 000011 : begin
      datar <= 16'b 0110101001101101;
      datai <= 16'b 1011100011100100;
      //96
    end
    6'b 000100 : begin
      datar <= 16'b 0101101010000010;
      datai <= 16'b 1010010101111110;
      //128
    end
    6'b 000101 : begin
      datar <= 16'b 0100011100011100;
      datai <= 16'b 1001010110010011;
      //160
    end
    6'b 000110 : begin
      datar <= 16'b 0011000011111011;
      datai <= 16'b 1000100110111111;
      //192
    end
    6'b 000111 : begin
      datar <= 16'b 0001100011111001;
      datai <= 16'b 1000001001110111;
      //224
    end
    6'b 001000 : begin
      datar <= 16'b 0000000000000000;
      datai <= 16'b 1000000000000001;
      //256
    end
    6'b 001001 : begin
      datar <= 16'b 1110011100000111;
      datai <= 16'b 1000001001110111;
      //288
    end
    6'b 001010 : begin
      datar <= 16'b 1100111100000101;
      datai <= 16'b 1000100110111111;
      //320
    end
    6'b 001011 : begin
      datar <= 16'b 1011100011100100;
      datai <= 16'b 1001010110010011;
      //352
    end
    6'b 001100 : begin
      datar <= 16'b 1010010101111110;
      datai <= 16'b 1010010101111110;
      //384
    end
    6'b 001101 : begin
      datar <= 16'b 1001010110010011;
      datai <= 16'b 1011100011100100;
      //416
    end
    6'b 001110 : begin
      datar <= 16'b 1000100110111111;
      datai <= 16'b 1100111100000101;
      //448
    end
    6'b 001111 : begin
      datar <= 16'b 1000001001110111;
      datai <= 16'b 1110011100000111;
      //480
    end
    6'b 010000 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 010001 : begin
      datar <= 16'b 0111111101100001;
      datai <= 16'b 1111001101110100;
      //16
    end
    6'b 010010 : begin
      datar <= 16'b 0111110110001001;
      datai <= 16'b 1110011100000111;
      //32
    end
    6'b 010011 : begin
      datar <= 16'b 0111101001111100;
      datai <= 16'b 1101101011011000;
      //48
    end
    6'b 010100 : begin
      datar <= 16'b 0111011001000001;
      datai <= 16'b 1100111100000101;
      //64
    end
    6'b 010101 : begin
      datar <= 16'b 0111000011100010;
      datai <= 16'b 1100001110101010;
      //80
    end
    6'b 010110 : begin
      datar <= 16'b 0110101001101101;
      datai <= 16'b 1011100011100100;
      //96
    end
    6'b 010111 : begin
      datar <= 16'b 0110001011110001;
      datai <= 16'b 1010111011001101;
      //112
    end
    6'b 011000 : begin
      datar <= 16'b 0101101010000010;
      datai <= 16'b 1010010101111110;
      //128
    end
    6'b 011001 : begin
      datar <= 16'b 0101000100110011;
      datai <= 16'b 1001110100001111;
      //144
    end
    6'b 011010 : begin
      datar <= 16'b 0100011100011100;
      datai <= 16'b 1001010110010011;
      //160
    end
    6'b 011011 : begin
      datar <= 16'b 0011110001010110;
      datai <= 16'b 1000111100011110;
      //176
    end
    6'b 011100 : begin
      datar <= 16'b 0011000011111011;
      datai <= 16'b 1000100110111111;
      //192
    end
    6'b 011101 : begin
      datar <= 16'b 0010010100101000;
      datai <= 16'b 1000010110000100;
      //208
    end
    6'b 011110 : begin
      datar <= 16'b 0001100011111001;
      datai <= 16'b 1000001001110111;
      //224
    end
    6'b 011111 : begin
      datar <= 16'b 0000110010001100;
      datai <= 16'b 1000000010011111;
      //240
    end
    6'b 100000 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 100001 : begin
      datar <= 16'b 0111101001111100;
      datai <= 16'b 1101101011011000;
      //48
    end
    6'b 100010 : begin
      datar <= 16'b 0110101001101101;
      datai <= 16'b 1011100011100100;
      //96
    end
    6'b 100011 : begin
      datar <= 16'b 0101000100110011;
      datai <= 16'b 1001110100001111;
      //144
    end
    6'b 100100 : begin
      datar <= 16'b 0011000011111011;
      datai <= 16'b 1000100110111111;
      //192
    end
    6'b 100101 : begin
      datar <= 16'b 0000110010001100;
      datai <= 16'b 1000000010011111;
      //240
    end
    6'b 100110 : begin
      datar <= 16'b 1110011100000111;
      datai <= 16'b 1000001001110111;
      //288
    end
    6'b 100111 : begin
      datar <= 16'b 1100001110101010;
      datai <= 16'b 1000111100011110;
      //336
    end
    6'b 101000 : begin
      datar <= 16'b 1010010101111110;
      datai <= 16'b 1010010101111110;
      //384
    end
    6'b 101001 : begin
      datar <= 16'b 1000111100011110;
      datai <= 16'b 1100001110101010;
      //432
    end
    6'b 101010 : begin
      datar <= 16'b 1000001001110111;
      datai <= 16'b 1110011100000111;
      //480
    end
    6'b 101011 : begin
      datar <= 16'b 1000000010011111;
      datai <= 16'b 0000110010001100;
      //528
    end
    6'b 101100 : begin
      datar <= 16'b 1000100110111111;
      datai <= 16'b 0011000011111011;
      //576
    end
    6'b 101101 : begin
      datar <= 16'b 1001110100001111;
      datai <= 16'b 0101000100110011;
      //624
    end
    6'b 101110 : begin
      datar <= 16'b 1011100011100100;
      datai <= 16'b 0110101001101101;
      //672
    end
    6'b 101111 : begin
      datar <= 16'b 1101101011011000;
      datai <= 16'b 0111101001111100;
      //720
    end
    6'b 110000 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 110001 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 110010 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 110011 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 110100 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 110101 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 110110 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 110111 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 111000 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 111001 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 111010 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 111011 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 111100 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 111101 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 111110 : begin
      datar <= 16'b 0111111111111111;
      datai <= 16'b 0000000000000000;
      //0
    end
    6'b 111111 : begin
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
