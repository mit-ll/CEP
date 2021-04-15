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

// logicalData param
`define LOGIC_XOR   0
`define LOGIC_OR    1
`define LOGIC_AND   2
`define LOGIC_SWAP  3

//
// 08/26/19: aduong : Channel A/D only (UL)
//
`include "v2c_top.incl"
`timescale 1ns/100ps
module tl_master_beh 
  #(parameter
    CHIP_ID=0, // for sim ONLY
    SRC_SIZE=2,
    SINK_SIZE=2,
    BUS_SIZE=8, 
    ADR_WIDTH=32,
    //
    localparam TL_SIZE=4, // $clog2(BUS_SIZE),
    localparam DATA_WIDTH=(BUS_SIZE*8)
    )
   ( // Master mode
     input 			 clock, 
     input 			 reset, 
     // Channel A
     input 			 tl_master_a_ready, 
     output reg 		 tl_master_a_valid=0, 
     output reg [2:0] 		 tl_master_a_bits_opcode=0, 
     output reg [2:0] 		 tl_master_a_bits_param=0, 
     output reg [TL_SIZE-1:0] 	 tl_master_a_bits_size=0, 
     output reg [SRC_SIZE-1:0] 	 tl_master_a_bits_source=0, 
     output reg [ADR_WIDTH-1:0]  tl_master_a_bits_address=0, 
     output reg [BUS_SIZE-1:0] 	 tl_master_a_bits_mask=0, 
     output reg [DATA_WIDTH-1:0] tl_master_a_bits_data=0, 
     output reg 		 tl_master_a_bits_corrupt=0, 
     // Channel B
     output reg 		 tl_master_b_ready=1, 
     input 			 tl_master_b_valid, 
     input [2:0] 		 tl_master_b_bits_opcode, 
     input [1:0] 		 tl_master_b_bits_param, 
     input [TL_SIZE-1:0] 	 tl_master_b_bits_size, 
     input [SRC_SIZE-1:0] 	 tl_master_b_bits_source, 
     input [ADR_WIDTH-1:0] 	 tl_master_b_bits_address, 
     input [BUS_SIZE-1:0] 	 tl_master_b_bits_mask, 
     input 			 tl_master_b_bits_corrupt,
    // Channel C    
     input 			 tl_master_c_ready, 
     output reg 		 tl_master_c_valid=0, 
     output reg [2:0] 		 tl_master_c_bits_opcode=0, 
     output reg [2:0] 		 tl_master_c_bits_param=0, 
     output reg [3:0] 		 tl_master_c_bits_size=0, //
     output reg [SRC_SIZE-1:0] 	 tl_master_c_bits_source=0, 
     output reg [ADR_WIDTH-1:0]  tl_master_c_bits_address=0, 
     output reg [DATA_WIDTH-1:0] tl_master_c_bits_data=0, 
     output reg 		 tl_master_c_bits_corrupt=0, 
    // Channel D
     output reg 		 tl_master_d_ready=1, // default 
     input 			 tl_master_d_valid, 
     input [2:0] 		 tl_master_d_bits_opcode, 
     input [1:0] 		 tl_master_d_bits_param, 
     input [3:0] 		 tl_master_d_bits_size,  //
     input [SRC_SIZE-1:0] 	 tl_master_d_bits_source, 
     input [SINK_SIZE-1:0] 	 tl_master_d_bits_sink, 
     input 			 tl_master_d_bits_denied, 
     input [DATA_WIDTH-1:0] 	 tl_master_d_bits_data, 
     input 			 tl_master_d_bits_corrupt, 
     // Channel E		   
     input 			 tl_master_e_ready, 
     output reg 		 tl_master_e_valid=0, 
     output reg [SINK_SIZE-1:0]  tl_master_e_bits_sink=0,
     // Misc.
     input [SRC_SIZE-1:0] 	 constants_hartid 
);

   localparam MAX_TIMEOUT = 1000;
   
 // 2560;
   //
   integer 			    i;
   reg 				    tl_err = 0;
