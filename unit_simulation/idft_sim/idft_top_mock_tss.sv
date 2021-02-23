//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      idft_top_mock_tss.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    idft_top wrapper with LLKI supports
// Notes:          
//************************************************************************
`timescale 1ns/1ns


//
// IDFT with LLKI wrapper
//
`include "llki_struct.h"
module idft_top_mock_tss #(parameter llki_s MY_STRUCT)
  (
   //
   // LLKI discrete's IOs
   //
   llki_discrete_if.slave llki,
   // core IO's
   output next_out,
   input clk, reset, next,
   
   input [15:0] X0,
      X1,
      X2,
      X3,

   output [15:0] Y0,
      Y1,
      Y2,
      Y3
   );

   //
   //
   //
`ifdef USE_MOCK_LLKI
   //
   // Use "mock" logic  until replace with real thing below
   //
   llki_discrete_slave #(.MY_STRUCT(MY_STRUCT)) discreteSlave(.rst(reset),.*);
   //
   wire [127:0]    XOR_exp_key = {MY_STRUCT.key[0],MY_STRUCT.key[1]};
   wire [127:0]    XOR_act_key = {llki.rx_key_s.key[0],llki.rx_key_s.key[1]};
   idft_top core(.X0(X0 ^ XOR_exp_key ^ XOR_act_key), .*);
`else
   //
   // IDFT with actual LLKI built in is here!!!! (defined in Makefile)
   //
   `CORE_WITH_LLKI core(.*);
   //
`endif // !`ifdef USE_MOCK_LLKI
   
endmodule // IDFT_llki
