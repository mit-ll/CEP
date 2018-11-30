//
// Copyright (C) 2018 Massachusetts Institute of Technology
//
// File         : orpsoc_top.sv
// Project      : Common Evaluation Platform (CEP)
// Description  : Top-level HDL file for the AXI4-Lite / MOR1KX variant of the CEP
// Notes        : Core licensing information may be found in licenseLog.txt
//

//////////////////////////////////////////////////////////////////////
///                                                               ////
/// ORPSoC top for VC707 board                                    ////
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

// Used for the various levels of debug (higher levels define lower levels)
`ifdef DEBUG5
    `define DEBUG4
    `define DEBUG3
    `define DEBUG2
    `define DEBUG1
`elsif DEBUG4
    `define DEBUG3
    `define DEBUG2
    `define DEBUG1
`elsif DEBUG3
    `define DEBUG2
    `define DEBUG1
`elsif DEBUG2
    `define DEBUG1
`endif

//import axi_test_extended::*;

`include "orpsoc-defines.sv"
`include "mor1kx-defines.v"

module orpsoc_top (

    sys_clk_in_p, 
    sys_clk_in_n,

    uart_srx_pad_i, 
    uart_stx_pad_o, 
    uart_rts_pad_o, 
    uart_cts_pad_i,

    GPIO_LED, 
    button_W, 
    button_N, 
    button_E, 
    button_S,

`ifdef RESET_HIGH
    rst_pad_i
`else
    rst_n_pad_i
`endif
 );

    input           sys_clk_in_p;
    input           sys_clk_in_n;

`ifdef RESET_HIGH
    input           rst_pad_i;
`else
    input           rst_n_pad_i;
`endif

    input           uart_srx_pad_i;
    output          uart_stx_pad_o;
    output          uart_rts_pad_o;
    input           uart_cts_pad_i;

    output [7:0]    GPIO_LED;
    input           button_W;
    input           button_N;
    input           button_E;
    input           button_S;

//
// For now, tie off the GPIO_LED signals
//
assign GPIO_LED     = 8'h00;

//
// Clock and Reset signals
//
wire                   core_clk;
wire                   core_rst;


//
// AXI4-Lite declerations for both the Instruction and Data Buses 
//

// Master Bus defininitions
// Master 0 - Instruction Bus
// Master 1 - Data Bus
AXI_LITE #(
    .AXI_ADDR_WIDTH (`CEP_AXI_ADDR_WIDTH),
    .AXI_DATA_WIDTH (`CEP_AXI_DATA_WIDTH)
) master[1:0](.clk_i(core_clk));

//
// Slave Bus Declaration (see orpsoc-defines.sv for additional info including
//      slave assignment)
//
AXI_LITE #(
    .AXI_ADDR_WIDTH (`CEP_AXI_ADDR_WIDTH),
    .AXI_DATA_WIDTH (`CEP_AXI_DATA_WIDTH)
) slave[11:0](.clk_i(core_clk));

//
// Decleration the routing rules for the AXI4-Lite Crossbar
//
AXI_ROUTING_RULES #(
    .AXI_ADDR_WIDTH (`CEP_AXI_ADDR_WIDTH),
    .NUM_SLAVE      (`CEP_NUM_OF_SLAVES),
    .NUM_RULES      (1)
) routing();

//
// Assign the routing rules (cep_routing_rules is declared and
// explained in orpsoc-defines.v)
//
for (genvar i = 0; i < `CEP_NUM_OF_SLAVES; i++) begin
    assign routing.rules[i][0].enabled  = cep_routing_rules[i][0][0];
    assign routing.rules[i][0].mask     = cep_routing_rules[i][1];
    assign routing.rules[i][0].base     = cep_routing_rules[i][2];
end // for (genvar i = 0; i < CEP_NUM_OF_SLAVES; i++)

