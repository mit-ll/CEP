//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
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
`define DUT_NAME  aesTLModule
`define CORE_NAME aes
`define CMD_FILE  "./sv_vectors/aes_playback.h"
`define VERBOSE  1
`endif

// SRoT: same for every one
`define SROT_FILE  "./sv_vectors/srot_playback.h"
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
   // MUST be the same for all cores
   //
   reg         clock = 0;
   reg         reset = 1;
   reg 	       llkiOn=0; // MUX to share same tl_master_beh
   initial begin
      forever #5 clock = !clock;
   end
   //
   // **************************************
   // TL master driver
   // **************************************   
   //
   //
   wire        a_ready; // I
   wire        a_valid; 
   wire [2:0]  a_bits_opcode; 
   wire [2:0]  a_bits_param; 
   wire [1:0]  a_bits_size; 
   wire [7:0]  a_bits_source; 
   wire [30:0] a_bits_address; 
   wire [7:0]  a_bits_mask; 
   wire [63:0] a_bits_data; 
   wire        a_bits_corrupt; 
   // Channel D
   wire        d_ready; 
   wire        d_valid; // I
   wire [2:0]  d_bits_opcode;  // I
   wire [1:0]  d_bits_param;  // I
   wire [1:0]  d_bits_size;  // I
   wire [7:0]  d_bits_source; // I
   wire [63:0] d_bits_data; // I
   wire [3-1:0] d_bits_sink; // I
   wire 	d_bits_denied; // I
   wire 	d_bits_corrupt; // I
   //
   tl_master_beh driver(.*);
   
   //
   // **************************************
   // DUT
   // **************************************   
   //
   // core signals
   wire         auto_slave_in_a_valid;
   wire [2:0] 	auto_slave_in_a_bits_opcode;
   wire [1:0] 	auto_slave_in_a_bits_size;
   wire [8:0] 	auto_slave_in_a_bits_source;
   wire [30:0] 	auto_slave_in_a_bits_address;
   wire [7:0] 	auto_slave_in_a_bits_mask;
   wire [63:0] 	auto_slave_in_a_bits_data;
   wire         auto_slave_in_d_ready;
   
   wire 	auto_slave_in_a_ready;
   wire 	auto_slave_in_d_valid;
   wire [2:0] 	auto_slave_in_d_bits_opcode;
   wire [1:0] 	auto_slave_in_d_bits_size;
   wire [8:0] 	auto_slave_in_d_bits_source;
   wire [63:0] 	auto_slave_in_d_bits_data;
   
   // LLKI
   wire 	auto_llki_in_a_ready;
   wire 	auto_llki_in_a_valid;
   wire [2:0] 	auto_llki_in_a_bits_opcode;
   wire [2:0] 	auto_llki_in_a_bits_param;
   wire [2:0] 	auto_llki_in_a_bits_size;
   wire [3:0] 	auto_llki_in_a_bits_source;
   wire [30:0] 	auto_llki_in_a_bits_address;
   wire [7:0] 	auto_llki_in_a_bits_mask;
   wire [63:0] 	auto_llki_in_a_bits_data;
   wire 	auto_llki_in_a_bits_corrupt;
   wire 	auto_llki_in_d_ready;
   
   wire 	auto_llki_in_d_valid;
   wire [2:0] 	auto_llki_in_d_bits_opcode;
   wire [1:0] 	auto_llki_in_d_bits_param;
   wire [2:0] 	auto_llki_in_d_bits_size;
   wire [3:0] 	auto_llki_in_d_bits_source;
   wire 	auto_llki_in_d_bits_sink;
   wire 	auto_llki_in_d_bits_denied;
   wire [63:0] 	auto_llki_in_d_bits_data;
   wire 	auto_llki_in_d_bits_corrupt;

   // NA

   `DUT_NAME dut(.*);

   //
   // MUX!!
   //
   // to driver
   assign #1 a_ready		= llkiOn ? auto_llki_in_a_ready	       : auto_slave_in_a_ready	; // I
   assign #1 d_valid		= llkiOn ? auto_llki_in_d_valid	       : auto_slave_in_d_valid	; // I
   assign #1 d_bits_opcode		= llkiOn ? auto_llki_in_d_bits_opcode  : auto_slave_in_d_bits_opcode;  // I
   assign #1 d_bits_param		= llkiOn ? auto_llki_in_d_bits_param   : 0;
   assign #1 d_bits_size		= llkiOn ? auto_llki_in_d_bits_size    : auto_slave_in_d_bits_size	;  // I
   assign #1 d_bits_source		= llkiOn ? auto_llki_in_d_bits_source  : auto_slave_in_d_bits_source; // I
   assign #1 d_bits_data		= llkiOn ? auto_llki_in_d_bits_data    : auto_slave_in_d_bits_data	; // I
   assign #1 d_bits_sink		= llkiOn ? auto_llki_in_d_bits_sink    : 0; 
   assign #1 d_bits_denied		= llkiOn ? auto_llki_in_d_bits_denied  : 0;
   assign #1 d_bits_corrupt	= llkiOn ? auto_llki_in_d_bits_corrupt : 0;
   // to LLKI
   assign #1 auto_llki_in_a_valid		= llkiOn ? a_valid		: 0;
   assign #1 auto_llki_in_a_bits_opcode	= llkiOn ? a_bits_opcode	: 0;
   assign #1 auto_llki_in_a_bits_param	= llkiOn ? a_bits_param		: 0;
   assign #1 auto_llki_in_a_bits_size	= llkiOn ? a_bits_size		: 0;
   assign #1 auto_llki_in_a_bits_source	= llkiOn ? a_bits_source	: 0;
   assign #1 auto_llki_in_a_bits_address	= llkiOn ? a_bits_address	: 0;
   assign #1 auto_llki_in_a_bits_mask	= llkiOn ? a_bits_mask		: 0;
   assign #1 auto_llki_in_a_bits_data	= llkiOn ? a_bits_data		: 0;
   assign #1 auto_llki_in_a_bits_corrupt	= llkiOn ? a_bits_corrupt	: 0;
   assign #1 auto_llki_in_d_ready		= llkiOn ? d_ready		: 1;
   // to core
   assign #1 auto_slave_in_a_valid		= !llkiOn ? a_valid		: 0;
   assign #1 auto_slave_in_a_bits_opcode	= !llkiOn ? a_bits_opcode	: 0;
   assign #1 auto_slave_in_a_bits_param	= !llkiOn ? a_bits_param	: 0;
   assign #1 auto_slave_in_a_bits_size	= !llkiOn ? a_bits_size		: 0;
   assign #1 auto_slave_in_a_bits_source	= !llkiOn ? a_bits_source	: 0;
   assign #1 auto_slave_in_a_bits_address	= !llkiOn ? a_bits_address	: 0;
   assign #1 auto_slave_in_a_bits_mask	= !llkiOn ? a_bits_mask		: 0;
   assign #1 auto_slave_in_a_bits_data	= !llkiOn ? a_bits_data		: 0;
   assign #1 auto_slave_in_a_bits_corrupt	= !llkiOn ? a_bits_corrupt	: 0;
   assign #1 auto_slave_in_d_ready		= !llkiOn ? d_ready		: 1;
   
   //
   // pull in the command sequences and task to play them back
   //
`include `SROT_FILE   
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
      repeat (1000) @(posedge clock);
      //
      // Access while core is locked => expect error
      //
      llkiOn = 0;
      @(posedge clock);      
      $display("=== Playing-back command sequences while core is locked for DUT=%s (core=%s) ===",`MKSTR(`DUT_NAME),`MKSTR(`CORE_NAME));
      cep_playback(`GET_CMD_NAME(`CORE_NAME), 
		   `GET_ADR_BASE(`CORE_NAME) + `GET_ADR_SIZE(`CORE_NAME) , `GET_ADR_BASE(`CORE_NAME),
		   `GET_CMD_CNT(`CORE_NAME), `GET_CMD_SIZE(`CORE_NAME), `VERBOSE, errCnt);
      if (errCnt == 0) begin
	 $display("NO ERROR while locked core is accessed?? Bad bad!!!\n");
	 errCnt++;
      end
      else begin
	 $display("ERROR detected as expected while locked core is accessed!!!\n");
	 errCnt=0;
      end
      //
      // Unlock LLKI
      //
      //
      if (!errCnt) begin
	 llkiOn = 1;
	 @(posedge clock);
	 $display("=== Playing-back unlocking LLKI sequences for DUT=%s (core=%s) ===",`MKSTR(`DUT_NAME),`MKSTR(`CORE_NAME));
	 cep_playback(srot_playback,
		      `GET_ADR_BASE(`CORE_NAME) + 'h9000 ,
		      `GET_ADR_BASE(`CORE_NAME) + 'h8000 ,		   
		      `srot_totalCommands, `srot_totalCommands, `VERBOSE, errCnt);
	 llkiOn = 0;
	 @(posedge clock);
	 //
	 // Playback command sequences and verify
	 //
	 $display("=== Playing-back command sequences for DUT=%s (core=%s) ===",`MKSTR(`DUT_NAME),`MKSTR(`CORE_NAME));
	 cep_playback(`GET_CMD_NAME(`CORE_NAME), 
		      `GET_ADR_BASE(`CORE_NAME) + `GET_ADR_SIZE(`CORE_NAME) , `GET_ADR_BASE(`CORE_NAME),
		      `GET_CMD_CNT(`CORE_NAME), `GET_CMD_SIZE(`CORE_NAME), `VERBOSE, errCnt);
      end
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

