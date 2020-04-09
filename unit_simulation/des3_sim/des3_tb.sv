//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      des3_tb.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    DES3 Core unit testbench
// Notes:        
//
//************************************************************************

`timescale 1ns/1ns
//
// Name of the DUT: des3 
//
`define DUT_NAME des3
//
// Stimulus/ExpectedData info
//
`define MAX_SAMPLES      (2554-17)
`define SAMPLE_WIDTH     (4+4+64+(3*56)+4+64)
`define OUTPUT_WIDTH     (1+64)
`define DATA_FILE        "DES3_stimulus.csv"
//
// Some derived macros
//
`define TB_NAME(d) d``_tb
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
`define APPLY_N_CHECK(x,j1,i1,j2,i2,i3,i4,i5,i6,j7,o1,o2) \
  {j1,i1,j2,i2,i3,i4,i5,i6,j7,exp_``o1,exp_``o2}=x; \
  exp_pat={exp_``o1,exp_``o2}; \
  act_pat={o1,o2}; \
  if (exp_pat!=act_pat) begin \
     $display("ERROR: miscompared at sample#%0d",i); \
     if (errCnt==0) $display("  PAT={%s,%s}", `"o1`",`"o2`"); \
     $display("  EXP=0x%x",exp_pat); \
     $display("  ACT=0x%x",act_pat); \
     errCnt++;\
  end

//
module `TB_NAME(`DUT_NAME) ; 
   //
   //
   //
   string dut_name_list [] = '{`MKSTR(`DUT_NAME)};
   reg [`SAMPLE_WIDTH-1:0] buffer[`MAX_SAMPLES-1:0];
   reg [`OUTPUT_WIDTH-1:0]  exp_pat, act_pat;
   //
   // IOs
   //
   reg 			    clk = 0;
   reg 			    reset = 1;   
   reg 			    start=0;
   reg [63:0] 		    desIn=0;
   reg [55:0] 		    key1=0;
   reg [55:0] 		    key2=0;
   reg [55:0] 		    key3=0;
   reg 			    decrypt=0;
   wire [63:0] 		    desOut;
   wire 		    out_valid;
   //
   // filler & expected output
   //
   reg [63:0] 		    exp_desOut=0;
   reg 			    exp_out_valid=0;

   reg [2:0] 		    j1=0;
   reg [2:0] 		    j2=0;
   reg [2:0] 		    j7=0;
   
   //
   int 		errCnt=0;
   //
   // Simple clock driving the DUT
   //
   initial begin
      forever #5 clk = !clk;
   end
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
      // do the unlocking or whatever here
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
      int fp;
      int i;
      event err;
      begin
	 //
	 // Pulse the DUT's reset & drive input to zeros (known states)
	 //
	 {start,
	  desIn,
	  key1,
	  key2,
	  key3,
	  decrypt}=0;
	 //
	 reset = 1;
	 repeat (21) @(posedge clk); // MUST be 21 for this core because slow clock must align
	 @(negedge clk);      // in stimulus, reset de-asserted after negedge
	 #2 reset = 0;
	 @(negedge clk);            
	 //
	 // open file for checking
	 //
	 $display("Reading %d samples from file %s",`MAX_SAMPLES,`DATA_FILE);
	 $readmemh(`DATA_FILE, buffer);
	 // now playback and check
	 for (i=0;i<`MAX_SAMPLES;i++) begin
	    // the order MUST match the samples' order
	    `APPLY_N_CHECK(buffer[i],
			   j1,start,
			   j2,decrypt,
			   desIn[63:0],
			   key1[55:0],
			   key2[55:0],
			   key3[55:0],
			   j7,out_valid,
			   desOut[63:0]);
	    @(negedge clk); // next sample	       
	 end // for (int i=0;i<`MAX_SAMPLES;i++)
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
   
endmodule // des3_tb

