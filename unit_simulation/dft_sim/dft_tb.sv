//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
//
// File Name:      dft_tb.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    DFT Core unit testbench
// Notes:        
//
//************************************************************************

`timescale 1ns/1ns

//
// Name of the DUT & TB if not pass in from Make
//
`ifndef DUT_NAME
 `define DUT_NAME dft
`endif

`ifndef TB_NAME
 `define TB_NAME(d) d``_tb
`endif

//
// Pull in the stimulus and other info
//
`include "dft_stimulus.txt"
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
// ignore X in this test
`define APPLY_N_CHECK(x,j1,i1,i2,i3,i4,i5,j6,o1,o2,o3,o4,o5) \
  {j1,i1,i2,i3,i4,i5,j6,exp_``o1,exp_``o2,exp_``o3,exp_``o4,exp_``o5}=x; \
  exp_pat={exp_``o1,exp_``o2,exp_``o3,exp_``o4,exp_``o5}; \
  act_pat={o1,o2,o3,o4,o5}; \
  if (exp_pat!=act_pat) begin \
     $display("ERROR: miscompared at sample#%0d",i); \
     if (errCnt==0) $display("  PAT={%s,%s,%s,%s,%s}", `"o1`",`"o2`",`"o3`",`"o4`",`"o5`"); \
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
   reg [`DFT_OUTPUT_WIDTH-1:0]  exp_pat, act_pat;
   //
   // IOs
   //
   reg 		    clk=0;
   reg 		    reset=1;
   reg 		    next=0;
   reg [15:0] 	    X0=0,      X1=0,      X2=0,      X3=0;
   wire 	    next_out;
   wire [15:0] 	    Y0,      Y1,      Y2,      Y3;
   
   //
   // filler & expected output
   //
   reg 		    exp_next_out=0;
   reg [15:0] 	    exp_Y0=0,      exp_Y1=0,      exp_Y2=0,      exp_Y3=0;
   
   reg [2:0] 	   j1=0;
   reg [2:0] 	   j6=0;
   
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
   llki_discrete_if #(.core_id(`DFT_ID)) discrete();
   // LLKI master
   llki_discrete_master discreteMaster(.llki(discrete.master), .rst(reset), .*);
   //    
   //
   // DUT instantiation
   //
   `DUT_NAME #(.MY_STRUCT(DFT_LLKI_STRUCT)) dut(.llki(discrete.slave),.*);   
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
	 {next,X0,X1,X2,X3}=0;
	 //
	 reset = 1;
	 repeat (21) @(posedge clk); // MUST be 21 for this core because slow clock must align
	 @(negedge clk);      // in stimulus, rst de-asserted after negedge
	 #2 reset = 0;
	 @(negedge clk);            
      
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
	 {next,X0,X1,X2,X3}=0;
	 //
	 reset = 1;
	 repeat (21) @(posedge clk);
	 @(negedge clk);      // in stimulus, reset de-asserted after negedge
	 #2 reset = 0;
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
	 $display("Reading %d samples from buffer DFT_buffer",`DFT_SAMPLE_COUNT);
	 // now playback and check
	 for (i=0;i<`DFT_SAMPLE_COUNT;i++) begin
	    // the order MUST match the samples' order
	    `APPLY_N_CHECK(DFT_buffer[i],
			   j1,next,X0[15:0],X1[15:0],X2[15:0],X3[15:0],
			   j6,next_out,Y0[15:0],Y1[15:0],Y2[15:0],Y3[15:0]);

	    @(negedge clk); // next sample
	     // get out as soon found one error
	    if (errCnt && StopOnError) break;
	 end // for (int i=0;i<`DFT_SAMPLE_COUNT;i++)
      end
   endtask //   
   
endmodule // dft_tb