//
// Clock and reset generation module
//
clkgen clkgen_inst (
    .sys_clk_in_p           (sys_clk_in_p),
    .sys_clk_in_n           (sys_clk_in_n),
    .core_clk_o             (core_clk),
    .core_rst_o             (core_rst),

    // Asynchronous reset (Active Low or Active High)
`ifdef RESET_HIGH
    .rst_n_pad_i            (~rst_pad_i)
`else
    .rst_n_pad_i            (rst_n_pad_i)
`endif
);

//
// Instantiate the AXI4-Lite crossbar
//
axi_lite_xbar #(
    .ADDR_WIDTH     (`CEP_AXI_ADDR_WIDTH ),
    .DATA_WIDTH     (`CEP_AXI_DATA_WIDTH ),
    .NUM_MASTER     (`CEP_NUM_OF_MASTERS ),
    .NUM_SLAVE      (`CEP_NUM_OF_SLAVES  ),
    .NUM_RULES      (1)
) axi_lite_xbar_inst (
    .clk_i          ( core_clk          ),
    .rst_ni         ( ~core_rst         ),
    .master         ( master            ),
    .slave          ( slave             ),
    .rules          ( routing           )
);

//
// MOR1KX OpenRISC processor (Mandatory)
//
wire [31:0] mor1kx_pic_ints;

// Debug interface is stubbed out (for now)
wire mor1kx_dbg_rst;
assign mor1kx_dbg_rst = 1'b0;

mor1kx_top_axi4lite mor1kx_top_axi4lite_inst (
    // Clock and Resets
    .clk_i              (core_clk),
    .rst_i              (core_rst | mor1kx_dbg_rst),

    // AXI4-Lite Instruction and Data master interfaces
    .master_i           (master[0]),
    .master_d           (master[1]),

    // MOR1KX Interrupt
    .mor1kx_pic_ints    (mor1kx_pic_ints),

    // MOR1KX Debug interface (currently unused)
    .mor1kx_dbg_adr_i   (16'd0),
    .mor1kx_dbg_stb_i   (1'd0),
    .mor1kx_dbg_dat_i   (`CEP_AXI_DATA_WIDTH'd0),
    .mor1kx_dbg_we_i    (1'd0),
    .mor1kx_dbg_dat_o   (),
    .mor1kx_dbg_ack_o   (),
    .mor1kx_dbg_stall_i (1'd0),
    .mor1kx_dbg_stall_o ()
);

//
// Generic AXI4-Lite RAM component (Mandatory)
//
ram_top_axi4lite #(
    .MEMORY_SIZE(`CEP_RAM_SIZE)
) ram_top_axi4lite_inst (
    .clk_i              (core_clk                   ),
    .rst_ni             (~core_rst                  ),
    .slave              (slave[`RAM_SLAVE_NUMBER]   )
);

//
// UART (Mandatory)
//
wire uart_irq;

uart_top_axi4lite uart_top_axi4lite_inst (
    .clk_i          ( core_clk                      ),
    .rst_ni         ( ~core_rst                     ),
    .slave          ( slave[`UART_SLAVE_NUMBER]     ),

    // UART Signals
    .srx_pad_i      (uart_srx_pad_i ),
    .stx_pad_o      (uart_stx_pad_o ),
    .rts_pad_o      (uart_rts_pad_o ),
    .cts_pad_i      (uart_cts_pad_i ),
    .dtr_pad_o      (               ),
    .dsr_pad_i      (1'b0           ),
    .ri_pad_i       (1'b0           ),
    .dcd_pad_i      (1'b0           ),

    // Processor Interrupt
    .int_o          (uart_irq       )
);

//
// AES (Optional - See orpsoc-defines.v)
//
generate
if(cep_routing_rules[`AES_SLAVE_NUMBER][0] == `CEP_SLAVE_ENABLED)
    aes_top_axi4lite aes_top_axi4lite_inst (
        .clk_i          ( core_clk                  ),
        .rst_ni         ( ~core_rst                 ),
        .slave          ( slave[`AES_SLAVE_NUMBER]  )
    );
endgenerate

//
// MD5 (Optional - See orpsoc-defines.v)
//
generate
if(cep_routing_rules[`MD5_SLAVE_NUMBER][0] == `CEP_SLAVE_ENABLED)
    md5_top_axi4lite md5_top_axi4lite_inst (
        .clk_i          ( core_clk                  ),
        .rst_ni         ( ~core_rst                 ),
        .slave          ( slave[`MD5_SLAVE_NUMBER]  )
    );
endgenerate

//
// SHA256 (Optional - See orpsoc-defines.v)
//
generate
if(cep_routing_rules[`SHA256_SLAVE_NUMBER][0] == `CEP_SLAVE_ENABLED)
    sha256_top_axi4lite sha256_top_axi4lite_inst (
        .clk_i          ( core_clk                      ),
        .rst_ni         ( ~core_rst                     ),
        .slave          ( slave[`SHA256_SLAVE_NUMBER]   )
    );
