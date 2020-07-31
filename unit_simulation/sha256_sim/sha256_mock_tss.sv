//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      sha256_mock_tss.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    sha256 wrapper with LLKI supports
// Notes:          
//************************************************************************


//
// SHA256 with LLKI wrapper
//
`include "llki_struct.h"
module sha256_mock_tss #(parameter llki_s MY_STRUCT)
  (
   //
   // LLKI discrete's IOs
   //
   llki_discrete_if.slave llki,
   // core IO's
   input wire            clk,
   input wire            rst,
   
   input wire            init,
   input wire            next,
   
   input wire [511 : 0]  block,
   
   output wire           ready,
   output wire [255 : 0] digest,
   output wire           digest_valid
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
   sha256 core(.block(block ^ XOR_exp_key ^ XOR_act_key), .*);
`else
   //
   // SHA256 with actual LLKI built in is here!!!! (defined in Makefile)
   //
   `CORE_WITH_LLKI core(.*);
   //
`endif // !`ifdef USE_MOCK_LLKI
   
endmodule // SHA256_llki
