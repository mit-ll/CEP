//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:      gps_mock_tss.sv
// Program:        Common Evaluation Platform (CEP)
// Description:    gps wrapper with LLKI supports
// Notes:          
//************************************************************************
`timescale 1ns/1ns

//
// GPS with LLKI wrapper
//
`include "llki_struct.h"
module gps_mock_tss #(parameter llki_s MY_STRUCT)
  (
   //
   // LLKI discrete's IOs
   //
   llki_discrete_if.slave llki,
   // core IO's
   input  sys_clk_50,
   input  sync_rst_in,
   input [5:0] sv_num,
   input startRound,
   output reg [12:0] ca_code,
   output [127:0] p_code,
   output [127:0] l_code,
   output l_code_valid
   );

   //
   //
   //
`ifdef USE_MOCK_LLKI
   //
   // Use "mock" logic  until replace with real thing below
   // 
   llki_discrete_slave #(.MY_STRUCT(MY_STRUCT)) discreteSlave(.clk(sys_clk_50),.rst(sync_rst_in),.*);
   //
   wire [127:0]    XOR_exp_key = {MY_STRUCT.key[0],MY_STRUCT.key[1]};
   wire [127:0]    XOR_act_key = {llki.rx_key_s.key[0],llki.rx_key_s.key[1]};
   gps core(.sv_num(sv_num ^ XOR_exp_key ^ XOR_act_key), .*);
`else
   //
   // GPS with actual LLKI built in is here!!!! (defined in Makefile)
   //
   `CORE_WITH_LLKI core(.*);
   //
`endif // !`ifdef USE_MOCK_LLKI
   
endmodule // GPS_llki