endgenerate

//
// RSA (Optional - See orpsoc-defines.v)
//
generate
if(cep_routing_rules[`RSA_SLAVE_NUMBER][0] == `CEP_SLAVE_ENABLED)
    rsa_top_axi4lite rsa_top_axi4lite_inst (
        .clk_i          ( core_clk                      ),
        .rst_ni         ( ~core_rst                     ),
        .slave          ( slave[`RSA_SLAVE_NUMBER]      )
    );
endgenerate

//
// DES3 (Optional - See orpsoc-defines.v)
//
generate
if(cep_routing_rules[`DES3_SLAVE_NUMBER][0] == `CEP_SLAVE_ENABLED)
    des3_top_axi4lite des3_top_axi4lite_inst (
        .clk_i          ( core_clk                      ),
        .rst_ni         ( ~core_rst                     ),
        .slave          ( slave[`DES3_SLAVE_NUMBER]      )
    );
endgenerate

//
// DFT (Optional - See orpsoc-defines.v)
//
generate
if(cep_routing_rules[`DFT_SLAVE_NUMBER][0] == `CEP_SLAVE_ENABLED)
    dft_top_axi4lite dft_top_axi4lite_inst (
        .clk_i          ( core_clk                      ),
        .rst_ni         ( ~core_rst                     ),
        .slave          ( slave[`DFT_SLAVE_NUMBER]      )
    );
endgenerate

//
// IDFT (Optional - See orpsoc-defines.v)
//
generate
if(cep_routing_rules[`IDFT_SLAVE_NUMBER][0] == `CEP_SLAVE_ENABLED)
    idft_top_axi4lite idft_top_axi4lite_inst (
        .clk_i          ( core_clk                      ),
        .rst_ni         ( ~core_rst                     ),
        .slave          ( slave[`IDFT_SLAVE_NUMBER]     )
    );
endgenerate

//
// IIR (Optional - See orpsoc-defines.v)
//
generate
if(cep_routing_rules[`IIR_SLAVE_NUMBER][0] == `CEP_SLAVE_ENABLED)
    iir_top_axi4lite iir_top_axi4lite_inst (
        .clk_i          ( core_clk                      ),
        .rst_ni         ( ~core_rst                     ),
        .slave          ( slave[`IIR_SLAVE_NUMBER]      )
    );
endgenerate

//
// FIR (Optional - See orpsoc-defines.v)
//
generate
if(cep_routing_rules[`FIR_SLAVE_NUMBER][0] == `CEP_SLAVE_ENABLED)
    fir_top_axi4lite fir_top_axi4lite_inst (
        .clk_i          ( core_clk                      ),
        .rst_ni         ( ~core_rst                     ),
        .slave          ( slave[`FIR_SLAVE_NUMBER]      )
    );
endgenerate

//
// GPS (Optional - See orpsoc-defines.v)
//
wire               gps_clk_fast;
wire               gps_clk_slow;

generate
if(cep_routing_rules[`GPS_SLAVE_NUMBER][0] == `CEP_SLAVE_ENABLED) begin
    // The GPS core leverages some additional clocks.  The clkgen component
    // has been removed from the core itself and is now generated at the top 
    // level of the CEP.
    // 10.23 MHz for P-code and 1.023 MHz for C/A code
    gps_clkgen gps_clkgen_inst (
        .sys_clk_50             ( core_clk     ),
        .sync_rst_in            ( core_rst     ),
        .gps_clk_fast           ( gps_clk_fast ),
        .gps_clk_slow           ( gps_clk_slow ),
        .gps_rst                (              )
    );

    // GPS Core
    gps_top_axi4lite gps_top_axi4lite_inst (
        .clk_i          ( core_clk                      ),
        .rst_ni         ( ~core_rst                     ),
        .gps_clk_fast   ( gps_clk_fast                  ),
        .gps_clk_slow   ( gps_clk_slow                  ),
        .slave          ( slave[`GPS_SLAVE_NUMBER]      )
    );
end else begin
    assign  gps_clk_fast = 0;
    assign  gps_clk_slow = 0;
end // end else
endgenerate

//
// mor1kx Interrupt assignment
//
assign mor1kx_pic_ints[0] = 0; // Non-maskable inside mor1kx
assign mor1kx_pic_ints[1] = 0; // Non-maskable inside mor1kx
assign mor1kx_pic_ints[2] = uart_irq;
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

