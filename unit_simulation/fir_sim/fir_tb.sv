//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      fir_tb.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    FIR Core unit testbench
// Notes:        
//
//************************************************************************

`timescale 1ns/1ns

//
// Name of the DUT & TB if not pass in from Make
//
`ifndef DUT_NAME
 `define DUT_NAME fir
`endif

`ifndef TB_NAME
 `define TB_NAME(d) d``_tb
`endif

//
// Pull in the stimulus and other info
//
`include "fir_stimulus.txt"
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
`define APPLY_N_CHECK(x,i2,o1) \
  {i2,exp_``o1}=x; \
  exp_pat={exp_``o1}; \
  act_pat={o1}; \
  if (exp_pat!=act_pat) begin \
     $display("ERROR: miscompared at sample#%0d",i); \
     if (errCnt==0) $display("  PAT={%s}", `"o1`"); \
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
   reg [`FIR_OUTPUT_WIDTH-1:0]  exp_pat, act_pat;
   //
   // IOs
   //
   reg 			    clk=0;                      // reg clock
   reg 			    reset=0;                    // active low
   reg [31:0] 		    inData;
   wire [31:0] 		    outData;
   

   //
   // filler & expected output
   //
   reg [31:0] 		    exp_outData;   

   reg [2:0] 		    j1;
   
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
   llki_discrete_if #(.core_id(`FIR_ID)) discrete();
   // LLKI master
   llki_discrete_master discreteMaster(.llki(discrete.master), .rst(reset), .*);
   //    
   //
   // DUT instantiation
   //
   `DUT_NAME #(.MY_STRUCT(FIR_LLKI_STRUCT)) dut(.llki(discrete.slave),.*);   
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
      inData = 0;
      //
      reset = 0;
      repeat (5) @(posedge clk);
      @(negedge clk);      // in stimulus, rst de-asserted after negedge
      #2 reset = 1;
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
	 inData = 0;	 
	 // active low
	 reset = 0;
	 repeat (21) @(posedge clk);
	 @(negedge clk);      // in stimulus, reset de-asserted after negedge
	 #2 reset = 1;
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
	 $display("Reading %d samples from buffer FIR_buffer",`FIR_SAMPLE_COUNT);
	 // now playback and check
	 for (i=0;i<`FIR_SAMPLE_COUNT;i++) begin
	    // the order MUST match the samples' order
	    `APPLY_N_CHECK(FIR_buffer[i],inData[31:0],outData[31:0]);
	    @(negedge clk); // next sample
	     // get out as soon found one error
	    if (errCnt && StopOnError) break;
	 end // for (int i=0;i<`FIR_SAMPLE_COUNT;i++)
      end
   endtask //   
   
endmodule // fir_tb

