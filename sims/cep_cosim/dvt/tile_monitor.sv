//--------------------------------------------------------------------------------------
// Copyright 2022 Massachusets Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:      tl_monitor.v
// Program:        Common Evaluation Platform (CEP)
// Description:    Component is used to provide Testbench level visability into what
//                 the RocketTiles are doing
// Notes:          
//
//--------------------------------------------------------------------------------------

`include "suite_config.v"
`include "cep_hierMap.incl"
`include "cep_adrMap.incl"
`include "v2c_cmds.incl"
`include "v2c_top.incl"

module tile_monitor (
  input         clock,
  input         reset,
  input         enable,
  input         auto_wfi_out_0,
  input         auto_int_local_in_3_0,
  input         auto_int_local_in_2_0,
  input         auto_int_local_in_1_0,
  input         auto_int_local_in_1_1,
  input         auto_int_local_in_0_0,
  input  [1:0]  auto_hartid_in,
  input         a_ready,
  input         a_valid,
  input [2:0]   a_bits_opcode,
  input [2:0]   a_bits_param,
  input [3:0]   a_bits_size,
  input [1:0]   a_bits_source,
  input [31:0]  a_bits_address,
  input [7:0]   a_bits_mask,
  input [63:0]  a_bits_data,
  input         b_ready,
  input         b_valid,
  input  [1:0]  b_bits_param,
  input  [3:0]  b_bits_size,
  input  [1:0]  b_bits_source,
  input  [31:0] b_bits_address,
  input         c_ready,
  input         c_valid,
  input [2:0]   c_bits_opcode,
  input [2:0]   c_bits_param,
  input [3:0]   c_bits_size,
  input [1:0]   c_bits_source,
  input [31:0]  c_bits_address,
  input [63:0]  c_bits_data,
  input         d_ready,
  input         d_valid,
  input  [2:0]  d_bits_opcode,
  input  [1:0]  d_bits_param,
  input  [3:0]  d_bits_size,
  input  [1:0]  d_bits_source,
  input  [1:0]  d_bits_sink,
  input         d_bits_denied,
  input  [63:0] d_bits_data,
  input         d_bits_corrupt,
  input         e_ready,
  input         e_valid,
  input [1:0]   e_bits_sink
);

  always @(posedge clock && enable) begin
    if (a_valid && a_ready) begin
      `logI("TILE_MONITOR: Tile #%0d A channel TL transaction - opcode=%0d, param=%0d, size=%0d, source=%0d, address=0x%x, mask=0x%x, data=0x%x",
        auto_hartid_in, a_bits_opcode, a_bits_param, a_bits_size, a_bits_source, a_bits_address, a_bits_mask, a_bits_data);
    end // end if a_valid)
    if (b_valid && b_ready) begin
    end // if (b_valid)
    if (c_valid && c_ready) begin
    end // if (c_valid)
    if (d_valid && d_ready) begin
      if (d_bits_denied && d_bits_corrupt)
        `logE("TILE_MONITOR: Tile #%0d D channel TL transaction - opcode=%0d, param=%0d, size=%0d, source=%0d, sink=%0d, denied=%0d, data=0x%x, corrupt=%0d",
          auto_hartid_in, d_bits_opcode, d_bits_param, d_bits_size, d_bits_source, d_bits_sink, d_bits_denied, d_bits_data, d_bits_corrupt);
      else
        `logI("TILE_MONITOR: Tile #%0d D channel TL transaction - opcode=%0d, param=%0d, size=%0d, source=%0d, sink=%0d, denied=%0d, data=0x%x, corrupt=%0d",
          auto_hartid_in, d_bits_opcode, d_bits_param, d_bits_size, d_bits_source, d_bits_sink, d_bits_denied, d_bits_data, d_bits_corrupt);
    end // if (d_valid)
    if (e_valid && e_ready) begin
    end // if (e_valid)
  end // always @(posedge clock && enable)

endmodule // tile_monitor