//
// Copyright (C) 2019 Massachusetts Institute of Technology
//
// File         : aes_top_axi4lite.v
// Project      : Common Evaluation Platform (CEP)
// Description  : This file provides an axi4-lite wrapper for the wishbone based-AES core
//

module aes_top_axi4lite (

    // Clock & Reset
    clk_i,
    rst_ni,    

    // AXI4 Slave Interface
    o_axi_awready,
    i_axi_awaddr, 
    i_axi_awcache, 
    i_axi_awprot, 
    i_axi_awvalid,

    o_axi_wready, 
    i_axi_wdata, 
    i_axi_wstrb, 
    i_axi_wvalid,

    o_axi_bresp, 
    o_axi_bvalid, 
    i_axi_bready,
    
    o_axi_arready,
    i_axi_araddr,
    i_axi_arcache,
    i_axi_arprot,
    i_axi_arvalid,

    o_axi_rresp,
    o_axi_rvalid,
    o_axi_rdata,
    i_axi_rready

);

    parameter C_AXI_DATA_WIDTH  = 32;   // Width of the AXI R&W data
    parameter C_AXI_ADDR_WIDTH  = 32;   // AXI Address width

    // Clocks and Resets
    input   wire                                clk_i;
    input   wire                                rst_ni;

    // AXI Slave Interface
    // AXI write address channel signals
    output  wire                                o_axi_awready;  // Slave is ready to accept
    input   wire    [C_AXI_ADDR_WIDTH-1:0]      i_axi_awaddr;   // Write address
    input   wire    [3:0]                       i_axi_awcache;  // Write Cache type
    input   wire    [2:0]                       i_axi_awprot;   // Write Protection type
    input   wire                                i_axi_awvalid;  // Write address valid

    // AXI write data channel signals
    output  wire                                o_axi_wready;   // Write data ready
    input   wire    [C_AXI_DATA_WIDTH-1:0]      i_axi_wdata;    // Write data
    input   wire    [C_AXI_DATA_WIDTH/8-1:0]    i_axi_wstrb;    // Write strobes
    input   wire                                i_axi_wvalid;   // Write valid

    // AXI write response channel signals
    output  wire    [1:0]                       o_axi_bresp;    // Write response
    output  wire                                o_axi_bvalid;   // Write reponse valid
    input   wire                                i_axi_bready;   // Response ready

    // AXI read address channel signals
    output  wire                                o_axi_arready;  // Read address ready
    input   wire    [C_AXI_ADDR_WIDTH-1:0]      i_axi_araddr;   // Read address
    input   wire    [3:0]                       i_axi_arcache;  // Read Cache type
    input   wire    [2:0]                       i_axi_arprot;   // Read Protection type
    input   wire                                i_axi_arvalid;  // Read address valid

    // AXI read data channel signals
    output  wire [1:0]                          o_axi_rresp;    // Read response
    output  wire                                o_axi_rvalid;   // Read reponse valid
    output  wire [C_AXI_DATA_WIDTH-1:0]         o_axi_rdata;    // Read data
    input   wire                                i_axi_rready;   // Read Response ready

    // Wishbone slave wires
    wire                                wb_rst;
    wire [C_AXI_ADDR_WIDTH - 3:0]       wbs_adr_i;
    wire [C_AXI_DATA_WIDTH - 1:0]       wbs_dat_i;
    wire [3:0]                          wbs_sel_i;
    wire                                wbs_we_i;
    wire                                wbs_cyc_i;
    wire                                wbs_stb_i;
    wire [C_AXI_DATA_WIDTH - 1:0]       wbs_dat_o;
    wire                                wbs_err_o;
    wire                                wbs_ack_o;

    // Instantiation of the pipelined AXI4 to Wishbone converter
    axlite2wbsp # (
        .C_AXI_DATA_WIDTH               (C_AXI_DATA_WIDTH),     // Width of the AXI R&W data
        .C_AXI_ADDR_WIDTH               (C_AXI_ADDR_WIDTH)      // AXI Address Width
    ) axlite2wbsp_inst ( 
        .i_clk                          (clk_i), 
        .i_axi_reset_n                  (rst_ni),

        // AXI4 Slave Interface
        .o_axi_awready                  (o_axi_awready),
        .i_axi_awaddr                   (i_axi_awaddr), 
        .i_axi_awcache                  (i_axi_awcache), 
        .i_axi_awprot                   (i_axi_awprot), 
        .i_axi_awvalid                  (i_axi_awvalid),

        .o_axi_wready                   (o_axi_wready), 
        .i_axi_wdata                    (i_axi_wdata), 
        .i_axi_wstrb                    (i_axi_wstrb), 
        .i_axi_wvalid                   (i_axi_wvalid),

        .o_axi_bresp                    (o_axi_bresp), 
        .o_axi_bvalid                   (o_axi_bvalid), 
        .i_axi_bready                   (i_axi_bready),

        .o_axi_arready                  (o_axi_arready),
        .i_axi_araddr                   (i_axi_araddr),
        .i_axi_arcache                  (i_axi_arcache),
        .i_axi_arprot                   (i_axi_arprot),
        .i_axi_arvalid                  (i_axi_arvalid),

        .o_axi_rresp                    (o_axi_rresp),
        .o_axi_rvalid                   (o_axi_rvalid),
        .o_axi_rdata                    (o_axi_rdata),
        .i_axi_rready                   (i_axi_rready),

        // Wishbone Master Interface
        .o_reset                        (wb_rst), 
        .o_wb_cyc                       (wbs_cyc_i), 
        .o_wb_stb                       (wbs_stb_i), 
        .o_wb_we                        (wbs_we_i), 
        .o_wb_addr                      (wbs_adr_i), 
        .o_wb_data                      (wbs_dat_i), 
        .o_wb_sel                       (wbs_sel_i),
        .i_wb_ack                       (wbs_ack_o), 
        .i_wb_stall                     (1'b0), 
        .i_wb_data                      (wbs_dat_o), 
        .i_wb_err                       (wbs_err_o)
    );
    
    // Instantiate the wishbone-based AES Core
    aes_top_wb #(
        .AW                             (C_AXI_ADDR_WIDTH - 2),
        .DW                             (C_AXI_DATA_WIDTH)
    ) aes_top_wb_inst (
        // Wishbone Slave interface
        .wb_clk_i                       (clk_i),
        .wb_rst_i                       (wb_rst),
        .wb_dat_i                       (wbs_dat_i),
        .wb_adr_i                       (wbs_adr_i),
        .wb_sel_i                       (wbs_sel_i[3:0]),
        .wb_we_i                        (wbs_we_i),
        .wb_cyc_i                       (wbs_cyc_i),
        .wb_stb_i                       (wbs_stb_i),
        .wb_dat_o                       (wbs_dat_o),
        .wb_err_o                       (wbs_err_o),
        .wb_ack_o                       (wbs_ack_o),

        // Processor interrupt
        .int_o                          ()
    );

endmodule   // end aes_top_axi4lite
