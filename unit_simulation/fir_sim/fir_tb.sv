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
   //
   // DUT instantiation
   //
   `DUT_NAME u1(.*);
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
      
      //
      // pulse the DUT's reset and playback
      //
      playback_data();
      $finish;
   end
   //
   // Read data from file into buffer and playback for compare
   //
   task playback_data;
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
	 end // for (int i=0;i<`FIR_SAMPLE_COUNT;i++)
	 //
	 // print summary
	 //
	 if (errCnt) begin
	    $display("==== DUT=%s TEST FAILED with %0d errors ====",dut_name_list[0],errCnt);
	 end
	 else begin
	    $display("==== DUT=%s TEST PASSED  ====",dut_name_list[0]);
	 end
      end
   endtask //   
   
endmodule // fir_tb

