//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      rsa_tb.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    RSA Core unit testbench
// Notes:        
//
//************************************************************************

`timescale 1ns/1ns
//
// Name of the DUT = rsa = modexp_core
//
`define DUT_NAME modexp_core
//
// Stimulus/ExpectedData info
//
`define MAX_SAMPLES      (96653-31)
`define SAMPLE_WIDTH     ((12*4)+(2*8)+(32*3)+4+64+(4*32))
`define OUTPUT_WIDTH     (1+64+(4*32))
`define DATA_FILE        "RSA_stimulus.csv"
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
`define APPLY_N_CHECK(x,ji1,i1,ji2,i2,ji3,i3,ji4,i4,ji5,i5,ji6,i6,ji7,i7,ji8,i8,ji9,i9,ji10,i10,ji11,i11,ji12,i12,i13,i14,i15,i16,i17,jo1,o1,o2,o3,o4,o5,o6) \
{ji1,i1,ji2,i2,ji3,i3,ji4,i4,ji5,i5,ji6,i6,ji7,i7,ji8,i8,ji9,i9,ji10,i10,ji11,i11,ji12,i12,i13,i14,i15,i16,i17,jo1,exp_``o1,exp_``o2,exp_``o3,exp_``o4,exp_``o5,exp_``o6} =x;\
  exp_pat={exp_``o1,exp_``o2,exp_``o3,exp_``o4,exp_``o5,exp_``o6};\
  act_pat={o1,o2,o3,o4,o5,o6}; \
  if (exp_pat!=act_pat) begin \
     $display("ERROR: miscompared at sample#%0d",i); \
     if (errCnt==0) $display("  PAT={%s,%s,%s,%s,%s,%s}", `"o1`",`"o2`",`"o3`",`"o4`",`"o5`",`"o6`"); \
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
   reg 			    clk=0;
   reg 			    reset_n=0;
   reg 			    start=0;
   reg [07 : 0] 	    exponent_length=0;
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
	 reset_n = 0;
	 repeat (5) @(posedge clk);
	 @(negedge clk);      // in stimulus, reset_n de-asserted after negedge
	 #2 reset_n = 1;
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
			   exponent_length[7:0],
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
   
endmodule // rsa_tb

