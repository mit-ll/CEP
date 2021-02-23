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
 `define DUT_NAME md5
`endif

`ifndef TB_NAME
 `define TB_NAME(d) d``_tb
`endif

//
// Pull in the stimulus and other info
//
`include "md5_stimulus.txt"
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
`define APPLY_N_CHECK(x,ji1,i1,ji2,i2,i3,jo1,o1,jo2,o2,o3) \
  {ji1,i1,ji2,i2,i3,jo1,exp_``o1,jo2,exp_``o2,exp_``o3}=x; \
  exp_pat={exp_``o1,exp_``o2,exp_``o3}; \
  act_pat={o1,o2,o3}; \
  if (exp_pat!==act_pat) begin \
     $display("ERROR: miscompared at sample#%0d",i); \
     if (errCnt==0) $display("  PAT={%s,%s,%s}", `"o1`",`"o2`",`"o3`"); \
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
   reg [`MD5_OUTPUT_WIDTH-1:0]  exp_pat, act_pat;
   //
   // IOs
   //
   reg 			    clk=0;                      // reg clock
   reg 			    rst=1;                      // global rst
   reg 			    init=0;                    // test also toggle rst
   reg [511:0] 		    msg_padded=0;               // input message, already padded
   reg 			    msg_in_valid=0;             // input message is valid, active high   
   wire [127:0] 	    msg_output;               // output message, always 128 bit wide
   wire 		    msg_out_valid;            // if asserted, output message is valid
   wire 		    ready;                    // the core is ready for an input message
   
   //
   // filler & expected output
   //
   reg [127:0] 	    exp_msg_output=0;               // output message, always 128 bit wide
   reg 		    exp_msg_out_valid=0;            // if asserted, output message is valid
   reg 		    exp_ready=0;                    // the core is ready for an input message

   reg [2:0] 	    ji1=0;
   reg [2:0] 	    ji2=0;
   reg [2:0] 	    jo1=0;
   reg [2:0] 	    jo2=0;
   
   //
   int 		errCnt=0;
   //
   // Simple clock driving the DUT
   //
   initial begin
      forever #5 clk = !clk;
   end
   //
   // include LLKI
   //
`ifdef LLKI_EN
 `include "../llki_supports/llki_rom.sv"
   //
   // LLKI supports
   //
   llki_discrete_if #(.core_id(`MD5_ID)) discrete();
   // LLKI master
   llki_discrete_master discreteMaster(.llki(discrete.master), .*);
   //    
   //
   // DUT instantiation
   //
   `DUT_NAME #(.MY_STRUCT(MD5_LLKI_STRUCT)) dut(.llki(discrete.slave),.*);   
`else
   //    
   //
   // DUT instantiation
   //
   `DUT_NAME dut(.*);
`endif
   //
   // -------------------
   // Test starts here
   // -------------------   
   //
   initial begin
      //
      // Pulse the DUT's reset & drive input to zeros (known states)
      //
      {init,msg_in_valid,msg_padded}=0;
      //
      rst = 1;
      repeat (5) @(posedge clk);
      @(negedge clk);      // in stimulus, rst de-asserted after negedge
      #2 rst = 0;
      repeat (30) @(negedge clk);  // need to wait this long for output to stablize
      //
      // do the unlocking here if enable
      //
`ifdef LLKI_EN
      discreteMaster.unlockReq(errCnt);
      discreteMaster.clearKey(errCnt);
      // do the playback and verify that it breaks since we clear the key
      playback_data(1);
      //
      if (errCnt) begin
	 $display("==== DUT=%s error count detected as expected due to logic lock... %0d errors ====",dut_name_list[0],errCnt);
	 errCnt  = 0;
	 //
	 // need to pulse the reset since the core might stuck in some bad state
	 //
	 {init,msg_in_valid,msg_padded}=0;
	 //
	 rst = 1;
	 repeat (21) @(posedge clk);
	 @(negedge clk);      // in stimulus, reset de-asserted after negedge
	 #2 rst = 0;
	 repeat (100) @(negedge clk);  // need to wait this long for output to stablize	 
	 //
	 // unlock again
	 //
	 discreteMaster.unlockReq(errCnt);      
      end
      else begin
	 $display("==== DUT=%s  error=%0d?? Expect at least 1 ====",dut_name_list[0],errCnt);
	 errCnt++; // fail
      end

`endif      
      //
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
	 $display("Reading %d samples from buffer MD5_buffer",`MD5_SAMPLE_COUNT);
	 // now playback and check
	 for (i=0;i<`MD5_SAMPLE_COUNT;i++) begin
	    // the order MUST match the samples' order
	    `APPLY_N_CHECK(MD5_buffer[i],ji1,init,ji2,msg_in_valid,msg_padded[511:0],jo1,ready,jo2,msg_out_valid,msg_output[127:0]);
	    @(negedge clk); // next sample
	    // 	       // get out as soon found one error
	    if (errCnt && StopOnError) break;
	 end // for (int i=0;i<`MD5_SAMPLE_COUNT;i++)
      end
   endtask //   
   
endmodule // md5_tb

