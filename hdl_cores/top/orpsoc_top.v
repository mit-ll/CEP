//////////////////////////////////////////////////////////////////////
///                                                               ////
/// ORPSoC top for ML501 board                                    ////
///                                                               ////
/// Instantiates modules, depending on ORPSoC defines file        ////
///                                                               ////
/// Julius Baxter, julius@opencores.org                           ////
///                                                               ////
//////////////////////////////////////////////////////////////////////
////                                                              ////
//// Copyright (C) 2009, 2010 Authors and OPENCORES.ORG           ////
////                                                              ////
//// This source file may be used and distributed without         ////
//// restriction provided that this copyright statement is not    ////
//// removed from the file and that any derivative work contains  ////
//// the original copyright notice and the associated disclaimer. ////
////                                                              ////
//// This source file is free software; you can redistribute it   ////
//// and/or modify it under the terms of the GNU Lesser General   ////
//// Public License as published by the Free Software Foundation; ////
//// either version 2.1 of the License, or (at your option) any   ////
//// later version.                                               ////
////                                                              ////
//// This source is distributed in the hope that it will be       ////
//// useful, but WITHOUT ANY WARRANTY; without even the implied   ////
//// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      ////
//// PURPOSE.  See the GNU Lesser General Public License for more ////
//// details.                                                     ////
////                                                              ////
//// You should have received a copy of the GNU Lesser General    ////
//// Public License along with this source; if not, download it   ////
//// from http://www.opencores.org/lgpl.shtml                     ////
////                                                              ////
//////////////////////////////////////////////////////////////////////

`include "orpsoc-defines.v"

module orpsoc_top (

    sys_clk_in_p, sys_clk_in_n,

`ifdef UART0
    uart0_srx_pad_i, uart0_stx_pad_o, uart0_rts_pad_o, uart0_cts_pad_i,
`endif

`ifdef DEBUGGING_GPIO
    GPIO_LED, button_W, button_N, button_E, button_S,
`endif

`ifdef RESET_HIGH
    rst_pad_i
`else
    rst_n_pad_i
`endif
 );

`include "orpsoc-params.v"
`include "mor1kx-defines.v"

input sys_clk_in_p,sys_clk_in_n;

`ifdef RESET_HIGH
    input       rst_pad_i;
`else
    input       rst_n_pad_i;
`endif

`ifdef UART0
    input       uart0_srx_pad_i;
    output      uart0_stx_pad_o;
    output      uart0_rts_pad_o;
    input       uart0_cts_pad_i;
`endif

`ifdef DEBUGGING_GPIO
    output [7:0]                GPIO_LED;
    input                       button_W, button_N, button_E, button_S;
`endif

// If not currently targeting synthesis, override the SIM parameter for UART0
`ifndef SYNTHESIS
    defparam uart16550_0.SIM = 1'b1;
`endif

////////////////////////////////////////////////////////////////////////
//
// Clock and reset generation module
//
////////////////////////////////////////////////////////////////////////
//
// Wires
//
wire    wb_clk, wb_rst;
wire    ddr2_if_clk, ddr2_if_rst;
wire    clk200;
wire    dbg_tck;

clkgen clkgen0 (
    .sys_clk_in_p           (sys_clk_in_p),
    .sys_clk_in_n           (sys_clk_in_n),

    .wb_clk_o               (wb_clk),
    .wb_rst_o               (wb_rst),

    // Asynchronous reset (Active Low or Active High)
`ifdef RESET_HIGH
     .rst_n_pad_i           (~rst_pad_i)
`else
     .rst_n_pad_i           (rst_n_pad_i)
`endif
);
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// Arbiter
//
////////////////////////////////////////////////////////////////////////

// Wire naming convention:
// First: wishbone master or slave (wbm/wbs)
// Second: Which bus it's on instruction or data (i/d)
// Third: Between which module and the arbiter the wires are
// Fourth: Signal name
// Fifth: Direction relative to module (not bus/arbiter!)
//        ie. wbm_d_or1k_adr_o is address OUT from the mor1kx

// mor1kx instruction bus wires
wire [wb_aw-1:0]        wbm_i_or1k_adr_o;
wire [wb_dw-1:0]        wbm_i_or1k_dat_o;
wire [3:0]              wbm_i_or1k_sel_o;
wire                    wbm_i_or1k_we_o;
wire                    wbm_i_or1k_cyc_o;
wire                    wbm_i_or1k_stb_o;
wire [2:0]              wbm_i_or1k_cti_o;
wire [1:0]              wbm_i_or1k_bte_o;

wire [wb_dw-1:0]        wbm_i_or1k_dat_i;
wire                    wbm_i_or1k_ack_i;
wire                    wbm_i_or1k_err_i;
wire                    wbm_i_or1k_rty_i;

// mor1kx data bus wires
wire [wb_aw-1:0]        wbm_d_or1k_adr_o;
wire [wb_dw-1:0]        wbm_d_or1k_dat_o;
wire [3:0]              wbm_d_or1k_sel_o;
wire                    wbm_d_or1k_we_o;
wire                    wbm_d_or1k_cyc_o;
wire                    wbm_d_or1k_stb_o;
wire [2:0]              wbm_d_or1k_cti_o;
wire [1:0]              wbm_d_or1k_bte_o;

wire [wb_dw-1:0]        wbm_d_or1k_dat_i;
wire                    wbm_d_or1k_ack_i;
wire                    wbm_d_or1k_err_i;
wire                    wbm_d_or1k_rty_i;

// Byte bus bridge master signals
wire [wb_aw-1:0]        wbm_b_d_adr_o;
wire [wb_dw-1:0]        wbm_b_d_dat_o;
wire [3:0]              wbm_b_d_sel_o;
wire                    wbm_b_d_we_o;
wire                    wbm_b_d_cyc_o;
wire                    wbm_b_d_stb_o;
wire [2:0]              wbm_b_d_cti_o;
wire [1:0]              wbm_b_d_bte_o;

wire [wb_dw-1:0]        wbm_b_d_dat_i;
wire                    wbm_b_d_ack_i;
wire                    wbm_b_d_err_i;
wire                    wbm_b_d_rty_i;

// Instruction bus slave wires //

// ram0 instruction bus wires
wire [31:0]             wbs_i_ram0_adr_i;
wire [wbs_i_rom0_data_width-1:0]    wbs_i_ram0_dat_i;
wire [3:0]              wbs_i_ram0_sel_i;
wire                    wbs_i_ram0_we_i;
wire                    wbs_i_ram0_cyc_i;
wire                    wbs_i_ram0_stb_i;
wire [2:0]              wbs_i_ram0_cti_i;
wire [1:0]              wbs_i_ram0_bte_i;
wire [wbs_i_rom0_data_width-1:0]    wbs_i_ram0_dat_o;
wire                    wbs_i_ram0_ack_o;
wire                    wbs_i_ram0_err_o;
wire                    wbs_i_ram0_rty_o;

// Data bus slave wires //

// ram0 data bus wires
wire [31:0]             wbs_d_ram0_adr_i;
wire [wbs_d_rom0_data_width-1:0]    wbs_d_ram0_dat_i;
wire [3:0]              wbs_d_ram0_sel_i;
wire                    wbs_d_ram0_we_i;
wire                    wbs_d_ram0_cyc_i;
wire                    wbs_d_ram0_stb_i;
wire [2:0]              wbs_d_ram0_cti_i;
wire [1:0]              wbs_d_ram0_bte_i;
wire [wbs_d_rom0_data_width-1:0]    wbs_d_ram0_dat_o;
wire                    wbs_d_ram0_ack_o;
wire                    wbs_d_ram0_err_o;
wire                    wbs_d_ram0_rty_o;

