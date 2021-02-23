//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
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
`ifndef DUT_NAME
 `define DUT_NAME des3
`endif
//
// Some derived macros
//
`ifndef TB_NAME
 `define TB_NAME(d) d``_tb
`endif

//
// Stimulus/ExpectedData info
//
`include   "des3_stimulus.txt"

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
  if (exp_pat!==act_pat) begin \
     $display("ERROR: miscompared at sample#%0d",i); \
     if (errCnt==0) $display("  PAT={%s,%s}", `"o1`",`"o2`"); \
     $display("  EXP=0x%x",exp_pat); \
     $display("  ACT=0x%x",act_pat); \
     errCnt++;\
  end

//
module `TB_NAME ;
   //
   //
   //
   string dut_name_list [] = '{`MKSTR(`DUT_NAME)};
   reg [`DES3_OUTPUT_WIDTH-1:0]  exp_pat, act_pat;
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
   // include LLKI
   //
`ifdef LLKI_EN
 `include "../llki_supports/llki_rom.sv"
   //
   // LLKI supports
   //
   llki_discrete_if #(.core_id(`DES3_ID)) discrete();
   // LLKI master
   llki_discrete_master discreteMaster(.llki(discrete.master), .rst(reset), .*);
   //    
   //
   // DUT instantiation
   //
   `DUT_NAME #(.MY_STRUCT(DES3_LLKI_STRUCT)) dut(.llki(discrete.slave),.*);   
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
      repeat (100) @(negedge clk);  // must wait this long for output to stablize          
      //
      // do the unlocking or whatever here
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
	 {start,
	  desIn,
	  key1,
	  key2,
	  key3,
	  decrypt}=0;
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
	 $display("Reading %d samples from buffer DES3_buffer",`DES3_SAMPLE_COUNT);
	 // now playback and check
	 for (i=0;i<`DES3_SAMPLE_COUNT;i++) begin
	    // the order MUST match the samples' order
	    `APPLY_N_CHECK(DES3_buffer[i],
			   j1,start,
			   j2,decrypt,
			   desIn[63:0],
			   key1[55:0],
			   key2[55:0],
			   key3[55:0],
			   j7,out_valid,
			   desOut[63:0]);
	    @(negedge clk); // next sample
	    // get out as soon found one error
	    if (errCnt && StopOnError) break;
	 end // for (int i=0;i<`MAX_SAMPLES;i++)
      end
   endtask //   
   
endmodule // des3_tb

