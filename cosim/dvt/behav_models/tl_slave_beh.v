//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************


//
// TileLink Defines
//
/*
  Channel Opcode Message Operation Response
A 0 PutFullData Put AccessAck
A 1 PutPartialData Put AccessAck
A 2 ArithmeticData Atomic AccessAckData
A 3 LogicalData Atomic AccessAckData
A 4 Get Get AccessAckData
A 5 Intent Intent HintAck
A 6 Acquire Acquire Grant or GrantData
B 0 PutFullData Put AccessAck
B 1 PutPartialData Put AccessAck
B 2 ArithmeticData Atomic AccessAckData
B 3 LogicalData Atomic AccessAckData
B 4 Get Get AccessAckData
B 5 Intent Intent HintAck
B 6 Probe Acquire ProbeAck or ProbeAckData
C 0 AccessAck Put
C 1 AccessAckData Get or Atomic
C 2 HintAck Intent
C 4 ProbeAck Acquire
C 5 ProbeAckData Acquire
C 6 Release Release ReleaseAck
C 7 ReleaseData Release ReleaseAck
D 0 AccessAck Put
D 1 AccessAckData Get or Atomic
D 2 HintAck Intent
D 4 Grant Acquire GrantAck
D 5 GrantData Acquire GrantAck
D 6 ReleaseAck Release
E - GrantAck Acquire
Table 5.3: Summary of TileLink messages, ordered by channel and opcode.
  */
//
// TileLink Opcodes
//
`define TL_A_PUTFULLDATA	0 	
`define TL_A_PUTPARTIALDATA 	1 
`define TL_A_ARITHMETICDATA 	2 
`define TL_A_LOGICALDATA 	3 
`define TL_A_GET 		4 
`define TL_A_INTENT 		5 
`define TL_A_ACQUIRE 		6 	

`define TL_B_PUTFULLDATA 	0 
`define TL_B_PUTPARTIALDATA 	1 
`define TL_B_ARITHMETICDATA 	2 
`define TL_B_LOGICALDATA 	3 
`define TL_B_GET 		4 
`define TL_B_INTENT 		5 
`define TL_B_PROBE 		6 

`define TL_C_ACCESSACK 		0 	
`define TL_C_ACCESSACKDATA 	1 
`define TL_C_HINTACK 		2 	
`define TL_C_PROBEACK 		4 	
`define TL_C_PROBEACKDATA 	5 
`define TL_C_RELEASE RELEASE 	6 	
`define TL_C_RELEASEDATA 	7 

`define TL_D_ACCESSACK 		0 	
`define TL_D_ACCESSACKDATA 	1 
`define TL_D_HINTACK 		2 	
`define TL_D_GRANT 		4 
`define TL_D_GRANTDATA 		5 	
`define TL_D_RELEASEACK 	6 	
//
// Parameters
// 
// Cap types (Grant = new permissions, Probe = permisions <= target)
`define CAP_toT    0
`define CAP_toB    1
`define CAP_toN    2

// Grow types (Acquire = permissions >= target)
`define GROW_NtoB  0
`define GROW_NtoT  1
`define GROW_BtoT  2
 
// Shrink types (ProbeAck, Release)
`define SHRINK_TtoB 0
`define SHRINK_TtoN 1
`define SHRINK_BtoN 2

// Report types (ProbeAck, Release)
`define REPORT_TtoT 3
`define REPORT_BtoB 4
`define REPORT_NtoN 5

//
// 08/26/19: aduong : Channel A/D only (UL)
//
`include "v2c_top.incl"
`timescale 1ns/100ps
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
      if (tl_a_bits_opcode == TL_A_PUTFULLDATA) begin // write
	 @(posedge clock);#1;
	 req   = 1;
	 adr   = tl_a_bits_address;	 	 
	 rdWr  = 0;
	 wrDat = tl_a_bits_data;
	 tl_d_valid = 1;
	 tl_d_bits_opcode= TL_D_ACCESSACK;
	 tl_d_bits_param = tl_a_bits_param;
 	 tl_d_bits_size  = tl_a_bits_size;
	 tl_d_bits_source= tl_a_bits_source;
	 tl_d_bits_sink  = 0;
	 @(posedge clock);#1;
	 tl_d_valid = 0;
	 @(posedge ack); // wait
	 req = 0;
      end // if (tl_a_bits_opcode == TL_A_PUTFULLDATA)
      else if (tl_a_bits_opcode == TL_A_GET) begin // read
	 // issue a read to system
	 @(posedge clock);#1;	 
	 req  = 1;
	 rdWr = 1;
	 adr  = tl_a_bits_address;	 
	 @(posedge ack); // wait
	 @(posedge clock);#1;	
	 req  = 0;
	 tl_d_valid = 1;
	 tl_d_bits_opcode= TL_D_ACCESSACKDATA;
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

