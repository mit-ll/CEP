//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      FIR_filter_mock_tss.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    FIR_filter wrapper with LLKI supports
// Notes:          
//************************************************************************
`timescale 1ns/1ns


//
// FIR with LLKI wrapper
//
`include "llki_struct.h"
module FIR_filter_mock_tss #(parameter llki_s MY_STRUCT)
  (
   //
   // LLKI discrete's IOs
   //
   llki_discrete_if.slave llki,
   // core IO's
  input   [31:0] inData,
  input    clk,
  output  [31:0] outData,
  input    reset
   );

   //
   //
   //
`ifdef USE_MOCK_LLKI
   //
   // Use "mock" logic  until replace with real thing below
   //
   // active low
   llki_discrete_slave #(.MY_STRUCT(MY_STRUCT)) discreteSlave(.rst(!reset),.*);
   //
   wire [127:0]    XOR_exp_key = {MY_STRUCT.key[0],MY_STRUCT.key[1]};
   wire [127:0]    XOR_act_key = {llki.rx_key_s.key[0],llki.rx_key_s.key[1]};
   //
   FIR_filter core(.inData(inData ^ XOR_exp_key ^ XOR_act_key), .*);
`else
   //
   // FIR with actual LLKI built in is here!!!! (defined in Makefile)
   //
   `CORE_WITH_LLKI core(.*);
   //
`endif // !`ifdef USE_MOCK_LLKI
   
endmodule // FIR_llki
