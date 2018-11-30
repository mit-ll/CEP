//
// Copyright (C) 2018 Massachusetts Institute of Technology
//
// File         : mor1kx_top_axi4lite.v
// Project      : Common Evaluation Platform (CEP)
// Description  : This file provides an axi4-lite wrapper for the wishbone based-MOR1KX processor
//

`include "mor1kx-defines.v"

module mor1kx_top_axi4lite (
    
    // Clock and Resets
    input                                   clk_i,
    input                                   rst_i,

    // AXI4-Lite Instruction and Data master interfaces
    AXI_LITE.out                            master_i,
    AXI_LITE.out                            master_d,

    // MOR1KX Interrupts
    input [31:0]                            mor1kx_pic_ints,

    // MOR1KX Debug interface
    input [15:0]                            mor1kx_dbg_adr_i,
    input                                   mor1kx_dbg_stb_i,
    input [master_i.AXI_DATA_WIDTH-1:0]     mor1kx_dbg_dat_i,
    input                                   mor1kx_dbg_we_i,
    output [master_i.AXI_DATA_WIDTH-1:0]    mor1kx_dbg_dat_o,
    output                                  mor1kx_dbg_ack_o,
    input                                   mor1kx_dbg_stall_i,
    output                                  mor1kx_dbg_stall_o

);

    // mor1kx wishbone instruction bus wires
    wire [master_i.AXI_ADDR_WIDTH-1:0]      wbm_i_or1k_adr_o;
    wire [master_i.AXI_DATA_WIDTH-1:0]      wbm_i_or1k_dat_o;
    wire [3:0]                              wbm_i_or1k_sel_o;
    wire                                    wbm_i_or1k_we_o;
    wire                                    wbm_i_or1k_cyc_o;
    wire                                    wbm_i_or1k_stb_o;
    wire [2:0]                              wbm_i_or1k_cti_o;
    wire [1:0]                              wbm_i_or1k_bte_o;
    wire [master_i.AXI_DATA_WIDTH-1:0]      wbm_i_or1k_dat_i;
    wire                                    wbm_i_or1k_ack_i;
    wire                                    wbm_i_or1k_err_i;
    wire                                    wbm_i_or1k_rty_i;
    
    // mor1kx wishbone data bus wires
    wire [master_d.AXI_ADDR_WIDTH-1:0]      wbm_d_or1k_adr_o;
    wire [master_d.AXI_DATA_WIDTH-1:0]      wbm_d_or1k_dat_o;
    wire [3:0]                              wbm_d_or1k_sel_o;
    wire                                    wbm_d_or1k_we_o;
    wire                                    wbm_d_or1k_cyc_o;
    wire                                    wbm_d_or1k_stb_o;
    wire [2:0]                              wbm_d_or1k_cti_o;
    wire [1:0]                              wbm_d_or1k_bte_o;
    wire [master_i.AXI_DATA_WIDTH-1:0]      wbm_d_or1k_dat_i;
    wire                                    wbm_d_or1k_ack_i;
    wire                                    wbm_d_or1k_err_i;
    wire                                    wbm_d_or1k_rty_i;

    // Instantiate the Wishbone to AXI4 Lite bridge (Instruction Bus)
    wb_to_axi4lite_bridge #(
        .DW(master_i.AXI_DATA_WIDTH),
        .AW(master_i.AXI_ADDR_WIDTH)
    ) wb_to_axi4lite_bridge_i_inst (
        .wb_clk_i(clk_i),
        .wb_rst_i(rst_i),

        // Wishbone
        .wb_adr_i(wbm_i_or1k_adr_o),
        .wb_dat_i(wbm_i_or1k_dat_o),
        .wb_sel_i(wbm_i_or1k_sel_o),
        .wb_we_i(wbm_i_or1k_we_o),
        .wb_cyc_i(wbm_i_or1k_cyc_o),
        .wb_stb_i(wbm_i_or1k_stb_o),
        .wb_cti_i(wbm_i_or1k_cti_o),
        .wb_bte_i(wbm_i_or1k_bte_o),
        .wb_dat_o(wbm_i_or1k_dat_i),
        .wb_ack_o(wbm_i_or1k_ack_i),
        .wb_err_o(wbm_i_or1k_err_i),
        .wb_rty_o(wbm_i_or1k_rty_i),

        // AXI4-Lite
        .m_axi_arready(master_i.ar_ready),
        .m_axi_awready(master_i.aw_ready),
        .m_axi_bvalid(master_i.b_valid),
        .m_axi_rvalid(master_i.r_valid),
        .m_axi_wready(master_i.w_ready),
        .m_axi_bresp(master_i.b_resp),
        .m_axi_rresp(master_i.r_resp),
        .m_axi_rdata(master_i.r_data),
        .m_axi_arvalid(master_i.ar_valid),
        .m_axi_awvalid(master_i.aw_valid),
        .m_axi_bready(master_i.b_ready),
        .m_axi_rready(master_i.r_ready),
        .m_axi_wvalid(master_i.w_valid),
        .m_axi_arprot(),
        .m_axi_awprot(),
        .m_axi_araddr(master_i.ar_addr),
        .m_axi_awaddr(master_i.aw_addr),
        .m_axi_wdata(master_i.w_data),
        .m_axi_wstrb(master_i.w_strb)
    );

// Instantiate the Wishbone to AXI4 Lite bridge (Data Bus)
    wb_to_axi4lite_bridge #(
        .DW(master_d.AXI_DATA_WIDTH),
        .AW(master_d.AXI_ADDR_WIDTH)
    ) wb_to_axi4lite_bridge_d_inst (
        .wb_clk_i(clk_i),
        .wb_rst_i(rst_i),

        // Wishbone
        .wb_adr_i(wbm_d_or1k_adr_o),
        .wb_dat_i(wbm_d_or1k_dat_o),
        .wb_sel_i(wbm_d_or1k_sel_o),
        .wb_we_i(wbm_d_or1k_we_o),
        .wb_cyc_i(wbm_d_or1k_cyc_o),
        .wb_stb_i(wbm_d_or1k_stb_o),
        .wb_cti_i(wbm_d_or1k_cti_o),
        .wb_bte_i(wbm_d_or1k_bte_o),
        .wb_dat_o(wbm_d_or1k_dat_i),
        .wb_ack_o(wbm_d_or1k_ack_i),
        .wb_err_o(wbm_d_or1k_err_i),
        .wb_rty_o(wbm_d_or1k_rty_i),

        // AXI4-Lite
        .m_axi_arready(master_d.ar_ready),
        .m_axi_awready(master_d.aw_ready),
        .m_axi_bvalid(master_d.b_valid),
        .m_axi_rvalid(master_d.r_valid),
        .m_axi_wready(master_d.w_ready),
        .m_axi_bresp(master_d.b_resp),
        .m_axi_rresp(master_d.r_resp),
        .m_axi_rdata(master_d.r_data),
        .m_axi_arvalid(master_d.ar_valid),
        .m_axi_awvalid(master_d.aw_valid),
        .m_axi_bready(master_d.b_ready),
        .m_axi_rready(master_d.r_ready),
        .m_axi_wvalid(master_d.w_valid),
        .m_axi_arprot(),
        .m_axi_awprot(),
        .m_axi_araddr(master_d.ar_addr),
        .m_axi_awaddr(master_d.aw_addr),
        .m_axi_wdata(master_d.w_data),
        .m_axi_wstrb(master_d.w_strb)
    );

    // Instantiate the MOR1KX
    // Parameter overrides are used to force non-bursting WB behavior as
    // bursts are NOT supported by AXI4-Lite
    mor1kx #(
        .IBUS_WB_TYPE               ("CLASSIC"),
        .DBUS_WB_TYPE               ("CLASSIC")
    ) mor1kx_inst (
        // Clock and Reset
        .clk                        (clk_i),
        .rst                        (rst_i),
    
        // Wishbone interface
        .iwbm_adr_o                 (wbm_i_or1k_adr_o),
        .iwbm_stb_o                 (wbm_i_or1k_stb_o),
        .iwbm_cyc_o                 (wbm_i_or1k_cyc_o),
        .iwbm_sel_o                 (wbm_i_or1k_sel_o),
        .iwbm_we_o                  (wbm_i_or1k_we_o),
        .iwbm_cti_o                 (wbm_i_or1k_cti_o),
        .iwbm_bte_o                 (wbm_i_or1k_bte_o),
        .iwbm_dat_o                 (wbm_i_or1k_dat_o),
        .iwbm_err_i                 (wbm_i_or1k_err_i),
        .iwbm_ack_i                 (wbm_i_or1k_ack_i),
        .iwbm_dat_i                 (wbm_i_or1k_dat_i),
        .iwbm_rty_i                 (wbm_i_or1k_rty_i),
    
        .dwbm_adr_o                 (wbm_d_or1k_adr_o),
        .dwbm_stb_o                 (wbm_d_or1k_stb_o),
        .dwbm_cyc_o                 (wbm_d_or1k_cyc_o),
        .dwbm_sel_o                 (wbm_d_or1k_sel_o),
        .dwbm_we_o                  (wbm_d_or1k_we_o),
        .dwbm_cti_o                 (wbm_d_or1k_cti_o),
        .dwbm_bte_o                 (wbm_d_or1k_bte_o),
        .dwbm_dat_o                 (wbm_d_or1k_dat_o),
        .dwbm_err_i                 (wbm_d_or1k_err_i),
        .dwbm_ack_i                 (wbm_d_or1k_ack_i),
        .dwbm_dat_i                 (wbm_d_or1k_dat_i),
        .dwbm_rty_i                 (wbm_d_or1k_rty_i),
    
        // Interrupts
        .irq_i                      (mor1kx_pic_ints),
    
        // Debug interface
        .du_addr_i                  (mor1kx_dbg_adr_i),
        .du_stb_i                   (mor1kx_dbg_stb_i),
        .du_dat_i                   (mor1kx_dbg_dat_i),
        .du_we_i                    (mor1kx_dbg_we_i),
        .du_dat_o                   (mor1kx_dbg_dat_o),
        .du_ack_o                   (mor1kx_dbg_ack_o),
    
        // Stall control from debug interface
        .du_stall_i                 (mor1kx_dbg_stall_i),
        .du_stall_o                 (mor1kx_dbg_stall_o),
    
        // Traceport (UNUSED)
        .traceport_exec_valid_o     (),
        .traceport_exec_pc_o        (),
        .traceport_exec_jb_o        (),
        .traceport_exec_jal_o       (),
        .traceport_exec_jr_o        (),
        .traceport_exec_jbtarget_o  (),
        .traceport_exec_insn_o      (),
        .traceport_exec_wbdata_o    (),
        .traceport_exec_wbreg_o     (),
        .traceport_exec_wben_o      (),
    
        // The multicore core identifier
        .multicore_coreid_i         (`OR1K_INSN_WIDTH'd0),
        
        // The number of cores
        .multicore_numcores_i       (`OR1K_INSN_WIDTH'd0),
    
        // Snoop interface (UNUSED)
        .snoop_adr_i                (32'h0000_0000),
        .snoop_en_i                 (1'b0)
        );

endmodule // mor1kx_axi4lite