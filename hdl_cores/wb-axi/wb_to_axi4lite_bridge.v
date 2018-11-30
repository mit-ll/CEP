/*
 * Copyright (c) 2015, Christian Svensson <blue@cmd.nu>
 * All rights reserved.
 *
 * Redistribution and use in source and non-source forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in non-source form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS WORK IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * WORK, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

module wb_to_axi4lite_bridge #(
  parameter DW = 32,
  parameter AW = 32
)(
  input       wb_clk_i,
  input       wb_rst_i,

  input [AW-1:0]    wb_adr_i,
  input [DW-1:0]    wb_dat_i,
  input [DW/8-1:0]  wb_sel_i,
  input             wb_we_i,
  input             wb_cyc_i,
  input             wb_stb_i,
  input [2:0]       wb_cti_i,
  input [1:0]       wb_bte_i,
  output [DW-1:0]   wb_dat_o,
  output            wb_ack_o,
  output            wb_err_o,
  output            wb_rty_o,

  input             m_axi_arready,
  input             m_axi_awready,
  input             m_axi_bvalid,
  input             m_axi_rvalid,
  input             m_axi_wready,
  input [1:0]       m_axi_bresp,
  input [1:0]       m_axi_rresp,
  input [DW-1:0]    m_axi_rdata,
  output            m_axi_arvalid,
  output            m_axi_awvalid,
  output            m_axi_bready,
  output            m_axi_rready,
  output            m_axi_wvalid,
  output [2:0]      m_axi_arprot,
  output [2:0]      m_axi_awprot,
  output [AW-1:0]   m_axi_araddr,
  output [AW-1:0]   m_axi_awaddr,
  output [DW-1:0]   m_axi_wdata,
  output [DW/8-1:0] m_axi_wstrb
);
  reg id_r;
  wire m_axi_rlast;
  wire [0:0] m_axi_rid;
  wire [0:0] m_axi_bid;
  wire [0:0] m_axi_arid;
  wire [0:0] m_axi_awid;

  wb_to_axi4_bridge #(
    .DW(DW), .AW(AW), .IDW(1), .USRW(0)
  ) wb_to_axi4_bridge_i (
    .wb_clk_i(wb_clk_i),
    .wb_rst_i(wb_rst_i),
    .wb_adr_i(wb_adr_i),
    .wb_dat_i(wb_dat_i),
    .wb_sel_i(wb_sel_i),
    .wb_we_i (wb_we_i),
    .wb_cyc_i(wb_cyc_i),
    .wb_stb_i(wb_stb_i),
    // TODO(bluecmd): Locked to "Classical cycle" since AXI4-Lite doesn't do
    // bursts.
    .wb_cti_i(3'b0),
    .wb_bte_i(wb_bte_i),
    .wb_dat_o(wb_dat_o),
    .wb_ack_o(wb_ack_o),
    .wb_err_o(wb_err_o),
    .wb_rty_o(wb_rty_o),

    .m_axi_awaddr(m_axi_awaddr),
    .m_axi_awid(m_axi_awid),
    .m_axi_awprot(m_axi_awprot),
    .m_axi_awvalid(m_axi_awvalid),
    .m_axi_awready(m_axi_awready),
    .m_axi_wdata(m_axi_wdata),
    .m_axi_wstrb(m_axi_wstrb),
    .m_axi_wvalid(m_axi_wvalid),
    .m_axi_wready(m_axi_wready),
    .m_axi_bresp(m_axi_bresp),
    .m_axi_bvalid(m_axi_bvalid),
    .m_axi_bready(m_axi_bready),
    .m_axi_araddr(m_axi_araddr),
    .m_axi_arid(m_axi_arid),
    .m_axi_arprot(m_axi_arprot),
    .m_axi_arvalid(m_axi_arvalid),
    .m_axi_arready(m_axi_arready),
    .m_axi_rdata(m_axi_rdata),
    .m_axi_rresp(m_axi_rresp),
    .m_axi_rvalid(m_axi_rvalid),
    .m_axi_rready(m_axi_rready),
    .m_axi_rid(m_axi_rid),
    .m_axi_bid(m_axi_bid),
    .m_axi_arburst(),
    .m_axi_arcache(),
    .m_axi_arlen(),
    .m_axi_arlock(),
    .m_axi_arqos(),
    .m_axi_arregion(),
    .m_axi_arsize(),
    .m_axi_awburst(),
    .m_axi_awcache(),
    .m_axi_awlen(),
    .m_axi_awlock(),
    .m_axi_awqos(),
    .m_axi_awregion(),
    .m_axi_awsize(),
    .m_axi_rlast(),
    .m_axi_wlast()
  );

  assign m_axi_bid = id_r;
  assign m_axi_rid = id_r;

  // AXI4-Lite doesn't support bursts, so everything is the last word
  assign m_axi_rlast = 1'b1;

  always @(posedge wb_clk_i)
  begin
    id_r <= wb_we_i ? m_axi_awid : m_axi_arid;
  end

endmodule
