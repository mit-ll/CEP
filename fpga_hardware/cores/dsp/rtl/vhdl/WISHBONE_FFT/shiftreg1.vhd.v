// File ./shiftreg1.vhd translated with vhd2vl v2.4 VHDL to Verilog RTL translator
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

// hds header_start 
//1 stage shift register, data_width bits wide. 
// no timescale needed

module shiftreg1(
clock,
enable,
clear,
read_data,
write_data,
resetn
);

parameter [31:0] data_width=25;
input clock;
input enable;
input clear;
output [data_width - 1:0] read_data;
input [data_width - 1:0] write_data;
input resetn;

wire clock;
wire enable;
wire clear;
reg [data_width - 1:0] read_data;
wire [data_width - 1:0] write_data;
wire resetn;

// Declarations 

// hds interface_end 
//signal reg00 : std_logic_vector(data_width-1 downto 0); 

  always @(posedge clock or posedge resetn) begin
    if((resetn == 1'b 0)) begin
      //      for                i in data_width-1 downto 0 loop 
      //    r eg00(i)<='0'; 
      read_data <= {(((data_width - 1))-((0))+1){1'b0}};
      //     end loop;  
    end else begin
      if((enable == 1'b 1)) begin
        if((clear == 1'b 1)) begin
          //    reg00<=write_data; 
          //   read _data<=reg00; 
          read_data <= {(((data_width - 1))-((0))+1){1'b0}};
        end
        else begin
          read_data <= write_data;
        end
      end
    end
  end


endmodule