// uart0 wires
wire [31:0]               wbs_d_uart0_adr_i;
wire [wbs_d_uart0_data_width-1:0] wbs_d_uart0_dat_i;
wire [3:0]                wbs_d_uart0_sel_i;
wire                  wbs_d_uart0_we_i;
wire                  wbs_d_uart0_cyc_i;
wire                  wbs_d_uart0_stb_i;
wire [2:0]                wbs_d_uart0_cti_i;
wire [1:0]                wbs_d_uart0_bte_i;
wire [wbs_d_uart0_data_width-1:0] wbs_d_uart0_dat_o;
wire                  wbs_d_uart0_ack_o;
wire                  wbs_d_uart0_err_o;
wire                  wbs_d_uart0_rty_o;

// AES slave wires
wire [31:0]                wbs_d_aes_adr_i;
wire [wbs_d_aes_data_width-1:0]    wbs_d_aes_dat_i;
wire [3:0]                 wbs_d_aes_sel_i;
wire                   wbs_d_aes_we_i;
wire                   wbs_d_aes_cyc_i;
wire                   wbs_d_aes_stb_i;
wire [2:0]                 wbs_d_aes_cti_i;
wire [1:0]                 wbs_d_aes_bte_i;
wire [wbs_d_aes_data_width-1:0]    wbs_d_aes_dat_o;
wire                   wbs_d_aes_ack_o;
wire                   wbs_d_aes_err_o;
wire                   wbs_d_aes_rty_o;
wire aes_irq;

// MD5 slave wires
wire [31:0]                wbs_d_md5_adr_i;
wire [wbs_d_md5_data_width-1:0]    wbs_d_md5_dat_i;
wire [3:0]                 wbs_d_md5_sel_i;
wire                   wbs_d_md5_we_i;
wire                   wbs_d_md5_cyc_i;
wire                   wbs_d_md5_stb_i;
wire [2:0]                 wbs_d_md5_cti_i;
wire [1:0]                 wbs_d_md5_bte_i;
wire [wbs_d_md5_data_width-1:0]    wbs_d_md5_dat_o;
wire                   wbs_d_md5_ack_o;
wire                   wbs_d_md5_err_o;
wire                   wbs_d_md5_rty_o;
wire md5_irq;

// SHA slave wires
wire [31:0]                wbs_d_sha_adr_i;
wire [wbs_d_sha_data_width-1:0]    wbs_d_sha_dat_i;
wire [3:0]                 wbs_d_sha_sel_i;
wire                   wbs_d_sha_we_i;
wire                   wbs_d_sha_cyc_i;
wire                   wbs_d_sha_stb_i;
wire [2:0]                 wbs_d_sha_cti_i;
wire [1:0]                 wbs_d_sha_bte_i;
wire [wbs_d_sha_data_width-1:0]    wbs_d_sha_dat_o;
wire                   wbs_d_sha_ack_o;
wire                   wbs_d_sha_err_o;
wire                   wbs_d_sha_rty_o;
wire sha_irq;

// RSA slave wires
wire [31:0]                wbs_d_rsa_adr_i;
wire [wbs_d_rsa_data_width-1:0]    wbs_d_rsa_dat_i;
wire [3:0]                 wbs_d_rsa_sel_i;
wire                   wbs_d_rsa_we_i;
wire                   wbs_d_rsa_cyc_i;
wire                   wbs_d_rsa_stb_i;
wire [2:0]                 wbs_d_rsa_cti_i;
wire [1:0]                 wbs_d_rsa_bte_i;
wire [wbs_d_rsa_data_width-1:0]    wbs_d_rsa_dat_o;
wire                   wbs_d_rsa_ack_o;
wire                   wbs_d_rsa_err_o;
wire                   wbs_d_rsa_rty_o;
wire rsa_irq;

// DES3 slave wires
wire [31:0]                wbs_d_des3_adr_i;
wire [wbs_d_des3_data_width-1:0]       wbs_d_des3_dat_i;
wire [3:0]                 wbs_d_des3_sel_i;
wire                   wbs_d_des3_we_i;
wire                   wbs_d_des3_cyc_i;
wire                   wbs_d_des3_stb_i;
wire [2:0]                 wbs_d_des3_cti_i;
wire [1:0]                 wbs_d_des3_bte_i;
wire [wbs_d_des3_data_width-1:0]       wbs_d_des3_dat_o;
wire                   wbs_d_des3_ack_o;
wire                   wbs_d_des3_err_o;
wire                   wbs_d_des3_rty_o;
wire des3_irq;

// DFT slave wires
wire [31:0]                wbs_d_dft_adr_i;
wire [wbs_d_dft_data_width-1:0]    wbs_d_dft_dat_i;
wire [3:0]                 wbs_d_dft_sel_i;
wire                   wbs_d_dft_we_i;
wire                   wbs_d_dft_cyc_i;
wire                   wbs_d_dft_stb_i;
wire [2:0]                 wbs_d_dft_cti_i;
wire [1:0]                 wbs_d_dft_bte_i;
wire [wbs_d_dft_data_width-1:0]    wbs_d_dft_dat_o;
wire                   wbs_d_dft_ack_o;
wire                   wbs_d_dft_err_o;
wire                   wbs_d_dft_rty_o;
wire dft_irq;

// iDFT slave wires
wire [31:0]                wbs_d_idft_adr_i;
wire [wbs_d_idft_data_width-1:0]       wbs_d_idft_dat_i;
wire [3:0]                 wbs_d_idft_sel_i;
wire                   wbs_d_idft_we_i;
wire                   wbs_d_idft_cyc_i;
wire                   wbs_d_idft_stb_i;
wire [2:0]                 wbs_d_idft_cti_i;
wire [1:0]                 wbs_d_idft_bte_i;
wire [wbs_d_idft_data_width-1:0]       wbs_d_idft_dat_o;
wire                   wbs_d_idft_ack_o;
wire                   wbs_d_idft_err_o;
wire                   wbs_d_idft_rty_o;
wire idft_irq;

// FIR slave wires
wire [31:0]                wbs_d_fir_adr_i;
wire [wbs_d_fir_data_width-1:0]    wbs_d_fir_dat_i;
wire [3:0]                 wbs_d_fir_sel_i;
wire                   wbs_d_fir_we_i;
wire                   wbs_d_fir_cyc_i;
wire                   wbs_d_fir_stb_i;
wire [2:0]                 wbs_d_fir_cti_i;
wire [1:0]                 wbs_d_fir_bte_i;
wire [wbs_d_fir_data_width-1:0]    wbs_d_fir_dat_o;
wire                   wbs_d_fir_ack_o;
wire                   wbs_d_fir_err_o;
wire                   wbs_d_fir_rty_o;
wire fir_irq;

// IIR slave wires
wire [31:0]                wbs_d_iir_adr_i;
wire [wbs_d_iir_data_width-1:0]    wbs_d_iir_dat_i;
wire [3:0]                 wbs_d_iir_sel_i;
wire                   wbs_d_iir_we_i;
wire                   wbs_d_iir_cyc_i;
wire                   wbs_d_iir_stb_i;
wire [2:0]                 wbs_d_iir_cti_i;
wire [1:0]                 wbs_d_iir_bte_i;
wire [wbs_d_iir_data_width-1:0]    wbs_d_iir_dat_o;
wire                   wbs_d_iir_ack_o;
wire                   wbs_d_iir_err_o;
wire                   wbs_d_iir_rty_o;
wire iir_irq;

