//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:       aes_192_mock_tss.sv
// Program:         Common Evaluation Platform (CEP)
// Description:     
// Notes:           
//************************************************************************
`timescale 1ns/1ns

module aes_192_mock_tss import llki_pkg::*; (

    // Clock and Reset
    input wire            clk,
    input wire            rst, 

    // Core I/O
    input wire            start,
    input wire [127:0]    state,
    input wire [191:0]    key,
    output wire [127:0]   out,
    output wire           out_valid,

    // LLKI Discrete I/O
    input [63:0]          llkid_key_data,
    input                 llkid_key_valid,
    output reg            llkid_key_ready,
    output reg            llkid_key_complete,
    input                 llkid_clear_key,
    output reg            llkid_clear_key_ack

);

  // Internal signals & localparams
  localparam KEY_WORDS          = AES_MOCK_TSS_NUM_KEY_WORDS;
  reg [(64*KEY_WORDS) - 1:0]    llkid_key_register;
  wire [(64*KEY_WORDS) - 1:0]   mock_tss_state;

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
  genvar i;
  generate
    for (i = 0; i < KEY_WORDS; i = i + 1) begin
      assign mock_tss_state[64*i +: 64] = AES_MOCK_TSS_KEY_WORDS[i] ^
                                          llkid_key_register[64*i +: 64] ^
                                          state[64*i +: 64];
    end
  endgenerate
  //------------------------------------------------------------------



  //------------------------------------------------------------------
  // Instantiate the original core
  //------------------------------------------------------------------
  aes_192 aes_192_inst (
    .clk          (clk),
    .rst          (rst),
    .start        (start),
    .state        (mock_tss_state),
    .key          (key),
    .out          (out),
    .out_valid    (out_valid)
  );
  //------------------------------------------------------------------

endmodule

