//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
//
// File Name:      aes_tb.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    AES Core unit testbench
// Notes:        
//
//************************************************************************

`timescale 1ns/1ns

//
// Name of the DUT & TB if not pass in from Make
//
`ifndef DUT_NAME
 `define DUT_NAME aes_192
`endif

`ifndef TB_NAME
 `define TB_NAME(d) d``_tb
`endif


//
// Pull in the stimulus and other info
//
`include "aes_stimulus.txt"
//
// Some derived macros
//
//
`define MKSTR(x) `"x`"
//
// Check and print if error
// Order of arguments MUST match sample order
//
// x=sample data
// i1=input#1, i2=input#2, etc..
// o1=output#1, o2=output#2, etc..
// j* = dont care input/output (used for HEX filler)
//
`define APPLY_N_CHECK(x,l1o,lr,lc,la,l1i,lkv,lck,ld,ji1,i1,i2,i3,jo1,o1,o2) \
  {l1o,elr,elc,ela,l1i,lkv,lck,ld, \
   ji1,i1,i2,i3,jo1,exp_``o1,exp_``o2}=x; \
  exp_pat={elr,elc,ela,exp_``o1,exp_``o2}; \
  act_pat={ lr, lc, la,o1,      o2}; \
  if (exp_pat!==act_pat) begin \
     $display("ERROR: miscompared at sample#%0d",i); \
     if (errCnt==0) $display("  PAT={%s,%s}", `"o1`",`"o2`"); \
     $display("  EXP=0x%x",exp_pat); \
     $display("  ACT=0x%x",act_pat); \
     errCnt++;\
  end


//
//
module `TB_NAME ;

   //
   //
   string dut_name_list [] = '{`MKSTR(`DUT_NAME)};
   reg [`AES_OUTPUT_WIDTH-1:0]  exp_pat, act_pat;
   //
   // LLKI IOs
   //
   reg 				  elr,elc,ela;
   reg 				  l1o;
   reg [1:0] 			  l1i;
   
   wire 			  llkid_key_ready;
   wire 			  llkid_key_complete;
   wire 			  llkid_clear_key_ack;
   reg 				  llkid_clear_key;   
   reg 				  llkid_key_valid;   
   reg [63:0] 			  llkid_key_data;
   //
   // IOs
   //
   reg 				clk = 0;
   reg 				rst = 1;   
   reg 				start=0;
   reg [127:0] 			state=0;
   reg [191:0] 			key=0;
   wire [127:0] 		out;
   wire 			out_valid;
   //
   // filler & expected output
   //
   reg [2:0] 			ji1=0;
   reg [2:0] 			jo1=0;
   reg [127:0] 			exp_out;
   reg 				exp_out_valid;
   //
   int 				errCnt=0;
   //
   // Simple clock driving the DUT
   //
   initial begin
      forever #5 clk = !clk;
   end
   //    
   //
   // DUT instantiation
   //
   `DUT_NAME dut(.*);
   //
   // -------------------
   // Test starts here
   // -------------------   
   //
   initial begin
      //
      // Pulse the DUT's reset & drive input to zeros (known states)
      //
      {llkid_key_valid,llkid_clear_key,llkid_key_data,start,key[191:0],state[127:0]} = 0;
      //
      rst = 1;
      repeat (5) @(posedge clk);
      @(negedge clk);      // in stimulus, rst de-asserted after negedge
      #2 rst = 0;
      repeat (30) @(negedge clk);  // need to wait this long for output to stablize
      //
      if (!errCnt) playback_data(0);
      //
      // print summary
      //
      if (errCnt) begin
	 $display("==== DUT=%s TEST FAILED with %0d errors ====",dut_name_list[0],errCnt);
      end
      else begin
	 $display("==== DUT=%s TEST PASSED  ====",dut_name_list[0]);
      end
      //
      $finish;
   end
   //
   // Read data from file into buffer and playback for compare
   //
   task playback_data(input int StopOnError);
      int i;
      event err;
      begin
	 //
	 // open file for checking
	 //
	 $display("Reading %d samples from buffer AES_buffer",`AES_SAMPLE_COUNT);
	 // now playback and check
	 for (i=0;i<`AES_SAMPLE_COUNT;i++) begin
	    // the order MUST match the samples' order
	    `APPLY_N_CHECK(AES_buffer[i],
			   l1o,llkid_key_ready,llkid_key_complete,llkid_clear_key_ack,
			   l1i,llkid_key_valid,llkid_clear_key,
			   llkid_key_data,
			   ji1,start,key[191:0],state[127:0],jo1,out_valid,out[127:0]);
	    @(negedge clk); // next sample
	    // get out as soon found one error
	    if (errCnt && StopOnError) break;
	 end // for (int i=0;i<`AES_SAMPLE_COUNT;i++)
      end
   endtask //   
   
endmodule // aes_tb

