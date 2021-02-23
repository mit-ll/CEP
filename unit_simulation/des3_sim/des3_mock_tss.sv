//************************************************************************
/// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      des3_mock_tss.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    des3 wrapper with LLKI supports
// Notes:          
//************************************************************************
`timescale 1ns/1ns
//
// DES with LLKI wrapper
//
`include "llki_struct.h"
module des3_mock_tss #(parameter llki_s MY_STRUCT)
  (
   //
   // LLKI discrete's IOs
   //
   llki_discrete_if.slave llki,
   // core IO's
   output reg[63:0] desOut,
   output           out_valid,
   input            start,
   input     [63:0] desIn,
   input     [55:0] key1,
   input     [55:0] key2,
   input     [55:0] key3,
   input            decrypt,
   input            clk,
   input            reset
   );

   //
   //
   //
`ifdef USE_MOCK_LLKI
   //
   // Use "mock" logic  until replace with real thing below
   //
   llki_discrete_slave #(.MY_STRUCT(MY_STRUCT)) discreteSlave(.rst(reset), .*);
   //
   wire [127:0]    XOR_exp_key = {MY_STRUCT.key[0],MY_STRUCT.key[1]};
   wire [127:0]    XOR_act_key = {llki.rx_key_s.key[0],llki.rx_key_s.key[1]};
   // This module logic will produce output as soon as out of reset so XOR the input/output will cause failure
   // to avoid this, only XOR when statt=1
   logic 	   ok2XOR = 0;
   always @(posedge clk) begin
      if (reset) ok2XOR = 0;
      else if (start) ok2XOR = 1;
   end
   //
   des3 core(.key1(ok2XOR ? (key1 ^ XOR_exp_key ^ XOR_act_key) : key1), .*);
`else
   //
   // DES with actual LLKI built in is here!!!! (defined in Makefile)
   //
   `CORE_WITH_LLKI core(.*);
   //
`endif // !`ifdef USE_MOCK_LLKI
   
endmodule // des_llki
