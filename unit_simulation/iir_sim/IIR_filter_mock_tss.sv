//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      IIR_filter_mock_tss.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    IIR_filter wrapper with LLKI supports
// Notes:          
//************************************************************************
`timescale 1ns/1ns


//
// IIR with LLKI wrapper
//
`include "llki_struct.h"
module IIR_filter_mock_tss #(parameter llki_s MY_STRUCT)
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
   // this module need reset for every transaction, so we need only reset the LLKI once
   reg 	    resetFlag = 1;
   always @(negedge reset) resetFlag = 0;
   // active low
   llki_discrete_slave #(.MY_STRUCT(MY_STRUCT)) discreteSlave(.rst(resetFlag),.*);
   //
   wire [127:0]    XOR_exp_key = {MY_STRUCT.key[0],MY_STRUCT.key[1]};
   wire [127:0]    XOR_act_key = {llki.rx_key_s.key[0],llki.rx_key_s.key[1]};
   IIR_filter core(.inData(inData ^ XOR_exp_key ^ XOR_act_key), .*);
`else
   //
   // IIR with actual LLKI built in is here!!!! (defined in Makefile)
   //
   `CORE_WITH_LLKI core(.*);
   //
`endif // !`ifdef USE_MOCK_LLKI
   
endmodule // IIR_llki
