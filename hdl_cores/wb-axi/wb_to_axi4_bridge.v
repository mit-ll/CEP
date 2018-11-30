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

module wb_to_axi4_bridge #(
  parameter DW = 32,
  parameter AW = 32,
  parameter IDW = 4,
  parameter USRW = 0
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

  // TODO(bluecmd): Add xUSER signals
  input             m_axi_arready,
  input             m_axi_awready,
  input             m_axi_bvalid,
  input             m_axi_rlast,
  input             m_axi_rvalid,
  input             m_axi_wready,
  input [IDW-1:0]   m_axi_bid,
  input [1:0]       m_axi_bresp,
  input [IDW-1:0]   m_axi_rid,
  input [1:0]       m_axi_rresp,
  input [DW-1:0]    m_axi_rdata,
  output [1:0]      m_axi_arburst,
  output [3:0]      m_axi_arcache,
  output [IDW-1:0]  m_axi_arid,
  output [7:0]      m_axi_arlen,
  output [0:0]      m_axi_arlock,
  output [2:0]      m_axi_arprot,
  output [2:0]      m_axi_arsize,
  output            m_axi_arvalid,
  output [3:0]      m_axi_arqos,
  output [3:0]      m_axi_arregion,
  output [1:0]      m_axi_awburst,
  output [3:0]      m_axi_awcache,
  output [IDW-1:0]  m_axi_awid,
  output [7:0]      m_axi_awlen,
  output [0:0]      m_axi_awlock,
  output [2:0]      m_axi_awprot,
  output [2:0]      m_axi_awsize,
  output            m_axi_awvalid,
  output [3:0]      m_axi_awqos,
  output [3:0]      m_axi_awregion,
  output            m_axi_bready,
  output            m_axi_rready,
  output            m_axi_wlast,
  output            m_axi_wvalid,
  output [AW-1:0]   m_axi_araddr,
  output [AW-1:0]   m_axi_awaddr,
  output [DW-1:0]   m_axi_wdata,
  output [DW/8-1:0] m_axi_wstrb
);
  wire active;
  wire complete;
  reg complete_r;
  reg asserted_addr_r = 0;
  reg asserted_write_r = 0;

  assign active = wb_stb_i & wb_cyc_i;

  assign complete = active & (wb_we_i ? m_axi_bvalid : m_axi_rvalid);

  // TODO(bluecmd): Byte mask?
  assign wb_dat_o = m_axi_rdata;
  assign m_axi_wdata = wb_dat_i;

  assign wb_ack_o = complete & (~complete_r);
  assign wb_err_o = complete & (~wb_we_i ? m_axi_rresp[1] : m_axi_bresp[1]);
  assign wb_rty_o = complete & wb_err_o &
    (~wb_we_i ? ~m_axi_rresp[0] : ~m_axi_bresp[0]);

  assign m_axi_arvalid = active & ~wb_we_i & ~asserted_addr_r;
  assign m_axi_awvalid = active & wb_we_i & ~asserted_addr_r;
  assign m_axi_rready = active & ~wb_we_i;
  assign m_axi_wvalid = active & wb_we_i & ~asserted_write_r;
  assign m_axi_araddr = wb_adr_i;
  assign m_axi_awaddr = wb_adr_i;
  assign m_axi_wstrb = wb_we_i ? wb_sel_i : 0;

  assign m_axi_bready = 1'b1;

  always @(posedge wb_clk_i)
  begin
    complete_r <= complete;

    asserted_addr_r <= asserted_addr_r;
    asserted_write_r <= asserted_write_r;

    if ((wb_we_i & m_axi_awvalid & m_axi_awready) |
       (~wb_we_i & m_axi_arvalid & m_axi_arready))
      asserted_addr_r <= 1'b1;
    if (wb_we_i & m_axi_wvalid & m_axi_wready)
      asserted_write_r <= 1'b1;
    if (wb_ack_o)
    begin
      asserted_addr_r <= 0;
      asserted_write_r <= 0;
    end
  end

  // Set all IDs we drive to 0, we do not support multiple transactions
  assign m_axi_awid = 0;
  assign m_axi_arid = 0;

  // TODO(bluecmd): Change when we want to support bursts
  assign m_axi_arburst = 2'b01;
  assign m_axi_arlen = 0;
  assign m_axi_arsize = $clog2(DW/8);

  assign m_axi_awburst = 2'b01;
  assign m_axi_awlen = 0;
  assign m_axi_awsize = $clog2(DW/8);
  assign m_axi_wlast = 1'b1;

  // We do not support protected modes
  assign m_axi_arprot = 0;
  assign m_axi_awprot = 0;

  // We do not support exclusive modes
  assign m_axi_arlock = 0;
  assign m_axi_awlock = 0;

  // We do not support cache modes
  assign m_axi_arcache = 0;
  assign m_axi_awcache = 0;

  // We do not support QoS modes
  assign m_axi_arqos = 0;
  assign m_axi_awqos = 0;

  // We do not currently support multiple regions
  // (You can use Xilinx's AXI interconnect for this)
  assign m_axi_arregion = 0;
  assign m_axi_awregion = 0;

endmodule
