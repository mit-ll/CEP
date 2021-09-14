// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
//************************************************************************
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// File Name:       tlul_adaptger_reg.sv
// Program:         Common Evaluation Platform (CEP)
// Description:     Component modified from OpenTitan original
//                  to support a broader range of the Tilelink
//                  protocol
//                  
//************************************************************************



`include "prim_assert.sv"

//
// Modifications:
// Copyright 2021 Massachusetts Institute of Technology
// SPDX short identifier: BSD-2-Clause
//
// Module modified to allow for a delayed read-response (and avoid the live-decode problem)
//


/**
 * Tile-Link UL adapter for Register interface
 */

module tlul_adapter_reg import tlul_pkg::*; #(
  parameter  int RegAw = 8,
  parameter  int RegDw = 32, // Shall be matched with TL_DW
  localparam int RegBw = RegDw/8
) (
  input clk_i,
  input rst_ni,

  // TL-UL interface
  input  tl_h2d_t tl_i,
  output tl_d2h_t tl_o,

  // Register interface
  output logic             re_o,
  output logic             we_o,
  output logic [RegAw-1:0] addr_o,
  output logic [RegDw-1:0] wdata_o,
  output logic [RegBw-1:0] be_o,
  input        [RegDw-1:0] rdata_i,
  input                    ack_i,
  input                    error_i
);

  localparam int IW  = $bits(tl_i.a_source);
  localparam int SZW = $bits(tl_i.a_size);

  logic d_valid;
  logic outstanding_read;   // A read transaction is outstanding
  logic outstanding_write;  // A write transaction is outstanding
  logic a_ack;

  logic [RegDw-1:0] rdata;
  logic             error, err_internal;

  logic malformed_meta_err; // User signal format error or unsupported
  logic tl_err;             // Common TL-UL error checker

  logic [IW-1:0]  req_id;
  logic [SZW-1:0] req_sz;
  tl_d_op_e       resp_opcode;

  logic rd_req, wr_req;

  // Assign individual signals to the D Channel
  assign tl_o = '{
    a_ready:  ~(outstanding_read || outstanding_write),
    d_valid:  d_valid,
    d_opcode: resp_opcode,
    d_param:  '0,
    d_size:   req_sz,
    d_source: req_id,
    d_sink:   '0,
    d_data:   rdata,
    d_user:  '0,
    d_error: error
  };

  // We will immediately begin processing the transaction if we don't currently have one outstanding
  assign a_ack          = tl_i.a_valid & tl_o.a_ready;
  
  // Request signal
  assign wr_req  = a_ack & ((tl_i.a_opcode == PutFullData) | (tl_i.a_opcode == PutPartialData));
  assign rd_req  = a_ack & (tl_i.a_opcode == Get);

  assign we_o    = wr_req & ~err_internal;
  assign re_o    = rd_req & ~err_internal;
  assign addr_o  = tl_i.a_address;
  assign wdata_o = tl_i.a_data;
  assign be_o    = tl_i.a_mask;

  // Given the amount of changes needed to allow an external component to control acknowledgement timing,
  // everything is being moved here
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      outstanding_read      <= '0;
      outstanding_write     <= '0;
      req_id                <= '0;
      req_sz                <= '0;
      resp_opcode           <= AccessAck;
      error                 <= '0;
      d_valid               <= '0;
      rdata                 <= '0;
    end else begin

      // Queue up some of the d channel response fields
      // Internal errors will be detected at the time A Channel is being processed
      if (a_ack) begin
        req_id              <= tl_i.a_source;
        req_sz              <= tl_i.a_size;        
        resp_opcode         <= (rd_req) ? AccessAckData : AccessAck;  // Return AccessAckData regardless of error
        error               <= err_internal; 
      end;

      // Acknowledgements (reads and writes) need to be delayed
      // to allow higher level (external) decode to do things
      if (wr_req) begin
        outstanding_write   <= '1;
      end else if (rd_req) begin
        outstanding_read    <= '1;
      end

      // If a read is ongoing and has been acknowleedge, time
      // to assert valid on the D channel (and capture rdata and or in the external error)
      // Adding a_ack allows for zero-cycle decode on reads
      if (ack_i & (outstanding_read | a_ack)) begin
        d_valid             <= '1;
        rdata               <= rdata_i;
        error               <= error | error_i;
      // If a write is ongoing, just capture the valid and or in the external error)
      // Adding a_ack allows for zero-cycle decode on writes
      end else if (ack_i & (outstanding_write | a_ack)) begin
        d_valid             <= '1;
        error               <= error | error_i;
      end else if (tl_i.d_ready & (outstanding_read | outstanding_write)) begin
        outstanding_read    <= '0;
        outstanding_write   <= '0;
        req_id              <= '0;
        req_sz              <= '0;
        resp_opcode         <= AccessAck;
        error               <= '0;
        d_valid             <= '0;
        rdata               <= '0;
      end

    end // end if (!rst_ni)
  end // end always

  ////////////////////
  // Error Handling //
  ////////////////////
  assign err_internal = malformed_meta_err | tl_err;

  // malformed_meta_err
  //    Raised if not supported feature is turned on or user signal has malformed
  assign malformed_meta_err = (tl_i.a_user.parity_en == 1'b1);

  // tl_err : separate checker
  tlul_err u_err (
    .clk_i,
    .rst_ni,
    .tl_i,
    .err_o (tl_err)
  );


  // Ensure we don't have a data-width mismatch
  `ASSERT_INIT(MatchedWidthAssert, RegDw == top_pkg::TL_DW)

  // Ensure we don't have an error asserted
  `ASSERT(tlul_d_channel_error, !tl_o.d_error, clk_i, !rst_ni)

endmodule
