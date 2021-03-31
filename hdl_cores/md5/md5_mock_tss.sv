//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:       md5_mock_tss.sv
// Program:         Common Evaluation Platform (CEP)
// Description:     
// Notes:           
//************************************************************************
`timescale 1ns/1ns

module md5_mock_tss import llki_pkg::*; (

    // Clock and Reset
    input wire            clk,
    input wire            rst,

    // Core I/O
    input wire            init,
    input wire            msg_in_valid,
    input wire [511 : 0]  msg_padded,
    output wire [127 : 0] msg_output,
    output wire           msg_out_valid,
    output wire           ready,

    // LLKI Discrete I/O
    input [63:0]          llkid_key_data,
    input                 llkid_key_valid,
    output reg            llkid_key_ready,
    output reg            llkid_key_complete,
    input                 llkid_clear_key,
    output reg            llkid_clear_key_ack

);

  // Internal signals & localparams
  localparam KEY_WORDS          = MD5_MOCK_TSS_NUM_KEY_WORDS;
  reg [(64*KEY_WORDS) - 1:0]    llkid_key_register;
  wire [(64*KEY_WORDS) - 1:0]   mock_tss_msg_padded;

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
      assign mock_tss_msg_padded[64*i +: 64] = MD5_MOCK_TSS_KEY_WORDS[i] ^
                                          llkid_key_register[64*i +: 64] ^
                                          msg_padded[64*i +: 64];
    end
  endgenerate
  //------------------------------------------------------------------



  //------------------------------------------------------------------
  // Instantiate the original core
  //------------------------------------------------------------------
  md5 md5_inst (
    .clk            (clk),
    .rst            (rst),
    .init           (init),
    .msg_padded     (mock_tss_msg_padded),
    .msg_in_valid   (msg_in_valid),
    .msg_output     (msg_output),
    .msg_out_valid  (msg_out_valid),
    .ready          (ready)
  );
  //------------------------------------------------------------------

endmodule

