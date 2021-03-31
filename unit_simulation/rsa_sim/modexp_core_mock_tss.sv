//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      modexp_core_mock_tss.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    modexp_core wrapper with LLKI supports
// Notes:          
//************************************************************************
`timescale 1ns/1ns


//
// RSA with LLKI wrapper
//
`include "llki_struct.h"
module modexp_core_mock_tss #(parameter llki_s MY_STRUCT)
  (
   //
   // LLKI discrete's IOs
   //
   llki_discrete_if.slave llki,
   // core IO's
   input wire 		clk,
   input wire 		reset_n,
   
   input wire 		start,
   output wire 		ready,
   
   //           input wire [07 : 0]  exponent_length,
   // tony duong 05/27/20: change from unit of 32-bits to actual bits to speed up
   input wire [12 : 0] 	exponent_length, 
   input wire [07 : 0] 	modulus_length,
   
   output wire [63 : 0] cycles,
   
   input wire 		exponent_mem_api_cs,
   input wire 		exponent_mem_api_wr,
   input wire 		exponent_mem_api_rst,
   input wire [31 : 0] 	exponent_mem_api_write_data,
   output wire [31 : 0] exponent_mem_api_read_data,
   
   input wire 		modulus_mem_api_cs,
   input wire 		modulus_mem_api_wr,
   input wire 		modulus_mem_api_rst,
   input wire [31 : 0] 	modulus_mem_api_write_data,
   output wire [31 : 0] modulus_mem_api_read_data,
   
   input wire 		message_mem_api_cs,
   input wire 		message_mem_api_wr,
   input wire 		message_mem_api_rst,
   input wire [31 : 0] 	message_mem_api_write_data,
   output wire [31 : 0] message_mem_api_read_data,
   
   input wire 		result_mem_api_cs,
   input wire 		result_mem_api_rst,
   output wire [31 : 0] result_mem_api_read_data

   );

   //
   //
   //
`ifdef USE_MOCK_LLKI
   //
   // Use "mock" logic  until replace with real thing below
   //
   llki_discrete_slave #(.MY_STRUCT(MY_STRUCT)) discreteSlave(.rst(!reset_n), .*);
   //
   wire [127:0]    XOR_exp_key = {MY_STRUCT.key[0],MY_STRUCT.key[1]};
   wire [127:0]    XOR_act_key = {llki.rx_key_s.key[0],llki.rx_key_s.key[1]};
   modexp_core core(.message_mem_api_write_data(message_mem_api_write_data ^ XOR_exp_key ^ XOR_act_key), .*);
`else
   //
   // RSA with actual LLKI built in is here!!!! (defined in Makefile)
   //
   `CORE_WITH_LLKI core(.*);
   //
`endif // !`ifdef USE_MOCK_LLKI
   
endmodule // RSA_llki