//`define TICK_DELAY #1
`define TICK_DELAY 
   //
   // checkReset
   //
   task checkReset;
      begin
	 tl_err = 0;
	 `ifdef REMOVE_ME
	 if (reset) begin
	    `logW("Chip is in reset... Waiting to get out");
	    i = 0;
	    @(posedge clock);
	    while (reset && !tl_err && (i < MAX_TIMEOUT)) begin
	       i = i + 1;	    	       
	       @(posedge clock);
	       if (i >= MAX_TIMEOUT) begin
		  `logE("** timeout while waiting for reset to de-asserted");
		  tl_err = 1;
	       end
	       //
	    end
	 end // if (reset)
	 `endif
      end
   endtask // if

   //
   // decode for CEP
   //    
   task tl_x_ul_read;
      input [SRC_SIZE-1:0] src_id;
      input [ADR_WIDTH-1:0] a;
      output [DATA_WIDTH-1:0] d;
      begin
	 tl_a_ul_read(src_id,a,d);
      end
   endtask // checkReset

   task tl_x_ul_write;
      input [SRC_SIZE-1:0] src_id;
      input [ADR_WIDTH-1:0] a;
      input [DATA_WIDTH-1:0] d;
      //
      begin
	 tl_a_ul_write_generic(src_id,a,d,(1<<BUS_SIZE)-1,3);
      end
   endtask // tl_a_ul_write_generic
   
   //
   // TL-UL-GET  (64 bits for now)
   //
   task tl_a_ul_read;
      input [SRC_SIZE-1:0] src_id;
      input [ADR_WIDTH-1:0] a;
      output [DATA_WIDTH-1:0] d;
      //
      begin
	 checkReset();
	 if (tl_err) disable tl_a_ul_read;
	 //`logI("%m : src=%d a=0x%x %t",src_id,a,$time);
	 //
	 // use negedge to drive output
	 @(posedge clock); `TICK_DELAY;
	 tl_master_a_bits_opcode = `TL_A_GET;
	 tl_master_a_bits_param  = 0; // must
	 tl_master_a_bits_size   = 3; // log2(8bytes)
	 tl_master_a_bits_source = src_id;
	 tl_master_a_bits_address= a;
	 tl_master_a_bits_mask   = 'hFF;
	 tl_master_a_bits_data   = 0; // dont care
	 tl_master_a_valid       = 1; // issue a valid
	 // wait until slave took it
	 i = 0;
	 @(posedge clock);
	 while (!tl_master_a_ready && (i < MAX_TIMEOUT)) begin
	    i = i + 1;	    	    
	    @(posedge clock);	    
	    if (i >= MAX_TIMEOUT) begin
	       `logE(" **ERROR** timeout while waiting for ready on channelA src=%d a=0x%x",src_id,a);
	       tl_err = 1;
	    end
	 end
	 //
	 `TICK_DELAY;	 
	 tl_master_d_ready = 1; // ready to accept
	 tl_master_a_valid = 0; // take valid away
	 //
	 // wait on channel D for  respond
	 i = 0;
	 @(posedge clock);	 	 
	 //
	 while (!tl_err && !tl_master_d_valid) begin
	    i = i + 1;
	    @(posedge clock);
	    if (i > MAX_TIMEOUT) begin
	       `logE("**ERROR** timeout while waiting for data from channelD src=%d a=0x%x",src_id,a);
	       tl_err = 1;		  
	    end
	 end
	 // check and verify the data is for this tracaction
	 if (!tl_err && 
	     ((tl_master_d_bits_opcode != `TL_D_ACCESSACKDATA) ||
	      (tl_master_d_bits_param != 0) ||
//	      (tl_master_d_bits_size != 3) ||
	      (tl_master_d_bits_source != src_id) ||
	      (tl_master_d_bits_denied) ||
	      (tl_master_d_bits_corrupt))) begin
	    `logE("**ERROR** src_id=%d reported from channelD op/par/sz/src/den/cor=%x/%x/%x/%x/%x/%x",src_id,
		     tl_master_d_bits_opcode,tl_master_d_bits_param,tl_master_d_bits_size,tl_master_d_bits_source,tl_master_d_bits_denied,tl_master_d_bits_corrupt);
	    tl_err = 1;
	 end
	 else if (!tl_err) begin
	    d = tl_master_d_bits_data;	       
	    `logI("src=%d a=0x%x d=0x%x",src_id,a,d);
	 end
	 @(posedge clock);	 
      end      
   endtask 

   task tl_x_ul_read_generic;
      input [SRC_SIZE-1:0] src_id;
      input [ADR_WIDTH-1:0] a;
      input [BUS_SIZE-1:0]   mask;
      input [1:0] 	     bits_size;       // log2(8bytes)
      output [DATA_WIDTH-1:0] d;
      //
      begin
	 checkReset();
	 if (tl_err) disable tl_a_ul_read;
	 //`logI("%m : src=%d a=0x%x %t",src_id,a,$time);
	 //
	 // use negedge to drive output
	 @(posedge clock); `TICK_DELAY;
	 tl_master_a_bits_opcode = `TL_A_GET;
	 tl_master_a_bits_param  = 0; // must
	 tl_master_a_bits_size   = bits_size;
	 /*
	 if (mask == ((1<<BUS_SIZE)-1))	 	 
	   tl_master_a_bits_size   = 3; // log2(8bytes)
	 else
	   tl_master_a_bits_size   = 2; // log2(8bytes)	 
	  */
	 tl_master_a_bits_source = src_id;
	 tl_master_a_bits_address= a;
