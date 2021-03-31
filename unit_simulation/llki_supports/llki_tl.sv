//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      llki_tl.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    
// Notes:          
//
//************************************************************************

`include "llki_struct.h"

import tlul_pkg::* ;
   
//
//
//
interface tlul_if();
   //
   // IOs
   //
   tl_chA_t chA;
   tl_chD_t chD;
   //
   // Master Mode
   //
   modport master(input chD, output chA);
   //
   // Slave Mode
   //
   modport slave(input chA, output chD);
   //
endinterface // tlul_if

//
//  Acts as Surrogate Root of Trust (SRoT)!!
//
`timescale 1ns/1ns
module SRoT_mock #(parameter MASTER_ID=8, llki_s MY_STRUCT)
  (
   tlul_if.master tlul,
   //
   input           clk, 
   input           rst
   );


   localparam UDP_BASE = 32'h12340000; // dont care
   
   //
   logic 	   tl_err = 0;
   int 		   i, to;
   
   //
   // UDP
   //
   reg [63:0] 	   udp_pkt [7:0];
   int 		   wordLen;

   reg [63:0] 	   status=0;

   function [15:0] compute_checksum;
      input int    wordLen;
      reg [31:0]   checkSum;
      begin
	 checkSum = 0;
	 for (int i=0;i<wordLen;i++) begin
	    checkSum += udp_pkt[i][63:48] + udp_pkt[i][47:32] + udp_pkt[i][31:16] + udp_pkt[i][15:0];
	 end
	 checkSum[15:0] = checkSum[15:0] + checkSum[31:16];
	 compute_checksum = ~checkSum[15:0];
      end
   endfunction // for
   //
   
   task unlockReq(output errCnt);
      begin
	 $displayh("=== %m Running Unlocking Sequence for core_id=%0d : ===", MY_STRUCT.core_id);
	 // build up an UDP
	 wordLen = (MY_STRUCT.key_size + 2);
	 // word[0]
	 udp_pkt[0][63:48] = MASTER_ID;
	 udp_pkt[0][47:32] = MY_STRUCT.core_id;
	 udp_pkt[0][31:16] = wordLen * 8;
	 udp_pkt[0][15:0]  = 0;
	 // word[1] = command & attribute
	 udp_pkt[1][63:56] = LoadKeyReq; 
	 udp_pkt[1][55:48] = MY_STRUCT.key_size;
	 udp_pkt[1][47:0]  = 0; // fixme
	 //
	 for (i=0;i<MY_STRUCT.key_size;i++) begin
	    udp_pkt[2+i] = MY_STRUCT.key[i];
	 end
	 //
	 // compute checksum!!! here
	 //
	 udp_pkt[0][15:0]  = compute_checksum(wordLen);

	 
	 // now send
	 for (int i=0;i<wordLen;i++) begin
	    tl_a_ul_write(MY_STRUCT.core_id, UDP_BASE, udp_pkt[i]);
	 end
	 // check for completion??
	 status = 0;
	 to = 100;
	 while ((to > 0) && (status[1:0] == 0)) begin
	    tl_a_ul_read(MY_STRUCT.core_id, UDP_BASE, status);
	    to--;
	    repeat (2) @(posedge clk);
	 end
	 if (status[1:0] != 2'b01) begin
	    $display("%m ERROR: command not successful status=0x%x",status);
	    tl_err = 1;	    
	 end
      end
   endtask //
      
   task clearKey(output errCnt);
      begin
	 $displayh("=== %m Running clearKey for core_id=%0d : ===", MY_STRUCT.core_id);
	 // build up an UDP
	 wordLen = 2;
	 //
	 udp_pkt[0][63:48] = MASTER_ID;
	 udp_pkt[0][47:32] = MY_STRUCT.core_id;
	 udp_pkt[0][31:16] = wordLen * 8;
	 udp_pkt[0][15:0]  = 0;
	 // word[1] = command & attribute
	 udp_pkt[1][63:56] = ClearKeyReq;
	 udp_pkt[1][55:48] = 0;
	 udp_pkt[1][47:0]  = 0; // fixme	 
	 //
	 // compute checksum!!! here
	 //
	 udp_pkt[0][15:0]  = compute_checksum(wordLen);	 
	 // now send
	 for (int i=0;i<wordLen;i++) begin
	    tl_a_ul_write(MY_STRUCT.core_id, UDP_BASE, udp_pkt[i]);
	 end
	 // check for completion??
	 status = 0;
	 to = 10;
	 while ((to > 0) && (status[1:0] == 0)) begin
	    tl_a_ul_read(MY_STRUCT.core_id, UDP_BASE, status);
	    to--;
	 end
	 if (status[1:0] != 2'b01) begin
	    $display("%m ERROR: command not successful status=0x%x",status);
	    tl_err = 1;
	    
	 end	 
      end
   endtask // clearKey
   
   //
   // TL-UL-GET  (64 bits for now)
   //
   task tl_a_ul_read;
      input [7:0] src_id;
      input [31:0] a;
      output [63:0] d;
      //
      begin
	 if (tl_err) disable tl_a_ul_read;
	 //$display("%m : src=%d a=0x%x %t",src_id,a,$time);
	 //
	 // use negedge to drive output
	 @(posedge clk); `TICK_DELAY;
	 tlul.chA.a_opcode = `TL_A_GET;
	 tlul.chA.a_param  = 0; // must
	 tlul.chA.a_size   = 3; // log2(8bytes)
	 tlul.chA.a_source = src_id;
	 tlul.chA.a_address= a;
	 tlul.chA.a_mask   = 'hFF;
	 tlul.chA.a_data   = 0; // dont care
	 tlul.chA.a_valid       = 1; // issue a valid
	 // wait until slave took it
	 i = 0;
	 @(posedge clk);
	 while (!tlul.chD.a_ready && (i < `MAX_TIMEOUT)) begin
	    i = i + 1;	    	    
	    @(posedge clk);	    
	    if (i >= `MAX_TIMEOUT) begin
	       $display(" **ERROR** timeout while waiting for ready on channelA src=%d a=0x%x",src_id,a);
	       tl_err = 1;
	    end
	 end
	 //
	 `TICK_DELAY;	 
	 tlul.chA.d_ready = 1; // ready to accept
	 tlul.chA.a_valid = 0; // take valid away
	 //
	 // wait on channel D for  respond
	 i = 0;
	 @(posedge clk);	 	 
	 //
	 while (!tl_err && !tlul.chD.d_valid) begin
	    i = i + 1;
	    @(posedge clk);
	    if (i > `MAX_TIMEOUT) begin
	       $display("**ERROR** timeout while waiting for data from channelD src=%d a=0x%x",src_id,a);
	       tl_err = 1;		  
	    end
	 end
	 // check and verify the data is for this tracaction
	 if (!tl_err && 
	     ((tlul.chD.d_opcode != `TL_D_ACCESSACKDATA) ||
	      (tlul.chD.d_param != 0) ||
	      (tlul.chD.d_size != 3) ||
	      (tlul.chD.d_source != src_id) ||
	      (tlul.chD.d_denied) ||
	      (tlul.chD.d_corrupt))) begin
	    $display("**ERROR** reported from channelD op/par/sz/src/den/cor=%x/%x/%x/%x/%x/%x",
		     tlul.chD.d_opcode,tlul.chD.d_param,tlul.chD.d_size,tlul.chD.d_source,tlul.chD.d_denied,tlul.chD.d_corrupt);
	    tl_err = 1;
	 end
	 else if (!tl_err) begin
	    d = tlul.chD.d_data;	       
	    $display("RD: src=%d a=0x%x d=0x%x",src_id,a,d);
	 end
	 @(posedge clk);	 
      end      
   endtask 
   
   //
   // TL-UL-PUT Full  (64 bits for now)
   //
   task tl_a_ul_write;
      input [7:0] src_id;
      input [31:0] a;
      input [63:0] d;
      //
      begin
	 //
	 tl_a_ul_write_generic(src_id,a,d,(1<<8)-1); 
      end
   endtask // tl_a_ul_write_generic
   
   //
   // TL-UL-TL_A_PUTPARTIALDATA
   //
   task tl_a_ul_write_generic; // support both full and partial
      input [7:0] src_id;
      input [31:0] a;
      input [63:0] d;
      input [7:0]  mask;
      //
      begin
	 //
	 if (tl_err) disable tl_a_ul_write;
	 $display("WR: src=%d a=0x%x d=0x%x mask=0x%x",src_id,a,d,mask);	 
	 @(posedge clk);  `TICK_DELAY;	 
	 if (mask == ((1<<8)-1))
	   tlul.chA.a_opcode = `TL_A_PUTFULLDATA;
	 else
	   tlul.chA.a_opcode = `TL_A_PUTPARTIALDATA;	 
	 tlul.chA.a_param  = 0; // must
	 tlul.chA.a_size   = 3; // log2(8bytes)
	 tlul.chA.a_source = src_id;
	 tlul.chA.a_address= a;
	 tlul.chA.a_mask   = mask;
	 tlul.chA.a_data   = d; // dont care
	 tlul.chA.a_valid       = 1; // issue a valid
	 // wait until slave took it
	 i = 0;
	 @(posedge clk);	 	 
	 while (!tlul.chD.a_ready && (i < `MAX_TIMEOUT)) begin
	    i = i + 1;			    
	    @(posedge clk);
	    if (i >= `MAX_TIMEOUT) begin
	       $display("**ERROR** timeout while waiting for ready on channelA src=%d a=0x%x",src_id,a);
	       tl_err = 1;
	    end
	 end
	 //
	 `TICK_DELAY;	 
	 tlul.chA.d_ready = 1; // ready to accept
	 tlul.chA.a_valid = 0; // take valid away
	 //
	 // wait on channel D for  respond
	 if (!tl_err) begin
	    //
	    i = 0;
	    @(posedge clk);
	    while (!tlul.chD.d_valid && (i < `MAX_TIMEOUT)) begin
	       i = i + 1;	       	       
	       @(posedge clk);
	       if (i >= `MAX_TIMEOUT) begin
		  $display("**ERROR** timeout while waiting for data from channelD src=%d a=0x%x",src_id,a);
		  tl_err = 1;		  
	       end
	    end
	    // check and verify the data is for this tracaction
	    if (!tl_err &&
		((tlul.chD.d_opcode != `TL_D_ACCESSACK) ||
		 (tlul.chD.d_param != 0) ||
		 (tlul.chD.d_size != 3) ||
		 (tlul.chD.d_source != src_id) ||
		 (tlul.chD.d_denied) ||
		 (tlul.chD.d_corrupt))) begin
	       $display("**ERROR** reported from channelD op/par/sz/src/den/cor=%x/%x/%x/%x/%x/%x",
			tlul.chD.d_opcode,tlul.chD.d_param,tlul.chD.d_size,tlul.chD.d_source,tlul.chD.d_denied,tlul.chD.d_corrupt);
	    end
	 end // if (tlul.chA.a_ready)
	 @(posedge clk);	 
      end
   endtask // checkReset
   
endmodule // tlul_master

//
// LLKI-PP (to be placed inside TL* wrapper)
//
module llki_pp_mock #(parameter MY_ID=0)
  (
   // TileLink Side
   tlul_if.slave           tlul,
   // LLKI-discrete side
   llki_discrete_if.master discrete,   
   // clk/rst
   input           clk, 
   input           rst
   );

   //
   logic [7:0] 	   save_src_id;
   logic [1:0] 	   save_req_size;
   logic [2:0] 	   return_opcode;
   logic 	   notReady;    // Indicates current request is notReady
   logic [63:0]    read_data;
   //
   //
   logic [63:0]    discrete_status;
   
   //
   // ******************************
   // TL-slave logic
   // ******************************   
   //   
   wire  chA_valid   = tlul.chA.a_valid & tlul.chD.a_ready;
   wire  chD_valid   = tlul.chD.d_valid & tlul.chA.d_ready;
   //
   wire  isWrite = chA_valid & (tlul.chA.a_opcode == `TL_A_PUTFULLDATA); // only 64-bit
   wire  isRead  = chA_valid & (tlul.chA.a_opcode == `TL_A_GET);
   
   //
   always @(posedge clk or posedge rst) begin
      if (rst) begin
	 notReady      <= 1'b0;
	 save_src_id   <= '0;
	 save_req_size <= '0;
	 return_opcode <= '0;
	 read_data     <= '0;	 
      end
      else begin
	 if (chA_valid) 
	   notReady <= 1'b1;
	 else if (chD_valid) 
	   notReady <= 1'b0;
	 //
	 if (chA_valid) begin
	    save_src_id   <= tlul.chA.a_source;
	    save_req_size <= tlul.chA.a_size;
	    return_opcode <= (isRead) ? `TL_D_ACCESSACKDATA : `TL_D_ACCESSACK ;
	 end
	 read_data <= discrete_status;
      end
   end
   //
   assign tlul.chD = '{a_ready  : ~notReady,
		       d_valid  : notReady,
		       d_opcode : return_opcode,
		       d_size   : save_req_size,
		       d_source : save_src_id,
		       d_data   : read_data,
		       d_param  : '0,		       
		       d_sink   : '0,
		       d_denied : '0,
		       d_corrupt: '0
		       };
   //
   // ******************************
   // discrete master logic
   // ******************************   
   //
   reg [63:0] 	     udp_pkt [7:0];
   logic 	     llki_err;
   logic 	     llki_done;
   logic [2:0] 	     keyId;
   logic [2:0] 	     wrP;
   logic [1:0] 	     state;
   localparam 
     IDLE      = 2'b00,
     DELAY     = 2'b01,
     LOAD_BUSY = 2'b11,
     WAIT4ACK  = 2'b10;
   
   //
   assign discrete_status = {
			     wrP[2:0],               // [6:4]
			     discrete.key_complete,  // [3]
			     discrete.key_ready,     // [2]
			     llki_err,               // [1]
			     llki_done               // [0]			     
			     };
   
   //
   // break down word[0] of UDP
   //
   wire [63:0] 	     udp_src    = udp_pkt[0][63:48];
   wire [15:0] 	     udp_dst    = udp_pkt[0][47:32];
   wire [15:0] 	     udp_len    = udp_pkt[0][31:16]; // must be multiple of 64-bits
   wire [15:0] 	     udp_chksum = udp_pkt[0][15:0];
   //
   // break down word[1] of UDP = LLKI command
   //
   wire [7:0] 	     llki_cmd   = udp_pkt[1][63:56];
   wire [7:0] 	     llki_size  = udp_pkt[1][55:48];
   wire [15:0] 	     llki_attr  = udp_pkt[1][47:32];
   // key 0 start at upd_pkt[2]...
   //
   wire 	     pkt_complete = (udp_len[15:3] == wrP) && (wrP != 0);
   wire 	     crc_err      = 0; // fixme!!!
   //
   //
   assign discrete.key_data = udp_pkt[2+keyId];
   //
   reg [31:0]  curChkSum;
   wire [15:0] actChkSum = curChkSum[15:0] + curChkSum[31:16];
   wire        chksumOk = actChkSum == 16'hFFFF;
   //
   always @(posedge clk or posedge rst) begin
      if (rst) begin
	 discrete.key_valid <= 0; // 
	 discrete.clear_key <= 0;
	 llki_err           <= 0;
	 llki_done          <= 0;
	 keyId              <= 0;
	 wrP                <= 0;
	 state              <= IDLE;
	 curChkSum          <= 0;
	 for (int i=0;i<8;i++) udp_pkt[i] <= 0;
      end
      else begin
	 discrete.key_valid <= 0; // pulse
	 discrete.clear_key <= 0;	 
	 //
	 // Push to UDP_PKT
	 //
	 if (isWrite) begin
	    udp_pkt[wrP] <= tlul.chA.a_data;
	    curChkSum    <= curChkSum + tlul.chA.a_data[63:48] + tlul.chA.a_data[47:32] + tlul.chA.a_data[31:16] + tlul.chA.a_data[15:0];	    
	    wrP          <= wrP + 1;
	    llki_done    <= 0;
	 end
	 // process if command receives is completed!!
	 case (state)
	   IDLE: begin
	      if (pkt_complete && (!chksumOk || crc_err || (MY_ID != udp_dst))) begin // start process the package
		 llki_err  <= 1; // set the flag and terminate
		 wrP       <= 0;
		 llki_done <= 1;
	      end
	      else if (pkt_complete) begin
		 // execute the command in udp[1]
		 case (llki_cmd)
		   LoadKeyReq: begin
		      keyId              <= 0;
		      llki_err           <= 0;
		      discrete.key_valid <= 1;
		      state              <= DELAY;
		   end
		   ClearKeyReq: begin
		      llki_err           <= 0;
		      discrete.clear_key <= 1;
		      state              <= WAIT4ACK;		      
		   end
		 endcase
	      end // else: !if(pkt_complete && crc_err)
	   end // case: IDLE
	   
	   DELAY: begin
	      state <= LOAD_BUSY;
	   end
	   
	   WAIT4ACK: begin
	      if (discrete.clear_key_ack) begin
		 state              <= IDLE;
		 llki_done          <= 1;
		 curChkSum          <= 0;		 
		 wrP                <= 0;		 
	      end
	   end
	   //
	   LOAD_BUSY: begin
	      // wait til ready
	      if (discrete.key_ready) begin
		 if (keyId == (llki_size-1)) begin // done
		    state              <= IDLE;
		    llki_done          <= 1;
		    curChkSum          <= 0;		 		    
		    wrP                <= 0;
		 end
		 else begin
		    keyId              <= keyId+1;
		    discrete.key_valid <= 1;
		    state              <= DELAY;
		 end
	      end
	   end // case: LOAD_BUSY
	 endcase // case (state)
      end
   end
   
endmodule // llki_pp
