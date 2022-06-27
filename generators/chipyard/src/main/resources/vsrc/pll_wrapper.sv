//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      cep_tb.v
// Program:        Common Evaluation Platform (CEP)
// Description:    CEP Co-Simulation Top Level Testbench 
// Notes:          
//
//--------------------------------------------------------------------------------------
module pll_wrapper (
  input   clk_in,
  input   reset_in,
  output  clk_out,
  output  reset_out
);


  assign clk_out    = clk_in;
  assign reset_out  = reset_in;

endmodule // pll_wrapper