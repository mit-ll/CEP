//************************************************************************
// Copyright (C) 2020 Massachusetts Institute of Technology
//
// File Name:      aes_mock_tss.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    aes wrapper with LLKI supports
// Notes:          
//************************************************************************

//
// AES with LLKI wrapper
//
`include "llki_struct.h"
module aes_192_mock_tss #(parameter llki_s MY_STRUCT)
  (
   //
   // LLKI discrete's IOs
   //
   llki_discrete_if.slave llki,
   // core IO's
   input           clk, 
   input           rst, 
   input           start,
   input  [127:0]  state,
   input  [191:0]  key,
   output  [127:0] out,
   output          out_valid
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
    aes_192 core(.state(state ^ XOR_exp_key ^ XOR_act_key), .*);
`else
   //
   // AES with actual LLKI built in is here!!!! (defined in Makefile)
   //
   `CORE_WITH_LLKI core(.*);
   //
`endif // !`ifdef USE_MOCK_LLKI
   
endmodule // aes_llki
