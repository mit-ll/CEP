//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      RocketTile_beh.v
// Program:        Common Evaluation Platform (CEP)
// Description:    Behavioral replacement of all instances of RocketTile
//                 in the generated verilog
// Notes:          Updated for Chipyard-based CEP
//
//--------------------------------------------------------------------------------------

module RocketTile_beh(
  input         clock,
  input         reset,
  output        auto_wfi_out_0,
  input         auto_int_local_in_3_0,  // unused
  input         auto_int_local_in_2_0,  // unused
  input         auto_int_local_in_1_0,  // unused
  input         auto_int_local_in_1_1,  // unused
  input         auto_int_local_in_0_0,  // unused
  input  [1:0]  auto_hartid_in,         // unused
  input         auto_tl_other_masters_out_a_ready,
  output        auto_tl_other_masters_out_a_valid,
  output [2:0]  auto_tl_other_masters_out_a_bits_opcode,
  output [2:0]  auto_tl_other_masters_out_a_bits_param,
  output [3:0]  auto_tl_other_masters_out_a_bits_size,
  output [1:0]  auto_tl_other_masters_out_a_bits_source,
  output [31:0] auto_tl_other_masters_out_a_bits_address,
  output [7:0]  auto_tl_other_masters_out_a_bits_mask,
  output [63:0] auto_tl_other_masters_out_a_bits_data,
  output        auto_tl_other_masters_out_b_ready,
  input         auto_tl_other_masters_out_b_valid,
  input  [1:0]  auto_tl_other_masters_out_b_bits_param,
  input  [3:0]  auto_tl_other_masters_out_b_bits_size,
  input  [1:0]  auto_tl_other_masters_out_b_bits_source,
  input  [31:0] auto_tl_other_masters_out_b_bits_address,
  input         auto_tl_other_masters_out_c_ready,
  output        auto_tl_other_masters_out_c_valid,
  output [2:0]  auto_tl_other_masters_out_c_bits_opcode,
  output [2:0]  auto_tl_other_masters_out_c_bits_param,
  output [3:0]  auto_tl_other_masters_out_c_bits_size,
  output [1:0]  auto_tl_other_masters_out_c_bits_source,
  output [31:0] auto_tl_other_masters_out_c_bits_address,
  output [63:0] auto_tl_other_masters_out_c_bits_data,
  output        auto_tl_other_masters_out_d_ready,
  input         auto_tl_other_masters_out_d_valid,
  input  [2:0]  auto_tl_other_masters_out_d_bits_opcode,
  input  [1:0]  auto_tl_other_masters_out_d_bits_param,
  input  [3:0]  auto_tl_other_masters_out_d_bits_size,
  input  [1:0]  auto_tl_other_masters_out_d_bits_source,
  input  [1:0]  auto_tl_other_masters_out_d_bits_sink,
  input         auto_tl_other_masters_out_d_bits_denied,
  input  [63:0] auto_tl_other_masters_out_d_bits_data,
  input         auto_tl_other_masters_out_d_bits_corrupt,
  input         auto_tl_other_masters_out_e_ready,
  output        auto_tl_other_masters_out_e_valid,
  output [1:0]  auto_tl_other_masters_out_e_bits_sink
);
 
  // Tie-off unused outputs
  assign auto_wfi_out_0 = 0;

  // Instantiate the Tilelink Master Behavioral Model
  tl_master_beh #(
    .CHIP_ID    (0),  // Will be overwritten by the testbench for each core and thus
                      // the auto_hartid_in is unused
    .SRC_SIZE   (2),  // Equivalent to OpenTitan's TL_AIW
    .SINK_SIZE  (2),  // Equivalent to OpenTitan's TL_DIW
    .BUS_SIZE   (8),  // Equivalent to OpenTitan's TL_DBW
    .ADR_WIDTH  (32)  // Equivalent to OpenTitan's TL_DW
  ) tl_master (
    .clock                        (clock),
    .reset                        (reset),
    .tl_master_a_ready            (auto_tl_other_masters_out_a_ready),
    .tl_master_a_valid            (auto_tl_other_masters_out_a_valid),
    .tl_master_a_bits_opcode      (auto_tl_other_masters_out_a_bits_opcode),
    .tl_master_a_bits_param       (auto_tl_other_masters_out_a_bits_param),
    .tl_master_a_bits_size        (auto_tl_other_masters_out_a_bits_size),
    .tl_master_a_bits_source      (auto_tl_other_masters_out_a_bits_source),
    .tl_master_a_bits_address     (auto_tl_other_masters_out_a_bits_address),
    .tl_master_a_bits_mask        (auto_tl_other_masters_out_a_bits_mask),
    .tl_master_a_bits_data        (auto_tl_other_masters_out_a_bits_data),
    .tl_master_a_bits_corrupt     (auto_tl_other_masters_out_a_bits_corrupt),
    .tl_master_b_ready            (auto_tl_other_masters_out_b_ready),
    .tl_master_b_valid            (auto_tl_other_masters_out_b_valid),
    .tl_master_b_bits_opcode      (3'h6),
    .tl_master_b_bits_size        (4'h6),
    .tl_master_b_bits_mask        (8'hff),
    .tl_master_b_bits_corrupt     (1'b0),
    .tl_master_b_bits_param       (auto_tl_other_masters_out_b_bits_param),
    .tl_master_b_bits_source      (auto_tl_other_masters_out_b_bits_source),
    .tl_master_b_bits_address     (auto_tl_other_masters_out_b_bits_address),
    .tl_master_c_ready            (auto_tl_other_masters_out_c_ready),
    .tl_master_c_valid            (auto_tl_other_masters_out_c_valid),
    .tl_master_c_bits_opcode      (auto_tl_other_masters_out_c_bits_opcode),
    .tl_master_c_bits_param       (auto_tl_other_masters_out_c_bits_param),
    .tl_master_c_bits_size        (auto_tl_other_masters_out_c_bits_size),
    .tl_master_c_bits_source      (auto_tl_other_masters_out_c_bits_source),
    .tl_master_c_bits_address     (auto_tl_other_masters_out_c_bits_address),
    .tl_master_c_bits_data        (auto_tl_other_masters_out_c_bits_data),
    .tl_master_c_bits_corrupt     (auto_tl_other_masters_out_c_bits_corrupt),
    .tl_master_d_ready            (auto_tl_other_masters_out_d_ready),
    .tl_master_d_valid            (auto_tl_other_masters_out_d_valid),
    .tl_master_d_bits_opcode      (auto_tl_other_masters_out_d_bits_opcode),
    .tl_master_d_bits_param       (auto_tl_other_masters_out_d_bits_param),
    .tl_master_d_bits_size        (auto_tl_other_masters_out_d_bits_size),
    .tl_master_d_bits_source      (auto_tl_other_masters_out_d_bits_source),
    .tl_master_d_bits_sink        (auto_tl_other_masters_out_d_bits_sink),
    .tl_master_d_bits_denied      (auto_tl_other_masters_out_d_bits_denied),
    .tl_master_d_bits_data        (auto_tl_other_masters_out_d_bits_data),
    .tl_master_d_bits_corrupt     (auto_tl_other_masters_out_d_bits_corrupt),
    .tl_master_e_ready            (auto_tl_other_masters_out_e_ready),
    .tl_master_e_valid            (auto_tl_other_masters_out_e_valid),
    .tl_master_e_bits_sink        (auto_tl_other_masters_out_e_bits_sink)
   );

endmodule // RocketTile_beh
