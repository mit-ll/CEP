//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      tl_master_beh.v
// Program:        Common Evaluation Platform (CEP)
// Description:    Tilelink Master Bus Functional Model for the
//                 CEP Co-Simulation Environment
// Notes:          
//
//--------------------------------------------------------------------------------------

`include "suite_config.v"
`include "tl_defines.incl"
`include "v2c_top.incl"

module tl_master_beh #(
  parameter CHIP_ID       = 0,
  parameter SRC_SIZE      = 2,
  parameter SINK_SIZE     = 2,
  parameter BUS_SIZE      = 8, 
  parameter ADR_WIDTH     = 32,

  localparam TL_SIZE      = 4, // $clog2(BUS_SIZE),
  localparam DATA_WIDTH   = (BUS_SIZE*8),
  localparam MAX_TIMEOUT  = 1000
) (
  input                       clock, 
  input                       reset, 
  
  // Channel A
  input                       tl_master_a_ready, 
  output reg                  tl_master_a_valid = 0, 
  output reg [2:0]            tl_master_a_bits_opcode = 0, 
  output reg [2:0]            tl_master_a_bits_param = 0, 
  output reg [TL_SIZE-1:0]    tl_master_a_bits_size = 0, 
  output reg [SRC_SIZE-1:0]   tl_master_a_bits_source = 0, 
  output reg [ADR_WIDTH-1:0]  tl_master_a_bits_address = 0, 
  output reg [BUS_SIZE-1:0]   tl_master_a_bits_mask = 0, 
  output reg [DATA_WIDTH-1:0] tl_master_a_bits_data = 0, 
  output reg                  tl_master_a_bits_corrupt = 0, 
  
  // Channel B
  output reg                  tl_master_b_ready = 1, 
  input                       tl_master_b_valid, 
  input [2:0]                 tl_master_b_bits_opcode, 
  input [1:0]                 tl_master_b_bits_param, 
  input [TL_SIZE-1:0]         tl_master_b_bits_size, 
  input [SRC_SIZE-1:0]        tl_master_b_bits_source, 
  input [ADR_WIDTH-1:0]       tl_master_b_bits_address, 
  input [BUS_SIZE-1:0]        tl_master_b_bits_mask, 
  input                       tl_master_b_bits_corrupt,
 
  // Channel C    
  input                       tl_master_c_ready, 
  output reg                  tl_master_c_valid = 0, 
  output reg [2:0]            tl_master_c_bits_opcode = 0, 
  output reg [2:0]            tl_master_c_bits_param = 0, 
  output reg [3:0]            tl_master_c_bits_size = 0,
  output reg [SRC_SIZE-1:0]   tl_master_c_bits_source = 0, 
  output reg [ADR_WIDTH-1:0]  tl_master_c_bits_address = 0, 
  output reg [DATA_WIDTH-1:0] tl_master_c_bits_data = 0, 
  output reg                  tl_master_c_bits_corrupt = 0, 
  
  // Channel D
  output reg                  tl_master_d_ready = 1,
  input                       tl_master_d_valid, 
  input [2:0]                 tl_master_d_bits_opcode, 
  input [1:0]                 tl_master_d_bits_param, 
  input [3:0]                 tl_master_d_bits_size,
  input [SRC_SIZE-1:0]        tl_master_d_bits_source, 
  input [SINK_SIZE-1:0]       tl_master_d_bits_sink, 
  input                       tl_master_d_bits_denied, 
  input [DATA_WIDTH-1:0]      tl_master_d_bits_data, 
  input                       tl_master_d_bits_corrupt, 
  
  // Channel E          
  input                       tl_master_e_ready, 
  output reg                  tl_master_e_valid = 0, 
  output reg [SINK_SIZE-1:0]  tl_master_e_bits_sink = 0
);

  integer              i;
  reg                  tl_err = 0;
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
//        (tl_master_d_bits_size != 3) ||
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
      input [1:0]        bits_size;       // log2(8bytes)
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
//   tl_master_a_bits_mask   = 'hFF;
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
//        (tl_master_d_bits_size != 3) ||
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
      input [1:0]        bits_size;
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
//       (tl_master_d_bits_size != 3) ||
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
      input [3:0]        bits_size;
      //
      int        i, ackCnt;
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
      input [3:0]       bits_size;
      //
      int       cnt,i,to;      
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
      input [2:0]        param   ;
      input [DATA_WIDTH-1:0] d;
      input [BUS_SIZE-1:0]   mask;
      input [1:0]        bits_size;
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
        
     end // if (tl_master_a_ready)
     @(posedge clock);   
      end
   endtask // checkReset

   //
   // =============================
   // To handle cache coherency!!!
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
      output            error;
      reg [ADR_WIDTH-1:0]   adr;
      reg [63:0]        wd64, rd64;
      integer           i,l;
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
   
endmodule // tl_master__beh