//	 tl_master_a_bits_mask   = 'hFF;
	 tl_master_a_bits_mask   = mask;	 
	 tl_master_a_bits_data   = 0; // dont care
	 tl_master_a_valid       = 1; // issue a valid
	 // wait until slave took it
	 i = 0;
	 @(posedge clock);
	 while (!tl_master_a_ready && (i < MAX_TIMEOUT)) begin
	    i = i + 1;	    	    
	    @(posedge clock);	    
	    if (i >= MAX_TIMEOUT) begin
	       `logE(" **ERROR** timeout while waiting for ready on channelA src=%d a=0x%x",src_id,a);
	       tl_err = 1;
	    end
	 end
	 //
	 `TICK_DELAY;	 
	 tl_master_d_ready = 1; // ready to accept
	 tl_master_a_valid = 0; // take valid away
	 //
	 // wait on channel D for  respond
	 i = 0;
	 @(posedge clock);	 	 
	 //
	 while (!tl_err && !tl_master_d_valid) begin
	    i = i + 1;
	    @(posedge clock);
	    if (i > MAX_TIMEOUT) begin
	       `logE("**ERROR** timeout while waiting for data from channelD src=%d a=0x%x",src_id,a);
	       tl_err = 1;		  
	    end
	 end
	 // check and verify the data is for this tracaction
	 if (!tl_err && 
	     ((tl_master_d_bits_opcode != `TL_D_ACCESSACKDATA) ||
	      (tl_master_d_bits_param != 0) ||
