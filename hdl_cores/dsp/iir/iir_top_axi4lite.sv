//
// Copyright (C) 2018 Massachusetts Institute of Technology
//
// File         : iir_top_axi4lite.v
// Project      : Common Evaluation Platform (CEP)
// Description  : This file provides an axi4-lite wrapper for the wishbone based-IIR core
//

module iir_top_axi4lite (

    // Clock & Reset
    input logic     clk_i,
    input logic     rst_ni,    

    // AXI4-Lite Slave Interface
    AXI_LITE.in     slave

);

    // AES wishbone slave wires
    wire                                wb_clk;
    wire                                wb_rst;
    wire [slave.AXI_ADDR_WIDTH - 1:0]   wbs_d_iir_adr_i;
    wire [slave.AXI_DATA_WIDTH - 1:0]   wbs_d_iir_dat_i;
    wire [3:0]                          wbs_d_iir_sel_i;
    wire                                wbs_d_iir_we_i;
    wire                                wbs_d_iir_cyc_i;
    wire                                wbs_d_iir_stb_i;
    wire [slave.AXI_DATA_WIDTH - 1:0]   wbs_d_iir_dat_o;
    wire                                wbs_d_iir_ack_o;
    wire                                wbs_d_iir_err_o;

    // Instantiate the AXI4Lite to Wishbone bridge
    bonfire_axi4l2wb #(
        .ADRWIDTH       (slave.AXI_ADDR_WIDTH),
        .FAST_READ_TERM (1'd0)
    ) bonfire_axi4l2wb_inst (
        .S_AXI_ACLK     (clk_i),
        .S_AXI_ARESETN  (rst_ni),
        .S_AXI_AWADDR   (slave.aw_addr),
        .S_AXI_AWVALID  (slave.aw_valid),
        .S_AXI_AWREADY  (slave.aw_ready),
        .S_AXI_WDATA    (slave.w_data),
        .S_AXI_WSTRB    (slave.w_strb),
        .S_AXI_WVALID   (slave.w_valid),
        .S_AXI_WREADY   (slave.w_ready),
        .S_AXI_ARADDR   (slave.ar_addr),
        .S_AXI_ARVALID  (slave.ar_valid),
        .S_AXI_ARREADY  (slave.ar_ready),
        .S_AXI_RDATA    (slave.r_data),
        .S_AXI_RRESP    (slave.r_resp),
        .S_AXI_RVALID   (slave.r_valid),
        .S_AXI_RREADY   (slave.r_ready),
        .S_AXI_BRESP    (slave.b_resp),
        .S_AXI_BVALID   (slave.b_valid),
        .S_AXI_BREADY   (slave.b_ready),
        .wb_clk_o       (wb_clk),
        .wb_rst_o       (wb_rst),
        .wb_addr_o      (wbs_d_iir_adr_i),
        .wb_dat_o       (wbs_d_iir_dat_i),
        .wb_we_o        (wbs_d_iir_we_i),
        .wb_sel_o       (wbs_d_iir_sel_i),
        .wb_stb_o       (wbs_d_iir_stb_i),
        .wb_cyc_o       (wbs_d_iir_cyc_i),
        .wb_dat_i       (wbs_d_iir_dat_o),
        .wb_ack_i       (wbs_d_iir_ack_o)
    );

    // Instantiate the wishbone-based DFT Core
    iir_top iir_top_inst (
        // Wishbone Slave interface
        .wb_clk_i       (wb_clk),
        .wb_rst_i       (wb_rst),
        .wb_dat_i       (wbs_d_iir_dat_i),
        .wb_adr_i       (wbs_d_iir_adr_i),
        .wb_sel_i       (wbs_d_iir_sel_i[3:0]),
        .wb_we_i        (wbs_d_iir_we_i),
        .wb_cyc_i       (wbs_d_iir_cyc_i),
        .wb_stb_i       (wbs_d_iir_stb_i),
        .wb_dat_o       (wbs_d_iir_dat_o),
        .wb_err_o       (),
        .wb_ack_o       (wbs_d_iir_ack_o),

        // Processor interrupt
        .int_o          ()
    );

endmodule   // end aes_top_axi4lite