// GPS slave wires
wire [31:0]                wbs_d_gps_adr_i;
wire [wbs_d_gps_data_width-1:0]    wbs_d_gps_dat_i;
wire [3:0]                 wbs_d_gps_sel_i;
wire                   wbs_d_gps_we_i;
wire                   wbs_d_gps_cyc_i;
wire                   wbs_d_gps_stb_i;
wire [2:0]                 wbs_d_gps_cti_i;
wire [1:0]                 wbs_d_gps_bte_i;
wire [wbs_d_gps_data_width-1:0]    wbs_d_gps_dat_o;
wire                   wbs_d_gps_ack_o;
wire                   wbs_d_gps_err_o;
wire                   wbs_d_gps_rty_o;
wire gps_irq;

// Wires used to "stub" unused arbiter I/O
wire [wb_dw - 1:0]     empty_slot_dat_o = 0;
wire [wb_aw - 1:0]     empty_slot_adr_o = 0;
wire                   empty_slot_ack_o = 0;
wire                   empty_slot_err_o = 0;
wire                   empty_slot_rty_o = 0;

////////////////////////////////////////////////////////////////////////
//
// Wishbone instruction bus arbiter
//
////////////////////////////////////////////////////////////////////////
arbiter_ibus arbiter_ibus0
             (
                 // Instruction Bus Master
                 // Inputs to arbiter from master
                 .wbm_adr_o            (wbm_i_or1k_adr_o),
                 .wbm_dat_o            (wbm_i_or1k_dat_o),
                 .wbm_sel_o            (wbm_i_or1k_sel_o),
                 .wbm_we_o             (wbm_i_or1k_we_o),
                 .wbm_cyc_o            (wbm_i_or1k_cyc_o),
                 .wbm_stb_o            (wbm_i_or1k_stb_o),
                 .wbm_cti_o            (wbm_i_or1k_cti_o),
                 .wbm_bte_o            (wbm_i_or1k_bte_o),
                 // Outputs to master from arbiter
                 .wbm_dat_i            (wbm_i_or1k_dat_i),
                 .wbm_ack_i            (wbm_i_or1k_ack_i),
                 .wbm_err_i            (wbm_i_or1k_err_i),
                 .wbm_rty_i            (wbm_i_or1k_rty_i),

                 // Slave 0
                 // Inputs to slave from arbiter
                 .wbs0_adr_i           (wbs_i_ram0_adr_i),
                 .wbs0_dat_i           (wbs_i_ram0_dat_i),
                 .wbs0_sel_i           (wbs_i_ram0_sel_i),
                 .wbs0_we_i            (wbs_i_ram0_we_i),
                 .wbs0_cyc_i           (wbs_i_ram0_cyc_i),
                 .wbs0_stb_i           (wbs_i_ram0_stb_i),
                 .wbs0_cti_i           (wbs_i_ram0_cti_i),
                 .wbs0_bte_i           (wbs_i_ram0_bte_i),
                 // Outputs from slave to arbiter
                 .wbs0_dat_o           (wbs_i_ram0_dat_o),
                 .wbs0_ack_o           (wbs_i_ram0_ack_o),
                 .wbs0_err_o           (wbs_i_ram0_err_o),
                 .wbs0_rty_o           (wbs_i_ram0_rty_o),

                 // Slave 1
                 // Inputs to slave from arbiter
                 .wbs1_adr_i           (),
                 .wbs1_dat_i           (),
                 .wbs1_sel_i           (),
                 .wbs1_we_i            (),
                 .wbs1_cyc_i           (),
                 .wbs1_stb_i           (),
                 .wbs1_cti_i           (),
                 .wbs1_bte_i           (),
                 // Outputs from slave to arbiter
                 .wbs1_dat_o           (empty_slot_dat_o),
                 .wbs1_ack_o           (empty_slot_ack_o),
                 .wbs1_err_o           (empty_slot_err_o),
                 .wbs1_rty_o           (empty_slot_rty_o),

                 // Clock, reset inputs
                 .wb_clk               (wb_clk),
                 .wb_rst               (wb_rst));

