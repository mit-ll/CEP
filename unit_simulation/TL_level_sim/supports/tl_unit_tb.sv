//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      tl_unit_tb.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    Unit level testbench with tilelink transaction Level wrapper for all cores.
// Notes: One testbench is used for all cores!!! See Makefile for core selection      
//
//************************************************************************

`timescale 1ns/1ns
//
// Name of the DUT (use AES as default)
//
`ifndef DUT_NAME
`define DUT_NAME  TLAES
`define CORE_NAME aes
`define CMD_FILE  "./sv_vectors/aes_playback.h"
`define VERBOSE  1
`endif
//
// Some derived macros
//
`define TB_NAME(d) d``_tb
`define MKSTR(x) `"x`"
`define GET_CMD_NAME(x)  x``_playback
`define GET_CMD_CNT(x)  `x``_totalCommands
`define GET_CMD_SIZE(x) `x``_size
`define GET_ADR_BASE(x) `x``_adrBase
`define GET_ADR_SIZE(x) `x``_adrSize
//
//
module `TB_NAME(`DUT_NAME) ; 

   //
   // Common TL IOs
   //
   reg         clock = 0; 
   reg         reset = 1; 
   //
   // These must match the IOs of the TL* module
   //
   wire         auto_control_xing_in_a_valid        ; 
   wire [2:0]   auto_control_xing_in_a_bits_opcode  ;
   wire [2:0]   auto_control_xing_in_a_bits_param   ;
   wire [1:0]   auto_control_xing_in_a_bits_size    ; 
   wire [7:0]   auto_control_xing_in_a_bits_source  ; 
   wire [30:0]  auto_control_xing_in_a_bits_address ; 
   wire [7:0]   auto_control_xing_in_a_bits_mask    ; 
   wire [63:0]  auto_control_xing_in_a_bits_data    ; 
   wire         auto_control_xing_in_a_bits_corrupt ; 
   wire         auto_control_xing_in_d_ready        ;
   wire 	auto_control_xing_in_a_ready;   
   wire 	auto_control_xing_in_d_valid; 
   wire [2:0] 	auto_control_xing_in_d_bits_opcode; 
   wire [1:0] 	auto_control_xing_in_d_bits_size; 
   wire [7:0] 	auto_control_xing_in_d_bits_source; 
   wire [63:0] 	auto_control_xing_in_d_bits_data;
   
   // not used
   reg [1:0] 	auto_control_xing_in_d_bits_param = 0;
   reg [3-1:0] 	auto_control_xing_in_d_bits_sink=0;
   reg 		auto_control_xing_in_d_bits_denied=0;
   reg 		auto_control_xing_in_d_bits_corrupt=0;
   //
   // Simple clock driving the DUT
   //
   initial begin
      forever #5 clock = !clock;
   end
`ifdef USE_MOCK_LLKI
 `include "../llki_supports/llki_rom.sv"
   //
   // LLKI supports
   //
   tlul_if tlulIf();
   //
   SRoT_mock #(.MASTER_ID('h88), .MY_STRUCT(`CORE_STRUCT_NAME)) srot(.tlul(tlulIf.master), .clk(clock), .rst(reset));
   //
   `DUT_NAME #(.MY_STRUCT(`CORE_STRUCT_NAME)) dut(.tlul(tlulIf.slave),.*);

`else
   //
   // DUT instantiation
   //
   `DUT_NAME dut(.*);
`endif
      
   //
   // TL master driver
   //
   tl_master_beh driver(.*);
   //
   // pull in the command sequences and task to play them back
   //
`include `CMD_FILE
`include "./supports/playback.sv"
   //
   // Main Task
   //
   int 	       errCnt = 0;
   //
   initial begin
      //
      // Reset
      //
      reset = 1;
      repeat (10) @(posedge clock);
      reset = 0;
      repeat (100) @(posedge clock);
      //
      // Do pre-playback stuffs here. For example: logic unlocking, etc..
      //
      //
`ifdef USE_MOCK_LLKI
      srot.unlockReq(errCnt);
      srot.clearKey(errCnt);
      // do the playback and verify that it breaks since we clear the key
      cep_playback(`GET_CMD_NAME(`CORE_NAME), 
		   `GET_ADR_BASE(`CORE_NAME) + `GET_ADR_SIZE(`CORE_NAME) , `GET_ADR_BASE(`CORE_NAME),
		   `GET_CMD_CNT(`CORE_NAME), `GET_CMD_SIZE(`CORE_NAME), `VERBOSE, errCnt);
      //
      if (errCnt) begin
	 $display("==== DUT=%s error count detected as expected due to logic lock... %0d errors ====",`MKSTR(`DUT_NAME),errCnt);
	 errCnt  = 0;
	 //
	 // need to pulse the reset since the core might stuck in some bad state
	 //
	 reset = 1;
	 repeat (10) @(posedge clock);
	 reset = 0;
	 repeat (100) @(posedge clock);
	 //
	 // unlock again
	 //
	 srot.unlockReq(errCnt);      
      end
      else begin
	 $display("==== DUT=%s  error=%0d?? Expect at least 1 ====",`MKSTR(`DUT_NAME),errCnt);
	 errCnt++; // fail
      end
      // now unlock again
`endif      
      
      //
      // Playback command sequences and verify
      //
      $display("=== Playing-back command sequences for DUT=%s (core=%s) ===",`MKSTR(`DUT_NAME),`MKSTR(`CORE_NAME));
      cep_playback(`GET_CMD_NAME(`CORE_NAME), 
		   `GET_ADR_BASE(`CORE_NAME) + `GET_ADR_SIZE(`CORE_NAME) , `GET_ADR_BASE(`CORE_NAME),
		   `GET_CMD_CNT(`CORE_NAME), `GET_CMD_SIZE(`CORE_NAME), `VERBOSE, errCnt);
      //
      // Check for pass/fail
      //
      $display("=== DUT=%s (core=%s) TEST %s (errCnt=%0d) ===",
	       `MKSTR(`DUT_NAME),`MKSTR(`CORE_NAME),
	       errCnt ? "FAILED" : "PASSED",errCnt);
      //
      // All done!
      //
      repeat (100) @(posedge clock);      
      $finish;
   end // initial begin
//   
endmodule //

