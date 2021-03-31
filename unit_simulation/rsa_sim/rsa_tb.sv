//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
//
// File Name:      rsa_tb.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    RSA Core unit testbench
// Notes:        
//
//************************************************************************

`timescale 1ns/1ns

//
// Name of the DUT & TB if not pass in from Make
//
`ifndef DUT_NAME
 `define DUT_NAME rsa
`endif

`ifndef TB_NAME
 `define TB_NAME(d) d``_tb
`endif

//
// Pull in the stimulus and other info
//
`include "rsa_stimulus.txt"
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
`define APPLY_N_CHECK(x,l1o,lr,lc,la,l1i,lkv,lck,ld,ji1,i1,ji2,i2,ji3,i3,ji4,i4,ji5,i5,ji6,i6,ji7,i7,ji8,i8,ji9,i9,ji10,i10,ji11,i11,ji12,i12,i13,ji14,i14,i15,i16,i17,jo1,o1,o2,o3,o4,o5,o6) \
{l1o,elr,elc,ela,l1i,lkv,lck,ld, \
 ji1,i1,ji2,i2,ji3,i3,ji4,i4,ji5,i5,ji6,i6,ji7,i7,ji8,i8,ji9,i9,ji10,i10,ji11,i11,ji12,i12,i13,ji14,i14,i15,i16,i17,jo1,exp_``o1,exp_``o2,exp_``o3,exp_``o4,exp_``o5,exp_``o6} =x;\
  exp_pat={elr,elc,ela,exp_``o1,exp_``o2,exp_``o3,exp_``o4,exp_``o5,exp_``o6};\
  act_pat={ lr, lc, la,o1,o2,o3,o4,o5,o6}; \
  if (exp_pat!=act_pat) begin \
     $display("ERROR: miscompared at sample#%0d",i); \
     if (errCnt==0) $display("  PAT={%s,%s,%s,%s,%s,%s}", `"o1`",`"o2`",`"o3`",`"o4`",`"o5`",`"o6`"); \
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
   reg [`RSA_OUTPUT_WIDTH-1:0]  exp_pat, act_pat;
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
   reg 			    clk=0;
   reg 			    rst=1;
   reg 			    start=0;
   reg [12 : 0] 	    exponent_length=0;
   reg [07 : 0] 	    modulus_length=0;
   reg 			    exponent_mem_api_cs=0;
   reg 			    exponent_mem_api_wr=0;
   reg 			    exponent_mem_api_rst=0;
   reg [31 : 0] 	    exponent_mem_api_write_data=0;
   reg 			    modulus_mem_api_cs=0;
   reg 			    modulus_mem_api_wr=0;
   reg 			    modulus_mem_api_rst=0;
   reg [31 : 0] 	    modulus_mem_api_write_data=0;
   reg 			    message_mem_api_cs=0;
   reg 			    message_mem_api_wr=0;
   reg 			    message_mem_api_rst=0;
   reg [31 : 0] 	    message_mem_api_write_data=0;
   reg 			    result_mem_api_cs=0;
   reg 			    result_mem_api_rst=0;
   wire 		    ready;
   wire [63 : 0] 	    cycles;
   wire [31 : 0] 	    exponent_mem_api_read_data;
   wire [31 : 0] 	    modulus_mem_api_read_data;
   wire [31 : 0] 	    message_mem_api_read_data;   
   wire [31 : 0] 	    result_mem_api_read_data;
   //
   // filler & expected output
   //
   reg 			    exp_ready=0;
   reg [63 : 0] 	    exp_cycles=0;
   reg [31 : 0] 	    exp_exponent_mem_api_read_data=0;
   reg [31 : 0] 	    exp_modulus_mem_api_read_data=0;
   reg [31 : 0] 	    exp_message_mem_api_read_data=0;
   reg [31 : 0] 	    exp_result_mem_api_read_data=0;

   reg [2:0] 		    ji1=0;
   reg [2:0] 		    ji2=0;
   reg [2:0] 		    ji3=0;
   reg [2:0] 		    ji4=0;
   reg [2:0] 		    ji5=0;
   reg [2:0] 		    ji6=0;
   reg [2:0] 		    ji7=0;
   reg [2:0] 		    ji8=0;
   reg [2:0] 		    ji9=0;
   reg [2:0] 		    ji10=0;
   reg [2:0] 		    ji11=0;
   reg [2:0] 		    ji12=0;
   reg [2:0] 		    ji14=0;   
   reg [2:0] 		    jo1=0;
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
      // Pulse the DUT's reset & drive input to zeros (known states)
      //
      {llkid_key_valid,llkid_clear_key,llkid_key_data,
       start,
       exponent_length,
       modulus_length,
       exponent_mem_api_cs,
       exponent_mem_api_wr,
       exponent_mem_api_rst,
       exponent_mem_api_write_data,
       modulus_mem_api_cs,
       modulus_mem_api_wr,
       modulus_mem_api_rst,
       modulus_mem_api_write_data,
       message_mem_api_cs,
       message_mem_api_wr,
       message_mem_api_rst,
       message_mem_api_write_data,
       result_mem_api_cs,
       result_mem_api_rst} = 0;
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
	 $display("Reading %d samples from buffer RSA_buffer",`RSA_SAMPLE_COUNT);
	 // now playback and check
	 for (i=0;i<`RSA_SAMPLE_COUNT;i++) begin
	    // the order MUST match the samples' order
	    `APPLY_N_CHECK(RSA_buffer[i],
			   l1o,llkid_key_ready,llkid_key_complete,llkid_clear_key_ack,
			   l1i,llkid_key_valid,llkid_clear_key,
			   llkid_key_data,
			   ji1,exponent_mem_api_cs,
			   ji2,exponent_mem_api_rst,
			   ji3,exponent_mem_api_wr,
			   ji4,message_mem_api_cs,
			   ji5,message_mem_api_rst,
			   ji6,message_mem_api_wr,
			   ji7,modulus_mem_api_cs,
			   ji8,modulus_mem_api_rst,
			   ji9,modulus_mem_api_wr,
			   ji10,result_mem_api_cs,
			   ji11,result_mem_api_rst,
			   ji12,start,
			   modulus_length[7:0],	 
			   ji14,exponent_length[12:0],
			   exponent_mem_api_write_data[31:0],
			   message_mem_api_write_data[31:0],
			   modulus_mem_api_write_data[31:0],
			   jo1,ready,	 	 
			   cycles[63:0],
			   exponent_mem_api_read_data[31:0],
			   message_mem_api_read_data[31:0],
			   modulus_mem_api_read_data[31:0],
			   result_mem_api_read_data[31:0]);

	    @(negedge clk); // next sample
	    // 	// 	       // get out as soon found one error
	    if (errCnt && StopOnError) break;       
	 end // for (int i=0;i<`RSA_SAMPLE_COUNT;i++)
      end
   endtask //   
   
endmodule // rsa_tb