defparam arbiter_ibus0.slave0_addr_width = ibus_arb_slave0_addr_width;
defparam arbiter_ibus0.slave1_addr_width = ibus_arb_slave1_addr_width;
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// Wishbone data bus arbiter
//
////////////////////////////////////////////////////////////////////////
arbiter_dbus arbiter_dbus0
             (
                 // Master 0
                 // Inputs to arbiter from master
                 .wbm0_adr_o           (wbm_d_or1k_adr_o),
                 .wbm0_dat_o           (wbm_d_or1k_dat_o),
                 .wbm0_sel_o           (wbm_d_or1k_sel_o),
                 .wbm0_we_o            (wbm_d_or1k_we_o),
                 .wbm0_cyc_o           (wbm_d_or1k_cyc_o),
                 .wbm0_stb_o           (wbm_d_or1k_stb_o),
                 .wbm0_cti_o           (wbm_d_or1k_cti_o),
                 .wbm0_bte_o           (wbm_d_or1k_bte_o),

                 // Outputs to master from arbiter
                 .wbm0_dat_i           (wbm_d_or1k_dat_i),
                 .wbm0_ack_i           (wbm_d_or1k_ack_i),
                 .wbm0_err_i           (wbm_d_or1k_err_i),
                 .wbm0_rty_i           (wbm_d_or1k_rty_i),

                 // Data Bus Master 1 unused
                 .wbm1_adr_o           (empty_slot_adr_o),
                 .wbm1_dat_o           (empty_slot_dat_o),
                 .wbm1_we_o            (1'b0),
                 .wbm1_cyc_o           (1'b0),
                 .wbm1_sel_o           (4'b0),
                 .wbm1_stb_o           (1'b0),
                 .wbm1_cti_o           (3'd0),
                 .wbm1_bte_o           (2'd0),

                 .wbm1_dat_i           (),
                 .wbm1_ack_i           (),
                 .wbm1_err_i           (),
                 .wbm1_rty_i           (),

                 // Slaves
                 .wbs0_adr_i           (),
                 .wbs0_dat_i           (),
                 .wbs0_sel_i           (),
                 .wbs0_we_i            (),
                 .wbs0_cyc_i           (),
                 .wbs0_stb_i           (),
                 .wbs0_cti_i           (),
                 .wbs0_bte_i           (),
                 .wbs0_dat_o           (empty_slot_dat_o),
                 .wbs0_ack_o           (empty_slot_ack_o),
                 .wbs0_err_o           (empty_slot_err_o),
                 .wbs0_rty_o           (empty_slot_rty_o),

                 .wbs1_adr_i           (wbs_d_aes_adr_i),
                 .wbs1_dat_i           (wbs_d_aes_dat_i),
                 .wbs1_sel_i           (wbs_d_aes_sel_i),
                 .wbs1_we_i            (wbs_d_aes_we_i),
                 .wbs1_cyc_i           (wbs_d_aes_cyc_i),
                 .wbs1_stb_i           (wbs_d_aes_stb_i),
                 .wbs1_cti_i           (wbs_d_aes_cti_i),
                 .wbs1_bte_i           (wbs_d_aes_bte_i),
                 .wbs1_dat_o           (wbs_d_aes_dat_o),
                 .wbs1_ack_o           (wbs_d_aes_ack_o),
                 .wbs1_err_o           (wbs_d_aes_err_o),
                 .wbs1_rty_o           (wbs_d_aes_rty_o),

                 .wbs2_adr_i           (wbm_b_d_adr_o),
                 .wbs2_dat_i           (wbm_b_d_dat_o),
                 .wbs2_sel_i           (wbm_b_d_sel_o),
                 .wbs2_we_i            (wbm_b_d_we_o),
                 .wbs2_cyc_i           (wbm_b_d_cyc_o),
                 .wbs2_stb_i           (wbm_b_d_stb_o),
                 .wbs2_cti_i           (wbm_b_d_cti_o),
                 .wbs2_bte_i           (wbm_b_d_bte_o),
                 .wbs2_dat_o           (wbm_b_d_dat_i),
                 .wbs2_ack_o           (wbm_b_d_ack_i),
                 .wbs2_err_o           (wbm_b_d_err_i),
                 .wbs2_rty_o           (wbm_b_d_rty_i),

                 .wbs3_adr_i           (wbs_d_ram0_adr_i),
                 .wbs3_dat_i           (wbs_d_ram0_dat_i),
                 .wbs3_sel_i           (wbs_d_ram0_sel_i),
                 .wbs3_we_i            (wbs_d_ram0_we_i),
                 .wbs3_cyc_i           (wbs_d_ram0_cyc_i),
                 .wbs3_stb_i           (wbs_d_ram0_stb_i),
                 .wbs3_cti_i           (wbs_d_ram0_cti_i),
                 .wbs3_bte_i           (wbs_d_ram0_bte_i),
                 .wbs3_dat_o           (wbs_d_ram0_dat_o),
                 .wbs3_ack_o           (wbs_d_ram0_ack_o),
                 .wbs3_err_o           (wbs_d_ram0_err_o),
                 .wbs3_rty_o           (wbs_d_ram0_rty_o),

                 .wbs4_adr_i           (wbs_d_md5_adr_i),
                 .wbs4_dat_i           (wbs_d_md5_dat_i),
                 .wbs4_sel_i           (wbs_d_md5_sel_i),
                 .wbs4_we_i            (wbs_d_md5_we_i),
                 .wbs4_cyc_i           (wbs_d_md5_cyc_i),
                 .wbs4_stb_i           (wbs_d_md5_stb_i),
                 .wbs4_cti_i           (wbs_d_md5_cti_i),
                 .wbs4_bte_i           (wbs_d_md5_bte_i),
                 .wbs4_dat_o           (wbs_d_md5_dat_o),
                 .wbs4_ack_o           (wbs_d_md5_ack_o),
                 .wbs4_err_o           (wbs_d_md5_err_o),
                 .wbs4_rty_o           (wbs_d_md5_rty_o),

                 .wbs5_adr_i           (wbs_d_sha_adr_i),
                 .wbs5_dat_i           (wbs_d_sha_dat_i),
                 .wbs5_sel_i           (wbs_d_sha_sel_i),
                 .wbs5_we_i            (wbs_d_sha_we_i),
                 .wbs5_cyc_i           (wbs_d_sha_cyc_i),
                 .wbs5_stb_i           (wbs_d_sha_stb_i),
                 .wbs5_cti_i           (wbs_d_sha_cti_i),
                 .wbs5_bte_i           (wbs_d_sha_bte_i),
                 .wbs5_dat_o           (wbs_d_sha_dat_o),
                 .wbs5_ack_o           (wbs_d_sha_ack_o),
                 .wbs5_err_o           (wbs_d_sha_err_o),
                 .wbs5_rty_o           (wbs_d_sha_rty_o),

                 .wbs6_adr_i           (wbs_d_rsa_adr_i),
                 .wbs6_dat_i           (wbs_d_rsa_dat_i),
                 .wbs6_sel_i           (wbs_d_rsa_sel_i),
                 .wbs6_we_i            (wbs_d_rsa_we_i),
                 .wbs6_cyc_i           (wbs_d_rsa_cyc_i),
                 .wbs6_stb_i           (wbs_d_rsa_stb_i),
                 .wbs6_cti_i           (wbs_d_rsa_cti_i),
                 .wbs6_bte_i           (wbs_d_rsa_bte_i),
                 .wbs6_dat_o           (wbs_d_rsa_dat_o),
                 .wbs6_ack_o           (wbs_d_rsa_ack_o),
                 .wbs6_err_o           (wbs_d_rsa_err_o),
                 .wbs6_rty_o           (wbs_d_rsa_rty_o),

                 .wbs7_adr_i           (wbs_d_des3_adr_i),
                 .wbs7_dat_i           (wbs_d_des3_dat_i),
                 .wbs7_sel_i           (wbs_d_des3_sel_i),
                 .wbs7_we_i            (wbs_d_des3_we_i),
                 .wbs7_cyc_i           (wbs_d_des3_cyc_i),
                 .wbs7_stb_i           (wbs_d_des3_stb_i),
                 .wbs7_cti_i           (wbs_d_des3_cti_i),
                 .wbs7_bte_i           (wbs_d_des3_bte_i),
                 .wbs7_dat_o           (wbs_d_des3_dat_o),
                 .wbs7_ack_o           (wbs_d_des3_ack_o),
                 .wbs7_err_o           (wbs_d_des3_err_o),
                 .wbs7_rty_o           (wbs_d_des3_rty_o),

                 .wbs8_adr_i           (wbs_d_dft_adr_i),
                 .wbs8_dat_i           (wbs_d_dft_dat_i),
                 .wbs8_sel_i           (wbs_d_dft_sel_i),
                 .wbs8_we_i            (wbs_d_dft_we_i),
                 .wbs8_cyc_i           (wbs_d_dft_cyc_i),
                 .wbs8_stb_i           (wbs_d_dft_stb_i),
                 .wbs8_cti_i           (wbs_d_dft_cti_i),
                 .wbs8_bte_i           (wbs_d_dft_bte_i),
                 .wbs8_dat_o           (wbs_d_dft_dat_o),
                 .wbs8_ack_o           (wbs_d_dft_ack_o),
                 .wbs8_err_o           (wbs_d_dft_err_o),
                 .wbs8_rty_o           (wbs_d_dft_rty_o),

                 .wbs9_adr_i           (wbs_d_idft_adr_i),
                 .wbs9_dat_i           (wbs_d_idft_dat_i),
                 .wbs9_sel_i           (wbs_d_idft_sel_i),
                 .wbs9_we_i            (wbs_d_idft_we_i),
                 .wbs9_cyc_i           (wbs_d_idft_cyc_i),
                 .wbs9_stb_i           (wbs_d_idft_stb_i),
                 .wbs9_cti_i           (wbs_d_idft_cti_i),
                 .wbs9_bte_i           (wbs_d_idft_bte_i),
                 .wbs9_dat_o           (wbs_d_idft_dat_o),
                 .wbs9_ack_o           (wbs_d_idft_ack_o),
                 .wbs9_err_o           (wbs_d_idft_err_o),
                 .wbs9_rty_o           (wbs_d_idft_rty_o),

                 .wbs10_adr_i          (wbs_d_fir_adr_i),
                 .wbs10_dat_i          (wbs_d_fir_dat_i),
                 .wbs10_sel_i          (wbs_d_fir_sel_i),
                 .wbs10_we_i           (wbs_d_fir_we_i),
                 .wbs10_cyc_i          (wbs_d_fir_cyc_i),
                 .wbs10_stb_i          (wbs_d_fir_stb_i),
                 .wbs10_cti_i          (wbs_d_fir_cti_i),
                 .wbs10_bte_i          (wbs_d_fir_bte_i),
                 .wbs10_dat_o          (wbs_d_fir_dat_o),
                 .wbs10_ack_o          (wbs_d_fir_ack_o),
                 .wbs10_err_o          (wbs_d_fir_err_o),
                 .wbs10_rty_o          (wbs_d_fir_rty_o),

                 .wbs11_adr_i          (wbs_d_iir_adr_i),
                 .wbs11_dat_i          (wbs_d_iir_dat_i),
                 .wbs11_sel_i          (wbs_d_iir_sel_i),
                 .wbs11_we_i           (wbs_d_iir_we_i),
                 .wbs11_cyc_i          (wbs_d_iir_cyc_i),
                 .wbs11_stb_i          (wbs_d_iir_stb_i),
                 .wbs11_cti_i          (wbs_d_iir_cti_i),
                 .wbs11_bte_i          (wbs_d_iir_bte_i),
                 .wbs11_dat_o          (wbs_d_iir_dat_o),
                 .wbs11_ack_o          (wbs_d_iir_ack_o),
                 .wbs11_err_o          (wbs_d_iir_err_o),
                 .wbs11_rty_o          (wbs_d_iir_rty_o),

                 .wbs12_adr_i          (wbs_d_gps_adr_i),
                 .wbs12_dat_i          (wbs_d_gps_dat_i),
                 .wbs12_sel_i          (wbs_d_gps_sel_i),
                 .wbs12_we_i           (wbs_d_gps_we_i),
                 .wbs12_cyc_i          (wbs_d_gps_cyc_i),
                 .wbs12_stb_i          (wbs_d_gps_stb_i),
                 .wbs12_cti_i          (wbs_d_gps_cti_i),
                 .wbs12_bte_i          (wbs_d_gps_bte_i),
                 .wbs12_dat_o          (wbs_d_gps_dat_o),
                 .wbs12_ack_o          (wbs_d_gps_ack_o),
                 .wbs12_err_o          (wbs_d_gps_err_o),
                 .wbs12_rty_o          (wbs_d_gps_rty_o),

                 .wbs13_adr_i          (),
                 .wbs13_dat_i          (),
                 .wbs13_sel_i          (),
                 .wbs13_we_i           (),
                 .wbs13_cyc_i          (),
                 .wbs13_stb_i          (),
                 .wbs13_cti_i          (),
                 .wbs13_bte_i          (),
                 .wbs13_dat_o          (empty_slot_dat_o),
                 .wbs13_ack_o          (empty_slot_ack_o),
                 .wbs13_err_o          (empty_slot_err_o),
                 .wbs13_rty_o          (empty_slot_rty_o),

                 .wbs14_adr_i          (),
                 .wbs14_dat_i          (),
                 .wbs14_sel_i          (),
                 .wbs14_we_i           (),
                 .wbs14_cyc_i          (),
                 .wbs14_stb_i          (),
                 .wbs14_cti_i          (),
                 .wbs14_bte_i          (),
                 .wbs14_dat_o          (empty_slot_dat_o),
                 .wbs14_ack_o          (empty_slot_ack_o),
                 .wbs14_err_o          (empty_slot_err_o),
                 .wbs14_rty_o          (empty_slot_rty_o),

                 .wbs15_adr_i          (),
                 .wbs15_dat_i          (),
                 .wbs15_sel_i          (),
                 .wbs15_we_i           (),
                 .wbs15_cyc_i          (),
                 .wbs15_stb_i          (),
                 .wbs15_cti_i          (),
                 .wbs15_bte_i          (),
                 .wbs15_dat_o          (empty_slot_dat_o),
                 .wbs15_ack_o          (empty_slot_ack_o),
                 .wbs15_err_o          (empty_slot_err_o),
                 .wbs15_rty_o          (empty_slot_rty_o),

                 // Clock, reset inputs
                 .wb_clk           (wb_clk),
                 .wb_rst           (wb_rst));

// These settings are from top level params file
defparam arbiter_dbus0.wb_addr_match_width = dbus_arb_wb_addr_match_width;
defparam arbiter_dbus0.wb_num_slaves = dbus_arb_wb_num_slaves;
defparam arbiter_dbus0.slave0_addr_width = dbus_arb_slave0_addr_width;
defparam arbiter_dbus0.slave1_adr = dbus_arb_slave1_adr;
defparam arbiter_dbus0.slave1_addr_width = dbus_arb_slave1_addr_width;
defparam arbiter_dbus0.slave2_addr_width = dbus_arb_slave2_addr_width;
defparam arbiter_dbus0.slave3_addr_width = dbus_arb_slave3_addr_width;
defparam arbiter_dbus0.slave4_adr = dbus_arb_slave4_adr;
defparam arbiter_dbus0.slave4_addr_width = dbus_arb_slave4_addr_width;
defparam arbiter_dbus0.slave5_adr = dbus_arb_slave5_adr;
defparam arbiter_dbus0.slave5_addr_width = dbus_arb_slave5_addr_width;
defparam arbiter_dbus0.slave6_adr = dbus_arb_slave6_adr;
defparam arbiter_dbus0.slave6_addr_width = dbus_arb_slave6_addr_width;
defparam arbiter_dbus0.slave7_adr = dbus_arb_slave7_adr;
defparam arbiter_dbus0.slave7_addr_width = dbus_arb_slave7_addr_width;
defparam arbiter_dbus0.slave8_adr = dbus_arb_slave8_adr;
defparam arbiter_dbus0.slave8_addr_width = dbus_arb_slave8_addr_width;
defparam arbiter_dbus0.slave9_adr = dbus_arb_slave9_adr;
defparam arbiter_dbus0.slave9_addr_width = dbus_arb_slave9_addr_width;
defparam arbiter_dbus0.slave10_adr = dbus_arb_slave10_adr;
defparam arbiter_dbus0.slave10_addr_width = dbus_arb_slave10_addr_width;
defparam arbiter_dbus0.slave11_adr = dbus_arb_slave11_adr;
defparam arbiter_dbus0.slave11_addr_width = dbus_arb_slave11_addr_width;
defparam arbiter_dbus0.slave12_adr = dbus_arb_slave12_adr;
defparam arbiter_dbus0.slave12_addr_width = dbus_arb_slave12_addr_width;
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// Wishbone byte-wide bus arbiter
//
////////////////////////////////////////////////////////////////////////
arbiter_bytebus arbiter_bytebus0
                (

                    // Master 0
                    // Inputs to arbiter from master
                    .wbm0_adr_o           (wbm_b_d_adr_o),
                    .wbm0_dat_o           (wbm_b_d_dat_o),
                    .wbm0_sel_o           (wbm_b_d_sel_o),
                    .wbm0_we_o            (wbm_b_d_we_o),
                    .wbm0_cyc_o           (wbm_b_d_cyc_o),
                    .wbm0_stb_o           (wbm_b_d_stb_o),
                    .wbm0_cti_o           (wbm_b_d_cti_o),
                    .wbm0_bte_o           (wbm_b_d_bte_o),

                    // Outputs to master from arbiter
                    .wbm0_dat_i           (wbm_b_d_dat_i),
                    .wbm0_ack_i           (wbm_b_d_ack_i),
                    .wbm0_err_i           (wbm_b_d_err_i),
                    .wbm0_rty_i           (wbm_b_d_rty_i),

                    // Byte bus slaves
                    .wbs0_adr_i           (wbs_d_uart0_adr_i),
                    .wbs0_dat_i           (wbs_d_uart0_dat_i),
                    .wbs0_we_i            (wbs_d_uart0_we_i),
                    .wbs0_cyc_i           (wbs_d_uart0_cyc_i),
                    .wbs0_stb_i           (wbs_d_uart0_stb_i),
                    .wbs0_cti_i           (wbs_d_uart0_cti_i),
                    .wbs0_bte_i           (wbs_d_uart0_bte_i),
                    .wbs0_dat_o           (wbs_d_uart0_dat_o),
                    .wbs0_ack_o           (wbs_d_uart0_ack_o),
                    .wbs0_err_o           (wbs_d_uart0_err_o),
                    .wbs0_rty_o           (wbs_d_uart0_rty_o),

                    .wbs1_adr_i           (),
                    .wbs1_dat_i           (),
                    .wbs1_we_i            (),
                    .wbs1_cyc_i           (),
                    .wbs1_stb_i           (),
                    .wbs1_cti_i           (),
                    .wbs1_bte_i           (),
                    .wbs1_dat_o           (8'd0),
                    .wbs1_ack_o           (1'b0),
                    .wbs1_err_o           (1'b0),
                    .wbs1_rty_o           (1'b0),

                    .wbs2_adr_i           (),
                    .wbs2_dat_i           (),
                    .wbs2_we_i            (),
                    .wbs2_cyc_i           (),
                    .wbs2_stb_i           (),
                    .wbs2_cti_i           (),
                    .wbs2_bte_i           (),
                    .wbs2_dat_o           (8'd0),
                    .wbs2_ack_o           (1'b0),
                    .wbs2_err_o           (1'b0),
                    .wbs2_rty_o           (1'b0),

                    .wbs3_adr_i           (),
                    .wbs3_dat_i           (),
                    .wbs3_we_i            (),
                    .wbs3_cyc_i           (),
                    .wbs3_stb_i           (),
                    .wbs3_cti_i           (),
                    .wbs3_bte_i           (),
                    .wbs3_dat_o           (8'd0),
                    .wbs3_ack_o           (1'b0),
                    .wbs3_err_o           (1'b0),
                    .wbs3_rty_o           (1'b0),

                    .wbs4_adr_i           (),
                    .wbs4_dat_i           (),
                    .wbs4_we_i            (),
                    .wbs4_cyc_i           (),
                    .wbs4_stb_i           (),
                    .wbs4_cti_i           (),
                    .wbs4_bte_i           (),
                    .wbs4_dat_o           (8'd0),
                    .wbs4_ack_o           (1'b0),
                    .wbs4_err_o           (1'b0),
                    .wbs4_rty_o           (1'b0),

                    // Clock, reset inputs
                    .wb_clk           (wb_clk),
                    .wb_rst           (wb_rst));
defparam arbiter_bytebus0.wb_addr_match_width = bbus_arb_wb_addr_match_width;
defparam arbiter_bytebus0.wb_num_slaves = bbus_arb_wb_num_slaves;
defparam arbiter_bytebus0.slave0_adr = bbus_arb_slave0_adr;
defparam arbiter_bytebus0.slave1_adr = bbus_arb_slave1_adr;
defparam arbiter_bytebus0.slave2_adr = bbus_arb_slave2_adr;
defparam arbiter_bytebus0.slave3_adr = bbus_arb_slave3_adr;
defparam arbiter_bytebus0.slave4_adr = bbus_arb_slave4_adr;
////////////////////////////////////////////////////////////////////////



//
// Wires
//

wire [31:0]                mor1kx_pic_ints;

wire [31:0]                mor1kx_dbg_dat_i;
wire [31:0]                mor1kx_dbg_adr_i;
wire                   mor1kx_dbg_we_i;
wire                   mor1kx_dbg_stb_i;
wire                   mor1kx_dbg_ack_o;
wire [31:0]                mor1kx_dbg_dat_o;

wire                   mor1kx_dbg_stall_i;
wire                   mor1kx_dbg_ewt_i;
wire [3:0]                 mor1kx_dbg_lss_o;
wire [1:0]                 mor1kx_dbg_is_o;
wire [10:0]                mor1kx_dbg_wp_o;
wire                   mor1kx_dbg_bp_o;
wire                   mor1kx_dbg_rst;

wire                   mor1kx_clk, mor1kx_rst;
wire                   sig_tick;

////////////////////////////////////////////////////////////////////////
//
// Mor1kx OpenRISC processor
//
////////////////////////////////////////////////////////////////////////

//
// Assigns
//
assign mor1kx_clk = wb_clk;
assign mor1kx_rst = wb_rst | mor1kx_dbg_rst;

mor1kx mor1kx0 (
    // Clock and Reset
    .clk                        (mor1kx_clk),
    .rst                        (mor1kx_rst),

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
    .du_addr_i                  (mor1kx_dbg_adr_i[15:0]),
    .du_stb_i                   (mor1kx_dbg_stb_i),
    .du_dat_i                   (mor1kx_dbg_dat_i),
    .du_we_i                    (mor1kx_dbg_we_i),
    .du_dat_o                   (mor1kx_dbg_dat_o),
    .du_ack_o                   (mor1kx_dbg_ack_o),

    // Stall control from debug interface
    .du_stall_i                 (mor1kx_dbg_stall_i),
    .du_stall_o                 (),

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
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// Generic Wishbone RAM component
//
////////////////////////////////////////////////////////////////////////
`ifndef SRAM_INITIALIZATION_FILE
    `define SRAM_INITIALIZATION_FILE    "sram.vmem"
`endif

ram_wb #(
    .dw                 (wb_aw),
    .aw                 (wb_dw),
    .mem_size_bytes     (1 << wbs_i_rom0_addr_width),  // 128K bytes
    .mem_adr_width      (wbs_i_rom0_addr_width),
    .memory_file        (`SRAM_INITIALIZATION_FILE)
) ram_wb0 (
           // Wishbone slave interface 0
           .wbm0_dat_i           (wbs_i_ram0_dat_i),
           .wbm0_adr_i           (wbs_i_ram0_adr_i),
           .wbm0_sel_i           (wbs_i_ram0_sel_i),
           .wbm0_cti_i           (wbs_i_ram0_cti_i),
           .wbm0_bte_i           (wbs_i_ram0_bte_i),
           .wbm0_we_i            (wbs_i_ram0_we_i ),
           .wbm0_cyc_i           (wbs_i_ram0_cyc_i),
           .wbm0_stb_i           (wbs_i_ram0_stb_i),
           .wbm0_dat_o           (wbs_i_ram0_dat_o),
           .wbm0_ack_o           (wbs_i_ram0_ack_o),
           .wbm0_err_o           (wbs_i_ram0_err_o),
           .wbm0_rty_o           (wbs_i_ram0_rty_o),
           // Wishbone slave interface 1
           .wbm1_dat_i           (wbs_d_ram0_dat_i),
           .wbm1_adr_i           (wbs_d_ram0_adr_i),
           .wbm1_sel_i           (wbs_d_ram0_sel_i),
           .wbm1_cti_i           (wbs_d_ram0_cti_i),
           .wbm1_bte_i           (wbs_d_ram0_bte_i),
           .wbm1_we_i            (wbs_d_ram0_we_i ),
           .wbm1_cyc_i           (wbs_d_ram0_cyc_i),
           .wbm1_stb_i           (wbs_d_ram0_stb_i),
           .wbm1_dat_o           (wbs_d_ram0_dat_o),
           .wbm1_ack_o           (wbs_d_ram0_ack_o),
           .wbm1_err_o           (wbs_d_ram0_err_o),
           .wbm1_rty_o           (wbs_d_ram0_rty_o),
           // Wishbone slave interface 2
           .wbm2_dat_i           (32'b0),
           .wbm2_adr_i           (32'b0),
           .wbm2_sel_i           (4'b0),
           .wbm2_cti_i           (3'b0),
           .wbm2_bte_i           (2'b0),
           .wbm2_we_i            (1'b0),
           .wbm2_cyc_i           (1'b0),
           .wbm2_stb_i           (1'b0),
           .wbm2_dat_o           (),
           .wbm2_ack_o           (),
           .wbm2_err_o           (),
           .wbm2_rty_o           (),
           // Clock, reset
           .wb_clk_i             (wb_clk),
           .wb_rst_i             (wb_rst));
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// AES
//
////////////////////////////////////////////////////////////////////////
`ifdef AES
    aes_top aes (
        // Wishbone Slave interface
        .wb_clk_i(wb_clk),
        .wb_rst_i(wb_rst),
        .wb_dat_i(wbs_d_aes_dat_i),
        .wb_adr_i(wbs_d_aes_adr_i),
        .wb_sel_i(wbs_d_aes_sel_i[3:0]),
        .wb_we_i (wbs_d_aes_we_i),
        .wb_cyc_i(wbs_d_aes_cyc_i),
        .wb_stb_i(wbs_d_aes_stb_i),
        .wb_dat_o(wbs_d_aes_dat_o),
        .wb_err_o(wbs_d_aes_err_o),
        .wb_ack_o(wbs_d_aes_ack_o),

        // Processor interrupt
        .int_o(aes_irq)
    );

    assign wbs_d_aes_rty_o = 0;

`else
    assign wbs_d_aes_dat_o = 0;
    assign wbs_d_aes_err_o = 0;
    assign wbs_d_aes_ack_o = 0;
    assign wbs_d_aes_rty_o = 0;
    assign aes_irq = 0;
`endif
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// MD5
//
////////////////////////////////////////////////////////////////////////
`ifdef MD5
    md5_top md5 (
        // Wishbone Slave interface
        .wb_clk_i(wb_clk),
        .wb_rst_i(wb_rst),
        .wb_dat_i(wbs_d_md5_dat_i),
        .wb_adr_i(wbs_d_md5_adr_i),
        .wb_sel_i(wbs_d_md5_sel_i[3:0]),
        .wb_we_i (wbs_d_md5_we_i),
        .wb_cyc_i(wbs_d_md5_cyc_i),
        .wb_stb_i(wbs_d_md5_stb_i),
        .wb_dat_o(wbs_d_md5_dat_o),
        .wb_err_o(wbs_d_md5_err_o),
        .wb_ack_o(wbs_d_md5_ack_o),

        // Processor interrupt
        .int_o(md5_irq)
    );

    assign wbs_d_md5_rty_o = 0;
`else
    assign wbs_d_md5_dat_o = 0;
    assign wbs_d_md5_err_o = 0;
    assign wbs_d_md5_ack_o = 0;
    assign wbs_d_md5_rty_o = 0;
    assign md5_irq = 0;
`endif
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// SHA
//
////////////////////////////////////////////////////////////////////////
`ifdef SHA
    sha256_top sha (
        // Wishbone Slave interface
        .wb_clk_i(wb_clk),
        .wb_rst_i(wb_rst),
        .wb_dat_i(wbs_d_sha_dat_i),
        .wb_adr_i(wbs_d_sha_adr_i),
        .wb_sel_i(wbs_d_sha_sel_i[3:0]),
        .wb_we_i (wbs_d_sha_we_i),
        .wb_cyc_i(wbs_d_sha_cyc_i),
        .wb_stb_i(wbs_d_sha_stb_i),
        .wb_dat_o(wbs_d_sha_dat_o),
        .wb_err_o(wbs_d_sha_err_o),
        .wb_ack_o(wbs_d_sha_ack_o),

        // Processor interrupt
        .int_o(sha_irq)
    );

    assign wbs_d_sha_rty_o = 0;
`else
    assign wbs_d_sha_dat_o = 0;
    assign wbs_d_sha_err_o = 0;
    assign wbs_d_sha_ack_o = 0;
    assign wbs_d_sha_rty_o = 0;
    assign sha_irq = 0;
`endif
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// RSA
//
////////////////////////////////////////////////////////////////////////
`ifdef RSA
    modexp_top rsa (
        // Wishbone Slave interface
        .wb_clk_i(wb_clk),
        .wb_rst_i(wb_rst),
        .wb_dat_i(wbs_d_rsa_dat_i),
        .wb_adr_i(wbs_d_rsa_adr_i),
        .wb_sel_i(wbs_d_rsa_sel_i[3:0]),
        .wb_we_i (wbs_d_rsa_we_i),
        .wb_cyc_i(wbs_d_rsa_cyc_i),
        .wb_stb_i(wbs_d_rsa_stb_i),
        .wb_dat_o(wbs_d_rsa_dat_o),
        .wb_err_o(wbs_d_rsa_err_o),
        .wb_ack_o(wbs_d_rsa_ack_o),

        // Processor interrupt
        .int_o(rsa_irq)
    );

    assign wbs_d_rsa_rty_o = 0;
`else
    assign wbs_d_rsa_dat_o = 0;
    assign wbs_d_rsa_err_o = 0;
    assign wbs_d_rsa_ack_o = 0;
    assign wbs_d_rsa_rty_o = 0;
    assign rsa_irq = 0;
`endif
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// DES3
//
////////////////////////////////////////////////////////////////////////
`ifdef DES3
    des3_top des3 (
        // Wishbone Slave interface
        .wb_clk_i(wb_clk),
        .wb_rst_i(wb_rst),
        .wb_dat_i(wbs_d_des3_dat_i),
        .wb_adr_i(wbs_d_des3_adr_i),
        .wb_sel_i(wbs_d_des3_sel_i[3:0]),
        .wb_we_i (wbs_d_des3_we_i),
        .wb_cyc_i(wbs_d_des3_cyc_i),
        .wb_stb_i(wbs_d_des3_stb_i),
        .wb_dat_o(wbs_d_des3_dat_o),
        .wb_err_o(wbs_d_des3_err_o),
        .wb_ack_o(wbs_d_des3_ack_o),

        // Processor interrupt
        .int_o(des3_irq)
    );

    assign wbs_d_des3_rty_o = 0;
`else
    assign wbs_d_des3_dat_o = 0;
    assign wbs_d_des3_err_o = 0;
    assign wbs_d_des3_ack_o = 0;
    assign wbs_d_des3_rty_o = 0;
    assign des3_irq = 0;
`endif
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// DFT
//
////////////////////////////////////////////////////////////////////////
`ifdef DFT
    dft_top_top dft (
        // Wishbone Slave interface
        .wb_clk_i(wb_clk),
        .wb_rst_i(wb_rst),
        .wb_dat_i(wbs_d_dft_dat_i),
        .wb_adr_i(wbs_d_dft_adr_i),
        .wb_sel_i(wbs_d_dft_sel_i[3:0]),
        .wb_we_i (wbs_d_dft_we_i),
        .wb_cyc_i(wbs_d_dft_cyc_i),
        .wb_stb_i(wbs_d_dft_stb_i),
        .wb_dat_o(wbs_d_dft_dat_o),
        .wb_err_o(wbs_d_dft_err_o),
        .wb_ack_o(wbs_d_dft_ack_o),

        // Processor interrupt
        .int_o(dft_irq)
    );

    assign wbs_d_dft_rty_o = 0;
`else
    assign wbs_d_dft_dat_o = 0;
    assign wbs_d_dft_err_o = 0;
    assign wbs_d_dft_ack_o = 0;
    assign wbs_d_dft_rty_o = 0;
    assign dft_irq = 0;
`endif
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// IDFT
//
////////////////////////////////////////////////////////////////////////
`ifdef IDFT
    idft_top_top idft (
        // Wishbone Slave interface
        .wb_clk_i(wb_clk),
        .wb_rst_i(wb_rst),
        .wb_dat_i(wbs_d_idft_dat_i),
        .wb_adr_i(wbs_d_idft_adr_i),
        .wb_sel_i(wbs_d_idft_sel_i[3:0]),
        .wb_we_i (wbs_d_idft_we_i),
        .wb_cyc_i(wbs_d_idft_cyc_i),
        .wb_stb_i(wbs_d_idft_stb_i),
        .wb_dat_o(wbs_d_idft_dat_o),
        .wb_err_o(wbs_d_idft_err_o),
        .wb_ack_o(wbs_d_idft_ack_o),

        // Processor interrupt
        .int_o(idft_irq)
    );

    assign wbs_d_idft_rty_o = 0;
`else
    assign wbs_d_idft_dat_o = 0;
    assign wbs_d_idft_err_o = 0;
    assign wbs_d_idft_ack_o = 0;
    assign wbs_d_idft_rty_o = 0;
    assign idft_irq = 0;
`endif
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// IIR
//
////////////////////////////////////////////////////////////////////////
`ifdef IIR
    iir_top iir (
        // Wishbone Slave interface
        .wb_clk_i(wb_clk),
        .wb_rst_i(wb_rst),
        .wb_dat_i(wbs_d_iir_dat_i),
        .wb_adr_i(wbs_d_iir_adr_i),
        .wb_sel_i(wbs_d_iir_sel_i[3:0]),
        .wb_we_i (wbs_d_iir_we_i),
        .wb_cyc_i(wbs_d_iir_cyc_i),
        .wb_stb_i(wbs_d_iir_stb_i),
        .wb_dat_o(wbs_d_iir_dat_o),
        .wb_err_o(wbs_d_iir_err_o),
        .wb_ack_o(wbs_d_iir_ack_o),

        // Processor interrupt
        .int_o(iir_irq)
    );

    assign wbs_d_iir_rty_o = 0;
`else
    assign wbs_d_iir_dat_o = 0;
    assign wbs_d_iir_err_o = 0;
    assign wbs_d_iir_ack_o = 0;
    assign wbs_d_iir_rty_o = 0;
    assign iir_irq = 0;
`endif
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// FIR
//
////////////////////////////////////////////////////////////////////////
`ifdef FIR
    fir_top fir (
        // Wishbone Slave interface
        .wb_clk_i(wb_clk),
        .wb_rst_i(wb_rst),
        .wb_dat_i(wbs_d_fir_dat_i),
        .wb_adr_i(wbs_d_fir_adr_i),
        .wb_sel_i(wbs_d_fir_sel_i[3:0]),
        .wb_we_i (wbs_d_fir_we_i),
        .wb_cyc_i(wbs_d_fir_cyc_i),
        .wb_stb_i(wbs_d_fir_stb_i),
        .wb_dat_o(wbs_d_fir_dat_o),
        .wb_err_o(wbs_d_fir_err_o),
        .wb_ack_o(wbs_d_fir_ack_o),

        // Processor interrupt
        .int_o(fir_irq)
    );

    assign wbs_d_fir_rty_o = 0;
`else
    assign wbs_d_fir_dat_o = 0;
    assign wbs_d_fir_err_o = 0;
    assign wbs_d_fir_ack_o = 0;
    assign wbs_d_fir_rty_o = 0;
    assign fir_irq = 0;
`endif
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// GPS
//
////////////////////////////////////////////////////////////////////////
`ifdef GPS
    gps_top gps (
        // Wishbone Slave interface
        .wb_clk_i(wb_clk),
        .wb_rst_i(wb_rst),
        .wb_dat_i(wbs_d_gps_dat_i),
        .wb_adr_i(wbs_d_gps_adr_i),
        .wb_sel_i(wbs_d_gps_sel_i[3:0]),
        .wb_we_i (wbs_d_gps_we_i),
        .wb_cyc_i(wbs_d_gps_cyc_i),
        .wb_stb_i(wbs_d_gps_stb_i),
        .wb_dat_o(wbs_d_gps_dat_o),
        .wb_err_o(wbs_d_gps_err_o),
        .wb_ack_o(wbs_d_gps_ack_o),

        // Processor interrupt
        .int_o(gps_irq)
    );

    assign wbs_d_gps_rty_o = 0;
`else
    assign wbs_d_gps_dat_o = 0;
    assign wbs_d_gps_err_o = 0;
    assign wbs_d_gps_ack_o = 0;
    assign wbs_d_gps_rty_o = 0;
    assign gps_irq = 0;
`endif
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// UART0
//
////////////////////////////////////////////////////////////////////////
`ifdef UART0
    //
    // Wires
    //
    wire        uart0_srx;
    wire        uart0_stx;
    wire        uart0_irq;

    //
    // Assigns
    //
    assign wbs_d_uart0_err_o = 0;
    assign wbs_d_uart0_rty_o = 0;
    assign uart0_srx = uart0_srx_pad_i;
    assign uart0_stx_pad_o = uart0_stx;

    uart_top uart16550_0 (
        // Wishbone slave interface
        .wb_clk_i             (wb_clk),
        .wb_rst_i             (wb_rst),
        .wb_adr_i             (wbs_d_uart0_adr_i[uart0_addr_width-1:0]),
        .wb_dat_i             (wbs_d_uart0_dat_i),
        .wb_we_i              (wbs_d_uart0_we_i),
        .wb_stb_i             (wbs_d_uart0_stb_i),
        .wb_cyc_i             (wbs_d_uart0_cyc_i),
        .wb_sel_i             (4'hF),
        .wb_dat_o             (wbs_d_uart0_dat_o),
        .wb_ack_o             (wbs_d_uart0_ack_o),

        .int_o                (uart0_irq),
        .stx_pad_o            (uart0_stx),
        .rts_pad_o            (uart0_rts_pad_0),
        .dtr_pad_o            (),

        // Inputs
        .srx_pad_i            (uart0_srx),
        .cts_pad_i            (uart0_cts_pad_i),
        .dsr_pad_i            (1'b0),
        .ri_pad_i             (1'b0),
        .dcd_pad_i            (1'b0));
`else // !`ifdef UART0
    //
    // Assigns
    //
    assign wbs_d_uart0_err_o = 0;
    assign wbs_d_uart0_rty_o = 0;
    assign wbs_d_uart0_ack_o = 0;
    assign wbs_d_uart0_dat_o = 0;

`endif // !`ifdef UART0
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// DEBUGGING IO
//
////////////////////////////////////////////////////////////////////////
`ifdef DEBUGGING_GPIO
// Buttons map I-bus address bits onto user LEDs (for debugging)
assign GPIO_LED[7:0] = button_W ? wbm_i_or1k_adr_o[31:24] : button_N ? wbm_i_or1k_adr_o[23:16] : button_E ? wbm_i_or1k_adr_o[15:8] : wbm_i_or1k_adr_o[7:0];
`endif
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
//
// mor1kx Interrupt assignment
//
////////////////////////////////////////////////////////////////////////

assign mor1kx_pic_ints[0] = 0; // Non-maskable inside mor1kx
assign mor1kx_pic_ints[1] = 0; // Non-maskable inside mor1kx
`ifdef UART0
assign mor1kx_pic_ints[2] = uart0_irq;
`else
assign mor1kx_pic_ints[2] = 0;
`endif

assign mor1kx_pic_ints[3] = 0;
assign mor1kx_pic_ints[4] = 0;
assign mor1kx_pic_ints[5] = 0;
assign mor1kx_pic_ints[6] = 0;
assign mor1kx_pic_ints[7] = 0;
assign mor1kx_pic_ints[8] = 0;
assign mor1kx_pic_ints[9] = 0;
assign mor1kx_pic_ints[10] = 0;
assign mor1kx_pic_ints[11] = 0;
assign mor1kx_pic_ints[12] = 0;
assign mor1kx_pic_ints[13] = 0;
assign mor1kx_pic_ints[14] = 0;
assign mor1kx_pic_ints[15] = 0;
assign mor1kx_pic_ints[16] = 0;
assign mor1kx_pic_ints[17] = 0;
assign mor1kx_pic_ints[18] = 0;
assign mor1kx_pic_ints[19] = 0;
assign mor1kx_pic_ints[20] = 0;
assign mor1kx_pic_ints[21] = 0;
assign mor1kx_pic_ints[22] = 0;
assign mor1kx_pic_ints[23] = 0;
assign mor1kx_pic_ints[24] = 0;
assign mor1kx_pic_ints[25] = 0;
assign mor1kx_pic_ints[26] = 0;
assign mor1kx_pic_ints[27] = 0;
assign mor1kx_pic_ints[28] = 0;
assign mor1kx_pic_ints[29] = 0;
assign mor1kx_pic_ints[30] = 0;
assign mor1kx_pic_ints[31] = 0;

endmodule // orpsoc_top