//	      (tl_master_d_bits_size != 3) ||
	      (tl_master_d_bits_source != src_id) ||
	      (tl_master_d_bits_denied) ||
	      (tl_master_d_bits_corrupt))) begin
	    `logE("**ERROR** src_id=%d reported from channelD op/par/sz/src/den/cor=%x/%x/%x/%x/%x/%x",src_id,
		     tl_master_d_bits_opcode,tl_master_d_bits_param,tl_master_d_bits_size,tl_master_d_bits_source,tl_master_d_bits_denied,tl_master_d_bits_corrupt);
	    tl_err = 1;
	 end
	 else if (!tl_err) begin
	    d = tl_master_d_bits_data;	       
	    `logI("src=%d a=0x%x d=0x%x",src_id,a,d);
	 end
	 @(posedge clock);	 
      end      
   endtask 
   

   
   //
   // TL-UL-PUT Full  (64 bits for now)
   //
   task tl_a_ul_write;
      input [SRC_SIZE-1:0] src_id;
      input [ADR_WIDTH-1:0] a;
      input [DATA_WIDTH-1:0] d;
      //
      begin
	 //
	 tl_a_ul_write_generic(src_id,a,d,(1<<BUS_SIZE)-1,3);
      end
   endtask // tl_a_ul_write_generic
   
   //
   // TL-UL-TL_A_PUTPARTIALDATA
   //
   task tl_a_ul_write_generic; // support both full and partial
      input [SRC_SIZE-1:0] src_id;
      input [ADR_WIDTH-1:0] a;
      input [DATA_WIDTH-1:0] d;
      input [BUS_SIZE-1:0]   mask;
      input [1:0] 	     bits_size;
      //
      begin
	 //
	 checkReset();
	 if (tl_err) disable tl_a_ul_write;
	 `logI("src=%d a=0x%x d=0x%x mask=0x%x",src_id,a,d,mask);	 
	 @(posedge clock);  `TICK_DELAY;
	 tl_master_a_bits_opcode = `TL_A_PUTFULLDATA;	 
	 /*
	 if (mask == ((1<<BUS_SIZE)-1))
	   tl_master_a_bits_opcode = `TL_A_PUTFULLDATA;
	 else
	   tl_master_a_bits_opcode = `TL_A_PUTPARTIALDATA;	 
	  */
	 tl_master_a_bits_param  = 0; // must
	 /*
	 if (mask == ((1<<BUS_SIZE)-1))	 
	   tl_master_a_bits_size   = 3; // log2(8bytes)
	 else
	   tl_master_a_bits_size   = 2; // log2(8bytes)	   
	  */
	 tl_master_a_bits_size   = bits_size;
	 //
	 tl_master_a_bits_source = src_id;
	 tl_master_a_bits_address= a;
	 tl_master_a_bits_mask   = mask;
	 tl_master_a_bits_data   = d; // dont care
	 tl_master_a_valid       = 1; // issue a valid
	 // wait until slave took it
	 i = 0;
	 @(posedge clock);	 	 
	 while (!tl_master_a_ready && (i < MAX_TIMEOUT)) begin
	    i = i + 1;			    
	    @(posedge clock);
	    if (i >= MAX_TIMEOUT) begin
	       `logE("**ERROR** timeout while waiting for ready on channelA src=%d a=0x%x",src_id,a);
	       tl_err = 1;
	    end
	 end
	 //
	 `TICK_DELAY;	 
	 tl_master_d_ready = 1; // ready to accept
	 tl_master_a_valid = 0; // take valid away
	 //
	 // wait on channel D for  respond
	 if (!tl_err) begin
	    //
	    i = 0;
	    @(posedge clock);
	    while (!tl_master_d_valid && (i < MAX_TIMEOUT)) begin
	       i = i + 1;	       	       
	       @(posedge clock);
	       if (i >= MAX_TIMEOUT) begin
		  `logE("**ERROR** timeout while waiting for data from channelD src=%d a=0x%x",src_id,a);
		  tl_err = 1;		  
	       end
	    end
	    // check and verify the data is for this tracaction
	    if (!tl_err &&
		((tl_master_d_bits_opcode != `TL_D_ACCESSACK) ||
		 (tl_master_d_bits_param != 0) ||
//		 (tl_master_d_bits_size != 3) ||
		 (tl_master_d_bits_source != src_id) ||
		 (tl_master_d_bits_denied) ||
		 (tl_master_d_bits_corrupt))) begin
	       `logE("**ERROR** src_id=%d reported from channelD op/par/sz/src/den/cor=%x/%x/%x/%x/%x/%x",src_id,
			tl_master_d_bits_opcode,tl_master_d_bits_param,tl_master_d_bits_size,tl_master_d_bits_source,tl_master_d_bits_denied,tl_master_d_bits_corrupt);
	    end
	 end // if (tl_master_a_ready)
	 @(posedge clock);	 
      end
   endtask // checkReset

   reg [DATA_WIDTH-1:0] tl_buf [31:0]; // for burst

   //
   // TL-UL-TL_A_PUTPARTIALDATA : burst!!
   // one ack per 64-bytes
   //
   task tl_a_ul_write_burst; 
      input [SRC_SIZE-1:0] src_id;
      input [ADR_WIDTH-1:0] a;
      input [BUS_SIZE-1:0]  mask;      
      input [3:0] 	     bits_size;
      //
      int  	     i, ackCnt;
      reg [7:0]      cnt;
      
      begin
	 //
	 checkReset();
	 if (tl_err) disable tl_a_ul_write_burst;
	 tl_master_d_ready = 1; // not ready to accept	 	 
	 `logI("src=%d a=0x%x size=%d mask=0x%x",src_id,a,cnt,mask);	 
	 @(posedge clock);  `TICK_DELAY;
	 tl_master_a_bits_opcode = `TL_A_PUTFULLDATA;	 
	 tl_master_a_bits_param  = 0; // must
	 tl_master_a_bits_size   = bits_size;
	 tl_master_a_bits_mask   = mask;
	 //
	 cnt = (1 << bits_size) >> 3;
	 i = 0;
	 ackCnt = 0;
	 while (i < cnt) begin
	    tl_master_a_bits_source = src_id;
	    tl_master_a_bits_address= a;
	    tl_master_a_bits_data   = tl_buf[i]; // dont care
	    tl_master_a_valid       = 1; // issue a valid
	    @(posedge clock);  `TICK_DELAY;
	    //`logI("Cnt=%d i=%d data=%x",cnt,i,tl_buf[i]);
	    if (tl_master_a_ready) i++;
	    if (tl_master_d_valid) ackCnt++;
	 end
	 tl_master_a_valid       = 0;
	 // wait until slave took it
	 i = 0;
	 @(posedge clock);
	 if (tl_master_d_valid) ackCnt++;	 
	 while (!tl_master_a_ready && (i < MAX_TIMEOUT)) begin
	    i = i + 1;			    
	    @(posedge clock);
	    if (tl_master_d_valid) ackCnt++;	    
	    if (i >= MAX_TIMEOUT) begin
	       `logE("**ERROR** timeout while waiting for ready on channelA src=%d a=0x%x",src_id,a);
	       tl_err = 1;
	    end
	 end
	 //
	 `TICK_DELAY;	 
	 tl_master_d_ready = 1; // ready to accept
	 tl_master_a_valid = 0; // take valid away
	 //
	 // wait on channel D for  respond
	 if (!tl_err) begin
	    // how many ack?? one per 8 64-bits
	    cnt = (cnt <= 8) ? 1 : (cnt >> 3);
	    while (ackCnt < cnt) begin
	       //
	       i = 0;
	       @(posedge clock);
	       while (!tl_master_d_valid && (i < MAX_TIMEOUT)) begin
		  i = i + 1;	       	       
		  @(posedge clock);
		  if (i >= MAX_TIMEOUT) begin
		     `logE("**ERROR** timeout while waiting for data from channelD src=%d a=0x%x",src_id,a);
		     tl_err = 1;		  
		  end
	       end
	       ackCnt++;
	       // check and verify the data is for this tracaction
	       if (!tl_err &&
		   ((tl_master_d_bits_opcode != `TL_D_ACCESSACK) ||
		    (tl_master_d_bits_param != 0) ||
		    (tl_master_d_bits_source != src_id) ||
		    (tl_master_d_bits_denied) ||
		    (tl_master_d_bits_corrupt))) begin
		  `logE("**ERROR** src_id=%d reported from channelD op/par/sz/src/den/cor=%x/%x/%x/%x/%x/%x",src_id,
			tl_master_d_bits_opcode,tl_master_d_bits_param,tl_master_d_bits_size,tl_master_d_bits_source,tl_master_d_bits_denied,tl_master_d_bits_corrupt);
	       end
	    end // while (ackCnt < cnt)
	 end // if (tl_master_a_ready)
	 @(posedge clock);	 
      end
   endtask // checkReset

   //
   // TL-UL-GET  (64 bits for now)
   //
   event mark;
   task tl_a_ul_read_burst;
      input [SRC_SIZE-1:0] src_id;
      input [ADR_WIDTH-1:0] a;
      input [3:0] 	    bits_size;
      //
      int  	    cnt,i,to;      
      //
      begin
	 checkReset();
	 if (tl_err) disable tl_a_ul_read_burst;
	 tl_master_d_ready = 0; // not ready to accept	 	 
	 `logI("%m : src=%x2d a=0x%x %t",src_id,a,$time);
	 //
	 // use negedge to drive output
	 @(posedge clock); `TICK_DELAY;
	 tl_master_a_bits_opcode = `TL_A_GET;
	 tl_master_a_bits_param  = 0; // must
	 tl_master_a_bits_size   = bits_size; // log2(8bytes)
	 tl_master_a_bits_source = src_id;
	 tl_master_a_bits_address= a;
	 tl_master_a_bits_mask   = 'hFF;
	 tl_master_a_bits_data   = 0; // dont care
	 tl_master_a_valid       = 1; // issue a valid
	 // wait until slave took it
	 to = 0;
	 @(posedge clock);
	 while (!tl_master_a_ready && (i < MAX_TIMEOUT)) begin
	    to++;
	    @(posedge clock);	    
	    if (to >= MAX_TIMEOUT) begin
	       `logE(" **ERROR** timeout while waiting for ready on channelA src=%d a=0x%x",src_id,a);
	       tl_err = 1;
	    end
	 end
	 //
	 `TICK_DELAY;	 
	 tl_master_d_ready = 1; // ready to accept
	 tl_master_a_valid = 0; // take valid away
	 //
	 // wait on channel D for  respond
	 cnt = (1 << bits_size) >> 3;
	 i = 0;
	 while (i < cnt) begin
	    @(posedge clock);
	    if (tl_master_d_valid) begin
	       tl_buf[i] = tl_master_d_bits_data;
	       //`logI("src=%d a=0x%x d[%d]=0x%x",src_id,a,i,tl_buf[i]);		  
	       i++;
	    end 
	 end // while (i < cnt)
      end
   endtask // checkReset

   //
   // TL-UL-LogicalData
   //
   task tl_a_ul_logical_data;
      input [SRC_SIZE-1:0]   src_id;
      input [ADR_WIDTH-1:0]  a;
      input [2:0] 	     param	 ;
      input [DATA_WIDTH-1:0] d;
      input [BUS_SIZE-1:0]   mask;
      input [1:0] 	     bits_size;
      //
      begin
	 //
	 checkReset();
	 if (tl_err) disable tl_a_ul_write;
	 `logI("src=%d a=0x%x d=0x%x mask=0x%x",src_id,a,d,mask);	 
	 @(posedge clock);  `TICK_DELAY;
	 tl_master_a_bits_opcode = `TL_A_LOGICALDATA;
	 tl_master_a_bits_param  = param; // must
	 tl_master_a_bits_size   = bits_size;
	 //
	 tl_master_a_bits_source = src_id;
	 tl_master_a_bits_address= a;
	 tl_master_a_bits_mask   = mask;
	 tl_master_a_bits_data   = d; // dont care
	 tl_master_a_valid       = 1; // issue a valid
	 // wait until slave took it
	 i = 0;
	 @(posedge clock);	 	 
	 while (!tl_master_a_ready && (i < MAX_TIMEOUT)) begin
	    i = i + 1;			    
	    @(posedge clock);
	    if (i >= MAX_TIMEOUT) begin
	       `logE("**ERROR** timeout while waiting for ready on channelA src=%d a=0x%x",src_id,a);
	       tl_err = 1;
	    end
	 end
	 //
	 `TICK_DELAY;	 
	 tl_master_d_ready = 1; // ready to accept
	 tl_master_a_valid = 0; // take valid away
	 //
	 // wait on channel D for  respond
	 if (!tl_err) begin
	    //
	    i = 0;
	    @(posedge clock);
	    while (!tl_master_d_valid && (i < MAX_TIMEOUT)) begin
	       i = i + 1;	       	       
	       @(posedge clock);
	       if (i >= MAX_TIMEOUT) begin
		  `logE("**ERROR** timeout while waiting for data from channelD src=%d a=0x%x",src_id,a);
		  tl_err = 1;		  
	       end
	    end
	    // get new data if SAWP?? FIXME!!!
	    
	 end // if (tl_master_a_ready)
	 @(posedge clock);	 
      end
   endtask // checkReset

   //
   // =============================
   // To hanle cache coherentcy!!!
   // =============================   
   // If see PROBE on B, send and ProbeAck on C
   integer t;
   
   always @(posedge clock) begin
      if (tl_master_b_valid) begin
	 if (tl_master_b_bits_opcode == `TL_B_PROBE) begin
	    `logD("Got a PROBE from channel B src=0x%x adr=0x%x cap=0x%x.. Sending back probeACK on C",
		  tl_master_b_bits_source,tl_master_b_bits_address,tl_master_b_bits_param);
	    //
	    // copy before they go away
	    tl_master_c_bits_param   = tl_master_b_bits_param;
	    tl_master_c_bits_size    = tl_master_b_bits_size;
	    tl_master_c_bits_source  = tl_master_b_bits_source;
	    tl_master_c_bits_address = tl_master_b_bits_address;
	    tl_master_c_bits_data    = 0;
	    tl_master_c_bits_corrupt = 0;
	    // respond
	    @(negedge clock);
	    tl_master_b_ready = 0;
	    tl_master_c_valid = 1;
	    tl_master_c_bits_opcode  = `TL_C_PROBEACK;
	    // wait until slave took it
	    t = 0;
	    @(posedge clock);	 	 
	    while (!tl_master_c_ready && (t < MAX_TIMEOUT)) begin
	       t = t + 1;			    
	       @(posedge clock);
	       if (i >= MAX_TIMEOUT) begin
		  `logE("**ERROR** timeout while waiting for ready on channelB src=%d a=0x%x",tl_master_c_bits_source,tl_master_c_bits_address);
		  tl_err = 1;
	       end
	    end
	    @(negedge clock);
	    tl_master_c_valid = 0; // take it away
	    tl_master_b_ready = 1;
	 end // if (tl_master_b_bits_param == `CAP_toN)
	 else begin
	    `logE("ERROR:Got a PROBE from channel B src=0x%x adr=0x%x.. but don't  know how to repsond to OPCODE=0x%x PARAM=%d",
		  tl_master_b_bits_source,tl_master_b_bits_address,tl_master_b_bits_opcode,tl_master_b_bits_param);	    
	 end
      end
   end // always @ (posedge clock)
   
   //
   // some quick self-test (called from the top)
   //
//`define WRITE_PARTIAL   
   task selfTest;
      input [ADR_WIDTH-1:0] baseReg; // register to play with
      //
      output 		    error;
      reg [ADR_WIDTH-1:0]   adr;
      reg [63:0] 	    wd64, rd64;
      integer 		    i,l;
      begin
	 error = 0;
	 tl_err = 0;
	 // =======================
	 // revision register and check
	 // =======================	 
	 //
	 adr = 'h700F_0000;
	 `logI("=== TL-UL Checking FPGA version ===");	 
	 wd64 = 64'h0002_0000_0000_0000;
	 for (l=0;l<2;l=l+1) begin // RO
	    tl_a_ul_read(CHIP_ID & 1, adr, rd64); // source=0 or 1 ONLY
	    if (wd64 !== rd64) begin
	       `logE("%m: **ERROR** Version Reg adr=0x%x exp=0x%x act=0x%x %t",adr,wd64,rd64,$time);
	       error = 1;
	    end
	    else begin
	       `logI("%m: OK Version adr=0x%x exp=0x%x act=0x%x %t",adr,wd64,rd64,$time);	    
	    end
	    // write and make sure it never change
	    if (l == 0) begin
	       tl_a_ul_write(CHIP_ID & 1, adr, ~wd64); // source=0 or 1 ONLY	       
	    end
	 end // for (l=0;l<2;l=l+1)
	 //
	 // =======================	 
	 // TL-UL read/write partial&full
	 // =======================	 
	 //
	 `ifdef WRITE_PARTIAL
	 //
	 // Using PUTPARTIAL
	 //
	 `logI("%m: === TL-UL Data bus walk 0/1 test with Partial mask ===");	 
	 adr = baseReg;	 
	 for (i=0;i<64;i=i+1) begin
	    if (error) break;
	    //
	    case (CHIP_ID)
	      3:       wd64 = 1 << i;         // walking 1 from LSB
	      2:       wd64 = 1 << (63-i);    // walking 1 from MSB
	      1:       wd64 = ~(1 << i);      // walking 0 from LSB
	      default: wd64 = ~(1 << (63-i)); // walking 1 from LSB	      
	    endcase // case (CHIP_ID)
	    // write
	    tl_a_ul_write_generic(CHIP_ID & 1, adr, {wd64[63:32],32'h12345678},'hF0,2); // upper
	    tl_a_ul_write_generic(CHIP_ID & 1, adr, {32'h9ABCDEF0,wd64[31:0]},'h0F,2); // lower
	    // read back and compare
	    tl_a_ul_read(CHIP_ID & 1, adr, rd64); // source=0 or 1 ONLY
	    //
	    if (wd64 != rd64) begin
	       `logI("%m: **ERROR** adr=0x%x exp=0x%x act=0x%x i=%d",adr,wd64,rd64,i);
	       error = 1;
	    end
	    else begin
	       `logI("%m: OK adr=0x%x exp=0x%x act=0x%x i=%d",adr,wd64,rd64,i);	    
	    end
	 end // for (i=0;i<64;i=i+1)
	 `endif
	 //
	 // Using PUTFULL
	 //
	 `logI("%m: === TL-UL Data bus walk 0/1 test with full mask ===");	 
	 adr = baseReg;	 
	 for (i=0;i<64;i=i+1) begin
	    if (error) break;	    
	    case (CHIP_ID)
	      0:       wd64 = 1 << i;         // walking 1 from LSB
	      1:       wd64 = 1 << (63-i);    // walking 1 from MSB
	      2:       wd64 = ~(1 << i);      // walking 0 from LSB
	      default: wd64 = ~(1 << (63-i)); // walking 1 from LSB	      
	    endcase // case (CHIP_ID)
	    // write
	    tl_a_ul_write(CHIP_ID & 1, adr, wd64); // source=0 or 1 ONLY
	    // read back and compare
	    tl_a_ul_read(CHIP_ID & 1, adr, rd64); // source=0 or 1 ONLY
	    //
	    if (wd64 != rd64) begin
	       `logI("%m: **ERROR** adr=0x%x exp=0x%x act=0x%x i=%d",adr,wd64,rd64,i);
	       error = 1;
	    end
	    else begin
	       `logI("%m: OK adr=0x%x exp=0x%x act=0x%x i=%d",adr,wd64,rd64,i);	    
	    end
	 end // for (i=0;i<64;i=i+1)
      end
   endtask
   //
`define fir_base_addr           32'h70070000
`define fir_ctrlstatus_addr	'h0000
`define fir_start_bit 0
`define fir_datain_we_bit 1
`define fir_dataout_valid_bit 2
   
`define fir_datain_addr_addr	'h0008
`define fir_datain_data_addr	'h0010
`define fir_dataout_addr_addr	'h0018
`define fir_dataout_data_addr	'h0020

      // assuming input is index*4      
      reg [31:0] fir_exp [0:31]  = '{32'h00000006,
				     32'h0000000c,
				  32'h00000012,
				  32'h00000018,
				  32'h0000001e,
				  32'h00000025,
				  32'h0000002c,
				  32'h00000031,
				  32'h00000039,
				  32'h0000003f,
				  32'h00000045,
				  32'h0000004d,
				  32'h00000052,
				  32'h0000005a,
				  32'h00000060,
				  32'h00000067,
				  32'h0000006c,
				  32'h00000075,
				  32'h0000007a,
				  32'h00000081,
				  32'h00000088,
				  32'h0000008e,
				  32'h00000095,
				  32'h0000009b,
				  32'h000000a2,
				  32'h000000a8,
				  32'h000000af,
				  32'h000000b4,
				  32'h000000b1,
				  32'h0000009f,
				  32'h0000007a,
				  32'h00000049}; // note this is index 31
   
   task fir_test;
      reg [63:0] rd64;      
      integer i;
      
      begin
	 //
	 // Read/Write FIR's data In FIFO
	 //
	 for (i=0;i<32;i=i+1) begin
	    tl_a_ul_write(0, `fir_base_addr + `fir_datain_data_addr, i*4); // and the data	 
	    tl_a_ul_write(0, `fir_base_addr + `fir_datain_addr_addr, i); // set the index
	    // issue a datain_we edge	 
	    tl_a_ul_write(0, `fir_base_addr + `fir_ctrlstatus_addr, 1 << `fir_datain_we_bit);
	    tl_a_ul_write(0, `fir_base_addr + `fir_ctrlstatus_addr, 0); // clear this bit
	 end
	 //
	 // issue a start edge
	 //
	 tl_a_ul_write(0, `fir_base_addr + `fir_ctrlstatus_addr, 1 << `fir_start_bit);
	 tl_a_ul_write(0, `fir_base_addr + `fir_ctrlstatus_addr, 0); // clear this bit
	 // wait til valid
	 // and then read to compare
	 i = 0;
	 rd64 = 0;
	 while (!(rd64 & (1 << `fir_dataout_valid_bit)) && (i < 32)) begin
	    tl_a_ul_read(0, `fir_base_addr + `fir_ctrlstatus_addr, rd64); 
	    i = i + 1;
	 end
	 //
	 for (i=0;i<32;i=i+1) begin
	    tl_a_ul_write(0, `fir_base_addr + `fir_dataout_addr_addr, i);
	    tl_a_ul_read(0, `fir_base_addr + `fir_dataout_data_addr, rd64);
	    if (rd64[31:0] !== fir_exp[i]) begin
	       `logI("%m: ERROR: FIR mismatch act=0x%x != exp=0x%x i=%d",rd64,fir_exp[i],i);
	       repeat (10) @(posedge clock);      	    
	       $finish;	    
	    end
	    else begin
	       `logI("%m: OK: FIR compare act=0x%x != exp=0x%x i=%d",rd64,fir_exp[i],i);	       
	    end
	 end
      end
   endtask // for
   
   
endmodule // tl_master__beh
