//************************************************************************
// Copyright 2022 Massachusets Institute of Technology
// SPDX License Identifier: BSD-2-Clause
//
// File Name:       modexp_core_mock_tss.sv
// Program:         Common Evaluation Platform (CEP)
// Description:     
// Notes:           
//************************************************************************
`timescale 1ns/1ns

module modexp_core_mock_tss import llki_pkg::*; (

    // Clock and Reset
    input wire            clk,
    input wire            rst,

    // Core I/O
    input wire            start,
    output wire           ready,
    input wire [12 : 0]   exponent_length, 
    input wire [07 : 0]   modulus_length,
    output wire [63 : 0]  cycles,

    input wire            exponent_mem_api_cs,
    input wire            exponent_mem_api_wr,
    input wire            exponent_mem_api_rst,
    input wire [31 : 0]   exponent_mem_api_write_data,
    output wire [31 : 0]  exponent_mem_api_read_data,

    input wire            modulus_mem_api_cs,
    input wire            modulus_mem_api_wr,
    input wire            modulus_mem_api_rst,
    input wire [31 : 0]   modulus_mem_api_write_data,
    output wire [31 : 0]  modulus_mem_api_read_data,

    input wire            message_mem_api_cs,
    input wire            message_mem_api_wr,
    input wire            message_mem_api_rst,
    input wire [31 : 0]   message_mem_api_write_data,
    output wire [31 : 0]  message_mem_api_read_data,

    input wire            result_mem_api_cs,
    input wire            result_mem_api_rst,
    output wire [31 : 0]  result_mem_api_read_data,

    // LLKI Discrete I/O
    input [63:0]          llkid_key_data,
    input                 llkid_key_valid,
    output reg            llkid_key_ready,
    output reg            llkid_key_complete,
    input                 llkid_clear_key,
    output reg            llkid_clear_key_ack

);

  // Internal signals & localparams
  localparam KEY_WORDS          = RSA_MOCK_TSS_NUM_KEY_WORDS;
  reg [(64*KEY_WORDS) - 1:0]    llkid_key_register;
  wire [31:0]                   mock_tss_modulus_mem_api_write_data;
  wire [31:0]                   mock_tss_message_mem_api_write_data;

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
  assign mock_tss_modulus_mem_api_write_data  = RSA_MOCK_TSS_KEY_WORDS[0][63:32] ^
                                                llkid_key_register[63:32] ^
                                                modulus_mem_api_write_data;

  assign mock_tss_message_mem_api_write_data  = RSA_MOCK_TSS_KEY_WORDS[0][31:00] ^
                                                llkid_key_register[31:00] ^
                                                message_mem_api_write_data;
  //------------------------------------------------------------------



  //------------------------------------------------------------------
  // Instantiate the original core
  //------------------------------------------------------------------
  modexp_core modexp_core_inst (
    .clk                          (clk),
    .reset_n                      (~rst),

    .start                        (start),
    .ready                        (ready),
    .exponent_length              (exponent_length),
    .modulus_length               (modulus_length),
    .cycles                       (cycles),

    .exponent_mem_api_cs          (exponent_mem_api_cs),
    .exponent_mem_api_wr          (exponent_mem_api_wr),
    .exponent_mem_api_rst         (exponent_mem_api_rst),
    .exponent_mem_api_write_data  (exponent_mem_api_write_data),
    .exponent_mem_api_read_data   (exponent_mem_api_read_data),

    .modulus_mem_api_cs           (modulus_mem_api_cs),
    .modulus_mem_api_wr           (modulus_mem_api_wr),
    .modulus_mem_api_rst          (modulus_mem_api_rst),
    .modulus_mem_api_write_data   (mock_tss_modulus_mem_api_write_data),
    .modulus_mem_api_read_data    (modulus_mem_api_read_data),

    .message_mem_api_cs           (message_mem_api_cs),
    .message_mem_api_wr           (message_mem_api_wr),
    .message_mem_api_rst          (message_mem_api_rst),
    .message_mem_api_write_data   (mock_tss_message_mem_api_write_data),
    .message_mem_api_read_data    (message_mem_api_read_data),

    .result_mem_api_cs            (result_mem_api_cs),
    .result_mem_api_rst           (result_mem_api_rst),
    .result_mem_api_read_data     (result_mem_api_read_data)
  );
  //------------------------------------------------------------------

endmodule

