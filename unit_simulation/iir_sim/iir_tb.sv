//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
//
// File Name:      iir_tb.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    IIR Core unit testbench
// Notes:        
//
//************************************************************************

`timescale 1ns/1ns

//
// Name of the DUT & TB if not pass in from Make
//
`ifndef DUT_NAME
 `define DUT_NAME iir
`endif

`ifndef TB_NAME
 `define TB_NAME(d) d``_tb
`endif

//
// Pull in the stimulus and other info
//
`include "iir_stimulus.txt"
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
// ONLY for IIR i1 = rst_dut : if =1 then skip the check since it is very hard to align to the exact even cycle
`define APPLY_N_CHECK(x,l1o,lr,lc,la,l1i,lkv,lck,ld,j1,i1,i2,o1) \
  {l1o,elr,elc,ela,l1i,lkv,lck,ld, \
   j1,i1,i2,exp_``o1}=x; \
  exp_pat={elr,elc,ela,exp_``o1}; \
  act_pat={ lr, lc, la,o1}; \
  if ((exp_pat!=act_pat) && (i1 == 0)) begin \
     $display("ERROR: miscompared at sample#%0d rst_dut=%d",i,i1); \
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
   reg [`IIR_OUTPUT_WIDTH-1:0]  exp_pat, act_pat;
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
   reg 			    clk=0;                      // reg clock
   reg 			    rst=1;                    // active hi now
   reg 			    rst_dut=1;
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
   `DUT_NAME dut(.*);
   //
   // -------------------
   // Test starts here
   // -------------------   
   //
   initial begin
      //
      // Pulse the DUT's rst & drive input to zeros (known states)
      //
      rst_dut = 1;
      {llkid_key_valid,llkid_clear_key,llkid_key_data,inData} = 0;
      //
      rst = 1;
      repeat (5) @(posedge clk);
      @(negedge clk);      // in stimulus, rst de-asserted after negedge
      #2 rst = 0;
      @(negedge clk);            
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
	 $display("Reading %d samples from buffer IIR_buffer",`IIR_SAMPLE_COUNT);
	 // now playback and check
	 for (i=0;i<`IIR_SAMPLE_COUNT;i++) begin
	    // the order MUST match the samples' order
	    `APPLY_N_CHECK(IIR_buffer[i],
			   l1o,llkid_key_ready,llkid_key_complete,llkid_clear_key_ack,
			   l1i,llkid_key_valid,llkid_clear_key,
			   llkid_key_data,
			   j1,rst_dut,
			   inData[31:0],outData[31:0]);
	    @(negedge clk); // next sample
	     // get out as soon found one error
	    if (errCnt && StopOnError) break;
	 end // for (int i=0;i<`IIR_SAMPLE_COUNT;i++)
      end
   endtask //   
   
endmodule // iir_tb

