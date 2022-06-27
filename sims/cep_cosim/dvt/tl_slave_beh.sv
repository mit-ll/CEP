//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      tl_slave_beh.v
// Program:        Common Evaluation Platform (CEP)
// Description:    Tilelink Slave Bus Functional Model for the
//                 CEP Co-Simulation Environment
// Notes:          
//
//--------------------------------------------------------------------------------------

`include "suite_config.v"
`include "tl_defines.incl"
`include "v2c_top.incl"

module tl_slave_beh 
  #(parameter
    CHIP_ID=0, // for sim ONLY
    SRC_SIZE=2,
    SINK_SIZE=3,
    BUS_SIZE=8, 
    ADR_WIDTH=32,
    //
    localparam TL_SIZE=$clog2(BUS_SIZE),
    localparam DATA_WIDTH=(BUS_SIZE*8)
    )
   ( // Master mode
     input 			 clock, 
     input 			 reset, 
     // Channel A
     output reg 		 tl_a_ready=1, 
     input 			 tl_a_valid, 
     input [2:0] 		 tl_a_bits_opcode, 
     input [2:0] 		 tl_a_bits_param, 
     input [TL_SIZE-1:0] 	 tl_a_bits_size, 
     input [SRC_SIZE-1:0] 	 tl_a_bits_source, 
     input [ADR_WIDTH-1:0] 	 tl_a_bits_address, 
     input [BUS_SIZE-1:0] 	 tl_a_bits_mask, 
     input [DATA_WIDTH-1:0] 	 tl_a_bits_data, 
     input 			 tl_a_bits_corrupt, 
    // Channel D
     input 			 tl_d_ready, // default 
     output reg 		 tl_d_valid=0, 
     output reg [2:0] 		 tl_d_bits_opcode=0, 
     output reg [1:0] 		 tl_d_bits_param=0, 
     output reg [TL_SIZE-1:0] 	 tl_d_bits_size=0, 
     output reg [SRC_SIZE-1:0] 	 tl_d_bits_source=0, 
     output reg [SINK_SIZE-1:0]  tl_d_bits_sink=0, 
     output reg 		 tl_d_bits_denied=0, 
     output reg [DATA_WIDTH-1:0] tl_d_bits_data=0, 
     output reg 		 tl_d_bits_corrupt=0,
     // system side
     output reg 		 req,
     output reg [ADR_WIDTH-1:0]  adr,
     output reg 		 rdWr=0,
     output reg [63:0] 		 wrDat,
     input [63:0] 		 rdDat,
     input 			 ack
     );

   //
   // Very simple TL-UL Only A/D
   //
   always @(posedge tl_a_valid) begin
      if (tl_a_bits_opcode == `TL_A_PUTFULLDATA) begin // write
	 @(posedge clock);#1;
	 req   = 1;
	 adr   = tl_a_bits_address;	 	 
	 rdWr  = 0;
	 wrDat = tl_a_bits_data;
	 tl_d_valid = 1;
	 tl_d_bits_opcode= `TL_D_ACCESSACK;
	 tl_d_bits_param = tl_a_bits_param;
 	 tl_d_bits_size  = tl_a_bits_size;
	 tl_d_bits_source= tl_a_bits_source;
	 tl_d_bits_sink  = 0;
	 @(posedge clock);#1;
	 tl_d_valid = 0;
	 @(posedge ack); // wait
	 req = 0;
      end // if (tl_a_bits_opcode == TL_A_PUTFULLDATA)
      else if (tl_a_bits_opcode == `TL_A_GET) begin // read
	 // issue a read to system
	 @(posedge clock);#1;	 
	 req  = 1;
	 rdWr = 1;
	 adr  = tl_a_bits_address;	 
	 @(posedge ack); // wait
	 @(posedge clock);#1;	
	 req  = 0;
	 tl_d_valid = 1;
	 tl_d_bits_opcode= `TL_D_ACCESSACKDATA;
	 tl_d_bits_param = tl_a_bits_param;
 	 tl_d_bits_size  = tl_a_bits_size;
	 tl_d_bits_source= tl_a_bits_source;
	 tl_d_bits_sink  = 0;
	 tl_d_bits_data  = rdDat;
	 @(posedge clock);#1;
	 tl_d_valid = 0;	 
      end
   end
endmodule // tl_slave_beh

