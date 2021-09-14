//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:       dft_top_mock_tss.sv
// Program:         Common Evaluation Platform (CEP)
// Description:     
// Notes:           
//************************************************************************
`timescale 1ns/1ns

module dft_top_mock_tss import llki_pkg::*; (

    // Clock and Reset
    input wire            clk,
    input wire            rst,
    
    // Core I/O
    input wire            next,
    output wire           next_out,

    input wire [15 : 0]   X0,
    input wire [15 : 0]   X1,
    input wire [15 : 0]   X2,
    input wire [15 : 0]   X3,

    output wire [15 : 0]  Y0,
    output wire [15 : 0]  Y1,
    output wire [15 : 0]  Y2,
    output wire [15 : 0]  Y3,

    // LLKI Discrete I/O
    input [63:0]          llkid_key_data,
    input                 llkid_key_valid,
    output reg            llkid_key_ready,
    output reg            llkid_key_complete,
    input                 llkid_clear_key,
    output reg            llkid_clear_key_ack

);

  // Internal signals & localparams
  localparam KEY_WORDS          = DFT_MOCK_TSS_NUM_KEY_WORDS;
  reg [(64*KEY_WORDS) - 1:0]    llkid_key_register;
  wire [15:0]                   mock_tss_X0;
  wire [15:0]                   mock_tss_X1;
  wire [15:0]                   mock_tss_X2;
  wire [15:0]                   mock_tss_X3;

  //------------------------------------------------------------------
  // Instantiate the Mock TSS Finite State Machine
  //------------------------------------------------------------------
  mock_tss_fsm #(
    .KEY_WORDS            (KEY_WORDS)
  ) mock_tss_fsm_inst (
    .clk                  (clk),
    .rst                  (rst),
    .llkid_key_data       (llkid_key_data),
    .llkid_key_valid      (llkid_key_valid),
    .llkid_key_ready      (llkid_key_ready),
    .llkid_key_complete   (llkid_key_complete),
    .llkid_clear_key      (llkid_clear_key),
    .llkid_clear_key_ack  (llkid_clear_key_ack),
    .llkid_key_register   (llkid_key_register)
  );
  //------------------------------------------------------------------



  //------------------------------------------------------------------
  // Create the Mock TSS input into the original core
  //------------------------------------------------------------------
  assign mock_tss_X0          = DFT_MOCK_TSS_KEY_WORDS[0][15:0] ^
                                llkid_key_register[15:0] ^
                                X0;
  assign mock_tss_X1          = DFT_MOCK_TSS_KEY_WORDS[0][31:16] ^
                                llkid_key_register[31:16] ^
                                X1;
  assign mock_tss_X2          = DFT_MOCK_TSS_KEY_WORDS[0][47:32] ^
                                llkid_key_register[47:32] ^
                                X2;
  assign mock_tss_X3          = DFT_MOCK_TSS_KEY_WORDS[0][63:48] ^
                                llkid_key_register[63:48] ^
                                X3;
  //------------------------------------------------------------------



  //------------------------------------------------------------------
  // Instantiate the original core
  //------------------------------------------------------------------
  dft_top dft_top_inst (
    .clk            (clk),
    .reset          (rst),
    .next           (next),
    .next_out       (next_out),
    .X0             (mock_tss_X0),
    .X1             (mock_tss_X1),
    .X2             (mock_tss_X2),
    .X3             (mock_tss_X3),
    .Y0             (Y0),
    .Y1             (Y1),
    .Y2             (Y2),
    .Y3             (Y3)
  );
  //------------------------------------------------------------------

endmodule

