//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX License Identifier: MIT
//
// File Name:       des3_mock_tss.sv
// Program:         Common Evaluation Platform (CEP)
// Description:     
// Notes:           
//************************************************************************
`timescale 1ns/1ns

module des3_mock_tss import llki_pkg::*; (

    // Clock and Reset
    input wire            clk,
    input wire            rst, 

    // Core I/O
    input wire            start,
    input wire [63:0]     desIn,
    input wire [55:0]     key1,
    input wire [55:0]     key2,
    input wire [55:0]     key3,
    input wire            decrypt,
    output wire [63:0]    desOut,
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
  localparam KEY_WORDS          = DES3_MOCK_TSS_NUM_KEY_WORDS;
  reg [(64*KEY_WORDS) - 1:0]    llkid_key_register;
  wire [(64*KEY_WORDS) - 1:0]   mock_tss_desIn;

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
      assign mock_tss_desIn[64*i +: 64] = DES3_MOCK_TSS_KEY_WORDS[i] ^
                                          llkid_key_register[64*i +: 64] ^
                                          desIn[64*i +: 64];
    end
  endgenerate
  //------------------------------------------------------------------



  //------------------------------------------------------------------
  // Instantiate the original core
  //------------------------------------------------------------------
  des3 des3_inst (
    .clk          (clk),
    .reset        (rst),
    .start        (start),
    .desIn        (mock_tss_desIn),
    .key1         (key1),
    .key2         (key2),
    .key3         (key3),
    .decrypt      (decrypt),
    .desOut       (desOut),
    .out_valid    (out_valid)
  );
  //------------------------------------------------------------------

endmodule

