//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      md5_mock_tss.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    md5 wrapper with LLKI supports
// Notes:          
//************************************************************************
`timescale 1ns/1ns

//
// MD5 with LLKI wrapper
//
`include "llki_struct.h"
module md5_mock_tss #(parameter llki_s MY_STRUCT)
  (
   //
   // LLKI discrete's IOs
   //
   llki_discrete_if.slave llki,
   // core IO's
   input 	  clk, // input clock
   input 	  rst, // global rst
   input          init,  // clear internal states to start a new transaction   
   input [511:0]  msg_padded, // input message, already padded
   input 	  msg_in_valid, // next transaction with current encryption
   
   output [127:0] msg_output, // output message, always 128 bit wide
   output 	  msg_out_valid, // if asserted, output message is valid
   output 	  ready                    // the core is ready for an input message
   );

   //
   //
   //
`ifdef USE_MOCK_LLKI
   //
   // Use "mock" logic  until replace with real thing below
   //
   llki_discrete_slave #(.MY_STRUCT(MY_STRUCT)) discreteSlave(.*);
   //
   wire [127:0]    XOR_exp_key = {MY_STRUCT.key[0],MY_STRUCT.key[1]};
   wire [127:0]    XOR_act_key = {llki.rx_key_s.key[0],llki.rx_key_s.key[1]};
   //
   md5 core(.msg_padded(msg_padded ^ XOR_exp_key ^ XOR_act_key), .*);
`else
   //
   // MD5 with actual LLKI built in is here!!!! (defined in Makefile)
   //
   `CORE_WITH_LLKI core(.*);
   //
`endif // !`ifdef USE_MOCK_LLKI
   
endmodule // MD5_llki
